/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// 机器人高频日志的周期统计汇总：每 5 分钟把各刷屏位点累计的次数取走清零，输出一行中文。
// 本周期没有任何统计到的机器人动作时不输出，避免空闲时段刷无意义的空行。
//By leewheel 2026年10月31日

#include "BotLogStats.h"

#include "Log.h"
#include "ScriptMgr.h"

#include <array>
#include <atomic>

namespace
{
// 汇总周期（毫秒）：5 分钟
constexpr uint32 STATS_INTERVAL_MS = 300000;

std::array<std::atomic<uint32>, BOT_LOG_STAT_COUNT> StatCounters{};
}

void AddBotLogStat(BotLogStat stat)
{
    if (stat >= BOT_LOG_STAT_COUNT)
        return;

    StatCounters[stat].fetch_add(1, std::memory_order_relaxed);
}

class BotLogStatsWorldScript : public WorldScript
{
public:
    BotLogStatsWorldScript() : WorldScript("BotLogStatsWorldScript", { WORLDHOOK_ON_UPDATE }) { }

    void OnUpdate(uint32 diff) override
    {
        _sinceLastReport += diff;
        if (_sinceLastReport < STATS_INTERVAL_MS)
            return;

        _sinceLastReport = 0;

        uint32 counts[BOT_LOG_STAT_COUNT] = {};
        uint32 total = 0;
        for (uint8 i = 0; i < BOT_LOG_STAT_COUNT; ++i)
        {
            counts[i] = StatCounters[i].exchange(0, std::memory_order_relaxed);
            total += counts[i];
        }

        if (!total)
            return;

        LOG_INFO("playerbots",
                 "[机器人日志统计] 近5分钟：死亡释放 {}，接取任务 {}，交付任务 {}，放弃任务 {}，"
                 "随机传送 {}，刷新 {}，装备随机化 {}，装备更新 {}",
                 counts[BOT_LOG_DEATH_RELEASE], counts[BOT_LOG_QUEST_ACCEPT], counts[BOT_LOG_QUEST_TURN_IN],
                 counts[BOT_LOG_QUEST_DROP], counts[BOT_LOG_RANDOM_TELEPORT], counts[BOT_LOG_REFRESH],
                 counts[BOT_LOG_RANDOMIZED], counts[BOT_LOG_COA_GEAR]);
    }

private:
    uint32 _sinceLastReport = 0;
};

void AddSC_bot_log_stats()
{
    new BotLogStatsWorldScript();
}
