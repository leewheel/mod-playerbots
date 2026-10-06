/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UKTriggers.h"
#include "AiObjectContext.h"
#include "Playerbots.h"

bool KelesethFrostTombTrigger::IsActive()
{
    GuidVector members = AI_VALUE(GuidVector, "group members");
    for (auto& member : members)
    {
        Unit* unit = botAI->GetUnit(member);
        if (unit && unit->HasAura(SPELL_FROST_TOMB))
        {
            return true;
        }
    }
    return false;
}

bool DalronnDpsTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "27389");
    if (!boss || !boss->isTargetableForAttack()) { return false; }

    // This doesn't cause issues with healers currently and they will continue to heal even when included here
    return !PlayerbotAI::IsTank(bot);
}

bool IngvarDreadfulRoarTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "23954");
    if (!boss) { return false; }

    if (boss->FindCurrentSpellBySpellId(SPELL_DREADFUL_ROAR))
    {
        return true;
    }
    return false;
}

bool IngvarSmashTankTrigger::IsActive()
{
// By leewheel 2026-10-07 合并 #2854：静态调用；目标按 entry 查找（汉化库英文名匹配不上）
    Unit* boss = AI_VALUE2(Unit*, "find target", "23954");
    if (!boss || !PlayerbotAI::IsTank(bot)) { return false; }

    if (boss->FindCurrentSpellBySpellId(SPELL_SMASH) ||
        boss->FindCurrentSpellBySpellId(SPELL_DARK_SMASH))
        {
            return true;
        }
    return false;
}

bool IngvarSmashTankReturnTrigger::IsActive()
{
// By leewheel 2026-10-07 合并 #2854：静态调用；目标按 entry 查找
    Unit* boss = AI_VALUE2(Unit*, "find target", "23954");
    // if (!boss || !PlayerbotAI::IsTank(bot) || boss->HasUnitState(UNIT_STATE_CASTING))
    // Ignore casting state as Ingvar will sometimes chain-cast a roar after a smash..
    // We don't want this to prevent our tank from repositioning properly.
    if (!boss || !PlayerbotAI::IsTank(bot)) { return false; }

    return true;
}

bool NotBehindIngvarTrigger::IsActive()
{
// By leewheel 2026-10-07 合并 #2854：静态调用；目标按 entry 查找
    Unit* boss = AI_VALUE2(Unit*, "find target", "23954");
    if (!boss || PlayerbotAI::IsTank(bot)) { return false; }

    return AI_VALUE2(bool, "behind", "current target");
}
