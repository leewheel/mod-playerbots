/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_AUTOTANKMARKTRIGGERS_H
#define PLAYERBOTS_AUTOTANKMARKTRIGGERS_H

#include "Trigger.h"

class PlayerbotAI;

// By leewheel 2026-07-15: 主坦克自动标记骷髅触发器
// By leewheel 2026-08-08: 扩展到副本外（野外）也标记
// 条件：配置开启 + 非战场/竞技场 + 是主坦克 + 有未标记骷髅的敌人
class MainTankMarkSkullTrigger : public Trigger
{
public:
    MainTankMarkSkullTrigger(PlayerbotAI* botAI) : Trigger(botAI, "main tank can mark skull") {}

    bool IsActive() override;
};

// By leewheel 2026-07-15: 副坦克自动标记叉叉触发器
// By leewheel 2026-08-08: 扩展到副本外（野外）也标记
// 条件：配置开启 + 非战场/竞技场 + 是副坦克 + 有未标记叉叉的敌人
class OffTankMarkCrossTrigger : public Trigger
{
public:
    OffTankMarkCrossTrigger(PlayerbotAI* botAI) : Trigger(botAI, "off tank can mark cross") {}

    bool IsActive() override;
};

// By leewheel 2026-07-15: 5人副本主坦克兼任标记叉叉触发器
// By leewheel 2026-08-08: 扩展到副本外（野外）也标记
// 条件：配置开启 + 非战场/竞技场 + 是主坦克 + 队伍中没有其他坦克（5人场景）
// + 叉叉未被占用 + 有至少2个未标记敌人
class MainTankMarkCrossTrigger : public Trigger
{
public:
    MainTankMarkCrossTrigger(PlayerbotAI* botAI) : Trigger(botAI, "main tank can mark cross") {}

    bool IsActive() override;
};

// By leewheel 2026-08-31: 兜底标骷髅触发器 —— 主坦克是真实玩家时
// 主坦克不是机器人则没有任何 Bot 满足 IsMainTank(bot)，骷髅永远不会被标记
// （玩家反馈"自动标记时有时无"的根因之一）。
// 条件：配置开启 + 非战场/竞技场 + 主坦克为真实玩家且已进战斗 + 骷髅槽位可用
class FallbackMarkSkullTrigger : public Trigger
{
public:
    FallbackMarkSkullTrigger(PlayerbotAI* botAI) : Trigger(botAI, "fallback mark skull") {}

    bool IsActive() override;
};

// By leewheel 2026-09-22: 主坦克自动标记月亮（CC 目标）触发器
// 参考 mod-playerbots-dungeon-lead 的月亮标记机制：给首领身边那只"需要被控制"的精英
// 打月亮，德鲁伊/术士等 CC 职业会以它为目标（rti cc 默认值就是 moon）。
// 条件：配置开启 + 非战场/竞技场 + 是主坦克 + 队伍里有能控的职业 + 战斗中 + 月亮槽位可用
class MainTankMarkMoonTrigger : public Trigger
{
public:
    MainTankMarkMoonTrigger(PlayerbotAI* botAI) : Trigger(botAI, "main tank can mark moon") {}

    bool IsActive() override;
};

// By leewheel 2026-09-22: 月亮标记的生命周期维护不单独设触发器 ——
//   参考 mod-playerbots-dungeon-lead 的做法，用上游通用的 "often"（RandomTrigger 5%）
//   限流，把动作直接挂在 "often" 上（见 AutoTankMarkStrategy.cpp）。
//   理由：维护动作需要"被标怪未进战斗"期间也能跑，且绝不能在每 tick 抢占战斗动作；
//   "often" 天然满足这两点，而一个"只要月亮被占用就持续激活"的专用触发器会一直
//   参与优先级竞争，反而可能饿死真正的战斗动作。

#endif
