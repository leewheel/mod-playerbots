/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "TKTriggers.h"
#include "EncounterHelpers.h"
#include "MoveSpline.h"
#include "Playerbots.h"
#include "TKHelpers.h"
#include <array>

using namespace TkHelpers;
using namespace EncounterHelpers;

// General

bool TempestKeepNoEncounterInProgressTrigger::IsActive()
{
    if (IsEncounterInProgress(bot, TK_MAP_ID))
        return false;

    return IsMechanicTrackerBot(bot, TK_MAP_ID);
}

bool TempestKeepStuckFallingTrigger::IsActive()
{
    if (!bot->HasUnitMovementFlag(MOVEMENTFLAG_FALLING) || !bot->movespline->Finalized())
        return false;

    if (bot->GetMapId() != TK_MAP_ID)
        return false;

    return !IsEncounterInProgress(bot, TK_MAP_ID);
}

// Trash

bool CrimsonHandCenturionCastsArcaneFlurryTrigger::IsActive()
{
// By leewheel 2026-09-13 合并brighton caa4094e: 采纳上游 1749c628 的 Centurion 判定写法 ——
//   改用 GetCenturionCastingArcaneFlurry(botAI)（基于 attackers 值 + 施法状态判定，比原先
//   单纯的 "find target 19510"(血手百夫长) 存活查找更精确）。该 helper 的定义见 TK/Util/TKHelpers。
// End By leewheel
    return bot->getClass() == CLASS_MAGE && GetCenturionCastingArcaneFlurry(botAI);
}

// Al'ar <Phoenix God>

bool AlarPullingBossTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    // By leewheel 2026-08-30 合并上游：HP常量统一BOSS_ENGAGED_HEALTH_PCT；entry规则查怪(19514=al'ar)
    Unit* alar = AI_VALUE2(Unit*, "find target", "19514");
    return alar && alar->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT;
    // End By leewheel
}

bool AlarFliesBetweenPlatformsTrigger::IsActiveInEncounter()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "19514");
    if (!alar || IsAlarInPhase2(alar->GetInstanceId()))
        return false;

    int8 locationIndex = GetAlarCurrentLocationIndex(alar);
    if (locationIndex == LOCATION_NONE)
        locationIndex = GetAlarDestinationLocationIndex(alar);

    return locationIndex != POINT_QUILL_OR_DIVE_IDX && locationIndex != POINT_MIDDLE_IDX;
}

bool AlarEmbersExplodeUponDeathTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "19551");
}

bool AlarShouldAssignNonTankTargetTrigger::IsActiveInEncounter()
{
// By leewheel 2026-09-05 合并：采纳上游"非坦克分配目标"的新逻辑，A'l'ar按entry规则查找(19514)
    return !PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "19514");
    // End By leewheel
}

bool AlarIncomingFlameQuillsTrigger::IsActiveInEncounter()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "19514");
    if (!alar || IsAlarInPhase2(alar->GetInstanceId()))
        return false;

    return GetAlarCurrentLocationIndex(alar) == POINT_QUILL_OR_DIVE_IDX ||
        GetAlarDestinationLocationIndex(alar) == POINT_QUILL_OR_DIVE_IDX;
}

bool AlarRisingFromTheAshesTrigger::IsActiveInEncounter()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "19514");
    if (!alar || alar->GetHealthPct() > 5.0f)
        return false;

    if (IsAlarInPhase2(alar->GetInstanceId()))
        return false;

    return GetAlarCurrentLocationIndex(alar) != POINT_QUILL_OR_DIVE_IDX &&
        GetAlarDestinationLocationIndex(alar) != POINT_QUILL_OR_DIVE_IDX;
}

bool AlarInPhase2Trigger::IsActiveInEncounter()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "19514");
    return alar && IsAlarInPhase2(alar->GetInstanceId());
}

bool AlarShouldManagePhaseTrackerTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, TK_MAP_ID) && AI_VALUE2(Unit*, "find target", "19514");
}

// Void Reaver

bool VoidReaverShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "19516");
}

bool VoidReaverKnockAwayPullsAggroToNonTanksTrigger::IsActiveInEncounter()
{
    if (bot->getClass() == CLASS_DEATH_KNIGHT || bot->getClass() == CLASS_DRUID ||
        bot->getClass() == CLASS_SHAMAN || bot->getClass() == CLASS_WARRIOR)
    {
        return false;
    }

    if (PlayerbotAI::IsTank(bot))
        return false;

    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "19516");
    return voidReaver && voidReaver->GetVictim() == bot;
}

bool VoidReaverRangedShouldStandBackTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "19516");
    if (!voidReaver || voidReaver->GetVictim() == bot)
        return false;

    return !IsNearActiveArcaneOrb(bot, ARCANE_ORB_BUFFER_DISTANCE);
}

bool VoidReaverArcaneOrbIsIncomingTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsTank(bot))
        return false;

    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "19516");
    if (!voidReaver || voidReaver->GetVictim() == bot)
        return false;

    return IsNearActiveArcaneOrb(bot, ARCANE_ORB_SAFE_DISTANCE);
}

// High Astromancer Solarian

bool HighAstromancerSolarianShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMainTank(bot))
        return false;

    Unit* astromancer = AI_VALUE2(Unit*, "find target", "18805");
    if (!astromancer)
        return false;

    Creature* astromancerCreature = astromancer->ToCreature();
    return astromancerCreature && astromancerCreature->GetReactState() != REACT_PASSIVE;
}

bool HighAstromancerSolarianWrathOfTheAstromancerTrigger::IsActiveInEncounter()
{
    return HasWrathOfTheAstromancer(bot);
}

bool HighAstromancerSolarianSolariumPriestsSpawnedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot) || PlayerbotAI::IsMainTank(bot))
        return false;

    return AI_VALUE2(Unit*, "find target", "18806");
}

// Kael'thas Sunstrider <Lord of the Blood Elves>

bool KaelthasSunstriderChasedByThaladredTrigger::IsActiveInEncounter()
{
    Unit* thaladred = AI_VALUE2(Unit*, "find target", "20064");
    if (!thaladred || thaladred->GetVictim() != bot)
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "19622");
    if (!kaelthas)
        return false;

    uint32 const phase = GetKaelthasTkPhase(kaelthas);
    if (PlayerbotAI::IsTank(bot) && phase == PHASE_ALL_ADVISORS)
        return false;

    return phase != PHASE_NONE;
}

bool KaelthasSunstriderPullingTankableAdvisorsTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "19622");
    if (!kaelthas)
        return false;

    uint32 const phase = GetKaelthasTkPhase(kaelthas);
    return phase == PHASE_SINGLE_ADVISOR || phase == PHASE_ALL_ADVISORS;
}

bool KaelthasSunstriderSanguinarOrTelonicusShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (PlayerbotAI::IsMainTank(bot))
        return IsAdvisorActive(AI_VALUE2(Unit*, "find target", "20060"));

    if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
        return IsAdvisorActive(AI_VALUE2(Unit*, "find target", "20063"));

    return false;
}

bool KaelthasSunstriderCapernianShouldBeTankedByWarlockTrigger::IsActiveInEncounter()
{
    if (!IsCapernianTank(bot))
        return false;

    return IsAdvisorActive(AI_VALUE2(Unit*, "find target", "20062"));
}

bool KaelthasSunstriderShouldStandBackFromCapernianTrigger::IsActiveInEncounter()
{
    if (!IsAdvisorActive(AI_VALUE2(Unit*, "find target", "20062")))
        return false;

    return !IsCapernianTank(bot);
}

bool KaelthasSunstriderShouldHoldPhase3PositionsTrigger::IsActiveInEncounter()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "19622");
    if (!kaelthas)
        return false;

    if (GetKaelthasTkPhase(kaelthas) != PHASE_ALL_ADVISORS)
        return false;

    // The designated healer stays in position by the melee tanks while the rest of the raid runs
    // all over the place to kite and kill Thaladred.
    // By leewheel 2026-09-13 合并brighton 8c96a663: 采纳上游注释措辞，保留 rule81 entry 化
    // （20060 = 萨古纳尔男爵 Lord Sanguinar）
    Unit* sanguinar = AI_VALUE2(Unit*, "find target", "20060");
    if (PlayerbotAI::IsAssistHealOfIndex(bot, 0, true))
        return sanguinar && sanguinar->IsAlive();

    // The Sanguinar check is a proxy for the revival/Kael talk phase (any non-selectable advisor
    // would do, since all four revive together, but Sanguinar is already needed for the healer).
    if (!sanguinar || !sanguinar->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
        return false;

    return PlayerbotAI::IsMainTank(bot) || PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) ||
        IsCapernianTank(bot);
}

bool KaelthasSunstriderDeterminingAdvisorKillOrderTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsMainTank(bot) || PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "19622");
    if (!kaelthas)
        return false;

    uint32 const phase = GetKaelthasTkPhase(kaelthas);
    return phase == PHASE_SINGLE_ADVISOR || phase == PHASE_ALL_ADVISORS;
}

bool KaelthasSunstriderShouldManageAdvisorDpsTimerTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, TK_MAP_ID))
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "19622");
    if (!kaelthas)
        return false;

    return GetKaelthasTkPhase(kaelthas) == PHASE_SINGLE_ADVISOR;
}

bool KaelthasSunstriderLegendaryWeaponsAreAliveTrigger::IsActiveInEncounter()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "19622");
    if (!kaelthas)
        return false;

    if (GetKaelthasTkPhase(kaelthas) != PHASE_WEAPONS)
        return false;

    return !PlayerbotAI::IsMainTank(bot);
}

bool KaelthasSunstriderLegendaryAxeCastsWhirlwindTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMainTank(bot))
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "19622");
    if (!kaelthas)
        return false;

    uint32 const phase = GetKaelthasTkPhase(kaelthas);
    if (phase < PHASE_WEAPONS || phase > PHASE_ALL_ADVISORS)
        return false;

    return GetLegendaryWeapon(botAI, Id(TkNpcs::NPC_DEVASTATION)) != nullptr;
}

bool KaelthasSunstriderLegendaryWeaponsAreDeadTrigger::IsActiveInEncounter()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "19622");
    if (!kaelthas)
        return false;

    uint32 const phase = GetKaelthasTkPhase(kaelthas);
    if (phase < PHASE_WEAPONS || phase > PHASE_ALL_ADVISORS)
        return false;

    Unit* axe = GetLegendaryWeapon(botAI, Id(TkNpcs::NPC_DEVASTATION));
    if (axe && axe->GetVictim() == bot)
        return false;

    return HasDeadLegendaryWeapon(botAI);
}

bool KaelthasSunstriderLegendaryWeaponsAreEquippedTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsHeal(bot))
        return false;

    if (PlayerbotAI::IsMelee(bot) && PlayerbotAI::IsDps(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "19622"))
        return false;

    return GetEquippedItemInSlot(
               bot, EQUIPMENT_SLOT_MAINHAND, Id(TkItems::ITEM_STAFF_OF_DISINTEGRATION)) ||
        GetEquippedItemInSlot(
               bot, EQUIPMENT_SLOT_RANGED, Id(TkItems::ITEM_NETHERSTRAND_LONGBOW)) ||
        GetEquippedItemInSlot(
               bot, EQUIPMENT_SLOT_OFFHAND, Id(TkItems::ITEM_PHASESHIFT_BULWARK));
}

bool KaelthasSunstriderLegendaryWeaponsWereLostTrigger::IsActive()
{
    if (bot->GetMapId() != TK_MAP_ID)
        return false;

    if (IsEncounterInProgress(bot, TK_MAP_ID))
        return false;

    if (AI_VALUE2(bool, "combat", "self target"))
        return false;

    auto const& creatureStore = bot->GetMap()->GetCreatureBySpawnIdStore();
    auto it = creatureStore.find(KAELTHAS_DB_GUID);
    if (it == creatureStore.end())
        return false;

    Creature* kaelthas = it->second;
    if (!kaelthas || bot->GetExactDist2d(kaelthas) > KAELTHAS_ROOM_SEARCH_DISTANCE)
        return false;

    static constexpr std::array weaponSlots = {
        EQUIPMENT_SLOT_MAINHAND, EQUIPMENT_SLOT_OFFHAND, EQUIPMENT_SLOT_RANGED, };

    for (uint8 slot : weaponSlots)
    {
        if (!bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot) && HasEquippableItemForSlot(bot, slot))
            return true;
    }

    return false;
}

bool KaelthasSunstriderHasEnteredTheFightTrigger::IsActiveInEncounter()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "19622");
    if (!kaelthas)
        return false;

    return GetKaelthasTkPhase(kaelthas) == PHASE_FINAL;
}

bool KaelthasSunstriderShouldAssignFinalPhaseTargetTrigger::IsActiveInEncounter()
{
// By leewheel 2026-09-05 合并：采纳上游 GetKaelthasTk/GetKaelthasTkPhase(GetKaelthasPhase已更名)，简化末盘目标分配判断
    if (PlayerbotAI::IsMainTank(bot))
        return false;

    // By leewheel 2026-09-12 按 AGENTS.md 第81条修正历史遗留英文名: 19622 = 凯尔萨斯·逐日者(风暴要塞)
    // End By leewheel
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "19622");
    if (!kaelthas || kaelthas->GetVictim() == bot)
        return false;
    // End By leewheel

    return GetKaelthasTkPhase(kaelthas) == PHASE_FINAL;
}

bool KaelthasSunstriderRaidMemberIsMindControlledTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsCaster(bot))
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "19622");
    if (!kaelthas)
        return false;

    if (PlayerbotAI::IsTank(bot) && kaelthas->GetVictim() == bot)
        return false;

    if (!bot->HasItemCount(Id(TkItems::ITEM_INFINITY_BLADE), 1, true))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->HasAura(Id(TkSpells::SPELL_KAELTHAS_MIND_CONTROL)))
            return true;
    }

    return false;
}

bool KaelthasSunstriderInGravityLapsePhaseTrigger::IsActiveInEncounter()
{
    constexpr float gravityLapseHpThreshold = 50.0f;
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "19622");
    return kaelthas && kaelthas->GetHealthPct() <= gravityLapseHpThreshold;
}
