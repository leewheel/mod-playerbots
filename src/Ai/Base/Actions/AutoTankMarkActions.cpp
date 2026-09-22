/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AutoTankMarkActions.h"

#include "AttackersValue.h"
#include "Creature.h"
#include "Event.h"
#include "Group.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"
#include "Timer.h"

// By leewheel 2026-09-22: 月亮（CC）标记的两条超时参数（参考 mod-playerbots-dungeon-lead
// 的 dungeonLeadCcTimeoutSeconds / dungeonLeadCcAbsoluteTimeoutSeconds 语义，但不引入新配置项，
// 直接用文件内常量，避免牵动 conf 结构与下发）：
//   - 宽限窗：从"被标怪进入战斗"起算，给 CC 职业留出选目标 + 读条的时间；
//   - 绝对上限：从"标记落地"起算且不重置，无论目标进不进战斗，到点一律释放，
//     防止一只永远不会参战的怪把月亮图标永久占住（参考实现 v1 的原始 bug）。
static constexpr uint32 AutoTankMarkCcGraceMs = 8000;
static constexpr uint32 AutoTankMarkCcAbsoluteTimeoutMs = 45000;

// By leewheel 2026-09-22: 月亮候选的最大搜索半径（以骷髅为圆心）。
// 参考实现用 40 码，但那是在"以 boss 为圆心 + 有 route 表"的前提下；
// 我方没有 route 表，缩小到 25 码，只收紧贴首领的那几只侍从/ADD，避免跨包误标。
static constexpr float AutoTankMarkCcSearchRange = 25.0f;

// By leewheel 2026-09-22: 目标的"威胁等级"分级。
//   参考 mod-playerbots-dungeon-lead 的 FindBossNear 只认 boss 的思路扩展而来：
//   3 = 副本首领 / 世界首领，2 = 精英（含稀有精英），1 = 稀有，0 = 普通怪。
//   旧实现只按 GetMaxHealth() 选目标，副本里血最厚的往往是被 buff 过的小怪而不是首领，
//   这就是玩家反馈"标记的目标不对劲"的直接来源。
static int GetThreatRank(Unit* unit)
{
    if (!unit || !unit->IsCreature())
        return 0;

    Creature* creature = unit->ToCreature();
    if (!creature)
        return 0;

    if (creature->IsDungeonBoss() || creature->isWorldBoss())
        return 3;

    CreatureTemplate const* ct = creature->GetCreatureTemplate();
    if (!ct)
        return 0;

    switch (ct->rank)
    {
        case CREATURE_ELITE_WORLDBOSS:
            return 3;
        case CREATURE_ELITE_RAREELITE:
        case CREATURE_ELITE_ELITE:
            return 2;
        case CREATURE_ELITE_RARE:
            return 1;
        default:
            return 0;
    }
}

// By leewheel 2026-09-22: 判断某只怪是否"适合被月亮标记"（即值得占用 CC 资源）。
//   参考 mod-playerbots-dungeon-lead 的 DungeonLeadMarkAction::FindCcCandidate：
//   - 排除骷髅自身、玩家、已死亡/消失/正在移除的目标；
//   - 排除副本首领 / 世界首领（免疫控制，标了也是浪费）；
//   - 只收精英级别（参考实现只收 CREATURE_ELITE_ELITE，我方额外接受
//     稀有精英与稀有，因为 335 副本里需要控制的这两类同样常见）。
static bool IsValidCcCandidate(Unit* unit, Unit* skull)
{
    if (!unit || unit == skull)
        return false;

    if (unit->IsPlayer() || !unit->IsCreature())
        return false;

    if (!unit->IsAlive() || !unit->IsInWorld() || unit->IsDuringRemoveFromWorld())
        return false;

    Creature* creature = unit->ToCreature();
    if (!creature)
        return false;

    if (creature->IsDungeonBoss() || creature->isWorldBoss())
        return false;

    CreatureTemplate const* ct = creature->GetCreatureTemplate();
    if (!ct)
        return false;

    return ct->rank == CREATURE_ELITE_ELITE || ct->rank == CREATURE_ELITE_RAREELITE ||
           ct->rank == CREATURE_ELITE_RARE;
}

// By leewheel 2026-09-22: 队伍里是否有"真正能对被标目标释放控制"的职业。
//   月亮标记会让 DPS 主动避开该目标（上游 FindNonCcTargetStrategy::IsCcTarget 会把
//   rti cc 目标排除在普通输出目标之外），所以队伍里根本没人能控时，标月亮只会让这只怪
//   白白"没人打"。只认拥有 RtiCcTrigger 子类的两个职业：
//   德鲁伊（缠绕 / 休眠 / 旋风）与术士（放逐 / 恐惧）。
static bool GroupHasCrowdControl(Group* group)
{
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
            continue;

        uint8 const cls = member->getClass();
        if (cls == CLASS_DRUID || cls == CLASS_WARLOCK)
            return true;
    }

    return false;
}

// By leewheel 2026-09-22: 释放月亮标记（清图标 + 清本地计时状态）。
static void ReleaseCcMark(PlayerbotAI* botAI, Group* group, ObjectGuid const& ccGuid, Player* bot)
{
    if (group && bot && !ccGuid.IsEmpty() && group->GetTargetIcon(RtiTargetValue::moonIndex) == ccGuid)
        group->SetTargetIcon(RtiTargetValue::moonIndex, bot->GetGUID(), ObjectGuid::Empty);

    if (botAI)
    {
        botAI->autoTankMarkCcGuid.Clear();
        botAI->autoTankMarkCcMarkedTs = 0;
        botAI->autoTankMarkCcAbsoluteTs = 0;
        botAI->autoTankMarkCcLandedTold = false;
    }
}

// By leewheel 2026-08-18: 选择最优标记目标
// mode=0: 骷髅 —— 优先采用坦克的"当前攻击目标"（进入战斗的第一时间坦克正在拉的那只怪），
//         保证开局瞬间即可标记、且标记稳定（不会在多只怪之间横跳）；
//         仅在无当前目标时退回从 attackers 选择。
// mode=1: 叉叉 —— 骷髅之外，从 attackers 选择第二大生命值的未标记目标作为第二仇恨目标。
// By leewheel 2026-09-22 优化（参考 mod-playerbots-dungeon-lead）：
//   1) 引入 GetThreatRank 分级 —— 先比威胁等级（首领 > 精英 > 普通），同级再比最大生命值；
//      旧的"只看最大生命值"会把骷髅标到吃满 buff 的小怪身上。
//   2) 骷髅的 current target 优先权加两道门槛（精英以上 + 已进战斗），
//      避免坦克还在打上一波残余杂鱼、或选中了远处未开怪的精英时被误标导致 ADD。
//   3) 骷髅 / 叉叉 / 月亮三个图标互斥，绝不同时指向同一只怪。
static Unit* SelectMarkTarget(PlayerbotAI* botAI, Group* group, int mode)
{
    if (!botAI || !group)
        return nullptr;

    Player* bot = botAI->GetBot();
    if (!bot)
        return nullptr;

    ObjectGuid const skullGuid = group->GetTargetIcon(RtiTargetValue::skullIndex);
    ObjectGuid const crossGuid = group->GetTargetIcon(RtiTargetValue::crossIndex);
    ObjectGuid const moonGuid = group->GetTargetIcon(RtiTargetValue::moonIndex);

    // By leewheel 2026-09-22: 图标互斥 —— mode 自己不参与排除（它就是要争取这个图标）
    auto isTakenByOtherIcon = [&](ObjectGuid const guid) -> bool
    {
        if (guid.IsEmpty())
            return false;

        if (mode != 0 && guid == skullGuid)
            return true;
        if (mode != 1 && guid == crossGuid)
            return true;
        if (mode != 2 && guid == moonGuid)
            return true;

        return false;
    };

    auto isUsableMonster = [&](Unit* unit) -> bool
    {
        return unit && !unit->IsPlayer() && unit->IsAlive() && unit->IsInWorld() &&
               !unit->IsDuringRemoveFromWorld() && !isTakenByOtherIcon(unit->GetGUID());
    };

    if (mode == 0)
    {
        // 骷髅优先用坦克当前正在攻击的目标（实时、可靠，进战斗即命中）
        // By leewheel 2026-09-22: 追加"精英以上 + 已在战斗中"两道门槛，理由见函数头注释第 2 条
        Unit* currentTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
        if (isUsableMonster(currentTarget) && GetThreatRank(currentTarget) >= 2 && currentTarget->IsInCombat())
            return currentTarget;
    }

    // 只用 attackers 列表（已进入战斗的怪），绝不用 possible targets（会标记远处未战斗的怪导致ADD）
    GuidVector const targets = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();

    Unit* bestTarget = nullptr;
    int bestRank = -1;
    uint32 bestMaxHealth = 0;

    for (ObjectGuid const guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!isUsableMonster(unit))
            continue;

        int const rank = GetThreatRank(unit);
        uint32 const maxHealth = unit->GetMaxHealth();

        // By leewheel 2026-09-22: 先比威胁等级，同级再比最大生命值（旧的"只看血量"已废弃）
        if (rank > bestRank || (rank == bestRank && maxHealth > bestMaxHealth))
        {
            bestRank = rank;
            bestMaxHealth = maxHealth;
            bestTarget = unit;
        }
    }

    return bestTarget;
}

// By leewheel 2026-09-22: 选月亮（CC）目标。参考 mod-playerbots-dungeon-lead 的
//   DungeonLeadMarkAction::FindCcCandidate，按我方架构做两处适配：
//   1) 先扫 attackers（已经打起来的怪）—— 月亮一落地，德鲁伊/术士的 RtiCcTrigger 立刻生效，
//      不会被"上游 CcTargetValue 只遍历 attackers 列表"卡在半空；
//   2) attackers 里没有合适的，再从 possible targets 里挑紧贴骷髅的精英做"提前占位"
//      （怪一进战斗，CC 职业立刻看到月亮标记并优先控制它）。
//   距离一律以【骷髅】为圆心而非以 bot 为圆心 —— 参考实现 DL-012 的教训：
//   possible targets 是按 bot 距离粗筛的，以 bot 为圆心容易选到另一包根本不相干的怪。
static Unit* SelectCcTarget(PlayerbotAI* botAI, Group* group, Unit* skull)
{
    if (!botAI || !group || !skull)
        return nullptr;

    Player* bot = botAI->GetBot();
    if (!bot)
        return nullptr;

    if (!skull->IsAlive() || !skull->IsInWorld())
        return nullptr;

    ObjectGuid const crossGuid = group->GetTargetIcon(RtiTargetValue::crossIndex);
    ObjectGuid const moonGuid = group->GetTargetIcon(RtiTargetValue::moonIndex);

    // By leewheel 2026-09-22: requireCombat=true 只收已在战斗的（第一轮），
    //   false 只收尚未参战的（第二轮"提前占位"）。两轮互不重叠。
    auto pickBest = [&](GuidVector const& targets, bool requireCombat, Unit*& out)
    {
        int bestRank = -1;
        float bestDist = AutoTankMarkCcSearchRange;

        for (ObjectGuid const guid : targets)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (!IsValidCcCandidate(unit, skull))
                continue;

            ObjectGuid const unitGuid = unit->GetGUID();
            if (unitGuid == crossGuid || unitGuid == moonGuid)
                continue;

            if (unit->IsInCombat() != requireCombat)
                continue;

            float const dist = skull->GetDistance(unit);
            if (dist > AutoTankMarkCcSearchRange)
                continue;

            int const rank = GetThreatRank(unit);
            if (rank > bestRank || (rank == bestRank && dist < bestDist))
            {
                bestRank = rank;
                bestDist = dist;
                out = unit;
            }
        }
    };

    GuidVector const attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();
    Unit* best = nullptr;
    pickBest(attackers, true, best);
    if (best)
        return best;

    GuidVector const possible = botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets")->Get();
    pickBest(possible, false, best);
    return best;
}

// By leewheel 2026-09-06 清理 C4100 警告：以下三处 Execute 的 event 参数未被引用，改为注释形式标注。
// End By leewheel 2026-09-06
bool MarkSkullTargetAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    if (bot->InBattleground())
        return false;

    // By leewheel 2026-08-18: 骷髅锁定 —— 一旦骷髅已指向一只存活的怪，保持标记不变，不因坦克换目标而改变
    //   唯一允许改变骷髅的场景由逃跑处理（FleeingTarget 用 ACTION_RAID 最高优先级覆盖骷髅），此处不干预
    ObjectGuid skullGuid = group->GetTargetIcon(RtiTargetValue::skullIndex);
    if (!skullGuid.IsEmpty())
    {
        Unit* skulled = botAI->GetUnit(skullGuid);
        // 目标仍存活：保持现有骷髅，不再改动
        if (skulled && skulled->IsAlive() && skulled->IsInWorld() && !skulled->IsPlayer())
            return true;
        // 目标已死亡/消失：清空骷髅，以便重新标记下一目标
        group->SetTargetIcon(RtiTargetValue::skullIndex, bot->GetGUID(), ObjectGuid::Empty);
    }

    // 骷髅标志为空：选坦克当前第一目标（无当前目标时回退 attackers 列表）
    Unit* target = SelectMarkTarget(botAI, group, 0);
    if (!target)
        return false;

    // By leewheel 2026-08-18: 去重 —— 骷髅图标已指向该目标时不重复 Set（避免反复广播图标变化导致聊天刷屏）
    if (group->GetTargetIcon(RtiTargetValue::skullIndex) != target->GetGUID())
        group->SetTargetIcon(RtiTargetValue::skullIndex, bot->GetGUID(), target->GetGUID());

    return true;
}

bool MarkCrossTargetAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    if (bot->InBattleground())
        return false;

    // By leewheel 2026-08-18: 叉叉锁定 —— 叉叉一旦指向存活的怪也保持不变，不随仇恨/换目标而反复变化
    ObjectGuid crossGuid = group->GetTargetIcon(RtiTargetValue::crossIndex);
    if (!crossGuid.IsEmpty())
    {
        Unit* crossed = botAI->GetUnit(crossGuid);
        if (crossed && crossed->IsAlive() && crossed->IsInWorld() && !crossed->IsPlayer())
            return true;
        group->SetTargetIcon(RtiTargetValue::crossIndex, bot->GetGUID(), ObjectGuid::Empty);
    }

    Unit* target = SelectMarkTarget(botAI, group, 1);
    if (!target)
        return false;

    // By leewheel 2026-08-18: 去重 —— 叉叉图标已指向该目标时不重复 Set（避免反复广播图标变化导致聊天刷屏）
    if (group->GetTargetIcon(RtiTargetValue::crossIndex) != target->GetGUID())
        group->SetTargetIcon(RtiTargetValue::crossIndex, bot->GetGUID(), target->GetGUID());

    return true;
}

bool FallbackMarkSkullAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    if (bot->InBattleground())
        return false;

    ObjectGuid const mainTankGuid = PlayerbotAI::GetMainTankGuid(group);
    if (mainTankGuid.IsEmpty())
        return false;

    Unit* mainTankUnit = botAI->GetUnit(mainTankGuid);
    Player* mainTank = mainTankUnit ? mainTankUnit->ToPlayer() : nullptr;
    if (!mainTank || GET_PLAYERBOT_AI(mainTank))
        return false;

    // 骷髅锁定规则与 MarkSkullTargetAction 一致: 指向存活怪时不变, 指向死亡/消失目标时清除重标
    ObjectGuid const skullGuid = group->GetTargetIcon(RtiTargetValue::skullIndex);
    if (!skullGuid.IsEmpty())
    {
        Unit* skulled = botAI->GetUnit(skullGuid);
        if (skulled && skulled->IsAlive() && skulled->IsInWorld() && !skulled->IsPlayer())
            return true;

        group->SetTargetIcon(RtiTargetValue::skullIndex, bot->GetGUID(), ObjectGuid::Empty);
    }

    // 优先标主坦克当前仇恨目标(正在拉的怪), 其次标主坦克选中的目标
    Unit* target = mainTank->GetVictim();
    if (!target || !target->IsCreature() || !target->IsAlive() || !target->IsInWorld())
    {
        target = botAI->GetUnit(mainTank->GetTarget());
        if (!target || !target->IsCreature() || !target->IsAlive() || !target->IsInWorld())
            return false;
    }

    // By leewheel 2026-08-31: 去重 —— 多个 Bot 可能同时触发, 图标已指向该目标时不重复 Set
    if (group->GetTargetIcon(RtiTargetValue::skullIndex) != target->GetGUID())
        group->SetTargetIcon(RtiTargetValue::skullIndex, bot->GetGUID(), target->GetGUID());

    return true;
}

// By leewheel 2026-09-22: 主坦克标记月亮（CC 目标）
// 参考 mod-playerbots-dungeon-lead 的 DungeonLeadMarkAction::Execute 的 moon 分支：
// 骷髅存在时才尝试标月亮（月亮是"给首领身边这只精英上控制"的从属语义），
// 月亮槽位可用时挑一只可被控制的精英打上，并记录生命周期起点。
bool MarkMoonTargetAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    if (bot->InBattleground())
        return false;

    // By leewheel 2026-09-22: 队伍里没有任何能释放控制的职业（德鲁伊/术士）时不标月亮 ——
    //   DPS 会主动避开月亮目标，标了没人控只会让这只怪"没人打"。
    if (!GroupHasCrowdControl(group))
        return false;

    // 月亮锁定规则与骷髅/叉叉一致：指向存活怪时保持不变，指向死亡/消失目标时清除重标
    ObjectGuid const moonGuid = group->GetTargetIcon(RtiTargetValue::moonIndex);
    if (!moonGuid.IsEmpty())
    {
        Unit* mooned = botAI->GetUnit(moonGuid);
        if (mooned && mooned->IsAlive() && mooned->IsInWorld() && !mooned->IsPlayer())
            return true;

        // 本地计时状态跟着图标一起清掉，避免残留状态让下一次标记立刻被判超时
        ReleaseCcMark(botAI, group, moonGuid, bot);
    }

    // 骷髅是月亮的锚点：没有骷髅（还没开怪 / 骷髅已清）就不标月亮
    Unit* skull = nullptr;
    ObjectGuid const skullGuid = group->GetTargetIcon(RtiTargetValue::skullIndex);
    if (!skullGuid.IsEmpty())
        skull = botAI->GetUnit(skullGuid);

    Unit* ccTarget = SelectCcTarget(botAI, group, skull);
    if (!ccTarget)
        return false;

    // By leewheel 2026-08-18: 去重 —— 图标已指向该目标时不重复 Set（避免反复广播图标变化导致聊天刷屏）
    if (group->GetTargetIcon(RtiTargetValue::moonIndex) != ccTarget->GetGUID())
        group->SetTargetIcon(RtiTargetValue::moonIndex, bot->GetGUID(), ccTarget->GetGUID());

    // By leewheel 2026-09-22: 记录月亮标记生命周期起点（供 CheckCcMarkAction 判断何时释放）
    botAI->autoTankMarkCcGuid = ccTarget->GetGUID();
    botAI->autoTankMarkCcMarkedTs = getMSTime();
    botAI->autoTankMarkCcAbsoluteTs = botAI->autoTankMarkCcMarkedTs;
    botAI->autoTankMarkCcLandedTold = false;

    return true;
}

// By leewheel 2026-09-22: 月亮（CC）标记生命周期维护
// 完全对照 mod-playerbots-dungeon-lead 的 DungeonLead::CheckCcMark 的四条判定，
// 顺序也保持一致（落地 → 绝对上限 → 未进战斗不计时 → 宽限窗）：
//   1) 目标已死亡/消失 → 释放标记（清图标 + 清状态）；
//   2) 目标正被可被伤害打断的控制命中 → 重置宽限窗（给"被打断后再控一次"留机会）；
//   3) 超过绝对上限 → 无条件释放（防永久占位）；
//   4) 目标尚未进战斗 → 重置宽限窗并返回（不计时，理由见下方注释）；
//   5) 超过宽限窗仍未被控制 → 释放标记，让 DPS 重新接管这只怪。
bool CheckCcMarkAction::Execute(Event /*event*/)
{
    // By leewheel 2026-09-22: 本动作挂在通用的 "often"（RandomTrigger 5%）上，触发器不带任何
    //   业务条件（参考 mod-playerbots-dungeon-lead 同样用 "often" 限流），所以这里必须自己
    //   把"谁该维护、维护哪只怪"判全 —— 与 MainTankMarkMoonTrigger 的条件保持对称。
    if (!sPlayerbotAIConfig.autoTankMarkEnabled)
        return false;

    if (bot->InBattleground() || bot->InArena())
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // 月亮只由主坦克维护：多坦克时若各自维护，会互相清掉对方的计时状态
    if (!botAI->IsMainTank(bot))
        return false;

    ObjectGuid const moonGuid = group->GetTargetIcon(RtiTargetValue::moonIndex);

    // 月亮图标已空（被玩家清掉 / 自己超时清掉）：同步清掉本地状态
    if (moonGuid.IsEmpty())
    {
        botAI->autoTankMarkCcGuid.Clear();
        botAI->autoTankMarkCcMarkedTs = 0;
        botAI->autoTankMarkCcAbsoluteTs = 0;
        botAI->autoTankMarkCcLandedTold = false;
        return false;
    }

    Unit* mooned = botAI->GetUnit(moonGuid);

    // 1) 目标已死亡/消失：释放标记
    if (!mooned || !mooned->IsAlive() || !mooned->IsInWorld() || mooned->IsPlayer())
    {
        ReleaseCcMark(botAI, group, moonGuid, bot);
        return true;
    }

    uint32 const now = getMSTime();

    // By leewheel 2026-09-22: 本地状态与图标不一致（玩家手动标了月亮、或主坦克换人）→
    //   以"此刻"为计时起点接管，避免用别人的残留时间直接把标记判超时。
    if (botAI->autoTankMarkCcGuid != moonGuid || botAI->autoTankMarkCcAbsoluteTs == 0)
    {
        botAI->autoTankMarkCcGuid = moonGuid;
        botAI->autoTankMarkCcMarkedTs = now;
        botAI->autoTankMarkCcAbsoluteTs = now;
        botAI->autoTankMarkCcLandedTold = false;
        return true;
    }

    // 2) 控制已经落地：重置宽限窗，给下一次打断/重控留机会
    //    （只记一次日志，避免每 tick 刷屏）
    if (mooned->HasBreakableByDamageCrowdControlAura())
    {
        botAI->autoTankMarkCcMarkedTs = now;
        botAI->autoTankMarkCcLandedTold = true;
        return true;
    }

    // 3) 绝对上限：无论进不进战斗、宽限窗怎么重置，到点一律释放
    if (GetMSTimeDiffToNow(botAI->autoTankMarkCcAbsoluteTs) >= AutoTankMarkCcAbsoluteTimeoutMs)
    {
        ReleaseCcMark(botAI, group, moonGuid, bot);
        return true;
    }

    // 4) 被标怪还没进战斗：不计时。
    //    上游 CcTargetValue::Calculate -> TargetValue::FindTarget 只遍历 attackers 列表，
    //    一只还没参战的怪对 CC 类职业而言是"看不见"的；提前标记的价值在于它一进战斗就能被
    //    立刻接管，若这段时间也计入超时，标记会在真正生效之前就被清掉
    //    （参考实现实测到 49 次误释放，大多数在标记后 10~13 秒内）。
    if (!mooned->IsInCombat())
    {
        botAI->autoTankMarkCcMarkedTs = now;
        return true;
    }

    // 5) 宽限窗内：给 CC 职业发挥的时间
    if (GetMSTimeDiffToNow(botAI->autoTankMarkCcMarkedTs) < AutoTankMarkCcGraceMs)
        return true;

    // 宽限窗耗尽仍未被控制（队里没有对应 CC、法术不适用、在冷却……）→ 释放标记让 DPS 接管
    ReleaseCcMark(botAI, group, moonGuid, bot);
    return true;
}
