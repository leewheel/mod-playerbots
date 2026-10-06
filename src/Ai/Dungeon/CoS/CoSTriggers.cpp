/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CoSTriggers.h"
#include "AiObjectContext.h"
#include "Playerbots.h"

bool ExplodeGhoulTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "26530");
    if (!boss) { return false; }

    float distance = 10.0f;
    float distanceExtra = 2.0f;
    GuidVector corpses = AI_VALUE(GuidVector, "nearest corpses");
    for (auto i = corpses.begin(); i != corpses.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (unit && unit->GetEntry() == NPC_RISEN_GHOUL)
        {
            if (bot->GetExactDist2d(unit) < distance + distanceExtra)
            {
                return true;
            }
        }
    }
    return false;
}

bool EpochRangedTrigger::IsActive()
{
// By leewheel 2026-10-07 合并 #2854：采纳上游静态调用写法 PlayerbotAI::IsMelee；
    //   目标仍按 entry 查找（本服 creature_template 名已汉化，英文名 chrono-lord epoch 匹配不上）。
    return !PlayerbotAI::IsMelee(bot) && AI_VALUE2(Unit*, "find target", "26532");
}
