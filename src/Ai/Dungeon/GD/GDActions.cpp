/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GDActions.h"
#include "Playerbots.h"

bool AvoidPoisonNovaAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "29304");
    if (!boss) { return false; }

    float distance = bot->GetExactDist2d(boss->GetPosition());
    float radius = 15.0f;
    float distanceExtra = 2.0f;

    if (distance < radius + distanceExtra)
    {
        return MoveAway(boss, radius + distanceExtra - distance);
    }

    return false;
}

bool AttackSnakeWrapAction::Execute(Event /*event*/)
{
    // By leewheel 2026-09-09 合并 brighton GD 重构: 攻击分配给本机器人的蛇缠目标
    Unit* snakeWrap = GundrakSladran::GetAssignedSnakeWrap(botAI);
    if (!snakeWrap) { return false; }

    if (AI_VALUE(Unit*, "current target") == snakeWrap) { return false; }

    return Attack(snakeWrap);
    // End By leewheel
}

bool SladranStackOnTankAction::Execute(Event /*event*/)
{
    // By leewheel 2026-09-09 合并 brighton GD 重构: 近战集结到坦克身旁迎击
    Player* tank = GundrakSladran::GetStackTank(botAI);
    if (!tank) { return false; }

    return MoveTo(tank, GundrakSladran::STACK_CLOSE_TO_YD, MovementPriority::MOVEMENT_COMBAT);
    // End By leewheel
}

bool SladranTankHoldAction::Execute(Event /*event*/)
{
    // By leewheel 2026-09-09 合并 brighton GD 重构: 坦克保持拉稳斯莱德酋长
    Unit* boss = GundrakSladran::GetTankHoldTarget(botAI);
    if (!boss) { return false; }

    return Attack(boss);
    // End By leewheel
}

bool AvoidWhirlingSlashAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "29306");
    if (!boss) { return false; }

    float distance = bot->GetExactDist2d(boss->GetPosition());
    float radius = 5.0f;
    float distanceExtra = 2.0f;

    if (distance < radius + distanceExtra)
    {
        if (botAI->IsTank(bot))
        {
            // The boss chases tank during this, leads to jittery stutter-stepping
            // by the tank if we don't pre-move additional range. 2*radius seems ok
            return MoveAway(boss, (2.0f * radius) + distanceExtra - distance);
        }
        // else
        return MoveAway(boss, radius + distanceExtra - distance);
    }

    return false;
}
