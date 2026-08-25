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

bool KarazhanBotIsNotInCombatTrigger::IsActive()
{
    return bot->GetMapId() == KARA_MAP_ID && !AI_VALUE2(bool, "combat", "self target");
}

bool KarazhanEnemiesCastFearTrigger::IsActive()
{
    if (bot->getClass() != CLASS_SHAMAN && bot->getClass() != CLASS_PRIEST)
        return false;

    return AI_VALUE2(Unit*, "find target", "17225") ||
        AI_VALUE2(Unit*, "find target", "15547") ||
        AI_VALUE2(Unit*, "find target", "17521");
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
bool AttumenTheHuntsmanPhaseOneActiveTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "16151") && !GetAttumenMounted(bot);
}

bool AttumenTheHuntsmanPhaseTwoActiveTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "16151") && GetAttumenMounted(bot);
}

bool AttumenTheHuntsmanPhaseTransitionTrigger::IsActive()
{
    if (!IsMechanicTrackerBot(botAI, bot, KARA_MAP_ID))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "16151"))
        return false;

    return GetAttumenMounted(bot);
}

// Moroes

bool MoroesShouldPrioritizeAddsTrigger::IsActive()
{
    return IsMechanicTrackerBot(botAI, bot, KARA_MAP_ID) && AI_VALUE2(Unit*, "find target", "15687");
}

// Maiden of Virtue

bool MaidenOfVirtueBossEngagedByTanksTrigger::IsActive()
{
    return PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "16457");
}

bool MaidenOfVirtueGroundingTotemConsumesHolyFireTrigger::IsActive()
{
    if (bot->getClass() != CLASS_SHAMAN)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "16457"))
        return false;

    return !AI_VALUE2(bool, "has totem", "grounding totem");
}

bool MaidenOfVirtueHolyWrathDealsChainDamageTrigger::IsActive()
{
    return PlayerbotAI::IsRanged(bot) && AI_VALUE2(Unit*, "find target", "16457");
}

// The Big Bad Wolf

bool BigBadWolfBossEngagedByTankTrigger::IsActive()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "17521"))
        return false;

    return !bot->HasAura(Id(KaraSpells::SPELL_LITTLE_RED_RIDING_HOOD));
}

bool BigBadWolfBossIsChasingLittleRedRidingHoodTrigger::IsActive()
{
    return bot->HasAura(Id(KaraSpells::SPELL_LITTLE_RED_RIDING_HOOD));
}

// Romulo and Julianne

bool RomuloAndJulianneBothBossesRevivedTrigger::IsActive()
{
    if (!IsMechanicTrackerBot(botAI, bot, KARA_MAP_ID))
        return false;

    return AI_VALUE2(Unit*, "find target", "17533") && AI_VALUE2(Unit*, "find target", "17534");
}

// The Wizard of Oz

bool WizardOfOzNeedTargetPriorityTrigger::IsActive()
{
    if (!IsMechanicTrackerBot(botAI, bot, KARA_MAP_ID))
        return false;

    for (const char* name : OZ_TARGETS)
    {
        if (AI_VALUE2(Unit*, "find target", name))
            return true;
    }

    return false;
}

bool WizardOfOzStrawmanIsVulnerableToFireTrigger::IsActive()
{
    return bot->getClass() == CLASS_MAGE && AI_VALUE2(Unit*, "find target", "17543");
}

// The Curator

bool TheCuratorAstralFlareSpawnedTrigger::IsActive()
{
    return IsMechanicTrackerBot(botAI, bot, KARA_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "17283");
}

bool TheCuratorBossEngagedByTanksTrigger::IsActive()
{
    return PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "15691");
}

bool TheCuratorBossEngagedByRangedTrigger::IsActive()
{
    return PlayerbotAI::IsRanged(bot) && AI_VALUE2(Unit*, "find target", "15691");
}

// Terestian Illhoof

bool TerestianIllhoofShouldPrioritizeChainsTrigger::IsActive()
{
    return IsMechanicTrackerBot(botAI, bot, KARA_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "15688");
}

// Shade of Aran

bool ShadeOfAranArcaneExplosionIsCastingTrigger::IsActive()
{
    Unit* aran = AI_VALUE2(Unit*, "find target", "16524");
    return aran && IsAranCastingArcaneExplosion(aran) && !IsFlameWreathActive(bot);
}

bool ShadeOfAranFlameWreathIsActiveTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "16524") && IsFlameWreathActive(bot);
}

bool ShadeOfAranConjuredElementalsSummonedTrigger::IsActive()
{
    return IsMechanicTrackerBot(botAI, bot, KARA_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "17167");
}

bool ShadeOfAranBossCastsCounterspellNearbyTrigger::IsActive()
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

bool NetherspiteRedBeamIsActiveTrigger::IsActive()
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "15689");
    if (!netherspite || IsBanishPhase(netherspite))
        return false;

    constexpr float searchRadius = 150.0f;
    return bot->FindNearestCreature(Id(KaraNpcs::NPC_RED_PORTAL), searchRadius);
}

bool NetherspiteBlueBeamIsActiveTrigger::IsActive()
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "15689");
    if (!netherspite || IsBanishPhase(netherspite))
        return false;

    constexpr float searchRadius = 150.0f;
    return bot->FindNearestCreature(Id(KaraNpcs::NPC_BLUE_PORTAL), searchRadius);
}

bool NetherspiteGreenBeamIsActiveTrigger::IsActive()
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "15689");
    if (!netherspite || IsBanishPhase(netherspite))
        return false;

    constexpr float searchRadius = 150.0f;
    return bot->FindNearestCreature(Id(KaraNpcs::NPC_GREEN_PORTAL), searchRadius);
}

bool NetherspiteBotIsNotBeamBlockerTrigger::IsActive()
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "15689");
    if (!netherspite || IsBanishPhase(netherspite))
        return false;

    auto [redBlocker, greenBlocker, blueBlocker] = GetCurrentBeamBlockers(bot);
    return bot != redBlocker && bot != blueBlocker && bot != greenBlocker;
}

bool NetherspiteBossIsBanishedTrigger::IsActive()
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "15689");
    return netherspite && IsBanishPhase(netherspite);
}

bool NetherspiteShouldManageTimersAndTrackersTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "15689");
}

// Prince Malchezaar

bool PrinceMalchezaarBotIsEnfeebledTrigger::IsActive()
{
    return bot->HasAura(Id(KaraSpells::SPELL_ENFEEBLE));
}

bool PrinceMalchezaarEngagedByNonTanksTrigger::IsActive()
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

bool PrinceMalchezaarBossEngagedByTanksTrigger::IsActive()
{
    Unit* malchezaar = AI_VALUE2(Unit*, "find target", "15690");
    if (!malchezaar)
        return false;

    return (PlayerbotAI::IsTank(bot) && malchezaar->GetVictim() == bot) ||
        PlayerbotAI::IsMainTank(bot);
}

// Nightbane

bool NightbaneBossEngagedByTanksTrigger::IsActive()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* nightbane = AI_VALUE2(Unit*, "find target", "17225");
    return nightbane && nightbane->GetPositionZ() <= NIGHTBANE_FLIGHT_Z;
}

bool NightbaneGroundPhaseEngagedByRangedTrigger::IsActive()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* nightbane = AI_VALUE2(Unit*, "find target", "17225");
    return nightbane && nightbane->GetPositionZ() <= NIGHTBANE_FLIGHT_Z;
}

bool NightbanePetsIgnoreCollisionToChaseFlyingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER && bot->getClass() != CLASS_WARLOCK)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "17225"))
        return false;

    Pet* pet = bot->GetPet();
    return pet && pet->IsAlive();
}

bool NightbaneBossIsFlyingTrigger::IsActive()
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "17225");
    if (!nightbane || nightbane->GetPositionZ() <= NIGHTBANE_FLIGHT_Z)
        return false;

    uint32 const instanceId = nightbane->GetMap()->GetInstanceId();
    time_t const now = std::time(nullptr);
    constexpr uint8 flightPhaseDurationSeconds = 35;
    // After 35s, Nightbane goes to land, and bots freely follow their master
    if (nightbaneFlightPhaseStartTimer.find(instanceId) == nightbaneFlightPhaseStartTimer.end())
        return false;

    return now - nightbaneFlightPhaseStartTimer[instanceId] < flightPhaseDurationSeconds;
}

bool NightbaneBotWentOutOfBoundsTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "17225"))
        return false;

    constexpr float outOfBoundsLeeway = 5.0f;
    return bot->GetPositionZ() < NIGHTBANE_GROUND_Z - outOfBoundsLeeway;
}

bool NightbaneShouldManageTimersAndTrackersTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "17225");
}
