/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SunwellHelpers.h"
#include "Creature.h"
#include "GameObject.h"
#include "Group.h"
#include "Playerbots.h"
#include "SpellAuras.h"
#include <algorithm>
#include <vector>

namespace SunwellHelpers
{

namespace
{

enum class RealmRole : uint8
{
    Tank,
    Heal,
    Dps,
};

RealmRole GetRealmRole(Player* member)
{
    if (PlayerbotAI::IsTank(member))
        return RealmRole::Tank;

    if (PlayerbotAI::IsHeal(member))
        return RealmRole::Heal;

    return RealmRole::Dps;
}

// Alive, outside the realm and free of Spectral Exhaustion: the rift refuses an exhausted player (44811 CheckCast).
bool CanUseRift(Player* member)
{
    return member && member->IsAlive() && !IsInSpectralRealm(member) &&
           !member->HasAura(static_cast<uint32>(KalecgosSpells::SPELL_SPECTRAL_EXHAUSTION));
}

}

Creature* GetKalecgos(Player* bot)
{
    if (bot->GetMapId() != SUNWELL_MAP_ID)
        return nullptr;

    Creature* kalecgos = bot->FindNearestCreature(static_cast<uint32>(KalecgosNpcs::NPC_KALECGOS), 150.0f);
    return kalecgos && kalecgos->IsInCombat() ? kalecgos : nullptr;
}

Creature* GetSathrovarr(Player* bot)
{
    if (bot->GetMapId() != SUNWELL_MAP_ID)
        return nullptr;

    return bot->FindNearestCreature(static_cast<uint32>(KalecgosNpcs::NPC_SATHROVARR), 150.0f);
}

GameObject* GetSpectralRift(Player* bot)
{
    // A rift opens where Spectral Blast lands and despawns after 15 s (SAI): 60 yd is reachable in that time.
    return bot->FindNearestGameObject(static_cast<uint32>(KalecgosObjects::GO_SPECTRAL_RIFT), 60.0f, true);
}

bool IsInSpectralRealm(Unit* unit)
{
    return unit && unit->HasAura(static_cast<uint32>(KalecgosSpells::SPELL_SPECTRAL_REALM));
}

bool IsBanished(Unit* unit)
{
    return unit && unit->HasAura(static_cast<uint32>(KalecgosSpells::SPELL_BANISH));
}

uint8 GetArcaneBuffetStacks(Unit* unit)
{
    Aura* buffet = unit ? unit->GetAura(static_cast<uint32>(KalecgosSpells::SPELL_ARCANE_BUFFET)) : nullptr;
    return buffet ? buffet->GetStackAmount() : 0;
}

bool IsKalecgosTank(Player* member, Creature* kalecgos)
{
    return PlayerbotAI::IsMainTank(member) || (kalecgos && kalecgos->GetVictim() == member);
}

bool ShouldEnterSpectralRealm(Player* bot, Creature* kalecgos)
{
    if (!kalecgos || !CanUseRift(bot))
        return false;

    Creature* sathrovarr = GetSathrovarr(bot);
    if (!sathrovarr || !sathrovarr->IsAlive() || IsBanished(sathrovarr))
        return false;

    // Kalecgos waits banished at 1% until Sathrovarr is down to 1% too: everyone who can still go finishes him.
    if (IsBanished(kalecgos))
        return true;

    // Never the dragon's tank: Spectral Blast never takes him either, and the dragon would turn on the raid
    // (ladiabale, Kalecgos 22/10).
    if (IsKalecgosTank(bot, kalecgos))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    RealmRole const role = GetRealmRole(bot);
    bool const largeRaid = group->GetMembersCount() > 10;
    uint8 const highStacks = 15;

    uint8 inRealm[3] = {0, 0, 0};
    bool offTankExists = false;
    std::vector<std::pair<uint8, Player*>> candidates;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->GetMapId() != bot->GetMapId())
            continue;

        RealmRole const memberRole = GetRealmRole(member);
        if (IsInSpectralRealm(member))
        {
            ++inRealm[static_cast<uint8>(memberRole)];
            continue;
        }

        if (IsKalecgosTank(member, kalecgos))
            continue;

        if (memberRole == RealmRole::Tank)
            offTankExists = true;

        // Only bots are ranked: a player who does not follow the rotation must not hold a slot.
        if (memberRole == role && CanUseRift(member) && GET_PLAYERBOT_AI(member))
            candidates.emplace_back(GetArcaneBuffetStacks(member), member);
    }

    uint8 const tanksIn = inRealm[static_cast<uint8>(RealmRole::Tank)];
    uint8 quota = 1;
    if (role == RealmRole::Heal)
        quota = largeRaid ? 2 : 1;
    else if (role == RealmRole::Dps)
        quota = largeRaid ? 5 : 3;

    // Arcane Buffet stacks only fall off in the realm and nobody went on purpose: 14.8k per buffet in the end. A bot
    // with high stacks gets extra room (ladiabale, Kalecgos 22/10).
    if (role != RealmRole::Tank && GetArcaneBuffetStacks(bot) >= highStacks)
        quota += 2;

    // The realm group waits for its tank, else Sathrovarr's Corruption Strike lands on a healer or a dps.
    if (role != RealmRole::Tank && offTankExists && tanksIn == 0)
        return false;

    uint8 const alreadyIn = inRealm[static_cast<uint8>(role)];
    if (alreadyIn >= quota)
        return false;

    // Highest stacks first, then group order, so the same bots do not go every time.
    std::stable_sort(candidates.begin(), candidates.end(),
                     [](auto const& a, auto const& b) { return a.first > b.first; });

    uint8 const freeSlots = quota - alreadyIn;
    for (uint8 i = 0; i < candidates.size() && i < freeSlots; ++i)
    {
        if (candidates[i].second == bot)
            return true;
    }

    return false;
}

}
