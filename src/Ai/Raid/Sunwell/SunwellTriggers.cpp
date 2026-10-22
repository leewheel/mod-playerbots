/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SunwellTriggers.h"
#include "Creature.h"
#include "GameObject.h"
#include "Playerbots.h"
#include "SunwellHelpers.h"
#include <cmath>

using namespace SunwellHelpers;

// Kalecgos

bool KalecgosSpectralRealmRotationTrigger::IsActive()
{
    // The rift was never used: bots only reached the realm through Spectral Blast (ladiabale, Kalecgos 22/10).
    Creature* kalecgos = GetKalecgos(bot);
    return kalecgos && GetSpectralRift(bot) && ShouldEnterSpectralRealm(bot, kalecgos);
}

bool KalecgosBotInSpectralRealmTrigger::IsActive()
{
    if (!IsInSpectralRealm(bot) || botAI->IsHeal(bot))
        return false;

    // Sathrovarr is invincible and banished at 1%: hitting him past that only wastes the realm's time.
    Creature* sathrovarr = GetSathrovarr(bot);
    return sathrovarr && sathrovarr->IsAlive() && !IsBanished(sathrovarr) && bot->GetVictim() != sathrovarr;
}

bool KalecgosBotOutsideFlanksTrigger::IsActive()
{
    if (IsInSpectralRealm(bot))
        return false;

    Creature* kalecgos = GetKalecgos(bot);
    if (!kalecgos || IsBanished(kalecgos) || !kalecgos->GetVictim() || IsKalecgosTank(bot, kalecgos))
        return false;

    // Frost Breath covers 30 yd in front and Tail Lash 30 yd behind; the flanks are clear. 40 degrees of slack
    // on each side of the flank so a turning dragon does not make the raid dance (ladiabale, Kalecgos 22/10).
    float const relative = kalecgos->GetRelativeAngle(bot);
    float const flank = relative < static_cast<float>(M_PI) ? static_cast<float>(M_PI / 2) :
                                                              static_cast<float>(3 * M_PI / 2);
    constexpr float slack = static_cast<float>(40.0 * M_PI / 180.0);
    return std::fabs(relative - flank) > slack;
}
