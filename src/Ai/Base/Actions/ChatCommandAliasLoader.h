#pragma once

#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>

#include "log.h"

class CommandAliasTranslator
{
public:
    static void Load(const std::string& path)
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
            LOG_ERROR("server", "[PB中文命令]  解析 JSON 失败: {}", e.what());
            return;
        }

        for (auto& [eng, aliases] : data.items())
        {
            for (auto& alias : aliases)
            {
                std::string zh = Normalize(alias.get<std::string>());
                _aliasMap[zh] = eng;
            }
        }

        LOG_INFO("server", ">>>>[PB中文命令] 加载 {} 个中文命令成功", _aliasMap.size());
    }

    static std::string Translate(const std::string& input)
    {
        std::string cmd = Normalize(input);

        //if (chatType != CHAT_MSG_SAY)
        //{
            // 优先精确匹配
            auto it = _aliasMap.find(cmd);
            if (it != _aliasMap.end())
            {
                LOG_INFO("server", "[PB中文命令] 聊天精确匹配 '{}' → '{}'", cmd, it->second);
                return it->second;
            }

            // 模糊匹配
            for (const auto& [alias, eng] : _aliasMap)
            {
                if (cmd.find(alias) != std::string::npos || alias.find(cmd) != std::string::npos)
                {
                    LOG_INFO("server", "[PB中文命令] 聊天模糊匹配 '{}' ~ '{}' → '{}'", cmd, alias, eng);
                    return eng;
                }
            }
        //}

        // 其它类型不处理
        return input;
    }

private:
    //static std::string Normalize(std::string str)
    //{
    //    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    //    str.erase(0, str.find_first_not_of(" \t\n\r"));
    //    str.erase(str.find_last_not_of(" \t\n\r") + 1);
    //    return str;
    //}
    // In ChatCommandAliasLoader.h

    static std::string Normalize(std::string str)
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

    static inline std::unordered_map<std::string, std::string> _aliasMap;
};
