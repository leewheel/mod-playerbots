/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SMActions.h"
#include "Playerbots.h"
#include "SMShared.h"
#include "SMTriggers.h"

using namespace SMShared;

bool ArcanistDoanMoveOutOfDetonationAction::Execute(Event /*event*/)
{
    Unit* doan = AI_VALUE2(Unit*, "find target", "arcanist doan");
    if (!doan)
        return false;

    float const distance = bot->GetExactDist2d(doan);
    if (distance >= DETONATION_SAFE_DISTANCE)
        return false;

    bot->CastStop();
    return MoveAway(doan, DETONATION_SAFE_DISTANCE - distance);
}

bool ArcanistDoanRangedKeepAwayAction::Execute(Event /*event*/)
{
    Unit* doan = AI_VALUE2(Unit*, "find target", "arcanist doan");
    if (!doan)
        return false;

    float const distance = bot->GetExactDist2d(doan);
    if (distance >= DOAN_RANGED_DISTANCE)
        return false;

    return MoveAway(doan, DOAN_RANGED_DISTANCE - distance);
}

bool HerodMoveFromWhirlwindAction::Execute(Event /*event*/)
{
    Unit* herod = AI_VALUE2(Unit*, "find target", "herod");
    if (!IsWhirlwinding(herod))
        return false;

    float const distance = bot->GetExactDist2d(herod);
    if (distance >= WHIRLWIND_SAFE_DISTANCE)
        return false;

    return MoveAway(herod, WHIRLWIND_SAFE_DISTANCE - distance);
}

bool AttackWhitemaneAction::Execute(Event /*event*/)
{
    Unit* whitemane = WhitemaneToFocus(botAI, context);
    if (!whitemane || AI_VALUE(Unit*, "current target") == whitemane)
        return false;

    return Attack(whitemane);
}
