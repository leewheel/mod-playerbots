/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ITEMCOUNTVALUE_H
#define PLAYERBOTS_ITEMCOUNTVALUE_H

#include "InventoryAction.h"
#include "Item.h"
#include "NamedObjectContext.h"

class PlayerbotAI;

class InventoryItemValueBase : public InventoryAction
{
public:
    InventoryItemValueBase(PlayerbotAI* botAI) : InventoryAction(botAI, "empty") {}

    bool Execute(Event /*event*/) override { return false; }

protected:
    std::vector<Item*> Find(std::string const qualifier);
};

class ItemCountValue : public Uint32CalculatedValue, public Qualified, InventoryItemValueBase
{
public:
    ItemCountValue(PlayerbotAI* botAI, std::string const name = "inventory items")
        : Uint32CalculatedValue(botAI, name), InventoryItemValueBase(botAI)
    {
    }

    uint32 Calculate() override;
};

class InventoryItemValue : public CalculatedValue<std::vector<Item*>>, public Qualified, InventoryItemValueBase
{
public:
    // By leewheel 2026-08-19
    // 性能优化：checkInterval 由默认 1（每 tick 重算）改为 1000ms。
    // Calculate() 会遍历整个背包构造物品列表，每 tick 每 bot 执行开销大；
    // 背包内容 1 秒刷新完全满足业务需求（RPG/物品使用/商人判断等）。
    InventoryItemValue(PlayerbotAI* botAI)
        : CalculatedValue<std::vector<Item*>>(botAI, "inventory items", 1000), InventoryItemValueBase(botAI)
    {
    }
    // End By leewheel

    std::vector<Item*> Calculate() override;
};

#endif
