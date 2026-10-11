/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

// CoA bots in PvP. The generic arena tactics place a bot by its class, and every CoA bot is class 27: they all went
// to the melee spot. Here the role decides. Each behaviour has its own AiPlayerbot.CoaArena* / CoaPvp* / CoaDuel*
// switch, read on every use so that ".reload config" takes effect at once.

#include "CoaPvpTactics.h"

#include "AscensionSpecialization.h"
#include "Config.h"
#include "CoaSpecialization.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

namespace
{
    bool On(char const* key) { return sConfigMgr->GetOption<bool>(key, true); }

    bool IsCoaBot(Player* bot) { return bot && IsAscensionCustomClassId(bot->getClass()); }
}

uint32 CoaPvp::ArenaSpot(Player* bot)
{
    if (!IsCoaBot(bot) || !On("AiPlayerbot.CoaArenaRoleSpot"))
        return 0;
    if (PlayerbotAI::IsHeal(bot, true))
        return 3;
    if (PlayerbotAI::IsRanged(bot, true) && !PlayerbotAI::IsTank(bot, true))
        return 9;
    return 6;
}
