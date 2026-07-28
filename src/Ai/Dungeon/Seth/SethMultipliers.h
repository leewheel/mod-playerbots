/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SETHMULTIPLIERS_H
#define PLAYERBOTS_SETHMULTIPLIERS_H

#include "Multiplier.h"

class SethekkProphetSetTremorTotemMultiplier : public Multiplier
{
public:
    SethekkProphetSetTremorTotemMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "sethekk prophet set tremor totem") {}
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    float GetValue(Action* action) override;
};

class AnzuControlSpellCastingWithSpellBombMultiplier : public Multiplier
{
public:
    AnzuControlSpellCastingWithSpellBombMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "anzu control spell casting with spell bomb") {}
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    float GetValue(Action* action) override;
};

class TalonKingIkissDelayBloodlustAndHeroismMultiplier : public Multiplier
{
public:
    TalonKingIkissDelayBloodlustAndHeroismMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "talon king ikiss delay bloodlust and heroism") {}
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    float GetValue(Action* action) override;
};

class TalonKingIkissControlMovementMultiplier : public Multiplier
{
public:
    TalonKingIkissControlMovementMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "talon king ikiss control movement") {}
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    float GetValue(Action* action) override;
};

#endif
