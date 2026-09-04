/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GuildValues.h"
#include "ItemCountValue.h"
#include "Playerbots.h"

uint8 PetitionSignsValue::Calculate()
{
    if (bot->GetGuildId())
        return 0;

    std::vector<Item*> petitions = AI_VALUE2(std::vector<Item*>, "inventory items", chat->FormatQItem(5863));
    // By leewheel 2026-09-04 防悬空崩溃: 过滤缓存列表中已失效的物品指针
    // End By leewheel
    petitions = InventoryItemValueBase::FilterLive(bot, petitions);
    if (petitions.empty())
        return 0;

    QueryResult result = CharacterDatabase.Query("SELECT playerguid FROM petition_sign WHERE petitionguid = {}",
                                                 petitions.front()->GetGUID().GetCounter());
    return result ? (uint8)result->GetRowCount() : 0;
}
