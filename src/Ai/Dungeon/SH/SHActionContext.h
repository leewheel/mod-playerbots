/*
* This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
* information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
* or (at your option) any later version.
*/

#ifndef PLAYERBOTS_SHACTIONCONTEXT_H
#define PLAYERBOTS_SHACTIONCONTEXT_H

#include "AiObjectContext.h"
#include "Action.h"
#include "SHActions.h"

// By leewheel 2026-09-16 破碎大厅（map 540）动作注册
class TbcDungeonShatteredHallsActionContext : public NamedObjectContext<Action>
{
public:
    TbcDungeonShatteredHallsActionContext() : NamedObjectContext<Action>(false, true)
    {
        // 高阶术士奈瑟库斯
        creators["nethekurse avoid dark spin"] =
            &TbcDungeonShatteredHallsActionContext::nethekurse_avoid_dark_spin;

        // 血卫士波伦 / 血卫士
        creators["porung move out of blaze"] =
            &TbcDungeonShatteredHallsActionContext::porung_move_out_of_blaze;

        // 战争使者沃姆罗格
        creators["omrogg avoid burning maul"] =
            &TbcDungeonShatteredHallsActionContext::omrogg_avoid_burning_maul;

        // 酋长卡加斯·刃拳
        creators["kargath avoid blade dance"] =
            &TbcDungeonShatteredHallsActionContext::kargath_avoid_blade_dance;

        creators["kargath mark assassins"] =
            &TbcDungeonShatteredHallsActionContext::kargath_mark_assassins;

        // 下水道（跟随主人跳下去）
        creators["shattered halls follow master into sewer"] =
            &TbcDungeonShatteredHallsActionContext::shattered_halls_follow_master_into_sewer;
    }
private:
    static Action* nethekurse_avoid_dark_spin(PlayerbotAI* botAI) {
        return new NethekurseAvoidDarkSpinAction(botAI);
    }

    static Action* porung_move_out_of_blaze(PlayerbotAI* botAI) {
        return new PorungMoveOutOfBlazeAction(botAI);
    }

    static Action* omrogg_avoid_burning_maul(PlayerbotAI* botAI) {
        return new OmroggAvoidBurningMaulAction(botAI);
    }

    static Action* kargath_avoid_blade_dance(PlayerbotAI* botAI) {
        return new KargathAvoidBladeDanceAction(botAI);
    }

    static Action* kargath_mark_assassins(PlayerbotAI* botAI) {
        return new KargathMarkAssassinsAction(botAI);
    }

    static Action* shattered_halls_follow_master_into_sewer(PlayerbotAI* botAI) {
        return new ShatteredHallsFollowMasterIntoSewerAction(botAI);
    }
};

#endif
