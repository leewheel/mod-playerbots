/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SMTriggers.h"

#include "DungeonBossHelpers.h"
#include "Playerbots.h"
#include "SMShared.h"

using namespace SMShared;

bool ArcanistDoanBossCastingDetonationTrigger::IsActive()
{
    Unit* doan = AI_VALUE2(Unit*, "find target", "arcanist doan");
    return doan && IsDetonationImminent(doan) && bot->GetExactDist2d(doan) < DETONATION_SAFE_DISTANCE;
}

bool IsWhirlwinding(Unit const* herod)
{
    return herod && herod->IsAlive() &&
        (DungeonBoss::HasAuraNamed(herod, "Whirlwind") || DungeonBoss::IsCastingNamed(herod, "Whirlwind"));
}

Unit* WhitemaneToFocus(PlayerbotAI* botAI, AiObjectContext* context)
{
    Unit* whitemane = AI_VALUE2(Unit*, "find target", "high inquisitor whitemane");
    Unit* mograine = AI_VALUE2(Unit*, "find target", "scarlet commander mograine");
    if (!whitemane || !mograine || !whitemane->IsAlive() || !mograine->IsAlive())
        return nullptr;

    // While Mograine lies feigning death he cannot be selected; only once she has raised him are both up.
    if (!whitemane->isTargetableForAttack() || !mograine->isTargetableForAttack())
        return nullptr;

    return whitemane;
}

bool ArcanistDoanRangedTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* doan = AI_VALUE2(Unit*, "find target", "arcanist doan");
    return doan && doan->IsAlive() && !IsDetonationImminent(doan) && bot->GetExactDist2d(doan) < DOAN_RANGED_DISTANCE;
}

bool HerodWhirlwindTrigger::IsActive()
{
    if (botAI->IsTank(bot))
        return false;

    Unit* herod = AI_VALUE2(Unit*, "find target", "herod");
    return IsWhirlwinding(herod) && bot->GetExactDist2d(herod) < WHIRLWIND_SAFE_DISTANCE;
}

bool WhitemaneFocusTrigger::IsActive()
{
    if (botAI->IsTank(bot) || botAI->IsHeal(bot))
        return false;

    Unit* whitemane = WhitemaneToFocus(botAI, context);
    return whitemane && AI_VALUE(Unit*, "current target") != whitemane;
}
