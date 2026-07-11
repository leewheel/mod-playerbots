/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PetsAction.h"

#include "CharmInfo.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "Pet.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "SharedDefines.h"

bool PetsAction::Execute(Event event)
{
    // Extract the command parameter from the event (e.g., "aggressive", "defensive", "attack", etc.)
    std::string param = event.getParam();
    if (param.empty() && !defaultCmd.empty())
        param = defaultCmd;

    if (param.empty())
    {
        // If no parameter is provided, show usage instructions and return.
        std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "pet_usage_error", "用法: 宠物 <主动|防御|被动|stance|attack|follow|stay>", {});
        botAI->TellError(text);
        return false;
    }

    Player* bot = botAI->GetBot();

    // Collect all controlled pets and guardians, except totems, into the targets vector.
    std::vector<Creature*> targets;
    Pet* pet = bot->GetPet();
    if (pet)
        targets.push_back(pet);

    for (Unit::ControlSet::const_iterator itr = bot->m_Controlled.begin(); itr != bot->m_Controlled.end(); ++itr)
    {
        Creature* creature = dynamic_cast<Creature*>(*itr);
        if (!creature)
            continue;
        if (pet && creature == pet)
            continue;
        if (creature->IsTotem())
            continue;
        targets.push_back(creature);
    }

    // If no pets or guardians are found, notify and return.
    if (targets.empty())
    {
        std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "pet_no_pet_error", "你没有宠物或守卫宠物。", {});
        botAI->TellError(text);
        return false;
    }

    ReactStates react;
    std::string stanceText;

    // Handle stance commands: aggressive, defensive, or passive.
    if (param == "aggressive")
    {
        react = REACT_AGGRESSIVE;
        stanceText = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "pet_stance_aggressive", "主动", {});
    }
    else if (param == "defensive")
    {
        react = REACT_DEFENSIVE;
        stanceText = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "pet_stance_defensive", "防御", {});
    }
    else if (param == "passive")
    {
        react = REACT_PASSIVE;
        stanceText = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "pet_stance_passive", "被动", {});
    }
    // The "stance" command simply reports the current stance of each pet/guardian.
    else if (param == "stance")
    {
        for (Creature* target : targets)
        {
            std::string type = target->IsPet() ?
                PlayerbotTextMgr::instance().GetBotTextOrDefault("pet_type_pet", "宠物", {}) :
                PlayerbotTextMgr::instance().GetBotTextOrDefault("pet_type_guardian", "守卫", {});
            std::string name = target->GetName();
            std::string stance;
            switch (target->GetReactState())
            {
                case REACT_AGGRESSIVE:
                    stance = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                        "pet_stance_aggressive", "主动", {});
                    break;
                case REACT_DEFENSIVE:
                    stance = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                        "pet_stance_defensive", "防御", {});
                    break;
                case REACT_PASSIVE:
                    stance = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                        "pet_stance_passive", "被动", {});
                    break;
                default:
                    stance = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                        "pet_stance_unknown", "未知", {});
                    break;
            }
            std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "pet_stance_report", "当前 %type \"%name\" 的姿态: %stance。",
                {{"type", type}, {"name", name}, {"stance", stance}});
            botAI->TellMaster(text);
        }
        return true;
    }
    // The "attack" command forces pets/guardians to attack the master's selected target.
    else if (param == "attack")
    {
        // Try to get the master's selected target.
        Player* master = botAI->GetMaster();
        Unit* targetUnit = nullptr;

        if (master)
        {
            ObjectGuid masterTargetGuid = master->GetTarget();
            if (!masterTargetGuid.IsEmpty())
                targetUnit = botAI->GetUnit(masterTargetGuid);
        }

        // If no valid target is selected, show an error and return.
        if (!targetUnit)
        {
            std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "pet_no_target_error", "主控未选择有效目标。", {});
            botAI->TellError(text);
            return false;
        }
        if (!targetUnit->IsAlive())
        {
            std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "pet_target_dead_error", "目标未存活。", {});
            botAI->TellError(text);
            return false;
        }
        if (!bot->IsValidAttackTarget(targetUnit))
        {
            std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "pet_invalid_target_error", "目标不是机器人的有效攻击目标。", {});
            botAI->TellError(text);
            return false;
        }
        if (sPlayerbotAIConfig.IsPvpProhibited(bot->GetZoneId(), bot->GetAreaId()) &&
            (targetUnit->IsPlayer() || targetUnit->IsPet()) &&
            (!bot->duel || bot->duel->Opponent != targetUnit))
        {
            std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "pet_pvp_prohibited_error", "在禁止 PvP 的区域无法命令宠物攻击玩家。", {});
            botAI->TellError(text);
            return false;
        }

        bool didAttack = false;
        // For each controlled 宠物/守卫, command them to attack the selected target.
        for (Creature* petCreature : targets)
        {
            CharmInfo* charmInfo = petCreature->GetCharmInfo();
            if (!charmInfo)
                continue;

            petCreature->ClearUnitState(UNIT_STATE_FOLLOW);
            // Only command attack if not already attacking the target, or if not currently under command attack.
            if (petCreature->GetVictim() != targetUnit ||
                (petCreature->GetVictim() == targetUnit && !charmInfo->IsCommandAttack()))
            {
                if (petCreature->GetVictim())
                    petCreature->AttackStop();

                if (!petCreature->IsPlayer() && petCreature->ToCreature()->IsAIEnabled)
                {
                    // For AI-enabled creatures (NPC 宠物s/守卫s): issue attack command and set flags.
                    charmInfo->SetIsCommandAttack(true);
                    charmInfo->SetIsAtStay(false);
                    charmInfo->SetIsFollowing(false);
                    charmInfo->SetIsCommandFollow(false);
                    charmInfo->SetIsReturning(false);

                    petCreature->ToCreature()->AI()->AttackStart(targetUnit);

                    didAttack = true;
                }
                else  // For charmed player 宠物s/守卫s
                {
                    if (petCreature->GetVictim() && petCreature->GetVictim() != targetUnit)
                        petCreature->AttackStop();

                    charmInfo->SetIsCommandAttack(true);
                    charmInfo->SetIsAtStay(false);
                    charmInfo->SetIsFollowing(false);
                    charmInfo->SetIsCommandFollow(false);
                    charmInfo->SetIsReturning(false);

                    petCreature->Attack(targetUnit, true);
                    didAttack = true;
                }
            }
        }
        // Inform the master if the command succeeded or failed.
        if (didAttack && sPlayerbotAIConfig.petChatCommandDebug == 1)
        {
            std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "pet_attack_success", "已命令宠物攻击你的目标。", {});
            botAI->TellMaster(text);
        }
        else if (!didAttack)
        {
            std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "pet_attack_failed", "宠物未攻击。（已在攻击或无法攻击目标）", {});
            botAI->TellError(text);
        }
        return didAttack;
    }
    // The "follow" command makes all 宠物s/守卫s follow the bot.
    else if (param == "follow")
    {
        botAI->PetFollow();
        if (sPlayerbotAIConfig.petChatCommandDebug == 1)
        {
            std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "pet_follow_success", "已命令宠物跟随。", {});
            botAI->TellMaster(text);
        }
        return true;
    }
    // The "stay" command causes all 宠物s/守卫s to stop and stay in place.
    else if (param == "stay")
    {
        for (Creature* target : targets)
        {
            // If not already in controlled motion, stop movement and set to idle.
            bool controlledMotion =
                target->GetMotionMaster()->GetMotionSlotType(MOTION_SLOT_CONTROLLED) != NULL_MOTION_TYPE;
            if (!controlledMotion)
            {
                target->StopMovingOnCurrentPos();
                target->GetMotionMaster()->Clear(false);
                target->GetMotionMaster()->MoveIdle();
            }

            CharmInfo* charmInfo = target->GetCharmInfo();
            if (charmInfo)
            {
                // Set charm/宠物 state flags for "stay".
                charmInfo->SetCommandState(COMMAND_STAY);
                charmInfo->SetIsCommandAttack(false);
                charmInfo->SetIsCommandFollow(false);
                charmInfo->SetIsFollowing(false);
                charmInfo->SetIsReturning(false);
                charmInfo->SetIsAtStay(!controlledMotion);
                charmInfo->SaveStayPosition(controlledMotion);
                if (target->ToPet())
                    target->ToPet()->ClearCastWhenWillAvailable();

                charmInfo->SetForcedSpell(0);
                charmInfo->SetForcedTargetGUID();
            }
        }
        if (sPlayerbotAIConfig.petChatCommandDebug == 1)
        {
            std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "pet_stay_success", "已命令宠物停留。", {});
            botAI->TellMaster(text);
        }
        return true;
    }
    // Unknown command: show usage instructions and return.
    else
    {
        std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "pet_unknown_command_error", "未知宠物命令: %param。用法: 宠物 <主动|防御|被动|stance|attack|follow|stay>",
            {{"param", param}});
        botAI->TellError(text);
        return false;
    }

    // For stance commands, apply the chosen stance to all targets.
    for (Creature* target : targets)
    {
        target->SetReactState(react);
        CharmInfo* charmInfo = target->GetCharmInfo();
        if (charmInfo)
            charmInfo->SetPlayerReactState(react);
    }

    // Inform the master of the new stance if debug is enabled.
    if (sPlayerbotAIConfig.petChatCommandDebug == 1)
    {
        std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "pet_stance_set_success", "宠物姿态已设为 %stance.",
            {{"stance", stanceText}});
        botAI->TellMaster(text);
    }

    return true;
}
