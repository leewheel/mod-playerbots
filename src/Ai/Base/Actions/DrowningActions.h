/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_DROWNINGACTIONS_H
#define PLAYERBOTS_DROWNINGACTIONS_H

#include "MovementActions.h"
#include "Playerbots.h"

class PlayerbotAI;

// Swims straight up to the water surface above the bot so it can breathe.
class SurfaceForBreathAction : public MovementAction
{
public:
    SurfaceForBreathAction(PlayerbotAI* botAI) : MovementAction(botAI, "surface for breath") {}

    bool Execute(Event /*event*/) override
    {
        if (!bot->IsAlive() || !bot->IsInWater())
            return false;

        Map* map = bot->GetMap();
        if (!map)
            return false;

        float const x = bot->GetPositionX();
        float const y = bot->GetPositionY();
        float const z = bot->GetPositionZ();

        float const surfaceZ = map->GetWaterOrGroundLevel(
            PHASEMASK_NORMAL, x, y, z, nullptr, false, bot->GetCollisionHeight());
        if (surfaceZ <= z || surfaceZ == VMAP_INVALID_HEIGHT_VALUE)
            return false;

        return MoveTo(bot->GetMapId(), x, y, surfaceZ, false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED);
    }
};

#endif
