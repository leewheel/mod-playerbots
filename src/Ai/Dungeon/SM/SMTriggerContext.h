/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SMTRIGGERCONTEXT_H
#define PLAYERBOTS_SMTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "SMTriggers.h"

class VanillaDungeonSMTriggerContext : public NamedObjectContext<Trigger>
{
public:
    VanillaDungeonSMTriggerContext()
    {
        creators["arcanist doan boss casting detonation"] =
            &VanillaDungeonSMTriggerContext::arcanist_doan_boss_casting_detonation;
        creators["arcanist doan ranged"] = &VanillaDungeonSMTriggerContext::arcanist_doan_ranged;
        creators["herod whirlwind"] = &VanillaDungeonSMTriggerContext::herod_whirlwind;
        creators["whitemane focus"] = &VanillaDungeonSMTriggerContext::whitemane_focus;
    }
private:
    static Trigger* arcanist_doan_boss_casting_detonation(PlayerbotAI* botAI) {
        return new ArcanistDoanBossCastingDetonationTrigger(botAI);
    }
    static Trigger* arcanist_doan_ranged(PlayerbotAI* botAI) { return new ArcanistDoanRangedTrigger(botAI); }
    static Trigger* herod_whirlwind(PlayerbotAI* botAI) { return new HerodWhirlwindTrigger(botAI); }
    static Trigger* whitemane_focus(PlayerbotAI* botAI) { return new WhitemaneFocusTrigger(botAI); }
};

#endif
