//By leewheel 2026-07-08
/*
 * 太阳之井高地 (Sunwell Plateau) 动作实现
 * 作者: leewheel
 * 对照 Acore 源码实现各BOSS的AI动作逻辑
 */
//End By leewheel

#include "SWPActions.h"

#include <vector>

#include "CreatureAI.h"
#include "Playerbots.h"
#include "SWPHelpers.h"
#include "RaidBossHelpers.h"

using namespace SunwellPlateauHelpers;

// ===== 通用 =====

bool SunwellEraseTimersAndTrackersAction::Execute(Event /*event*/)
{
    const ObjectGuid guid = bot->GetGUID();
    const uint32 instanceId = bot->GetMap()->GetInstanceId();

    bool erased = false;

    // 卡雷苟斯相关清理
    if (!AI_VALUE2(Unit*, "find target", "kalecgos"))
    {
        if (kalecgosPhaseTimer.erase(instanceId) > 0)
            erased = true;
        if (kalecgosHasEnteredSpectral.erase(guid) > 0)
            erased = true;
    }

    // 布鲁塔卢斯相关清理
    if (!AI_VALUE2(Unit*, "find target", "brutallus"))
    {
        if (brutallusBurnTimer.erase(guid) > 0)
            erased = true;
    }

    // 菲米丝相关清理
    if (!AI_VALUE2(Unit*, "find target", "felmyst"))
    {
        if (felmystPhaseTimer.erase(instanceId) > 0)
            erased = true;
    }

    // 艾瑞达双子相关清理
    if (!AI_VALUE2(Unit*, "find target", "lady sacrolash") &&
        !AI_VALUE2(Unit*, "find target", "grand warlock alythess"))
    {
        if (twinsKillOrder.erase(instanceId) > 0)
            erased = true;
    }

    // 穆鲁相关清理
    if (!AI_VALUE2(Unit*, "find target", "muru") &&
        !AI_VALUE2(Unit*, "find target", "entropius"))
    {
        if (muruPhaseTimer.erase(instanceId) > 0)
            erased = true;
    }

    // 基尔加丹相关清理
    if (!AI_VALUE2(Unit*, "find target", "kil'jaeden"))
    {
        if (kiljaedenLastPhase.erase(instanceId) > 0)
            erased = true;
        if (kiljaedenPhaseTimer.erase(instanceId) > 0)
            erased = true;
    }

    return erased;
}

// ===== 卡雷苟斯 (Kalecgos) =====
// 卡雷苟斯战斗机制：
// - 现实位面(Z>50)：与卡雷苟斯龙形态战斗
// - 幽灵领域(Z<50)：与萨斯罗瓦尔战斗，帮助卡雷苟斯人形态
// - BOSS施放幽灵冲击(44869)随机传送玩家到幽灵领域
// - 玩家也可通过点击幽灵裂缝(GO 187355)主动进入
// - 退出后获得幽灵力竭(44867)，60秒内不能再进入

bool KalecgosMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    if (!kalecgos)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", kalecgos))
    {
        return botAI->CastSpell("steady shot", kalecgos);
    }

    return false;
}

bool KalecgosTanksPositionBossAction::Execute(Event /*event*/)
{
    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    if (!kalecgos)
        return false;

    if (AI_VALUE(Unit*, "current target") != kalecgos)
        return Attack(kalecgos);

    // 坦克将卡雷苟斯拉到指定位置
    if (kalecgos->GetVictim() == bot && bot->IsWithinMeleeRange(kalecgos))
    {
        const Position& position = KALECGOS_TANK_POSITION;
        const float distToPosition = bot->GetExactDist2d(position.GetPositionX(),
                                                         position.GetPositionY());
        if (distToPosition > 3.0f)
        {
            const float dX = position.GetPositionX() - bot->GetPositionX();
            const float dY = position.GetPositionY() - bot->GetPositionY();
            const float moveDist = std::min(5.0f, distToPosition);
            const float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            const float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(SUNWELL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

bool KalecgosRangedDisperseAction::Execute(Event /*event*/)
{
    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    if (!kalecgos)
        return false;

    // 远程DPS分散站位，避免奥术冲击叠加过多
    const float distFromBoss = 25.0f;
    const float angle = bot->GetAngle(kalecgos) + static_cast<float>(rand()) / RAND_MAX * 0.4f - 0.2f;
    const float moveX = kalecgos->GetPositionX() + distFromBoss * std::cos(angle);
    const float moveY = kalecgos->GetPositionY() + distFromBoss * std::sin(angle);

    return MoveTo(SUNWELL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                  false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
}

bool KalecgosEnterSpectralRealmAction::Execute(Event /*event*/)
{
    // 如果已经在幽灵领域中，不需要再次进入
    if (IsInSpectralRealm(bot))
        return false;

    // 如果有幽灵力竭debuff，不能进入
    if (bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_SPECTRAL_EXHAUSTION)))
        return false;

    // 查找附近的幽灵裂缝游戏对象
    GameObject* portal = bot->FindNearestGameObject(static_cast<uint32>(SunwellObjects::GO_SPECTRAL_RIFT), 50.0f);
    if (portal)
    {
        const float dist = bot->GetExactDist2d(portal);
        if (dist > 3.0f)
        {
            // 移动到传送门位置
            return MoveTo(SUNWELL_MAP_ID, portal->GetPositionX(), portal->GetPositionY(),
                          portal->GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, true);
        }

        // 到达传送门附近，记录已进入状态
        kalecgosHasEnteredSpectral[bot->GetGUID()] = true;
        // 传送门的使用由服务端脚本处理，玩家靠近即可触发
        return true;
    }

    // 如果找不到GO，尝试直接移动到传送门坐标位置
    const Position& portalPos = KALECGOS_PORTAL_POSITION;
    const float dist = bot->GetExactDist2d(portalPos.GetPositionX(), portalPos.GetPositionY());
    if (dist > 3.0f)
    {
        return MoveTo(SUNWELL_MAP_ID, portalPos.GetPositionX(), portalPos.GetPositionY(),
                      portalPos.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, true);
    }

    return false;
}

bool KalecgosAttackSathrovarrAction::Execute(Event /*event*/)
{
    // 在幽灵领域中，攻击萨斯罗瓦尔
    Unit* sathrovarr = AI_VALUE2(Unit*, "find target", "sathrovarr the corruptor");
    if (!sathrovarr || !sathrovarr->IsAlive())
        return false;

    return Attack(sathrovarr);
}

// ===== 卡雷苟斯新增动作 =====
// 内外场血量同步控制：
// Acore源码机制：双方10%时互触CRAZED_RAGE(44807)狂暴，1%时放逐
// 战术要求：保持内外场血量差<10%，避免一方先死亡导致战斗无法结束

bool KalecgosHealthSyncAction::Execute(Event /*event*/)
{
    // 获取血量差异
    float diff = GetKalecgosHealthDifference(botAI, bot);
    if (diff == FLT_MAX)
        return false;

    bool inSpectral = IsInSpectralRealm(bot);

    // 外场(卡雷荷斯)血量更高(diff > 0)：
    //   - 外场玩家应继续输出外场BOSS（不需要减速）
    //   - 内场玩家应停手等待外场追上
    // 内场(萨斯罗瓦尔)血量更高(diff < 0)：
    //   - 内场玩家应继续输出内场BOSS
    //   - 外场玩家应停手等待内场追上

    if (inSpectral)
    {
        // 在内场：如果内场BOSS血量更低（diff > 10），内场被压太快，停手
        if (diff > 10.0f)
            return true;  // 消费动作但不攻击，控制内场DPS节奏
    }
    else
    {
        // 在外场：如果外场BOSS血量更低（diff < -10），外场被压太快，停手
        if (diff < -10.0f)
            return true;  // 消费动作但不攻击，控制外场DPS节奏
    }

    return false;  // 正常输出
}

bool KalecgosManageArcaneBuffetAction::Execute(Event /*event*/)
{
    // 奥术冲击层数过高时进入幽灵领域刷新
    // 进入幽灵领域后debuff会被清除
    if (IsInSpectralRealm(bot))
        return false;

    if (bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_SPECTRAL_EXHAUSTION)))
        return false;

    // 查找附近的幽灵裂缝游戏对象（GO_SPECTRAL_RIFT = 187355）
    GameObject* portal = bot->FindNearestGameObject(static_cast<uint32>(SunwellObjects::GO_SPECTRAL_RIFT), 50.0f);
    if (portal)
    {
        const float dist = bot->GetExactDist2d(portal);
        if (dist > 3.0f)
        {
            return MoveTo(SUNWELL_MAP_ID, portal->GetPositionX(), portal->GetPositionY(),
                          portal->GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_FORCED, true, true);
        }
        return true;
    }

    // 如果找不到GO，尝试移动到传送门坐标位置
    const Position& portalPos = KALECGOS_PORTAL_POSITION;
    const float dist = bot->GetExactDist2d(portalPos.GetPositionX(), portalPos.GetPositionY());
    if (dist > 3.0f)
    {
        return MoveTo(SUNWELL_MAP_ID, portalPos.GetPositionX(), portalPos.GetPositionY(),
                      portalPos.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, true);
    }

    return false;
}

bool KalecgosDispellingCurseAction::Execute(Event /*event*/)
{
    // 无尽痛苦诅咒(45032)驱散后会转移给邻近玩家(45034)
    // 需要能驱散诅咒的职业处理：法师/德鲁伊(remove curse)、萨满(cleanse spirit)

    // 查找附近中了诅咒的队友
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* cursedPlayer = nullptr;
    float minDist = 40.0f;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
            continue;

        if (member->HasAura(static_cast<uint32>(SunwellSpells::SPELL_CURSE_OF_BOUNDLESS_AGONY)) ||
            member->HasAura(static_cast<uint32>(SunwellSpells::SPELL_CURSE_OF_BOUNDLESS_AGONY_PLR)))
        {
            float dist = bot->GetExactDist2d(member);
            if (dist < minDist)
            {
                minDist = dist;
                cursedPlayer = member;
            }
        }
    }

    if (!cursedPlayer)
        return false;

    // 根据职业施放驱散法术
    switch (bot->getClass())
    {
        case CLASS_MAGE:
        case CLASS_DRUID:
            return botAI->CastSpell("remove curse", cursedPlayer);
        case CLASS_SHAMAN:
            return botAI->CastSpell("cleanse spirit", cursedPlayer);
        default:
            return false;
    }
}

bool KalecgosDispellingFrostBreathAction::Execute(Event /*event*/)
{
    // 冰霜吐息(44799)：降攻速75%，坦克中后需立即驱散防倒T
    // 可驱散魔法：牧师(dispel magic)、圣骑士(cleanse)

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank || !mainTank->IsAlive())
        return false;

    if (!mainTank->HasAura(static_cast<uint32>(SunwellSpells::SPELL_FROST_BREATH)))
        return false;

    // 根据职业施放驱散法术
    switch (bot->getClass())
    {
        case CLASS_PRIEST:
            return botAI->CastSpell("dispel magic", mainTank);
        case CLASS_PALADIN:
            return botAI->CastSpell("cleanse", mainTank);
        default:
            return false;
    }
}

// ===== 布鲁塔卢斯 (Brutallus) =====
// 布鲁塔卢斯战斗机制：
// - 流星猛击(45150)：锥形分摊伤害，需要多人分摊
// - 燃烧(45141→46394)：随机点名DoT，需要跑开
// - 踩踏(45185)：击倒+伤害，打断施法
// - 狂暴(26662)：6分钟后狂暴

bool BrutallusMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    if (!brutallus)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", brutallus))
    {
        return botAI->CastSpell("steady shot", brutallus);
    }

    return false;
}

bool BrutallusTanksPositionBossAction::Execute(Event /*event*/)
{
    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    if (!brutallus)
        return false;

    if (AI_VALUE(Unit*, "current target") != brutallus)
        return Attack(brutallus);

    // 主坦将布鲁塔卢斯拉到指定位置
    if (brutallus->GetVictim() == bot && bot->IsWithinMeleeRange(brutallus))
    {
        const Position& position = BRUTALLUS_TANK_POSITION;
        const float distToPosition = bot->GetExactDist2d(position.GetPositionX(),
                                                         position.GetPositionY());
        if (distToPosition > 3.0f)
        {
            const float dX = position.GetPositionX() - bot->GetPositionX();
            const float dY = position.GetPositionY() - bot->GetPositionY();
            const float moveDist = std::min(5.0f, distToPosition);
            const float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            const float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(SUNWELL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

bool BrutallusSoakMeteorSlashAction::Execute(Event /*event*/)
{
    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    if (!brutallus)
        return false;

    // 流星猛击是锥形伤害，需要站在BOSS前方分摊
    // 非坦克的近战DPS需要移动到BOSS前方分摊区域
    if (botAI->IsTank(bot) && brutallus->GetVictim() == bot)
        return false;  // 主坦不需要移动，保持位置

    // 移动到BOSS前方分摊位置（主坦前方）
    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    // 站在主坦附近分摊流星猛击
    const float distToMainTank = bot->GetExactDist2d(mainTank);
    if (distToMainTank > 5.0f)
    {
        const float dX = mainTank->GetPositionX() - bot->GetPositionX();
        const float dY = mainTank->GetPositionY() - bot->GetPositionY();
        const float moveDist = std::min(5.0f, distToMainTank);
        const float moveX = bot->GetPositionX() + (dX / distToMainTank) * moveDist;
        const float moveY = bot->GetPositionY() + (dY / distToMainTank) * moveDist;

        return MoveTo(SUNWELL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                      false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
    }

    return false;
}

bool BrutallusBurnMoveAwayAction::Execute(Event /*event*/)
{
    // 被燃烧点名后，跑到远离人群的位置
    // 燃烧DoT会跳跃给附近玩家，所以需要隔离
    constexpr float safeDistFromPlayer = 15.0f;

    // 查找最近的玩家
    Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer);
    if (!nearestPlayer)
        return false;  // 已经在安全距离

    // 朝远离最近玩家的方向移动
    const float angle = bot->GetAngle(nearestPlayer) + M_PI;
    const float moveDist = 5.0f;
    const float moveX = bot->GetPositionX() + moveDist * std::cos(angle);
    const float moveY = bot->GetPositionY() + moveDist * std::sin(angle);

    return MoveTo(SUNWELL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                  false, false, MovementPriority::MOVEMENT_FORCED, true, true);
}

// ===== 菲米丝 (Felmyst) =====
// 菲米丝战斗机制：
// - 地面阶段（约1分钟）：腐蚀(45866)、毒气新星(45855)、包裹(45661)、顺劈
// - 飞行阶段：恶魔蒸汽、深呼吸（三条通道选一）
// - 狂暴(45078)：10分钟后狂暴

bool FelmystMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", felmyst))
    {
        return botAI->CastSpell("steady shot", felmyst);
    }

    return false;
}

bool FelmystTanksPositionBossAction::Execute(Event /*event*/)
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    if (AI_VALUE(Unit*, "current target") != felmyst)
        return Attack(felmyst);

    // 坦克拉到指定位置
    if (felmyst->GetVictim() == bot && bot->IsWithinMeleeRange(felmyst))
    {
        const Position& position = FELMYST_TANK_POSITION;
        const float distToPosition = bot->GetExactDist2d(position.GetPositionX(),
                                                         position.GetPositionY());
        if (distToPosition > 3.0f)
        {
            const float dX = position.GetPositionX() - bot->GetPositionX();
            const float dY = position.GetPositionY() - bot->GetPositionY();
            const float moveDist = std::min(5.0f, distToPosition);
            const float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            const float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(SUNWELL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

bool FelmystDisperseFromGasNovaAction::Execute(Event /*event*/)
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    // 毒气新星是近战范围AoE，近战需要跑开
    if (bot->GetExactDist2d(felmyst) < 15.0f)
    {
        const float angle = bot->GetAngle(felmyst) + M_PI;
        const float moveDist = 5.0f;
        const float moveX = bot->GetPositionX() + moveDist * std::cos(angle);
        const float moveY = bot->GetPositionY() + moveDist * std::sin(angle);

        return MoveTo(SUNWELL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                      false, false, MovementPriority::MOVEMENT_FORCED, true, true);
    }

    return false;
}

bool FelmystAvoidEncapsulateAction::Execute(Event /*event*/)
{
    // 包裹是地面AoE通道，需要远离目标区域
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    // 检查是否在包裹范围内
    if (HasEncapsulateNearby(botAI, bot))
    {
        // 远离菲米丝的当前目标
        Unit* victim = felmyst->GetVictim();
        if (victim)
        {
            const float angle = bot->GetAngle(victim) + M_PI;
            const float moveDist = 10.0f;
            const float moveX = bot->GetPositionX() + moveDist * std::cos(angle);
            const float moveY = bot->GetPositionY() + moveDist * std::sin(angle);

            return MoveTo(SUNWELL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_FORCED, true, true);
        }
    }

    return false;
}

bool FelmystFlightPhaseSpreadAction::Execute(Event /*event*/)
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    // 飞行阶段时分散站位，避免深呼吸命中多人
    constexpr float safeDistFromPlayer = 8.0f;
    Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer);
    if (!nearestPlayer)
        return false;

    // 朝远离最近玩家的方向移动
    const float angle = bot->GetAngle(nearestPlayer) + M_PI;
    const float moveDist = 5.0f;
    const float moveX = bot->GetPositionX() + moveDist * std::cos(angle);
    const float moveY = bot->GetPositionY() + moveDist * std::sin(angle);

    return MoveTo(SUNWELL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                  false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
}

bool FelmystManagePhaseTimerAction::Execute(Event /*event*/)
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    const uint32 instanceId = bot->GetMap()->GetInstanceId();
    const time_t now = std::time(nullptr);

    // 记录阶段计时器
    int phase = GetFelmystPhase(felmyst);
    auto it = felmystPhaseTimer.find(instanceId);
    if (it == felmystPhaseTimer.end() || it->second == 0)
    {
        felmystPhaseTimer[instanceId] = now;
        return true;
    }

    // 地面阶段约60秒后进入飞行阶段
    if (phase == 0)
    {
        // 地面阶段，更新计时器
        felmystPhaseTimer[instanceId] = now;
    }
    else
    {
        // 飞行阶段，清空计时器等待下次地面阶段
        felmystPhaseTimer[instanceId] = 0;
    }

    return false;
}

// ===== 艾瑞达双子 (Eredar Twins) =====
// 艾瑞达双子战斗机制：
// - 萨洛拉尔（暗影）：暗影之刃、混淆打击、暗影新星、暗影影像ADD
// - 艾莉赛斯（火焰）：烈焰、烈焰灼烧、炽热、爆燃
// - 暗影触碰/火焰触碰：需要去对应源头消除
// - 击杀一个后另一个获得充能(45366)，技能增强
// - 狂暴(46587)：6分钟

bool EredarTwinsMisdirectBossToTanksAction::Execute(Event /*event*/)
{
    // 猎人误导萨洛拉尔到主坦
    Unit* sacrolash = AI_VALUE2(Unit*, "find target", "lady sacrolash");
    if (!sacrolash)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", sacrolash))
    {
        return botAI->CastSpell("steady shot", sacrolash);
    }

    return false;
}

bool EredarTwinsAssignKillOrderAction::Execute(Event /*event*/)
{
    // 标准击杀顺序：先杀萨洛拉尔（暗影），后杀艾莉赛斯（火焰）
    const uint32 instanceId = bot->GetMap()->GetInstanceId();

    Unit* sacrolash = AI_VALUE2(Unit*, "find target", "lady sacrolash");
    Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");

    if (!sacrolash && !alythess)
        return false;

    // 设置击杀顺序：0=先杀萨洛拉尔，1=先杀艾莉赛斯
    auto it = twinsKillOrder.find(instanceId);
    if (it == twinsKillOrder.end())
    {
        twinsKillOrder[instanceId] = 0;  // 默认先杀萨洛拉尔
    }

    // 根据击杀顺序设置攻击目标
    int order = twinsKillOrder[instanceId];
    Unit* target = nullptr;

    if (order == 0)
    {
        // 先杀萨洛拉尔
        if (sacrolash && sacrolash->IsAlive())
            target = sacrolash;
        else if (alythess && alythess->IsAlive())
            target = alythess;
    }
    else
    {
        // 先杀艾莉赛斯
        if (alythess && alythess->IsAlive())
            target = alythess;
        else if (sacrolash && sacrolash->IsAlive())
            target = sacrolash;
    }

    if (target && AI_VALUE(Unit*, "current target") != target)
        return Attack(target);

    return false;
}

bool EredarTwinsMoveToFlameSourceAction::Execute(Event /*event*/)
{
    // 有暗影触碰debuff时，需要去火焰源头（艾莉赛斯）附近消除
    Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
    if (!alythess)
        return false;

    // 移动到艾莉赛斯附近
    const float distToAlythess = bot->GetExactDist2d(alythess);
    if (distToAlythess > 20.0f)
    {
        const float dX = alythess->GetPositionX() - bot->GetPositionX();
        const float dY = alythess->GetPositionY() - bot->GetPositionY();
        const float moveDist = std::min(5.0f, distToAlythess - 15.0f);
        const float moveX = bot->GetPositionX() + (dX / distToAlythess) * moveDist;
        const float moveY = bot->GetPositionY() + (dY / distToAlythess) * moveDist;

        return MoveTo(SUNWELL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                      false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
    }

    return false;
}

bool EredarTwinsMoveToShadowSourceAction::Execute(Event /*event*/)
{
    // 有火焰触碰debuff时，需要去暗影源头（萨洛拉尔）附近消除
    Unit* sacrolash = AI_VALUE2(Unit*, "find target", "lady sacrolash");
    if (!sacrolash)
        return false;

    // 移动到萨洛拉尔附近
    const float distToSacrolash = bot->GetExactDist2d(sacrolash);
    if (distToSacrolash > 20.0f)
    {
        const float dX = sacrolash->GetPositionX() - bot->GetPositionX();
        const float dY = sacrolash->GetPositionY() - bot->GetPositionY();
        const float moveDist = std::min(5.0f, distToSacrolash - 15.0f);
        const float moveX = bot->GetPositionX() + (dX / distToSacrolash) * moveDist;
        const float moveY = bot->GetPositionY() + (dY / distToSacrolash) * moveDist;

        return MoveTo(SUNWELL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                      false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
    }

    return false;
}

bool EredarTwinsAvoidConflagrationAction::Execute(Event /*event*/)
{
    // 爆燃点名时跑开
    // 爆燃会使目标昏迷并受到持续火焰伤害
    // 被点名的玩家需要跑到远离人群的位置
    constexpr float safeDistFromPlayer = 12.0f;
    Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer);
    if (!nearestPlayer)
        return false;

    const float angle = bot->GetAngle(nearestPlayer) + M_PI;
    const float moveDist = 5.0f;
    const float moveX = bot->GetPositionX() + moveDist * std::cos(angle);
    const float moveY = bot->GetPositionY() + moveDist * std::sin(angle);

    return MoveTo(SUNWELL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                  false, false, MovementPriority::MOVEMENT_FORCED, true, true);
}

// ===== 穆鲁 (Muru) =====
// 穆鲁战斗机制：
// 阶段1（穆鲁）：
// - 负能量(46009)：周期性随机目标伤害
// - 召唤血精灵(46041)：狂暴者+怒火法师
// - 开门(45994)：召唤虚空哨兵
// - 黑暗(45998)：召唤暗魔
// 阶段2（恩特罗皮乌斯）：
// - 负能量(46284)：递增伤害
// - 黑暗(46269)：随机目标AoE
// - 黑洞(46282)：吸引玩家

bool MuruMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    // 穆鲁阶段不需要误导（穆鲁是固定的）
    // 恩特罗皮乌斯阶段需要误导
    Unit* entropius = AI_VALUE2(Unit*, "find target", "entropius");
    if (!entropius)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", entropius))
    {
        return botAI->CastSpell("steady shot", entropius);
    }

    return false;
}

bool MuruHandleAddsAction::Execute(Event /*event*/)
{
    // 优先处理召唤的血精灵ADD（狂暴者、怒火法师）
    Unit* berserker = AI_VALUE2(Unit*, "find target", "shadowsword berserker");
    if (berserker && berserker->IsAlive())
    {
        if (AI_VALUE(Unit*, "current target") != berserker)
            return Attack(berserker);
        return false;
    }

    Unit* furyMage = AI_VALUE2(Unit*, "find target", "shadowsword fury mage");
    if (furyMage && furyMage->IsAlive())
    {
        if (AI_VALUE(Unit*, "current target") != furyMage)
            return Attack(furyMage);
        return false;
    }

    return false;
}

bool MuruHandleVoidSentinelAction::Execute(Event /*event*/)
{
    // 优先处理虚空哨兵
    Unit* voidSentinel = GetNearestVoidSentinel(botAI, bot);
    if (voidSentinel && voidSentinel->IsAlive())
    {
        if (AI_VALUE(Unit*, "current target") != voidSentinel)
            return Attack(voidSentinel);
        return false;
    }

    return false;
}

bool MuruAvoidDarknessAction::Execute(Event /*event*/)
{
    // 穆鲁施放黑暗时，需要远离黑暗区域
    Unit* muru = AI_VALUE2(Unit*, "find target", "muru");
    if (!muru)
        return false;

    // 检查附近是否有暗魔（NPC_DARK_FIEND = 25744）
    // Acore源码: SPELL_DARKNESS_PERIODIC(45998)的第2tick召唤暗魔
    Unit* darkness = bot->FindNearestCreature(static_cast<uint32>(SunwellNpcs::NPC_DARK_FIEND), 30.0f);
    if (darkness)
    {
        const float dist = bot->GetExactDist2d(darkness);
        if (dist < 20.0f)
        {
            const float angle = bot->GetAngle(darkness) + M_PI;
            const float moveDist = 5.0f;
            const float moveX = bot->GetPositionX() + moveDist * std::cos(angle);
            const float moveY = bot->GetPositionY() + moveDist * std::sin(angle);

            return MoveTo(SUNWELL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_FORCED, true, true);
        }
    }

    return false;
}

bool MuruEntropiusPhaseAction::Execute(Event /*event*/)
{
    // 恩特罗皮乌斯阶段：坦克建立仇恨，DPS输出
    Unit* entropius = AI_VALUE2(Unit*, "find target", "entropius");
    if (!entropius || !entropius->IsAlive())
        return false;

    if (AI_VALUE(Unit*, "current target") != entropius)
        return Attack(entropius);

    // 恩特罗皮乌斯阶段需要注意黑洞 avoidance
    // 检查附近是否有奇点/黑洞（NPC_SINGULARITY = 25855）
    Unit* blackHole = bot->FindNearestCreature(static_cast<uint32>(SunwellNpcs::NPC_SINGULARITY), 20.0f);
    if (blackHole && bot->GetExactDist2d(blackHole) < 10.0f)
    {
        const float angle = bot->GetAngle(blackHole) + M_PI;
        const float moveDist = 5.0f;
        const float moveX = bot->GetPositionX() + moveDist * std::cos(angle);
        const float moveY = bot->GetPositionY() + moveDist * std::sin(angle);

        return MoveTo(SUNWELL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                      false, false, MovementPriority::MOVEMENT_FORCED, true, true);
    }

    return false;
}

// ===== 基尔加丹 (Kil'jaeden) =====
// 基尔加丹战斗机制：
// 阶段1：击杀欺骗者之手（3个ADD）
// 阶段2（85%）：基尔加丹出现，灵魂抽打、军团闪电、火焰之花
// 阶段3（85%触发）：邪恶映像、暗影之刺、火焰飞镖、千魂之暗
// 阶段4（55%）：末日审判（陨石雨）、千魂之暗
// 阶段5（25%）：安薇娜牺牲、全力输出

bool KiljaedenMisdirectToTankAction::Execute(Event /*event*/)
{
    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", kiljaeden))
    {
        return botAI->CastSpell("steady shot", kiljaeden);
    }

    return false;
}

bool KiljaedenAvoidDarknessAction::Execute(Event /*event*/)
{
    // 千魂之暗：需要躲到蓝龙护盾后面
    // 检查基尔加丹是否在施放千魂之暗
    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden)
        return false;

    if (!IsKiljaedenCastingDarkness(kiljaeden))
        return false;

    // 查找最近的蓝龙之环游戏对象（GO 187869-188116）
    // 或者查找蓝龙护盾NPC
    // 优先寻找附近的蓝龙宝珠
    // NPC_SHIELD_ORB = 25502 是基尔加丹的护盾宝珠，不是蓝龙护盾
    // 蓝龙护盾由蓝龙宝珠(GO)触发，玩家点击后获得蓝龙保护
    // 这里简化处理：移动到安全区域

    // 查找最近的有蓝龙护盾光环的玩家
    Group* group = bot->GetGroup();
    if (group)
    {
        Player* shieldedPlayer = nullptr;
        float minDist = FLT_MAX;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive())
                continue;

            // 检查是否有蓝龙保护光环（SPELL_VENGEANCE_OF_THE_BLUE_FLIGHT = 45839）
            if (member->HasAura(static_cast<uint32>(SunwellSpells::SPELL_VENGEANCE_OF_THE_BLUE_FLIGHT)))
            {
                float dist = bot->GetExactDist2d(member);
                if (dist < minDist)
                {
                    minDist = dist;
                    shieldedPlayer = member;
                }
            }
        }

        if (shieldedPlayer && minDist > 5.0f)
        {
            const float dX = shieldedPlayer->GetPositionX() - bot->GetPositionX();
            const float dY = shieldedPlayer->GetPositionY() - bot->GetPositionY();
            const float moveDist = std::min(5.0f, minDist);
            const float moveX = bot->GetPositionX() + (dX / minDist) * moveDist;
            const float moveY = bot->GetPositionY() + (dY / minDist) * moveDist;

            return MoveTo(SUNWELL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_FORCED, true, true);
        }
    }

    // 如果没有找到蓝龙护盾玩家，移动到安全位置
    const Position& safePos = KILJAEDEN_SAFE_POSITION;
    const float distToSafe = bot->GetExactDist2d(safePos.GetPositionX(), safePos.GetPositionY());
    if (distToSafe > 3.0f)
    {
        const float dX = safePos.GetPositionX() - bot->GetPositionX();
        const float dY = safePos.GetPositionY() - bot->GetPositionY();
        const float moveDist = std::min(5.0f, distToSafe);
        const float moveX = bot->GetPositionX() + (dX / distToSafe) * moveDist;
        const float moveY = bot->GetPositionY() + (dY / distToSafe) * moveDist;

        return MoveTo(SUNWELL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                      false, false, MovementPriority::MOVEMENT_FORCED, true, true);
    }

    return false;
}

bool KiljaedenAvoidArmageddonAction::Execute(Event /*event*/)
{
    // 末日审判：陨石从天而降，需要躲避
    // NPC_ARMAGEDDON_TARGET = 25735
    Unit* armageddon = bot->FindNearestCreature(static_cast<uint32>(SunwellNpcs::NPC_ARMAGEDDON_TARGET), 30.0f);
    if (armageddon)
    {
        const float dist = bot->GetExactDist2d(armageddon);
        if (dist < 15.0f)
        {
            const float angle = bot->GetAngle(armageddon) + M_PI;
            const float moveDist = 5.0f;
            const float moveX = bot->GetPositionX() + moveDist * std::cos(angle);
            const float moveY = bot->GetPositionY() + moveDist * std::sin(angle);

            return MoveTo(SUNWELL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_FORCED, true, true);
        }
    }

    return false;
}

bool KiljaedenHandleSinisterReflectionAction::Execute(Event /*event*/)
{
    // 邪恶映像：基尔加丹召唤玩家镜像，需要优先击杀
    auto reflections = GetSinisterReflections(botAI, bot);
    if (reflections.empty())
        return false;

    // 优先攻击最近的邪恶映像
    Unit* nearest = nullptr;
    float minDist = FLT_MAX;
    for (Unit* reflection : reflections)
    {
        float dist = bot->GetExactDist2d(reflection);
        if (dist < minDist)
        {
            minDist = dist;
            nearest = reflection;
        }
    }

    if (nearest && AI_VALUE(Unit*, "current target") != nearest)
        return Attack(nearest);

    return false;
}

bool KiljaedenHandleShieldOrbAction::Execute(Event /*event*/)
{
    // 护盾宝珠：在基尔加丹周围旋转，需要远程DPS优先击杀
    Unit* orb = GetShieldOrb(botAI, bot);
    if (!orb || !orb->IsAlive())
        return false;

    // 只有远程DPS才处理护盾宝珠
    if (!botAI->IsRangedDps(bot))
        return false;

    if (AI_VALUE(Unit*, "current target") != orb)
        return Attack(orb);

    return false;
}

bool KiljaedenManagePhaseAction::Execute(Event /*event*/)
{
    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden)
        return false;

    const uint32 instanceId = bot->GetMap()->GetInstanceId();
    const time_t now = std::time(nullptr);

    int currentPhase = GetKiljaedenPhase(kiljaeden);
    auto it = kiljaedenLastPhase.find(instanceId);

    if (it == kiljaedenLastPhase.end() || it->second != currentPhase)
    {
        // 阶段转换
        kiljaedenLastPhase[instanceId] = currentPhase;
        kiljaedenPhaseTimer[instanceId] = now;
        return true;
    }

    return false;
}

bool KiljaedenRangedDisperseAction::Execute(Event /*event*/)
{
    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden)
        return false;

    // 远程DPS分散站位，避免火焰飞镖命中多人
    constexpr float safeDistFromPlayer = 8.0f;
    Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer);
    if (!nearestPlayer)
        return false;

    const float angle = bot->GetAngle(nearestPlayer) + M_PI;
    const float moveDist = 5.0f;
    const float moveX = bot->GetPositionX() + moveDist * std::cos(angle);
    const float moveY = bot->GetPositionY() + moveDist * std::sin(angle);

    return MoveTo(SUNWELL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                  false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
}
