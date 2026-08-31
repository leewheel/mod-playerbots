/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BATTLEGROUNDSTRATEGY_H
#define PLAYERBOTS_BATTLEGROUNDSTRATEGY_H

#include "PassThroughStrategy.h"

class BGStrategy : public PassThroughStrategy
{
public:
    BGStrategy(PlayerbotAI* botAI);

    uint32 GetType() const override { return STRATEGY_TYPE_NONCOMBAT; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "bg"; }
};

class BattlegroundStrategy : public Strategy
{
public:
    BattlegroundStrategy(PlayerbotAI* botAI) : Strategy(botAI){};

    uint32 GetType() const override { return STRATEGY_TYPE_NONCOMBAT; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "Battleground"; }
};

// By leewheel 2026-09-01
// PVP 交战循环策略（修复批次2/8-29 挂载点缺陷）：
//   原六条 PVP 自保/循环触发链挂在 BattlegroundStrategy（NONCOMBAT 类型，只在脱战引擎生效），
//   而"被控→解控→远遁→绷带"恰恰全部发生在战斗状态——战斗引擎加载的是 warsong/alterac/
//   arathi/eye/isle（与本类平级，非父子），导致触发链在战斗中不生效。
//   本策略类型 GENERIC，由 AiFactory 战场分支同时加入战斗/非战斗引擎；
//   触发器自带 GetBattleground/InArena 守卫，野外与副本挂载无副作用。
// End By leewheel
class PvpCycleStrategy : public Strategy
{
public:
    PvpCycleStrategy(PlayerbotAI* botAI) : Strategy(botAI){};

    uint32 GetType() const override { return STRATEGY_TYPE_GENERIC; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "pvp cycle"; }
};

class WarsongStrategy : public Strategy
{
public:
    WarsongStrategy(PlayerbotAI* botAI) : Strategy(botAI){};

    uint32 GetType() const override { return STRATEGY_TYPE_GENERIC; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "warsong"; }
};

class AlteracStrategy : public Strategy
{
public:
    AlteracStrategy(PlayerbotAI* botAI) : Strategy(botAI){};

    uint32 GetType() const override { return STRATEGY_TYPE_GENERIC; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "alterac"; }
};

class ArathiStrategy : public Strategy
{
public:
    ArathiStrategy(PlayerbotAI* botAI) : Strategy(botAI){};

    uint32 GetType() const override { return STRATEGY_TYPE_GENERIC; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "arathi"; }
};

class EyeStrategy : public Strategy
{
public:
    EyeStrategy(PlayerbotAI* botAI) : Strategy(botAI){};

    uint32 GetType() const override { return STRATEGY_TYPE_GENERIC; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "eye"; }
};

class IsleStrategy : public Strategy
{
public:
    IsleStrategy(PlayerbotAI* botAI) : Strategy(botAI){};

    uint32 GetType() const override { return STRATEGY_TYPE_GENERIC; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "isle"; }
};

class ArenaStrategy : public Strategy
{
public:
    ArenaStrategy(PlayerbotAI* botAI) : Strategy(botAI){};

    uint32 GetType() const override { return STRATEGY_TYPE_GENERIC; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "arena"; }
};

#endif
