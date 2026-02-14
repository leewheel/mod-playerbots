// RaidNaxxActionContext.h
#ifndef _PLAYERBOT_RAIDNAXXACTIONS_CONTEXT_H
#define _PLAYERBOT_RAIDNAXXACTIONS_CONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "RaidNaxxActions.h"

class RaidNaxxActionContext : public NamedObjectContext<Action>
{
public:
    RaidNaxxActionContext()
    {
        // Patchwerk actions
        creators["naxx patchwerk offtank position"] = &RaidNaxxActionContext::patchwerk_offtank_position;
        creators["naxx patchwerk burn"] = &RaidNaxxActionContext::patchwerk_burn;
    }

private:
    static Action* patchwerk_offtank_position(PlayerbotAI* ai) 
    { 
        return new NaxxPatchwerkOffTankPositionAction(ai); 
    }
    
    static Action* patchwerk_burn(PlayerbotAI* ai) 
    { 
        return new NaxxPatchwerkBurnPhaseAction(ai); 
    }
};

#endif
