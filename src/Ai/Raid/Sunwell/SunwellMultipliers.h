/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SUNWELLMULTIPLIERS_H
#define PLAYERBOTS_SUNWELLMULTIPLIERS_H

#include "Multiplier.h"

class KalecgosStayOnFlanksMultiplier : public Multiplier
{
public:
    KalecgosStayOnFlanksMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kalecgos stay on flanks multiplier") {}
    float GetValue(Action* action) override;
};

class KalecgosSpectralRealmFocusMultiplier : public Multiplier
{
public:
    KalecgosSpectralRealmFocusMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kalecgos spectral realm focus multiplier") {}
    float GetValue(Action* action) override;
};

#endif
