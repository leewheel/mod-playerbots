/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SUNWELLTRIGGERS_H
#define PLAYERBOTS_SUNWELLTRIGGERS_H

#include "PlayerbotAI.h"
#include "Trigger.h"

class KalecgosSpectralRealmRotationTrigger : public Trigger
{
public:
    KalecgosSpectralRealmRotationTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kalecgos spectral realm rotation") {}
    bool IsActive() override;
};

class KalecgosBotInSpectralRealmTrigger : public Trigger
{
public:
    KalecgosBotInSpectralRealmTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kalecgos bot in spectral realm") {}
    bool IsActive() override;
};

class KalecgosBotOutsideFlanksTrigger : public Trigger
{
public:
    KalecgosBotOutsideFlanksTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kalecgos bot outside flanks") {}
    bool IsActive() override;
};

#endif
