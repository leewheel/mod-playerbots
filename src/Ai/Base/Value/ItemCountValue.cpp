/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ItemCountValue.h"
#include "Bag.h"
#include "PlayerbotAI.h"
#include "Player.h"

std::vector<Item*> InventoryItemValueBase::Find(std::string const qualifier)
{
    std::vector<Item*> result;

    std::vector<Item*> items = InventoryAction::parseItems(qualifier);
    for (Item* item : items)
        result.push_back(item);

    return result;
}

uint32 ItemCountValue::Calculate()
{
    uint32 count = 0;
    std::vector<Item*> items = Find(qualifier);
    for (Item* item : items)
        count += item->GetCount();

    return count;
}

std::vector<Item*> InventoryItemValue::Calculate() { return Find(qualifier); }

// By leewheel 2026-09-04
// 防悬空过滤器实现：收集 bot 当前所有随身物品（背包+装备+银行）指针进集合，
// 候选指针与集合按值比对（不解引用候选），不在集合中的即为已销毁/已移动的悬空指针。
// End By leewheel
std::vector<Item*> InventoryItemValueBase::FilterLive(Player* bot, std::vector<Item*> const& candidates)
{
    if (candidates.empty() || !bot)
        return {};

    std::set<Item*> live;
    // 主背包格与钥匙串
    for (uint32 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
        if (Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            live.insert(item);
    for (uint32 i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; ++i)
        if (Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            live.insert(item);
    // 背包容器及其内格
    for (uint32 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
        if (Bag* bag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            live.insert(bag);
            for (uint32 j = 0; j < bag->GetBagSize(); ++j)
                if (Item* item = bag->GetItemByPos(j))
                    live.insert(item);
        }
    // 装备栏
    for (uint32 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
        if (Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            live.insert(item);
    // 银行（缓存来源 ITERATE_ALL_ITEMS 可能包含银行物品）
    for (uint32 i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; ++i)
        if (Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            live.insert(item);
    for (uint32 i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
        if (Bag* bag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            live.insert(bag);
            for (uint32 j = 0; j < bag->GetBagSize(); ++j)
                if (Item* item = bag->GetItemByPos(j))
                    live.insert(item);
        }

    std::vector<Item*> result;
    result.reserve(candidates.size());
    for (Item* candidate : candidates)
        if (candidate && live.count(candidate))
            result.push_back(candidate);

    return result;
}
