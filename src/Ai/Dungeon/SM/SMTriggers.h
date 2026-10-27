/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SMTRIGGERS_H
#define PLAYERBOTS_SMTRIGGERS_H

#include "Trigger.h"

class AiObjectContext;
class Unit;

class ArcanistDoanBossCastingDetonationTrigger : public Trigger
{
public:
    ArcanistDoanBossCastingDetonationTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "arcanist doan boss casting detonation") {}
    bool IsActive() override;
};

class ArcanistDoanRangedTrigger : public Trigger
{
public:
    ArcanistDoanRangedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "arcanist doan ranged") {}
    bool IsActive() override;
};

class HerodWhirlwindTrigger : public Trigger
{
public:
    HerodWhirlwindTrigger(PlayerbotAI* botAI) : Trigger(botAI, "herod whirlwind") {}
    bool IsActive() override;
};

class WhitemaneFocusTrigger : public Trigger
{
public:
    WhitemaneFocusTrigger(PlayerbotAI* botAI) : Trigger(botAI, "whitemane focus") {}
    bool IsActive() override;
};

// Herod while his Whirlwind spins.
bool IsWhirlwinding(Unit const* herod);
// Whitemane once she has raised Mograine: she is the one to kill first.
Unit* WhitemaneToFocus(PlayerbotAI* botAI, AiObjectContext* context);

#endif
