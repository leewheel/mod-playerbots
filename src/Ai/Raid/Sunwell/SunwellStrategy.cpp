/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SunwellStrategy.h"
#include "SunwellMultipliers.h"

void RaidSunwellPlateauStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Kalecgos
    triggers.push_back(new TriggerNode("kalecgos spectral realm rotation", {
        NextAction("kalecgos enter spectral realm", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("kalecgos bot in spectral realm", {
        NextAction("kalecgos attack sathrovarr", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("kalecgos bot outside flanks", {
        NextAction("kalecgos move to flank", ACTION_RAID + 2) }));
}

void RaidSunwellPlateauStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new KalecgosStayOnFlanksMultiplier(botAI));
    multipliers.push_back(new KalecgosSpectralRealmFocusMultiplier(botAI));
}
