/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

//By leewheel 2026-07-27 引入brighton-chi的UB(幽暗沼泽)副本策略
// 幽暗沼泽触发器实现文件
#include "UBTriggers.h"
#include "Playerbots.h"
#include "UBShared.h"

using namespace UnderbogHungarfen;

// 恶臭孢子触发器：Boss拥有恶臭孢子光环时激活
bool UBFoulSporesTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "hungarfen");
    return boss && boss->HasAura(SPELL_FOUL_SPORES);
}

// 孢子云危险触发器：附近存在危险蘑菇时激活
bool UBSporeCloudDangerTrigger::IsActive()
{
    // 不在与Hungarfen战斗时不激活
    if (!AI_VALUE2(Unit*, "find target", "hungarfen"))
        return false;

    // 检查附近是否有危险蘑菇
    GuidVector const& mushrooms = AI_VALUE_REF(GuidVector, "ub mushrooms");
    return GetNearestDangerousMushroom(bot, mushrooms, MushroomDangerRange(bot)) != nullptr;
}
