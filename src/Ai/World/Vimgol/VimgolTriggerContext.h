/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option) any later version.
 */

#ifndef PLAYERBOTS_VIMGOLTRIGGERCONTEXT_H
#define PLAYERBOTS_VIMGOLTRIGGERCONTEXT_H

#include "VimgolTriggers.h"
#include "NamedObjectContext.h"

class VimgolTriggerContext : public NamedObjectContext<Trigger>
{
public:
    VimgolTriggerContext() : NamedObjectContext<Trigger>(false, true)
    {
        creators["vimgol near circle"] = &VimgolTriggerContext::vimgol_near_circle;
        creators["vimgol summoning phase"] = &VimgolTriggerContext::vimgol_summoning_phase;
        creators["vimgol unholy growth"] = &VimgolTriggerContext::vimgol_unholy_growth;
    }

private:
    static Trigger* vimgol_near_circle(PlayerbotAI* botAI)
    {
        return new VimgolNearCircleTrigger(botAI);
    }

    static Trigger* vimgol_summoning_phase(PlayerbotAI* botAI)
    {
        return new VimgolSummoningPhaseTrigger(botAI);
    }

    static Trigger* vimgol_unholy_growth(PlayerbotAI* botAI)
    {
        return new VimgolUnholyGrowthTrigger(botAI);
    }
};

#endif
