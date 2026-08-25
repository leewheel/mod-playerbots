/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ACTriggers.h"
#include "Playerbots.h"

// Shirrak the Dead Watcher

bool ShirrakTankPositionBossTrigger::IsActive()
{
    return botAI->IsTank(bot) &&
           AI_VALUE2(Unit*, "find target", "18371");
}

bool ShirrakFleeFocusFireTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "18371"))
        return false;

    return bot->FindNearestCreature(NPC_FOCUS_FIRE, FLARE_SEARCH_RADIUS, true);
}

bool ShirrakRangedKeepDistanceTrigger::IsActive()
{
    return botAI->IsRanged(bot) &&
           AI_VALUE2(Unit*, "find target", "18371");
}
