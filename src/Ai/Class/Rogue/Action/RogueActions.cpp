/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RogueActions.h"
#include "ItemCountValue.h"
#include "Event.h"
#include "ObjectGuid.h"
#include "Player.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"

namespace
{
constexpr uint32 BG_WS_SPELL_WARSONG_FLAG = 23333;
constexpr uint32 BG_WS_SPELL_SILVERWING_FLAG = 23335;
constexpr uint32 BG_EY_NETHERSTORM_FLAG_SPELL = 34976;
constexpr uint32 SPELL_MASTER_POISONER_RANK_3 = 58410;
}

bool CastStealthAction::isUseful()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (target && bot->GetDistance(target) >= sPlayerbotAIConfig.spellDistance)
        return false;
    return true;
}

bool CastStealthAction::isPossible()
{
    // do not use with WSG flag or EYE flag
    return !bot->HasAura(BG_WS_SPELL_WARSONG_FLAG) && !bot->HasAura(BG_WS_SPELL_SILVERWING_FLAG) &&
           !bot->HasAura(BG_EY_NETHERSTORM_FLAG_SPELL);
}

bool UnstealthAction::Execute(Event /*event*/)
{
    botAI->RemoveAura("stealth");
    // botAI->ChangeStrategy("+dps,-stealthed", BOT_STATE_COMBAT);

    return true;
}

bool CheckStealthAction::Execute(Event /*event*/)
{
    if (botAI->HasAura("stealth", bot))
    {
        botAI->ChangeStrategy("-dps,+stealthed", BOT_STATE_COMBAT);
    }
    else
    {
        botAI->ChangeStrategy("+dps,-stealthed", BOT_STATE_COMBAT);
    }

    return true;
}

bool CastVanishAction::isUseful()
{
    // do not use with WSG flag or EYE flag
    return !bot->HasAura(BG_WS_SPELL_WARSONG_FLAG) && !bot->HasAura(BG_WS_SPELL_SILVERWING_FLAG) &&
           !bot->HasAura(BG_EY_NETHERSTORM_FLAG_SPELL);
}

bool CastEnvenomAction::isUseful()
{
    return AI_VALUE2(uint8, "energy", "self target") >= 35;
}

bool CastEnvenomAction::isPossible()
{
    // alternate to eviscerate if talents unlearned
    return bot->HasAura(SPELL_MASTER_POISONER_RANK_3);
}

bool CastTricksOfTheTradeOnMainTankAction::isUseful()
{
    return CastSpellAction::isUseful() && AI_VALUE2(float, "distance", GetTargetName()) < 20.0f;
}

bool UseDeadlyPoisonAction::Execute(Event /*event*/)
{
    // By leewheel 2026-09-04 合并brighton-chi/the-lab: 上游重写毒药使用逻辑, 去掉旧的后缀循环,
    // 直接按名称取一次"Deadly Poison"(库存物品值查询本身按名称前后缀匹配), 并新增消耗品类过滤。
    // 上游注释译: 该检查仅在按名称匹配时才有必要, 唯一冲突项是"致命毒药手册V"(书本物品),
    // 可能被某些途径恢复进背包, 保险起见保留此过滤。
    std::vector<Item*> items =
        AI_VALUE2(std::vector<Item*>, "inventory items", "Deadly Poison");
    // By leewheel 2026-09-04 防悬空崩溃: 过滤缓存列表中已失效的物品指针, 防止访问已释放内存
    items = InventoryItemValueBase::FilterLive(bot, items);
    // End By leewheel
    for (Item* const item : items)
    {
        if (item->GetTemplate()->Class != ITEM_CLASS_CONSUMABLE)
            continue;

        Item* const itemForSpell = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
        return UseItem(item, ObjectGuid::Empty, itemForSpell);
    }

    return false;
}

bool UseInstantPoisonAction::Execute(Event /*event*/)
{
    // By leewheel 2026-09-04 合并brighton-chi/the-lab: 上游重写毒药使用逻辑(同致命毒药, 见上),
    // 保留本分支 FilterLive 防悬空崩溃过滤。
    std::vector<Item*> items =
        AI_VALUE2(std::vector<Item*>, "inventory items", "Instant Poison");
    // By leewheel 2026-09-04 防悬空崩溃: 过滤缓存列表中已失效的物品指针, 防止访问已释放内存
    items = InventoryItemValueBase::FilterLive(bot, items);
    // End By leewheel
    for (Item* const item : items)
    {
        // 上游注释译: 速效毒药本身无需此检查, 为对称性保留以引导潜在重构。
        if (item->GetTemplate()->Class != ITEM_CLASS_CONSUMABLE)
            continue;

        Item* const itemForSpell = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
        return UseItem(item, ObjectGuid::Empty, itemForSpell);
    }

    return false;
}

bool UseInstantPoisonOffHandAction::Execute(Event /*event*/)
{
    // By leewheel 2026-09-04 合并brighton-chi/the-lab: 上游重写毒药使用逻辑(同致命毒药, 见上),
    // 保留本分支 FilterLive 防悬空崩溃过滤。
    std::vector<Item*> items =
        AI_VALUE2(std::vector<Item*>, "inventory items", "Instant Poison");
    // By leewheel 2026-09-04 防悬空崩溃: 过滤缓存列表中已失效的物品指针, 防止访问已释放内存
    items = InventoryItemValueBase::FilterLive(bot, items);
    // End By leewheel
    for (Item* const item : items)
    {
        // 上游注释译: 速效毒药本身无需此检查, 为对称性保留以引导潜在重构。
        if (item->GetTemplate()->Class != ITEM_CLASS_CONSUMABLE)
            continue;

        Item* const itemForSpell = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
        return UseItem(item, ObjectGuid::Empty, itemForSpell);
    }

    return false;
}
