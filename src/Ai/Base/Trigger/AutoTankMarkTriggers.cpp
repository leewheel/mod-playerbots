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

// By leewheel 2026-08-08: 检查是否允许自动标记
// 玩家需求：坦克标记功能扩展到副本外（野外）也生效。
// 原实现只允许副本/团本（IsInInstanceOrRaid），现放宽为排除战场/竞技场即可，
// 与 FleeingTargetTrigger（逃跑怪标记）的野外行为保持一致。
static bool IsMarkAllowed(Player* bot)
{
    if (!bot || !bot->GetMap())
        return false;

    if (bot->InBattleground() || bot->InArena())
        return false;

    return true;
}

// By leewheel 2026-08-31: 判断标记槽位是否可用(空 或 指向已死亡/消失的目标)
// 核心不会在怪物死亡时清除队伍标记图标, 原"仅图标为空才触发"的逻辑
// 会导致骷髅标记的怪死后图标一直指向尸体, 触发器永远不再激活,
// 下一次开怪无法再标记(硬性规则失效)。
// 图标指向存活怪物时返回 false —— 骷髅/叉叉锁定不变, 直到目标死亡。
static bool IsMarkSlotAvailable(PlayerbotAI* botAI, Group* group, uint8 iconIndex)
{
    ObjectGuid const guid = group->GetTargetIcon(iconIndex);
    if (guid.IsEmpty())
        return true;

    Unit* unit = botAI->GetUnit(guid);
    return !unit || !unit->IsAlive() || !unit->IsInWorld() || unit->IsPlayer();
}

bool MainTankMarkSkullTrigger::IsActive()
{
    if (!sPlayerbotAIConfig.autoTankMarkEnabled)
        return false;

    if (!IsMarkAllowed(bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    if (!botAI->IsMainTank(bot))
        return false;

    // By leewheel 2026-08-31: 硬性规则 —— 骷髅指向存活怪时锁定不变;
    // 图标为空或指向已死亡/消失目标时激活, 由动作清除陈旧标记并重新标记
    if (!IsMarkSlotAvailable(botAI, group, RtiTargetValue::skullIndex))
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

    if (!IsMarkAllowed(bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (botAI->IsMainTank(bot))
        return false;

    // By leewheel 2026-08-31: 同骷髅逻辑 —— 叉叉指向存活怪时锁定, 指向死亡/消失目标时允许重新标记
    if (!IsMarkSlotAvailable(botAI, group, RtiTargetValue::crossIndex))
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

    if (!IsMarkAllowed(bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    if (!botAI->IsMainTank(bot))
        return false;

    // 5人副本场景：队伍中只有一个坦克时，主坦克兼任标记叉叉
    if (CountOtherTanks(bot, group) > 0)
        return false;

    // By leewheel 2026-08-31: 同骷髅逻辑 —— 叉叉指向存活怪时锁定, 指向死亡/消失目标时允许重新标记
    if (!IsMarkSlotAvailable(botAI, group, RtiTargetValue::crossIndex))
        return false;

    // 战斗中才触发（与NoRtiTrigger一致，不依赖attackers列表）
    if (!bot->IsInCombat())
        return false;

    return true;
}
