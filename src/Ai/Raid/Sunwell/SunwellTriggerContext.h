/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SUNWELLTRIGGERCONTEXT_H
#define PLAYERBOTS_SUNWELLTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "SunwellTriggers.h"

class RaidSunwellPlateauTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidSunwellPlateauTriggerContext() : NamedObjectContext<Trigger>()
    {
        // Kalecgos
        creators["kalecgos spectral realm rotation"] =
            &RaidSunwellPlateauTriggerContext::kalecgos_spectral_realm_rotation;

        creators["kalecgos bot in spectral realm"] =
            &RaidSunwellPlateauTriggerContext::kalecgos_bot_in_spectral_realm;

        creators["kalecgos bot outside flanks"] =
            &RaidSunwellPlateauTriggerContext::kalecgos_bot_outside_flanks;
    }

private:
    // Kalecgos
    static Trigger* kalecgos_spectral_realm_rotation(PlayerbotAI* botAI) {
        return new KalecgosSpectralRealmRotationTrigger(botAI);
    }

    static Trigger* kalecgos_bot_in_spectral_realm(PlayerbotAI* botAI) {
        return new KalecgosBotInSpectralRealmTrigger(botAI);
    }

    static Trigger* kalecgos_bot_outside_flanks(PlayerbotAI* botAI) {
        return new KalecgosBotOutsideFlanksTrigger(botAI);
    }
};

#endif
