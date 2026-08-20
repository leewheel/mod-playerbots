/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ATTACKACTION_H
#define PLAYERBOTS_ATTACKACTION_H

#include "MovementActions.h"

class PlayerbotAI;

class AttackAction : public MovementAction
{
public:
    AttackAction(PlayerbotAI* botAI, std::string const name) : MovementAction(botAI, name) {}

    bool Execute(Event event) override;

protected:
    bool Attack(Unit* target, bool with_pet = true);
};

class AttackMyTargetAction : public AttackAction
{
public:
    AttackMyTargetAction(PlayerbotAI* botAI, std::string const name = "attack my target") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};

class AttackDuelOpponentAction : public AttackAction
{
public:
    AttackDuelOpponentAction(PlayerbotAI* botAI, std::string const name = "attack duel opponent")
        : AttackAction(botAI, name)
    {
    }

public:
    bool Execute(Event event) override;
    bool isUseful() override;
};

class MeleeAction : public AttackAction
{
public:
    MeleeAction(PlayerbotAI* botAI) : AttackAction(botAI, "melee") {}

    std::string const GetTargetName() override { return "current target"; }
    bool isUseful() override;
};

// By leewheel 2026-07-15（2026-08-21 随 GenericActions 删除重构迁入 AttackAction）:
// 破潜行动作：当队伍成员被潜行怪物闷棍(如破碎大厅的碎手刺客)时，
// 使用职业适用的 AoE 法术把隐藏的偷袭者打出来。
class BreakStealthAction : public Action
{
public:
    BreakStealthAction(PlayerbotAI* botAI) : Action(botAI, "break stealth") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};
// End By leewheel

#endif
