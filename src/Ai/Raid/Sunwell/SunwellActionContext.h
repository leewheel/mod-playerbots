/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SUNWELLACTIONCONTEXT_H
#define PLAYERBOTS_SUNWELLACTIONCONTEXT_H

#include "NamedObjectContext.h"
#include "SunwellActions.h"

class RaidSunwellPlateauActionContext : public NamedObjectContext<Action>
{
public:
    RaidSunwellPlateauActionContext()
    {
        // Kalecgos
        creators["kalecgos enter spectral realm"] =
            &RaidSunwellPlateauActionContext::kalecgos_enter_spectral_realm;

        creators["kalecgos attack sathrovarr"] =
            &RaidSunwellPlateauActionContext::kalecgos_attack_sathrovarr;

        creators["kalecgos move to flank"] =
            &RaidSunwellPlateauActionContext::kalecgos_move_to_flank;
    }

private:
    // Kalecgos
    static Action* kalecgos_enter_spectral_realm(PlayerbotAI* botAI) {
        return new KalecgosEnterSpectralRealmAction(botAI);
    }

    static Action* kalecgos_attack_sathrovarr(PlayerbotAI* botAI) {
        return new KalecgosAttackSathrovarrAction(botAI);
    }

    static Action* kalecgos_move_to_flank(PlayerbotAI* botAI) {
        return new KalecgosMoveToFlankAction(botAI);
    }
};

#endif
