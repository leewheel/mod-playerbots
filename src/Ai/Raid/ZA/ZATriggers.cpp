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

bool ZulAmanPullingBossTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", _bossName);
    return boss && boss->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT;
}

// Trash

bool AmanishiMedicineManSummonedWardTrigger::IsActive()
{
    return IsMechanicTrackerBot(bot, ZA_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "23581");
}

// Akil'zon <Eagle Avatar>

bool AkilzonShouldBeTankedTrigger::IsActiveInEncounter()
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

bool AkilzonShouldTrackElectricalStormTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, ZA_MAP_ID))
        return false;

    // 合并brighton 2026-08-26: akil'zon按entry规则转23574 --By leewheel 2026年8月26日
    return AI_VALUE2(Unit*, "find target", "23574");
}

// Nalorakk <Bear Avatar>

bool NalorakkBothFormsShouldBeTankedTrigger::IsActiveInEncounter()
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

bool JanalaiShouldBeTankedTrigger::IsActiveInEncounter()
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

bool JanalaiIsFireBombingTrigger::IsActiveInEncounter()
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

    return bot->FindNearestCreature(Id(ZaNpcs::NPC_AMANISHI_HATCHER), ZA_CREATURE_SEARCH_RADIUS);
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

bool HexLordMalacrassChannelingWhirlwindTrigger::IsActiveInEncounter()
{
    // 合并brighton 2026-08-26: hex lord malacrass按entry规则转24239; 攻击者为bot时视为安全(非通道旋风目标); 移除孤立的BossHasSpellReflectionTrigger(无声明) --By leewheel 2026年8月26日
    Unit* malacrass = AI_VALUE2(Unit*, "find target", "24239");
    if (!malacrass || malacrass->GetVictim() == bot)
        return false;

    return malacrass->HasAura(Id(ZaSpells::SPELL_HEX_LORD_WHIRLWIND));
}

bool HexLordMalacrassFreezingTrapPlacedTrigger::IsActiveInEncounter()
{
    if (!AI_VALUE2(Unit*, "find target", "24239"))
        return false;

    return GetNearbyFreezingTrap(botAI);
}

// Zul'jin

bool ZuljinShouldBeTankedTrigger::IsActiveInEncounter()
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

bool ZuljinChannelingWhirlwindInTrollFormTrigger::IsActiveInEncounter()
{
    Unit* zuljin = AI_VALUE2(Unit*, "find target", "23863");
    if (!zuljin || !zuljin->HasAura(Id(ZaSpells::SPELL_ZULJIN_WHIRLWIND)))
        return false;

    return !PlayerbotAI::IsTank(bot) || zuljin->GetVictim() != bot;
}

bool ZuljinCreepingParalysisInBearFormTrigger::IsActiveInEncounter()
{
    // By leewheel 2026-09-09 合并brighton 2026-09-08: 采用brighton重塑逻辑(牧师+熊形态+可驱散目标),
    //   boss查找按entry规则用23863 -- End By leewheel
    if (bot->getClass() != CLASS_PRIEST)
        return false;

    Unit* zuljin = AI_VALUE2(Unit*, "find target", "23863");
    if (!zuljin || !zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_BEAR)))
        return false;

    return GetZuljinCreepingParalysisDispelTarget(bot);
}

bool ZuljinSummoningCyclonesInEagleFormTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    // By leewheel 2026-09-09 合并brighton: Zul'jin按entry规则查找(23863)
    Unit* zuljin = AI_VALUE2(Unit*, "find target", "23863");
    if (!zuljin)
        return false;

    if (zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_EAGLE)))
        return true;

    // The aura check is cleaner, but the health check here allows ranged to head to their
    // positions during the phase transition sequence.
    float const healthPct = zuljin->GetHealthPct();
    return healthPct <= 60.0f && healthPct > 40.0f;
}

bool ZuljinSpreadForDragonhawkAoeTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* zuljin = AI_VALUE2(Unit*, "find target", "23863");
    return zuljin && zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_DRAGONHAWK));
}
