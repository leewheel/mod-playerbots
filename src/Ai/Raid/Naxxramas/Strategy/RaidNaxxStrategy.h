// RaidNaxxStrategy.h
#ifndef _PLAYERBOT_RAIDNAXXSTRATEGY_H_
#define _PLAYERBOT_RAIDNAXXSTRATEGY_H_

#include "Strategy.h"

class RaidNaxxStrategy : public Strategy
{
public:
    RaidNaxxStrategy(PlayerbotAI* ai) : Strategy(ai) {}

    std::string const getName() override { return "naxx"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
