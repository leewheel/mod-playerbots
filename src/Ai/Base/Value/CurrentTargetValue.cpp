/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CurrentTargetValue.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"

Unit* CurrentTargetValue::Get()
{
    if (selection.IsEmpty())
        return nullptr;

    Unit* unit = ObjectAccessor::GetUnit(*bot, selection);
    // if (unit && !bot->IsWithinLOSInMap(unit))
    //     return nullptr;

    return unit;
}

void CurrentTargetValue::Set(Unit* target)
{
    ObjectGuid const next = target ? target->GetGUID() : ObjectGuid::Empty;
    // CoA: AiPlayerbot.CoaLogTargets, one line per change of target.
    if (next != selection && CoaLogsTargets(bot))
    {
        Unit* previous = selection.IsEmpty() ? nullptr : ObjectAccessor::GetUnit(*bot, selection);
        LOG_INFO("playerbots", "coa target: {} {} -> {}{}", bot->GetName(),
                 previous ? previous->GetName() + (previous->IsAlive() ? (bot->IsWithinLOSInMap(previous) ? "" : " (out of sight)") : " (dead)") : "none",
                 target ? target->GetName() : "none",
                 target ? " at " + std::to_string(int(bot->GetDistance(target))) + " yd" + (bot->IsWithinLOSInMap(target) ? "" : ", out of sight") : "");
    }
    selection = next;
}

bool CoaLogsTargets(Player* bot)
{
    uint32 const mode = sPlayerbotAIConfig.coaLogTargets;
    return bot && (mode == 2 ? bot->GetGroup() != nullptr : mode == 1 && bot->InBattleground());
}
