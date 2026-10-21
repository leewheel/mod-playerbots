/*
 * Preferred Wildcard draft picks.
 *
 * A Wildcard Hero (Darkmoon realm) drafts its abilities and talents from random rolls. A player rerolls what does not
 * fit and keeps the abilities a strong build is made of: this table holds those, for each of the five archetypes a bot
 * can play, with a weight, and the named builds of each one (the Season 10 Hero Archetypes and community Wildcard
 * builds), a Hero drafting toward one of them chosen by its GUID. It only steers the bot's own choices (its starting
 * abilities among many draws, and the rolls it spends a Scroll of Fortune on); the server still rolls everything. Entries are Character Advancement ids
 * (CharacterAdvancement.dbc, the ids the Wildcard tables load). Sources and reasons: WILDCARD-PICKS.md of the Wildcard
 * work notes.
 */

#ifndef PLAYERBOTS_COAWILDCARDPICKS_H
#define PLAYERBOTS_COAWILDCARDPICKS_H

#include "Define.h"

#include <vector>

enum class WildcardArchetype : uint8
{
    Tank,
    Heal,
    Melee,
    Ranged,
    Caster
};

// What the archetype thinks of an entry: above 0 a pick it builds around (up to 6, Auto Shot for a ranged Hero), below
// 0 one to reroll first, 0 when the table does not know it.
int32 WildcardPickWeight(uint32 entryId, WildcardArchetype archetype);

// The same, for a Hero drafting toward one build of its archetype: the abilities and talents that build is made of
// weigh more (the Hero Archetypes of the Season 10 client and community Wildcard builds, WILDCARD-PICKS.md).
int32 WildcardPickWeight(uint32 entryId, WildcardArchetype archetype, uint8 build);

constexpr uint8 NO_WILDCARD_BUILD = 0xFF;

// The build of `archetype` a Hero drafts toward: the one its kept entries (its starting abilities, for good from level
// 10) already hold the most of, else the one its GUID spreads it on, so that a realm's Heroes play all of them.
uint8 WildcardBuildFor(uint32 guidCounter, WildcardArchetype archetype, std::vector<uint32> const& kept);

char const* WildcardBuildName(uint8 build);

char const* WildcardArchetypeName(WildcardArchetype archetype);

#endif
