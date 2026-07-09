/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option) any later version.
 */

#include "VimgolActions.h"
#include "Playerbots.h"

// Creature entries
static constexpr uint32 NPC_VIMGOL_CIRCLE_BUNNY = 23040;
static constexpr uint32 SPELL_UNHOLY_GROWTH     = 40545;

// Radius to search for circle bunnies
static constexpr float CIRCLE_SEARCH_RADIUS = 80.0f;
// How close the bot needs to be to the fire ring position
static constexpr float FIRE_RING_ARRIVAL_DIST = 3.0f;

// Helper: Get the list of Circle Bunny creatures sorted by distance from center
static std::vector<Creature*> GetSortedCircleBunnies(Player* bot)
{
    std::list<Creature*> bunnyList;
    bot->GetCreatureListWithEntryInGrid(bunnyList, NPC_VIMGOL_CIRCLE_BUNNY, CIRCLE_SEARCH_RADIUS);
    if (bunnyList.empty())
        return {};

    // Sort by GUID to ensure consistent ordering across all bots
    std::vector<Creature*> bunnies(bunnyList.begin(), bunnyList.end());
    std::sort(bunnies.begin(), bunnies.end(),
              [](Creature* a, Creature* b)
              { return a->GetGUID().GetCounter() < b->GetGUID().GetCounter(); });

    return bunnies;
}

// Helper: Get this bot's index in the group (0-based)
static int8 GetBotGroupIndex(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return 0;

    int8 index = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member == bot)
            return index;
        index++;
    }
    return 0;
}

// Helper: Assign each group member to a fire ring (Circle Bunny) position
// Uses round-robin assignment based on group index
static Creature* GetAssignedFireRing(Player* bot)
{
    auto bunnies = GetSortedCircleBunnies(bot);
    if (bunnies.empty())
        return nullptr;

    int8 botIndex = GetBotGroupIndex(bot);
    uint8 bunnyIndex = static_cast<uint8>(botIndex) % bunnies.size();
    return bunnies[bunnyIndex];
}

bool VimgolMoveToFireRingAction::Execute(Event /*event*/)
{
    Creature* fireRing = GetAssignedFireRing(bot);
    if (!fireRing)
        return false;

    float dist = bot->GetExactDist2d(fireRing);
    if (dist <= FIRE_RING_ARRIVAL_DIST)
        return false;  // Already at the fire ring

    return MoveTo(fireRing, 0.0f, MovementPriority::MOVEMENT_FORCED);
}

bool VimgolInterruptGrowthAction::Execute(Event /*event*/)
{
    Creature* fireRing = GetAssignedFireRing(bot);
    if (!fireRing)
        return false;

    float dist = bot->GetExactDist2d(fireRing);
    if (dist <= FIRE_RING_ARRIVAL_DIST)
        return false;  // Already at the fire ring position

    // Move with forced priority to interrupt the growth spell quickly
    return MoveTo(fireRing, 0.0f, MovementPriority::MOVEMENT_FORCED);
}
