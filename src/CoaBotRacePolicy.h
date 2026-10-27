/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef COA_BOT_RACE_POLICY_H
#define COA_BOT_RACE_POLICY_H

#include <algorithm>
#include <cstdint>
#include <vector>

inline bool IsCoaBotRaceAllowed(std::uint32_t race, bool customRacesEnabled,
                               std::vector<std::uint32_t> const& excludedRaces)
{
    bool const stockRace = race >= 1 && race <= 11 && race != 9;
    return (stockRace || customRacesEnabled) &&
           std::find(excludedRaces.begin(), excludedRaces.end(), race) == excludedRaces.end();
}

#endif
