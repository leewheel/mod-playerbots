/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_DROWNINGTRIGGERS_H
#define PLAYERBOTS_DROWNINGTRIGGERS_H

#include "Playerbots.h"
#include "Trigger.h"

class PlayerbotAI;

// Active while the bot's breath timer is running low, so it can swim to the
// surface before drowning damage starts (bots never surface on their own).
class LowBreathTrigger : public Trigger
{
public:
    LowBreathTrigger(PlayerbotAI* botAI) : Trigger(botAI, "low breath") {}

    bool IsActive() override
    {
        if (!bot->IsAlive() || !bot->IsInWorld())
            return false;

        int32 const breath = bot->GetMirrorTimerValue(BREATH_TIMER);
        if (breath == DISABLED_MIRROR_TIMER)
            return false;

        // Surface while a full minute of air is left: reaching the surface
        // takes time and drowning damage is 20% max health per second.
        return breath < 60 * IN_MILLISECONDS;
    }
};

#endif
