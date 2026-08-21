/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ZATriggers.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"
#include "ZAActions.h"
#include "ZAHelpers.h"

using namespace ZulAmanHelpers;

// Trash

bool AmanishiMedicineManSummonedWardTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "23581");
}

// Akil'zon <Eagle Avatar>

bool AkilzonPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* akilzon = AI_VALUE2(Unit*, "find target", "23574");
    return akilzon && akilzon->GetHealthPct() > 95.0f;
}

bool AkilzonBossEngagedByTanksTrigger::IsActive()
{
    if (!botAI->IsTank(bot) ||
        !AI_VALUE2(Unit*, "find target", "23574"))
        return false;

    return !GetElectricalStormTarget(bot);
}

bool AkilzonBossCastsStaticDisruptionTrigger::IsActive()
{
    if (!botAI->IsRanged(bot) ||
        !AI_VALUE2(Unit*, "find target", "23574"))
        return false;

    auto it = akilzonStormTimer.find(bot->GetMap()->GetInstanceId());
    if (it == akilzonStormTimer.end())
        return true;

    return !IsInStormWindow(it->second, std::time(nullptr));
}

bool AkilzonElectricalStormIncomingTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "23574"))
        return false;

    auto it = akilzonStormTimer.find(bot->GetMap()->GetInstanceId());
    if (it == akilzonStormTimer.end())
        return false;

    return IsInStormWindow(it->second, std::time(nullptr));
}

bool AkilzonBotsNeedToPrepareForElectricalStormTrigger::IsActive()
{
    return IsMechanicTrackerBot(botAI, bot, ZULAMAN_MAP_ID, nullptr);
}

// Nalorakk <Bear Avatar>

bool NalorakkPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* nalorakk = AI_VALUE2(Unit*, "find target", "23576");
    return nalorakk && nalorakk->GetHealthPct() > 95.0f;
}

bool NalorakkBossSwitchesFormsTrigger::IsActive()
{
    return (botAI->IsMainTank(bot) || botAI->IsAssistTankOfIndex(bot, 0, true)) &&
           AI_VALUE2(Unit*, "find target", "23576");
}

bool NalorakkBossCastsSurgeTrigger::IsActive()
{
    return botAI->IsRanged(bot) &&
           AI_VALUE2(Unit*, "find target", "23576");
}

// Jan'alai <Dragonhawk Avatar>

bool JanalaiPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* janalai = AI_VALUE2(Unit*, "find target", "23578");
    return janalai && janalai->GetHealthPct() > 95.0f;
}

bool JanalaiBossEngagedByTanksTrigger::IsActive()
{
    if (!botAI->IsTank(bot) ||
        !AI_VALUE2(Unit*, "find target", "23578"))
        return false;

    return !HasFireBombNearby(bot);
}

bool JanalaiBossCastsFlameBreathTrigger::IsActive()
{
    if (!botAI->IsRanged(bot) ||
        !AI_VALUE2(Unit*, "find target", "23578") ||
        AI_VALUE2(Unit*, "find target", "15649"))
        return false;

    return !HasFireBombNearby(bot);
}

bool JanalaiBossSummoningFireBombsTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "23578") &&
           HasFireBombNearby(bot);
}

bool JanalaiAmanishiHatchersSpawnedTrigger::IsActive()
{
    if (!botAI->IsRangedDps(bot) ||
        !AI_VALUE2(Unit*, "find target", "23578"))
        return false;

    return bot->FindNearestCreature(
               static_cast<uint32>(ZulAmanNPCs::NPC_AMANISHI_HATCHER), 40.0f);
}

// Halazzi <Lynx Avatar>

bool HalazziPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* halazzi = AI_VALUE2(Unit*, "find target", "23577");
    return halazzi && halazzi->GetHealthPct() > 95.0f;
}

bool HalazziBossEngagedByMainTankTrigger::IsActive()
{
    return botAI->IsMainTank(bot) &&
           AI_VALUE2(Unit*, "find target", "23577");
}

bool HalazziBossSummonsSpiritLynxTrigger::IsActive()
{
    return botAI->IsAssistTankOfIndex(bot, 0, true) &&
           AI_VALUE2(Unit*, "find target", "23577");
}

bool HalazziDeterminingDpsTargetTrigger::IsActive()
{
    return botAI->IsDps(bot) &&
           AI_VALUE2(Unit*, "find target", "23577");
}

// Hex Lord Malacrass

bool HexLordMalacrassPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* malacrass = AI_VALUE2(Unit*, "find target", "24239");
    return malacrass && malacrass->GetHealthPct() > 95.0f;
}

bool HexLordMalacrassDeterminingKillOrderTrigger::IsActive()
{
    return botAI->IsDps(bot) &&
           AI_VALUE2(Unit*, "find target", "24239");
}

bool HexLordMalacrassBossIsChannelingWhirlwindTrigger::IsActive()
{
    Unit* malacrass = AI_VALUE2(Unit*, "find target", "24239");
    if (!malacrass ||
        !malacrass->HasAura(static_cast<uint32>(ZulAmanSpells::SPELL_HEX_LORD_WHIRLWIND)))
        return false;

    return !(botAI->IsTank(bot) && malacrass->GetVictim() == bot);
}

bool HexLordMalacrassBossHasSpellReflectionTrigger::IsActive()
{
    if (!botAI->IsCaster(bot))
        return false;

    Unit* malacrass = AI_VALUE2(Unit*, "find target", "24239");
    return malacrass &&
           malacrass->HasAura(static_cast<uint32>(ZulAmanSpells::SPELL_HEX_LORD_SPELL_REFLECTION));
}

bool HexLordMalacrassBossPlacedFreezingTrapTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "24239") &&
           bot->FindNearestGameObject(
               static_cast<uint32>(ZulAmanObjects::GO_FREEZING_TRAP), 20.0f, true);
}

// Zul'jin

bool ZuljinMainTankNeedsAggroUponPullOrPhaseChangeTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* zuljin = AI_VALUE2(Unit*, "find target", "23863");
    if (!zuljin)
        return false;

    float hp = zuljin->GetHealthPct();

    return (hp <= 100.0f && hp > 95.0f) ||
           (hp <= 80.0f && hp > 75.0f &&
            zuljin->HasAura(static_cast<uint32>(ZulAmanSpells::SPELL_SHAPE_OF_THE_BEAR))) ||
           (hp <= 40.0f && hp > 35.0f &&
            zuljin->HasAura(static_cast<uint32>(ZulAmanSpells::SPELL_SHAPE_OF_THE_LYNX))) ||
           (hp <= 20.0f && hp > 15.0f &&
            zuljin->HasAura(static_cast<uint32>(ZulAmanSpells::SPELL_SHAPE_OF_THE_DRAGONHAWK)));
}

bool ZuljinBossEngagedByTanksTrigger::IsActive()
{
    if (!botAI->IsTank(bot))
        return false;

    Unit* zuljin = AI_VALUE2(Unit*, "find target", "23863");
    return zuljin &&
           !zuljin->HasAura(static_cast<uint32>(ZulAmanSpells::SPELL_SHAPE_OF_THE_EAGLE)) &&
           !zuljin->HasAura(static_cast<uint32>(ZulAmanSpells::SPELL_SHAPE_OF_THE_DRAGONHAWK));
}

bool ZuljinBossIsChannelingWhirlwindInTrollFormTrigger::IsActive()
{
    Unit* zuljin = AI_VALUE2(Unit*, "find target", "23863");
    if (!zuljin ||
        !zuljin->HasAura(static_cast<uint32>(ZulAmanSpells::SPELL_ZULJIN_WHIRLWIND)))
        return false;

    return !(botAI->IsTank(bot) && zuljin->GetVictim() == bot);
}

bool ZuljinBossIsSummoningCyclonesInEagleFormTrigger::IsActive()
{
    Unit* zuljin = AI_VALUE2(Unit*, "find target", "23863");
    return zuljin &&
           zuljin->HasAura(static_cast<uint32>(ZulAmanSpells::SPELL_SHAPE_OF_THE_EAGLE));
}

bool ZuljinBossCastsAoeAbilitiesInDragonhawkFormTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* zuljin = AI_VALUE2(Unit*, "find target", "23863");
    return zuljin &&
           zuljin->HasAura(static_cast<uint32>(ZulAmanSpells::SPELL_SHAPE_OF_THE_DRAGONHAWK));
}
