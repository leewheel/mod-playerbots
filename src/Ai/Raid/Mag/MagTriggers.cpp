/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "MagTriggers.h"
#include "EncounterHelpers.h"
#include "MagHelpers.h"
#include "Playerbots.h"

using namespace MagHelpers;
using namespace EncounterHelpers;

bool MagtheridonNoEncounterInProgressTrigger::IsActive()
{
    if (IsEncounterInProgress(bot, MAG_MAP_ID))
        return false;

    return IsMechanicTrackerBot(bot, MAG_MAP_ID);
}

bool MagtheridonMainTankShouldTankChannelersTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMainTank(bot))
        return false;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "17257");
    return magtheridon && !IsMagtheridonActive(magtheridon);
}

bool MagtheridonAssistTanksShouldTankChannelersTrigger::IsActiveInEncounter()
{
    if (!AI_VALUE2(Unit*, "find target", "17257"))
        return false;

    if (GetChanneler(bot, NORTHWEST_CHANNELER_DB_GUID) &&
        PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
    {
        return true;
    }

    return GetChanneler(bot, NORTHEAST_CHANNELER_DB_GUID) &&
        PlayerbotAI::IsAssistTankOfIndex(bot, 1, true);
}

bool MagtheridonPullingWestAndEastChannelersTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "17257"))
        return false;

    return GetChanneler(bot, WEST_CHANNELER_DB_GUID) || GetChanneler(bot, EAST_CHANNELER_DB_GUID);
}

bool MagtheridonDeterminingKillOrderTrigger::IsActiveInEncounter()
{
    // By leewheel 2026-09-09 合并brighton: 采纳brighton简化版(非坦克+在场即拉击杀顺序), boss按entry规则用17257
    return !PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "17257");
}

bool MagtheridonBurningAbyssalSpawnedTrigger::IsActiveInEncounter()
{
    // By leewheel 2026-09-13 合并brighton 8c96a663: 采纳上游新 helper GetBurningAbyssals(botAI)，
    // 不再需要按英文名"burning abyssal"/entry 17454 查找，故 rule81 entry 化在此处退役
    return bot->getClass() == CLASS_WARLOCK && !GetBurningAbyssals(botAI).empty();
}

bool MagtheridonShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "17257");
    if (!magtheridon || !IsMagtheridonActive(magtheridon))
        return false;

    // Include an assist tank that pulls aggro
    return magtheridon->GetVictim() == bot || PlayerbotAI::IsMainTank(bot);
}

bool MagtheridonShouldSpreadRangedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    // By leewheel 2026-09-09 合并brighton: 采纳brighton现代版(注释掉dpsWait计时，增加被boss紧盯判定), boss按entry规则用17257，保留GetMap()取实例id
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "17257");
    if (!magtheridon || !IsMagtheridonActive(magtheridon) || magtheridon->GetVictim() == bot)
        return false;

    // By leewheel 2026-09-13 合并brighton 8c96a663: 上游已彻底移除 dpsWait 计时分支
    // （原 2026-08-26 的注释停用写法随之退役）
    // End By leewheel
    if (!IsCubeClicker(bot))
        return true;

    auto timerIt = blastNovaTimer.find(magtheridon->GetMap()->GetInstanceId());
    if (timerIt == blastNovaTimer.end())
        return true;

    return getMSTimeDiff(timerIt->second, getMSTime()) < BLAST_NOVA_INTERIM_MS;
}

bool MagtheridonStandingInDebrisTrigger::IsActiveInEncounter()
{
    // By leewheel 2026-09-13 合并brighton 8c96a663: 采纳上游新增的 IsCeilingCollapsed 判定, 保留 entry 化
    if (!IsCeilingCollapsed(bot) || !AI_VALUE2(Unit*, "find target", "17257"))
        return false;

    return IsPositionInActiveDebris(botAI, bot->GetPositionX(), bot->GetPositionY());
}

bool MagtheridonIncomingBlastNovaTrigger::IsActiveInEncounter()
{
    // By leewheel 2026-09-13 合并brighton 8c96a663: 采纳上游把 IsCubeClicker 提前的写法, 保留 entry 化
    if (!IsCubeClicker(bot))
        return false;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "17257");
    return magtheridon && IsMagtheridonActive(magtheridon);
}

bool MagtheridonShouldManageTimersAndAssignmentsTrigger::IsActiveInEncounter()
{
    // By leewheel 2026-09-09 合并brighton: 采纳brighton版(加IsMagtheridonActive判定), boss按entry规则用17257
    if (!IsMechanicTrackerBot(bot, MAG_MAP_ID))
        return false;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "17257");
    return magtheridon && IsMagtheridonActive(magtheridon);
    // End By leewheel
}
// 合并brighton 2026-08-26: 移除无声明无引用的孤立MagtheridonBotIsNotInCombatTrigger --By leewheel 2026年8月26日
