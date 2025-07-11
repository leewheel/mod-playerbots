//#pragma once
//
//#include <algorithm>
//#include <fstream>
//#include <nlohmann/json.hpp>
//#include <string>
//#include <unordered_map>
//#include "log.h"
//
//class CommandAliasTranslator
//{
//public:
//    static void Load(const std::string& path)
//    {
//        std::ifstream file(path);
//        if (!file.is_open())
//        {
//            LOG_ERROR("server", "[PB中文命令] 打开命令表: {} 失败", path);
//            return;
//        }
//
//        nlohmann::json data;
//        file >> data;
//
//        for (auto& [eng, aliases] : data.items())
//        {
//            for (auto& alias : aliases)
//            {
//                std::string zh = alias.get<std::string>();
//                _aliasMap[zh] = eng;
//            }
//        }
//
//        LOG_INFO("server", "[PB中文命令] 加载{}个中文命令成功",_aliasMap.size());
//    }
//
//    static std::string Translate(const std::string& input)
//    {
//        std::string cmd = Normalize(input);
//
//        auto it = _aliasMap.find(cmd);
//        if (it != _aliasMap.end())
//        {
//            LOG_INFO("server", "[PB中文命令] '{}' -> '{}'", cmd, it->second);
//            return it->second;
//        }
//
//        return cmd;
//    }
//
//private:
//    static std::string Normalize(std::string str)
//    {
//        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
//        str.erase(0, str.find_first_not_of(" \t\n\r"));
//        str.erase(str.find_last_not_of(" \t\n\r") + 1);
//        return str;
//    }
//
//    static inline std::unordered_map<std::string, std::string> _aliasMap;
//};
//#pragma once
//
//#include <algorithm>
//#include <fstream>
//#include <nlohmann/json.hpp>
//#include <string>
//#include <unordered_map>
//
//#include "log.h"
//
//class CommandAliasTranslator
//{
//public:
//    static void Load(const std::string& path)
//    {
//        std::ifstream file(path);
//        if (!file.is_open())
//        {
//            LOG_ERROR("server", "[PB中文命令] 打开命令表失败: {}", path);
//            return;
//        }
//
//        nlohmann::json data;
//        file >> data;
//
//        for (auto& [eng, aliases] : data.items())
//        {
//            for (auto& alias : aliases)
//            {
//                std::string zh = alias.get<std::string>();
//                _aliasMap[zh] = eng;
//            }
//        }
//
//        LOG_INFO("server", "[PB中文命令] 加载 {} 个中文命令成功", _aliasMap.size());
//    }
//
//    static std::string Translate(const std::string& input)
//    {
//        std::string cmd = Normalize(input);
//        //LOG_INFO("server", "[PB翻译] 原始='{}', Normalize='{}'", input, cmd);
//
//        // 精确匹配
//        auto it = _aliasMap.find(cmd);
//        if (it != _aliasMap.end())
//        {
//            LOG_INFO("server", "[PB中文命令]  精确匹配 '{}' → '{}'", cmd, it->second);
//            return it->second;
//        }
//
//        // 模糊匹配（双向包含）
//        for (const auto& [alias, eng] : _aliasMap)
//        {
//            if (alias.find(cmd) != std::string::npos || cmd.find(alias) != std::string::npos)
//            {
//                LOG_INFO("server", "[PB中文命令] 模糊匹配 '{}' <-> '{}' → '{}'", cmd, alias, eng);
//                return eng;
//            }
//        }
//
//       // LOG_INFO("server", "[PB中文命令] ❌ 未匹配命令: '{}'", cmd);
//        return cmd;
//    }
//
//private:
//    static std::string Normalize(std::string str)
//    {
//        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
//        str.erase(0, str.find_first_not_of(" \t\n\r"));
//        str.erase(str.find_last_not_of(" \t\n\r") + 1);
//        return str;
//    }
//
//    static inline std::unordered_map<std::string, std::string> _aliasMap;
//};
#pragma once

#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>

#include "Chat.h"
#include "Common.h"
#include "Log.h"

class CommandAliasTranslator
{
public:
    static void Load(const std::string& path)
    {
        std::ifstream file(path);
        if (!file.is_open())
        {
            LOG_ERROR("server", "[PB中文命令]  打开命令表失败: {}", path);
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

        LOG_INFO("server", "[PB中文命令]加载 {} 条中文命令映射成功", _aliasMap.size());
    }

    static std::string Translate(const std::string& input, uint32 chatType)
    {
        std::string cmd = Normalize(input);

        // 🎯 类型 1：/s 控制台命令，仅支持精确匹配
        if (chatType != CHAT_MSG_SAY)
        {
            auto it = _aliasMap.find(cmd);
            if (it != _aliasMap.end())
            {
                LOG_INFO("server", "[PB中文命令] 聊天精确匹配 '.{}' → '{}'", cmd, it->second);
                return it->second;
            }
            return input;  // ⛔ 不翻译
        }

        // 🎯 类型 2：/p /r /g /c 等聊天行为命令，支持模糊匹配
        if (chatType == CHAT_MSG_PARTY || chatType == CHAT_MSG_RAID || chatType == CHAT_MSG_RAID_WARNING ||
            chatType == CHAT_MSG_GUILD || chatType == CHAT_MSG_OFFICER || chatType == CHAT_MSG_CHANNEL)
        {
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
        }

        // 其它类型不处理
        return input;
    }


    static std::string TranslateForConsole(const std::string& input)
    {
        std::string cmd = Normalize(input);
        
        for (const auto& [alias, eng] : _aliasMap)
        {
            if (cmd == alias)  // 只允许精确匹配
            {
                // ✅ 增加限制：只有包含 playerbot 和 addclass 才允许进入控制台命令处理
                if (eng.find("playerbot") != std::string::npos && eng.find("addclass") != std::string::npos)
                {
                    return eng;
                }
                else
                {
                    LOG_INFO("server", "[PB中文命令] 匹配命令 '{}' → '{}'，但不包含关键字 'playerbot addclass'，忽略",
                             cmd, eng);
                    return input;
                }
            }
        }

        //std::string fallback = Translate(input, CHAT_MSG_PARTY);

        //if (fallback != input)
        //{
        //    LOG_INFO("server", "[PB中文命令] TranslateForConsole 回退模糊匹配 '{}' → '{}'", input, fallback);
        //    return fallback;
        //}
        return input;
    }

private:
    static std::string Normalize(std::string str)
    {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        str.erase(0, str.find_first_not_of(" \t\n\r"));
        str.erase(str.find_last_not_of(" \t\n\r") + 1);
        return str;
    }

    static inline std::unordered_map<std::string, std::string> _aliasMap;
};
