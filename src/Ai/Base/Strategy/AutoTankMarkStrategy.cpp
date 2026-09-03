/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AutoTankMarkStrategy.h"

#include "Playerbots.h"

void AutoTankMarkStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // 逃跑怪优先集火（最高优先级！）
    // 检测到正在逃跑或即将逃跑的怪物时，立即标记为骷髅并集火
    triggers.push_back(new TriggerNode(
        "fleeing target",
        { NextAction("prioritize fleeing target", ACTION_RAID) }
    ));

    // 主坦克自动标记骷髅（第一攻击目标）
    // By leewheel 2026-08-31: 硬性规则要求开怪立即标记, 优先级从 NORMAL+5 提升到 HIGH,
    // 解决玩家反馈的"标记延迟, 快打死了才标上"
    triggers.push_back(new TriggerNode(
        "main tank can mark skull",
        { NextAction("mark skull target", ACTION_HIGH) }
    ));

    // By leewheel 2026-08-31: 主坦克是真实玩家时, 由任意队伍 Bot 兜底标骷髅
    // （否则没有任何 Bot 满足 IsMainTank, 骷髅永远不会被标记）
    triggers.push_back(new TriggerNode(
        "fallback mark skull",
        { NextAction("fallback mark skull", ACTION_HIGH) }
    ));

    // 副坦克自动标记叉叉（第二攻击目标）—— 团本场景
    triggers.push_back(new TriggerNode(
        "off tank can mark cross",
        { NextAction("mark cross target", ACTION_NORMAL + 4.0f) }
    ));

    // 5人副本场景：队伍中只有一个坦克时，主坦克兼任标记叉叉
    triggers.push_back(new TriggerNode(
        "main tank can mark cross",
        { NextAction("mark cross target", ACTION_NORMAL + 3.0f) }
    ));
}
