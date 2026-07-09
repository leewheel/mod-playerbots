/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option) any later version.
 */

#ifndef PLAYERBOTS_VIMGOLACTIONCONTEXT_H
#define PLAYERBOTS_VIMGOLACTIONCONTEXT_H

#include "VimgolActions.h"
#include "NamedObjectContext.h"

class VimgolActionContext : public NamedObjectContext<Action>
{
public:
    VimgolActionContext()
    {
        creators["vimgol move to fire ring"] = &VimgolActionContext::vimgol_move_to_fire_ring;
        creators["vimgol interrupt growth"] = &VimgolActionContext::vimgol_interrupt_growth;
    }

private:
    static Action* vimgol_move_to_fire_ring(PlayerbotAI* botAI)
    {
        return new VimgolMoveToFireRingAction(botAI);
    }

    static Action* vimgol_interrupt_growth(PlayerbotAI* botAI)
    {
        return new VimgolInterruptGrowthAction(botAI);
    }
};

#endif
