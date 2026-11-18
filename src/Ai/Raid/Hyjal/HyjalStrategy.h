/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HYJALSTRATEGY_H
#define PLAYERBOTS_HYJALSTRATEGY_H

#include "Strategy.h"
#include <string>
#include <vector>

class RaidHyjalStrategy : public Strategy
{
public:
    RaidHyjalStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}
    std::string const getName() override { return "hyjal"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;

    // By leewheel 2026-11-18：合并 brighton 时上游整体删掉了 Hyjal 的
    //   AppendTargetExclusions 实现（TargetValueExclusionType 段），但三路合并把
    //   本文件这两行**声明**留了下来 ⇒ RaidHyjalStrategy 的 vtable 仍引用该虚函数，
    //   链接期 LNK2001（worldserver.exe）。基类 Strategy::AppendTargetExclusions 本身有
    //   空默认实现，删掉这两行即与上游一致，海加尔策略的其余逻辑不受影响。
    // End By leewheel
};

#endif
