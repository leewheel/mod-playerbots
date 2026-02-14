/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license
 */

#include "PlayerbotDiagnostics.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Group.h"
#include "Log.h"

std::string PlayerbotDiagnostics::GetBotInfo(Player* bot)
{
    if (!bot)
        return "NULL";
    
    std::ostringstream out;
    out << bot->GetName() << " (Level " << (uint32)bot->GetLevel() << ")";
    return out.str();
}

std::string PlayerbotDiagnostics::GetPlayerInfo(Player* player)
{
    if (!player)
        return "NULL";
    
    std::ostringstream out;
    out << player->GetName();
    return out.str();
}

void PlayerbotDiagnostics::LogBotLogin(Player* bot, Player* master)
{
    if (!enabled || !bot)
        return;
    
    LOG_INFO("playerbots.diag", "========================================");
    LOG_INFO("playerbots.diag", "[DIAG] BOT LOGIN: {}", GetBotInfo(bot));
    LOG_INFO("playerbots.diag", "[DIAG]   Master: {}", GetPlayerInfo(master));
    LOG_INFO("playerbots.diag", "[DIAG]   In Group: {}", bot->GetGroup() ? "YES" : "NO");
    if (bot->GetGroup())
    {
        LOG_INFO("playerbots.diag", "[DIAG]   Group Size: {}", bot->GetGroup()->GetMembersCount());
        LOG_INFO("playerbots.diag", "[DIAG]   Group Leader: {}", 
            bot->GetGroup()->GetLeaderName());
    }
    LOG_INFO("playerbots.diag", "[DIAG]   In Battleground: {}", bot->InBattleground() ? "YES" : "NO");
    LOG_INFO("playerbots.diag", "========================================");
}

void PlayerbotDiagnostics::LogStrategyReset(Player* bot, bool hasGroup, bool isRandomBot)
{
    if (!enabled || !bot)
        return;
    
    LOG_INFO("playerbots.diag", "[DIAG] [{}] STRATEGY RESET", bot->GetName());
    LOG_INFO("playerbots.diag", "[DIAG] [{}]   Has Group: {}", bot->GetName(), hasGroup ? "YES" : "NO");
    LOG_INFO("playerbots.diag", "[DIAG] [{}]   Is Random Bot: {}", bot->GetName(), isRandomBot ? "YES" : "NO");
}

void PlayerbotDiagnostics::LogStrategyAdded(Player* bot, const std::string& strategy, uint8 state)
{
    if (!enabled || !bot)
        return;
    
    const char* stateName = "UNKNOWN";
    switch (state)
    {
        case 0: stateName = "COMBAT"; break;
        case 1: stateName = "NON_COMBAT"; break;
        case 2: stateName = "DEAD"; break;
    }
    
    LOG_INFO("playerbots.diag", "[DIAG] [{}] STRATEGY ADDED: '{}' (State: {})", 
        bot->GetName(), strategy, stateName);
}

void PlayerbotDiagnostics::LogStrategyRemoved(Player* bot, const std::string& strategy, uint8 state)
{
    if (!enabled || !bot)
        return;
    
    const char* stateName = "UNKNOWN";
    switch (state)
    {
        case 0: stateName = "COMBAT"; break;
        case 1: stateName = "NON_COMBAT"; break;
        case 2: stateName = "DEAD"; break;
    }
    
    LOG_INFO("playerbots.diag", "[DIAG] [{}] STRATEGY REMOVED: '{}' (State: {})", 
        bot->GetName(), strategy, stateName);
}

void PlayerbotDiagnostics::LogStrategyList(Player* bot, uint8 state)
{
    if (!enabled || !bot)
        return;
    
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
        return;
    
    const char* stateName = "UNKNOWN";
    switch (state)
    {
        case 0: stateName = "COMBAT"; break;
        case 1: stateName = "NON_COMBAT"; break;
        case 2: stateName = "DEAD"; break;
    }
    
    LOG_INFO("playerbots.diag", "[DIAG] [{}] STRATEGY LIST (State: {}):", bot->GetName(), stateName);
    
    // 获取策略列表
    std::vector<std::string> strategies = botAI->GetStrategies((BotState)state);
    if (strategies.empty())
    {
        LOG_INFO("playerbots.diag", "[DIAG] [{}]   (No strategies)", bot->GetName());
    }
    else
    {
        for (const auto& strategy : strategies)
        {
            LOG_INFO("playerbots.diag", "[DIAG] [{}]   - {}", bot->GetName(), strategy);
        }
    }
}

void PlayerbotDiagnostics::LogGroupJoin(Player* bot, Player* master)
{
    if (!enabled || !bot)
        return;
    
    LOG_INFO("playerbots.diag", "[DIAG] [{}] GROUP JOIN", bot->GetName());
    LOG_INFO("playerbots.diag", "[DIAG] [{}]   Master: {}", bot->GetName(), GetPlayerInfo(master));
    if (bot->GetGroup())
    {
        LOG_INFO("playerbots.diag", "[DIAG] [{}]   Group Size: {}", bot->GetName(), 
            bot->GetGroup()->GetMembersCount());
    }
}

void PlayerbotDiagnostics::LogMasterSet(Player* bot, Player* master)
{
    if (!enabled || !bot)
        return;
    
    LOG_INFO("playerbots.diag", "[DIAG] [{}] MASTER SET: {}", bot->GetName(), GetPlayerInfo(master));
}

void PlayerbotDiagnostics::LogAIUpdate(Player* bot, const std::string& reason)
{
    if (!enabled || !bot)
        return;
    
    LOG_INFO("playerbots.diag", "[DIAG] [{}] AI UPDATE: {}", bot->GetName(), reason);
}

void PlayerbotDiagnostics::LogCommandReceived(Player* bot, const std::string& command, Player* from)
{
    if (!enabled || !bot)
        return;
    
    LOG_INFO("playerbots.diag", "[DIAG] [{}] COMMAND RECEIVED: '{}' from {}", 
        bot->GetName(), command, GetPlayerInfo(from));
}

void PlayerbotDiagnostics::LogCommandProcessed(Player* bot, const std::string& command, bool success)
{
    if (!enabled || !bot)
        return;
    
    LOG_INFO("playerbots.diag", "[DIAG] [{}] COMMAND PROCESSED: '{}' - {}", 
        bot->GetName(), command, success ? "SUCCESS" : "FAILED");
}
