/*
* This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
* information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
* or (at your option) any later version.
*/

#ifndef PLAYERBOTS_SHACTIONS_H
#define PLAYERBOTS_SHACTIONS_H

#include "AttackAction.h"
#include "MovementActions.h"
#include "Action.h"

// By leewheel 2026-09-16 破碎大厅（map 540）机器人动作定义

// 高阶术士奈瑟库斯：暗影旋风期间非坦克离开近战圈
class NethekurseAvoidDarkSpinAction : public MovementAction
{
public:
    NethekurseAvoidDarkSpinAction(
        PlayerbotAI* botAI, std::string const name = "nethekurse avoid dark spin") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

// 血卫士波伦 / 血卫士：踩到地面烈焰就挪开
class PorungMoveOutOfBlazeAction : public MovementAction
{
public:
    PorungMoveOutOfBlazeAction(
        PlayerbotAI* botAI, std::string const name = "porung move out of blaze") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

// 战争使者沃姆罗格：燃烧之锤阶段非坦克离开近战圈
class OmroggAvoidBurningMaulAction : public MovementAction
{
public:
    OmroggAvoidBurningMaulAction(
        PlayerbotAI* botAI, std::string const name = "omrogg avoid burning maul") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

// 酋长卡加斯·刃拳：刃舞期间非坦克离开近战圈
class KargathAvoidBladeDanceAction : public MovementAction
{
public:
    KargathAvoidBladeDanceAction(
        PlayerbotAI* botAI, std::string const name = "kargath avoid blade dance") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

// 酋长卡加斯：给破碎刺客挂骷髅（机制标记位）
class KargathMarkAssassinsAction : public Action
{
public:
    KargathMarkAssassinsAction(
        PlayerbotAI* botAI, std::string const name = "kargath mark assassins") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

// 下水道：主人跳下去了，机器人跟着跳下去（MoveTo 到洞口边缘 → JumpTo 落到下层）
class ShatteredHallsFollowMasterIntoSewerAction : public MovementAction
{
public:
    ShatteredHallsFollowMasterIntoSewerAction(
        PlayerbotAI* botAI, std::string const name = "shattered halls follow master into sewer") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

#endif
