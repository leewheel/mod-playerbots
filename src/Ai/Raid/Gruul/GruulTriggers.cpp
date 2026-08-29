/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GruulTriggers.h"
#include "GruulHelpers.h"
#include "InstanceScript.h"
#include "Playerbots.h"

using namespace GruulHelpers;

// General

bool GruulsLairNoEncounterInProgress::IsActive()
{
    if (bot->GetMapId() != GRUUL_MAP_ID)
        return false;

    InstanceScript* instance = bot->GetInstanceScript();
    return instance && !instance->IsEncounterInProgress();
}

// High King Maulgar

bool HighKingMaulgarThreeOgresNeedMeleeTanksTrigger::IsActive()
{
    if (IsBlindeyeTank(bot))
        return AI_VALUE2(Unit*, "find target", "blindeye the seer");

<<<<<<< HEAD
    if (!AI_VALUE2(Unit*, "find target", "18831"))
        return false;
=======
    if (IsOlmTank(bot))
        return AI_VALUE2(Unit*, "find target", "olm the summoner");
>>>>>>> brighton-chi/the-lab

    return IsMaulgarTank(bot) && AI_VALUE2(Unit*, "find target", "high king maulgar");
}

bool HighKingMaulgarKroshNeedsMageTankTrigger::IsActive()
{
<<<<<<< HEAD
    if (bot->getClass() != CLASS_MAGE)
        return false;

    return AI_VALUE2(Unit*, "find target", "18832") && GetKroshMageTank(bot) == bot;
=======
    return IsKroshMageTank(bot) && AI_VALUE2(Unit*, "find target", "krosh firehand");
>>>>>>> brighton-chi/the-lab
}

bool HighKingMaulgarKigglerNeedsMoonkinTankTrigger::IsActive()
{
<<<<<<< HEAD
    if (bot->getClass() != CLASS_DRUID)
        return false;

    return AI_VALUE2(Unit*, "find target", "18835") &&
        GetKigglerMoonkinTank(bot) == bot;
=======
    return IsKigglerMoonkinTank(bot) && AI_VALUE2(Unit*, "find target", "kiggler the crazed");
>>>>>>> brighton-chi/the-lab
}

bool HighKingMaulgarDeterminingKillOrderTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "18831"))
        return false;

    if (IsMaulgarTank(bot))
        return false;

    if (IsOlmTank(bot))
        return !AI_VALUE2(Unit*, "find target", "18834");

    if (IsBlindeyeTank(bot))
        return !AI_VALUE2(Unit*, "find target", "18836");

<<<<<<< HEAD
    if (bot->getClass() == CLASS_MAGE && GetKroshMageTank(bot) == bot)
        return !AI_VALUE2(Unit*, "find target", "18832");

    if (bot->getClass() == CLASS_DRUID && GetKigglerMoonkinTank(bot) == bot)
        return !AI_VALUE2(Unit*, "find target", "18835");
=======
    if (IsKroshMageTank(bot))
        return !AI_VALUE2(Unit*, "find target", "krosh firehand");

    if (IsKigglerMoonkinTank(bot))
        return !AI_VALUE2(Unit*, "find target", "kiggler the crazed");
>>>>>>> brighton-chi/the-lab

    return true;
}

bool HighKingMaulgarBossChannelingWhirlwindTrigger::IsActive()
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "18831");
    if (!maulgar || !maulgar->HasAura(Id(GruulSpells::SPELL_WHIRLWIND)))
        return false;

    return !IsMaulgarTank(bot);
}

bool HighKingMaulgarKroshCastsBlastWaveTrigger::IsActive()
{
<<<<<<< HEAD
    if (!AI_VALUE2(Unit*, "find target", "18832"))
=======
    if (PlayerbotAI::IsTank(bot) || IsKroshMageTank(bot))
>>>>>>> brighton-chi/the-lab
        return false;

    return AI_VALUE2(Unit*, "find target", "krosh firehand");
}

bool HighKingMaulgarWildFelStalkerSpawnedTrigger::IsActive()
{
    return bot->getClass() == CLASS_WARLOCK && AI_VALUE2(Unit*, "find target", "18847");
}

bool HighKingMaulgarPullingOgreCouncilTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

<<<<<<< HEAD
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "18836");
    return blindeye && blindeye->GetHealthPct() > 80.0f;
=======
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");
    return blindeye && blindeye->GetHealthPct() > BLINDEYE_PULL_COMPLETE_HP_PERCENT;
}

bool HighKingMaulgarBossCastsIntimidatingRoarTrigger::IsActive()
{
    return bot->getClass() == CLASS_PRIEST && AI_VALUE2(Unit*, "find target", "high king maulgar");
>>>>>>> brighton-chi/the-lab
}

// Gruul the Dragonkiller

bool GruulTheDragonkillerShouldBeTankedTrigger::IsActive()
{
    return PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "19044");
}

bool GruulTheDragonkillerRangedShouldSpreadTrigger::IsActive()
{
    return PlayerbotAI::IsRanged(bot) && AI_VALUE2(Unit*, "find target", "19044");
}

bool GruulTheDragonkillerIncomingShatterTrigger::IsActive()
{
    return HasGroundSlam(bot);
}
