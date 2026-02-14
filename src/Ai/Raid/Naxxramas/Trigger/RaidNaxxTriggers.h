// RaidNaxxTriggers.h
#ifndef _PLAYERBOT_RAIDNAXXTRIGGERS_H_
#define _PLAYERBOT_RAIDNAXXTRIGGERS_H_

#include "PlayerbotAI.h"
#include "Trigger.h"

// ==================== Patchwerk Triggers ====================

class NaxxPatchwerkCombatTrigger : public Trigger
{
public:
    NaxxPatchwerkCombatTrigger(PlayerbotAI* botAI);
    bool IsActive() override;
};

class NaxxPatchwerkFrenzyTrigger : public Trigger
{
public:
    NaxxPatchwerkFrenzyTrigger(PlayerbotAI* botAI);
    bool IsActive() override;
};

class NaxxPatchwerkBerserkTrigger : public Trigger
{
public:
    NaxxPatchwerkBerserkTrigger(PlayerbotAI* botAI);
    bool IsActive() override;
};

class NaxxPatchwerkOffTankPositionTrigger : public Trigger
{
public:
    NaxxPatchwerkOffTankPositionTrigger(PlayerbotAI* botAI);
    bool IsActive() override;
};

#endif
