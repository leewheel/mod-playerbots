/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "TellReputationAction.h"

#include <algorithm>

#include "Event.h"
#include "PlayerbotAI.h"
#include "ReputationMgr.h"

#include "SharedDefines.h"

std::string TellReputationAction::BuildReputationLine(FactionEntry const* entry)
{
    ReputationMgr& repMgr = bot->GetReputationMgr();
    ReputationRank rank = repMgr.GetRank(entry);
    int32 reputation = repMgr.GetReputation(entry->ID);

    std::ostringstream out;
    out << entry->name[0] << ": |cff";

    switch (rank)
    {
        case REP_HATED:
            out << "cc2222憎恨";
            break;
        case REP_HOSTILE:
            out << "ff0000敌对";
            break;
        case REP_UNFRIENDLY:
            out << "ee6622冷淡";
            break;
        case REP_NEUTRAL:
            out << "ffff00中立";
            break;
        case REP_FRIENDLY:
            out << "00ff00友善";
            break;
        case REP_HONORED:
            out << "00ff88尊敬";
            break;
        case REP_REVERED:
            out << "00ffcc崇敬";
            break;
        case REP_EXALTED:
            out << "00ffff崇拜";
            break;
        default:
            out << "808080未知";
            break;
    }

    out << "|cffffffff";

    int32 base = ReputationMgr::Reputation_Cap + 1;
    for (int32 i = MAX_REPUTATION_RANK - 1; i >= rank; --i)
        base -= ReputationMgr::PointsInRank[i];

    out << " (" << (reputation - base) << "/" << ReputationMgr::PointsInRank[rank] << ")";
    return out.str();
}

bool TellReputationAction::Execute(Event event)
{
    std::string const param = event.getParam();
    if (param == "all")
    {
        ReputationMgr& repMgr = bot->GetReputationMgr();
        std::vector<std::string> lines;

        FactionStateList const& stateList = repMgr.GetStateList();
        lines.reserve(stateList.size());

        for (auto const& itr : stateList)
        {
            FactionState const& faction = itr.second;
            if (!(faction.Flags & FACTION_FLAG_VISIBLE))
                continue;

            if (faction.Flags & (FACTION_FLAG_HIDDEN | FACTION_FLAG_INVISIBLE_FORCED) &&
                !(faction.Flags & FACTION_FLAG_SPECIAL))
                continue;

            FactionEntry const* entry = sFactionStore.LookupEntry(faction.ID);
            if (!entry)
                continue;

            lines.push_back(BuildReputationLine(entry));
        }

        std::sort(lines.begin(), lines.end());

        botAI->TellMaster("=== 声望 ===");
        for (auto const& line : lines)
            botAI->TellMaster(line);

        return true;
    }

    Player* master = GetMaster();
    if (!master)
        return false;

    ObjectGuid selection = master->GetTarget();
    if (selection.IsEmpty())
        return false;

    Unit* unit = ObjectAccessor::GetUnit(*master, selection);
    if (!unit)
        return false;

    FactionTemplateEntry const* factionTemplate = unit->GetFactionTemplateEntry();

    FactionEntry const* entry = sFactionStore.LookupEntry(factionTemplate->faction);
    if (!entry)
        return false;

    botAI->TellMaster(BuildReputationLine(entry));

    return true;
}
