/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "HyjalTriggers.h"
#include "EncounterHelpers.h"
#include "HyjalHelpers.h"
#include "Playerbots.h"

using namespace HyjalHelpers;
using namespace EncounterHelpers;

// General

bool HyjalNoEncounterInProgressTrigger::IsActive()
{
    if (bot->GetMapId() != HYJAL_MAP_ID)
        return false;

    return !IsEncounterInProgress(bot, HYJAL_MAP_ID);
}

bool HyjalPullingBossTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", _bossName);
    return boss && boss->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT;
}

bool HyjalBossShouldBeTankedTrigger::IsActiveInEncounter()
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

bool RageWinterchillRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsRanged(bot) && AI_VALUE2(Unit*, "find target", "17767");
}

bool RageWinterchillMeleeNearDeathAndDecayTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot))
        return false;

    Unit* winterchill = AI_VALUE2(Unit*, "find target", "17767");
    if (!winterchill || winterchill->GetVictim() == bot)
        return false;

    if (PlayerbotAI::IsMainTank(bot))
        return false;

    // 同步 brighton 2026-09-08 重构: 触发范围统一为危险控制半径(池边缘+5码控制带), 与multiplier/机动helper一致,
    // 覆盖"通往boss路径被抑制"的整段范围, 使本动作成为该区域内唯一能移动bot的机制
    return IsNearDeathAndDecay(botAI, DEATH_AND_DECAY_CONTROL_RADIUS);
}

bool RageWinterchillRangedInDeathAndDecayTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "17767"))
        return false;

    return IsInDeathAndDecay(botAI);
}

// Anetheron

bool AnetheronPullingBossOrInfernalTrigger::IsActiveInEncounter()
{
    return bot->getClass() == CLASS_HUNTER && AI_VALUE2(Unit*, "find target", "17808");
}

bool AnetheronRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* anetheron = AI_VALUE2(Unit*, "find target", "17808");
    if (!anetheron)
        return false;

    if (GetInfernoTarget(anetheron) == bot)
        return false;

    return !GetInfernalToAttack(botAI, anetheron);
}

// By leewheel 2026-09-04 合并冲突解决: 采纳brighton新方法名IsActiveInEncounter(brighton的encounter门控框架)
// Whoever is holding Anetheron stays put: walking him across the platform costs the raid more than
// a two second stun costs one bot. The Inferno target itself is excluded because it has its own job
// -- carrying the summon to the gathering spot -- and nothing it does avoids a stun centred on it
bool AnetheronBotIsNearInfernoTargetTrigger::IsActiveInEncounter()
{
    Unit* anetheron = AI_VALUE2(Unit*, "find target", "17808");
    if (!anetheron || anetheron->GetVictim() == bot)
        return false;

    Player* infernoTarget = GetInfernoTarget(anetheron);
    if (!infernoTarget || infernoTarget == bot)
        return false;

    return bot->GetExactDist2d(infernoTarget) < INFERNAL_ESCAPE_DISTANCE;
}

bool AnetheronBotIsTargetedByInfernalTrigger::IsActiveInEncounter()
{
    Unit* anetheron = AI_VALUE2(Unit*, "find target", "17808");
    if (!anetheron || anetheron->GetVictim() == bot)
        return false;

    if (GetInfernoTarget(anetheron) == bot)
        return true;

    if (IsInfernalTank(bot))
        return false;

    return GetInfernalTargetingBot(botAI);
}

bool AnetheronInfernalsPulseImmolationTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsTank(bot))
        return false;

    // By leewheel 2026-09-05 清理英文遗留：Anetheron按entry规则(17808)
    if (!AI_VALUE2(Unit*, "find target", "17808"))
        return false;

    Unit* infernal = GetNearestInfernal(botAI);
    if (!infernal || infernal->GetVictim() == bot)
        return false;

    return bot->GetExactDist2d(infernal) < INFERNAL_DANGER_RADIUS;
}

bool AnetheronInfernalsShouldBeTankedAwayTrigger::IsActiveInEncounter()
{
    if (!IsInfernalTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "17808"))
        return false;

    Unit* infernal = GetInfernalTargetingBot(botAI);
    return infernal && bot->IsWithinMeleeRange(infernal);
}

bool AnetheronShouldDivideDpsTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsDps(bot) && AI_VALUE2(Unit*, "find target", "17808");
}

// Kaz'rogal

bool KazrogalCanSplitMalevolentCleaveDamageTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsAssistTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "17888"))
        return false;

    if (bot->getClass() != CLASS_PALADIN)
        return true;

    return !botsBelowManaThreshold.contains(bot->GetGUID());
}

bool KazrogalRangedShouldAvoidWarStompTrigger::IsActiveInEncounter()
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

bool KazrogalBotIsLowOnManaTrigger::IsActiveInEncounter()
{
    if (!IsKazrogalManaUser(botAI))
        return false;

    // 同步 brighton 2026-09-08 重构: 猎人与术士均不逃离 -- 猎人只靠蝰蛇守护,
    // 术士靠生命分流/暗影之幕应对印记(低血也强制分流, 见KazrogalWarlockManageManaAction)
    if (bot->getClass() == CLASS_HUNTER || bot->getClass() == CLASS_WARLOCK)
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

bool KazrogalHunterShouldPreserveManaTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "17888"))
        return false;

    if (bot->HasAura(Id(HyjalSpells::SPELL_ASPECT_OF_THE_VIPER)))
        return false;

    // Eligible to switch back at MARK_REJOIN_MANA, per the multiplier.
    return bot->GetPower(POWER_MANA) <= MARK_DANGER_MANA;
}

bool KazrogalMarkOnMageOrPaladinTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_MAGE && bot->getClass() != CLASS_PALADIN)
        return false;

    Unit* kazrogal = AI_VALUE2(Unit*, "find target", "17888");
    if (!kazrogal || kazrogal->GetVictim() == bot)
        return false;

    Aura* mark = bot->GetAura(Id(HyjalSpells::SPELL_MARK_OF_KAZROGAL));
    if (!mark)
        return false;

    uint32 const mana = bot->GetPower(POWER_MANA);
    if (mana >= MARK_FULL_DRAIN)
        return false;

    // Blowing Ice Block/Divine Shield is worth it only where the Mark outlasts mana.
    //   2400-2999  needs 5s left      1200-1799  needs 3s left      0-599  needs 1s left
    //   1800-2399  needs 4s left       600-1199  needs 2s left
    int32 const requiredMs = static_cast<int32>((mana / MARK_TICK_DRAIN + 1) * IN_MILLISECONDS);
    return mark->GetDuration() >= requiredMs;
}

bool KazrogalWarlockShouldManageManaTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "17888"))
        return false;

    if (bot->GetPower(POWER_MANA) <= MARK_LIFE_TAP_MANA)
        return true;

    if (!HasMarkOfKazrogal(bot) || botAI->HasAura("shadow ward", bot))
        return false;

    return bot->GetPower(POWER_MANA) <= MARK_TICK_DRAIN;
}

bool KazrogalImmunityNoLongerNeededTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_MAGE &&
        (bot->getClass() != CLASS_PALADIN || PlayerbotAI::IsHeal(bot)))
    {
        return false;
    }

    uint32 const spellId = GetSelfImmunitySpell(bot);
    if (!spellId || !bot->HasAura(spellId))
        return false;

    if (HasMarkOfKazrogal(bot))
        return false;

    // 50% is a proxy for the bot potentially being in range of getting blown up by other bots,
    // so don't wipe the immunity if below that HP.
    constexpr float keepImmunityHealthPct = 50.0f;
    if (bot->GetHealthPct() <= keepImmunityHealthPct)
        return false;

    // By leewheel 2026-09-05 清理英文遗留：Kaz'rogal按entry规则(17888)
    return AI_VALUE2(Unit*, "find target", "17888");
}

// Azgalor

bool AzgalorRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* azgalor = AI_VALUE2(Unit*, "find target", "17842");
    if (!azgalor || azgalor->GetVictim() == bot)
        return false;

    if (IsDoomed(bot))
        return false;

    // 同步 brighton 2026-09-08 重构: 抑制半径统一为CONTROL_RADIUS, 与multiplier一致
    return !IsNearRainOfFire(botAI, RAIN_OF_FIRE_CONTROL_RADIUS);
}

bool AzgalorMeleeNearRainOfFireTrigger::IsActiveInEncounter()
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

    // 同步 brighton 2026-09-08 重构: 近战触发范围统一为CONTROL_RADIUS, 覆盖"其他移动被清零"的整段,
    // 本动作是火雨范围内唯一能移动bot的机制 --By leewheel 2026-09-08
    return IsNearRainOfFire(botAI, RAIN_OF_FIRE_CONTROL_RADIUS);
}

bool AzgalorRangedInRainOfFireTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "17842"))
        return false;

    if (IsDoomed(bot))
        return false;

    return IsInRainOfFire(botAI);
}

bool AzgalorBotIsDoomedTrigger::IsActiveInEncounter()
{
    return IsDoomed(bot);
}

bool AzgalorShouldControlDoomguardsTrigger::IsActiveInEncounter()
{
    if (!IsDoomguardTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "17842"))
        return false;

    // 保留本地entry化(boss/小怪英文名一律改用NPC entry, 项目规则), 17864=末日守卫 --By leewheel 2026-09-08
    return AI_VALUE2(Unit*, "find target", "17864") || AnyGroupMemberHasDoom(bot);
}

bool AzgalorShouldDivideDpsTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsDps(bot) && AI_VALUE2(Unit*, "find target", "17842");
}

// Archimonde

bool ArchimondeBossCastsFearTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_SHAMAN)
        return false;

    Unit* archimonde = AI_VALUE2(Unit*, "find target", "17968");
    if (!archimonde || archimonde->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT)
        return false;

    return !HasProtectionOfElune(bot);
}

bool ArchimondeBossCastingAirBurstTrigger::IsActiveInEncounter()
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

// By leewheel 2026-09-04 合并冲突解决: 采纳brighton新方法名IsActiveInEncounter
// No longer gated to the opening. Ranged drift back together across a fight this long, and the
// spread is cheap: the action rate limits itself and the Doomfire multiplier removes it near a
// trail, so leaving it live costs nothing where it would otherwise get in the way
bool ArchimondeRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "17968"))
        return false;

    return !HasProtectionOfElune(bot);
}

bool ArchimondeBotIsNearDoomfireTrigger::IsActiveInEncounter()
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

bool ArchimondeBotStoodInDoomfireTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_MAGE && bot->getClass() != CLASS_ROGUE &&
        bot->getClass() != CLASS_PALADIN)
    {
        return false;
    }

    if (HasProtectionOfElune(bot))
        return false;

    if (!bot->HasAura(Id(HyjalSpells::SPELL_DOOMFIRE)) &&
        !bot->HasAura(Id(HyjalSpells::SPELL_DOOMFIRE_DOT)))
    {
        return false;
    }

    constexpr float dangerHealthPct = 40.0f;
    return bot->GetHealthPct() < dangerHealthPct;
}
