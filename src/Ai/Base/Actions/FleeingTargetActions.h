/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_FLEEINGTARGETACTIONS_H
#define PLAYERBOTS_FLEEINGTARGETACTIONS_H

#include "Action.h"

class PlayerbotAI;

// By leewheel 2026-07-15: 逃跑怪优先集火动作
// 检测到正在逃跑/寻求支援的怪物时，立即将其标记为骷髅（覆盖现有骷髅），
// 并设为优先集火目标，让所有机器人切换目标集火击杀
class PrioritizeFleeingTargetAction : public Action
{
public:
    PrioritizeFleeingTargetAction(PlayerbotAI* botAI) : Action(botAI, "prioritize fleeing target") {}

    bool Execute(Event event) override;
};

#endif
