/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MCACTIONCONTEXT_H
#define PLAYERBOTS_MCACTIONCONTEXT_H

#include "Action.h"
#include "BossAuraActions.h"
#include "MCActions.h"
#include "MCHelpers.h"
#include "NamedObjectContext.h"

class RaidMcActionContext : public NamedObjectContext<Action>
{
public:
    RaidMcActionContext()
    {
        creators["mc lucifron shadow resistance"] = &RaidMcActionContext::lucifron_shadow_resistance;
        creators["mc magmadar fire resistance"] = &RaidMcActionContext::magmadar_fire_resistance;
        creators["mc gehennas shadow resistance"] = &RaidMcActionContext::gehennas_shadow_resistance;
        creators["mc garr fire resistance"] = &RaidMcActionContext::garr_fire_resistance;
        creators["mc baron geddon fire resistance"] = &RaidMcActionContext::baron_geddon_fire_resistance;
        creators["mc move from group"] = &RaidMcActionContext::check_should_move_from_group;
        creators["mc move from baron geddon"] = &RaidMcActionContext::move_from_baron_geddon;
        creators["mc shazzrah move away"] = &RaidMcActionContext::shazzrah_move_away;
        creators["mc sulfuron harbinger fire resistance"] = &RaidMcActionContext::sulfuron_harbinger_fire_resistance;
        creators["mc golemagg fire resistance"] = &RaidMcActionContext::golemagg_fire_resistance;
        creators["mc golemagg mark boss"] = &RaidMcActionContext::golemagg_mark_boss;
        creators["mc golemagg main tank attack golemagg"] = &RaidMcActionContext::golemagg_main_tank_attack_golemagg;
        creators["mc golemagg assist tank attack core rager"] = &RaidMcActionContext::golemagg_assist_tank_attack_core_rager;
        creators["mc majordomo shadow resistance"] = &RaidMcActionContext::majordomo_shadow_resistance;
        creators["mc ragnaros fire resistance"] = &RaidMcActionContext::ragnaros_fire_resistance;
        creators["mc core hound mark"] = &RaidMcActionContext::core_hound_mark;

        //By leewheel 2026年7月12日
        // 自定义Boss: Smolder
        creators["mc smolder fire resistance"] = &RaidMcActionContext::smolder_fire_resistance;
        creators["mc smolder avoid flame tsunami"] = &RaidMcActionContext::smolder_avoid_flame_tsunami;
        creators["mc smolder fear ward"] = &RaidMcActionContext::smolder_fear_ward;

        // 自定义Boss: Hazzrash
        creators["mc hazzrash evocation"] = &RaidMcActionContext::hazzrash_evocation;
        creators["mc hazzrash ranged spread"] = &RaidMcActionContext::hazzrash_ranged_spread;
        //End By leewheel
    }

private:
    static Action* lucifron_shadow_resistance(PlayerbotAI* botAI) { return new BossShadowResistanceAction(botAI, "lucifron"); }
    static Action* magmadar_fire_resistance(PlayerbotAI* botAI) { return new BossFireResistanceAction(botAI, "magmadar"); }
    static Action* gehennas_shadow_resistance(PlayerbotAI* botAI) { return new BossShadowResistanceAction(botAI, "gehennas"); }
    static Action* garr_fire_resistance(PlayerbotAI* botAI) { return new BossFireResistanceAction(botAI, "garr"); }
    static Action* baron_geddon_fire_resistance(PlayerbotAI* botAI) { return new BossFireResistanceAction(botAI, "baron geddon"); }
    static Action* check_should_move_from_group(PlayerbotAI* botAI) { return new McMoveFromGroupAction(botAI); }
    static Action* move_from_baron_geddon(PlayerbotAI* botAI) { return new McMoveFromBaronGeddonAction(botAI); }
    static Action* shazzrah_move_away(PlayerbotAI* botAI) { return new McShazzrahMoveAwayAction(botAI); }
    static Action* sulfuron_harbinger_fire_resistance(PlayerbotAI* botAI) { return new BossFireResistanceAction(botAI, "sulfuron harbinger"); }
    static Action* golemagg_fire_resistance(PlayerbotAI* botAI) { return new BossFireResistanceAction(botAI, "golemagg the incinerator"); }
    static Action* golemagg_mark_boss(PlayerbotAI* botAI) { return new McGolemaggMarkBossAction(botAI); }
    static Action* golemagg_main_tank_attack_golemagg(PlayerbotAI* botAI) { return new McGolemaggMainTankAttackGolemaggAction(botAI); }
    static Action* golemagg_assist_tank_attack_core_rager(PlayerbotAI* botAI) { return new McGolemaggAssistTankAttackCoreRagerAction(botAI); }
    static Action* majordomo_shadow_resistance(PlayerbotAI* botAI) { return new BossShadowResistanceAction(botAI, "majordomo executus"); }
    static Action* ragnaros_fire_resistance(PlayerbotAI* botAI) { return new BossFireResistanceAction(botAI, "ragnaros"); }
    static Action* core_hound_mark(PlayerbotAI* botAI) { return new McCoreHoundMarkAction(botAI); }

    //By leewheel 2026年7月12日
    // 自定义Boss: Smolder
    static Action* smolder_fire_resistance(PlayerbotAI* botAI) { return new BossFireResistanceAction(botAI, MoltenCoreHelpers::BOSS_NAME_SMOLDER); }
    static Action* smolder_avoid_flame_tsunami(PlayerbotAI* botAI) { return new McSmolderAvoidFlameTsunamiAction(botAI); }
    static Action* smolder_fear_ward(PlayerbotAI* botAI) { return new McSmolderFearWardAction(botAI); }

    // 自定义Boss: Hazzrash
    static Action* hazzrash_evocation(PlayerbotAI* botAI) { return new McHazzrashEvocationAction(botAI); }
    static Action* hazzrash_ranged_spread(PlayerbotAI* botAI) { return new McHazzrashRangedSpreadAction(botAI); }
    //End By leewheel
};

#endif
