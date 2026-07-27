/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TKSTRATEGY_H
#define PLAYERBOTS_TKSTRATEGY_H

#include "Strategy.h"

//By leewheel 2026-07-27 引入brighton-chi的TK目标排除功能
// 新增 HasTargetExclusions 和 AppendTargetExclusions 声明
// 用于近战DPS排除毁灭斧和灰烬之子
class RaidTempestKeepStrategy : public Strategy
{
public:
    RaidTempestKeepStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "tempestkeep"; }

    // 启用目标排除：近战DPS排除特定目标
    bool HasTargetExclusions() const override { return true; }
    void AppendTargetExclusions(GuidSet& exclusions, TargetValueExclusionType type) override;
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
