/*
 * Warcraft Reborn bots.
 *
 * On a Warcraft Reborn realm (CoA.ClassModel = "wcr") characters play the original classes with the "classic+"
 * Reborn copies of their spells (stock id + 1100000), taught by the class trainers the core rebuilds for them, and
 * spend their talents through Character Advancement. This side creates random bots of those classes, teaches them
 * the Reborn spells a trainer would, gives them the build of one of their three trees (playerbots_coa_wcr_builds)
 * and hands the upstream class AI the role of that tree. Everything here is inert on any other realm.
 */

#ifndef PLAYERBOTS_COAREBORN_H
#define PLAYERBOTS_COAREBORN_H

#include "CoaSpecialization.h"
#include "ObjectGuid.h"

#include <string>
#include <vector>

class Player;
class Unit;

namespace Trainer
{
class Trainer;
}

// Whether this realm plays Warcraft Reborn (CoA.ClassModel = "wcr").
bool IsRebornRealm();

// One of the original classes (1 to 11, the Hero excepted).
bool IsRebornClass(uint8 classId);

// A character of an original class on a Warcraft Reborn realm.
bool IsRebornBot(Player const* player);

// The Reborn copy of a stock spell on a Warcraft Reborn realm when it exists, else the spell itself.
uint32 RebornSpellId(uint32 spellId);

// The stock spell or its Reborn copy.
bool KnowsSpellOrTwin(Player const* player, uint32 spellId);
bool HasAuraOrTwin(Unit const* unit, uint32 spellId, ObjectGuid caster = ObjectGuid::Empty);

// The class trainer a Reborn character is served in place of this one (the core's own rows), or `stock`.
Trainer::Trainer const* RebornTrainerFor(Trainer::Trainer const* stock, Player const* player);

// What the core grants a Reborn character at login and level up: class skills, starting and automatic spells, the
// spells granted with others.
void LearnRebornBasics(Player* bot);

// Forgets the stock spells whose Reborn copy the bot knows. Returns how many went.
uint32 ForgetStockTwins(Player* bot);

// A Feral Druid fighting in Cat Form or in Bear Form: from its talents (Cat Form known, Thick Hide), or for a Reborn
// bot from the role of its tree.
bool PlaysCatDruid(Player const* bot, uint32 catForm, uint32 thickHide);
bool PlaysBearDruid(Player const* bot, uint32 thickHide);

enum class WcrRole : uint8
{
    Tank,
    Heal,
    MeleeDps,
    RangedDps
};

// The tree (0 to 2, talent tab order) a Reborn bot plays: the one stored for it, else the one its build matches,
// else one picked for it.
uint8 GetWcrSpec(Player const* bot);
WcrRole GetWcrSpecRole(uint8 classId, uint8 spec);
std::string GetWcrSpecName(uint8 classId, uint8 spec);
char const* WcrRoleName(WcrRole role);
CoaRole ToCoaRole(WcrRole role);

CoaRole GetWcrRole(Player const* bot);
bool IsWcrRanged(Player const* bot);

// Whether one of the class's trees plays the role.
bool WcrClassCanPlay(uint8 classId, CoaRole role);

// Gives the bot the Character Advancement build of its tree at its level (AscensionFreepick::ApplyGuide), the
// unlearn costs paid for it, then the Mystic Enchants of its tree (playerbots_coa_wcr_enchants, AiPlayerbot.CoaWcrMysticEnchants).
// Returns false when it has no build or the core refused it (logged once).
bool ApplyWcrBuild(Player* bot);

// ApplyWcrBuild once a session for a random Reborn bot, out of combat, for bots built before their table was filled.
void EnsureWcrBuild(Player* bot);

// Stores the tree and applies its build. False with the reason in `error`.
bool SetWcrSpec(Player* bot, uint8 spec, std::string& error);

// Moves the bot to a tree of that role. False when none of its class plays it.
bool SwitchWcrRole(Player* bot, CoaRole role);

// "WCR spec 1: Protection (tank)" and the three trees, the active one marked with ">".
std::vector<std::string> DescribeWcr(Player const* bot);

// Reads playerbots_coa_wcr_builds again. Returns the number of rows.
uint32 LoadWcrBuilds();

#endif
