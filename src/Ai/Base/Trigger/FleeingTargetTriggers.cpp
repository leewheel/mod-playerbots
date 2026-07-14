/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "FleeingTargetTriggers.h"

#include "AttackersValue.h"
#include "MotionMaster.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

// By leewheel 2026-07-15: 检测怪物是否正在逃跑或寻求支援
static bool IsUnitFleeing(Unit* unit)
{
    if (!unit || !unit->IsCreature())
        return false;

    // 检查逃跑状态
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
static bool IsUnitAboutToFlee(Unit* unit)
{
    if (!unit || !unit->IsCreature())
        return false;

    Creature* creature = unit->ToCreature();
    if (!creature)
        return false;

    // Boss不逃跑
    if (creature->isWorldBoss() || creature->IsDungeonBoss())
        return false;

    // 血量低于20%且在战斗中
    if (unit->GetHealthPct() <= 20.0f && unit->IsInCombat())
        return true;

    return false;
}

bool FleeingTargetTrigger::IsActive()
{
    if (!sPlayerbotAIConfig.autoTankMarkEnabled)
        return false;

    // 只在副本/团本中触发
    if (bot->InBattleground() || bot->InArena())
        return false;

    // 必须有队伍
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // 任何坦克都可以触发（不限于主坦克/副坦克）
    if (!PlayerbotAI::IsTank(bot))
        return false;

    // 检查攻击者列表中是否有逃跑/即将逃跑的怪物
    GuidVector attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();

    // 优先检查正在逃跑的怪
    for (ObjectGuid const guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || unit->IsPlayer() || !unit->IsAlive())
            continue;

        if (IsUnitFleeing(unit))
            return true;
    }

    // 其次检查即将逃跑的怪
    for (ObjectGuid const guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || unit->IsPlayer() || !unit->IsAlive())
            continue;

        if (IsUnitAboutToFlee(unit))
            return true;
    }

    return false;
}
