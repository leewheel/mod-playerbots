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

    // 排除战场和竞技场
    if (bot->InBattleground() || bot->InArena())
        return false;

    // 检查地图是否是副本类型
    InstanceMap const* instanceMap = bot->GetMap()->ToInstanceMap();
    if (instanceMap)
        return true;

    // 也检查是否通过GetInstanceId判断
    return bot->GetInstanceId() != 0;
}

// By leewheel 2026-07-15: 检查目标是否已被标记
static bool IsTargetMarked(Group* group, ObjectGuid targetGuid)
{
    if (!group || targetGuid.IsEmpty())
        return false;

    for (uint8 i = 0; i < 8; i++)
    {
        if (group->GetTargetIcon(i) == targetGuid)
            return true;
    }
    return false;
}

// By leewheel 2026-07-15: 检查指定索引的标记是否已被占用
static bool IsIconOccupied(Group* group, uint8 iconIndex)
{
    if (!group)
        return false;

    return !group->GetTargetIcon(iconIndex).IsEmpty();
}

bool MainTankMarkSkullTrigger::IsActive()
{
    // 检查配置开关
    if (!sPlayerbotAIConfig.autoTankMarkEnabled)
        return false;

    // 必须在副本/团本中
    if (!IsInInstanceOrRaid(bot))
        return false;

    // 必须有队伍
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // 必须是主坦克
    if (!botAI->IsMainTank(bot))
        return false;

    // 骷髅标记(index=7)已被占用就不重复标记
    if (IsIconOccupied(group, RtiTargetValue::skullIndex))
        return false;

    // 战斗中或有攻击者时才触发
    if (!bot->IsInCombat())
    {
        // 非战斗状态也允许预先标记（如果有攻击者在范围内）
        GuidVector attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();
        if (attackers.empty())
            return false;
    }

    // 确认至少有一个未标记的敌人
    GuidVector attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();
    for (ObjectGuid const guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || unit->IsPlayer())
            continue;

        if (!IsTargetMarked(group, unit->GetGUID()))
            return true;  // 有未标记的敌人，可以标记骷髅
    }

    return false;
}

bool OffTankMarkCrossTrigger::IsActive()
{
    // 检查配置开关
    if (!sPlayerbotAIConfig.autoTankMarkEnabled)
        return false;

    // 必须在副本/团本中
    if (!IsInInstanceOrRaid(bot))
        return false;

    // 必须有队伍
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // 必须是坦克但不是主坦克（即为副坦克）
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (botAI->IsMainTank(bot))
        return false;

    // 叉叉标记(index=6)已被占用就不重复标记
    if (IsIconOccupied(group, RtiTargetValue::crossIndex))
        return false;

    // 战斗中或有攻击者时才触发
    if (!bot->IsInCombat())
    {
        GuidVector attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();
        if (attackers.empty())
            return false;
    }

    // 确认至少有一个未标记的敌人（排除骷髅标记的目标）
    GuidVector attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();
    ObjectGuid skullGuid = group->GetTargetIcon(RtiTargetValue::skullIndex);

    for (ObjectGuid const guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || unit->IsPlayer())
            continue;

        // 跳过已被骷髅标记的目标
        if (unit->GetGUID() == skullGuid)
            continue;

        if (!IsTargetMarked(group, unit->GetGUID()))
            return true;  // 有未标记的敌人，可以标记叉叉
    }

    return false;
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
    // 检查配置开关
    if (!sPlayerbotAIConfig.autoTankMarkEnabled)
        return false;

    // 必须在副本/团本中
    if (!IsInInstanceOrRaid(bot))
        return false;

    // 必须有队伍
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // 必须是主坦克
    if (!botAI->IsMainTank(bot))
        return false;

    // 队伍中没有其他坦克（5人副本场景：只有一个坦克）
    if (CountOtherTanks(bot, group) > 0)
        return false;

    // 叉叉标记(index=6)已被占用就不重复标记
    if (IsIconOccupied(group, RtiTargetValue::crossIndex))
        return false;

    // 战斗中或有攻击者时才触发
    if (!bot->IsInCombat())
    {
        GuidVector attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();
        if (attackers.empty())
            return false;
    }

    // 确认至少有一个未标记的敌人（排除骷髅标记的目标）
    GuidVector attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();
    ObjectGuid skullGuid = group->GetTargetIcon(RtiTargetValue::skullIndex);

    for (ObjectGuid const guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || unit->IsPlayer())
            continue;

        // 跳过已被骷髅标记的目标
        if (unit->GetGUID() == skullGuid)
            continue;

        if (!IsTargetMarked(group, unit->GetGUID()))
            return true;  // 有未标记的敌人，可以标记叉叉
    }

    return false;
}
