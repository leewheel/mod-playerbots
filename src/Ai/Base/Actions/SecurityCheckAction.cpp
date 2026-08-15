/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SecurityCheckAction.h"
#include "Event.h"
#include "Playerbots.h"

bool SecurityCheckAction::isUseful()
{
    return RandomPlayerbotMgr::instance().IsRandomBot(bot)
        && botAI->GetMaster()
        && !botAI->GetMaster()->CanBeGameMaster()
        && !GET_PLAYERBOT_AI(botAI->GetMaster());
}

bool SecurityCheckAction::Execute(Event /*event*/)
{
    if (Group* group = bot->GetGroup())
    {
        LootMethod method = group->GetLootMethod();
        ItemQualities threshold = group->GetLootThreshold();
        if (method == MASTER_LOOT || method == FREE_FOR_ALL || threshold > ITEM_QUALITY_UNCOMMON)
        {
            if ((botAI->GetGroupLeader()->GetSession()->GetSecurity() == SEC_PLAYER) &&
                (!bot->GetGuildId() || bot->GetGuildId() != botAI->GetGroupLeader()->GetGuildId()))
            {
                botAI->TellError("只有加入你的公会，我才会使用这种拾取方式 :/");
                botAI->ChangeStrategy("+passive,+stay", BOT_STATE_NON_COMBAT);
                botAI->ChangeStrategy("+passive,+stay", BOT_STATE_COMBAT);
            }
            return true;
        }
    }

    return false;
}
