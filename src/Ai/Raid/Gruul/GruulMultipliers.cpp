/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GruulMultipliers.h"
#include "ChooseTargetActions.h"
#include "EncounterHelpers.h"
#include "GruulActions.h"
#include "GruulHelpers.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"
#include "ShamanActions.h"

using namespace GruulHelpers;
using namespace EncounterHelpers;

float GruulsLairDelayDpsCooldownsMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!IsDpsCooldownAction(bot, action))
        return 1.0f;

    Unit* gruul = AI_VALUE2(Unit*, "find target", "19044");
    if (gruul && gruul->GetHealthPct() > 95.0f)
        return 0.0f;

    // By leewheel 2026-08-29 合并：采用对侧新逻辑(优先杀Blindeye治疗，与AssignDpsPriority一致)，entry规则查找
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "18831");
    return blindeye && blindeye->GetHealthPct() > BLINDEYE_PULL_COMPLETE_HP_PERCENT ? 0.0f : 1.0f;
    // End By leewheel
}

float HighKingMaulgarControlTankActionsMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!dynamic_cast<TankAssistAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action))
    {
        return 1.0f;
    }

    return AI_VALUE2(Unit*, "find target", "18831") ? 0.0f : 1.0f;
}

float HighKingMaulgarRestrictTauntingMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    bool const isAoeThreat = IsAoeThreatAction(bot, action);
    if (!isAoeThreat && !IsTauntAction(bot, action))
        return 1.0f;

    // By leewheel 2026-08-29 合并：采用对侧新增逻辑(主坦克放行/Blindeye在场禁AoE威胁)，entry规则查找
    // The main tank stays on Maulgar the whole time so it can do whatever.
    if (PlayerbotAI::IsMainTank(bot))
        return 1.0f;

    // Blindeye and Olm are tanked next to each other by separate tanks; until Blindeye is dead,
    // don't use AoE threat abilities.
    if (isAoeThreat && AI_VALUE2(Unit*, "find target", "18831"))
        return 0.0f;

    // Kiggler is the only ogre for which taunting is a problem because he is the only one that is
    // both (1) tanked by a non-traditional-tank and (2) directed to be attacked by traditional
    // tanks (the Blindeye and Olm tanks after both are down).
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "18835");
    // End By leewheel
    if (!kiggler)
        return 1.0f;

    if (!GetKigglerMoonkinTank(bot))
        return 1.0f;

    return AI_VALUE(Unit*, "current target") == kiggler ? 0.0f : 1.0f;
}

float HighKingMaulgarDisableDpsAssistMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!dynamic_cast<DpsAssistAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "18831") ? 0.0f : 1.0f;
}

float HighKingMaulgarAvoidWhirlwindMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<HighKingMaulgarRunAwayFromWhirlwindAction*>(action))
        return 1.0f;

    Unit* maulgar = AI_VALUE2(Unit*, "find target", "18831");
    if (!maulgar || !maulgar->HasAura(Id(GruulSpells::SPELL_WHIRLWIND)))
        return 1.0f;

    if (PlayerbotAI::IsMainTank(bot))
        return 1.0f;

    return bot->GetDistance2d(maulgar) < WHIRLWIND_SAFE_DISTANCE + 5.0f ? 0.0f : 1.0f;
}

float HighKingMaulgarControlHunterActionsMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    bool const isMainTankMisdirect = dynamic_cast<CastMisdirectionOnMainTankAction*>(action);
    if (!isMainTankMisdirect && !dynamic_cast<CastArcaneShotAction*>(action))
        return 1.0f;

    // Krosh/Kiggler will be the last to die before Maulgar
    // When only Maulgar is left, the standard Misdirection strategy is fine
    if (isMainTankMisdirect &&
        ((AI_VALUE2(Unit*, "find target", "18832")) ||
         (AI_VALUE2(Unit*, "find target", "18835"))))
    {
        return 0.0f;
    }

    // Arcane Shot removes Spell Shield, which the mage tank needs to survive
    Unit* krosh = AI_VALUE2(Unit*, "find target", "18832");
    return krosh && action->GetTarget() == krosh ? 0.0f : 1.0f;
}

float HighKingMaulgarControlMageTankActionsMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_MAGE)
        return 1.0f;

    if (action->getThreatType() != Action::ActionThreatType::Aoe &&
        !dynamic_cast<CastIceBlockAction*>(action) &&
        !dynamic_cast<CastInvisibilityAction*>(action))
    {
        return 1.0f;
    }

    if (!AI_VALUE2(Unit*, "find target", "18832"))
        return 1.0f;

    return GetKroshMageTank(bot) == bot ? 0.0f : 1.0f;
}

float GruulTheDragonkillerControlTankMovementMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<AvoidAoeAction*>(action))
    {
        return 1.0f;
    }

    // By leewheel 2026-08-29 合并：采用对侧单行写法，entry规则查找
    Unit* gruul = AI_VALUE2(Unit*, "find target", "19044");
    return gruul && gruul->GetVictim() == bot ? 0.0f : 1.0f;
    // End By leewheel
}

float GruulTheDragonkillerStaySpreadForShatterMultiplier::GetValue(Action* action)
{
    if (!HasGroundSlam(bot))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    return dynamic_cast<GruulTheDragonkillerShatterSpreadAction*>(action) ? 1.0f : 0.0f;
}

// MoveTo does not check speed, and thus even with a snare of -100% or more, it starts a spline
// and calculates IsWaitingForLastMove from distance / speed, which is infinite in that case and
// clamps to MaxWaitForMove (5s), blocking all movements for that duration. This multiplier is
// needed to solve the issue for Gruul because the snare he applies (Gronn Lord's Grasp) persists
// 300ms beyond the Shatter sequence, meaning that bots would otherwise be unable to move for 5s
// after the Shatter sequence, even though no in-game factors would prevent their movement.
float GruulTheDragonkillerHoldWhileSnaredMultiplier::GetValue(Action* action)
{
    if (bot->GetSpeed(MOVE_RUN) > 0.0f)
        return 1.0f;

    // By leewheel 2026-08-29 entry化修正（gruul->19044）
    if (!AI_VALUE2(Unit*, "find target", "19044"))
        return 1.0f;

    return dynamic_cast<MovementAction*>(action) ? 0.0f : 1.0f;
}
