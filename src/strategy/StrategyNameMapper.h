#pragma once

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include "log.h"

class StrategyNameMapper
{
public:
    static void Load(const std::string& path)
    {
        std::ifstream file(path);
        if (!file.is_open())
        {
            LOG_ERROR("playerbot", "无法打开策略名称映射文件: {}", path);
            return;
        }

        nlohmann::json json;
        file >> json;

        for (auto& [internal, display] : json.items())
        {
            strategyMap[internal] = display.get<std::string>();
        }

        LOG_INFO("playerbot", "加载策略名称映射表成功，共 {} 项", strategyMap.size());
    }

    static std::string GetDisplayName(const std::string& internalName)
    {
        auto it = strategyMap.find(internalName);
        if (it != strategyMap.end())
            return it->second;

        return internalName;
    }

private:
    static inline std::unordered_map<std::string, std::string> strategyMap;
};
