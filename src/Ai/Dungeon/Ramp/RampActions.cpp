/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RampActions.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "RampShared.h"

using namespace RampShared;
using namespace EncounterHelpers;

// By leewheel 2026-09-19 合并上游 the-lab（2b15a2fb..799274b4）：上游把本模块的 HFR/（地狱火城墙，
//   我方已在提交 31446315 完成 entry 化）整体删除并重写为同目录 Ramp/；重写时 Boss 名称又被写回
//   英文字符串，我方 entry 化成果随之丢失。按项目规则第 97 条（机器人策略里的 Boss 名称一律用
//   entry，不得依赖客户端本地化名称，否则中文客户端下 find target 永远命中不到），此处把本文件
//   全部 "find target" 参数补回 entry。映射取自被我方 entry 化过的旧文件
//   modules/mod-playerbots/src/Ai/Dungeon/HFR/HFR*.cpp（提交 31446315）：
//     "hellfire watcher"   => 17309   （原 HFR/HFRActions.cpp:21）
//     "fiendish hound"     => 17280   （原 HFR/HFRActions.cpp:60）
//     "omor the unscarred" => 17308   （原 HFR/HFRMultipliers.cpp:21）
//     "vazruden"           => 17537   （原 HFR/HFRActions.cpp:96）
//     "nazan"              => 17536   （原 HFR/HFRMultipliers.cpp:67）
//   注意：本文件里的 "tremor totem" / "fire resistance aura" / "has totem" 是【法术名与 AI 关键字】，
//   不是 Boss 名，必须保持英文，不得替换。
// End By leewheel

// Watchkeeper Gargolmar

// Hellfire Watchers will be marked with skull

bool GargolmarMarkHellfireWatchersAction::Execute(Event /*event*/)
{
    Unit* watcher = AI_VALUE2(Unit*, "find target", "17309");
    if (!watcher)
        return false;

    return MarkTargetWithSkull(bot, watcher);
}

// Omor the Unscarred

// Flee from other players if you have Treacherous Aura or Bane of Treachery
bool OmorTreacheryAuraFleeFromPlayersAction::Execute(Event /*event*/)
{
    if (!GetNearestPlayerInRadius(bot, OMOR_TREACHERY_AURA_SAFE_DISTANCE))
        return false;

    bot->CastStop();

    return MoveFromGroup(OMOR_TREACHERY_AURA_SAFE_DISTANCE);
}

// Nearby bots should flee from the tank if it has Treacherous Aura or Bane of Treachery
bool OmorTreacheryAuraFleeFromTankAction::Execute(Event /*event*/)
{
    Unit* omor = AI_VALUE2(Unit*, "find target", "17308");

    if (!omor)
        return false;

    Unit* tank = GetGroupMainTank(bot);

    if (!tank)
        return false;

    if (bot->GetExactDist2d(tank) >= OMOR_TREACHERY_AURA_SAFE_DISTANCE)
        return false;

    bot->CastStop();
    return MoveAway(tank, OMOR_TREACHERY_AURA_SAFE_DISTANCE);
}

// Ranged spread out from each other
bool OmorRangedSpreadAction::Execute(Event /*event*/)
{
    constexpr float minDistance = OMOR_TREACHERY_AURA_SAFE_DISTANCE;

    if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, minDistance))
        return FleePosition(nearestPlayer->GetPosition(), minDistance);

    return false;
}

// Mark Fiendish Hound with skull
bool OmorMarkFiendishHoundAction::Execute(Event /*event*/)
{
    Unit* hound = AI_VALUE2(Unit*, "find target", "17280");
    if (!hound)
        return false;

    return MarkTargetWithSkull(bot, hound);
}

// Vazruden & Nazan

// Tank Vazruden in the middle of the platform
bool VazrudenTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* vazruden = AI_VALUE2(Unit*, "find target", "17537");
    if (!vazruden)
        return false;

    if (AI_VALUE(Unit*, "current target") != vazruden)
        return Attack(vazruden);

    if (vazruden->GetVictim() != bot || !bot->IsWithinMeleeRange(vazruden) || bot->GetHealthPct() <= 25.0f)
        return false;

    Position const& position = VAZRUDEN_TANK_POSITION;
    constexpr float arrivalDist = 10.0f;
    float distToPosition = bot->GetExactDist2d(position);

    if (distToPosition <= arrivalDist)
        return false;

    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, position, arrivalDist, vazruden, moveX, moveY, backwards))
        return false;

    return MoveTo(RAMP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// Mark Vazruden with 'Skull'
bool VazrudenMarkBossAction::Execute(Event /*event*/)
{
    Unit* vazruden = AI_VALUE2(Unit*, "find target", "17537");
    if (!vazruden)
        return false;

    return MarkTargetWithSkull(bot, vazruden);
}

// Shamans use Tremor totem when Nazan is active
bool NazanSetTremorTotemAction::Execute(Event /*event*/)
{
    return  !AI_VALUE2(bool, "has totem", "tremor totem") &&
            botAI->CanCastSpell("tremor totem", bot) &&
            botAI->CastSpell("tremor totem", bot);
}

// Shamans use Fire Resistance totem when Nazan is active
bool NazanSetFireResistanceTotemAction::Execute(Event /*event*/)
{
    return  !AI_VALUE2(bool, "has totem", "fire resistance totem") &&
            botAI->CanCastSpell("fire resistance totem", bot) &&
            botAI->CastSpell("fire resistance totem", bot);
}

// Paladins use Fire Resistance aura when Nazan is active
bool NazanSetFireResistanceAuraAction::Execute(Event /*event*/)
{
    return  !botAI->HasAura("fire resistance aura", bot) &&
            botAI->CanCastSpell("fire resistance aura", bot) &&
            botAI->CastSpell("fire resistance aura", bot);
}
