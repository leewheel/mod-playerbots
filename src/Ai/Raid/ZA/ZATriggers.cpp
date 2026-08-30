/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ZATriggers.h"
#include "EncounterHelpers.h"
#include "InstanceScript.h"
#include "Playerbots.h"
#include "ZAHelpers.h"

using namespace ZaHelpers;
using namespace EncounterHelpers;

// General

bool ZulAmanNoEncounterInProgressTrigger::IsActive()
{
    if (bot->GetMapId() != ZA_MAP_ID)
        return false;

    InstanceScript* instance = bot->GetInstanceScript();
    if (!instance || instance->IsEncounterInProgress())
        return false;

    return IsMechanicTrackerBot(bot, ZA_MAP_ID);
}

// The misdirect on the pull is the same job on every boss, and every Zul'Aman boss - and nothing
// else in the instance - runs a BossAI, so "boss target" resolves whichever one the raid is on.
bool ZulAmanPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* boss = AI_VALUE(Unit*, "boss target");
    return boss && boss->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT;
}

// Trash

bool AmanishiMedicineManSummonedWardTrigger::IsActive()
{
    return IsMechanicTrackerBot(bot, ZA_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "23581");
}

// Akil'zon <Eagle Avatar>

bool AkilzonBossEngagedByTanksTrigger::IsActive()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "23574"))
        return false;

    return !GetElectricalStormTarget(bot);
}

bool AkilzonBossCastsStaticDisruptionTrigger::IsActive()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "23574"))
        return false;

    auto it = akilzonStormTimer.find(bot->GetInstanceId());
    if (it == akilzonStormTimer.end())
        return true;

    return !IsInStormWindow(it->second);
}

bool AkilzonElectricalStormIncomingTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "23574"))
        return false;

    auto it = akilzonStormTimer.find(bot->GetInstanceId());
    if (it == akilzonStormTimer.end())
        return false;

    return IsInStormWindow(it->second);
}

bool AkilzonBotsNeedToPrepareForElectricalStormTrigger::IsActive()
{
    if (!IsMechanicTrackerBot(bot, ZA_MAP_ID))
        return false;

    // 合并brighton 2026-08-26: akil'zon按entry规则转23574 --By leewheel 2026年8月26日
    return AI_VALUE2(Unit*, "find target", "23574");
}

// Nalorakk <Bear Avatar>

bool NalorakkBossSwitchesFormsTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "23576"))
        return false;

    return PlayerbotAI::IsMainTank(bot) || PlayerbotAI::IsAssistTankOfIndex(bot, 0, true);
}

bool NalorakkBossCastsSurgeTrigger::IsActive()
{
    return PlayerbotAI::IsRanged(bot) && AI_VALUE2(Unit*, "find target", "23576");
}

// Jan'alai <Dragonhawk Avatar>

bool JanalaiBossEngagedByTanksTrigger::IsActive()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    // By leewheel 2026-08-30 合并上游：改用IsJanalaiBombing判定；entry规则查怪(23578=jan'alai)
    Unit* janalai = AI_VALUE2(Unit*, "find target", "23578");

    return janalai && !IsJanalaiBombing(janalai);
}

bool JanalaiBossCastsFlameBreathTrigger::IsActive()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    // By leewheel 2026-08-30 合并上游：改用IsJanalaiBombing判定；entry规则查怪(23578=jan'alai)
    Unit* janalai = AI_VALUE2(Unit*, "find target", "23578");
    if (!janalai)
        return false;

    if (AI_VALUE2(Unit*, "find target", "23598"))
        return false;

    return !IsJanalaiBombing(janalai);
}

bool JanalaiBossSummoningFireBombsTrigger::IsActive()
{
    // By leewheel 2026-08-30 合并上游：改用IsJanalaiBombing helper；entry规则查怪(23578=jan'alai)
    return IsJanalaiBombing(AI_VALUE2(Unit*, "find target", "23578"));
    // End By leewheel
}

bool JanalaiAmanishiHatchersSpawnedTrigger::IsActive()
{
    if (!PlayerbotAI::IsRangedDps(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "23578"))
        return false;

    // Just need to find one Hatcher to fire the trigger
    constexpr float searchRadius = 40.0f;
    return bot->FindNearestCreature(Id(ZaNpcs::NPC_AMANISHI_HATCHER), searchRadius);
}

// Halazzi <Lynx Avatar>

bool HalazziShouldBeTankedTrigger::IsActive()
{
    return PlayerbotAI::IsMainTank(bot) && AI_VALUE2(Unit*, "find target", "23577");
}

bool HalazziSpiritLynxHasAppearedTrigger::IsActive()
{
    return PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) &&
        AI_VALUE2(Unit*, "find target", "23577");
}

bool HalazziShouldFocusDpsTrigger::IsActive()
{
    return PlayerbotAI::IsDps(bot) && AI_VALUE2(Unit*, "find target", "23577");
}

// Hex Lord Malacrass

bool HexLordMalacrassShouldPrioritizeAddsTrigger::IsActive()
{
    return PlayerbotAI::IsDps(bot) && AI_VALUE2(Unit*, "find target", "24239");
}

bool HexLordMalacrassBossIsChannelingWhirlwindTrigger::IsActive()
{
    // 合并brighton 2026-08-26: hex lord malacrass按entry规则转24239; 攻击者为bot时视为安全(非通道旋风目标); 移除孤立的BossHasSpellReflectionTrigger(无声明) --By leewheel 2026年8月26日
    Unit* malacrass = AI_VALUE2(Unit*, "find target", "24239");
    if (!malacrass || malacrass->GetVictim() == bot)
        return false;

    return malacrass->HasAura(Id(ZaSpells::SPELL_HEX_LORD_WHIRLWIND));
}

bool HexLordMalacrassBossPlacedFreezingTrapTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "24239"))
        return false;

    return bot->FindNearestGameObject(
        Id(ZaObjects::GO_FREEZING_TRAP), ZA_FREEZING_TRAP_SEARCH_RADIUS, true);
}

// Zul'jin

bool ZuljinBossEngagedByTanksTrigger::IsActive()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* zuljin = AI_VALUE2(Unit*, "find target", "23863");
    return zuljin &&
           !zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_EAGLE)) &&
           !zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_DRAGONHAWK));
}

bool ZuljinBossIsChannelingWhirlwindInTrollFormTrigger::IsActive()
{
    Unit* zuljin = AI_VALUE2(Unit*, "find target", "23863");
    if (!zuljin || !zuljin->HasAura(Id(ZaSpells::SPELL_ZULJIN_WHIRLWIND)))
        return false;

    return !(PlayerbotAI::IsTank(bot) && zuljin->GetVictim() == bot);
}

bool ZuljinBossIsSummoningCyclonesInEagleFormTrigger::IsActive()
{
    Unit* zuljin = AI_VALUE2(Unit*, "find target", "23863");
    return zuljin && zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_EAGLE));
}

bool ZuljinBossCastsAoeAbilitiesInDragonhawkFormTrigger::IsActive()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* zuljin = AI_VALUE2(Unit*, "find target", "23863");
    return zuljin && zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_DRAGONHAWK));
}
