/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AttackAction.h"
#include "CreatureAI.h"
#include "Event.h"
#include "LastMovementValue.h"
#include "LootObjectStack.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "Unit.h"
#include "WaitForAttackStrategy.h"
#include <vector>

bool AttackAction::Execute(Event /*event*/)
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    if (!target->IsInWorld())
        return false;

    return Attack(target);
}

bool AttackMyTargetAction::Execute(Event /*event*/)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    ObjectGuid guid = master->GetTarget();
    if (!guid)
    {
        if (verbose)
            botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "pull_no_target_error", "你没有目标", {}));

        return false;
    }

    botAI->GetAiObjectContext()->GetValue<GuidVector>("prioritized targets")->Set({guid});
    bool result = Attack(botAI->GetUnit(guid));
    if (result)
        context->GetValue<ObjectGuid>("pull target")->Set(guid);

    return result;
}

bool AttackAction::Attack(Unit* target, bool /*with_pet*/ /*true*/)
{
    if (!target)
    {
        if (verbose)
            botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "attack_no_target_error", "我没有目标", {}));

        return false;
    }

    if (!target->IsInWorld())
    {
        if (verbose)
            botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "attack_target_not_in_world_error",
                "%target 已不在世界中。",
                {{"%target", target->GetName()}}));

        return false;
    }

    if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == FLIGHT_MOTION_TYPE ||
        bot->HasUnitState(UNIT_STATE_IN_FLIGHT))
    {
        if (verbose)
            botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "attack_in_flight_error", "飞行中无法攻击", {}));

        return false;
    }

    // Check if bot OR target is in prohibited zone/area (skip for duels)
    if ((target->IsPlayer() || target->IsPet()) &&
        (!bot->duel || bot->duel->Opponent != target) &&
        (sPlayerbotAIConfig.IsPvpProhibited(bot->GetZoneId(), bot->GetAreaId()) ||
        sPlayerbotAIConfig.IsPvpProhibited(target->GetZoneId(), target->GetAreaId())))
    {
        if (verbose)
            botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "attack_pvp_prohibited_error",
                "在禁止 PvP 的区域无法攻击其他玩家。",
                {}));

        return false;
    }

    if (bot->IsFriendlyTo(target))
    {
        if (verbose)
            botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "attack_target_friendly_error",
                "%target 是我的友方。",
                {{"%target", target->GetName()}}));

        return false;
    }

    if (target->isDead())
    {
        if (verbose)
            botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "attack_target_dead_error",
                "%target 已死亡。",
                {{"%target", target->GetName()}}));

        return false;
    }

    if (!bot->IsWithinLOSInMap(target))
    {
        if (verbose)
            botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "attack_target_not_in_sight_error",
                "%target 不在我的视线内。",
                {{"%target", target->GetName()}}));

        return false;
    }

    // Infantry attacks are not allowed from vehicles drivers.
    // Check is needed to stop some auto-attack situations.
    if (botAI->IsInVehicle() && !botAI->IsInVehicle(false, false, true))
        return false;

    Unit* oldTarget = context->GetValue<Unit*>("current target")->Get();
    bool shouldMelee = bot->IsWithinMeleeRange(target) || botAI->IsMelee(bot);

    bool sameTarget = oldTarget == target && bot->GetVictim() == target;
    bool inCombat = botAI->GetState() == BOT_STATE_COMBAT;
    bool sameAttackMode = bot->HasUnitState(UNIT_STATE_MELEE_ATTACKING) == shouldMelee;

    if (sameTarget && inCombat && sameAttackMode)
    {
        if (verbose)
            botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "attack_already_attacking_error",
                "我已在攻击 %target。",
                {{"%target", target->GetName()}}));

        return false;
    }

    if (!bot->IsValidAttackTarget(target))
    {
        if (verbose)
            botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "attack_invalid_target_error", "我无法攻击无效目标。", {}));

        return false;
    }

    // if (bot->IsMounted() && bot->IsWithinLOSInMap(target))
    // {
    //     WorldPacket emptyPacket;
    //     bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);
    // }

    ObjectGuid guid = target->GetGUID();
    bot->SetSelection(target->GetGUID());

    context->GetValue<Unit*>("old target")->Set(oldTarget);
    context->GetValue<Unit*>("current target")->Set(target);
    context->GetValue<LootObjectStack*>("available loot")->Get()->Add(guid);

    LastMovement& lastMovement = AI_VALUE(LastMovement&, "last movement");
    bool moveControlled = bot->GetMotionMaster()->GetMotionSlotType(MOTION_SLOT_CONTROLLED) != NULL_MOTION_TYPE;
    if (lastMovement.priority < MovementPriority::MOVEMENT_COMBAT && bot->isMoving() && !moveControlled)
    {
        AI_VALUE(LastMovement&, "last movement").clear();
        bot->GetMotionMaster()->Clear(false);
        bot->StopMoving();
    }

    if (botAI->CanMove() && !bot->HasInArc(CAST_ANGLE_IN_FRONT, target))
        ServerFacade::instance().SetFacingTo(bot, target);

    botAI->ChangeEngine(BOT_STATE_COMBAT);

    if (!WaitForAttackStrategy::ShouldWait(botAI))
        bot->Attack(target, shouldMelee);
    /* prevent 宠物 dead immediately in group */
    // if (bot->GetMap()->IsDungeon() && bot->GetGroup() && !target->IsInCombat())
    // {
    //     with_pet = false;
    // }
    // if (Pet* pet = bot->GetPet())
    // {
    //     if (with_pet)
    //     {
    //         pet->SetReactState(REACT_DEFENSIVE);
    //         pet->SetTarget(target->GetGUID());
    //         pet->GetCharmInfo()->SetIsCommandAttack(true);
    //         pet->AI()->AttackStart(target);
    //     }
    //     else
    //     {
    //         pet->SetReactState(REACT_PASSIVE);
    //         pet->GetCharmInfo()->SetIsCommandFollow(true);
    //         pet->GetCharmInfo()->IsReturning();
    //     }
    // }
    return true;
}

bool AttackDuelOpponentAction::isUseful() { return AI_VALUE(Unit*, "duel target"); }

bool AttackDuelOpponentAction::Execute(Event /*event*/) { return Attack(AI_VALUE(Unit*, "duel target")); }

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

// By leewheel 2026-07-15（2026-08-21 随 GenericActions 删除重构迁入 AttackAction）:
// 破潜行动作实现：按职业选择最有效的 AoE 法术尝试破除潜行。
// 法术列表按反潜行效果优先级排列。

// 各职业用于破除潜行的瞬发或快速 AoE 法术名
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
    // 自身处于闷棍/沉睡/恐惧/眩晕/混乱状态时不可用
    if (bot->HasAuraWithMechanic(1 << MECHANIC_SAPPED) ||
        bot->HasAuraWithMechanic(1 << MECHANIC_SLEEP) ||
        bot->HasAuraType(SPELL_AURA_MOD_FEAR) ||
        bot->HasAuraType(SPELL_AURA_MOD_STUN) ||
        bot->HasAuraType(SPELL_AURA_MOD_CONFUSE))
        return false;

    // 未进入战斗时不可用
    if (!bot->IsInCombat())
        return false;

    // 不在队伍中时不可用（没有需要救援的队员）
    if (!bot->GetGroup())
        return false;

    return true;
}

bool BreakStealthAction::Execute(Event /*event*/)
{
    uint8 playerClass = bot->getClass();

    // 查找当前职业对应的法术列表
    for (auto const& entry : breakStealthSpells)
    {
        if (entry.playerClass != playerClass)
            continue;

        // 按优先级依次尝试每个法术
        for (auto const& spellName : entry.spells)
        {
            if (botAI->CanCastSpell(spellName, bot))
            {
                if (botAI->CastSpell(spellName, bot))
                {
                    LOG_DEBUG("playerbots", "破潜行动作: {} 施放 {} 破除潜行",
                        bot->GetName(), spellName);
                    return true;
                }
            }
        }
        break;  // 已找到本职业条目，无需继续查找
    }

    return false;
}
// End By leewheel
