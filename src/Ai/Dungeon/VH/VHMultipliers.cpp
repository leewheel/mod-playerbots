/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "VHMultipliers.h"
#include "ChooseTargetActions.h"
#include "Playerbots.h"
#include "VHActions.h"
#include "VHTriggers.h"

float ErekemMultiplier::GetValue(Action* action)
{
// By leewheel 2026-10-07 合并 #2854：静态调用；目标按 entry 查找（汉化库英文名匹配不上）
    Unit* boss = AI_VALUE2(Unit*, "find target", "29315");
    if (!boss || !PlayerbotAI::IsDps(bot)) { return 1.0f; }

    if (dynamic_cast<DpsAssistAction*>(action))
    {
        return 0.0f;
    }
    if (action->getThreatType() == Action::ActionThreatType::Aoe)
    {
        return 0.0f;
    }
    return 1.0f;
}

float IchoronMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "29313");
    if (!boss) { return 1.0f; }

    if (dynamic_cast<DpsAssistAction*>(action)
        || dynamic_cast<TankAssistAction*>(action)
        || dynamic_cast<DropTargetAction*>(action))
    {
        return 0.0f;
    }
    return 1.0f;
}

float ZuramatMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "29314");
    if (!boss) { return 1.0f; }

    if (bot->HasAura(SPELL_VOID_SHIFTED))
    {
        if (dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<TankAssistAction*>(action))
        {
            return 0.0f;
        }
    }

    if (boss->HasAura(SPELL_SHROUD_OF_DARKNESS) && dynamic_cast<AttackAction*>(action))
    {
        return 0.0f;
    }
    return 1.0f;
}
