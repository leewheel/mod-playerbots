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
            LOG_ERROR("server", "[PB中文命令] 打开命令表: {} 失败", path);
            return;
        }

        nlohmann::json data;
        file >> data;

        for (auto& [eng, aliases] : data.items())
        {
            for (auto& alias : aliases)
            {
                std::string zh = alias.get<std::string>();
                _aliasMap[zh] = eng;
            }
        }

        LOG_INFO("server", "[PB中文命令] 加载 {} 个中文命令成功", _aliasMap.size());
    }

    static std::string Translate(const std::string& input)
    {
        std::string cmd = Normalize(input);

        // ✅ 1. 先做精确匹配
        auto it = _aliasMap.find(cmd);
        if (it != _aliasMap.end())
        {
            LOG_INFO("server", "[PB中文命令] 精确匹配 '{}' → '{}'", cmd, it->second);
            return it->second;
        }

        // ✅ 2. 模糊匹配（alias 包含在 cmd 中）
        for (const auto& [alias, eng] : _aliasMap)
        {
            if (cmd.find(alias) != std::string::npos)
            {
                LOG_INFO("server", "[PB中文命令] 模糊匹配 '{}' 包含 '{}' → '{}'", cmd, alias, eng);
                return eng;
            }
        }

        // ❌ 未匹配
        return cmd;
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
