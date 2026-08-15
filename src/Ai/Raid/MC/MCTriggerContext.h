/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MCTRIGGERCONTEXT_H
#define PLAYERBOTS_MCTRIGGERCONTEXT_H

#include "BossAuraTriggers.h"
#include "MCTriggers.h"
#include "MCHelpers.h"
#include "NamedObjectContext.h"

class RaidMcTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidMcTriggerContext()
    {
        creators["mc lucifron shadow resistance"] = &RaidMcTriggerContext::lucifron_shadow_resistance;
        creators["mc magmadar fire resistance"] = &RaidMcTriggerContext::magmadar_fire_resistance;
        creators["mc gehennas shadow resistance"] = &RaidMcTriggerContext::gehennas_shadow_resistance;
        creators["mc garr fire resistance"] = &RaidMcTriggerContext::garr_fire_resistance;
        creators["mc baron geddon fire resistance"] = &RaidMcTriggerContext::baron_geddon_fire_resistance;
        creators["mc living bomb debuff"] = &RaidMcTriggerContext::living_bomb_debuff;
        creators["mc baron geddon inferno"] = &RaidMcTriggerContext::baron_geddon_inferno;
        creators["mc shazzrah ranged"] = &RaidMcTriggerContext::shazzrah_ranged;
        creators["mc sulfuron harbinger fire resistance"] = &RaidMcTriggerContext::sulfuron_harbinger_fire_resistance;
        creators["mc golemagg fire resistance"] = &RaidMcTriggerContext::golemagg_fire_resistance;
        creators["mc golemagg mark boss"] = &RaidMcTriggerContext::golemagg_mark_boss;
        creators["mc golemagg is main tank"] = &RaidMcTriggerContext::golemagg_is_main_tank;
        creators["mc golemagg is assist tank"] = &RaidMcTriggerContext::golemagg_is_assist_tank;
        creators["mc majordomo shadow resistance"] = &RaidMcTriggerContext::majordomo_shadow_resistance;
        creators["mc ragnaros fire resistance"] = &RaidMcTriggerContext::ragnaros_fire_resistance;
        creators["mc core hound mark"] = &RaidMcTriggerContext::core_hound_mark;

        //By leewheel 2026年7月12日
        // 自定义Boss: Smolder
        creators["mc smolder fire resistance"] = &RaidMcTriggerContext::smolder_fire_resistance;
        creators["mc smolder flame tsunami"] = &RaidMcTriggerContext::smolder_flame_tsunami;
        creators["mc smolder fear ward"] = &RaidMcTriggerContext::smolder_fear_ward;

        // 自定义Boss: Hazzrash
        creators["mc hazzrash evocation"] = &RaidMcTriggerContext::hazzrash_evocation;
        creators["mc hazzrash ranged spread"] = &RaidMcTriggerContext::hazzrash_ranged_spread;
        //End By leewheel
    }

private:
    static Trigger* lucifron_shadow_resistance(PlayerbotAI* botAI) { return new BossShadowResistanceTrigger(botAI, "lucifron"); }
    static Trigger* magmadar_fire_resistance(PlayerbotAI* botAI) { return new BossFireResistanceTrigger(botAI, "magmadar"); }
    static Trigger* gehennas_shadow_resistance(PlayerbotAI* botAI) { return new BossShadowResistanceTrigger(botAI, "gehennas"); }
    static Trigger* garr_fire_resistance(PlayerbotAI* botAI) { return new BossFireResistanceTrigger(botAI, "garr"); }
    static Trigger* baron_geddon_fire_resistance(PlayerbotAI* botAI) { return new BossFireResistanceTrigger(botAI, "baron geddon"); }
    static Trigger* living_bomb_debuff(PlayerbotAI* botAI) { return new McLivingBombDebuffTrigger(botAI); }
    static Trigger* baron_geddon_inferno(PlayerbotAI* botAI) { return new McBaronGeddonInfernoTrigger(botAI); }
    static Trigger* shazzrah_ranged(PlayerbotAI* botAI) { return new McShazzrahRangedTrigger(botAI); }
    static Trigger* sulfuron_harbinger_fire_resistance(PlayerbotAI* botAI) { return new BossFireResistanceTrigger(botAI, "sulfuron harbinger"); }
    static Trigger* golemagg_fire_resistance(PlayerbotAI* botAI) { return new BossFireResistanceTrigger(botAI, "golemagg the incinerator"); }
    static Trigger* golemagg_mark_boss(PlayerbotAI* botAI) { return new McGolemaggMarkBossTrigger(botAI); }
    static Trigger* golemagg_is_main_tank(PlayerbotAI* botAI) { return new McGolemaggIsMainTankTrigger(botAI); }
    static Trigger* golemagg_is_assist_tank(PlayerbotAI* botAI) { return new McGolemaggIsAssistTankTrigger(botAI); }
    static Trigger* majordomo_shadow_resistance(PlayerbotAI* botAI) { return new BossShadowResistanceTrigger(botAI, "majordomo executus"); }
    static Trigger* ragnaros_fire_resistance(PlayerbotAI* botAI) { return new BossFireResistanceTrigger(botAI, "ragnaros"); }
    static Trigger* core_hound_mark(PlayerbotAI* botAI) { return new McCoreHoundMarkTrigger(botAI); }

    //By leewheel 2026年7月12日
    // 自定义Boss: Smolder
    static Trigger* smolder_fire_resistance(PlayerbotAI* botAI) { return new BossFireResistanceTrigger(botAI, MoltenCoreHelpers::BOSS_NAME_SMOLDER); }
    static Trigger* smolder_flame_tsunami(PlayerbotAI* botAI) { return new McSmolderFlameTsunamiTrigger(botAI); }
    static Trigger* smolder_fear_ward(PlayerbotAI* botAI) { return new McSmolderFearWardTrigger(botAI); }

    // 自定义Boss: Hazzrash
    static Trigger* hazzrash_evocation(PlayerbotAI* botAI) { return new McHazzrashEvocationTrigger(botAI); }
    static Trigger* hazzrash_ranged_spread(PlayerbotAI* botAI) { return new McHazzrashRangedSpreadTrigger(botAI); }
    //End By leewheel
};

#endif
