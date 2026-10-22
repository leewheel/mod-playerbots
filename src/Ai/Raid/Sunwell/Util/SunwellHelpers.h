/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SUNWELLHELPERS_H
#define PLAYERBOTS_SUNWELLHELPERS_H

#include "PlayerbotAI.h"

class Creature;
class GameObject;

namespace SunwellHelpers
{

constexpr uint32 SUNWELL_MAP_ID = 580;

enum class KalecgosSpells : uint32
{
    SPELL_SPECTRAL_EXHAUSTION = 44867,
    SPELL_SPECTRAL_REALM      = 46021,
    SPELL_ARCANE_BUFFET       = 45018,
    SPELL_BANISH              = 44836,
};

enum class KalecgosNpcs : uint32
{
    NPC_KALECGOS   = 24850,
    NPC_KALEC      = 24891,
    NPC_SATHROVARR = 24892,
};

enum class KalecgosObjects : uint32
{
    GO_SPECTRAL_RIFT = 187055,
};

// Kalecgos, engaged. Looked up by entry: the realm bots never have him on their threat list.
Creature* GetKalecgos(Player* bot);
// Sathrovarr, alive, from either realm (he is invisible from outside, not out of reach of a grid search).
Creature* GetSathrovarr(Player* bot);
GameObject* GetSpectralRift(Player* bot);

bool IsInSpectralRealm(Unit* unit);
bool IsBanished(Unit* unit);
uint8 GetArcaneBuffetStacks(Unit* unit);
bool IsKalecgosTank(Player* member, Creature* kalecgos);
bool ShouldEnterSpectralRealm(Player* bot, Creature* kalecgos);

}

#endif
