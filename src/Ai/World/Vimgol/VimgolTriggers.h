/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option) any later version.
 */

#ifndef PLAYERBOTS_VIMGOLTRIGGERS_H
#define PLAYERBOTS_VIMGOLTRIGGERS_H

#include "Trigger.h"

// Trigger: bot is near Vim'gol's Circle (any Circle Bunny creature 23040 within 80 yards)
class VimgolNearCircleTrigger : public Trigger
{
public:
    VimgolNearCircleTrigger(PlayerbotAI* botAI) : Trigger(botAI, "vimgol near circle") {}
    bool IsActive() override;
};

// Trigger: Vim'gol is not yet spawned but we are near the circle -> summoning phase
class VimgolSummoningPhaseTrigger : public Trigger
{
public:
    VimgolSummoningPhaseTrigger(PlayerbotAI* botAI) : Trigger(botAI, "vimgol summoning phase") {}
    bool IsActive() override;
};

// Trigger: Vim'gol is alive and casting/has Unholy Growth (spell 40545) -> interrupt phase
class VimgolUnholyGrowthTrigger : public Trigger
{
public:
    VimgolUnholyGrowthTrigger(PlayerbotAI* botAI) : Trigger(botAI, "vimgol unholy growth") {}
    bool IsActive() override;
};

#endif
