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

#endif
