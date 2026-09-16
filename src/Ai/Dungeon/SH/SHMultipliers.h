/*
* This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
* information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
* or (at your option) any later version.
*/

#ifndef PLAYERBOTS_SHMULTIPLIERS_H
#define PLAYERBOTS_SHMULTIPLIERS_H

#include "Multiplier.h"

// By leewheel 2026-09-16 破碎大厅（map 540）机器人权重调整

// 刃舞期间：非坦克只做"远离卡加斯"这一个移动，其他走位一律归零
// （模板取自 Gundrak 的 GaldarahMultiplier：避免机器人在 AoE 里来回横跳）
class KargathBladeDanceMultiplier : public Multiplier
{
public:
    KargathBladeDanceMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "kargath blade dance") {}

    float GetValue(Action* action) override;
};

// 主人跳下下水道期间：只做"走到洞口 + 跳下去"，其他移动一律归零
// （否则普通的跟随动作会不停尝试寻路到下层主人，与跳跃动作互相打架）
class ShatteredHallsSewerJumpMultiplier : public Multiplier
{
public:
    ShatteredHallsSewerJumpMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "shattered halls sewer jump") {}

    float GetValue(Action* action) override;
};

#endif
