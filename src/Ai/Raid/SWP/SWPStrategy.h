//By leewheel 2026-07-08
/*
 * 太阳之井高地 (Sunwell Plateau) 策略声明
 * 作者: leewheel
 */
//End By leewheel

#ifndef PLAYERBOTS_SWPSTRATEGY_H
#define PLAYERBOTS_SWPSTRATEGY_H

#include "Strategy.h"

class RaidSunwellPlateauStrategy : public Strategy
{
public:
    RaidSunwellPlateauStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "sunwell"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
