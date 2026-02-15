#pragma once

#include <string>
#include <unordered_map>

class StrategyNameMapper
{
public:
    static void Load(const std::string& path);
    static std::string GetDisplayName(const std::string& internalName);

private:
    static std::unordered_map<std::string, std::string>& GetStrategyMap();
};
