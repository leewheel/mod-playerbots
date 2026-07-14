/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_FLEEINGTARGETTRIGGERS_H
#define PLAYERBOTS_FLEEINGTARGETTRIGGERS_H

#include "Trigger.h"

class PlayerbotAI;

// By leewheel 2026-07-15: 逃跑怪优先集火触发器
// 条件：在副本/团本中 + 是坦克 + 有正在逃跑/即将逃跑的怪物
class FleeingTargetTrigger : public Trigger
{
public:
    FleeingTargetTrigger(PlayerbotAI* botAI) : Trigger(botAI, "fleeing target", 1) {}

    bool IsActive() override;
};

#endif
