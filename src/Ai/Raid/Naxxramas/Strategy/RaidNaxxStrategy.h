/*
 * 版权所有 (C) 2026 Leewheel
 * 
 * 文件功能：纳克萨玛斯团队副本机器人策略头文件
 * 定义纳克萨玛斯副本的AI策略类，包括触发器和倍率初始化
 * 
 * By Leewheel 2026-02-14
 */

#ifndef _PLAYERBOT_RAIDNAXXSTRATEGY_H_
#define _PLAYERBOT_RAIDNAXXSTRATEGY_H_

#include "Strategy.h"

class RaidNaxxStrategy : public Strategy
{
public:
    RaidNaxxStrategy(PlayerbotAI* ai) : Strategy(ai) {}

    std::string const getName() override { return "naxx"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif

// By Leewheel 2026-02-14
