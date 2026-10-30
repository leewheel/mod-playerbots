/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// 机器人高频日志的周期统计：各刷屏位点只做一次原子计数，由 BotLogStatsWorldScript 每 5 分钟
// 汇总成一行中文输出到 Playerbots.log（LOG_INFO 级），取代逐条刷屏的 DEBUG 明细。
// 需要逐条明细排查时，把 worldserver.conf 的 Logger.playerbots 调回 5 即可，原 DEBUG 日志都还在。
//By leewheel 2026年10月31日

#ifndef _BOT_LOG_STATS_H_
#define _BOT_LOG_STATS_H_

#include "Define.h"

enum BotLogStat : uint8
{
    BOT_LOG_DEATH_RELEASE = 0,  // 死亡后释放灵魂
    BOT_LOG_QUEST_ACCEPT,       // RPG 接取任务
    BOT_LOG_QUEST_TURN_IN,      // RPG 交付任务
    BOT_LOG_QUEST_DROP,         // RPG 放弃任务
    BOT_LOG_RANDOM_TELEPORT,    // 随机传送
    BOT_LOG_REFRESH,            // 机器人数据刷新
    BOT_LOG_RANDOMIZED,         // 装备随机化
    BOT_LOG_COA_GEAR,           // CoA 装备更新

    BOT_LOG_STAT_COUNT
};

// 机器人逻辑跑在各地图线程，这里只需原子自增，OnUpdate 汇总时再取走清零。
void AddBotLogStat(BotLogStat stat);

#endif
