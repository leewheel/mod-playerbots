/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GenericActions.h"
#include "PlayerbotAI.h"
#include "Player.h"
#include "Pet.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotTextMgr.h"
#include "CreatureAI.h"
#include "Playerbots.h"
#include "CharmInfo.h"
#include "SpellMgr.h"
#include "SpellInfo.h"
#include <vector>
#include <algorithm>

enum PetSpells
{
    PET_PROWL_1 = 24450,
    PET_PROWL_2 = 24452,
    PET_PROWL_3 = 24453,
    PET_COWER = 1742,
    PET_LEAP = 47482,
    PET_SPELL_LOCK_1 = 19244,
    PET_SPELL_LOCK_2 = 19647,
    PET_DEVOUR_MAGIC_1 = 19505,
    PET_DEVOUR_MAGIC_2 = 19731,
    PET_DEVOUR_MAGIC_3 = 19734,
    PET_DEVOUR_MAGIC_4 = 19736,
    PET_DEVOUR_MAGIC_5 = 27276,
    PET_DEVOUR_MAGIC_6 = 27277,
    PET_DEVOUR_MAGIC_7 = 48011,
    PET_SPIRIT_WOLF_LEAP = 58867
};

static std::vector<uint32> disabledPetSpells = {
    PET_PROWL_1, PET_PROWL_2, PET_PROWL_3,
    PET_COWER, PET_LEAP,
    PET_SPELL_LOCK_1, PET_SPELL_LOCK_2,
    PET_DEVOUR_MAGIC_1, PET_DEVOUR_MAGIC_2, PET_DEVOUR_MAGIC_3,
    PET_DEVOUR_MAGIC_4, PET_DEVOUR_MAGIC_5, PET_DEVOUR_MAGIC_6, PET_DEVOUR_MAGIC_7, PET_SPIRIT_WOLF_LEAP
};

bool MeleeAction::isUseful()
{
    // do not allow if can't attack from vehicle
    if (botAI->IsInVehicle() && !botAI->IsInVehicle(false, false, true))
        return false;

    // Do not start autoattack while prowled — let opener spells break stealth intentionally.
    // Future rogue stealth implementation should use this instead:
    // return !(botAI->HasAura("stealth", bot) || botAI->HasAura("prowl", bot));
    return !botAI->HasAura("prowl", bot);
}

bool TogglePetSpellAutoCastAction::Execute(Event /*event*/)
{
    Pet* pet = bot->GetPet();
    if (!pet)
    {
        return false;
    }
    // hack on high level spell after low level initialization
    std::vector<unsigned int> shouldRemove;
    for (unsigned int& m_autospell : pet->m_autospells)
    {
        if (!pet->HasSpell(m_autospell))
        {
            shouldRemove.push_back(m_autospell);
        }
    }
    for (unsigned int spellId : shouldRemove)
    {
        auto autospellItr = std::find(pet->m_autospells.begin(), pet->m_autospells.end(), spellId);
        if (autospellItr != pet->m_autospells.end())
            pet->m_autospells.erase(autospellItr);
    }
    bool toggled = false;
    for (PetSpellMap::const_iterator itr = pet->m_spells.begin(); itr != pet->m_spells.end(); ++itr)
    {
        if (itr->second.state == PETSPELL_REMOVED)
            continue;

        uint32 spellId = itr->first;
        const SpellInfo* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo || !spellInfo->IsAutocastable())
            continue;

        bool shouldApply = true;
        for (uint32 disabledSpell : disabledPetSpells)
        {
            if (spellId == disabledSpell)
            {
                shouldApply = false;
                break;
            }
        }
        bool isAutoCast = false;
        for (unsigned int& m_autospell : pet->m_autospells)
        {
            if (m_autospell == spellId)
            {
                isAutoCast = true;
                break;
            }
        }
        if (shouldApply != isAutoCast)
        {
            pet->ToggleAutocast(spellInfo, shouldApply);
            toggled = true;
        }
    }

    // Debug message if 宠物 spells have been toggled and debug is enabled
    if (toggled && sPlayerbotAIConfig.petChatCommandDebug == 1)
        botAI->TellMaster("宠物自动施法已切换。");

    return toggled;
}

bool PetAttackAction::Execute(Event /*event*/)
{
    Guardian* pet = bot->GetGuardianPet();
    if (!pet)
        return false;

    // Do not attack if the pet's stance is set to "被动".
    if (pet->GetReactState() == REACT_PASSIVE)
        return false;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    if (!bot->IsValidAttackTarget(target))
        return false;

    // This section has been commented because it was overriding the
    // 宠物's stance to "被动" every time the attack action was executed.
    // pet->SetReactState(REACT_PASSIVE);

    pet->ClearUnitState(UNIT_STATE_FOLLOW);
    pet->AttackStop();
    pet->SetTarget(target->GetGUID());

    pet->GetCharmInfo()->SetIsCommandAttack(true);
    pet->GetCharmInfo()->SetIsAtStay(false);
    pet->GetCharmInfo()->SetIsFollowing(false);
    pet->GetCharmInfo()->SetIsCommandFollow(false);
    pet->GetCharmInfo()->SetIsReturning(false);

    pet->ToCreature()->AI()->AttackStart(target);
    return true;
}

bool SetPetStanceAction::Execute(Event /*event*/)
{
    // Prepare a list to hold all controlled 宠物 and 守卫 creatures
    std::vector<Creature*> targets;

    // Add the bot's main 宠物 (if it exists) to the target list
    Pet* pet = bot->GetPet();
    if (pet)
        targets.push_back(pet);

    // Loop through all units controlled by the bot (could be 宠物s, 守卫s, etc.)
    for (Unit::ControlSet::const_iterator itr = bot->m_Controlled.begin(); itr != bot->m_Controlled.end(); ++itr)
    {
        // Only add creatures (skip players, vehicles, etc.)
        Creature* creature = dynamic_cast<Creature*>(*itr);
        if (!creature)
            continue;
        // Avoid adding the main 宠物 twice
        if (pet && creature == pet)
            continue;
        targets.push_back(creature);
    }

    // If there are no controlled 宠物s or 守卫s, notify the player and exit
    if (targets.empty())
    {
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "pet_no_pet_error", "你没有宠物或守卫宠物。", {}));
        return false;
    }

    // Get the default 宠物 stance from the configuration
    int32 stance = sPlayerbotAIConfig.defaultPetStance;
    ReactStates react = REACT_DEFENSIVE;
    std::string stanceText = "防御（来自配置，回退）";

    // Map the config stance integer to a ReactStates value and a message
    switch (stance)
    {
        case 0:
            react = REACT_PASSIVE;
            stanceText = "被动（来自配置）";
            break;
        case 1:
            react = REACT_DEFENSIVE;
            stanceText = "防御（来自配置）";
            break;
        case 2:
            react = REACT_AGGRESSIVE;
            stanceText = "主动（来自配置）";
            break;
        default:
            react = REACT_DEFENSIVE;
            stanceText = "防御（来自配置，回退）";
            break;
    }

    // Apply the stance to all target creatures (宠物s/守卫s)
    for (Creature* target : targets)
    {
        target->SetReactState(react);
        CharmInfo* charmInfo = target->GetCharmInfo();
        // If the creature has a CharmInfo, set the player-visible stance as well
        if (charmInfo)
            charmInfo->SetPlayerReactState(react);
    }

    // If debug is enabled in config, inform the master of the new stance
    if (sPlayerbotAIConfig.petChatCommandDebug == 1)
        botAI->TellMaster("宠物姿态已设为 " + stanceText + "（已应用于所有宠物/守卫）。");

    return true;
}

// By leewheel 2026-07-15: Break stealth action - tries class-appropriate AoE spells to flush out stealthed mobs.
// This is triggered when a party member is sapped (e.g. by Shattered Hand Assassins in Shattered Halls).

// Class-specific instant-cast or fast AoE spell names that can break stealth.
// Ordered by preference (most effective anti-stealth first).
struct BreakStealthSpellList
{
    uint8 playerClass;
    std::vector<std::string> spells;
};

static const BreakStealthSpellList breakStealthSpells[] = {
    {CLASS_WARRIOR,   {"thunder clap", "demoralizing shout", "piercing howl", "shockwave", "whirlwind"}},
    {CLASS_PALADIN,   {"consecration", "holy wrath"}},
    {CLASS_HUNTER,    {"flare", "volley", "multi-shot"}},
    {CLASS_MAGE,      {"arcane explosion", "frost nova", "cone of cold", "blast wave", "flamestrike", "blizzard"}},
    {CLASS_PRIEST,    {"holy nova"}},
    {CLASS_WARLOCK,   {"shadowfury", "rain of fire", "hellfire"}},
    {CLASS_DRUID,     {"hurricane", "starfall", "swipe"}},
    {CLASS_SHAMAN,    {"fire nova", "magma totem", "stoneclaw totem"}},
    {CLASS_ROGUE,     {"fan of knives"}},
    {CLASS_DEATH_KNIGHT, {"blood boil", "howling blast", "pestilence", "death and decay"}}
};

bool BreakStealthAction::isUseful()
{
    // Not useful if the bot itself is sapped/feared/sleeping
    if (bot->HasAuraWithMechanic(1 << MECHANIC_SAPPED) ||
        bot->HasAuraWithMechanic(1 << MECHANIC_SLEEP) ||
        bot->HasAuraType(SPELL_AURA_MOD_FEAR) ||
        bot->HasAuraType(SPELL_AURA_MOD_STUN) ||
        bot->HasAuraType(SPELL_AURA_MOD_CONFUSE))
        return false;

    // Not useful if not in combat
    if (!bot->IsInCombat())
        return false;

    // Not useful if not in a group (no one to be sapped)
    if (!bot->GetGroup())
        return false;

    return true;
}

bool BreakStealthAction::Execute(Event /*event*/)
{
    uint8 playerClass = bot->getClass();

    // Find the spell list for this class
    for (auto const& entry : breakStealthSpells)
    {
        if (entry.playerClass != playerClass)
            continue;

        // Try each spell in order of preference
        for (auto const& spellName : entry.spells)
        {
            if (botAI->CanCastSpell(spellName, bot))
            {
                if (botAI->CastSpell(spellName, bot))
                {
                    LOG_DEBUG("playerbots", "BreakStealthAction: {} cast {} to break stealth",
                        bot->GetName(), spellName);
                    return true;
                }
            }
        }
        break;  // Found the class entry, no need to keep searching
    }

    return false;
}

// End By leewheel
