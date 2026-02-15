/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "ChatCommandAliasLoader.h"

#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>
#include "log.h"

std::unordered_map<std::string, std::string>& CommandAliasTranslator::GetAliasMap()
{
    static std::unordered_map<std::string, std::string> aliasMap;
    return aliasMap;
}

std::string CommandAliasTranslator::Normalize(std::string str)
{
    // FIX: Safely lowercase ONLY ASCII characters.
    // Standard ::tolower breaks multibyte UTF-8 characters (like 坦克).

    for (char& c : str)
    {
        // Only lowercase if it's a standard ASCII letter (A-Z)
        if (static_cast<unsigned char>(c) < 128)
        {
            c = std::tolower(static_cast<unsigned char>(c));
        }
    }

    // Standard Trim Logic (Keep this)
    str.erase(0, str.find_first_not_of(" \t\n\r"));
    if (str.find_last_not_of(" \t\n\r") != std::string::npos)
        str.erase(str.find_last_not_of(" \t\n\r") + 1);

    return str;
}

void CommandAliasTranslator::Load(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        LOG_ERROR("server", "[PB中文命令] 打开命令表失败: {}", path);
        return;
    }

    nlohmann::json data;
    try
    {
        file >> data;
    }
    catch (std::exception const& e)
    {
        LOG_ERROR("server", "[PB中文命令] 解析 JSON 失败: {}", e.what());
        return;
    }

    auto& aliasMap = GetAliasMap();
    for (auto& [eng, aliases] : data.items())
    {
        for (auto& alias : aliases)
        {
            std::string zh = Normalize(alias.get<std::string>());
            aliasMap[zh] = eng;
        }
    }

    LOG_INFO("server", ">>>>[PB中文命令] 加载 {} 个中文命令成功", aliasMap.size());
}

std::string CommandAliasTranslator::Translate(const std::string& input)
{
    std::string cmd = Normalize(input);
    auto& aliasMap = GetAliasMap();

    // 优先精确匹配
    auto it = aliasMap.find(cmd);
    if (it != aliasMap.end())
    {
        LOG_INFO("server", "[PB中文命令] 聊天精确匹配 '{}' → '{}'", cmd, it->second);
        return it->second;
    }

    // 模糊匹配
    for (const auto& [alias, eng] : aliasMap)
    {
        if (cmd.find(alias) != std::string::npos || alias.find(cmd) != std::string::npos)
        {
            LOG_INFO("server", "[PB中文命令] 聊天模糊匹配 '{}' ~ '{}' → '{}'", cmd, alias, eng);
            return eng;
        }
    }

    // 其它类型不处理
    return input;
}
