/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SUNWELLACTIONS_H
#define PLAYERBOTS_SUNWELLACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

class KalecgosEnterSpectralRealmAction : public MovementAction
{
public:
    KalecgosEnterSpectralRealmAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "kalecgos enter spectral realm") {}
    bool Execute(Event event) override;
};

class KalecgosAttackSathrovarrAction : public AttackAction
{
public:
    KalecgosAttackSathrovarrAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "kalecgos attack sathrovarr") {}
    bool Execute(Event event) override;
};

class KalecgosMoveToFlankAction : public MovementAction
{
public:
    KalecgosMoveToFlankAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "kalecgos move to flank") {}
    bool Execute(Event event) override;
};

#endif
