// RaidNaxxTriggerContext.h
#ifndef _PLAYERBOT_RAIDNAXXTRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDNAXXTRIGGERCONTEXT_H

#include "AiObjectContext.h"
#include "NamedObjectContext.h"
#include "RaidNaxxTriggers.h"

class RaidNaxxTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidNaxxTriggerContext()
    {
        // Patchwerk triggers
        creators["naxx patchwerk combat"] = &RaidNaxxTriggerContext::patchwerk_combat;
        creators["naxx patchwerk frenzy"] = &RaidNaxxTriggerContext::patchwerk_frenzy;
        creators["naxx patchwerk berserk"] = &RaidNaxxTriggerContext::patchwerk_berserk;
        creators["naxx patchwerk offtank position"] = &RaidNaxxTriggerContext::patchwerk_offtank_position;
    }

private:
    static Trigger* patchwerk_combat(PlayerbotAI* ai) 
    { 
        return new NaxxPatchwerkCombatTrigger(ai); 
    }
    
    static Trigger* patchwerk_frenzy(PlayerbotAI* ai) 
    { 
        return new NaxxPatchwerkFrenzyTrigger(ai); 
    }
    
    static Trigger* patchwerk_berserk(PlayerbotAI* ai) 
    { 
        return new NaxxPatchwerkBerserkTrigger(ai); 
    }
    
    static Trigger* patchwerk_offtank_position(PlayerbotAI* ai) 
    { 
        return new NaxxPatchwerkOffTankPositionTrigger(ai); 
    }
};

#endif
