/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AiObjectContext.h"
#include "MechValueContext.h"
#include "ValueContext.h"
#include "Ai/Dungeon/Mech/MechValueContext.h"
//By leewheel 2026-07-27 引入brighton-chi的UB(幽暗沼泽)副本策略
#include "Ai/Dungeon/UB/UBValueContext.h"
//End By leewheel

void AiObjectContext::BuildSharedValueContexts(SharedNamedObjectContextList<UntypedValue>& valueContexts)
{
    valueContexts.Add(new ValueContext());
    valueContexts.Add(new TbcDungeonMechValueContext());
    //By leewheel 2026-07-27 引入brighton-chi的UB(幽暗沼泽)副本策略
    valueContexts.Add(new TbcDungeonUnderbogValueContext());
    //End By leewheel
}
