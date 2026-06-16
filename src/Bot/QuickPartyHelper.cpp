/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "QuickPartyHelper.h"

#include "Item.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotFactory.h"
#include "Playerbots.h"
#include "SharedDefines.h"

uint32 QuickPartyHelper::GearQualityForLevel(uint32 level)
{
    return level >= 60 ? ITEM_QUALITY_EPIC : ITEM_QUALITY_RARE;
}

void QuickPartyHelper::StripHeirloomItems(Player* player)
{
    for (uint8 slot = EQUIPMENT_SLOT_START; slot < INVENTORY_SLOT_ITEM_END; ++slot)
    {
        if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
        {
            if (ItemTemplate const* proto = item->GetTemplate())
            {
                if (proto->Quality == ITEM_QUALITY_HEIRLOOM)
                    player->DestroyItem(INVENTORY_SLOT_BAG_0, slot, true);
            }
        }
    }

    for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; ++bag)
    {
        if (Bag* pBag = player->GetBagByPos(bag))
        {
            for (uint32 slot = 0; slot < pBag->GetBagSize(); ++slot)
            {
                if (Item* item = pBag->GetItemByPos(slot))
                {
                    if (ItemTemplate const* proto = item->GetTemplate())
                    {
                        if (proto->Quality == ITEM_QUALITY_HEIRLOOM)
                            player->DestroyItem(bag, slot, true);
                    }
                }
            }
        }
    }
}

std::vector<std::string> QuickPartyHelper::RandomGear(Player* invoker, Player* target)
{
    std::vector<std::string> messages;

    if (!target)
        target = invoker;

    if (target != invoker && !invoker->CanBeGameMaster() && !GET_PLAYERBOT_AI(target))
    {
        messages.push_back("只能对自己或机器人使用随机装备。");
        return messages;
    }

    if (target->isDead())
        target->ResurrectPlayer(1.0f, false);

    StripHeirloomItems(target);

    uint32 level = target->GetLevel();
    uint32 quality = GearQualityForLevel(level);

    PlayerbotFactory factory(target, level, quality);
    factory.SetExcludeHeirloom(true);
    factory.InitTalentsTree(true, true, true);
    factory.InitEquipment(true);
    factory.InitBags(true);
    factory.InitAmmo();
    if (level >= sPlayerbotAIConfig.minEnchantingBotLevel)
        factory.ApplyEnchantAndGemsNew();
    target->DurabilityRepairAll(false, 1.0f, false);

    messages.push_back("已为 " + target->GetName() + " 随机装备（含36格背包，不含传家宝）。");
    return messages;
}
