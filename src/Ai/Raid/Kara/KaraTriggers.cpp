/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

//By leewheel 20260729 同步 brighton-chi/mod-playerbots 最终版本
//End By leewheel

#include "KaraTriggers.h"
#include "EncounterHelpers.h"
#include "KaraActions.h"
#include "KaraHelpers.h"
#include "Playerbots.h"
#include "ThreatHelpers.h"

using namespace KaraHelpers;
using namespace EncounterHelpers;

// Shared

bool KarazhanNoEncounterInProgressTrigger::IsActive()
{
    return !IsEncounterInProgress(bot, KARA_MAP_ID);
}

bool KarazhanEnemiesCastFearTrigger::IsActive()
{
    if (bot->getClass() != CLASS_SHAMAN)
        return false;

    //By leewheel 2026-11-04 采纳上游 IsKarazhanFearCasterActive（内部：幽灵战马/大灰狼/夜之魇落地/
    //  多萝西与咆哮 四路判定，全部走 entry 登记，功能覆盖本核旧的四段手写判定）
    return IsKarazhanFearCasterActive(bot) && !AI_VALUE2(bool, "has totem", "tremor totem");
}

// Trash

bool ManaWarpCastsWarpBreachTrigger::IsActive()
{
    return GetManaWarpToStun(botAI, WARP_BREACH_MIN_REACTION_MS);
}

bool ManaWarpStunnerShouldHoldTrigger::IsActive()
{
    return GetManaWarpToHoldFor(botAI);
}

// Attumen the Huntsman

// Midnight is still present as a separate (invisible) unit after Attumen mounts.
// A Midnight threat list check will capture the entire encounter.
bool AttumenTheHuntsmanShouldBeTankedTrigger::IsActiveInEncounter()
{
    // First assist tank
    if (!PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
        return false;

    Unit* midnight = FindThreatTarget(bot, "midnight");
    return midnight && !IsAttumenMounted(*midnight) && GetEncounterBoss();
}

bool AttumenTheHuntsmanMidnightShouldBeFocusedTrigger::IsActiveInEncounter()
{
    Unit* midnight = FindThreatTarget(bot, "midnight");
    if (!midnight || IsAttumenMounted(*midnight) || AI_VALUE(Unit*, "current target") == midnight)
        return false;

    // The first assist tank takes Attumen once he is up.
    return !PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) || !GetEncounterBoss();
}

bool AttumenTheHuntsmanMountedShouldBeTankedTrigger::IsActiveInEncounter()
{
    //By leewheel 2026-11-04 采纳上游 IsTank 判据：本触发器问的是「骑乘中的阿图门该由谁坦克」，
    //  IsMechanicTrackerBot 的 4 参版本要求 IsDps（EncounterHelpers.cpp:446），对坦克恒 false ⇒ 无人接管。
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* attumen = GetMountedAttumen(bot);
    return attumen && attumen->GetVictim() == bot;
}

bool AttumenTheHuntsmanShouldStackBehindTrigger::IsActiveInEncounter()
{
    Unit* attumen = GetMountedAttumen(bot);
    return attumen && attumen->GetVictim() != bot;
}

bool AttumenTheHuntsmanShouldManageDpsTimerTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, KARA_MAP_ID))
        return false;

    Unit* midnight = FindThreatTarget(bot, "midnight");
    return midnight && IsAttumenMounted(*midnight);
}

// Moroes

bool MoroesShouldPrioritizeAddsTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, KARA_MAP_ID) && GetEncounterBoss();
}

// Maiden of Virtue

bool MaidenOfVirtueShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot) && GetEncounterBoss();
}

bool MaidenOfVirtueShouldBreakRepentanceTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* maiden = GetEncounterBoss();
    return maiden && maiden->GetVictim() == bot;
}

bool MaidenOfVirtueShouldGroundHolyFireTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_SHAMAN)
        return false;

    if (!GetEncounterBoss())
        return false;

    return !AI_VALUE2(bool, "has totem", "grounding totem");
}

bool MaidenOfVirtueRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsRanged(bot) && GetEncounterBoss();
}

// The Big Bad Wolf

bool BigBadWolfShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (!GetOperaActor(bot, "the big bad wolf"))
        return false;

    return !bot->HasAura(Id(KaraSpells::SPELL_LITTLE_RED_RIDING_HOOD));
}

bool BigBadWolfChasingLittleRedRidingHoodTrigger::IsActiveInEncounter()
{
    return bot->HasAura(Id(KaraSpells::SPELL_LITTLE_RED_RIDING_HOOD));
}

// Romulo and Julianne

bool RomuloAndJulianneShouldBeKilledTogetherTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(botAI, bot, KARA_MAP_ID))
        return false;

    return GetOperaActor(bot, "romulo") && GetOperaActor(bot, "julianne");
}

// The Wizard of Oz

bool WizardOfOzShouldPrioritizeTargetsTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(botAI, bot, KARA_MAP_ID))
        return false;

    for (char const* actor : OZ_TARGETS)
    {
        if (GetOperaActor(bot, actor))
            return true;
    }

    return false;
}

bool WizardOfOzMageShouldBurnStrawmanTrigger::IsActiveInEncounter()
{
    return bot->getClass() == CLASS_MAGE && GetOperaActor(bot, "strawman");
}

// The Curator

bool TheCuratorShouldPrioritizeAstralFlaresTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, KARA_MAP_ID) && FindThreatTarget(bot, "astral flare");
}

bool TheCuratorShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot) && GetEncounterBoss();
}

bool TheCuratorRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsRanged(bot) && GetEncounterBoss();
}

// Terestian Illhoof

bool TerestianIllhoofShouldPrioritizeChainsTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, KARA_MAP_ID) && GetEncounterBoss();
}

// Shade of Aran

bool ShadeOfAranArcaneExplosionIsCastingTrigger::IsActiveInEncounter()
{
    Unit* aran = GetEncounterBoss();
    return aran && IsAranCastingArcaneExplosion(aran) && !IsFlameWreathActive(bot);
}

bool ShadeOfAranFlameWreathIsActiveTrigger::IsActiveInEncounter()
{
    return GetEncounterBoss() && IsFlameWreathActive(bot);
}

bool ShadeOfAranConjuredElementalsSummonedTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, KARA_MAP_ID) && FindThreatTarget(bot, "conjured elemental");
}

bool ShadeOfAranRangedShouldMaintainDistanceTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* aran = GetEncounterBoss();
    if (!aran)
        return false;

    if (bot->HasAura(Id(KaraSpells::SPELL_BLIZZARD)))
        return false;

    return !IsAranCastingArcaneExplosion(aran) && !IsFlameWreathActive(bot);
}

// Netherspite

bool NetherspiteRedBeamIsActiveTrigger::IsActiveInEncounter()
{
    Unit* netherspite = GetEncounterBoss();
    if (!netherspite || IsBanishPhase(netherspite))
        return false;

    constexpr float searchRadius = 150.0f;
    return bot->FindNearestCreature(Id(KaraNpcs::NPC_RED_PORTAL), searchRadius);
}

bool NetherspiteBlueBeamIsActiveTrigger::IsActiveInEncounter()
{
    Unit* netherspite = GetEncounterBoss();
    if (!netherspite || IsBanishPhase(netherspite))
        return false;

    constexpr float searchRadius = 150.0f;
    return bot->FindNearestCreature(Id(KaraNpcs::NPC_BLUE_PORTAL), searchRadius);
}

bool NetherspiteGreenBeamIsActiveTrigger::IsActiveInEncounter()
{
    Unit* netherspite = GetEncounterBoss();
    if (!netherspite || IsBanishPhase(netherspite))
        return false;

    constexpr float searchRadius = 150.0f;
    return bot->FindNearestCreature(Id(KaraNpcs::NPC_GREEN_PORTAL), searchRadius);
}

bool NetherspiteBotIsNotBeamBlockerTrigger::IsActiveInEncounter()
{
    Unit* netherspite = GetEncounterBoss();
    if (!netherspite || IsBanishPhase(netherspite))
        return false;

    auto [redBlocker, greenBlocker, blueBlocker] = GetCurrentBeamBlockers(bot);
    return bot != redBlocker && bot != blueBlocker && bot != greenBlocker;
}

bool NetherspiteInBanishPhaseTrigger::IsActiveInEncounter()
{
    Unit* netherspite = GetEncounterBoss();
    return netherspite && IsBanishPhase(netherspite);
}

bool NetherspiteShouldManageTimersAndTrackersTrigger::IsActiveInEncounter()
{
    return GetEncounterBoss();
}

// Prince Malchezaar

bool PrinceMalchezaarBotIsEnfeebledTrigger::IsActiveInEncounter()
{
    return bot->HasAura(Id(KaraSpells::SPELL_ENFEEBLE));
}

bool PrinceMalchezaarEngagedByNonTanksTrigger::IsActiveInEncounter()
{
    Unit* malchezaar = GetEncounterBoss();
    if (!malchezaar)
        return false;

    if (bot->HasAura(Id(KaraSpells::SPELL_ENFEEBLE)))
        return false;

    if ((PlayerbotAI::IsTank(bot) && malchezaar->GetVictim() == bot) ||
        PlayerbotAI::IsMainTank(bot))
    {
        return false;
    }

    return true;
}

bool PrinceMalchezaarShouldBeTankedTrigger::IsActiveInEncounter()
{
    Unit* malchezaar = GetEncounterBoss();
    if (!malchezaar)
        return false;

    return (PlayerbotAI::IsTank(bot) && malchezaar->GetVictim() == bot) ||
        PlayerbotAI::IsMainTank(bot);
}

// Nightbane

bool NightbaneShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* nightbane = GetEncounterBoss();
    return nightbane && nightbane->GetPositionZ() <= NIGHTBANE_FLIGHT_Z;
}

bool NightbaneGroundPhaseEngagedByRangedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* nightbane = GetEncounterBoss();
    return nightbane && nightbane->GetPositionZ() <= NIGHTBANE_FLIGHT_Z;
}

bool NightbanePetsChaseFlyingBossOutOfBoundsTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER && bot->getClass() != CLASS_WARLOCK)
        return false;

    if (!GetEncounterBoss())
        return false;

    Pet* pet = bot->GetPet();
    return pet && pet->IsAlive();
}

bool NightbaneInFlightPhaseTrigger::IsActiveInEncounter()
{
    Unit* nightbane = GetEncounterBoss();
    if (!nightbane || nightbane->GetPositionZ() <= NIGHTBANE_FLIGHT_Z)
        return false;

    // By leewheel 2026-08-29 合并：采用对侧GetMSTimeDiff毫秒制计时(与后续逻辑配套)，entry规则不变
    constexpr uint32 flightPhaseDurationMs = 35 * IN_MILLISECONDS;
    // End By leewheel
    // After 35s, Nightbane goes to land, and bots freely follow their master
    KaraInstanceState const* state = FindKaraState(nightbane->GetInstanceId());
    if (!state || !state->nightbaneFlightPhaseStartTimer)
        return false;

    return getMSTimeDiff(*state->nightbaneFlightPhaseStartTimer, getMSTime()) <
        flightPhaseDurationMs;
}

bool NightbaneBotWentOutOfBoundsTrigger::IsActiveInEncounter()
{
    if (!GetEncounterBoss())
        return false;

    constexpr float outOfBoundsLeeway = 5.0f;
    return bot->GetPositionZ() < NIGHTBANE_GROUND_Z - outOfBoundsLeeway;
}

bool NightbaneShouldManageTimersAndTrackersTrigger::IsActiveInEncounter()
{
    return GetEncounterBoss();
}
