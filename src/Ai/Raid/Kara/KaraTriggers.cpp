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

using namespace KaraHelpers;
using namespace EncounterHelpers;

// General

bool KarazhanNoEncounterInProgressTrigger::IsActive()
{
    return !IsEncounterInProgress(bot, KARA_MAP_ID);
}

bool KarazhanEnemiesCastFearTrigger::IsActive()
{
    if (bot->getClass() != CLASS_SHAMAN)
        return false;

// By leewheel 2026-09-05 合并：采纳上游颤抖图腾与Nightbane飞行高度新逻辑，boss按entry规则查找
    if (AI_VALUE2(bool, "has totem", "tremor totem"))
        return false;

// By leewheel 2026-09-27 合并brighton the-lab：brighton 用名字（nightbane/spectral charger/大灰狼/roar），
    //   本分支已按规则第97条 entry 化（17225=Nightbane, 15547=Spectral Charger, 17521=Big Bad Wolf），保留。
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "17225");
    return (nightbane && nightbane->GetPositionZ() <= NIGHTBANE_FLIGHT_Z) ||
        AI_VALUE2(Unit*, "find target", "15547") ||
        AI_VALUE2(Unit*, "find target", "17521");
    // End By leewheel
}

// Trash

bool ManaWarpIsAboutToExplodeTrigger::IsActive()
{
    if (bot->getClass() == CLASS_DEATH_KNIGHT || bot->getClass() == CLASS_HUNTER ||
        bot->getClass() == CLASS_MAGE || bot->getClass() == CLASS_PRIEST)
    {
        return false;
    }

    return AI_VALUE2(Unit*, "find target", "16530");
}

// Attumen the Huntsman

// Midnight is still present as a separate (invisible) unit after Attumen mounts.
// A Midnight threat list check will capture the entire encounter.
bool AttumenTheHuntsmanPhaseOneActiveTrigger::IsActiveInEncounter()
{
    return AI_VALUE2(Unit*, "find target", "16151") && !GetAttumenMounted(bot);
}

bool AttumenTheHuntsmanPhaseTwoActiveTrigger::IsActiveInEncounter()
{
    return AI_VALUE2(Unit*, "find target", "16151") && GetAttumenMounted(bot);
}

bool AttumenTheHuntsmanPhaseTransitionTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(botAI, bot, KARA_MAP_ID))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "16151"))
        return false;

    return GetAttumenMounted(bot);
}

// Moroes

bool MoroesShouldPrioritizeAddsTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(botAI, bot, KARA_MAP_ID) && AI_VALUE2(Unit*, "find target", "15687");
}

// Maiden of Virtue

bool MaidenOfVirtueShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "16457");
}

bool MaidenOfVirtueGroundingTotemConsumesHolyFireTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_SHAMAN)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "16457"))
        return false;

    return !AI_VALUE2(bool, "has totem", "grounding totem");
}

bool MaidenOfVirtueRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsRanged(bot) && AI_VALUE2(Unit*, "find target", "16457");
}

// The Big Bad Wolf

bool BigBadWolfShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "17521"))
        return false;

    return !bot->HasAura(Id(KaraSpells::SPELL_LITTLE_RED_RIDING_HOOD));
}

bool BigBadWolfChasingLittleRedRidingHoodTrigger::IsActiveInEncounter()
{
    return bot->HasAura(Id(KaraSpells::SPELL_LITTLE_RED_RIDING_HOOD));
}

// Romulo and Julianne

bool RomuloAndJulianneBothBossesRevivedTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(botAI, bot, KARA_MAP_ID))
        return false;

    return AI_VALUE2(Unit*, "find target", "17533") && AI_VALUE2(Unit*, "find target", "17534");
}

// The Wizard of Oz

bool WizardOfOzNeedTargetPriorityTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(botAI, bot, KARA_MAP_ID))
        return false;

    for (char const* name : OZ_TARGETS)
    {
        if (AI_VALUE2(Unit*, "find target", name))
            return true;
    }

    return false;
}

bool WizardOfOzStrawmanIsVulnerableToFireTrigger::IsActiveInEncounter()
{
    return bot->getClass() == CLASS_MAGE && AI_VALUE2(Unit*, "find target", "17543");
}

// The Curator

bool TheCuratorAstralFlareSpawnedTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(botAI, bot, KARA_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "17283");
}

bool TheCuratorShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "15691");
}

bool TheCuratorRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsRanged(bot) && AI_VALUE2(Unit*, "find target", "15691");
}

// Terestian Illhoof

bool TerestianIllhoofShouldPrioritizeChainsTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(botAI, bot, KARA_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "15688");
}

// Shade of Aran

bool ShadeOfAranArcaneExplosionIsCastingTrigger::IsActiveInEncounter()
{
    Unit* aran = AI_VALUE2(Unit*, "find target", "16524");
    return aran && IsAranCastingArcaneExplosion(aran) && !IsFlameWreathActive(bot);
}

bool ShadeOfAranFlameWreathIsActiveTrigger::IsActiveInEncounter()
{
    return AI_VALUE2(Unit*, "find target", "16524") && IsFlameWreathActive(bot);
}

bool ShadeOfAranConjuredElementalsSummonedTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(botAI, bot, KARA_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "17167");
}

bool ShadeOfAranRangedShouldMaintainDistanceTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* aran = AI_VALUE2(Unit*, "find target", "16524");
    if (!aran)
        return false;

    if (bot->HasAura(Id(KaraSpells::SPELL_BLIZZARD)))
        return false;

    return !IsAranCastingArcaneExplosion(aran) && !IsFlameWreathActive(bot);
}

// Netherspite

bool NetherspiteRedBeamIsActiveTrigger::IsActiveInEncounter()
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "15689");
    if (!netherspite || IsBanishPhase(netherspite))
        return false;

    constexpr float searchRadius = 150.0f;
    return bot->FindNearestCreature(Id(KaraNpcs::NPC_RED_PORTAL), searchRadius);
}

bool NetherspiteBlueBeamIsActiveTrigger::IsActiveInEncounter()
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "15689");
    if (!netherspite || IsBanishPhase(netherspite))
        return false;

    constexpr float searchRadius = 150.0f;
    return bot->FindNearestCreature(Id(KaraNpcs::NPC_BLUE_PORTAL), searchRadius);
}

bool NetherspiteGreenBeamIsActiveTrigger::IsActiveInEncounter()
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "15689");
    if (!netherspite || IsBanishPhase(netherspite))
        return false;

    constexpr float searchRadius = 150.0f;
    return bot->FindNearestCreature(Id(KaraNpcs::NPC_GREEN_PORTAL), searchRadius);
}

bool NetherspiteBotIsNotBeamBlockerTrigger::IsActiveInEncounter()
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "15689");
    if (!netherspite || IsBanishPhase(netherspite))
        return false;

    auto [redBlocker, greenBlocker, blueBlocker] = GetCurrentBeamBlockers(bot);
    return bot != redBlocker && bot != blueBlocker && bot != greenBlocker;
}

bool NetherspiteInBanishPhaseTrigger::IsActiveInEncounter()
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "15689");
    return netherspite && IsBanishPhase(netherspite);
}

bool NetherspiteShouldManageTimersAndTrackersTrigger::IsActiveInEncounter()
{
    return AI_VALUE2(Unit*, "find target", "15689");
}

// Prince Malchezaar

bool PrinceMalchezaarBotIsEnfeebledTrigger::IsActiveInEncounter()
{
    return bot->HasAura(Id(KaraSpells::SPELL_ENFEEBLE));
}

bool PrinceMalchezaarEngagedByNonTanksTrigger::IsActiveInEncounter()
{
    Unit* malchezaar = AI_VALUE2(Unit*, "find target", "15690");
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
    Unit* malchezaar = AI_VALUE2(Unit*, "find target", "15690");
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

    Unit* nightbane = AI_VALUE2(Unit*, "find target", "17225");
    return nightbane && nightbane->GetPositionZ() <= NIGHTBANE_FLIGHT_Z;
}

bool NightbaneGroundPhaseEngagedByRangedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* nightbane = AI_VALUE2(Unit*, "find target", "17225");
    return nightbane && nightbane->GetPositionZ() <= NIGHTBANE_FLIGHT_Z;
}

bool NightbanePetsChaseFlyingBossOutOfBoundsTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER && bot->getClass() != CLASS_WARLOCK)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "17225"))
        return false;

    Pet* pet = bot->GetPet();
    return pet && pet->IsAlive();
}

bool NightbaneInFlightPhaseTrigger::IsActiveInEncounter()
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "17225");
    if (!nightbane || nightbane->GetPositionZ() <= NIGHTBANE_FLIGHT_Z)
        return false;

    // By leewheel 2026-08-29 合并：采用对侧GetMSTimeDiff毫秒制计时(与后续逻辑配套)，entry规则不变
    constexpr uint32 flightPhaseDurationMs = 35 * IN_MILLISECONDS;
    // End By leewheel
    // After 35s, Nightbane goes to land, and bots freely follow their master
    auto const it = nightbaneFlightPhaseStartTimer.find(nightbane->GetInstanceId());
    if (it == nightbaneFlightPhaseStartTimer.end())
        return false;

    return getMSTimeDiff(it->second, getMSTime()) < flightPhaseDurationMs;
}

bool NightbaneBotWentOutOfBoundsTrigger::IsActiveInEncounter()
{
    if (!AI_VALUE2(Unit*, "find target", "17225"))
        return false;

    constexpr float outOfBoundsLeeway = 5.0f;
    return bot->GetPositionZ() < NIGHTBANE_GROUND_Z - outOfBoundsLeeway;
}

bool NightbaneShouldManageTimersAndTrackersTrigger::IsActiveInEncounter()
{
    return AI_VALUE2(Unit*, "find target", "17225");
}
