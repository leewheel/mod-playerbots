/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_DROWNINGACTIONS_H
#define PLAYERBOTS_DROWNINGACTIONS_H

#include "MovementActions.h"
#include "Playerbots.h"

#include <algorithm>

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

        if (!MoveTo(bot->GetMapId(), x, y, surfaceZ, false, false, false, false,
                    MovementPriority::MOVEMENT_FORCED))
            return false;

        // By leewheel 2026-09-05 修复: 深水区上浮会被战斗追击动作打断导致淹死
        //  MoveTo 内部把"最近移动"的抑制时长(maxWaitForMove, 默认5000毫秒)封顶了,
        //  深水里 5 秒后战斗追击(MOVEMENT_COMBAT, 优先级低于 MOVEMENT_FORCED)
        //  会把机器人重新拽回水下, 使它永远爬不到水面, 一直泡在水里直到淹死。
        //  这里在上浮成功后把抑制时长按"3D距离/游泳速度+3秒余量"重新写入,
        //  让战斗追击在成功抵达水面换气前无法打断上浮动作。
        float const dist = bot->GetExactDist(x, y, surfaceZ);
        float const swimSpeed = std::max(0.1f, bot->GetSpeed(MOVE_SWIM));
        float const suppressDelay = 1000.0f * (dist / swimSpeed) + 3000.0f;
        AI_VALUE(LastMovement&, "last movement")
            .Set(bot->GetMapId(), x, y, surfaceZ, bot->GetOrientation(), suppressDelay,
                 MovementPriority::MOVEMENT_FORCED);
        // End By leewheel

        return true;
    }
};

#endif
