/*
* This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
* information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License, or (at your option) any later version.
*/

#ifndef PLAYERBOTS_SHSTRATEGY_H
#define PLAYERBOTS_SHSTRATEGY_H

#include "AiObjectContext.h"
#include "Strategy.h"
#include "Multiplier.h"

// By leewheel 2026-09-16
// 地狱火堡垒：破碎大厅（map 540）机器人策略。
// 此前 mod-playerbots 只有 HFR(543)/AC(558)/Seth(556)/Mech(554)/UB(546)/MgT(585) 六个 TBC 五人本策略，
// 破碎大厅完全没有 —— 本次从零建立：四个首领的关键技能规避 + 下水道跟随跳跃。
class TbcDungeonShatteredHallsStrategy : public Strategy
{
public:
    TbcDungeonShatteredHallsStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    virtual std::string const getName() override { return "tbc-sh"; }

    virtual void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    virtual void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
