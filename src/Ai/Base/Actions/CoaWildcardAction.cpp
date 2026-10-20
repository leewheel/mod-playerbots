/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CoaWildcardAction.h"

#include "CoaWildcard.h"
#include "Event.h"
#include "Playerbots.h"
#include "RandomPlayerbotMgr.h"

#include <mutex>
#include <sstream>
#include <unordered_set>

bool CoaWildcardAction::Execute(Event event)
{
    if (!IsWildcardRealm() || !IsWildcardHeroBot(bot))
    {
        // Said once a bot, so that an add-on asking every bot does not flood its player.
        static std::mutex lock;
        static std::unordered_set<ObjectGuid::LowType> told;
        std::lock_guard<std::mutex> guard(lock);
        if (told.insert(bot->GetGUID().GetCounter()).second)
            botAI->TellMaster(IsWildcardRealm() ? "not a Wildcard Hero" : "not a Wildcard realm");
        return true;
    }

    std::istringstream in(event.getParam());
    std::string word, value;
    in >> word >> value;

    if (word == "redraft")
    {
        if (!sRandomPlayerbotMgr.IsRandomBot(bot))
        {
            botAI->TellMaster("Only a random bot rolls its build again: an alt keeps the rolls of its owner.");
            return true;
        }
        if (bot->IsInCombat())
        {
            botAI->TellMaster("Not in combat.");
            return true;
        }
        RedraftWildcardBot(bot);
    }
    else if (word == "role")
    {
        CoaRole role;
        if (value == "tank")
            role = CoaRole::Tank;
        else if (value == "heal" || value == "healer")
            role = CoaRole::Heal;
        else if (value == "dps" || value == "damage")
            role = CoaRole::Dps;
        else
        {
            botAI->TellMaster("Usage: wildcard role <tank|heal|dps>");
            return true;
        }

        std::string error;
        if (!SetWildcardRole(bot, role, true, error))
        {
            botAI->TellMaster(std::string("WILDCARD ") + WildcardRoleName(role) + ": " + error);
            return true;
        }
        botAI->ResetStrategies();
    }
    else if (!word.empty() && word != "?")
    {
        botAI->TellMaster("Usage: wildcard [role <tank|heal|dps>|redraft]");
        return true;
    }

    for (std::string const& line : DescribeWildcard(bot))
        botAI->TellMaster(line);
    return true;
}
