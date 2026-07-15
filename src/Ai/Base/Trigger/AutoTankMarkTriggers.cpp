/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AutoTankMarkTriggers.h"

#include "AttackersValue.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"

// By leewheel 2026-07-15: 检查是否在副本/团本中
static bool IsInInstanceOrRaid(Player* bot)
{
    if (!bot || !bot->GetMap())
        return false;

    if (bot->InBattleground() || bot->InArena())
        return false;

    InstanceMap const* instanceMap = bot->GetMap()->ToInstanceMap();
    if (instanceMap)
        return true;

    return bot->GetInstanceId() != 0;
}

bool MainTankMarkSkullTrigger::IsActive()
{
    if (!sPlayerbotAIConfig.autoTankMarkEnabled)
        return false;

    if (!IsInInstanceOrRaid(bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    if (!botAI->IsMainTank(bot))
        return false;

    if (!group->GetTargetIcon(RtiTargetValue::skullIndex).IsEmpty())
        return false;

    // 战斗中才触发（与NoRtiTrigger一致，不依赖attackers列表）
    if (!bot->IsInCombat())
        return false;

    return true;
}

bool OffTankMarkCrossTrigger::IsActive()
{
    if (!sPlayerbotAIConfig.autoTankMarkEnabled)
        return false;

    if (!IsInInstanceOrRaid(bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (botAI->IsMainTank(bot))
        return false;

    if (!group->GetTargetIcon(RtiTargetValue::crossIndex).IsEmpty())
        return false;

    // 战斗中才触发（与NoRtiTrigger一致，不依赖attackers列表）
    if (!bot->IsInCombat())
        return false;

    return true;
}

// By leewheel 2026-07-15: 统计队伍中其他坦克数量（排除自己）
static int CountOtherTanks(Player* bot, Group* group)
{
    if (!bot || !group)
        return 0;

    int count = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive())
            continue;

        if (PlayerbotAI::IsTank(member))
            count++;
    }
    return count;
}

bool MainTankMarkCrossTrigger::IsActive()
{
    if (!sPlayerbotAIConfig.autoTankMarkEnabled)
        return false;

    if (!IsInInstanceOrRaid(bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    if (!botAI->IsMainTank(bot))
        return false;

    // 5人副本场景：队伍中只有一个坦克时，主坦克兼任标记叉叉
    if (CountOtherTanks(bot, group) > 0)
        return false;

    if (!group->GetTargetIcon(RtiTargetValue::crossIndex).IsEmpty())
        return false;

    // 战斗中才触发（与NoRtiTrigger一致，不依赖attackers列表）
    if (!bot->IsInCombat())
        return false;

    return true;
}
