/*
 * Preferred Wildcard draft picks.
 *
 * A Wildcard Hero (Darkmoon realm) drafts its abilities and talents from random rolls. A player rerolls what does not
 * fit and keeps the abilities a strong build is made of: this table holds those, for each of the five archetypes a bot
 * can play, with a weight. It only steers the bot's own choices (its starting abilities among many draws, and the
 * rolls it spends a Scroll of Fortune on); the server still rolls everything. Entries are Character Advancement ids
 * (CharacterAdvancement.dbc, the ids the Wildcard tables load). Sources and reasons: WILDCARD-PICKS.md of the Wildcard
 * work notes.
 */

#ifndef PLAYERBOTS_COAWILDCARDPICKS_H
#define PLAYERBOTS_COAWILDCARDPICKS_H

#include "Define.h"

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

char const* WildcardArchetypeName(WildcardArchetype archetype);

#endif
