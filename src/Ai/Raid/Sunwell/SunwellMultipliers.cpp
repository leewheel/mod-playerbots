/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SunwellMultipliers.h"
#include "ChooseTargetActions.h"
#include "Creature.h"
#include "MovementActions.h"
#include "Playerbots.h"
#include "SunwellHelpers.h"

using namespace SunwellHelpers;

// Kalecgos

float KalecgosStayOnFlanksMultiplier::GetValue(Action* action)
{
    if (IsInSpectralRealm(bot))
        return 1.0f;

    Creature* kalecgos = GetKalecgos(bot);
    if (!kalecgos || IsBanished(kalecgos) || IsKalecgosTank(bot, kalecgos))
        return 1.0f;

    // "set behind" walked the melee into Tail Lash, 30 yd behind him (ladiabale, Kalecgos 22/10).
    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 0.0f;

    // The CoA healer tether pulls toward the tank, into Frost Breath: the flank is within heal range anyway.
    if (action->getName() == "coa stay near tank")
        return 0.0f;

    return 1.0f;
}

float KalecgosSpectralRealmFocusMultiplier::GetValue(Action* action)
{
    if (!IsInSpectralRealm(bot) || botAI->IsHeal(bot))
        return 1.0f;

    Creature* sathrovarr = GetSathrovarr(bot);
    if (!sathrovarr || !sathrovarr->IsAlive() || IsBanished(sathrovarr))
        return 1.0f;

    // In the realm only Sathrovarr counts: the target pickers would hand Kalecgos back
    // (ladiabale, Kalecgos 22/10).
    if (dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<TankAssistAction*>(action) ||
        dynamic_cast<DpsAoeAction*>(action) || dynamic_cast<AttackRtiTargetAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}
