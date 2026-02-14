// RaidNaxxActions.h
#ifndef _PLAYERBOT_RAIDNAXXACTIONS_H_
#define _PLAYERBOT_RAIDNAXXACTIONS_H_

#include "Action.h"
#include "AttackAction.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"

class PlayerbotAI;

// ==================== Patchwerk Actions ====================

class NaxxPatchwerkOffTankPositionAction : public MovementAction
{
public:
    NaxxPatchwerkOffTankPositionAction(PlayerbotAI* botAI, 
                                       std::string const name = "naxx patchwerk offtank position")
        : MovementAction(botAI, name)
    {
    }
    bool Execute(Event event) override;
};

class NaxxPatchwerkBurnPhaseAction : public AttackAction
{
public:
    NaxxPatchwerkBurnPhaseAction(PlayerbotAI* botAI, 
                                 std::string const name = "naxx patchwerk burn")
        : AttackAction(botAI, name)
    {
    }
    bool Execute(Event event) override;
};

#endif
