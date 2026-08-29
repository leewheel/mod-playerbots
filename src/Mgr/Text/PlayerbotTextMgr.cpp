/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PlayerbotTextMgr.h"
#include "DatabaseEnv.h"
#include "QueryResult.h"    // Required due to a poor implementation by AC
#include "Random.h"
#include "WorldSessionMgr.h"

void PlayerbotTextMgr::replaceAll(std::string& str, const std::string& from, const std::string& to)
{
    if (from.empty())
        return;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();  // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

void PlayerbotTextMgr::LoadBotTexts()
{
    LOG_INFO("playerbots", "正在加载玩家机器人文本...");

    uint32 count = 0;
    if (PreparedQueryResult result =
            PlayerbotsDatabase.Query(PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_TEXT)))
    {
        do
        {
            std::map<uint32, std::string> text;
            Field* fields = result->Fetch();
            std::string name = fields[0].Get<std::string>();
            text[0] = fields[1].Get<std::string>();
            uint8 sayType = fields[2].Get<uint8>();
            uint8 replyType = fields[3].Get<uint8>();
            for (uint8 i = 1; i < TOTAL_LOCALES; ++i)
            {
                text[i] = fields[i + 3].Get<std::string>();
            }

            botTexts[name].push_back(BotTextEntry(name, text, sayType, replyType));
            ++count;
        } while (result->NextRow());
    }

    LOG_INFO("playerbots", "已加载 {} 条玩家机器人文本", count);
}

void PlayerbotTextMgr::LoadBotTextChance()
{
    if (botTextChance.empty())
    {
        QueryResult results = PlayerbotsDatabase.Query("SELECT name, probability FROM ai_playerbot_texts_chance");
        if (results)
        {
            do
            {
                Field* fields = results->Fetch();
                std::string name = fields[0].Get<std::string>();
                uint32 probability = fields[1].Get<uint32>();

                botTextChance[name] = probability;
            } while (results->NextRow());
        }
    }
}

// general texts

std::string PlayerbotTextMgr::GetBotText(std::string name)
{
    if (botTexts.empty())
    {
        LOG_ERROR("playerbots", "无法获取机器人文本 {}！未加载任何机器人文本！", name);
        return "";
    }

    if (botTexts[name].empty())
    {
        LOG_ERROR("playerbots", "无法获取机器人文本 {}！没有此名称的机器人文本！", name);
        return "";
    }

    std::vector<BotTextEntry>& list = botTexts[name];
    BotTextEntry textEntry = list[urand(0, list.size() - 1)];
    return !textEntry.m_text[GetLocalePriority()].empty() ? textEntry.m_text[GetLocalePriority()] : textEntry.m_text[0];
}

std::string PlayerbotTextMgr::GetBotText(std::string name, std::map<std::string, std::string> placeholders)
{
    std::string botText = GetBotText(name);
    if (botText.empty())
        return "";

    for (std::map<std::string, std::string>::iterator i = placeholders.begin(); i != placeholders.end(); ++i)
        replaceAll(botText, i->first, i->second);

    return botText;
}

std::string PlayerbotTextMgr::GetBotTextOrDefault(std::string name, std::string defaultText,
    std::map<std::string, std::string> placeholders)
{
    // By leewheel 2026-08-29 - 带默认值的查询属于"缺失是预期"的路径：先静默判断文本是否加载，
    //   只有库里确实有该文本时才走GetBotText(占位符替换)，缺失时直接用默认值，不再刷"没有此名称的机器人文本"错误日志。
    auto const itr = botTexts.find(name);
    if (itr != botTexts.end() && !itr->second.empty())
    {
        std::string botText = GetBotText(name, placeholders);
        if (!botText.empty())
            return botText;
    }

    for (std::map<std::string, std::string>::iterator i = placeholders.begin(); i != placeholders.end(); ++i)
    {
        replaceAll(defaultText, i->first, i->second);
    }
    return defaultText;
}

// chat replies

std::string PlayerbotTextMgr::GetBotText(ChatReplyType replyType, std::map<std::string, std::string> placeholders)
{
    if (botTexts.empty())
    {
        LOG_ERROR("playerbots", "无法获取机器人文本回复 {}！未加载任何机器人文本！", replyType);
        return "";
    }
    if (botTexts["reply"].empty())
    {
        LOG_ERROR("playerbots", "无法获取机器人文本回复 {}！没有机器人文本回复！", replyType);
        return "";
    }

    std::vector<BotTextEntry>& list = botTexts["reply"];
    std::vector<BotTextEntry> proper_list;
    for (auto text : list)
    {
        if (text.m_replyType == replyType)
            proper_list.push_back(text);
    }

    if (proper_list.empty())
        return "";

    BotTextEntry textEntry = proper_list[urand(0, proper_list.size() - 1)];
    std::string botText =
        !textEntry.m_text[GetLocalePriority()].empty() ? textEntry.m_text[GetLocalePriority()] : textEntry.m_text[0];
    for (auto& placeholder : placeholders)
        replaceAll(botText, placeholder.first, placeholder.second);

    return botText;
}

std::string PlayerbotTextMgr::GetBotText(ChatReplyType replyType, std::string name)
{
    std::map<std::string, std::string> placeholders;
    placeholders["%s"] = name;

    return GetBotText(replyType, placeholders);
}

// probabilities

bool PlayerbotTextMgr::rollTextChance(std::string name)
{
    if (!botTextChance[name])
        return true;

    return urand(0, 100) < botTextChance[name];
}

bool PlayerbotTextMgr::GetBotText(std::string name, std::string& text)
{
    if (!rollTextChance(name))
        return false;

    text = GetBotText(name);
    return !text.empty();
}

bool PlayerbotTextMgr::GetBotText(std::string name, std::string& text, std::map<std::string, std::string> placeholders)
{
    if (!rollTextChance(name))
        return false;

    text = GetBotText(name, placeholders);
    return !text.empty();
}

void PlayerbotTextMgr::AddLocalePriority(uint32 locale)
{
    if (locale >= TOTAL_LOCALES)
    {
        LOG_WARN("playerbots", "忽略机器人文本的语言 {}，因为超出 TOTAL_LOCALES ({})", locale, TOTAL_LOCALES - 1);
        return;
    }

    botTextLocalePriority[locale]++;
}

uint32 PlayerbotTextMgr::GetLocalePriority()
{
    // if no real players online, reset top locale
    uint32 const activeSessions = sWorldSessionMgr->GetActiveSessionCount();
    if (!activeSessions)
    {
        ResetLocalePriority();
        return 0;
    }

    uint32 topLocale = 0;
    for (uint8 i = 0; i < TOTAL_LOCALES; ++i)
    {
        if (botTextLocalePriority[i] > botTextLocalePriority[topLocale])
            topLocale = i;
    }

    return topLocale;
}

void PlayerbotTextMgr::ResetLocalePriority()
{
    for (uint8 i = 0; i < TOTAL_LOCALES; ++i)
    {
        botTextLocalePriority[i] = 0;
    }
}
