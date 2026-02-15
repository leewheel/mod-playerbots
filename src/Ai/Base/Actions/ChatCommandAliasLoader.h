#pragma once

#include <string>
#include <unordered_map>

class CommandAliasTranslator
{
public:
    static void Load(const std::string& path);
    static std::string Translate(const std::string& input);

private:
    static std::string Normalize(std::string str);
    static std::unordered_map<std::string, std::string>& GetAliasMap();
};
