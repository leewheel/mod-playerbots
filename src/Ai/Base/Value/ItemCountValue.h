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

    // By leewheel 2026-09-04
    // 防悬空过滤器：过滤掉"inventory items"等缓存列表中已失效的裸指针。
    //   背景（玩家崩溃日志 2026-09-04）：InventoryItemValue 缓存 1000ms，窗口内物品
    //   被用掉/堆叠合并/交易后销毁，缓存中的 Item* 悬空，消费方取用即 C0000005
    //   （栈：CanUseItem→Item::GetTemplate→Object::GetUInt32Value）。
    //   实现：遍历 bot 当前背包/装备建立"活指针集合"，候选指针按值比对过滤——
    //   全程不解引用候选指针，悬空指针也不会引发访问违例。
    // End By leewheel
    static std::vector<Item*> FilterLive(Player* bot, std::vector<Item*> const& candidates);

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
