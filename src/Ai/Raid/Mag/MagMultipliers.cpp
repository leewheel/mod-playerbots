/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "MagMultipliers.h"
#include "ChooseTargetActions.h"
#include "EncounterHelpers.h"
#include "FollowActions.h"
#include "HunterActions.h"
#include "MagActions.h"
#include "MagHelpers.h"
#include "MageActions.h"
#include "MovementActions.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"

using namespace MagHelpers;
using namespace EncounterHelpers;

float MagtheridonUseManticronCubeMultiplier::GetValueInEncounter(Action* action)
{
    if (dynamic_cast<AttackAction*>(action) ||
        dynamic_cast<MagtheridonUseManticronCubeAction*>(action))
    {
        return 1.0f;
    }

    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action) &&
        !dynamic_cast<CastBlinkBackAction*>(action) &&
        !dynamic_cast<CastDisengageAction*>(action))
    {
        return 1.0f;
    }

    // By leewheel 2026-09-13 合并brighton 8c96a663: 采纳上游把 IsCubeClicker 判定前移的结构
    if (!IsCubeClicker(bot))
        return 1.0f;

// By leewheel 2026-09-09 合并brighton: boss按entry规则用17257, 采纳brighton加IsMagtheridonActive活性判定
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "17257");
    if (!magtheridon || !IsMagtheridonActive(magtheridon))
        return 1.0f;
    // End By leewheel

    // By leewheel 2026-09-13 合并brighton 8c96a663: 此处原重复的 IsCubeClicker 判定已随上游前移而删除
    // （2026-08-26 的"简化计时判定"写法已并入上游代码）
    // End By leewheel
    auto timerIt = blastNovaTimer.find(bot->GetInstanceId());
    if (timerIt == blastNovaTimer.end())
        return 1.0f;

    return getMSTimeDiff(timerIt->second, getMSTime()) >= BLAST_NOVA_INTERIM_MS ? 0.0f : 1.0f;
}

float MagtheridonHoldDpsMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<AttackAction*>(action) && !dynamic_cast<CastSpellAction*>(action))
        return 1.0f;

    if (dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "17257");
    if (!magtheridon || !IsMagtheridonActive(magtheridon))
        return 1.0f;

    if (PlayerbotAI::IsMainTank(bot))
        return 1.0f;

    //By leewheel 2026-09-15 合并brighton b2f6e460：采纳上游把 dpsWaitTimer 统一改名为 magDpsWaitTimer，
    //  并把 6 秒等待时长提取为公共常量 MAG_DPS_HOLD_MS（我方 2026-08-26 的局部 dpsWaitMs 写法随之退役）
    auto it = magDpsWaitTimer.find(magtheridon->GetInstanceId());
    if (it == magDpsWaitTimer.end())
        return 0.0f;

    return getMSTimeDiff(it->second, getMSTime()) <= MAG_DPS_HOLD_MS ? 0.0f : 1.0f;
}

float MagtheridonControlTankActionsMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    bool const isAvoidAoe = dynamic_cast<AvoidAoeAction*>(action);
    bool const isReachTargetSpell = dynamic_cast<CastReachTargetSpellAction*>(action);

    if (!isAvoidAoe && !isReachTargetSpell && !IsTauntAction(bot, action) &&
        !dynamic_cast<TankAssistAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action))
    {
        return 1.0f;
    }

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "17257");
    if (!magtheridon)
        return 1.0f;

    if (isAvoidAoe && magtheridon->GetVictim() != bot)
        return 1.0f;

    // The purpose is to block the main tank from charging the assist tanks' Channelers while moving
    // to the waiting position.
    if (isReachTargetSpell && PlayerbotAI::IsMainTank(bot))
        return IsMagtheridonActive(magtheridon) ? 1.0f : 0.0f;

    return 0.0f;
}

float MagtheridonAvoidDebrisDangerMultiplier::GetValueInEncounter(Action* action)
{
    if (dynamic_cast<AttackAction*>(action) ||
        dynamic_cast<MagtheridonUseManticronCubeAction*>(action) ||
        dynamic_cast<MagtheridonMoveOutOfDebrisAction*>(action))
    {
        return 1.0f;
    }

// By leewheel 2026-09-09 合并brighton: 采纳brighton细化(仅抑制移动/施法类action), boss按entry规则用17257
    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    // By leewheel 2026-09-13 合并brighton 8c96a663: 采纳上游新增的 IsCeilingCollapsed 判定, 保留 entry 化
    if (!IsCeilingCollapsed(bot))
        return 1.0f;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "17257");
    if (!magtheridon || !IsMagtheridonActive(magtheridon))
        return 1.0f;
    // End By leewheel

    constexpr float debrisSuppressionZone = 15.0f;
    return IsPositionInActiveDebris(
        botAI, bot->GetPositionX(), bot->GetPositionY(), debrisSuppressionZone) ? 0.0f : 1.0f;
}
