/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ItemVisitors.h"
#include "Playerbots.h"

bool FindUsableItemVisitor::Visit(Item* item)
{
    if (bot->CanUseItem(item->GetTemplate()) == EQUIP_ERR_OK)
        return FindItemVisitor::Visit(item);

    return true;
}

bool FindPotionVisitor::Accept(ItemTemplate const* proto)
{
    if (proto->Class == ITEM_CLASS_CONSUMABLE &&
        (proto->SubClass == ITEM_SUBCLASS_POTION || proto->SubClass == ITEM_SUBCLASS_FLASK))
    {
        for (uint8 j = 0; j < MAX_ITEM_PROTO_SPELLS; j++)
        {
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(proto->Spells[j].SpellId);
            if (!spellInfo)
                return false;

            // Not a potion that only works somewhere else: 41618 (Tempest Keep only) was tried every tick and failed
            // 1,740 times for one bot in Gruul's Lair (ladiabale, 18/10).
            if (bot && spellInfo->CheckLocation(bot->GetMapId(), bot->GetZoneId(), bot->GetAreaId(), bot) != SPELL_CAST_OK)
                return false;

            for (uint8 i = 0; i < 3; i++)
            {
                // A mana potion gives mana: a Rage Potion energizes too, and Cultist healers drank
                // them as mana potions (jealous-sound #5077).
                if (spellInfo->Effects[i].Effect == effectId &&
                    (effectId != SPELL_EFFECT_ENERGIZE || spellInfo->Effects[i].MiscValue == POWER_MANA))
                    return true;
            }
        }
    }

    return false;
}

bool FindMountVisitor::Accept(ItemTemplate const* proto)
{
    for (uint8 j = 0; j < MAX_ITEM_PROTO_SPELLS; j++)
    {
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(proto->Spells[j].SpellId);
        if (!spellInfo)
            return false;

        for (uint8 i = 0; i < 3; i++)
        {
            if (spellInfo->Effects[i].ApplyAuraName == SPELL_AURA_MOUNTED)
                return true;
        }
    }

    return false;
}

bool FindPetVisitor::Accept(ItemTemplate const* proto)
{
    if (proto->Class == ITEM_CLASS_MISC)
    {
        for (uint8 j = 0; j < MAX_ITEM_PROTO_SPELLS; j++)
        {
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(proto->Spells[j].SpellId);
            if (!spellInfo)
                return false;

            for (uint8 i = 0; i < 3; i++)
            {
                if (spellInfo->Effects[i].Effect == SPELL_EFFECT_SUMMON_PET)
                    return true;
            }
        }
    }

    return false;
}

FindItemUsageVisitor::FindItemUsageVisitor(Player* bot, ItemUsage usage) : FindUsableItemVisitor(bot), usage(usage)
{
    context = GET_PLAYERBOT_AI(bot)->GetAiObjectContext();
};

bool FindItemUsageVisitor::Accept(ItemTemplate const* proto)
{
    if (AI_VALUE2(ItemUsage, "item usage", proto->ItemId) == usage)
        return true;

    return false;
}

bool FindUsableNamedItemVisitor::Accept(ItemTemplate const* proto)
{
    return proto && !proto->Name1.empty() && strstri(proto->Name1.c_str(), name.c_str());
}
