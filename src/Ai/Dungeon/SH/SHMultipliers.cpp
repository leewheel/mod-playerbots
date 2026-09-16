/*
* This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
* information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
* or (at your option) any later version.
*/

#include "SHMultipliers.h"
#include "SHActions.h"
#include "SHTriggers.h"
#include "MovementActions.h"
#include "Playerbots.h"

// By leewheel 2026-09-16 破碎大厅 Multiplier 实现

// 刃舞期间：除"远离卡加斯"之外的所有走位动作权重归零。
// 同 Gundrak 的 GaldarahMultiplier —— 不然机器人会在刃舞冲锋路径上反复进出。
float KargathBladeDanceMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<KargathAvoidBladeDanceAction*>(action))
        return 1.0f;

    Unit* boss = AI_VALUE2(Unit*, "find target", "16808");
    if (!boss)
        return 1.0f;

    auto const danceSpell = static_cast<uint32>(ShatteredHallsIDs::SPELL_BLADE_DANCE_DMG);
    if (!boss->HasAura(danceSpell) &&
        !boss->HasAura(static_cast<uint32>(ShatteredHallsIDs::SPELL_BLADE_DANCE_TARGETING)))
        return 1.0f;

    if (botAI->IsTank(bot))
        return 1.0f;

    if (dynamic_cast<MovementAction*>(action))
        return 0.0f;

    return 1.0f;
}

// 主人在下水道、机器人还在上层：只保留"跟着跳下去"，抑制其他移动。
// 否则普通跟随动作会一直尝试寻路到下层的主人，而那条路根本走不通（寻路失败），
// 结果就是机器人在洞口边缘反复起步 / 停下，永远跳不下去。
float ShatteredHallsSewerJumpMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<ShatteredHallsFollowMasterIntoSewerAction*>(action))
        return 1.0f;

    // 复用触发器的判据，避免逻辑分叉
    ShatteredHallsMasterInSewerTrigger sewerTrigger(botAI);
    if (!sewerTrigger.IsActive())
        return 1.0f;

    if (dynamic_cast<MovementAction*>(action))
        return 0.0f;

    return 1.0f;
}
