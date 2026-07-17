/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GENERICACTIONS_H
#define PLAYERBOTS_GENERICACTIONS_H

#include "AttackAction.h"
#include "Action.h"
#include "PlayerbotAI.h"

class PlayerbotAI;

class MeleeAction : public AttackAction
{
public:
    MeleeAction(PlayerbotAI* botAI) : AttackAction(botAI, "melee") {}

    std::string const GetTargetName() override { return "current target"; }
    bool isUseful() override;
};

class TogglePetSpellAutoCastAction : public Action
{
public:
    TogglePetSpellAutoCastAction(PlayerbotAI* ai) : Action(ai, "toggle pet spell") {}
    virtual bool Execute(Event event) override;
};

class PetAttackAction : public Action
{
public:
    PetAttackAction(PlayerbotAI* ai) : Action(ai, "pet attack") {}
    virtual bool Execute(Event event) override;
};

class SetPetStanceAction : public Action
{
public:
    SetPetStanceAction(PlayerbotAI* botAI) : Action(botAI, "set pet stance") {}

    bool Execute(Event event) override;
};

// By leewheel 2026-07-15: When a party member is sapped by stealthed mobs, use AoE to break stealth.
// This action tries class-appropriate instant-cast or short-cast AoE spells to flush out stealthed assassins.
class BreakStealthAction : public Action
{
public:
    BreakStealthAction(PlayerbotAI* botAI) : Action(botAI, "break stealth") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

// End By leewheel

#endif
