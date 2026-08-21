/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GruulTriggers.h"
#include "GruulHelpers.h"
#include "Playerbots.h"

using namespace GruulHelpers;

// General

bool GruulsLairBotIsNotInCombatTrigger::IsActive()
{
    return bot->GetMapId() == GRUUL_MAP_ID && !AI_VALUE2(bool, "combat", "self target");
}

// High King Maulgar

bool HighKingMaulgarBossesEngagedByMeleeTanksTrigger::IsActive()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "18831"))
        return false;

    return IsMaulgarTank(bot) || IsOlmTank(bot) || IsBlindeyeTank(bot);
}

bool HighKingMaulgarKroshEngagedByMageTankTrigger::IsActive()
{
    if (bot->getClass() != CLASS_MAGE)
        return false;

    return AI_VALUE2(Unit*, "find target", "18832") && GetKroshMageTank(bot) == bot;
}

bool HighKingMaulgarKigglerEngagedByMoonkinTankTrigger::IsActive()
{
    if (bot->getClass() != CLASS_DRUID)
        return false;

    return AI_VALUE2(Unit*, "find target", "18835") &&
        GetKigglerMoonkinTank(bot) == bot;
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

    if (bot->getClass() == CLASS_MAGE && GetKroshMageTank(bot) == bot)
        return !AI_VALUE2(Unit*, "find target", "18832");

    if (bot->getClass() == CLASS_DRUID && GetKigglerMoonkinTank(bot) == bot)
        return !AI_VALUE2(Unit*, "find target", "18835");

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
    if (!AI_VALUE2(Unit*, "find target", "18832"))
        return false;

    return !PlayerbotAI::IsTank(bot) && GetKroshMageTank(bot) != bot;
}

bool HighKingMaulgarWildFelStalkerSpawnedTrigger::IsActive()
{
    return bot->getClass() == CLASS_WARLOCK && AI_VALUE2(Unit*, "find target", "18847");
}

bool HighKingMaulgarPullingOgreCouncilTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* blindeye = AI_VALUE2(Unit*, "find target", "18836");
    return blindeye && blindeye->GetHealthPct() > 80.0f;
}

// Gruul the Dragonkiller

bool GruulTheDragonkillerBossEngagedByTanksTrigger::IsActive()
{
    return PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "19044");
}

bool GruulTheDragonkillerRangedShouldSpreadTrigger::IsActive()
{
    return PlayerbotAI::IsRanged(bot) && AI_VALUE2(Unit*, "find target", "19044");
}

bool GruulTheDragonkillerIncomingShatterTrigger::IsActive()
{
    return bot->HasAura(Id(GruulSpells::SPELL_GROUND_SLAM));
}
