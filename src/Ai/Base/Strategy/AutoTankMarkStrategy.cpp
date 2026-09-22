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

    // By leewheel 2026-09-22: 主坦克自动标记月亮（CC 目标）
    //   参考 mod-playerbots-dungeon-lead 的月亮机制：给首领身边那只要被控制的精英打月亮。
    //   德鲁伊（缠绕/休眠/旋风）与术士（放逐/恐惧）的 RtiCcTrigger 子类直接以
    //   "rti cc target"（默认 moon）为目标，所以月亮一落地，控制就会自动接上；
    //   同时上游 FindNonCcTargetStrategy 会让普通 DPS 避开月亮目标，等于替 CC 保住了不被打断。
    //   优先级略高于叉叉（+5.0 对 +4.0/+3.0）：月亮标上后叉叉会自动避让同一只怪。
    triggers.push_back(new TriggerNode(
        "main tank can mark moon",
        { NextAction("mark moon target", ACTION_NORMAL + 5.0f) }
    ));

    // By leewheel 2026-09-22: 月亮标记的生命周期维护（清死亡目标 / 释放超时标记）
    //   与参考实现 DungeonLeadStrategy 完全一致地挂在通用 "often"（RandomTrigger 5%）上：
    //   既保证"被标怪还没进战斗"期间也在检查，又绝不在每 tick 抢占真正的战斗动作。
    triggers.push_back(new TriggerNode(
        "often",
        { NextAction("check cc mark", ACTION_NORMAL) }
    ));
}
