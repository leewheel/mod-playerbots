/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_AUTOTANKMARKSTRATEGY_H
#define PLAYERBOTS_AUTOTANKMARKSTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

// By leewheel 2026-07-15: 自动坦克标记策略
// 在副本/团本中，主坦克自动标记骷髅，副坦克自动标记叉叉
// 通过配置参数 AiPlayerbot.AutoTankMarkEnabled 控制开关（默认开启）
class AutoTankMarkStrategy : public Strategy
{
public:
    AutoTankMarkStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "auto tank mark"; }
    uint32 GetType() const override { return STRATEGY_TYPE_TANK; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
