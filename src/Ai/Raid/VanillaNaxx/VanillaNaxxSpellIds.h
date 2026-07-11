#ifndef PLAYERBOTS_VANILLANAXXSPELLIDS_H
#define PLAYERBOTS_VANILLANAXXSPELLIDS_H

#include <initializer_list>

#include "PlayerbotAI.h"

// Vanilla Naxx (60-level, 40-man) spell IDs
// Based on naxx40Scripts boss scripts
namespace VanillaNaxxSpellIds
{
    // Heigan
    static constexpr uint32 Eruption40 = 29371;

    // Grobbulus
    static constexpr uint32 PoisonCloud = 28240;

    // Thaddius polarity
    static constexpr uint32 PositiveCharge = 28062;
    static constexpr uint32 PositiveChargeStack = 29659;
    static constexpr uint32 NegativeCharge = 28085;
    static constexpr uint32 NegativeChargeStack = 29660;
    static constexpr uint32 PositivePolarity = 28059;
    static constexpr uint32 NegativePolarity = 28084;

    // Sapphiron
    static constexpr uint32 Icebolt = 28522;
    static constexpr uint32 Chill = 28547;

    // Gluth
    static constexpr uint32 Decimate = 28374;
    static constexpr uint32 MortalWound = 25646;

    // Anub'Rekhan
    static constexpr uint32 LocustSwarm = 28785;

    // Loatheb - Vanilla uses Corrupted Mind instead of Necrotic Aura
    static constexpr uint32 CorruptedMind = 29201;
    static constexpr uint32 NecroticAura = 55593; // kept for fallback

    inline bool HasAnyAura(Unit* unit, std::initializer_list<uint32> spellIds)
    {
        if (!unit)
            return false;

        for (uint32 spellId : spellIds)
        {
            if (unit->HasAura(spellId))
                return true;
        }
        return false;
    }

    inline Aura* GetAnyAura(Unit* unit, std::initializer_list<uint32> spellIds)
    {
        if (!unit)
            return nullptr;

        for (uint32 spellId : spellIds)
        {
            if (Aura* aura = unit->GetAura(spellId))
                return aura;
        }
        return nullptr;
    }

    inline bool MatchesAnySpellId(SpellInfo const* info, std::initializer_list<uint32> spellIds)
    {
        if (!info)
            return false;

        for (uint32 spellId : spellIds)
        {
            if (info->Id == spellId)
                return true;
        }
        return false;
    }
}  // namespace VanillaNaxxSpellIds

#endif
