/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ACMultipliers.h"
#include "ACActions.h"
#include "ACTriggers.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "MovementActions.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"

// Shirrak the Dead Watcher

// Flee from Focus Fire and dont run back in
float ShirrakFleeFocusFireMultiplier::GetValue(Action* action)
{
    //By leewheel 2026-08-26 合并：采用对侧移动施法过滤新逻辑，查找串按本项目规则转回entry(Shirrak=18371)
    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    bool const isMovementSpell = dynamic_cast<CastReachTargetSpellAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action) || dynamic_cast<CastDisengageAction*>(action);

    if (!isMovementSpell && !dynamic_cast<MovementAction*>(action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "18371"))
        return 1.0f;

    if (dynamic_cast<ShirrakFleeFocusFireAction*>(action))
        return 1.0f;

    Creature* flare = bot->FindNearestCreature(NPC_FOCUS_FIRE, FLARE_SEARCH_RADIUS);
    if (!flare)
        return 1.0f;

    if (isMovementSpell)
        return 0.0f;

    float currentDistance = bot->GetExactDist2d(flare);
    constexpr float safeDistance = 12.0f;
    constexpr float buffer = 2.0f;
    return currentDistance < safeDistance + buffer ? 0.0f : 1.0f;
}
