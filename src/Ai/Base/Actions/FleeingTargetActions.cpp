/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "FleeingTargetActions.h"

#include "AttackersValue.h"
#include "Event.h"
#include "MotionMaster.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"

// By leewheel 2026-07-15: 检测怪物是否正在逃跑或寻求支援
static bool IsUnitFleeing(Unit* unit)
{
    if (!unit || !unit->IsCreature())
        return false;

    // 检查逃跑状态（恐惧/逃跑buff）
    if (unit->HasUnitState(UNIT_STATE_FLEEING | UNIT_STATE_FLEEING_MOVE))
        return true;

    // 检查移动类型
    MovementGeneratorType motionType = unit->GetMotionMaster()->GetCurrentMovementGeneratorType();
    if (motionType == FLEEING_MOTION_TYPE ||
        motionType == TIMED_FLEEING_MOTION_TYPE ||
        motionType == ASSISTANCE_MOTION_TYPE ||
        motionType == ASSISTANCE_DISTRACT_MOTION_TYPE)
        return true;

    return false;
}

// By leewheel 2026-07-15: 检测怪物是否低血量即将逃跑
// 很多副本怪在血量低于~20%时会尝试逃跑寻求支援
static bool IsUnitAboutToFlee(Unit* unit)
{
    if (!unit || !unit->IsCreature())
        return false;

    Creature* creature = unit->ToCreature();
    if (!creature)
        return false;

    // Boss不逃跑，跳过
    if (creature->isWorldBoss() || creature->IsDungeonBoss())
        return false;

    // 血量低于20%且在战斗中，可能即将逃跑
    if (unit->GetHealthPct() <= 20.0f && unit->IsInCombat())
        return true;

    return false;
}

// By leewheel 2026-07-15: 在攻击者列表中找到逃跑/即将逃跑的怪物
static Unit* FindFleeingTarget(PlayerbotAI* botAI)
{
    if (!botAI)
        return nullptr;

    GuidVector attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();

    // 优先找正在逃跑的怪（最紧急）
    for (ObjectGuid const guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || unit->IsPlayer() || !unit->IsAlive())
            continue;

        if (IsUnitFleeing(unit))
            return unit;
    }

    // 其次找即将逃跑的怪（低血量）
    for (ObjectGuid const guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || unit->IsPlayer() || !unit->IsAlive())
            continue;

        if (IsUnitAboutToFlee(unit))
            return unit;
    }

    return nullptr;
}

bool PrioritizeFleeingTargetAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Unit* fleeingTarget = FindFleeingTarget(botAI);
    if (!fleeingTarget)
        return false;

    // 检查骷髅标记是否已经指向这个逃跑怪
    ObjectGuid currentSkullGuid = group->GetTargetIcon(RtiTargetValue::skullIndex);
    if (currentSkullGuid == fleeingTarget->GetGUID())
        return false;  // 已经标记了，不需要重复

    // 立即标记逃跑怪为骷髅（覆盖现有骷髅）
    group->SetTargetIcon(RtiTargetValue::skullIndex, bot->GetGUID(), fleeingTarget->GetGUID());

    // 设为优先集火目标，让所有机器人切换目标
    botAI->GetAiObjectContext()->GetValue<GuidVector>("prioritized targets")->Set({fleeingTarget->GetGUID()});

    return true;
}
