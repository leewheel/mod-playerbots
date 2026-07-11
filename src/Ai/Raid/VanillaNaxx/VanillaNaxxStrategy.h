
#ifndef PLAYERBOTS_VANILLANAXXSTRATEGY_H
#define PLAYERBOTS_VANILLANAXXSTRATEGY_H

#include "Strategy.h"

class RaidVanillaNaxxStrategy : public Strategy
{
public:
    RaidVanillaNaxxStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    virtual std::string const getName() override { return "vanilla naxx"; }
    virtual void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    virtual void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
