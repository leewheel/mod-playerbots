/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CoaWildcard.h"

#include "AscensionCoATalentState.h"
#include "AscensionSpecialization.h"
#include "AscensionWildcard.h"
#include "CoaWildcardPicks.h"
#include "Config.h"
#include "Log.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotFactory.h"
#include "Playerbots.h"
#include "RandomPlayerbotMgr.h"
#include "SharedDefines.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "SpellMgr.h"

#include <algorithm>
#include <array>
#include <mutex>
#include <optional>
#include <sstream>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

namespace
{
// The five stat paths: Character Advancement entries and the spells they teach.
constexpr uint32 PATH_STRENGTH = 1149;
constexpr uint32 PATH_AGILITY = 1150;
constexpr uint32 PATH_INTELLECT = 1151;
constexpr uint32 PATH_HEALING = 1152;
constexpr uint32 PATH_DUALITY = 18149;
constexpr uint32 SPELL_PATH_STRENGTH = 84864;
constexpr uint32 SPELL_PATH_AGILITY = 84865;
constexpr uint32 SPELL_PATH_INTELLECT = 84866;
constexpr uint32 SPELL_PATH_HEALING = 84867;
// The glow of a level up, shown to everyone around.
constexpr uint32 SPELL_LEVEL_UP_VISUAL = 24298;
// The entry that teaches Auto Shot: without it a Hero cannot shoot its bow or gun.
constexpr uint32 ENTRY_AUTO_SHOT = 40309;

// A bot tries this many sets of starting abilities and keeps the best, as a player rerolls until the four fit.
constexpr uint32 STARTER_DRAWS = 1024;
// Scrolls of Fortune a bot spends at most per draft on rolls it cannot use.
constexpr uint32 MISFIT_REROLLS_PER_DRAFT = 8;

// Spell tags of SpellTagTypes.dbc.
constexpr uint32 TAG_ROLE_TANK = 122;
constexpr uint32 FIRST_CLASS_TAG = 58;
constexpr uint32 LAST_CLASS_TAG = 67;
constexpr uint32 FIRST_SPEC_TAG = 78;
constexpr uint32 LAST_SPEC_TAG = 107;
constexpr uint32 FIRST_CROWD_CONTROL_TAG = 23;
constexpr uint32 LAST_CROWD_CONTROL_TAG = 48;
constexpr uint32 FIRST_DISPEL_TAG = 69;
constexpr uint32 LAST_DISPEL_TAG = 76;
// Mobility, raid, party and personal buffs, immunity, resurrection, defensives.
constexpr std::array<uint32, 8> SUPPORT_TAGS = { 2, 3, 55, 77, 120, 121, 159, 160 };

// Bot AI runs in the map update threads, so what a draft decided is kept here under a lock rather than in
// RandomPlayerbotMgr's value cache, which only the world thread may touch: whether the bot shoots, whether
// a draft in combat left its stat path for after the fight, and the role a player or a recruit asked it to play.
std::mutex BuildStateLock;
std::unordered_map<ObjectGuid::LowType, bool> Shooters;
std::unordered_set<ObjectGuid::LowType> PathsDue;
std::unordered_map<ObjectGuid::LowType, CoaRole> ChosenRoles;

AscensionWildcard::Entry const* EntryOf(uint32 entryId)
{
    static std::unordered_map<uint32, AscensionWildcard::Entry> const byEntry = []
    {
        std::unordered_map<uint32, AscensionWildcard::Entry> entries;
        for (AscensionWildcard::Entry const& entry : AscensionWildcard::LoadedTables().Entries)
            entries.emplace(entry.EntryId, entry);
        return entries;
    }();

    auto const found = byEntry.find(entryId);
    return found == byEntry.end() ? nullptr : &found->second;
}

std::vector<uint32> const* RankSpellsOf(uint32 entryId)
{
    AscensionWildcard::Entry const* entry = EntryOf(entryId);
    return entry ? &entry->RankSpells : nullptr;
}

std::vector<uint32> const& TagsOf(uint32 spellId)
{
    static std::vector<uint32> const none;
    auto const& tags = AscensionWildcard::LoadedTables().SpellTags;
    auto const found = tags.find(spellId);
    return found == tags.end() ? none : found->second;
}

bool Heals(SpellInfo const* info)
{
    for (SpellEffectInfo const& effect : info->Effects)
        if (effect.Effect == SPELL_EFFECT_HEAL ||
            (effect.IsAura() && (effect.ApplyAuraName == SPELL_AURA_PERIODIC_HEAL ||
                                 effect.ApplyAuraName == SPELL_AURA_SCHOOL_ABSORB)))
            return true;
    return false;
}

bool Taunts(SpellInfo const* info)
{
    for (SpellEffectInfo const& effect : info->Effects)
        if (effect.Effect == SPELL_EFFECT_ATTACK_ME ||
            (effect.IsAura() && effect.ApplyAuraName == SPELL_AURA_MOD_TAUNT))
            return true;
    return false;
}

// Damage of its own or of the spells it triggers (Arcane Missiles).
bool Damages(SpellInfo const* info, uint8 depth = 0)
{
    for (SpellEffectInfo const& effect : info->Effects)
    {
        if (effect.Effect == SPELL_EFFECT_SCHOOL_DAMAGE || effect.Effect == SPELL_EFFECT_WEAPON_DAMAGE ||
            effect.Effect == SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL || effect.Effect == SPELL_EFFECT_NORMALIZED_WEAPON_DMG ||
            effect.Effect == SPELL_EFFECT_WEAPON_PERCENT_DAMAGE ||
            (effect.IsAura() && (effect.ApplyAuraName == SPELL_AURA_PERIODIC_DAMAGE ||
                                 effect.ApplyAuraName == SPELL_AURA_PERIODIC_LEECH)))
            return true;

        bool const triggers = effect.Effect == SPELL_EFFECT_TRIGGER_SPELL ||
            (effect.IsAura() && effect.ApplyAuraName == SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        if (triggers && depth < 2)
            if (SpellInfo const* triggered = sSpellMgr->GetSpellInfo(effect.TriggerSpell))
                if (Damages(triggered, depth + 1))
                    return true;
    }
    return false;
}

std::vector<uint32> FormsGiven(SpellInfo const* info)
{
    std::vector<uint32> forms;
    for (SpellEffectInfo const& effect : info->Effects)
        if (effect.IsAura() && effect.ApplyAuraName == SPELL_AURA_MOD_SHAPESHIFT && effect.MiscValue > 0)
            forms.push_back(uint32(effect.MiscValue));
    return forms;
}

bool CastableIn(SpellInfo const* info, uint32 form)
{
    return info->CheckShapeshift(form) == SPELL_CAST_OK;
}

// Threat for a tank: a taunt, a threat aura, or a form the game marks as a tank's (Bear Form, Defensive Stance,
// Righteous Fury, Frost Presence).
bool Threatens(SpellInfo const* info)
{
    if (Taunts(info))
        return true;
    for (SpellEffectInfo const& effect : info->Effects)
        if (effect.IsAura() && effect.ApplyAuraName == SPELL_AURA_MOD_THREAT)
            return true;
    std::vector<uint32> const& tags = TagsOf(info->Id);
    return !FormsGiven(info).empty() && std::find(tags.begin(), tags.end(), TAG_ROLE_TANK) != tags.end();
}

bool Supports(SpellInfo const* info)
{
    for (uint32 tag : TagsOf(info->Id))
        if ((tag >= FIRST_CROWD_CONTROL_TAG && tag <= LAST_CROWD_CONTROL_TAG) ||
            (tag >= FIRST_DISPEL_TAG && tag <= LAST_DISPEL_TAG) ||
            std::find(SUPPORT_TAGS.begin(), SUPPORT_TAGS.end(), tag) != SUPPORT_TAGS.end())
            return true;
    return false;
}

// A spell that needs a form comes with it, and a form comes with something to cast in it: no cat attack
// without Cat Form, no Cat Form without a cat attack.
bool FormsFit(std::vector<SpellInfo const*> const& spells)
{
    std::vector<uint32> forms;
    for (SpellInfo const* info : spells)
        for (uint32 form : FormsGiven(info))
            forms.push_back(form);

    for (SpellInfo const* info : spells)
        if (!CastableIn(info, 0) &&
            std::none_of(forms.begin(), forms.end(), [info](uint32 form) { return CastableIn(info, form); }))
            return false;

    for (SpellInfo const* former : spells)
        for (uint32 form : FormsGiven(former))
            if (std::none_of(spells.begin(), spells.end(), [former, form](SpellInfo const* other)
                    { return other != former && FormsGiven(other).empty() && CastableIn(other, form); }))
                return false;
    return true;
}

// Two of one specialization, or at least of one class.
uint32 Grouping(std::vector<SpellInfo const*> const& spells)
{
    std::unordered_map<uint32, uint32> specs;
    std::unordered_map<uint32, uint32> classes;
    for (SpellInfo const* info : spells)
        for (uint32 tag : TagsOf(info->Id))
        {
            if (tag >= FIRST_SPEC_TAG && tag <= LAST_SPEC_TAG)
                ++specs[tag];
            else if (tag >= FIRST_CLASS_TAG && tag <= LAST_CLASS_TAG)
                ++classes[tag];
        }

    auto const paired = [](std::unordered_map<uint32, uint32> const& counts)
    { return std::any_of(counts.begin(), counts.end(), [](auto const& count) { return count.second >= 2; }); };
    return paired(specs) ? 2 : paired(classes) ? 1 : 0;
}

// The fighting style a damage dealer drafts for, kept by its GUID: weapon strikes, shots or spells, a third each,
// so that a realm's Heroes are not nearly all melee Agility (the draws hold more strikes than anything else).
uint8 DpsStyleOf(Player const* bot)
{
    return uint8(((uint64(bot->GetGUID().GetCounter()) * 2654435761u) >> 12) % 3);
}

WildcardArchetype ArchetypeOf(CoaRole role, uint8 dpsStyle)
{
    switch (role)
    {
        case CoaRole::Tank:
            return WildcardArchetype::Tank;
        case CoaRole::Heal:
            return WildcardArchetype::Heal;
        default:
            return dpsStyle == 0 ? WildcardArchetype::Melee :
                   dpsStyle == 1 ? WildcardArchetype::Ranged : WildcardArchetype::Caster;
    }
}

// A shot takes the range of a ranged weapon; a strike hits with the weapon in melee; a spell is magic. Thunder Clap
// and its Smash versions, of the ranged damage class but cast around the Hero, are none of them.
bool Shot(SpellInfo const* info)
{
    return info->RangeEntry && (info->RangeEntry->Flags & SPELL_RANGE_RANGED);
}

bool Strike(SpellInfo const* info)
{
    return !Shot(info) && info->DmgClass == SPELL_DAMAGE_CLASS_MELEE;
}

bool MagicAttack(SpellInfo const* info)
{
    return !Shot(info) && info->DmgClass == SPELL_DAMAGE_CLASS_MAGIC;
}

SpellInfo const* FirstRankOf(uint32 entryId)
{
    std::vector<uint32> const* ranks = entryId ? RankSpellsOf(entryId) : nullptr;
    return ranks && !ranks->empty() ? sSpellMgr->GetSpellInfo(ranks->front()) : nullptr;
}

// What an archetype makes of an entry: its weight in the picks table (with the build the Hero drafts toward), plus or
// minus for the way the entry attacks (a caster has no use for a weapon strike, a ranged Hero for a melee one). Below
// -1 it is a roll to spend a Scroll of Fortune on.
int32 ArchetypeScore(uint32 entryId, WildcardArchetype archetype, uint8 build = NO_WILDCARD_BUILD)
{
    int32 score = WildcardPickWeight(entryId, archetype, build);
    SpellInfo const* info = FirstRankOf(entryId);
    if (!info || info->IsPassive())
        return score;
    if (Heals(info))
        return score + (archetype == WildcardArchetype::Heal ? 1 : 0);
    if (archetype == WildcardArchetype::Tank && Threatens(info))
        return score + 1;
    if (!Damages(info))
        return score;

    bool const shot = Shot(info);
    bool const strike = Strike(info);
    bool const spell = MagicAttack(info);
    switch (archetype)
    {
        case WildcardArchetype::Melee:
            return score + (strike ? 1 : shot ? -2 : spell ? -1 : 0);
        case WildcardArchetype::Ranged:
            return score + (shot ? 1 : strike ? -2 : spell ? -1 : 0);
        case WildcardArchetype::Caster:
            return score + (spell ? 1 : strike || shot ? -2 : 0);
        case WildcardArchetype::Tank:
            return score + (shot ? -2 : 0);
        default:
            return score + (strike || shot ? -2 : 0);
    }
}

// How well four starting abilities suit an archetype, best first: forms that work; what it cannot do without (tank:
// threat and a damage spell; healer: a heal and a damage spell to quest with; melee: two strikes; ranged: Auto Shot
// and a shot; caster: two damage spells); the picks table and the way they attack; what the role needs (damage: two
// of a group and a support ability); more of the role's own abilities; then their synergy with each other.
using StarterFit = std::tuple<bool, bool, int32, uint32, uint32, uint32>;

StarterFit FitOf(std::vector<AscensionWildcard::Slot> const& starters, WildcardArchetype archetype,
                 uint8 build = NO_WILDCARD_BUILD)
{
    std::vector<SpellInfo const*> spells;
    std::vector<uint32> entries;
    for (AscensionWildcard::Slot const& slot : starters)
        if (SpellInfo const* info = FirstRankOf(slot.EntryId))
        {
            spells.push_back(info);
            entries.push_back(slot.EntryId);
        }

    uint32 heals = 0;
    uint32 damage = 0;
    uint32 threat = 0;
    uint32 taunts = 0;
    uint32 support = 0;
    uint32 strikes = 0;
    uint32 shots = 0;
    uint32 magic = 0;
    // A damaging ability without a cooldown: without one, a damage dealer stands idle once its others recharge.
    uint32 fillers = 0;
    for (SpellInfo const* info : spells)
    {
        bool const heal = Heals(info);
        bool const hurts = Damages(info);
        heals += heal ? 1 : 0;
        damage += hurts ? 1 : 0;
        threat += Threatens(info) ? 1 : 0;
        taunts += Taunts(info) ? 1 : 0;
        support += Supports(info) ? 1 : 0;
        if (hurts && !heal)
        {
            strikes += Strike(info) ? 1 : 0;
            shots += Shot(info) ? 1 : 0;
            magic += MagicAttack(info) ? 1 : 0;
            fillers += !info->RecoveryTime && !info->CategoryRecoveryTime ? 1 : 0;
        }
    }
    bool const autoShot = std::find(entries.begin(), entries.end(), ENTRY_AUTO_SHOT) != entries.end();

    bool essential = false;
    uint32 needs = 0;
    uint32 depth = 0;
    switch (archetype)
    {
        case WildcardArchetype::Heal:
            essential = heals && damage;
            needs = (heals ? 1 : 0) + (damage ? 1 : 0);
            depth = heals;
            break;
        case WildcardArchetype::Tank:
            // A tank without a taunt never takes the enemy back from a damage dealer.
            essential = taunts && damage;
            needs = (threat ? 1 : 0) + (damage ? 1 : 0);
            depth = threat;
            break;
        default:
            essential = archetype == WildcardArchetype::Melee ? strikes >= 2 && fillers >= 1 :
                        archetype == WildcardArchetype::Ranged ? autoShot && shots >= 1 : magic >= 2 && fillers >= 1;
            needs = Grouping(spells) + (support ? 1 : 0);
            depth = damage;
            break;
    }

    int32 picks = 0;
    uint32 synergy = 0;
    for (std::size_t index = 0; index < entries.size(); ++index)
    {
        picks += ArchetypeScore(entries[index], archetype, build);
        std::vector<uint32> others = entries;
        others.erase(others.begin() + index);
        synergy += AscensionWildcard::SynergyScore(AscensionWildcard::LoadedTables(), entries[index], others);
    }
    return { FormsFit(spells), essential, picks, needs, depth, synergy };
}

std::size_t PickStarters(std::vector<std::vector<AscensionWildcard::Slot>> const& candidates, WildcardArchetype archetype,
                         uint8 build)
{
    std::size_t best = 0;
    StarterFit bestFit{};
    for (std::size_t index = 0; index < candidates.size(); ++index)
        if (StarterFit const fit = FitOf(candidates[index], archetype, build); index == 0 || fit > bestFit)
        {
            best = index;
            bestFit = fit;
        }
    return best;
}

// The role a bot drafts for: the one it was asked to play (talents spec, wildcard role, a recruit), else the one its
// stat path already gives it, so that a bot made a tank stays one after a restart; else one bot in five tanks and
// one in five heals, as a dungeon group wants, the GUID keeping the choice across drafts.
CoaRole TargetRoleOf(Player const* bot)
{
    {
        std::lock_guard<std::mutex> lock(BuildStateLock);
        if (auto const found = ChosenRoles.find(bot->GetGUID().GetCounter()); found != ChosenRoles.end())
            return found->second;
    }
    if (AscensionWildcard::PrimaryStat(bot))
        return GetWildcardRole(bot);
    switch (bot->GetGUID().GetCounter() % 5)
    {
        case 0:
            return CoaRole::Tank;
        case 1:
            return CoaRole::Heal;
        default:
            return CoaRole::Dps;
    }
}

// Rolls of a higher level than the bot has: it was rebuilt at a lower level.
bool BuildAboveLevel(Player const* bot)
{
    for (AscensionWildcard::Slot const& slot : AscensionWildcard::Slots(bot))
        if (AscensionWildcard::Entry const* entry = slot.EntryId ? EntryOf(slot.EntryId) : nullptr;
            entry && entry->MinLevel > bot->GetLevel())
            return true;
    return false;
}

// The starting abilities a bot keeps, for good from level 10.
std::vector<uint32> StarterEntriesOf(Player const* bot)
{
    std::vector<uint32> entries;
    std::vector<AscensionWildcard::Slot> const slots = AscensionWildcard::Slots(bot);
    for (std::size_t index = 0; index < slots.size() && index < AscensionWildcard::STARTING_ABILITY_COUNT; ++index)
        if (slots[index].EntryId)
            entries.push_back(slots[index].EntryId);
    return entries;
}

// The archetype a bot drafts for: its role's, and for a damage dealer the style its GUID gives it, unless its starting
// abilities, kept for good from level 10, were drafted for another style (a build made before the styles): it then
// keeps to the style they play best rather than reroll everything else away from them.
WildcardArchetype DraftArchetypeOf(Player const* bot, CoaRole role)
{
    WildcardArchetype const target = ArchetypeOf(role, DpsStyleOf(bot));
    if (role != CoaRole::Dps)
        return target;

    std::vector<AscensionWildcard::Slot> starters = AscensionWildcard::Slots(bot);
    if (starters.size() < AscensionWildcard::STARTING_ABILITY_COUNT ||
        std::none_of(starters.begin(), starters.end(), [](AscensionWildcard::Slot const& slot) { return slot.EntryId; }))
        return target;
    starters.resize(AscensionWildcard::STARTING_ABILITY_COUNT);
    if (std::get<1>(FitOf(starters, target)))
        return target;

    WildcardArchetype best = target;
    StarterFit bestFit = FitOf(starters, target);
    for (WildcardArchetype style : { WildcardArchetype::Melee, WildcardArchetype::Ranged, WildcardArchetype::Caster })
        if (StarterFit const fit = FitOf(starters, style); fit > bestFit)
        {
            best = style;
            bestFit = fit;
        }
    return best;
}

// The rolls a bot would spend a Scroll of Fortune on, first to last, as a player rerolls what does not fit: an ability
// it cannot cast for want of its form, then one its fighting logic never casts, then the rolls its archetype has no
// use for, worst first (a weapon strike for a caster, a shot for a melee Hero, a talent left with nothing to improve).
// A pick of its archetype stays (Auto Shot, which the fighting logic does not cast itself), and so do the starting
// abilities, picked with care.
std::vector<uint32> MisfitsOf(Player const* bot, WildcardArchetype archetype, uint8 build)
{
    std::vector<AscensionWildcard::Slot> const slots = AscensionWildcard::Slots(bot);
    std::vector<SpellInfo const*> spells(slots.size(), nullptr);
    std::vector<uint32> forms;
    std::vector<uint32> entries;
    for (std::size_t index = 0; index < slots.size(); ++index)
    {
        AscensionWildcard::Slot const& slot = slots[index];
        std::vector<uint32> const* ranks = slot.EntryId ? RankSpellsOf(slot.EntryId) : nullptr;
        if (!ranks || slot.Rank < 1 || slot.Rank > ranks->size())
            continue;
        entries.push_back(slot.EntryId);
        spells[index] = sSpellMgr->GetSpellInfo((*ranks)[slot.Rank - 1]);
        if (spells[index])
            for (uint32 form : FormsGiven(spells[index]))
                forms.push_back(form);
    }

    std::vector<uint32> formless;
    std::vector<uint32> unused;
    std::vector<std::pair<int32, uint32>> offArchetype;
    for (std::size_t index = AscensionWildcard::STARTING_ABILITY_COUNT; index < slots.size(); ++index)
    {
        SpellInfo const* info = spells[index];
        uint32 const entry = slots[index].EntryId;
        if (!info || slots[index].Locked)
            continue;

        if (slots[index].Talent)
        {
            int32 score = WildcardPickWeight(entry, archetype, build);
            std::vector<uint32> others;
            for (uint32 other : entries)
                if (other != entry)
                    others.push_back(other);
            if (score <= 0 && !AscensionWildcard::SynergyScore(AscensionWildcard::LoadedTables(), entry, others))
                score -= 2;
            if (score <= -2)
                offArchetype.emplace_back(score, entry);
            continue;
        }
        if (info->IsPassive())
            continue;
        if (!CastableIn(info, 0) &&
            std::none_of(forms.begin(), forms.end(), [info](uint32 form) { return CastableIn(info, form); }))
            formless.push_back(entry);
        // The archetype's own weight, not the build's: a build does not keep an ability the fighting logic never casts.
        else if (!CoaBotCasts(info) && WildcardPickWeight(entry, archetype) <= 0)
            unused.push_back(entry);
        else if (int32 const score = ArchetypeScore(entry, archetype, build); score <= -2)
            offArchetype.emplace_back(score, entry);
    }

    std::stable_sort(offArchetype.begin(), offArchetype.end(),
                     [](auto const& left, auto const& right) { return left.first < right.first; });
    std::vector<uint32> misfits = formless;
    misfits.insert(misfits.end(), unused.begin(), unused.end());
    for (auto const& misfit : offArchetype)
        misfits.push_back(misfit.second);
    return misfits;
}

// The higher ranks of its rolled abilities that its level allows, as a player trains them at any class trainer.
void TrainRanks(Player* bot)
{
    for (AscensionWildcard::Slot const& slot : AscensionWildcard::Slots(bot))
    {
        std::vector<uint32> const* ranks = slot.EntryId && !slot.Talent ? RankSpellsOf(slot.EntryId) : nullptr;
        if (!ranks || slot.Rank < 1 || slot.Rank > ranks->size())
            continue;
        // A Hero trains the higher ranks of the client's SpellRank ladder at its trainer: they are not spell chains
        // of the server, so the bot learns them here, up to its level.
        auto const& ladders = AscensionWildcard::LoadedTables().RankLadders;
        if (auto const ladder = ladders.find(ranks->front()); ladder != ladders.end())
        {
            for (uint32 rankSpellId : ladder->second)
            {
                SpellInfo const* info = rankSpellId ? sSpellMgr->GetSpellInfo(rankSpellId) : nullptr;
                if (!info || info->SpellLevel > bot->GetLevel())
                    continue;
                if (!bot->HasSpell(rankSpellId))
                    bot->learnSpell(rankSpellId);
            }
            continue;
        }
        for (uint32 next = sSpellMgr->GetNextSpellInChain((*ranks)[slot.Rank - 1]); next;
             next = sSpellMgr->GetNextSpellInChain(next))
        {
            SpellInfo const* info = sSpellMgr->GetSpellInfo(next);
            if (!info || info->SpellLevel > bot->GetLevel())
                break;
            if (!bot->HasSpell(next))
                bot->learnSpell(next);
        }
    }
}

void RerollMisfits(Player* bot, WildcardArchetype archetype, uint8 build)
{
    AscensionWildcard::ClaimLevelingScrolls(bot);
    for (uint32 reroll = 0; reroll < MISFIT_REROLLS_PER_DRAFT; ++reroll)
    {
        // A roll the server will not take back (its essence is spent on what came after it) leaves the next one to try.
        std::vector<uint32> const misfits = MisfitsOf(bot, archetype, build);
        auto const rerolled = std::find_if(misfits.begin(), misfits.end(),
            [bot](uint32 misfit) { return AscensionWildcard::UnlearnForReroll(bot, misfit); });
        if (rerolled == misfits.end())
            return;
        LOG_INFO("playerbots.coa", "{} rerolls Wildcard entry {} with a Scroll of Fortune ({} {} build)",
                 bot->GetName(), *rerolled, WildcardBuildName(build), WildcardArchetypeName(archetype));
        AscensionWildcard::DraftBuild(bot);
    }
}

// What the rolled abilities of a build do: strike, shoot or cast.
struct BuildShape
{
    uint32 strikes = 0;
    uint32 shots = 0;
    uint32 spells = 0;
};

BuildShape ShapeOf(Player const* bot)
{
    BuildShape shape;
    for (uint32 spellId : WildcardBuildSpells(bot))
    {
        SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId);
        if (!info || info->IsPassive() || Heals(info))
            continue;

        if (info->DmgClass == SPELL_DAMAGE_CLASS_MELEE)
            ++shape.strikes;
        else if (info->DmgClass == SPELL_DAMAGE_CLASS_RANGED)
            ++shape.shots;
        else if (info->DmgClass == SPELL_DAMAGE_CLASS_MAGIC)
            ++shape.spells;
    }
    return shape;
}

// The path of the bot's role: Healing for a healer, Strength for a tank; a damage dealer takes Agility for a
// weapon build, Intellect for a caster and Duality for a build that does both. A bot that drafted for weapons or for
// spells takes their path as long as its build holds at least as many of them as of the others.
uint32 PathFor(CoaRole role, BuildShape const& shape, std::optional<WildcardArchetype> archetype = std::nullopt)
{
    if (role == CoaRole::Heal)
        return PATH_HEALING;
    if (role == CoaRole::Tank)
        return PATH_STRENGTH;

    uint32 const weapon = shape.strikes + shape.shots;
    if (archetype == WildcardArchetype::Caster && shape.spells && shape.spells >= weapon)
        return PATH_INTELLECT;
    if ((archetype == WildcardArchetype::Melee || archetype == WildcardArchetype::Ranged) && weapon &&
        weapon >= shape.spells)
        return PATH_AGILITY;
    if (weapon > shape.spells + 1)
        return PATH_AGILITY;
    if (shape.spells > weapon + 1)
        return PATH_INTELLECT;
    return PATH_DUALITY;
}

// A weapon Hero fights from range when its shots, which need distance, and the talents that favour them weigh more
// than its melee strikes and theirs. Each ability and talent weighs one plus its synergy with the rest of its side;
// a talent sides with the attacks it has more synergy with.
struct RangeWeights
{
    uint32 shots = 0;
    uint32 shotTalents = 0;
    uint32 shotWeight = 0;
    uint32 strikes = 0;
    uint32 strikeTalents = 0;
    uint32 strikeWeight = 0;

    bool AtRange() const { return shotWeight > strikeWeight; }
};

uint32 SideWeight(std::vector<uint32> const& side)
{
    uint32 weight = 0;
    for (size_t i = 0; i < side.size(); ++i)
    {
        std::vector<uint32> rest(side);
        rest.erase(rest.begin() + i);
        weight += 1 + AscensionWildcard::SynergyScore(AscensionWildcard::LoadedTables(), side[i], rest);
    }
    return weight;
}

RangeWeights WeighRange(Player const* bot)
{
    std::vector<uint32> shots;
    std::vector<uint32> strikes;
    std::vector<uint32> talents;
    for (AscensionWildcard::Slot const& slot : AscensionWildcard::Slots(bot))
    {
        std::vector<uint32> const* ranks = slot.EntryId ? RankSpellsOf(slot.EntryId) : nullptr;
        SpellInfo const* info = ranks && !ranks->empty() ? sSpellMgr->GetSpellInfo(ranks->front()) : nullptr;
        if (!info)
            continue;
        if (slot.Talent)
        {
            talents.push_back(slot.EntryId);
            continue;
        }
        if (info->IsPassive() || Heals(info))
            continue;

        if (info->RangeEntry && (info->RangeEntry->Flags & SPELL_RANGE_RANGED))
            shots.push_back(slot.EntryId);
        else if (info->DmgClass == SPELL_DAMAGE_CLASS_MELEE)
            strikes.push_back(slot.EntryId);
    }

    std::vector<uint32> shotSide = shots;
    std::vector<uint32> strikeSide = strikes;
    for (uint32 talent : talents)
    {
        uint32 const toShots = AscensionWildcard::SynergyScore(AscensionWildcard::LoadedTables(), talent, shots);
        uint32 const toStrikes = AscensionWildcard::SynergyScore(AscensionWildcard::LoadedTables(), talent, strikes);
        if (toShots > toStrikes)
            shotSide.push_back(talent);
        else if (toStrikes > toShots)
            strikeSide.push_back(talent);
    }

    RangeWeights weights;
    weights.shots = shots.size();
    weights.shotTalents = shotSide.size() - shots.size();
    weights.shotWeight = SideWeight(shotSide);
    weights.strikes = strikes.size();
    weights.strikeTalents = strikeSide.size() - strikes.size();
    weights.strikeWeight = SideWeight(strikeSide);
    return weights;
}

bool Shoots(Player const* bot)
{
    ObjectGuid::LowType const guid = bot->GetGUID().GetCounter();
    {
        std::lock_guard<std::mutex> lock(BuildStateLock);
        if (auto const found = Shooters.find(guid); found != Shooters.end())
            return found->second;
    }
    bool const atRange = WeighRange(bot).AtRange();
    std::lock_guard<std::mutex> lock(BuildStateLock);
    return Shooters.insert_or_assign(guid, atRange).first->second;
}
}  // namespace

bool IsWildcardRealm()
{
    return AscensionWildcard::PlaysWildcard(
        sConfigMgr->GetOption<std::string>("CoAChallenges.GameModes.Realm", "", false));
}

bool IsWildcardHeroBot(Player const* player)
{
    return player && AscensionWildcard::IsWildcardHero(player);
}

bool IsCoaBot(Player const* player)
{
    return player && (IsAscensionCustomClassId(player->getClass()) || IsWildcardHeroBot(player));
}

CoaRole GetWildcardRole(Player const* player)
{
    if (player->HasSpell(SPELL_PATH_HEALING))
        return CoaRole::Heal;
    if (player->HasSpell(SPELL_PATH_STRENGTH))
        return CoaRole::Tank;
    return CoaRole::Dps;
}

CoaStyle GetWildcardStyle(Player const* player)
{
    if (player->HasSpell(SPELL_PATH_INTELLECT) || player->HasSpell(SPELL_PATH_HEALING))
        return CoaStyle::Caster;
    // A tank holds its enemies in melee; any other Hero, on Agility, Duality or no path yet, fights where its
    // build is stronger.
    if (!player->HasSpell(SPELL_PATH_STRENGTH) && Shoots(player))
        return CoaStyle::Ranged;
    return CoaStyle::Melee;
}

uint8 GetWildcardPrimaryStats(Player const* player)
{
    if (player->HasSpell(SPELL_PATH_HEALING))
        return COA_STAT_INTELLECT | COA_STAT_SPIRIT;
    if (player->HasSpell(SPELL_PATH_INTELLECT))
        return COA_STAT_INTELLECT;
    if (player->HasSpell(SPELL_PATH_STRENGTH))
        return COA_STAT_STRENGTH | COA_STAT_STAMINA;
    if (player->HasSpell(SPELL_PATH_AGILITY))
        return COA_STAT_AGILITY;
    return COA_STAT_STRENGTH | COA_STAT_INTELLECT;
}

bool DraftWildcardBuild(Player* bot)
{
    if (!IsWildcardHeroBot(bot) || !sRandomPlayerbotMgr.IsRandomBot(bot))
        return false;

    CoaRole const role = GetWildcardRole(bot);
    CoaStyle const style = GetWildcardStyle(bot);

    if (BuildAboveLevel(bot))
        ResetWildcardBuild(bot);

    CoaRole const target = TargetRoleOf(bot);
    WildcardArchetype const starterArchetype = DraftArchetypeOf(bot, target);
    uint8 const starterBuild = WildcardBuildFor(bot->GetGUID().GetCounter(), starterArchetype, StarterEntriesOf(bot));
    AscensionWildcard::DraftBuild(bot,
        [starterArchetype, starterBuild](std::vector<std::vector<AscensionWildcard::Slot>> const& candidates)
        { return PickStarters(candidates, starterArchetype, starterBuild); },
        STARTER_DRAWS);
    WildcardArchetype const archetype = DraftArchetypeOf(bot, target);
    uint8 const build = WildcardBuildFor(bot->GetGUID().GetCounter(), archetype, StarterEntriesOf(bot));
    RerollMisfits(bot, archetype, build);
    TrainRanks(bot);

    uint32 const path = PathFor(target, ShapeOf(bot), archetype);
    RangeWeights const range = WeighRange(bot);
    // The server refuses a path change in combat: the bot takes its path when the fight is over.
    bool const pathDue = bot->IsInCombat() && AscensionWildcard::PrimaryStat(bot) != path;
    {
        std::lock_guard<std::mutex> lock(BuildStateLock);
        Shooters.insert_or_assign(bot->GetGUID().GetCounter(), range.AtRange());
        if (pathDue)
            PathsDue.insert(bot->GetGUID().GetCounter());
        else
            PathsDue.erase(bot->GetGUID().GetCounter());
    }
    if (path == PATH_AGILITY || path == PATH_DUALITY)
        LOG_INFO("playerbots.coa", "{} fights {}: {} shots and {} talents weigh {}, {} strikes and {} talents weigh {}",
                 bot->GetName(), range.AtRange() ? "at range" : "in melee", range.shots, range.shotTalents,
                 range.shotWeight, range.strikes, range.strikeTalents, range.strikeWeight);
    if (!pathDue)
    {
        std::vector<AscensionCoATalentState::KnownEntry> upload = AscensionWildcard::KnownEntries(bot);
        upload.push_back({ path, 1 });
        AscensionWildcard::ApplyBuildUpload(bot, upload);
    }

    ForgetWildcardKit(bot);
    return role != GetWildcardRole(bot) || style != GetWildcardStyle(bot);
}

bool TakeDueWildcardPath(Player const* bot)
{
    std::lock_guard<std::mutex> lock(BuildStateLock);
    return PathsDue.erase(bot->GetGUID().GetCounter()) > 0;
}

bool RedraftWildcardBot(Player* bot, bool reequip)
{
    if (!IsWildcardHeroBot(bot) || !sRandomPlayerbotMgr.IsRandomBot(bot))
        return false;

    // The reset takes the stat path away, and with it the role the bot played: it drafts for that role again.
    CoaRole const target = TargetRoleOf(bot);
    {
        std::lock_guard<std::mutex> lock(BuildStateLock);
        ChosenRoles.insert_or_assign(bot->GetGUID().GetCounter(), target);
    }
    uint8 const stats = GetWildcardPrimaryStats(bot);

    ResetWildcardBuild(bot);
    DraftWildcardBuild(bot);
    // Its gear was picked for the stats of its old path.
    if (reequip && GetWildcardPrimaryStats(bot) != stats && PlayerbotFactory::MayGearItself(bot))
        PlayerbotFactory(bot, bot->GetLevel()).InitEquipment(true);
    if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot))
        botAI->ResetStrategies();
    bot->CastSpell(bot, SPELL_LEVEL_UP_VISUAL, true);
    return true;
}

bool SetWildcardRole(Player* bot, CoaRole role, bool reequip, std::string& error)
{
    if (!IsWildcardHeroBot(bot))
    {
        error = "not a Wildcard Hero";
        return false;
    }
    if (bot->IsInCombat())
    {
        error = "not in combat";
        return false;
    }

    if (sRandomPlayerbotMgr.IsRandomBot(bot))
    {
        {
            std::lock_guard<std::mutex> lock(BuildStateLock);
            ChosenRoles.insert_or_assign(bot->GetGUID().GetCounter(), role);
        }
        // Its build already plays the role: nothing to roll again.
        if (AscensionWildcard::PrimaryStat(bot) && GetWildcardRole(bot) == role)
            return true;
        RedraftWildcardBot(bot, reequip);
        if (GetWildcardRole(bot) != role)
        {
            error = "the new build does not play that role";
            return false;
        }
        return true;
    }

    // A player's alt keeps the rolls its owner made: only its stat path changes, and the role with it.
    uint32 const path = PathFor(role, ShapeOf(bot));
    if (AscensionWildcard::PrimaryStat(bot) == path)
        return true;
    std::vector<AscensionCoATalentState::KnownEntry> upload = AscensionWildcard::KnownEntries(bot);
    upload.erase(std::remove_if(upload.begin(), upload.end(),
                                [](AscensionCoATalentState::KnownEntry const& known)
                                {
                                    return std::any_of(AscensionWildcard::PRIMARY_STAT_PATHS.begin(),
                                                       AscensionWildcard::PRIMARY_STAT_PATHS.end(),
                                                       [&known](AscensionWildcard::PrimaryStatPath const& candidate)
                                                       { return candidate.EntryId == known.EntryId; });
                                }),
                 upload.end());
    upload.push_back({ path, 1 });
    AscensionWildcard::BuildChoice const choice = AscensionWildcard::ApplyBuildUpload(bot, upload);
    if (choice.PrimaryStat != path)
    {
        error = choice.Result;
        return false;
    }
    {
        std::lock_guard<std::mutex> lock(BuildStateLock);
        Shooters.erase(bot->GetGUID().GetCounter());
    }
    ForgetWildcardKit(bot);
    return true;
}

char const* WildcardRoleName(CoaRole role)
{
    switch (role)
    {
        case CoaRole::Tank: return "tank";
        case CoaRole::Heal: return "healer";
        default:            return "damage";
    }
}

std::vector<std::string> DescribeWildcard(Player const* bot)
{
    CoaRole const role = GetWildcardRole(bot);
    char const* const position = GetWildcardStyle(bot) == CoaStyle::Melee ? "close" : "ranged";
    char const* path = "no";
    switch (AscensionWildcard::PrimaryStat(bot))
    {
        case PATH_STRENGTH:  path = "Strength"; break;
        case PATH_AGILITY:   path = "Agility"; break;
        case PATH_INTELLECT: path = "Intellect"; break;
        case PATH_HEALING:   path = "Healing"; break;
        case PATH_DUALITY:   path = "Duality"; break;
        default:             break;
    }

    std::vector<std::string> abilities;
    std::vector<std::string> talents;
    for (AscensionWildcard::Slot const& slot : AscensionWildcard::Slots(bot))
    {
        std::vector<uint32> const* ranks = slot.EntryId ? RankSpellsOf(slot.EntryId) : nullptr;
        if (!ranks || slot.Rank < 1 || slot.Rank > ranks->size())
            continue;
        SpellInfo const* info = sSpellMgr->GetSpellInfo((*ranks)[slot.Rank - 1]);
        if (!info || !info->SpellName[0] || !*info->SpellName[0])
            continue;
        std::string name = info->SpellName[0];
        if (ranks->size() > 1)
            name += " " + std::to_string(slot.Rank);
        (slot.Talent ? talents : abilities).push_back(name);
    }

    std::vector<std::string> lines;
    std::ostringstream head;
    WildcardArchetype const archetype = DraftArchetypeOf(bot, role);
    head << "WILDCARD " << WildcardRoleName(role) << ": " << path << " path (" << position << "), "
         << abilities.size() << " abilities, " << talents.size() << " talents, "
         << WildcardBuildName(WildcardBuildFor(bot->GetGUID().GetCounter(), archetype, StarterEntriesOf(bot))) << " "
         << WildcardArchetypeName(archetype) << " build";
    lines.push_back(head.str());

    // Whispers stay under the 255 characters of a chat line.
    auto const list = [&lines](std::string const& title, std::vector<std::string> const& names)
    {
        std::string line = title;
        bool first = true;
        for (std::string const& name : names)
        {
            if (!first && line.size() + name.size() + 2 > 240)
            {
                lines.push_back(line);
                line = title;
                first = true;
            }
            line += (first ? " " : ", ") + name;
            first = false;
        }
        if (!first)
            lines.push_back(line);
    };
    list("Abilities:", abilities);
    list("Talents:", talents);
    return lines;
}

void ResetWildcardBuild(Player* bot)
{
    if (!IsWildcardHeroBot(bot))
        return;

    AscensionWildcard::PrestigeSpecialization(bot);
    ForgetWildcardKit(bot);
}

std::vector<std::pair<uint32, uint32>> WildcardBuildSynergies(Player const* bot)
{
    std::vector<std::pair<uint32, uint32>> spells;
    std::vector<uint32> entries;
    for (AscensionWildcard::Slot const& slot : AscensionWildcard::Slots(bot))
    {
        std::vector<uint32> const* ranks = slot.EntryId ? RankSpellsOf(slot.EntryId) : nullptr;
        if (ranks && slot.Rank >= 1 && slot.Rank <= ranks->size())
        {
            spells.emplace_back((*ranks)[slot.Rank - 1], slot.EntryId);
            entries.push_back(slot.EntryId);
        }
    }

    for (auto& [spellId, score] : spells)
    {
        uint32 const entry = score;
        std::vector<uint32> others;
        for (uint32 other : entries)
            if (other != entry)
                others.push_back(other);
        score = AscensionWildcard::SynergyScore(AscensionWildcard::LoadedTables(), entry, others);
    }
    return spells;
}

std::vector<uint32> WildcardBuildSpells(Player const* bot)
{
    std::vector<uint32> spells;
    for (AscensionWildcard::Slot const& slot : AscensionWildcard::Slots(bot))
    {
        std::vector<uint32> const* ranks = slot.EntryId ? RankSpellsOf(slot.EntryId) : nullptr;
        if (ranks && slot.Rank >= 1 && slot.Rank <= ranks->size())
            spells.push_back((*ranks)[slot.Rank - 1]);
    }
    return spells;
}
