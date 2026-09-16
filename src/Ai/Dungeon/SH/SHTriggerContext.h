/*
* This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
* information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
* or (at your option) any later version.
*/

#ifndef PLAYERBOTS_SHTRIGGERCONTEXT_H
#define PLAYERBOTS_SHTRIGGERCONTEXT_H

#include "AiObjectContext.h"
#include "TriggerContext.h"
#include "SHTriggers.h"

// By leewheel 2026-09-16 破碎大厅（map 540）触发条件注册
class TbcDungeonShatteredHallsTriggerContext : public NamedObjectContext<Trigger>
{
public:
    TbcDungeonShatteredHallsTriggerContext()
    {
        // 高阶术士奈瑟库斯
        creators["nethekurse dark spin"] =
            &TbcDungeonShatteredHallsTriggerContext::nethekurse_dark_spin;

        // 血卫士波伦 / 血卫士
        creators["porung blaze on ground"] =
            &TbcDungeonShatteredHallsTriggerContext::porung_blaze_on_ground;

        // 战争使者沃姆罗格
        creators["omrogg burning maul"] =
            &TbcDungeonShatteredHallsTriggerContext::omrogg_burning_maul;

        // 酋长卡加斯·刃拳
        creators["kargath blade dance"] =
            &TbcDungeonShatteredHallsTriggerContext::kargath_blade_dance;

        creators["kargath assassins are active"] =
            &TbcDungeonShatteredHallsTriggerContext::kargath_assassins_are_active;

        // 下水道（跟随主人跳下去）
        creators["shattered halls master in sewer"] =
            &TbcDungeonShatteredHallsTriggerContext::shattered_halls_master_in_sewer;
    }
private:
    static Trigger* nethekurse_dark_spin(PlayerbotAI* botAI) {
        return new NethekurseDarkSpinTrigger(botAI);
    }

    static Trigger* porung_blaze_on_ground(PlayerbotAI* botAI) {
        return new PorungBlazeOnGroundTrigger(botAI);
    }

    static Trigger* omrogg_burning_maul(PlayerbotAI* botAI) {
        return new OmroggBurningMaulTrigger(botAI);
    }

    static Trigger* kargath_blade_dance(PlayerbotAI* botAI) {
        return new KargathBladeDanceTrigger(botAI);
    }

    static Trigger* kargath_assassins_are_active(PlayerbotAI* botAI) {
        return new KargathAssassinsAreActiveTrigger(botAI);
    }

    static Trigger* shattered_halls_master_in_sewer(PlayerbotAI* botAI) {
        return new ShatteredHallsMasterInSewerTrigger(botAI);
    }
};

#endif
