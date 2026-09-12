/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "HyjalMultipliers.h"
#include "ChooseTargetActions.h"
#include "EncounterHelpers.h"
#include "HunterActions.h"
#include "HyjalActions.h"
#include "HyjalHelpers.h"
#include "ReachTargetActions.h"
#include "ShamanActions.h"

using namespace HyjalHelpers;
using namespace EncounterHelpers;

// Note: BOT_STATE_NON_COMBAT checks cannot be used by any multiplier that could result in a bot
// having no valid targets as it will then swap to the non-combat engine, even during a boss fight.
// This concern implicates any avoidance action that could hold the bot out of attack range.

float HyjalDelayDpsCooldownsMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->GetMapId() != HYJAL_MAP_ID) // In case strategy persists outside (e.g., server reset)
        return 1.0f;

    if (!IsDpsCooldownAction(bot, action))
        return 1.0f;

    // Suppress Bloodlust/Heroism during all trash waves. It's blown on CD otherwise.
    // By leewheel 2026-08-30 合并上游：改用新value "boss target"，替代本服entry循环查怪
    Unit* boss = AI_VALUE(Unit*, "boss target");
    // End By leewheel
    if (!boss)
    {
        return bot->getClass() == CLASS_SHAMAN &&
            (dynamic_cast<CastBloodlustAction*>(action) ||
             dynamic_cast<CastHeroismAction*>(action)) ? 0.0f : 1.0f;
    }

    return boss->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT ? 0.0f : 1.0f;
}

float HyjalDisableDisperseAndTankFaceMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<CombatFormationMoveAction*>(action))
        return 1.0f;

    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    if (HasProtectionOfElune(bot))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "17968") ||
        AI_VALUE2(Unit*, "find target", "17888") ||
        AI_VALUE2(Unit*, "find target", "17767") ? 0.0f : 1.0f;
}

// Rage Winterchill

float RageWinterchillMeleeControlAvoidanceMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsMelee(bot))
        return 1.0f;

    bool const isAvoidAoe = dynamic_cast<AvoidAoeAction*>(action);

    if (!isAvoidAoe &&
        !dynamic_cast<ReachTargetAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
    {
        return 1.0f;
    }

    Unit* winterchill = AI_VALUE2(Unit*, "find target", "17767");
    if (!winterchill)
        return 1.0f;

    // 同步 brighton 2026-09-08 重构: 采用统一危险控制半径(危险边缘+5码控制带),
    // 与触发/机动helper共用同一半径体系, 避免各环节半径不一致产生行为空档 --By leewheel 2026-09-08
    if (!IsNearDeathAndDecay(botAI, DEATH_AND_DECAY_CONTROL_RADIUS))
        return 1.0f;

    if (isAvoidAoe)
        return 0.0f;

    return winterchill->GetVictim() == bot || PlayerbotAI::IsMainTank(bot) ? 1.0f : 0.0f;
}

// By leewheel 2026-09-04 合并冲突解决: 采纳brighton新方法名GetValueInEncounter(基类HyjalSummitEncounterMultiplier门控),
// 保留HEAD侧的注释说明
// Stock avoid-aoe discards Death and Decay outright: it drops any hazard whose own radius exceeds
// AiPlayerbot.MaxAoeAvoidRadius, and at the default 15 a 20 yard pool never qualifies. Where it
// does run it flees to the raw radius, which still sits inside the aura once the target's combat
// reach is added. The hardcoded action handles both, so keep the two from fighting over the bot
float RageWinterchillRangedControlAvoidanceMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsRanged(bot))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action))
        return 1.0f;

    if (dynamic_cast<RageWinterchillRangedGetOutOfDeathAndDecayAction*>(action))
        return 1.0f;

    // Acquiring a target is not movement. It only reads as such because AttackAction derives from
    // MovementAction, and Attack itself paths nowhere--it sets selection, faces the target, and if
    // anything stops movement. Unlike Azgalor there is no hardcoded targeting action here to spare,
    // so what this would otherwise suppress is stock "dps assist", which every dps bot runs and
    // which nothing else at this fight disables: ranged near a pool could not pick up a target at
    // all until it expired
    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "17767"))
        return 1.0f;

    if (dynamic_cast<AvoidAoeAction*>(action))
        return 0.0f;

    // 同步 brighton 2026-09-08 重构: 远程规避抑制半径统一为危险控制半径, 该函数内无需再叠加自定义扩展值
    return IsNearDeathAndDecay(botAI, DEATH_AND_DECAY_CONTROL_RADIUS) ? 0.0f : 1.0f;
}

// Anetheron

float AnetheronDisableAssistTargetingMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    bool const isTankAssist = dynamic_cast<TankAssistAction*>(action);

    if (!isTankAssist && !dynamic_cast<DpsAssistAction*>(action))
        return 1.0f;

    if (isTankAssist && IsInfernalTank(bot))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "17808") ? 0.0f : 1.0f;
}

// By leewheel 2026-09-04 合并冲突解决: 采纳brighton新方法名GetValueInEncounter
// 防止非地狱火坦克在无意间用奉献、雷霆一击等技能拉到仇恨
float AnetheronAvoidAccidentalInfernalAggroMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!IsAoeThreatAction(bot, action))
        return 1.0f;

    constexpr float holdTankAoeRadius = 20.0f; // 任意取值，但大于范围伤害技能半径
    Unit* infernal = GetNearestInfernal(botAI);
    if (!infernal || infernal->GetExactDist2d(bot) > holdTankAoeRadius)
        return 1.0f;

    return IsInfernalTank(bot) ? 1.0f : 0.0f;
}

float AnetheronInfernalTargetRunToPositionMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<AnetheronBringInfernalToInfernalTankAction*>(action))
        return 1.0f;

    Unit* anetheron = AI_VALUE2(Unit*, "find target", "17808");
    if (!anetheron || anetheron->GetVictim() == bot)
        return 1.0f;

    if (IsInfernalTank(bot))
        return 1.0f;

    return GetInfernoTarget(anetheron) == bot || GetInfernalTargetingBot(botAI) ? 0.0f : 1.0f;
}

float AnetheronControlMovementMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    bool const isTankAvoidAoe =
        PlayerbotAI::IsTank(bot) && dynamic_cast<AvoidAoeAction*>(action);

    if (!isTankAvoidAoe && !dynamic_cast<CombatFormationMoveAction*>(action))
        return 1.0f;

    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "17808") ? 0.0f : 1.0f;
}

float AnetheronControlMisdirectionMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "17808") ? 0.0f : 1.0f;
}

// Kaz'rogal

float KazrogalControlLowManaMovementMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsKazrogalManaUser(botAI) || bot->getClass() == CLASS_HUNTER)
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<KazrogalMoveAwayFromGroupAction*>(action))
        return 1.0f;

    Unit* kazrogal = AI_VALUE2(Unit*, "find target", "17888");
    if (!kazrogal || kazrogal->GetVictim() == bot)
        return 1.0f;

    return AI_VALUE(bool, "kaz'rogal below mana threshold") ? 0.0f : 1.0f;
}

float KazrogalKeepAspectOfTheViperActiveMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!dynamic_cast<CastAspectOfTheHawkAction*>(action) &&
        !dynamic_cast<CastAspectOfTheWildAction*>(action) &&
        !dynamic_cast<CastAspectOfTheDragonhawkAction*>(action) &&
        !dynamic_cast<CastAspectOfTheCheetahAction*>(action) &&
        !dynamic_cast<CastAspectOfThePackAction*>(action) &&
        !dynamic_cast<CastAspectOfTheMonkeyAction*>(action))
    {
        return 1.0f;
    }

    if (!AI_VALUE2(Unit*, "find target", "17888"))
        return 1.0f;

    return bot->GetPower(POWER_MANA) <= MARK_REJOIN_MANA ? 0.0f : 1.0f;
}

// Azgalor

float AzgalorDisableAutoTargetingAndPositioningMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<DpsAssistAction*>(action) &&
        !dynamic_cast<TankAssistAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<AvoidAoeAction*>(action))
    {
        return 1.0f;
    }

    // Set Behind Target is still disabled in RoF (in AzgalorMeleeDpsControlAvoidanceMultiplier)
    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "17842") ? 0.0f : 1.0f;
}

float AzgalorDoomedBotPrioritizePositioningMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsDoomed(bot))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action))
        return 1.0f;

    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    return dynamic_cast<AzgalorMoveToDoomguardTankAction*>(action) ? 1.0f : 0.0f;
}

// By leewheel 2026-09-04 合并冲突解决: 采纳brighton新方法名GetValueInEncounter, 保留HEAD注释
// Leave the escape action as the only thing that moves melee while Rain of Fire is a threat
float AzgalorMeleeDpsControlAvoidanceMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsMelee(bot) || PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    // Doom outranks standing in fire, and it has its own positioning to do
    if (IsDoomed(bot))
        return 1.0f;

    if (dynamic_cast<AzgalorMeleeManeuverThroughFireAction*>(action))
        return 1.0f;

    // Acquiring a target is not movement. It only reads as such because AttackAction derives from
    // MovementAction, and Attack itself paths nowhere--it sets selection, faces the target, and if
    // anything stops movement. Suppressing it would leave a melee bot that entered the fire without
    // a live target unable to pick one up until the pool expired
    if (dynamic_cast<AzgalorDetermineDpsPriorityAction*>(action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "17842"))
        return 1.0f;

    // 同步 brighton 2026-09-08 重构: 阿兹加洛近战火雨控制半径统一为CONTROL_RADIUS, 与触发/机动helper一致
    return IsNearRainOfFire(botAI, RAIN_OF_FIRE_CONTROL_RADIUS) ? 0.0f : 1.0f;
}

// By leewheel 2026-09-04 合并冲突解决: 采纳brighton新方法名GetValueInEncounter, 保留HEAD注释
// Rain of Fire is 15 yards, so unlike Death and Decay it does scrape past the default
// MaxAoeAvoidRadius and stock avoid-aoe does handle it--but only out to the raw radius, which
// leaves the bot inside the aura. The hardcoded action owns this instead.
//
// The dispersal action is the other thing that moves ranged here, and it runs for the whole fight
// rather than settling like the spreads at the other bosses do. It only loses to the escape on the
// ticks the escape actually returns true, so on any tick FleePosition declines an angle it would
// be free to walk the bot back into the fire it has just left
float AzgalorRangedControlAvoidanceMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsRanged(bot))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action))
        return 1.0f;

    // Doom outranks standing in fire, and it has its own positioning to do
    if (IsDoomed(bot))
        return 1.0f;

    if (dynamic_cast<AzgalorRangedGetOutOfRainOfFireAction*>(action))
        return 1.0f;

    // Spared for the same reason as on the melee side: acquiring a target is not movement, it only
    // reads as such because AttackAction derives from MovementAction. Suppressing it would leave a
    // ranged bot near a pool stuck on whatever it was already hitting--unable to switch onto a
    // Doomguard as one spawns, or back onto Azgalor once it dies
    if (dynamic_cast<AzgalorDetermineDpsPriorityAction*>(action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "17842"))
        return 1.0f;

    // 同步 brighton 2026-09-08 重构: 远程火雨规避抑制半径统一为CONTROL_RADIUS, 与触发/机动helper一致,
    // 避免抑制带与触发带不一致造成拉扯(helper已内置刚离开火池的等待逻辑)
    return IsNearRainOfFire(botAI, RAIN_OF_FIRE_CONTROL_RADIUS) ? 0.0f : 1.0f;
}

// Archimonde

// By leewheel 2026-09-04 合并冲突解决: 采纳brighton新方法名GetValueInEncounter, 保留HEAD注释
// Leave the Doomfire avoidance as the only thing that moves a bot near a trail. Its push tapers to
// nothing at DOOMFIRE_DANGER_RADIUS, so without this anything that wants the bot elsewhere--closing
// to spell range, stock avoid-aoe on the same patches, the ranged spread--takes over the instant it
// stops being pushed, drags it back inside, and the two swap the bot every tick
float ArchimondeControlDoomfireAvoidanceMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    // Air Burst outranks Doomfire: a knockback lands the bot somewhere unpredictable anyway, and
    // its own action already reaches further than this suppression does
    if (dynamic_cast<ArchimondeAvoidDoomfireAction*>(action) ||
        dynamic_cast<ArchimondeKeepAirBurstAwayFromTankAction*>(action))
    {
        return 1.0f;
    }

    if (!AI_VALUE2(Unit*, "find target", "17968"))
        return 1.0f;

    // Stock avoid-aoe goes for the whole fight, not merely near a trail. Between them the hardcoded
    // actions cover both hazards here, and stock flees each trail patch to its own 6y radius--a
    // different figure, reached by a different route, pulling against the repulsion the moment the
    // two disagree about which patch matters
    if (dynamic_cast<AvoidAoeAction*>(action))
        return 0.0f;

    if (HasProtectionOfElune(bot))
        return 1.0f;

    // Wider than the radius the avoidance reacts at, so a bot pushed to the edge is still held
    return IsNearDoomfire(botAI, DOOMFIRE_CONTROL_RADIUS) ? 0.0f : 1.0f;
}

float ArchimondeSetTremorTotemMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    if (!dynamic_cast<CastStrengthOfEarthTotemAction*>(action) &&
        !dynamic_cast<CastStoneskinTotemAction*>(action) &&
        !dynamic_cast<CastStoneclawTotemAction*>(action) &&
        !dynamic_cast<CastEarthbindTotemAction*>(action))
    {
        return 1.0f;
    }

    Unit* archimonde = AI_VALUE2(Unit*, "find target", "17968");
    if (!archimonde || archimonde->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT)
        return 1.0f;

    return !HasProtectionOfElune(bot) ? 0.0f : 1.0f;
}
