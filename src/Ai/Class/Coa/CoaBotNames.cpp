/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "CoaBotNames.h"

#include "CharacterCache.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "ObjectGuid.h"
#include "PlayerbotAIConfig.h"

#include <unordered_set>
#include <vector>

namespace
{
// 全中文名要求：bot 名以中文前缀「机器人 」开头，让玩家一眼分辨 bot 与真人 //By leewheel 2026年10月28日
constexpr char const* Prefix = "机器人 ";

bool HasPrefix(std::string const& name)
{
    std::string const prefix(Prefix);
    return name.size() > prefix.size() && name.compare(0, prefix.size(), prefix) == 0;
}
}  // namespace

std::string CoaBotName(std::string const& firstName)
{
    if (!sPlayerbotAIConfig.coaBotSurname || firstName.find(' ') != std::string::npos)
        return firstName;
    return std::string(Prefix) + firstName;
}

void ApplyCoaBotSurnames()
{
    // At startup only: a config reload runs this again with the bots online, whose names would
    // then differ from the ones the database and the name cache hold.
    static bool applied = false;
    if (applied)
        return;
    applied = true;

    bool const wanted = sPlayerbotAIConfig.coaBotSurname;

    std::unordered_set<uint32> accounts;
    if (QueryResult result = LoginDatabase.Query("SELECT id FROM account WHERE username LIKE '{}%'",
                                                 sPlayerbotAIConfig.randomBotAccountPrefix))
        do
            accounts.insert(result->Fetch()[0].Get<uint32>());
        while (result->NextRow());
    if (accounts.empty())
        return;

    struct Row { uint32 guid; uint32 account; std::string name; };
    std::vector<Row> rows;
    std::unordered_set<std::string> names;
    if (QueryResult result = CharacterDatabase.Query("SELECT guid, account, name FROM characters"))
        do
        {
            Field* fields = result->Fetch();
            rows.push_back({ fields[0].Get<uint32>(), fields[1].Get<uint32>(), fields[2].Get<std::string>() });
            names.insert(rows.back().name);
        } while (result->NextRow());

    uint32 renamed = 0, skipped = 0;
    CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
    for (Row const& row : rows)
    {
        if (!accounts.count(row.account))
            continue;

        bool const has = HasPrefix(row.name);
        if (has == wanted || (wanted && row.name.find(' ') != std::string::npos))
            continue;

        std::string renamedTo = wanted ? std::string(Prefix) + row.name : row.name.substr(std::string(Prefix).size());
        // A player may hold the name already: that bot keeps its own.
        if (names.count(renamedTo))
        {
            ++skipped;
            continue;
        }

        names.erase(row.name);
        names.insert(renamedTo);
        sCharacterCache->UpdateCharacterData(ObjectGuid::Create<HighGuid::Player>(row.guid), renamedTo);
        CharacterDatabase.EscapeString(renamedTo);
        trans->Append("UPDATE characters SET name = '{}' WHERE guid = {}", renamedTo, row.guid);
        ++renamed;
    }
    CharacterDatabase.CommitTransaction(trans);

    if (renamed || skipped)
        LOG_INFO("server.loading", ">> {} 中文前缀 \"机器人\" 于 {} 个随机 bot（{} 个名字已被占用）",
                 wanted ? "加上" : "移除", renamed, skipped);
}
