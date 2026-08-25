/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "MagMultipliers.h"
#include "ChooseTargetActions.h"
#include "DKActions.h"
#include "DruidBearActions.h"
#include "EncounterHelpers.h"
#include "FollowActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "MagActions.h"
#include "MagHelpers.h"
#include "MageActions.h"
#include "MovementActions.h"
#include "PaladinActions.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"
#include "WarriorActions.h"
#include "WipeAction.h"

using namespace MagHelpers;
using namespace EncounterHelpers;

// When a cube clicker is in the handling phase (waiting near cube or moving
// to use), suppress movement actions that would pull them away from the cube
float MagtheridonUseManticronCubeMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<FleeAction*>(action) &&
        !dynamic_cast<FollowAction*>(action) &&
        !dynamic_cast<ReachTargetAction*>(action) &&
        !dynamic_cast<CastBlinkBackAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action) &&
        !dynamic_cast<CastDisengageAction*>(action))
    {
        return 1.0;
    }

    if (!AI_VALUE2(Unit*, "find target", "17257"))
        return 1.0f;

    if (!IsCubeClicker(bot))
        return 1.0f;

    //By leewheel 2026-08-26 合并：采用对侧简化后的计时判定(GetInstanceId直取)
    auto timerIt = blastNovaTimer.find(bot->GetInstanceId());
    if (timerIt != blastNovaTimer.end())
        return 0.0f;

    return getMSTimeDiff(timerIt->second, getMSTime()) >= BLAST_NOVA_INTERIM_MS ? 0.0f : 1.0f;
}

// Wait for 6 seconds after Magtheridon becomes attackable before engaging
float MagtheridonWaitToAttackMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<AttackAction*>(action) &&
        !dynamic_cast<CastSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "17257");
    if (!magtheridon || !IsMagtheridonActive(magtheridon))
        return 1.0f;

    if (PlayerbotAI::IsMainTank(bot))
        return 1.0f;

    constexpr uint32 dpsWaitMs = 6 * IN_MILLISECONDS;
    //By leewheel 2026-08-26 合并：同上采用对侧简化判定
    auto it = dpsWaitTimer.find(magtheridon->GetInstanceId());
    if (it == dpsWaitTimer.end())
        return 0.0f;

    return getMSTimeDiff(it->second, getMSTime()) <= dpsWaitMs ? 0.0f : 1.0f;
}

float MagtheridonControlTankActionsMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    bool const isAvoidAoe = dynamic_cast<AvoidAoeAction*>(action);
    bool const isReachTargetSpell =
        dynamic_cast<CastReachTargetSpellAction*>(action);

    if (!isAvoidAoe && !isReachTargetSpell && !IsTauntAction(bot, action) &&
        !dynamic_cast<TankAssistAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action))
    {
        return 1.0f;
    }

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "17257");
    if (!magtheridon)
        return 1.0f;

    if (isAvoidAoe && magtheridon->GetVictim() != bot)
        return 1.0f;

    if (isReachTargetSpell && !PlayerbotAI::IsMainTank(bot))
        return 1.0f;

    if (GetChanneler(bot, NORTHWEST_CHANNELER))
        return 0.0f;

    return GetChanneler(bot, NORTHEAST_CHANNELER) ? 0.0f : 1.0f;
}

float MagtheridonDebrisDangerMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<WipeAction*>(action) ||
        dynamic_cast<MagtheridonMoveOutOfDebrisAction*>(action))
    {
        return 1.0f;
    }

    if (!AI_VALUE2(Unit*, "find target", "17257"))
        return 1.0f;

    constexpr float debrisSuppressionZone = 15.0f;
    if (IsPositionInActiveDebris(
            bot, bot->GetPositionX(), bot->GetPositionY(), debrisSuppressionZone))
    {
        return 0.0f;
    }

    return 1.0f;
}
