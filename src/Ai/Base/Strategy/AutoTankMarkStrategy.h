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
// By leewheel 2026-08-08: 扩展到副本外（野外）也标记（战场/竞技场除外）
// 通过配置参数 AiPlayerbot.AutoTankMarkEnabled 控制开关（默认开启）
// By leewheel 2026-07-15: 修复 GetType 返回值
// 原值 STRATEGY_TYPE_TANK 会导致所有加载了此策略的 Bot 被误判为坦克。
// 原因：ApplyInstanceStrategies 会给所有 Bot 添加 "auto tank mark" 策略，
//       如果此策略的 GetType() 返回 STRATEGY_TYPE_TANK，
//       则 Engine::Init() 会设置 strategyTypeMask 的 TANK 位，
//       导致 ContainsStrategy(STRATEGY_TYPE_TANK) 对所有 Bot 返回 true，
//       进而 IsTank(bot, false) 对所有 Bot 返回 true，
//       最终 LFG 角色检查中 GetRoles() 对所有 Bot 返回 PLAYER_ROLE_TANK。
//       这就是猎人和盗贼也会被分配为坦克的根因。
// 修复：改为 STRATEGY_TYPE_GENERIC，因为"自动标记"是辅助功能，不代表 Bot 是坦克。
class AutoTankMarkStrategy : public Strategy
{
public:
    AutoTankMarkStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "auto tank mark"; }
    uint32 GetType() const override { return STRATEGY_TYPE_GENERIC; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
