/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "HyjalTriggers.h"
#include "EncounterHelpers.h"
#include "HyjalActions.h"
#include "HyjalHelpers.h"
#include "InstanceScript.h"
#include "Playerbots.h"

using namespace HyjalHelpers;
using namespace EncounterHelpers;

// General

bool HyjalSummitNoEncounterInProgress::IsActive()
{
    if (bot->GetMapId() != HYJAL_MAP_ID)
        return false;

    InstanceScript* instance = bot->GetInstanceScript();
    return instance && !instance->IsEncounterInProgress();
}

bool HyjalPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", _bossName);
    return boss && boss->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT;
}

bool HyjalBossShouldBeTankedTrigger::IsActive()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    // IsMainTank() does not require an actual tank (by strategy or spec), but the raid strategy
    // assumes the main tank will be a tank.
    if (_mainTankOnly && !PlayerbotAI::IsMainTank(bot))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", _bossName);
    return boss && boss->GetHealthPct() > _activeAboveHealthPct;
}

// Rage Winterchill

bool RageWinterchillRangedShouldSpreadTrigger::IsActive()
{
    return PlayerbotAI::IsRanged(bot) && AI_VALUE2(Unit*, "find target", "17767");
}

bool RageWinterchillMeleeNearDeathAndDecayTrigger::IsActive()
{
    if (!PlayerbotAI::IsMelee(bot))
        return false;

    Unit* winterchill = AI_VALUE2(Unit*, "find target", "17767");
    if (!winterchill || winterchill->GetVictim() == bot)
        return false;

    if (PlayerbotAI::IsMainTank(bot))
        return false;

    // Reaches as far as the suppression that accompanies it, not just to the pool. Everything
    // inside that radius has had its path back to the boss zeroed, so this action has to keep
    // running across the whole of it--it is the only thing left that can walk the bot anywhere,
    // including back onto Winterchill once the pool no longer blocks the ring
    return IsNearDeathAndDecay(botAI, DEATH_AND_DECAY_MELEE_CONTROL_RADIUS);
}

bool RageWinterchillRangedInDeathAndDecayTrigger::IsActive()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "17767"))
        return false;

    return IsInDeathAndDecay(botAI);
}

// Anetheron

bool AnetheronPullingBossOrInfernalTrigger::IsActive()
{
    return bot->getClass() == CLASS_HUNTER && AI_VALUE2(Unit*, "find target", "17808");
}

bool AnetheronRangedShouldSpreadTrigger::IsActive()
{
    if (PlayerbotAI::IsMelee(bot))
        return false;

    Unit* anetheron = AI_VALUE2(Unit*, "find target", "17808");
    if (!anetheron)
        return false;

    if (GetInfernoTarget(anetheron) == bot)
        return false;

    return !GetInfernalToAttack(botAI, anetheron);
}

// Whoever is holding Anetheron stays put: walking him across the platform costs the raid more than
// a two second stun costs one bot. The Inferno target itself is excluded because it has its own job
// -- carrying the summon to the gathering spot -- and nothing it does avoids a stun centred on it
bool AnetheronBotIsNearInfernoTargetTrigger::IsActive()
{
    Unit* anetheron = AI_VALUE2(Unit*, "find target", "17808");
    if (!anetheron || anetheron->GetVictim() == bot)
        return false;

    Player* infernoTarget = GetInfernoTarget(anetheron);
    if (!infernoTarget || infernoTarget == bot)
        return false;

    return bot->GetExactDist2d(infernoTarget) < INFERNAL_ESCAPE_DISTANCE;
}

bool AnetheronBotIsTargetedByInfernalTrigger::IsActive()
{
    Unit* anetheron = AI_VALUE2(Unit*, "find target", "17808");
    if (!anetheron || anetheron->GetVictim() == bot)
        return false;

    if (GetInfernoTarget(anetheron) == bot)
        return true;

    if (IsInfernalTank(bot))
        return false;

    return GetInfernalTargetingBot(bot);
}

bool AnetheronInfernalsPulseImmolationTrigger::IsActive()
{
    if (PlayerbotAI::IsTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "anetheron"))
        return false;

    Unit* infernal = GetNearestInfernal(bot);
    return infernal && infernal->GetVictim() != bot &&
        bot->GetExactDist2d(infernal) < INFERNAL_DANGER_RADIUS;
}

bool AnetheronInfernalsShouldBeTankedAwayTrigger::IsActive()
{
    if (!IsInfernalTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "17808"))
        return false;

    Unit* infernal = GetInfernalTargetingBot(bot);
    return infernal && bot->IsWithinMeleeRange(infernal);
}

bool AnetheronShouldDivideDpsTrigger::IsActive()
{
    return PlayerbotAI::IsDps(bot) && AI_VALUE2(Unit*, "find target", "17808");
}

// Kaz'rogal

bool KazrogalCanSplitMalevolentCleaveDamageTrigger::IsActive()
{
    if (!PlayerbotAI::IsAssistTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "17888"))
        return false;

    if (bot->getClass() != CLASS_PALADIN)
        return true;

    return !botsBelowManaThreshold.contains(bot->GetGUID());
}

bool KazrogalRangedShouldAvoidWarStompTrigger::IsActive()
{
    // This is what puts ranged on the arc, so it is ranged that belong in it. Melee mana users--a
    // ret paladin, an enhancement shaman--pass every other test here and would be walked out to a
    // slot they have no business in, then dragged back by ReachMelee the moment they arrived
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    // By leewheel 2026-08-30 合并上游简化(去掉mana user前置判断)；entry规则查怪(17888=kaz'rogal)
    if (!AI_VALUE2(Unit*, "find target", "17888"))
        return false;

    return !botsBelowManaThreshold.contains(bot->GetGUID());
}

bool KazrogalBotIsLowOnManaTrigger::IsActive()
{
    if (!IsKazrogalManaUser(bot)) // By leewheel 2026-08-30 合并上游单参签名
        return false;

    // Hunters never run. They rely only on Aspect of the Viper.
    if (bot->getClass() == CLASS_HUNTER)
        return false;

    Unit* kazrogal = AI_VALUE2(Unit*, "find target", "17888");
    if (!kazrogal || kazrogal->GetVictim() == bot)
        return false;

    if (bot->GetPower(POWER_MANA) <= MARK_DANGER_MANA)
    {
        botsBelowManaThreshold.insert(bot->GetGUID());
        return true;
    }

    return botsBelowManaThreshold.contains(bot->GetGUID());
}

bool KazrogalHunterShouldPreserveManaTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "17888"))
        return false;

    if (bot->HasAura(Id(HyjalSpells::SPELL_ASPECT_OF_THE_VIPER)))
        return false;

    // Activate at 3200 mana; switch back based on normal Hunter aspect strategies.
    return bot->GetPower(POWER_MANA) <= MARK_DANGER_MANA;
}

bool KazrogalMarkOnMageOrPaladinTrigger::IsActive()
{
    if (bot->getClass() != CLASS_MAGE && bot->getClass() != CLASS_PALADIN)
        return false;

    Unit* kazrogal = AI_VALUE2(Unit*, "find target", "17888");
    if (!kazrogal || kazrogal->GetVictim() == bot)
        return false;

    Aura* aura = bot->GetAura(Id(HyjalSpells::SPELL_MARK_OF_KAZROGAL));
    if (!aura)
        return false;

    uint32 const mana = bot->GetPower(POWER_MANA);
    constexpr float markFullyDrainedMana = 3000.0f;
    if (mana >= markFullyDrainedMana)
        return false;

    // Blowing Ice Block/Divine Shield is worth it only where the Mark outlasts mana.
    //   2400-2999  needs 5s left      1200-1799  needs 3s left      0-599  needs 1s left
    //   1800-2399  needs 4s left       600-1199  needs 2s left
    uint32 const tickDrain = static_cast<uint32>(MARK_TICK_DRAIN);
    int32 const requiredMs = static_cast<int32>(mana / tickDrain + 1) * IN_MILLISECONDS;

    return aura->GetDuration() >= requiredMs;
}

bool KazrogalWarlockShouldManageManaTrigger::IsActive()
{
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "17888"))
        return false;

    if (bot->GetPower(POWER_MANA) <= MARK_LIFE_TAP_MANA &&
        bot->GetHealthPct() > sPlayerbotAIConfig.lowHealth)
    {
        return true;
    }

    if (!HasMarkOfKazrogal(bot) || botAI->HasAura("shadow ward", bot))
        return false;

    return bot->GetPower(POWER_MANA) <= MARK_TICK_DRAIN;
}

bool KazrogalImmunityNoLongerNeededTrigger::IsActive()
{
    if (bot->getClass() != CLASS_MAGE &&
        (bot->getClass() != CLASS_PALADIN || PlayerbotAI::IsHeal(bot)))
    {
        return false;
    }

    uint32 const spellId = GetKazrogalImmunitySpell(bot);
    if (!spellId || !bot->HasAura(spellId))
        return false;

    if (HasMarkOfKazrogal(bot))
        return false;

    // 50% is a proxy for the bot potentially being in range of getting blown up by other bots,
    // so don't wipe the immunity if below that HP.
    constexpr float keepImmunityHealthPct = 50.0f;
    if (bot->GetHealthPct() <= keepImmunityHealthPct)
        return false;

    return AI_VALUE2(Unit*, "find target", "kaz'rogal");
}

// Azgalor

bool AzgalorRangedShouldSpreadTrigger::IsActive()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* azgalor = AI_VALUE2(Unit*, "find target", "17842");
    if (!azgalor || azgalor->GetVictim() == bot)
        return false;

    if (IsDoomed(bot))
        return false;

    constexpr float suppressionRadius = RAIN_OF_FIRE_RADIUS + 10.0f;
    return !IsNearRainOfFire(botAI, suppressionRadius);
}

bool AzgalorMeleeNearRainOfFireTrigger::IsActive()
{
    if (!PlayerbotAI::IsMelee(bot))
        return false;

    Unit* azgalor = AI_VALUE2(Unit*, "find target", "17842");
    if (!azgalor || azgalor->GetVictim() == bot)
        return false;

    if (IsDoomed(bot))
        return false;

    // The Doomguard tank is excluded due to needing to hold at the Doomguard tanking position.
    // This isn't ideal, but special avoidance of a not-that-dangerous ability for one role that
    // needs specific positioning is not worth the time and effort.
    if (IsDoomguardTank(bot))
        return false;

    // Reaches as far as the suppression that accompanies it, not just to the fire. Everything
    // inside that radius has had its other movement zeroed, so this action has to keep running
    // across the whole of it--it is the only thing left that can walk the bot anywhere, including
    // back onto Azgalor once the tank has dragged him clear of the pool
    return IsNearRainOfFire(botAI, RAIN_OF_FIRE_MELEE_CONTROL_RADIUS);
}

bool AzgalorRangedInRainOfFireTrigger::IsActive()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "17842"))
        return false;

    if (IsDoomed(bot))
        return false;

    return IsInRainOfFire(botAI);
}

bool AzgalorBotIsDoomedTrigger::IsActive()
{
    return IsDoomed(bot);
}

bool AzgalorShouldControlDoomguardsTrigger::IsActive()
{
    if (!IsDoomguardTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "17842"))
        return false;

    return AI_VALUE2(Unit*, "find target", "17864") || AnyGroupMemberHasDoom(bot);
}

bool AzgalorShouldDivideDpsTrigger::IsActive()
{
    return PlayerbotAI::IsDps(bot) && AI_VALUE2(Unit*, "find target", "17842");
}

// Archimonde

bool ArchimondeBossCastsFearTrigger::IsActive()
{
    if (bot->getClass() != CLASS_PRIEST && bot->getClass() != CLASS_SHAMAN)
        return false;

    Unit* archimonde = AI_VALUE2(Unit*, "find target", "17968");
    if (!archimonde || archimonde->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT)
        return false;

    return !HasProtectionOfElune(bot);
}

bool ArchimondeBossCastingAirBurstTrigger::IsActive()
{
    Unit* archimonde = AI_VALUE2(Unit*, "find target", "17968");
    if (!archimonde || archimonde->GetVictim() == bot)
        return false;

    if (HasProtectionOfElune(bot))
        return false;

    if (PlayerbotAI::IsMainTank(bot))
        return false;

    return GetPendingAirBurstCast(bot->GetMap()->GetInstanceId());
}

// No longer gated to the opening. Ranged drift back together across a fight this long, and the
// spread is cheap: the action rate limits itself and the Doomfire multiplier removes it near a
// trail, so leaving it live costs nothing where it would otherwise get in the way
bool ArchimondeRangedShouldSpreadTrigger::IsActive()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "17968"))
        return false;

    return !HasProtectionOfElune(bot);
}

bool ArchimondeBotIsNearDoomfireTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "17968"))
        return false;

    if (HasProtectionOfElune(bot))
        return false;

    // The same radius the multiplier suppresses at, so the action owns movement across exactly the
    // area cleared for it. Nothing it does reaches further: the push only has a magnitude inside
    // DOOMFIRE_DANGER_RADIUS, the trapped sweep needs a patch inside DOOMFIRE_BURN_RADIUS, and past
    // this ordinary movement is free again and closes the gap to Archimonde perfectly well. Gating
    // here rather than in the action is what keeps the 18y field sweep off every bot on every tick
    // of a fight where a trail is usually nowhere near them
    return IsNearDoomfire(botAI, DOOMFIRE_CONTROL_RADIUS);
}

bool ArchimondeBotStoodInDoomfireTrigger::IsActive()
{
    if (bot->getClass() != CLASS_MAGE && bot->getClass() != CLASS_ROGUE &&
        bot->getClass() != CLASS_PALADIN)
    {
        return false;
    }

    if (HasProtectionOfElune(bot))
        return false;

    return bot->GetHealthPct() < 40.0f && // Arbitrary high risk-of-death threshold
        (bot->HasAura(Id(HyjalSpells::SPELL_DOOMFIRE)) ||
         bot->HasAura(Id(HyjalSpells::SPELL_DOOMFIRE_DOT)));
}
