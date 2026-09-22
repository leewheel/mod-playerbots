/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_AUTOTANKMARKACTIONS_H
#define PLAYERBOTS_AUTOTANKMARKACTIONS_H

#include "Action.h"

class PlayerbotAI;

// By leewheel 2026-07-15: 主坦克标记骷髅动作
// 选择最大生命值的敌人标记为骷髅（第一攻击目标）
class MarkSkullTargetAction : public Action
{
public:
    MarkSkullTargetAction(PlayerbotAI* botAI) : Action(botAI, "mark skull target") {}

    bool Execute(Event event) override;
};

// By leewheel 2026-07-15: 副坦克标记叉叉动作
// 选择第二大生命值的敌人标记为叉叉（第二攻击目标）
class MarkCrossTargetAction : public Action
{
public:
    MarkCrossTargetAction(PlayerbotAI* botAI) : Action(botAI, "mark cross target") {}

    bool Execute(Event event) override;
};

// By leewheel 2026-08-31: 兜底标骷髅动作 —— 主坦克是真实玩家时,
// 把骷髅标记打在真实玩家主坦克当前正在拉的怪(仇恨目标/当前目标)上
class FallbackMarkSkullAction : public Action
{
public:
    FallbackMarkSkullAction(PlayerbotAI* botAI) : Action(botAI, "fallback mark skull") {}

    bool Execute(Event event) override;
};

// By leewheel 2026-09-22: 主坦克标记月亮动作 —— 给"需要被控制"的精英怪打月亮。
// 参考 mod-playerbots-dungeon-lead 的 DungeonLeadMarkAction::FindCcCandidate：
//   以骷髅（boss）为锚点挑一只可被控制的精英，
//   让德鲁伊（缠绕/休眠/旋风）、术士（放逐/恐惧）等 CC 职业优先控制它
//   （这些职业的 RtiCcTrigger 子类直接以 "rti cc target"（默认 moon）为目标）。
class MarkMoonTargetAction : public Action
{
public:
    MarkMoonTargetAction(PlayerbotAI* botAI) : Action(botAI, "mark moon target") {}

    bool Execute(Event event) override;
};

// By leewheel 2026-09-22: 月亮（CC）标记生命周期维护动作。
// 参考 mod-playerbots-dungeon-lead 的 DungeonLead::CheckCcMark：
//   目标死亡/消失 → 释放标记；控制已落地 → 重置宽限窗；
//   被标怪尚未进战斗 → 不计时（因为上游 CcTargetValue 只遍历 attackers 列表，
//   未参战的怪对被控方而言"看不见"，这段时间不该计入超时）；
//   宽限窗或绝对上限到点 → 释放标记，避免永久占位挡住 DPS。
class CheckCcMarkAction : public Action
{
public:
    CheckCcMarkAction(PlayerbotAI* botAI) : Action(botAI, "check cc mark") {}

    bool Execute(Event event) override;
};

#endif
