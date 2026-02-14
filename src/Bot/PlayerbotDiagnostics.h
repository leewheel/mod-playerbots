/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license
 */

#ifndef _PLAYERBOT_DIAGNOSTICS_H
#define _PLAYERBOT_DIAGNOSTICS_H

#include "Common.h"
#include "Player.h"
#include <string>

class PlayerbotDiagnostics
{
public:
    static PlayerbotDiagnostics& instance()
    {
        static PlayerbotDiagnostics instance;
        return instance;
    }

    // 启用/禁用诊断
    void Enable() { enabled = true; }
    void Disable() { enabled = false; }
    bool IsEnabled() const { return enabled; }

    // 诊断输出方法
    void LogBotLogin(Player* bot, Player* master);
    void LogStrategyReset(Player* bot, bool hasGroup, bool isRandomBot);
    void LogStrategyAdded(Player* bot, const std::string& strategy, uint8 state);
    void LogStrategyRemoved(Player* bot, const std::string& strategy, uint8 state);
    void LogStrategyList(Player* bot, uint8 state);
    void LogGroupJoin(Player* bot, Player* master);
    void LogMasterSet(Player* bot, Player* master);
    void LogAIUpdate(Player* bot, const std::string& reason);
    void LogCommandReceived(Player* bot, const std::string& command, Player* from);
    void LogCommandProcessed(Player* bot, const std::string& command, bool success);

private:
    PlayerbotDiagnostics() : enabled(false) {}
    bool enabled;

    // 辅助方法
    std::string GetBotInfo(Player* bot);
    std::string GetPlayerInfo(Player* player);
};

#define sDiagnostics PlayerbotDiagnostics::instance()

// 诊断宏 - 只在启用时输出
#define DIAG_LOG(bot, format, ...) \
    if (sDiagnostics.IsEnabled()) { \
        LOG_INFO("playerbots.diag", "[DIAG] [{}] " format, (bot)->GetName(), ##__VA_ARGS__); \
    }

#endif
