/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "TradeStatusAction.h"
#include "CraftValue.h"
#include "Event.h"
#include "GuildTaskMgr.h"
#include "ItemTemplate.h"
#include "ItemUsageValue.h"
#include "ItemVisitors.h"
#include "ObjectMgr.h"
#include "Opcodes.h"
#include "PlayerbotMgr.h"
#include "PlayerbotSecurity.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"
#include "RandomPlayerbotMgr.h"
#include "SetCraftAction.h"
#include "SpellMgr.h"
#include "Util.h"

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
        //By leewheel 2026-08-05 交易确认前补放 conjure 面包水/术士石头
        //  原因：BEGIN_TRADE 时施法 conjure 的物品依赖 "item push result" 事件补放，
        //        该事件在某些场景不触发导致水/石头不进交易栏；玩家点确认时补放可覆盖。
        //        GiveOne 已有"交易栏已有同类则跳过"判断，不会重复放置。
        TryGiveConjuredRefreshment(trader, master);
        TryGiveWarlockStones(trader, master);
        //End By leewheel

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

        // 法师机器人交易时自动给玩家法力面包和水 --By leewheel 2026-07-22
        TryGiveConjuredRefreshment(trader, master);
        // 术士机器人交易时自动给玩家治疗石/灵魂石 --By leewheel 2026-08-05
        TryGiveWarlockStones(trader, master);

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
            out << "折扣上限：" << chat->formatMoney(discount);
            botAI->TellMaster(out);
        }
    }
}

bool TradeStatusAction::CheckTrade()
{
    Player* trader = bot->GetTrader();
    if (!bot->GetTradeData() || !trader || !trader->GetTradeData())
        return false;

    if (!IsRealPlayer(botAI->GetMaster()) && GET_PLAYERBOT_AI(bot->GetTrader()))
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
                botAI->HasGameClientMaster())
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

// 法师机器人交易时自动给玩家法力面包和水 --By leewheel 2026-07-22
// 打开交易窗口时：先把背包里已有的魔法面包/水放入交易栏，缺少的则施法 conjure，
// conjure 完成后由 "item push result" 触发器再次调用 give conjured refreshment 补放。
void TradeStatusAction::TryGiveConjuredRefreshment(Player* trader, Player* master)
{
    if (!sPlayerbotAIConfig.enableMageTradeFoodWater)
        return;

    // 仅法师机器人
    if (bot->getClass() != CLASS_MAGE)
        return;

    // 交易对象必须是真实玩家
    if (!trader || GET_PLAYERBOT_AI(trader))
        return;

    // 仅主人或同队/团队成员
    if (trader != master && (!bot->GetGroup() || !bot->GetGroup()->IsMember(trader->GetGUID())))
        return;

    if (!bot->GetTradeData())
        return;

    // 先把背包里已有的魔法面包/水放入交易栏
    botAI->DoSpecificAction("give conjured refreshment", Event(), true);

    // 缺少的施法 conjure（背包和交易栏都没有时才施法，避免重复制造）--By leewheel 2026-08-15
    if (parseItems("conjured food", ITERATE_ITEMS_IN_BAGS).empty() && !TradeHasConjured(11))
        CastConjure("conjure food", 11);

    if (parseItems("conjured water", ITERATE_ITEMS_IN_BAGS).empty() && !TradeHasConjured(59))
        CastConjure("conjure water", 59);
}

// 施放 conjure food/water，按生成物品的法术类别匹配（兼容中文客户端法术名） --By leewheel 2026-07-22
void TradeStatusAction::CastConjure(std::string const& spell, uint32 category)
{
    uint32 castId = 0;

    for (PlayerSpellMap::iterator itr = bot->GetSpellMap().begin(); itr != bot->GetSpellMap().end(); ++itr)
    {
        uint32 spellId = itr->first;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo)
            continue;

        if (spellInfo->Effects[0].Effect != SPELL_EFFECT_CREATE_ITEM)
            continue;

        uint32 itemId = spellInfo->Effects[0].ItemType;
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
        if (!proto || bot->CanUseItem(proto) != EQUIP_ERR_OK)
            continue;

        // 优先按生成物品的法术类别匹配（11=食物 59=饮水），与客户端语言无关
        bool match = proto->IsConjuredConsumable() && proto->Spells[0].SpellCategory == category;

        // 兼容按法术英文名匹配
        if (!match)
        {
            std::string const namepart = spellInfo->SpellName[0];
            std::wstring wnamepart;
            if (Utf8toWStr(namepart, wnamepart))
            {
                wstrToLower(wnamepart);
                match = Utf8FitTo(spell, wnamepart);
            }
        }

        if (!match)
            continue;

        // 取最高等级
        if (spellInfo->Id > castId)
            castId = spellInfo->Id;
    }

    if (castId)
        botAI->CastSpell(castId, bot);
}

bool GiveConjuredRefreshmentAction::isUseful()
{
    return bot->GetTrader() && bot->getClass() == CLASS_MAGE && sPlayerbotAIConfig.enableMageTradeFoodWater;
}

bool GiveConjuredRefreshmentAction::Execute(Event /*event*/)
{
    Player* trader = bot->GetTrader();
    if (!trader || GET_PLAYERBOT_AI(trader))
        return false;

    if (bot->getClass() != CLASS_MAGE || !sPlayerbotAIConfig.enableMageTradeFoodWater)
        return false;

    if (!bot->GetTradeData())
        return false;

    bool given = false;
    given = GiveOne("conjured food", 11) || given;
    given = GiveOne("conjured water", 59) || given;
    return given;
}

// 把背包里一个对应类别的魔法物品放入空闲交易栏 --By leewheel 2026-07-22
bool GiveConjuredRefreshmentAction::GiveOne(std::string const parseName, uint32 category)
{
    TradeData* pTrade = bot->GetTradeData();
    if (!pTrade)
        return false;

    // 交易栏里已有该类别的魔法物品则不再重复放
    for (uint32 slot = 0; slot < TRADE_SLOT_TRADED_COUNT; ++slot)
    {
        Item* item = pTrade->GetItem((TradeSlots)slot);
        if (item && item->GetTemplate()->IsConjuredConsumable() &&
            item->GetTemplate()->Spells[0].SpellCategory == category)
            return false;
    }

    std::vector<Item*> items = parseItems(parseName, ITERATE_ITEMS_IN_BAGS);
    for (Item* item : items)
    {
        if (!item || item->IsInTrade())
            continue;

        ItemTemplate const* proto = item->GetTemplate();
        if (!proto || !proto->IsConjuredConsumable() || proto->Spells[0].SpellCategory != category)
            continue;

        // 找一个空闲交易栏
        int8 tradeSlot = -1;
        for (uint8 i = 0; i < TRADE_SLOT_TRADED_COUNT && tradeSlot == -1; i++)
            if (pTrade->GetItem(TradeSlots(i)) == nullptr)
                tradeSlot = i;

        if (tradeSlot == -1)
            return false;

        WorldPacket packet(CMSG_SET_TRADE_ITEM, 3);
        packet << (uint8)tradeSlot;
        packet << (uint8)item->GetBagSlot();
        packet << (uint8)item->GetSlot();
        bot->GetSession()->HandleSetTradeItemOpcode(packet);
        return true;
    }

    return false;
}

// 术士机器人交易时自动给玩家治疗石/灵魂石(糖果) --By leewheel 2026-08-05
// 打开交易窗口时：先把背包里已有的治疗石/灵魂石放入交易栏，缺少的则施法制造，
// 制造完成后由 "item push result" 触发器或交易确认前补放。
void TradeStatusAction::TryGiveWarlockStones(Player* trader, Player* master)
{
    if (!sPlayerbotAIConfig.enableWarlockTradeStones)
        return;

    // 仅术士机器人
    if (bot->getClass() != CLASS_WARLOCK)
        return;

    // 交易对象必须是真实玩家
    if (!trader || GET_PLAYERBOT_AI(trader))
        return;

    // 仅主人或同队/团队成员
    if (trader != master && (!bot->GetGroup() || !bot->GetGroup()->IsMember(trader->GetGUID())))
        return;

    if (!bot->GetTradeData())
        return;

    // 先把背包里已有的治疗石/灵魂石放入交易栏
    botAI->DoSpecificAction("give warlock stone", Event(), true);

    // 缺少的施法制造（背包和交易栏都没有时才施法，避免重复制造）--By leewheel 2026-08-15
    if (parseItems("healthstone", ITERATE_ITEMS_IN_BAGS).empty() && !TradeHasItem("healthstone"))
        CastWarlockStone("create healthstone");

    if (parseItems("soulstone", ITERATE_ITEMS_IN_BAGS).empty() && !TradeHasItem("soulstone"))
        CastWarlockStone("create soulstone");
}

// 施放 create healthstone / create soulstone，按法术英文名(enUS)匹配 --By leewheel 2026-08-05
void TradeStatusAction::CastWarlockStone(std::string const& spell)
{
    uint32 castId = 0;

    for (PlayerSpellMap::iterator itr = bot->GetSpellMap().begin(); itr != bot->GetSpellMap().end(); ++itr)
    {
        uint32 spellId = itr->first;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo)
            continue;

        // 只处理生成物品的法术
        if (spellInfo->Effects[0].Effect != SPELL_EFFECT_CREATE_ITEM)
            continue;

        // 按法术名匹配（含中文客户端法术名）
        bool match = false;

        std::string const namepart = spellInfo->SpellName[0];
        std::wstring wnamepart;
        if (Utf8toWStr(namepart, wnamepart))
        {
            wstrToLower(wnamepart);
            match = Utf8FitTo(spell, wnamepart);
        }

        if (!match)
            continue;

        // 取最高等级
        if (spellInfo->Id > castId)
            castId = spellInfo->Id;
    }

    if (castId)
        botAI->CastSpell(castId, bot);
}

bool GiveWarlockStoneAction::isUseful()
{
    return bot->GetTrader() && bot->getClass() == CLASS_WARLOCK && sPlayerbotAIConfig.enableWarlockTradeStones;
}

bool GiveWarlockStoneAction::Execute(Event /*event*/)
{
    Player* trader = bot->GetTrader();
    if (!trader || GET_PLAYERBOT_AI(trader))
        return false;

    if (bot->getClass() != CLASS_WARLOCK || !sPlayerbotAIConfig.enableWarlockTradeStones)
        return false;

    if (!bot->GetTradeData())
        return false;

    bool given = false;
    given = GiveOne("healthstone", false) || given;
    given = GiveOne("soulstone", true) || given;
    return given;
}

// 把背包里一个对应名称的治疗石/灵魂石放入空闲交易栏 --By leewheel 2026-08-05
bool GiveWarlockStoneAction::GiveOne(std::string const itemName, bool soul)
{
    TradeData* pTrade = bot->GetTradeData();
    if (!pTrade)
        return false;

    // 交易栏里已有同类石头则不再重复放
    for (uint32 slot = 0; slot < TRADE_SLOT_TRADED_COUNT; ++slot)
    {
        Item* item = pTrade->GetItem((TradeSlots)slot);
        if (item && item->GetTemplate() &&
            strstri(item->GetTemplate()->Name1.c_str(), (soul ? "soulstone" : "healthstone")))
            return false;
    }

    // 从背包找一个对应名称的石头
    std::vector<Item*> items = parseItems(itemName, ITERATE_ITEMS_IN_BAGS);
    for (Item* item : items)
    {
        if (!item || item->IsInTrade())
            continue;

        ItemTemplate const* proto = item->GetTemplate();
        if (!proto || !strstri(proto->Name1.c_str(), (soul ? "soulstone" : "healthstone")))
            continue;

        // 找一个空闲交易栏
        int32 tradeSlot = -1;
        for (uint32 slot = 0; slot < TRADE_SLOT_TRADED_COUNT; ++slot)
        {
            if (!pTrade->GetItem((TradeSlots)slot))
            {
                tradeSlot = slot;
                break;
            }
        }

        if (tradeSlot == -1)
            return false;

        WorldPacket packet(CMSG_SET_TRADE_ITEM, 3);
        packet << (uint8)tradeSlot;
        packet << (uint8)item->GetBagSlot();
        packet << (uint8)item->GetSlot();
        bot->GetSession()->HandleSetTradeItemOpcode(packet);
        return true;
    }

    return false;
}

// 判断交易栏是否已有指定名称的物品(术士石头用) --By leewheel 2026-08-15
bool TradeStatusAction::TradeHasItem(std::string const itemName) const
{
    TradeData* pTrade = bot->GetTradeData();
    if (!pTrade)
        return false;

    for (uint32 slot = 0; slot < TRADE_SLOT_TRADED_COUNT; ++slot)
    {
        Item* item = pTrade->GetItem((TradeSlots)slot);
        if (item && item->GetTemplate() &&
            strstri(item->GetTemplate()->Name1.c_str(), itemName.c_str()))
            return true;
    }
    return false;
}

// 判断交易栏是否已有指定类别的魔法食物/饮水(法师面包水用) --By leewheel 2026-08-15
bool TradeStatusAction::TradeHasConjured(uint32 category) const
{
    TradeData* pTrade = bot->GetTradeData();
    if (!pTrade)
        return false;

    for (uint32 slot = 0; slot < TRADE_SLOT_TRADED_COUNT; ++slot)
    {
        Item* item = pTrade->GetItem((TradeSlots)slot);
        if (item && item->GetTemplate() && item->GetTemplate()->IsConjuredConsumable() &&
            item->GetTemplate()->Spells[0].SpellCategory == category)
            return true;
    }
    return false;
}
