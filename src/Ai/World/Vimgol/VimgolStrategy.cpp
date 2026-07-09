/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option) any later version.
 */

#include "VimgolStrategy.h"
#include "Playerbots.h"

void VimgolStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Phase 1: Summoning - when near the circle and Vim'gol is not yet spawned,
    // all group members move to their assigned fire ring positions
    triggers.push_back(new TriggerNode("vimgol summoning phase",
            { NextAction("vimgol move to fire ring", ACTION_RAID + 1) }));

    // Phase 2: Interrupt - when Vim'gol casts Unholy Growth (aura 40545),
    // all group members rush to their assigned fire ring positions to interrupt
    triggers.push_back(new TriggerNode("vimgol unholy growth",
            { NextAction("vimgol interrupt growth", ACTION_EMERGENCY + 5) }));
}
