/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "TameAction.h"
#include "DBCStructure.h"
#include "ObjectMgr.h"
#include "Pet.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotFactory.h"
#include "PlayerbotTextMgr.h"
#include "SpellMgr.h"
#include "WorldSession.h"
#include <algorithm>
#include <cctype>
#include <random>
#include <set>
#include <sstream>

bool IsExoticPet(CreatureTemplate const* creature)
{
    // Use the IsExotic() method from CreatureTemplate
    return creature && creature->IsExotic();
}

bool HasBeastMastery(Player* bot)
{
    // Beast Mastery talent aura ID for WotLK is 53270
    return bot->HasAura(53270);
}

bool TameAction::Execute(Event event)
{
    // Parse the user's input command into mode and value (e.g. "name wolf", "id 1234", etc.)
    std::string param = event.getParam();
    std::istringstream iss(param);
    std::string mode, value;
    iss >> mode;
    std::getline(iss, value);
    value.erase(0, value.find_first_not_of(" "));  // Remove leading spaces from value

    bool found = false;

    // Reset any previous pet name/id state
    lastPetName = "";
    lastPetId = 0;

    // If the command is "family" with no value, list all available pet families
    if (mode == "family" && value.empty())
    {
        std::set<std::string> normalFamilies;
        std::set<std::string> exoticFamilies;

        // Loop over all creature templates and collect tameable families
        CreatureTemplateContainer const* creatures = sObjectMgr->GetCreatureTemplates();
        for (auto itr = creatures->begin(); itr != creatures->end(); ++itr)
        {
            CreatureTemplate const& creature = itr->second;
            if (!creature.IsTameable(true))
                continue;

            CreatureFamilyEntry const* familyEntry = sCreatureFamilyStore.LookupEntry(creature.family);
            if (!familyEntry)
                continue;

            std::string familyName = familyEntry->Name[0];
            if (familyName.empty())
                continue;

            if (creature.IsExotic())
                exoticFamilies.insert(familyName);
            else
                normalFamilies.insert(familyName);
        }

        // Build the output message for the user
        std::ostringstream oss;
        oss << "可用宠物家族: ";
        size_t count = 0;
        for (auto const& name : normalFamilies)
        {
            if (count++ != 0)
                oss << ", ";
            oss << name;
        }
        if (!exoticFamilies.empty())
        {
            if (!normalFamilies.empty())
                oss << " | ";
            oss << "Exotic: ";
            count = 0;
            for (auto const& name : exoticFamilies)
            {
                if (count++ != 0)
                    oss << ", ";
                oss << name;
            }
        }

        botAI->TellError(oss.str());
        return true;
    }

    // Handle "tame abandon" command to give up your current pet
    if (mode == "abandon")
    {
        return AbandonPet();
    }

    // Try to process the command based on mode and value
    if (mode == "name" && !value.empty())
    {
        found = SetPetByName(value);
    }
    else if (mode == "id" && !value.empty())
    {
        // Try to convert value to an integer and set 宠物 by ID
        try
        {
            uint32 id = std::stoul(value);
            found = SetPetById(id);
        }
        catch (...)
        {
            botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "tame_invalid_id_error", "无效的驯服 ID。", {}));
        }
    }
    else if (mode == "family" && !value.empty())
    {
        found = SetPetByFamily(value);
    }
    else if (mode == "rename" && !value.empty())
    {
        found = RenamePet(value);
    }
    else
    {
        // Unrecognized command or missing argument; show usage
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "tame_usage_error",
            "用法: tame name <名称> | tame id <id> | tame family <家族> | tame rename <新名称> | tame abandon",
            {}));
        return false;
    }

    // If the requested tame/rename failed, return failure
    if (!found)
        return false;

    // For all non-rename commands, initialize the new pet and talents, then notify the master
    if (mode != "rename")
    {
        Player* bot = botAI->GetBot();
        PlayerbotFactory factory(bot, bot->GetLevel());
        factory.InitPet();
        factory.InitPetTalents();

        if (!lastPetName.empty() && lastPetId != 0)
        {
            std::ostringstream oss;
            botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "tame_pet_changed",
                "宠物已更换为 %name，ID: %id。",
                {{"%name", lastPetName}, {"%id", std::to_string(lastPetId)}}));
        }
        else
        {
            botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "tame_pet_changed_initialized", "宠物已更换并初始化！", {}));
        }
    }

    return true;
}

bool TameAction::SetPetByName(std::string const& name)
{
    // Make a lowercase copy of the input name for case-insensitive comparison
    std::string lowerName = name;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

    // Get the full list of creature templates from the object manager
    CreatureTemplateContainer const* creatures = sObjectMgr->GetCreatureTemplates();
    Player* bot = botAI->GetBot();

    // Iterate through all creature templates
    for (auto itr = creatures->begin(); itr != creatures->end(); ++itr)
    {
        CreatureTemplate const& creature = itr->second;
        std::string creatureName = creature.Name;
        // Convert creature's name to lowercase for comparison
        std::transform(creatureName.begin(), creatureName.end(), creatureName.begin(), ::tolower);

        // If the input name matches this creature's name
        if (creatureName == lowerName)
        {
            // Skip if the creature isn't tameable at all
            if (!creature.IsTameable(true))
                continue;

            // If the creature is exotic and the bot doesn't have Beast Mastery, show error and fail
            if (IsExoticPet(&creature) && !HasBeastMastery(bot))
            {
                botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                    "tame_exotic_requires_beast_mastery",
                    "除非拥有野兽掌控天赋，否则我无法使用 exotic pet。",
                    {}));
                return false;
            }

            // Skip if the creature isn't tameable by this bot (respecting exotic pet rules)
            if (!creature.IsTameable(bot->CanTameExoticPets()))
                continue;

            // Store the found 宠物's name and entry ID for later use/feedback
            lastPetName = creature.Name;
            lastPetId = creature.Entry;
            // Create and set this pet for the bot
            return CreateAndSetPet(creature.Entry);
        }
    }

    // If no suitable 宠物 found, show an error and return failure
    botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "tame_no_pet_by_name", "未找到可驯服的宠物，名称: %name", {{"%name", name}}));
    return false;
}

bool TameAction::SetPetById(uint32 id)
{
    // Look up the creature template by its numeric entry/id
    CreatureTemplate const* creature = sObjectMgr->GetCreatureTemplate(id);
    Player* bot = botAI->GetBot();

    // Proceed only if a valid creature was found
    if (creature)
    {
        // Check if this creature is ever tameable (ignore bot's own restrictions for now)
        if (!creature->IsTameable(true))
        {
            // If not tameable at all, show an error and fail
            botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "tame_no_pet_by_id", "未找到可驯服的宠物，ID: %id", {{"%id", std::to_string(id)}}));
            return false;
        }

        // If it's an exotic pet, make sure the bot has the Beast Mastery talent
        if (IsExoticPet(creature) && !HasBeastMastery(bot))
        {
            botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "tame_exotic_requires_beast_mastery",
                "除非拥有野兽掌控天赋，否则我无法使用 exotic pet。",
                {}));
            return false;
        }

        // Check if the bot is actually allowed to tame this pet (honoring exotic pet rules)
        if (!creature->IsTameable(bot->CanTameExoticPets()))
        {
            botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "tame_no_pet_by_id", "未找到可驯服的宠物，ID: %id", {{"%id", std::to_string(id)}}));
            return false;
        }

        // Remember this pet's name and id for later feedback
        lastPetName = creature->Name;
        lastPetId = creature->Entry;
        // Set and create the pet for the bot
        return CreateAndSetPet(creature->Entry);
    }

    // If no valid creature was found by id, show an error
    botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "tame_no_pet_by_id", "未找到可驯服的宠物，ID: %id", {{"%id", std::to_string(id)}}));
    return false;
}

bool TameAction::SetPetByFamily(std::string const& family)
{
    // Convert the input family name to lowercase for case-insensitive comparison
    std::string lowerFamily = family;
    std::transform(lowerFamily.begin(), lowerFamily.end(), lowerFamily.begin(), ::tolower);

    // Get all creature templates from the object manager
    CreatureTemplateContainer const* creatures = sObjectMgr->GetCreatureTemplates();
    Player* bot = botAI->GetBot();

    // Prepare a list of candidate creatures and track if any exotic pet is found
    std::vector<CreatureTemplate const*> candidates;
    bool foundExotic = false;

    // Iterate through all creature templates
    for (auto itr = creatures->begin(); itr != creatures->end(); ++itr)
    {
        CreatureTemplate const& creature = itr->second;

        // Skip if this creature is never tameable
        if (!creature.IsTameable(true))
            continue;

        // Look up the family entry for this creature
        CreatureFamilyEntry const* familyEntry = sCreatureFamilyStore.LookupEntry(creature.family);
        if (!familyEntry)
            continue;

        // Compare the family name in a case-insensitive way
        std::string familyName = familyEntry->Name[0];
        std::transform(familyName.begin(), familyName.end(), familyName.begin(), ::tolower);

        if (familyName != lowerFamily)
            continue;

        // If the creature is exotic, check Beast Mastery talent requirements
        if (IsExoticPet(&creature))
        {
            foundExotic = true;
            if (!HasBeastMastery(bot))
                continue;
        }

        // Only add as candidate if this bot is allowed to tame it (including exotic rules)
        if (!creature.IsTameable(bot->CanTameExoticPets()))
            continue;

        candidates.push_back(&creature);
    }

    // If no candidates found, inform the user of the reason and return false
    if (candidates.empty())
    {
        if (foundExotic && !HasBeastMastery(bot))
            botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "tame_exotic_requires_beast_mastery",
                "除非拥有野兽掌控天赋，否则我无法使用 exotic pet。",
                {}));
        else
            botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "tame_no_pet_by_family", "未找到可驯服的宠物，家族: %family", {{"%family", family}}));
        return false;
    }

    // Randomly select one candidate from the list to tame
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, candidates.size() - 1);

    CreatureTemplate const* selected = candidates[dis(gen)];

    // Save the selected 宠物's name and id for feedback
    lastPetName = selected->Name;
    lastPetId = selected->Entry;
    // Attempt to create and set the new pet for the bot
    return CreateAndSetPet(selected->Entry);
}

bool TameAction::RenamePet(std::string const& newName)
{
    Player* bot = botAI->GetBot();
    Pet* pet = bot->GetPet();
    // Check if the bot currently has a pet
    if (!pet)
    {
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "tame_no_pet_to_rename", "你没有可重命名的宠物。", {}));
        return false;
    }

    // Validate the new name: must not be empty and max 12 characters
    if (newName.empty() || newName.length() > 12)
    {
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "tame_pet_name_length_error",
            "宠物名称必须为 1 到 12 个字母字符。",
            {}));
        return false;
    }

    // Ensure all characters in the new name are alphabetic
    for (char c : newName)
    {
        if (!std::isalpha(static_cast<unsigned char>(c)))
        {
            botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "tame_pet_name_alpha_error",
                "宠物名称只能包含字母（A-Z, a-z）。",
                {}));
            return false;
        }
    }

    // Normalize the name: capitalize the first letter, lowercase the rest
    std::string normalized = newName;
    normalized[0] = std::toupper(normalized[0]);
    for (size_t i = 1; i < normalized.size(); ++i)
        normalized[i] = std::tolower(normalized[i]);

    // Check if the new name is reserved or forbidden
    if (sObjectMgr->IsReservedName(normalized))
    {
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "tame_pet_name_forbidden_error",
            "该宠物名称被禁止，请选择其他名称。",
            {}));
        return false;
    }

    // Set the pet's name and save it to the database
    pet->SetName(normalized);
    pet->SavePetToDB(PET_SAVE_AS_CURRENT);
    bot->GetSession()->SendPetNameQuery(pet->GetGUID(), pet->GetEntry());

    // Notify the master about the rename and give a tip to update the client name display
    botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "tame_pet_renamed", "你的宠物已重命名为 %name！", {{"%name", normalized}}));
    botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "tame_pet_rename_refresh_hint",
        "如果未看到新名称，请解散并重新召唤宠物。",
        {}));

    // Remove the current pet and (re-)cast Call Pet spell if the bot is a hunter
    bot->RemovePet(nullptr, PET_SAVE_AS_CURRENT, true);
    constexpr uint32 SPELL_CALL_PET = 883;
    if (bot->getClass() == CLASS_HUNTER && bot->HasSpell(SPELL_CALL_PET))
        bot->CastSpell(bot, SPELL_CALL_PET, true);

    return true;
}

bool TameAction::CreateAndSetPet(uint32 creatureEntry)
{
    Player* bot = botAI->GetBot();
    // Ensure the player is a hunter and at least level 10 (required for pets)
    if (bot->getClass() != CLASS_HUNTER || bot->GetLevel() < 10)
    {
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "tame_only_hunters_level_10", "只有 10 级以上的猎人才能有宠物。", {}));
        return false;
    }

    // Retrieve the creature template for the given entry (宠物 species info)
    CreatureTemplate const* creature = sObjectMgr->GetCreatureTemplate(creatureEntry);
    if (!creature)
    {
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "tame_creature_template_not_found", "未找到生物模板。", {}));
        return false;
    }

    // If the bot already has a current pet or an unslotted pet, remove them to avoid conflicts
    if (bot->GetPetStable() && bot->GetPetStable()->CurrentPet)
    {
        bot->RemovePet(nullptr, PET_SAVE_AS_CURRENT);
        bot->RemovePet(nullptr, PET_SAVE_NOT_IN_SLOT);
    }
    if (bot->GetPetStable() && bot->GetPetStable()->GetUnslottedHunterPet())
    {
        bot->GetPetStable()->UnslottedPets.clear();
        bot->RemovePet(nullptr, PET_SAVE_AS_CURRENT);
        bot->RemovePet(nullptr, PET_SAVE_NOT_IN_SLOT);
    }

    // Create the new tamed pet from the specified creature entry
    Pet* pet = bot->CreateTamedPetFrom(creatureEntry, 0);
    if (!pet)
    {
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "tame_create_pet_failed", "创建宠物失败。", {}));
        return false;
    }

    // Set the pet's level to one below the bot's current level, then add to the map and set to full level
    pet->SetUInt32Value(UNIT_FIELD_LEVEL, bot->GetLevel() - 1);
    pet->GetMap()->AddToMap(pet->ToCreature());
    pet->SetUInt32Value(UNIT_FIELD_LEVEL, bot->GetLevel());
    // Set the pet as the bot's active minion
    bot->SetMinion(pet, true);
    // Initialize talents appropriate for the pet's level
    pet->InitTalentForLevel();
    // Save pet to the database as the current pet
    pet->SavePetToDB(PET_SAVE_AS_CURRENT);
    // Initialize available 宠物 spells
    bot->PetSpellInitialize();

    // Further initialize pet stats to match the bot's level
    pet->InitStatsForLevel(bot->GetLevel());
    pet->SetLevel(bot->GetLevel());
    // Set happiness and health of the pet to maximum values
    pet->SetPower(POWER_HAPPINESS, pet->GetMaxPower(Powers(POWER_HAPPINESS)));
    pet->SetHealth(pet->GetMaxHealth());

    // Enable autocast for all active (not removed) non-passive spells the pet knows
    for (PetSpellMap::const_iterator itr = pet->m_spells.begin(); itr != pet->m_spells.end(); ++itr)
    {
        if (itr->second.state == PETSPELL_REMOVED)
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(itr->first);
        if (!spellInfo)
            continue;

        if (spellInfo->IsPassive())
            continue;

        pet->ToggleAutocast(spellInfo, true);
    }

    return true;
}

bool TameAction::AbandonPet()
{
    // Get the bot player and its current pet (if any)
    Player* bot = botAI->GetBot();
    Pet* pet = bot->GetPet();

    // Check if the bot has a pet and that it is a hunter pet
    if (pet && pet->getPetType() == HUNTER_PET)
    {
        // Remove the pet from the bot and mark it as deleted in the database
        bot->RemovePet(pet, PET_SAVE_AS_DELETED);
        // Inform the bot's master/player that the pet was abandoned
        botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "tame_pet_abandoned", "你的宠物已被放弃。", {}));
        return true;
    }
    else
    {
        // If there is no hunter pet, show an error message
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "tame_no_hunter_pet_to_abandon", "你没有可放弃的猎人宠物。", {}));
        return false;
    }
}
