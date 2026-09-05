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
    if (IsEncounterInProgress(bot, ZA_MAP_ID))
        return false;

    return IsMechanicTrackerBot(bot, ZA_MAP_ID);
}

// Same Misdirect on pull for all bosses
bool ZulAmanPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    if (bot->GetMapId() != ZA_MAP_ID)
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

bool AkilzonBossEngagedByTanksTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "23574"))
        return false;

    return !GetElectricalStormTarget(bot);
}

bool AkilzonSpreadForStaticDisruptionTrigger::IsActiveInEncounter()
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

bool AkilzonElectricalStormIncomingTrigger::IsActiveInEncounter()
{
    if (!AI_VALUE2(Unit*, "find target", "23574"))
        return false;

    auto it = akilzonStormTimer.find(bot->GetInstanceId());
    if (it == akilzonStormTimer.end())
        return false;

    return IsInStormWindow(it->second);
}

bool AkilzonBotsNeedToPrepareForElectricalStormTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, ZA_MAP_ID))
        return false;

    // 合并brighton 2026-08-26: akil'zon按entry规则转23574 --By leewheel 2026年8月26日
    return AI_VALUE2(Unit*, "find target", "23574");
}

// Nalorakk <Bear Avatar>

bool NalorakkBossSwitchesFormsTrigger::IsActiveInEncounter()
{
    if (!AI_VALUE2(Unit*, "find target", "23576"))
        return false;

    return PlayerbotAI::IsMainTank(bot) || PlayerbotAI::IsAssistTankOfIndex(bot, 0, true);
}

bool NalorakkSpreadForSurgeTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsRanged(bot) && AI_VALUE2(Unit*, "find target", "23576");
}

// Jan'alai <Dragonhawk Avatar>

bool JanalaiBossEngagedByTanksTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

// By leewheel 2026-08-30 合并上游：改用IsJanalaiBombing判定；entry规则查怪(23578=jan'alai)
    Unit* janalai = AI_VALUE2(Unit*, "find target", "23578");

    return janalai && !IsJanalaiBombing(janalai);
}

bool JanalaiSpreadForFlameBreathTrigger::IsActiveInEncounter()
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

bool JanalaiBossSummoningFireBombsTrigger::IsActiveInEncounter()
{
    // By leewheel 2026-08-30 合并上游：改用IsJanalaiBombing helper；entry规则查怪(23578=jan'alai)
    return IsJanalaiBombing(AI_VALUE2(Unit*, "find target", "23578"));
    // End By leewheel
}

bool JanalaiAmanishiHatchersSpawnedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRangedDps(bot))
        return false;

    // By leewheel 2026-09-04 合并冲突解决: 采纳brighton新增的孵化血量判断(JANALAI_HATCH_ALL_HEALTH_PCT),
    //   boss查找保留entry"23578"遵循项目规则
    // End By leewheel
    Unit* janalai = AI_VALUE2(Unit*, "find target", "23578");
    if (!janalai || janalai->GetHealthPct() <= JANALAI_HATCH_ALL_HEALTH_PCT)
        return false;

    constexpr float searchRadius = 40.0f;
    return bot->FindNearestCreature(Id(ZaNpcs::NPC_AMANISHI_HATCHER), searchRadius);
}

// Halazzi <Lynx Avatar>

bool HalazziShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsMainTank(bot) && AI_VALUE2(Unit*, "find target", "23577");
}

bool HalazziSpiritLynxHasAppearedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) &&
        AI_VALUE2(Unit*, "find target", "23577");
}

bool HalazziShouldFocusDpsTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsDps(bot) && AI_VALUE2(Unit*, "find target", "23577");
}

// Hex Lord Malacrass

bool HexLordMalacrassShouldPrioritizeAddsTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsDps(bot) && AI_VALUE2(Unit*, "find target", "24239");
}

bool HexLordMalacrassBossIsChannelingWhirlwindTrigger::IsActiveInEncounter()
{
    // 合并brighton 2026-08-26: hex lord malacrass按entry规则转24239; 攻击者为bot时视为安全(非通道旋风目标); 移除孤立的BossHasSpellReflectionTrigger(无声明) --By leewheel 2026年8月26日
    Unit* malacrass = AI_VALUE2(Unit*, "find target", "24239");
    if (!malacrass || malacrass->GetVictim() == bot)
        return false;

    return malacrass->HasAura(Id(ZaSpells::SPELL_HEX_LORD_WHIRLWIND));
}

bool HexLordMalacrassBossPlacedFreezingTrapTrigger::IsActiveInEncounter()
{
    if (!AI_VALUE2(Unit*, "find target", "24239"))
        return false;

    return GetNearbyFreezingTrap(botAI) != nullptr;
}

// Zul'jin

bool ZuljinBossEngagedByTanksTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

// By leewheel 2026-09-05 合并：Zul'jin按entry规则查找(23863)，替代上游名字查找
    Unit* zuljin = AI_VALUE2(Unit*, "find target", "23863");
    return zuljin &&
           !zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_EAGLE)) &&
           !zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_DRAGONHAWK));
    // End By leewheel
}

bool ZuljinBossIsChannelingWhirlwindInTrollFormTrigger::IsActiveInEncounter()
{
    Unit* zuljin = AI_VALUE2(Unit*, "find target", "23863");
    if (!zuljin || !zuljin->HasAura(Id(ZaSpells::SPELL_ZULJIN_WHIRLWIND)))
        return false;

    return !PlayerbotAI::IsTank(bot) || zuljin->GetVictim() != bot;
}

bool ZuljinBossIsSummoningCyclonesInEagleFormTrigger::IsActiveInEncounter()
{
    Unit* zuljin = AI_VALUE2(Unit*, "find target", "23863");
    return zuljin && zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_EAGLE));
}

bool ZuljinSpreadForDragonhawkAoeTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* zuljin = AI_VALUE2(Unit*, "find target", "23863");
    return zuljin && zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_DRAGONHAWK));
}
