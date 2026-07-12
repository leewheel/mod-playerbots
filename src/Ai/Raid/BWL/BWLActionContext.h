#ifndef PLAYERBOTS_BWLACTIONCONTEXT_H
#define PLAYERBOTS_BWLACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "BWLActions.h"
#include "BWLHelpers.h"
#include "BossAuraActions.h"

class RaidBwlActionContext : public NamedObjectContext<Action>
{
public:
    RaidBwlActionContext()
    {
        creators["bwl check onyxia scale cloak"] = &RaidBwlActionContext::bwl_check_onyxia_scale_cloak;
        creators["bwl turn off suppression device"] = &RaidBwlActionContext::bwl_turn_off_suppression_device;

        creators["bwl razorgore fire resistance"] = &RaidBwlActionContext::bwl_razorgore_fire_resistance_action;
        creators["bwl razorgore avoid aoe"] = &RaidBwlActionContext::bwl_razorgore_avoid_aoe;
        creators["bwl razorgore mark boss"] = &RaidBwlActionContext::bwl_razorgore_mark_boss;

        creators["bwl vaelastrasz fire resistance"] = &RaidBwlActionContext::bwl_vaelastrasz_fire_resistance_action;
        creators["bwl vaelastrasz move away"] = &RaidBwlActionContext::bwl_vaelastrasz_move_away;

        creators["bwl use hourglass sand"] = &RaidBwlActionContext::bwl_use_hourglass_sand;
        creators["bwl nefarian fear ward"] = &RaidBwlActionContext::bwl_nefarian_fear_ward;
        creators["bwl death talon wyrmguard tank move away"] = &RaidBwlActionContext::bwl_death_talon_wyrmguard_tank_move_away;
        creators["bwl death talon wyrmguard ranged move away"] = &RaidBwlActionContext::bwl_death_talon_wyrmguard_ranged_move_away;

        //By leewheel 2026年7月12日
        // 自定义Boss: Valthorax
        creators["bwl valthorax frost resistance"] = &RaidBwlActionContext::bwl_valthorax_frost_resistance;
        creators["bwl valthorax shadow resistance"] = &RaidBwlActionContext::bwl_valthorax_shadow_resistance;
        creators["bwl valthorax avoid frost bomb"] = &RaidBwlActionContext::bwl_valthorax_avoid_frost_bomb;
        creators["bwl valthorax attack vabomination"] = &RaidBwlActionContext::bwl_valthorax_attack_vabomination;
        creators["bwl valthorax attack adds"] = &RaidBwlActionContext::bwl_valthorax_attack_adds;
    }

private:
    static Action* bwl_check_onyxia_scale_cloak(PlayerbotAI* ai) { return new BwlOnyxiaScaleCloakAuraCheckAction(ai); }
    static Action* bwl_turn_off_suppression_device(PlayerbotAI* ai) { return new BwlTurnOffSuppressionDeviceAction(ai); }
    static Action* bwl_razorgore_fire_resistance_action(PlayerbotAI* ai) { return new BossFireResistanceAction(ai, "razorgore the untamed"); }
    static Action* bwl_razorgore_avoid_aoe(PlayerbotAI* ai) { return new BwlRazorgoreAvoidAoeAction(ai); }
    static Action* bwl_razorgore_mark_boss(PlayerbotAI* ai) { return new BwlRazorgoreMarkBossAction(ai); }
    static Action* bwl_vaelastrasz_fire_resistance_action(PlayerbotAI* ai) { return new BossFireResistanceAction(ai, "vaelastrasz the corrupt"); }
    static Action* bwl_vaelastrasz_move_away(PlayerbotAI* ai) { return new BwlVaelastraszMoveAwayAction(ai); }
    static Action* bwl_use_hourglass_sand(PlayerbotAI* ai) { return new BwlUseHourglassSandAction(ai); }
    static Action* bwl_nefarian_fear_ward(PlayerbotAI* ai) { return new BwlNefarianFearWardAction(ai); }
    static Action* bwl_death_talon_wyrmguard_tank_move_away(PlayerbotAI* ai) { return new BwlDeathTalonWyrmguardTankMoveAwayAction(ai); }
    static Action* bwl_death_talon_wyrmguard_ranged_move_away(PlayerbotAI* ai) { return new BwlDeathTalonWyrmguardRangedMoveAwayAction(ai); }
    // Custom Boss: Valthorax
    static Action* bwl_valthorax_frost_resistance(PlayerbotAI* ai) { return new BossFrostResistanceAction(ai, BlackwingLairHelpers::BOSS_NAME_VALTHORAX); }
    static Action* bwl_valthorax_shadow_resistance(PlayerbotAI* ai) { return new BossShadowResistanceAction(ai, BlackwingLairHelpers::BOSS_NAME_VALTHORAX); }
    static Action* bwl_valthorax_avoid_frost_bomb(PlayerbotAI* ai) { return new BwlValthoraxAvoidFrostBombAction(ai); }
    static Action* bwl_valthorax_attack_vabomination(PlayerbotAI* ai) { return new BwlValthoraxAttackVabominationAction(ai); }
    static Action* bwl_valthorax_attack_adds(PlayerbotAI* ai) { return new BwlValthoraxAttackAddsAction(ai); }
    //End By leewheel
};

#endif
