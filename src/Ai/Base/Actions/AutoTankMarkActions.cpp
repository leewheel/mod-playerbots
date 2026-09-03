/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AutoTankMarkActions.h"

#include "AttackersValue.h"
#include "Event.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"

// By leewheel 2026-08-18: 选择最优标记目标
// mode=0: 骷髅 —— 优先采用坦克的"当前攻击目标"（进入战斗的第一时间坦克正在拉的那只怪），
//         保证开局瞬间即可标记、且标记稳定（不会在多只怪之间横跳）；
//         仅在无当前目标时退回从 attackers 选择。
// mode=1: 叉叉 —— 骷髅之外，从 attackers 选择第二大生命值的未标记目标作为第二仇恨目标。
static Unit* SelectMarkTarget(PlayerbotAI* botAI, Group* group, int mode)
{
    if (!botAI || !group)
        return nullptr;

    Player* bot = botAI->GetBot();
    if (!bot)
        return nullptr;

    Unit* target = nullptr;

    if (mode == 0)
    {
        // 骷髅优先用坦克当前正在攻击的目标（实时、可靠，进战斗即命中）
        Unit* currentTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
        if (currentTarget && currentTarget->IsAlive() && !currentTarget->IsPlayer() &&
            currentTarget->IsInWorld() && currentTarget->IsValidAttackTarget(bot, nullptr))
            return currentTarget;
    }

    // 只用 attackers 列表（已进入战斗的怪），绝不用 possible targets（会标记远处未战斗的怪导致ADD）
    GuidVector targets = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();

    ObjectGuid skullGuid = group->GetTargetIcon(RtiTargetValue::skullIndex);

    Unit* bestTarget = nullptr;
    uint32 bestMaxHealth = 0;

    for (ObjectGuid const guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || unit->IsPlayer())
            continue;

        if (!unit->IsAlive() || !unit->IsInWorld())
            continue;

        // 叉叉模式：跳过骷髅标记的目标
        if (mode == 1 && unit->GetGUID() == skullGuid)
            continue;

        // 选择最大生命值的目标（通常是最危险的）
        uint32 maxHealth = unit->GetMaxHealth();
        if (maxHealth > bestMaxHealth)
        {
            bestMaxHealth = maxHealth;
            bestTarget = unit;
        }
    }

    // 如果没找到最大生命值的目标，退而选择任意目标
    if (!bestTarget)
    {
        for (ObjectGuid const guid : targets)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (!unit || unit->IsPlayer() || !unit->IsAlive() || !unit->IsInWorld())
                continue;

            if (mode == 1 && unit->GetGUID() == skullGuid)
                continue;

            bestTarget = unit;
            break;
        }
    }

    return bestTarget;
}

bool MarkSkullTargetAction::Execute(Event event)
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

bool MarkCrossTargetAction::Execute(Event event)
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

bool FallbackMarkSkullAction::Execute(Event event)
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
