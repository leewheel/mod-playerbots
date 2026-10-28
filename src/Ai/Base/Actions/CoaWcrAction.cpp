/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CoaWcrAction.h"

#include "CoaReborn.h"
#include "Event.h"
#include "PlayerbotFactory.h"
#include "Playerbots.h"
#include "RandomPlayerbotMgr.h"

#include <mutex>
#include <sstream>
#include <unordered_set>

bool CoaWcrAction::Execute(Event event)
{
    std::string const param = event.getParam();

    if (!IsRebornRealm())
    {
        // Said once a bot, so that an add-on asking every bot does not flood its player.
        static std::mutex lock;
        static std::unordered_set<ObjectGuid::LowType> told;
        std::lock_guard<std::mutex> guard(lock);
        if (told.insert(bot->GetGUID().GetCounter()).second)
            botAI->TellMaster("not a Warcraft Reborn realm");
        return true;
    }
    if (!IsRebornBot(bot))
    {
        botAI->TellMaster("not a Warcraft Reborn class");
        return true;
    }

    std::istringstream in(param);
    std::string word, number;
    in >> word >> number;
    if (word == "spec")
    {
        if (number.size() != 1 || number[0] < '0' || number[0] > '2')
        {
            botAI->TellMaster("Usage: wcr spec <0-2>");
            return true;
        }

        CoaRole const before = GetWcrRole(bot);
        std::string error;
        if (!SetWcrSpec(bot, uint8(number[0] - '0'), error))
        {
            botAI->TellMaster("WCR spec " + number + ": " + error);
            if (GetWcrSpec(bot) != uint8(number[0] - '0'))
                return true;
        }
        // Only a random bot's gear is the bot's to change: an alt keeps what its owner gave it.
        if (GetWcrRole(bot) != before && sRandomPlayerbotMgr.IsRandomBot(bot) && PlayerbotFactory::MayGearItself(bot))
            PlayerbotFactory(bot, bot->GetLevel()).InitEquipment(true);
        botAI->ResetStrategies();
    }

    for (std::string const& line : DescribeWcr(bot))
        botAI->TellMaster(line);
    return true;
}
