/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

//By leewheel 2026-09-22
// 赵与风专用独立策略：always need（全需求）
//   需求来源：老大要求设计一个特殊机器人「赵与风」（战士），
//   玩家组随机本或团本时 25% 概率组到他，他拥有独立策略——所有 Roll 的东西全需求。
//   本策略本身不挂任何触发（纯标记策略），仅作为「该机器人对任何 Roll 一律投 NEED」的开关，
//   由 LootRollAction 通过 botAI->HasStrategy("always need", ...) 查询。
//End By leewheel

#ifndef PLAYERBOTS_ALWAYS_NEED_STRATEGY_H
#define PLAYERBOTS_ALWAYS_NEED_STRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

class AlwaysNeedStrategy : public Strategy
{
public:
    AlwaysNeedStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "always need"; }
};

#endif
