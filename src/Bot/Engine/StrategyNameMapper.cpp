/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "StrategyNameMapper.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include "log.h"

std::unordered_map<std::string, std::string>& StrategyNameMapper::GetStrategyMap()
{
    static std::unordered_map<std::string, std::string> strategyMap;
    return strategyMap;
}

void StrategyNameMapper::Load(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        LOG_ERROR("playerbot", "无法打开策略名称映射文件: {}", path);
        return;
    }

    nlohmann::json json;
    try
    {
        file >> json;
    }
    catch (std::exception const& e)
    {
        LOG_ERROR("playerbot", "解析策略名称映射文件失败: {}", e.what());
        return;
    }

    auto& strategyMap = GetStrategyMap();
    for (auto& [internal, display] : json.items())
    {
        strategyMap[internal] = display.get<std::string>();
    }

    LOG_INFO("server", ">>>加载策略名称映射表成功，共 {} 项", strategyMap.size());
}

std::string StrategyNameMapper::GetDisplayName(const std::string& internalName)
{
    auto& strategyMap = GetStrategyMap();
    auto it = strategyMap.find(internalName);
    if (it != strategyMap.end())
        return it->second;

    return internalName;
}
