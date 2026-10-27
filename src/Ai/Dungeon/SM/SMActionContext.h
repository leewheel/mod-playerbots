/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SMACTIONCONTEXT_H
#define PLAYERBOTS_SMACTIONCONTEXT_H

#include "NamedObjectContext.h"
#include "SMActions.h"

class VanillaDungeonSMActionContext : public NamedObjectContext<Action>
{
public:
    VanillaDungeonSMActionContext()
    {
        creators["arcanist doan move out of detonation"] =
            &VanillaDungeonSMActionContext::arcanist_doan_move_out_of_detonation;
        creators["arcanist doan ranged keep away"] = &VanillaDungeonSMActionContext::arcanist_doan_ranged_keep_away;
        creators["herod move from whirlwind"] = &VanillaDungeonSMActionContext::herod_move_from_whirlwind;
        creators["attack whitemane"] = &VanillaDungeonSMActionContext::attack_whitemane;
    }
private:
    static Action* arcanist_doan_move_out_of_detonation(PlayerbotAI* botAI) {
        return new ArcanistDoanMoveOutOfDetonationAction(botAI);
    }
    static Action* arcanist_doan_ranged_keep_away(PlayerbotAI* botAI) { return new ArcanistDoanRangedKeepAwayAction(botAI); }
    static Action* herod_move_from_whirlwind(PlayerbotAI* botAI) { return new HerodMoveFromWhirlwindAction(botAI); }
    static Action* attack_whitemane(PlayerbotAI* botAI) { return new AttackWhitemaneAction(botAI); }
};

#endif
