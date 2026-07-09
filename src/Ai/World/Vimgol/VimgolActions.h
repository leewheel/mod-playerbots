/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option) any later version.
 */

#ifndef PLAYERBOTS_VIMGOLACTIONS_H
#define PLAYERBOTS_VIMGOLACTIONS_H

#include "MovementActions.h"

// Action: Move bot to an assigned fire ring (Circle Bunny) position for summoning Vim'gol
class VimgolMoveToFireRingAction : public MovementAction
{
public:
    VimgolMoveToFireRingAction(PlayerbotAI* botAI, std::string const name = "vimgol move to fire ring")
        : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

// Action: Move bot to an assigned fire ring (Circle Bunny) position to interrupt Unholy Growth
class VimgolInterruptGrowthAction : public MovementAction
{
public:
    VimgolInterruptGrowthAction(PlayerbotAI* botAI, std::string const name = "vimgol interrupt growth")
        : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

#endif
