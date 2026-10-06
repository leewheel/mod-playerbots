/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BankAction.h"
#include "Event.h"
#include "ItemCountValue.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"

bool BankAction::Execute(Event event)
{
    std::string const text = event.getParam();

    GuidVector npcs = AI_VALUE(GuidVector, "nearest npcs");
    for (GuidVector::iterator i = npcs.begin(); i != npcs.end(); i++)
    {
        Unit* npc = botAI->GetUnit(*i);
        if (!npc || !npc->HasNpcFlag(UNIT_NPC_FLAG_BANKER))
            continue;

        return ExecuteBank(text, npc);
    }

    botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "bank_no_banker_nearby_error", "附近找不到银行职员", {}));
    return false;
}

bool BankAction::ExecuteBank(std::string const text, Unit* /*bank*/)
{
    if (text.empty() || text == "?")
    {
        ListItems();
        return true;
    }

    bool result = false;
    if (text[0] == '-')
    {
        // A withdrawal can free a later entry, so read every id first.
        std::vector<uint32> itemIds;
        for (Item* item : parseItems(text.substr(1), ITERATE_ITEMS_IN_BANK))
            itemIds.push_back(item->GetEntry());

        for (uint32 const itemId : itemIds)
            result &= Withdraw(itemId);
    }
    else
    {
        std::vector<Item*> found = parseItems(text, ITERATE_ITEMS_IN_BAGS);
        if (found.empty())
            return false;

        for (std::vector<Item*>::iterator i = found.begin(); i != found.end(); i++)
        {
            Item* item = *i;
            if (!item)
                continue;

            result &= Deposit(item);
        }
    }

    return result;
}

bool BankAction::Withdraw(uint32 itemid)
{
    Item* pItem = FindItemInBank(itemid);
    if (!pItem)
        return false;

    ItemPosCountVec dest;
    InventoryResult msg = bot->CanStoreItem(NULL_BAG, NULL_SLOT, dest, pItem, false);
    if (msg != EQUIP_ERR_OK)
    {
        bot->SendEquipError(msg, pItem, nullptr);
        return false;
    }

    std::ostringstream out;
    out << "got " << chat->FormatItem(pItem->GetTemplate(), pItem->GetCount()) << " from bank";

    bot->RemoveItem(pItem->GetBagSlot(), pItem->GetSlot(), true);
    bot->StoreItem(dest, pItem, true);

//By leewheel 2026-10-06 合并 brighton the-lab: 上游新增了英文 TellMaster 输出，
    //   而本核原有的中文输出版本是重复声明 std::ostringstream out（自动合并把两句都留下了）。
    //   处置：out 只声明一次，英文为主句、中文为补充，保留上游逻辑同时不丢本核汉化信息。
    out << "（从银行取出 " << chat->FormatItem(pItem->GetTemplate(), pItem->GetCount()) << "）";
    botAI->TellMaster(out.str());
    return true;
}

bool BankAction::Deposit(Item* pItem)
{
    ItemPosCountVec dest;
    InventoryResult msg = bot->CanBankItem(NULL_BAG, NULL_SLOT, dest, pItem, false);
    if (msg != EQUIP_ERR_OK)
    {
        bot->SendEquipError(msg, pItem, nullptr);
        return false;
    }

    std::ostringstream out;
    out << "put " << chat->FormatItem(pItem->GetTemplate(), pItem->GetCount()) << " to bank";

    bot->RemoveItem(pItem->GetBagSlot(), pItem->GetSlot(), true);
    bot->BankItem(dest, pItem, true);

//By leewheel 2026-10-06 合并 brighton the-lab: 同 Withdraw，out 只声明一次，英文为主句、中文为补充
    out << "（存入银行 " << chat->FormatItem(pItem->GetTemplate(), pItem->GetCount()) << "）";
    botAI->TellMaster(out.str());
    return true;
}

void BankAction::ListItems()
{
    botAI->TellMaster("=== 银行 ===");

    std::map<uint32, uint32> items;
    std::map<uint32, bool> soulbound;
    for (uint32 i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; ++i)
        if (Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            if (pItem)
            {
                items[pItem->GetTemplate()->ItemId] += pItem->GetCount();
                soulbound[pItem->GetTemplate()->ItemId] = pItem->IsSoulBound();
            }

    for (uint32 i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
        if (Bag* pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            if (pBag)
                for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
                    if (Item* pItem = pBag->GetItemByPos(j))
                        if (pItem)
                        {
                            items[pItem->GetTemplate()->ItemId] += pItem->GetCount();
                            soulbound[pItem->GetTemplate()->ItemId] = pItem->IsSoulBound();
                        }

    TellItems(items, soulbound);
}

Item* BankAction::FindItemInBank(uint32 ItemId)
{
    for (uint8 slot = BANK_SLOT_ITEM_START; slot < BANK_SLOT_ITEM_END; slot++)
    {
        if (Item* const pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
        {
            ItemTemplate const* const pItemProto = pItem->GetTemplate();
            if (!pItemProto)
                continue;

            if (pItemProto->ItemId == ItemId)  // have required item
                return pItem;
        }
    }

    for (uint8 bag = BANK_SLOT_BAG_START; bag < BANK_SLOT_BAG_END; ++bag)
    {
        Bag const* const pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, bag);
        if (pBag)
            for (uint8 slot = 0; slot < pBag->GetBagSize(); ++slot)
            {
                Item* const pItem = bot->GetItemByPos(bag, slot);
                if (pItem)
                {
                    ItemTemplate const* const pItemProto = pItem->GetTemplate();
                    if (!pItemProto)
                        continue;

                    if (pItemProto->ItemId == ItemId)
                        return pItem;
                }
            }
    }

    return nullptr;
}
