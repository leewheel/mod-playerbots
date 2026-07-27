/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

//By leewheel 2026-07-27 引入brighton-chi的UB(幽暗沼泽)副本策略
// 幽暗沼泽（Underbog）副本策略头文件，定义 TbcDungeonUnderbogStrategy 策略类
// 包含 Hungarfen（饥饿者）遭遇战的触发器、乘数和目标排除逻辑
#ifndef PLAYERBOTS_UBSTRATEGY_H
#define PLAYERBOTS_UBSTRATEGY_H

#include "AiObjectContext.h"
#include "Multiplier.h"
#include "Strategy.h"

// 幽暗沼泽副本策略类
class TbcDungeonUnderbogStrategy : public Strategy
{
public:
    TbcDungeonUnderbogStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    virtual std::string const getName() override { return "tbc-ub"; }

    virtual void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    virtual void InitMultipliers(std::vector<Multiplier*>& multipliers) override;

    // 启用目标排除：DPS不攻击蘑菇，专注于Boss
    bool HasTargetExclusions() const override { return true; }
    void AppendTargetExclusions(GuidSet& exclusions, TargetValueExclusionType type) override;
};

#endif
