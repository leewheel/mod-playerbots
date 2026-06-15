/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "TradeStatusAction.h"

#include "CraftValue.h"
#include "Event.h"
#include "GuildTaskMgr.h"
#include "ItemUsageValue.h"
#include "ItemVisitors.h"
#include "PlayerbotMgr.h"
#include "PlayerbotSecurity.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"
#include "RandomPlayerbotMgr.h"
#include "SetCraftAction.h"

bool TradeStatusAction::Execute(Event event)
{
    Player* trader = bot->GetTrader();
    Player* master = GetMaster();
    if (!trader)
        return false;

    PlayerbotAI* traderBotAI = GET_PLAYERBOT_AI(trader);

    // Allow the master and group members to trade
    if (trader != master && !traderBotAI && (!bot->GetGroup() || !bot->GetGroup()->IsMember(trader->GetGUID())))
    {
        bot->Whisper(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                         "trade_busy_now", "我现在有点忙", {}),
                     LANG_UNIVERSAL, trader);
        return false;
    }

    if (sPlayerbotAIConfig.enableRandomBotTrading == 0 && (sRandomPlayerbotMgr.IsRandomBot(bot)|| sRandomPlayerbotMgr.IsAddclassBot(bot)))
    {
        bot->Whisper(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                         "trade_disabled", "交易已禁用", {}),
                     LANG_UNIVERSAL, trader);
        return false;
    }

    // Allow trades from group members or bots
    if ((!bot->GetGroup() || !bot->GetGroup()->IsMember(trader->GetGUID())) &&
        (trader != master || !botAI->GetSecurity()->CheckLevelFor(PLAYERBOT_SECURITY_ALLOW_ALL, true, master)) &&
        !traderBotAI)
    {
        WorldPacket p;
        uint32 status = 0;
        p << status;
        bot->GetSession()->HandleCancelTradeOpcode(p);
        return false;
    }

    WorldPacket p(event.getPacket());
    p.rpos(0);
    uint32 status;
    p >> status;

    if (status == TRADE_STATUS_TRADE_ACCEPT || (status == TRADE_STATUS_BACK_TO_TRADE && trader->GetTradeData() && trader->GetTradeData()->IsAccepted()))
    {
        WorldPacket p;
        uint32 status = 0;
        p << status;

        uint32 discount = sRandomPlayerbotMgr.GetTradeDiscount(bot, trader);
        if (CheckTrade())
        {
            std::map<uint32, uint32> givenItemIds, takenItemIds;
            for (uint32 slot = 0; slot < TRADE_SLOT_TRADED_COUNT; ++slot)
            {
                Item* item = trader->GetTradeData()->GetItem((TradeSlots)slot);
                if (item)
                    givenItemIds[item->GetTemplate()->ItemId] += item->GetCount();

                item = bot->GetTradeData()->GetItem((TradeSlots)slot);
                if (item)
                    takenItemIds[item->GetTemplate()->ItemId] += item->GetCount();
            }

            bot->GetSession()->HandleAcceptTradeOpcode(p);
            if (bot->GetTradeData())
            {
                sRandomPlayerbotMgr.SetTradeDiscount(bot, trader, discount);
                return false;
            }

            for (std::map<uint32, uint32>::iterator i = givenItemIds.begin(); i != givenItemIds.end(); ++i)
            {
                uint32 itemId = i->first;
                uint32 count = i->second;

                CraftData& craftData = AI_VALUE(CraftData&, "craft");
                if (!craftData.IsEmpty() && craftData.IsRequired(itemId))
                {
                    craftData.AddObtained(itemId, count);
                }

                GuildTaskMgr::instance().CheckItemTask(itemId, count, trader, bot);
            }

            for (std::map<uint32, uint32>::iterator i = takenItemIds.begin(); i != takenItemIds.end(); ++i)
            {
                uint32 itemId = i->first;
                uint32 count = i->second;

                CraftData& craftData = AI_VALUE(CraftData&, "craft");
                if (!craftData.IsEmpty() && craftData.itemId == itemId)
                {
                    craftData.Crafted(count);
                }
            }

            return true;
        }
    }
    else if (status == TRADE_STATUS_BEGIN_TRADE)
    {
        if (!bot->HasInArc(CAST_ANGLE_IN_FRONT, trader, sPlayerbotAIConfig.sightDistance))
            bot->SetFacingToObject(trader);

        BeginTrade();

        return true;
    }
    return false;
}

void TradeStatusAction::BeginTrade()
{
    Player* trader = bot->GetTrader();
    if (!trader || GET_PLAYERBOT_AI(bot->GetTrader()))
        return;

    WorldPacket p;
    bot->GetSession()->HandleBeginTradeOpcode(p);

    ListItemsVisitor visitor;
    IterateItems(&visitor);

    botAI->TellMaster("=== 背包 ===");
    TellItems(visitor.items, visitor.soulbound);

    if (sRandomPlayerbotMgr.IsRandomBot(bot))
    {
        uint32 discount = sRandomPlayerbotMgr.GetTradeDiscount(bot, botAI->GetMaster());
        if (discount)
        {
            std::ostringstream out;
            out << "Discount up to: " << chat->formatMoney(discount);
            botAI->TellMaster(out);
        }
    }
}

bool TradeStatusAction::CheckTrade()
{
    Player* trader = bot->GetTrader();
    if (!bot->GetTradeData() || !trader || !trader->GetTradeData())
        return false;

    if (!botAI->HasActivePlayerMaster() && GET_PLAYERBOT_AI(bot->GetTrader()))
    {
        for (uint32 slot = 0; slot < TRADE_SLOT_TRADED_COUNT; ++slot)
        {
            Item* item = bot->GetTradeData()->GetItem((TradeSlots)slot);
            if (item)
                break;
        }
        bool isGettingItem = false;
        for (uint32 slot = 0; slot < TRADE_SLOT_TRADED_COUNT; ++slot)
        {
            Item* item = trader->GetTradeData()->GetItem((TradeSlots)slot);
            if (item)
            {
                isGettingItem = true;
                break;
            }
        }

        if (isGettingItem)
        {
            if (bot->GetGroup() && bot->GetGroup()->IsMember(bot->GetTrader()->GetGUID()) &&
                botAI->HasRealPlayerMaster())
                botAI->TellMasterNoFacing(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                    "trade_thank_you_player",
                    "谢谢你 %player",
                    {{"%player", chat->FormatWorldobject(bot->GetTrader())}}));
            else
                bot->Say(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                             "trade_thank_you_player",
                             "谢谢你 %player",
                             {{"%player", chat->FormatWorldobject(bot->GetTrader())}}),
                         (bot->GetTeamId() == TEAM_ALLIANCE ? LANG_COMMON : LANG_ORCISH));
        }
        return isGettingItem;
    }
    if (!bot->GetSession())
    {
        return false;
    }
    uint32 accountId = bot->GetSession()->GetAccountId();
    if (!sPlayerbotAIConfig.IsInRandomAccountList(accountId))
    {
        int32 botItemsMoney = CalculateCost(bot, true);
        int32 botMoney = bot->GetTradeData()->GetMoney() + botItemsMoney;
        int32 playerItemsMoney = CalculateCost(trader, false);
        int32 playerMoney = trader->GetTradeData()->GetMoney() + playerItemsMoney;
        if (playerMoney || botMoney)
            botAI->PlaySound(playerMoney < botMoney ? TEXT_EMOTE_SIGH : TEXT_EMOTE_THANK);

        return true;
    }

    int32 botItemsMoney = CalculateCost(bot, true);
    int32 botMoney = bot->GetTradeData()->GetMoney() + botItemsMoney;
    int32 playerItemsMoney = CalculateCost(trader, false);
    int32 playerMoney = trader->GetTradeData()->GetMoney() + playerItemsMoney;
    if (botItemsMoney > 0 && sPlayerbotAIConfig.enableRandomBotTrading == 2 && (sRandomPlayerbotMgr.IsRandomBot(bot)|| sRandomPlayerbotMgr.IsAddclassBot(bot)))
    {
        bot->Whisper(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                         "trade_selling_disabled", "出售已禁用。", {}),
                     LANG_UNIVERSAL, trader);
        return false;
    }
    if (playerItemsMoney && sPlayerbotAIConfig.enableRandomBotTrading == 3 && (sRandomPlayerbotMgr.IsRandomBot(bot)|| sRandomPlayerbotMgr.IsAddclassBot(bot)))
    {
        bot->Whisper(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                         "trade_buying_disabled", "购买已禁用。", {}),
                     LANG_UNIVERSAL, trader);
        return false;
    }
    for (uint32 slot = 0; slot < TRADE_SLOT_TRADED_COUNT; ++slot)
    {
        Item* item = bot->GetTradeData()->GetItem((TradeSlots)slot);
        if (item && !item->GetTemplate()->SellPrice && !item->GetTemplate()->IsConjuredConsumable())
        {
            std::ostringstream out;
            botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "trade_item_not_for_sale",
                "%item - 此物品不出售",
                {{"%item", chat->FormatItem(item->GetTemplate())}}));
            botAI->PlaySound(TEXT_EMOTE_NO);
            return false;
        }

        item = trader->GetTradeData()->GetItem((TradeSlots)slot);
        if (item)
        {
            std::ostringstream out;
            out << item->GetTemplate()->ItemId;
            ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", out.str());
            if ((botMoney && !item->GetTemplate()->BuyPrice) || usage == ITEM_USAGE_NONE)
            {
                std::ostringstream out;
                botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                    "trade_item_not_needed",
                    "%item - 我不需要此物品",
                    {{"%item", chat->FormatItem(item->GetTemplate())}}));
                botAI->PlaySound(TEXT_EMOTE_NO);
                return false;
            }
        }
    }

    if (!botMoney && !playerMoney)
        return true;

    if (!botItemsMoney && !playerItemsMoney)
    {
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "trade_no_items_error", "没有可交易的物品", {}));
        return false;
    }

    int32 discount = (int32)sRandomPlayerbotMgr.GetTradeDiscount(bot, trader);
    int32 delta = playerMoney - botMoney;
    int32 moneyDelta = (int32)trader->GetTradeData()->GetMoney() - (int32)bot->GetTradeData()->GetMoney();
    bool success = false;
    if (delta < 0)
    {
        if (delta + discount >= 0)
        {
            if (moneyDelta < 0)
            {
                botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                    "trade_discount_buy_only", "折扣只能用于购买物品", {}));
                botAI->PlaySound(TEXT_EMOTE_NO);
                return false;
            }
            success = true;
        }
    }
    else
        success = true;

    if (success)
    {
        sRandomPlayerbotMgr.AddTradeDiscount(bot, trader, delta);
        switch (urand(0, 4))
        {
            case 0:
                botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                    "trade_success_pleasure", "很高兴与你交易", {}));
                break;
            case 1:
                botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                    "trade_success_fair_trade", "公平交易", {}));
                break;
            case 2:
                botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                    "trade_success_thanks", "谢谢", {}));
                break;
            case 3:
                botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                    "trade_success_off_with_you", "走吧", {}));
                break;
        }

        botAI->PlaySound(TEXT_EMOTE_THANK);
        return true;
    }

    std::ostringstream out;
    botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "trade_want_money_for_this",
        "此物品我要 %money",
        {{"%money", chat->formatMoney(-(delta + discount))}}));
    botAI->PlaySound(TEXT_EMOTE_NO);
    return false;
}

int32 TradeStatusAction::CalculateCost(Player* player, bool sell)
{
    Player* trader = bot->GetTrader();
    TradeData* data = player->GetTradeData();
    if (!data)
        return 0;

    uint32 sum = 0;
    for (uint32 slot = 0; slot < TRADE_SLOT_TRADED_COUNT; ++slot)
    {
        Item* item = data->GetItem((TradeSlots)slot);
        if (!item)
            continue;

        ItemTemplate const* proto = item->GetTemplate();
        if (!proto)
            continue;

        if (proto->Quality < ITEM_QUALITY_NORMAL)
            return 0;

        CraftData& craftData = AI_VALUE(CraftData&, "craft");
        if (!craftData.IsEmpty())
        {
            if (player == trader && !sell && craftData.IsRequired(proto->ItemId))
                continue;

            if (player == bot && sell && craftData.itemId == proto->ItemId && craftData.IsFulfilled())
            {
                sum += item->GetCount() * SetCraftAction::GetCraftFee(craftData);
                continue;
            }
        }

        if (sell)
            sum += item->GetCount() * proto->SellPrice * sRandomPlayerbotMgr.GetSellMultiplier(bot);

        else
            sum += item->GetCount() * proto->BuyPrice * sRandomPlayerbotMgr.GetBuyMultiplier(bot);

    }

    return sum;
}
