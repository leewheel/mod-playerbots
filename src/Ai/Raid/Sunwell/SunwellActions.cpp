/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SunwellActions.h"
#include "Creature.h"
#include "GameObject.h"
#include "Playerbots.h"
#include "SunwellHelpers.h"
#include <algorithm>
#include <cmath>

using namespace SunwellHelpers;

// Kalecgos

bool KalecgosEnterSpectralRealmAction::Execute(Event /*event*/)
{
    GameObject* rift = GetSpectralRift(bot);
    if (!rift)
        return false;

    if (bot->GetExactDist2d(rift) > 3.0f)
        return MoveTo(rift, 1.0f, MovementPriority::MOVEMENT_FORCED);

    // The rift is a goober casting 44811, whose script teleports the user in (Spectral Exhaustion checked there too).
    // GameObject::Use, as for Magtheridon's cubes: the rift has no range check of its own.
    bot->StopMoving();
    bot->CastStop();
    rift->Use(bot);
    return true;
}

bool KalecgosAttackSathrovarrAction::Execute(Event /*event*/)
{
    Creature* sathrovarr = GetSathrovarr(bot);
    if (!sathrovarr || !IsInSpectralRealm(bot))
        return false;

    // Bots sent through by Spectral Blast kept Kalecgos as their target and did nothing in the realm
    // (ladiabale, Kalecgos 22/10).
    return Attack(sathrovarr);
}

bool KalecgosMoveToFlankAction::Execute(Event /*event*/)
{
    Creature* kalecgos = GetKalecgos(bot);
    if (!kalecgos)
        return false;

    // The nearer flank, 90 degrees from his facing. "set behind" put the melee in the Tail Lash cone
    // (ladiabale, Kalecgos 22/10).
    float const relative = kalecgos->GetRelativeAngle(bot);
    float const flank = relative < static_cast<float>(M_PI) ? static_cast<float>(M_PI / 2) :
                                                              static_cast<float>(3 * M_PI / 2);
    float const angle = kalecgos->ToAbsoluteAngle(flank);

    float const distance = botAI->IsRanged(bot) ? kalecgos->GetCombatReach() + 18.0f :
                                                  std::max(2.0f, bot->GetMeleeRange(kalecgos) - 1.5f);

    float const x = kalecgos->GetPositionX() + std::cos(angle) * distance;
    float const y = kalecgos->GetPositionY() + std::sin(angle) * distance;

    return MoveTo(bot->GetMapId(), x, y, bot->GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT);
}
