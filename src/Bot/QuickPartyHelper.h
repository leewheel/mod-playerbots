/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_QUICKPARTYHELPER_H
#define _PLAYERBOT_QUICKPARTYHELPER_H

#include "Define.h"
#include <string>
#include <vector>

class Player;

class QuickPartyHelper
{
public:
    static std::vector<std::string> RandomGear(Player* invoker, Player* target);

private:
    static void StripHeirloomItems(Player* player);
    static uint32 GearQualityForLevel(uint32 level);
};

#endif
