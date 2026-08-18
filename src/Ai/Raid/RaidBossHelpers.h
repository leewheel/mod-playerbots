/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAIDBOSSHELPERS_H
#define PLAYERBOTS_RAIDBOSSHELPERS_H

#include "Common.h"
#include "Position.h"
#include <string>
#include <vector>

class Action;
class Player;
class PlayerbotAI;
class Unit;

// Answers whether the bot can take one short step towards a destination, and where that step
// lands. It says nothing about the destination itself--only about the next hop. stepX/Y/Z are
// written on success and left untouched on failure
bool CanTakeStepTowards(
    Player* bot, float destinationX, float destinationY, float moveDist,
    float& stepX, float& stepY, float& stepZ);
bool MarkTargetWithIcon(Player* bot, Unit* target, uint8 iconId);
bool MarkTargetWithSkull(Player* bot, Unit* target);
bool MarkTargetWithSquare(Player* bot, Unit* target);
bool MarkTargetWithStar(Player* bot, Unit* target);
bool MarkTargetWithCircle(Player* bot, Unit* target);
bool MarkTargetWithDiamond(Player* bot, Unit* target);
bool MarkTargetWithTriangle(Player* bot, Unit* target);
bool MarkTargetWithCross(Player* bot, Unit* target);
bool MarkTargetWithMoon(Player* bot, Unit* target);
bool ClearTargetIcon(Player* bot, uint8 iconId);
void SetRtiTarget(PlayerbotAI* botAI, const std::string& rtiName, Unit* target);
// Fork note: keeps the extended signature (botAI + optional exclude) so the mechanic tracker
// role stays limited to DPS bots, matching the fork's TK/SWP/ZA behavior.
bool IsMechanicTrackerBot(PlayerbotAI* botAI, Player* bot, uint32 mapId, Player* exclude = nullptr);
// 2-param overload for brighton-chi raid strategies (TK/SWP/HFR/Mag etc.)
// Returns true if the bot is the first alive bot in the group on the given map
bool IsMechanicTrackerBot(Player* bot, uint32 mapId);
Player* GetGroupMainTank(PlayerbotAI* botAI, Player* bot);
Player* GetGroupAssistTank(PlayerbotAI* botAI, Player* bot, uint8 index);
Unit* GetFirstAliveUnitByEntry(PlayerbotAI* botAI, uint32 entry);
Player* GetNearestPlayerInRadius(Player* bot, float radius);
std::vector<Position> GetDynamicObjectPositions(Player* bot, float searchRadius, uint32 spellId);
bool IsDpsCooldownAction(Player* bot, Action* action);
bool IsTauntAction(Player* bot, Action* action);
bool IsAoeThreatAction(Player* bot, Action* action);

#endif
