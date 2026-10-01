/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RampTriggers.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "RampBossHelper.h"
#include "RampShared.h"

using namespace EncounterHelpers;
using namespace RampShared;

// Watchkeeper Gargolmar

bool GargolmarHellfireWatchersAreActiveTrigger::IsActive()
{
    return  IsMechanicTrackerBot(bot, RAMP_MAP_ID) &&
            AI_VALUE2(Unit*, "find target", "17309");
}

// Omor the Unscarred

bool OmorTreacheryAuraTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "17308"))
        return false;

    if (!helper.HasTreacheryAura(bot))
        return false;

    return !PlayerbotAI::IsMainTank(bot);
}

bool OmorTankHasTreacheryAuraTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "17308"))
        return false;

    Player* tank = GetGroupMainTank(bot);

    if (!tank || tank == bot)
        return false;

    return helper.HasTreacheryAura(tank);
}

bool OmorRangedSpreadTrigger::IsActive()
{
    return  PlayerbotAI::IsRanged(bot) &&
            GetNearestPlayerInRadius(bot, OMOR_TREACHERY_AURA_SAFE_DISTANCE) &&
            AI_VALUE2(Unit*, "find target", "17308");
}

bool OmorFiendishHoundIsActiveTrigger::IsActive()
{
    return  IsMechanicTrackerBot(bot, RAMP_MAP_ID) &&
            AI_VALUE2(Unit*, "find target", "17280");
}

// Vazruden & Nazan

bool VazrudenTankPositionBossTrigger::IsActive()
{
    Unit* nazan = AI_VALUE2(Unit*, "find target", "17536");

    return
        PlayerbotAI::IsMainTank(bot) &&
        AI_VALUE2(Unit*, "find target", "17537") &&
        (!nazan || nazan->IsFlying());
}

bool VazrudenBossIsActiveTrigger::IsActive()
{
    return  IsMechanicTrackerBot(bot, RAMP_MAP_ID) &&
            AI_VALUE2(Unit*, "find target", "17537");
}

bool NazanBossTremorTotemTrigger::IsActive()
{
    if (bot->getClass() != CLASS_SHAMAN)
        return false;

    Unit* nazan = AI_VALUE2(Unit*, "find target", "17536");

    if (!nazan || nazan->IsFlying())
        return false;

    Map* map = nazan->GetMap();
    if (!map || !map->IsHeroic())
        return false;

    return !AI_VALUE2(bool, "has totem", "tremor totem");
}

bool NazanBossFireResistanceTotemTrigger::IsActive()
{
    if (bot->getClass() != CLASS_SHAMAN)
        return false;

    Unit* nazan = AI_VALUE2(Unit*, "find target", "17536");

    if (!nazan || nazan->IsFlying())
        return false;

    return !AI_VALUE2(bool, "has totem", "fire resistance totem");
}

bool NazanBossFireResistanceAuraTrigger::IsActive()
{
    if (bot->getClass() != CLASS_PALADIN)
        return false;

    Unit* nazan = AI_VALUE2(Unit*, "find target", "17536");

    if (!nazan || nazan->IsFlying())
        return false;

    return !botAI->HasAura("fire resistance aura", bot);
}
