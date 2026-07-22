/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TRADESTATUSACTION_H
#define PLAYERBOTS_TRADESTATUSACTION_H

#include "InventoryAction.h"
#include "QueryItemUsageAction.h"

class Player;
class PlayerbotAI;

class TradeStatusAction : public QueryItemUsageAction
{
public:
    TradeStatusAction(PlayerbotAI* botAI) : QueryItemUsageAction(botAI, "accept trade") {}

    bool Execute(Event event) override;

private:
    void BeginTrade();
    bool CheckTrade();
    int32 CalculateCost(Player* player, bool sell);
    void TryGiveConjuredRefreshment(Player* trader, Player* master);  // --By leewheel 2026-07-22
    void CastConjure(std::string const& spell, uint32 category);      // --By leewheel 2026-07-22
};

// 法师机器人交易时自动给玩家法力面包和水 --By leewheel 2026-07-22
class GiveConjuredRefreshmentAction : public InventoryAction
{
public:
    GiveConjuredRefreshmentAction(PlayerbotAI* botAI) : InventoryAction(botAI, "give conjured refreshment") {}

    bool Execute(Event event) override;
    bool isUseful() override;

private:
    bool GiveOne(std::string const parseName, uint32 category);
};

#endif
