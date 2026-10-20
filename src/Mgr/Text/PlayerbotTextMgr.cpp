/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PlayerbotTextMgr.h"
#include "PlayerbotsDatabase.h"
#include "DatabaseEnv.h"
#include "QueryResult.h"    // Required due to a poor implementation by AC
#include "Random.h"
#include "WorldSessionMgr.h"

namespace
{
// %item prefixes %item_link, and the map's reverse order replaces the longer key first.
void ApplyPlaceholders(std::string& text, std::map<std::string, std::string> const& placeholders)
{
    for (auto it = placeholders.rbegin(); it != placeholders.rend(); ++it)
        PlayerbotTextMgr::replaceAll(text, it->first, it->second);
}
}

void PlayerbotTextMgr::replaceAll(std::string& str, std::string const& from, std::string const& to)
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
            // By leewheel 2026-10-16 修复 acore 上游 #28081（allow modules to support a new locale）
            // 把 TOTAL_LOCALES 从 9 扩到 16（slot 9..15 留给模块注册）导致的越界段错误：
            // 表 ai_playerbot_texts 的 locale 列固定为 text_loc1..text_loc8（对应 LocaleConstant
            // 1..8 即 LOCALE_koKR..LOCALE_ruRU），SELECT 恒为 12 列。循环上界必须绑定表 schema，
            // 不能跟 TOTAL_LOCALES 挂钩——原写法 i 跑到 15 会越界读 fields[12..18] 的垃圾字段，
            // Field::GetDataString 用坏指针构造 std::string 时 ACCESS_VIOLATION（启动即崩，
            // 转储指 LoadBotTexts+328）。上游 core 侧同批适配模式为绑定已注册列数（见
            // ObjectMgr::LoadAcoreStrings），此处按表固定 8 列遍历到 LOCALE_ruRU。
            // By leewheel 2026-10-21 与上游 brighton 合并冲突处置：上游 2ec63794「temp crash fix
            // re text columns」改的是同一处崩溃，写法为 constexpr uint8 textColumns = 8，
            // 与此处 LOCALE_ruRU(=8) 语义等价（二者均为表固定 8 列）。按「只叠加不删改」保留
            // 本核语义化写法（绑定 LocaleConstant 而非裸魔数），并在此注明上游已同步修法，
            // 后续若表结构再扩列，两边须同改。
            for (uint8 i = 1; i <= LOCALE_ruRU; ++i)
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
        PlayerbotsDatabasePreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_TEXT_CHANCE);
        PreparedQueryResult results = PlayerbotsDatabase.Query(stmt);
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

    ApplyPlaceholders(botText, placeholders);

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
        // By leewheel 2026-10-17 合并：保留本核写法——库里确实有该文本时优先用真实文本
        // （GetBotText 内部按 locale 优先级取，含占位符替换），取不到再落到默认值；
        // 上游 PR 直接 ApplyPlaceholders(defaultText) 会绕过汉化文本库，故不采纳。
        std::string botText = GetBotText(name, placeholders);
        if (!botText.empty())
            return botText;
        // End By leewheel
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
    ApplyPlaceholders(botText, placeholders);

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
