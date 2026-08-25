/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BWLTRIGGERCONTEXT_H
#define PLAYERBOTS_BWLTRIGGERCONTEXT_H

#include "BWLHelpers.h"
#include "BWLTriggers.h"
#include "BossAuraTriggers.h"
#include "NamedObjectContext.h"

class RaidBwlTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidBwlTriggerContext()
    {
        creators["bwl suppression device"] = &RaidBwlTriggerContext::bwl_suppression_device;

        creators["bwl razorgore fire resistance"] = &RaidBwlTriggerContext::bwl_razorgore_fire_resistance_trigger;
        creators["bwl razorgore not mind controlled"] = &RaidBwlTriggerContext::bwl_razorgore_not_mind_controlled;

        creators["bwl vaelastrasz fire resistance"] = &RaidBwlTriggerContext::bwl_vaelastrasz_fire_resistance_trigger;
        creators["bwl vaelastrasz positioning"] = &RaidBwlTriggerContext::bwl_vaelastrasz_positioning;
        creators["bwl vaelastrasz burning adrenaline"] = &RaidBwlTriggerContext::bwl_vaelastrasz_burning_adrenaline;

        creators["bwl broodlord fire resistance"] = &RaidBwlTriggerContext::bwl_broodlord_fire_resistance_trigger;

        creators["bwl firemaw fire resistance"] = &RaidBwlTriggerContext::bwl_firemaw_fire_resistance_trigger;
        creators["bwl flamegor fire resistance"] = &RaidBwlTriggerContext::bwl_flamegor_fire_resistance_trigger;

        creators["bwl affliction bronze"] = &RaidBwlTriggerContext::bwl_affliction_bronze;
        creators["bwl wild magic"] = &RaidBwlTriggerContext::bwl_wild_magic;
        creators["bwl nefarian fear ward"] = &RaidBwlTriggerContext::bwl_nefarian_fear_ward;
        creators["bwl death talon wyrmguard tank"] = &RaidBwlTriggerContext::bwl_death_talon_wyrmguard_tank;
        creators["bwl death talon wyrmguard ranged"] = &RaidBwlTriggerContext::bwl_death_talon_wyrmguard_ranged;

        //By leewheel 2026年7月12日
        // 自定义Boss: Valthorax
        creators["bwl valthorax frost resistance"] = &RaidBwlTriggerContext::bwl_valthorax_frost_resistance;
        creators["bwl valthorax shadow resistance"] = &RaidBwlTriggerContext::bwl_valthorax_shadow_resistance;
        creators["bwl valthorax frost bomb"] = &RaidBwlTriggerContext::bwl_valthorax_frost_bomb;
        creators["bwl valthorax vabomination"] = &RaidBwlTriggerContext::bwl_valthorax_vabomination;
        creators["bwl valthorax adds"] = &RaidBwlTriggerContext::bwl_valthorax_adds;
    }

private:
    static Trigger* bwl_suppression_device(PlayerbotAI* ai) { return new BwlSuppressionDeviceTrigger(ai); }
    static Trigger* bwl_razorgore_fire_resistance_trigger(PlayerbotAI* ai) { return new BossFireResistanceTrigger(ai, "razorgore the untamed"); }
    static Trigger* bwl_razorgore_not_mind_controlled(PlayerbotAI* ai) { return new BwlRazorgoreNotMindControlledTrigger(ai); }
    static Trigger* bwl_vaelastrasz_fire_resistance_trigger(PlayerbotAI* ai) { return new BossFireResistanceTrigger(ai, "vaelastrasz the corrupt"); }
    static Trigger* bwl_vaelastrasz_positioning(PlayerbotAI* ai) { return new BwlVaelastraszPositioningTrigger(ai); }
    static Trigger* bwl_vaelastrasz_burning_adrenaline(PlayerbotAI* ai) { return new BwlVaelastraszBurningAdrenalineTrigger(ai); }
    static Trigger* bwl_broodlord_fire_resistance_trigger(PlayerbotAI* ai) { return new BossFireResistanceTrigger(ai, "broodlord lashlayer"); }
    static Trigger* bwl_firemaw_fire_resistance_trigger(PlayerbotAI* ai) { return new BossFireResistanceTrigger(ai, "firemaw"); }
    static Trigger* bwl_flamegor_fire_resistance_trigger(PlayerbotAI* ai) { return new BossFireResistanceTrigger(ai, "flamegor"); }
    static Trigger* bwl_affliction_bronze(PlayerbotAI* ai) { return new BwlAfflictionBronzeTrigger(ai); }
    static Trigger* bwl_wild_magic(PlayerbotAI* ai) { return new BwlWildMagicTrigger(ai); }
    static Trigger* bwl_nefarian_fear_ward(PlayerbotAI* ai) { return new BwlNefarianFearWardTrigger(ai); }
    static Trigger* bwl_death_talon_wyrmguard_tank(PlayerbotAI* ai) { return new BwlDeathTalonWyrmguardTankTrigger(ai); }
    static Trigger* bwl_death_talon_wyrmguard_ranged(PlayerbotAI* ai) { return new BwlDeathTalonWyrmguardRangedTrigger(ai); }
    // Custom Boss: Valthorax
    static Trigger* bwl_valthorax_frost_resistance(PlayerbotAI* ai) { return new BossFrostResistanceTrigger(ai, BlackwingLairHelpers::BOSS_NAME_VALTHORAX); }
    static Trigger* bwl_valthorax_shadow_resistance(PlayerbotAI* ai) { return new BossShadowResistanceTrigger(ai, BlackwingLairHelpers::BOSS_NAME_VALTHORAX); }
    static Trigger* bwl_valthorax_frost_bomb(PlayerbotAI* ai) { return new BwlValthoraxFrostBombTrigger(ai); }
    static Trigger* bwl_valthorax_vabomination(PlayerbotAI* ai) { return new BwlValthoraxVabominationTrigger(ai); }
    static Trigger* bwl_valthorax_adds(PlayerbotAI* ai) { return new BwlValthoraxAddsTrigger(ai); }
    //End By leewheel
};

#endif
