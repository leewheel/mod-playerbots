/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "HoLMultipliers.h"
#include "Action.h"
#include "ChooseTargetActions.h"
#include "GenericSpellActions.h"
#include "HoLActions.h"
#include "HoLTriggers.h"
#include "MovementActions.h"
#include "WarriorActions.h"

float BjarngrimMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "28586");
    if (!boss || botAI->IsHeal(bot)) { return 1.0f; }

    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_WHIRLWIND_BJARNGRIM))
    {
        if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<AvoidWhirlwindAction*>(action))
        {
            return 0.0f;
        }
    }

    // Detect boss adds this way as sometimes they don't get added to threat table on dps bots,
    // and some dps just stand at range and don't engage the boss at all as they can't find the adds
    // Unit* boss_add = AI_VALUE2(Unit*, "find target", "29240");
    Unit* boss_add = nullptr;
    GuidVector targets = AI_VALUE(GuidVector, "possible targets no los");

    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (unit && unit->GetEntry() == NPC_STORMFORGED_LIEUTENANT)
        {
            boss_add = unit;
            break;
        }
    }

    if (!boss_add || botAI->IsTank(bot)) { return 1.0f; }

    if (dynamic_cast<DpsAssistAction*>(action))
    {
        return 0.0f;
    }

    if (action->getThreatType() == Action::ActionThreatType::Aoe)
    {
        return 0.0f;
    }

    return 1.0f;
}

float VolkhanMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "28587");
    if (!boss || botAI->IsTank(bot) || botAI->IsHeal(bot)) { return 1.0f; }

    if (dynamic_cast<DpsAssistAction*>(action))
    {
        return 0.0f;
    }

    if (action->getThreatType() == Action::ActionThreatType::Aoe)
    {
        return 0.0f;
    }

    return 1.0f;
}

float IonarMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "28546");
    if (!boss) { return 1.0f; }

    // Check if the boss has dispersed into Sparks (not visible).
    if (!bot->CanSeeOrDetect(boss))
    {
        // Block MovementActions except for specific exceptions.
        if (dynamic_cast<MovementAction*>(action)
            && !dynamic_cast<DispersePositionAction*>(action)
            && !dynamic_cast<StaticOverloadSpreadAction*>(action))
        {
            return 0.0f;
        }
    }

    if (boss->FindCurrentSpellBySpellId(SPELL_DISPERSE))
    {
        // Explicitly block the CastChargeAction during dispersal.
        if (dynamic_cast<CastChargeAction*>(action))
        {
            return 0.0f;
        }
    }
    return 1.0f;
}
float LokenMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "28923");
    if (!boss) { return 1.0f; }

    // Prevent FleeAction from being executed.
    if (dynamic_cast<FleeAction*>(action)) { return 0.0f; }

    // Prevent MovementActions during Lightning Nova unless it's AvoidLightningNovaAction.
    if (boss->FindCurrentSpellBySpellId(SPELL_LIGHTNING_NOVA))
    {
        if (dynamic_cast<MovementAction*>(action)
            && !dynamic_cast<AvoidLightningNovaAction*>(action))
        {
            return 0.0f;
        }

        // Specifically prevent Charge during Lightning Nova.
        if (dynamic_cast<CastChargeAction*>(action))
        {
            return 0.0f;
        }
    }

    return 1.0f; // Default multiplier value for other cases.
}
