/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_QUICKPARTYHELPER_H
#define _PLAYERBOT_QUICKPARTYHELPER_H

#include "PlayerbotAI.h"

#include <set>
#include <string>
#include <unordered_map>
#include <vector>

class Group;
class Player;

class QuickPartyHelper
{
public:
    static std::vector<std::string> FormParty(Player* player, uint32 size);
    static std::vector<std::string> DisbandParty(Player* player);
    static std::vector<std::string> RandomGear(Player* invoker, Player* target);

private:
    static void GetRoleRequirements(uint32 partySize, std::unordered_map<BotRoles, uint32>& roles);
    static void CountExistingRoles(Player* player, Group* group, std::unordered_map<BotRoles, uint32>& roles);
    static BotRoles GetPlayerRole(Player* player);
    static bool CanClassFillRole(uint8 cls, BotRoles role);
    static bool BotMatchesRole(Player* bot, BotRoles role);
    static uint32 GetRoleSpecTab(uint8 cls, BotRoles role);
    static void ApplyRoleTalents(Player* bot, BotRoles role);
    static void PrepareBotForGroup(Player* bot, Player* player, BotRoles role);
    static bool AddBotToGroup(Player* player, Player* bot);
    static void CollectAvailableBots(Player* player, std::vector<Player*>& out, std::set<ObjectGuid> const& used);
    static Player* PickBotForRole(std::vector<Player*>& candidates, BotRoles role, std::set<ObjectGuid>& used);
    static void StripHeirloomItems(Player* player);
    static uint32 GearQualityForLevel(uint32 level);
};

#endif
