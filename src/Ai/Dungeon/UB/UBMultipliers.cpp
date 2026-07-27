/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

//By leewheel 2026-07-27 引入brighton-chi的UB(幽暗沼泽)副本策略
// 幽暗沼泽乘数实现文件
#include "UBMultipliers.h"
#include "AttackAction.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"
#include "UBActions.h"
#include "UBShared.h"

using namespace UnderbogHungarfen;

// 恶臭孢子乘数：Boss施放恶臭孢子时，允许撤退/规避/攻击动作，压制其他移动和接近动作
float HungarfenFoulSporesMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "hungarfen");
    if (!boss || !boss->HasAura(SPELL_FOUL_SPORES))
        return 1.0f;

    // 允许撤退、规避和攻击动作
    if (dynamic_cast<UBRetreatFromFoulSporesAction*>(action) || dynamic_cast<UBVacateSporeCloudAction*>(action) ||
        dynamic_cast<AttackAction*>(action))
        return 1.0f;

    // 压制移动和接近目标类动作
    if (dynamic_cast<MovementAction*>(action) || dynamic_cast<CastReachTargetSpellAction*>(action))
        return 0.0f;

    return 1.0f;
}

// 蘑菇忽略乘数：DPS在与Hungarfen战斗时，压制AOE法术（避免打蘑菇）
float HungarfenMushroomIgnoreMultiplier::GetValue(Action* action)
{
    // 只影响AOE动作
    if (action->getThreatType() != Action::ActionThreatType::Aoe)
        return 1.0f;

    // 只影响DPS
    if (!botAI->IsDps(bot))
        return 1.0f;

    // 治疗AOE不受影响
    if (dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    // 不在与Hungarfen战斗时不受影响
    if (!AI_VALUE2(Unit*, "find target", "hungarfen"))
        return 1.0f;

    // 压制DPS的AOE法术
    return 0.0f;
}
