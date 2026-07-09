//By leewheel 2026-07-08
/*
 * 太阳之井高地 (Sunwell Plateau) 触发器上下文
 * 作者: leewheel
 * 将触发器名称映射到对应的Trigger类
 */
//End By leewheel

#ifndef PLAYERBOTS_SWPTRIGGERCONTEXT_H
#define PLAYERBOTS_SWPTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "SWPTriggers.h"

class RaidSunwellPlateauTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidSunwellPlateauTriggerContext()
    {
        // 通用
        creators["sunwell bot is not in combat"] =
            &RaidSunwellPlateauTriggerContext::sunwell_bot_is_not_in_combat;

        // 卡雷苟斯
        creators["kalecgos pulling boss"] =
            &RaidSunwellPlateauTriggerContext::kalecgos_pulling_boss;
        creators["kalecgos boss engaged by tanks"] =
            &RaidSunwellPlateauTriggerContext::kalecgos_boss_engaged_by_tanks;
        creators["kalecgos boss engaged by ranged"] =
            &RaidSunwellPlateauTriggerContext::kalecgos_boss_engaged_by_ranged;
        creators["kalecgos need enter spectral realm"] =
            &RaidSunwellPlateauTriggerContext::kalecgos_need_enter_spectral_realm;
        creators["kalecgos in spectral realm"] =
            &RaidSunwellPlateauTriggerContext::kalecgos_in_spectral_realm;
        creators["kalecgos health not synced"] =
            &RaidSunwellPlateauTriggerContext::kalecgos_health_not_synced;
        creators["kalecgos need arcane buffet reset"] =
            &RaidSunwellPlateauTriggerContext::kalecgos_need_arcane_buffet_reset;
        creators["kalecgos curse of boundless agony"] =
            &RaidSunwellPlateauTriggerContext::kalecgos_curse_of_boundless_agony;
        creators["kalecgos frost breath on tank"] =
            &RaidSunwellPlateauTriggerContext::kalecgos_frost_breath_on_tank;

        // 布鲁塔卢斯
        creators["brutallus pulling boss"] =
            &RaidSunwellPlateauTriggerContext::brutallus_pulling_boss;
        creators["brutallus boss engaged by tanks"] =
            &RaidSunwellPlateauTriggerContext::brutallus_boss_engaged_by_tanks;
        creators["brutallus casting meteor slash"] =
            &RaidSunwellPlateauTriggerContext::brutallus_casting_meteor_slash;
        creators["brutallus bot has burn"] =
            &RaidSunwellPlateauTriggerContext::brutallus_bot_has_burn;

        // 菲米丝
        creators["felmyst pulling boss"] =
            &RaidSunwellPlateauTriggerContext::felmyst_pulling_boss;
        creators["felmyst boss engaged by tanks"] =
            &RaidSunwellPlateauTriggerContext::felmyst_boss_engaged_by_tanks;
        creators["felmyst casting gas nova"] =
            &RaidSunwellPlateauTriggerContext::felmyst_casting_gas_nova;
        creators["felmyst casting encapsulate"] =
            &RaidSunwellPlateauTriggerContext::felmyst_casting_encapsulate;
        creators["felmyst in flight phase"] =
            &RaidSunwellPlateauTriggerContext::felmyst_in_flight_phase;
        creators["felmyst need to manage phase timer"] =
            &RaidSunwellPlateauTriggerContext::felmyst_need_to_manage_phase_timer;

        // 艾瑞达双子
        creators["eredar twins pulling bosses"] =
            &RaidSunwellPlateauTriggerContext::eredar_twins_pulling_bosses;
        creators["eredar twins determining kill order"] =
            &RaidSunwellPlateauTriggerContext::eredar_twins_determining_kill_order;
        creators["eredar twins bot has dark touched"] =
            &RaidSunwellPlateauTriggerContext::eredar_twins_bot_has_dark_touched;
        creators["eredar twins bot has flame touched"] =
            &RaidSunwellPlateauTriggerContext::eredar_twins_bot_has_flame_touched;
        creators["eredar twins bot has conflagration"] =
            &RaidSunwellPlateauTriggerContext::eredar_twins_bot_has_conflagration;

        // 穆鲁
        creators["muru entropius spawned"] =
            &RaidSunwellPlateauTriggerContext::muru_entropius_spawned;
        creators["muru adds spawned"] =
            &RaidSunwellPlateauTriggerContext::muru_adds_spawned;
        creators["muru void sentinel spawned"] =
            &RaidSunwellPlateauTriggerContext::muru_void_sentinel_spawned;
        creators["muru casting darkness"] =
            &RaidSunwellPlateauTriggerContext::muru_casting_darkness;
        creators["muru entropius phase"] =
            &RaidSunwellPlateauTriggerContext::muru_entropius_phase;

        // 基尔加丹
        creators["kil'jaeden pulling boss"] =
            &RaidSunwellPlateauTriggerContext::kiljaeden_pulling_boss;
        creators["kil'jaeden casting darkness of souls"] =
            &RaidSunwellPlateauTriggerContext::kiljaeden_casting_darkness_of_souls;
        creators["kil'jaeden casting armageddon"] =
            &RaidSunwellPlateauTriggerContext::kiljaeden_casting_armageddon;
        creators["kil'jaeden spawned sinister reflection"] =
            &RaidSunwellPlateauTriggerContext::kiljaeden_spawned_sinister_reflection;
        creators["kil'jaeden shield orb spawned"] =
            &RaidSunwellPlateauTriggerContext::kiljaeden_shield_orb_spawned;
        creators["kil'jaeden need to manage phase"] =
            &RaidSunwellPlateauTriggerContext::kiljaeden_need_to_manage_phase;
        creators["kil'jaeden boss engaged by ranged"] =
            &RaidSunwellPlateauTriggerContext::kiljaeden_boss_engaged_by_ranged;
    }

private:
    // 通用
    static Trigger* sunwell_bot_is_not_in_combat(
        PlayerbotAI* botAI) { return new SunwellBotIsNotInCombatTrigger(botAI); }

    // 卡雷苟斯
    static Trigger* kalecgos_pulling_boss(
        PlayerbotAI* botAI) { return new KalecgosPullingBossTrigger(botAI); }
    static Trigger* kalecgos_boss_engaged_by_tanks(
        PlayerbotAI* botAI) { return new KalecgosBossEngagedByTanksTrigger(botAI); }
    static Trigger* kalecgos_boss_engaged_by_ranged(
        PlayerbotAI* botAI) { return new KalecgosBossEngagedByRangedTrigger(botAI); }
    static Trigger* kalecgos_need_enter_spectral_realm(
        PlayerbotAI* botAI) { return new KalecgosNeedEnterSpectralRealmTrigger(botAI); }
    static Trigger* kalecgos_in_spectral_realm(
        PlayerbotAI* botAI) { return new KalecgosInSpectralRealmTrigger(botAI); }
    static Trigger* kalecgos_health_not_synced(
        PlayerbotAI* botAI) { return new KalecgosHealthNotSyncedTrigger(botAI); }
    static Trigger* kalecgos_need_arcane_buffet_reset(
        PlayerbotAI* botAI) { return new KalecgosNeedArcaneBuffetResetTrigger(botAI); }
    static Trigger* kalecgos_curse_of_boundless_agony(
        PlayerbotAI* botAI) { return new KalecgosCurseOfBoundlessAgonyTrigger(botAI); }
    static Trigger* kalecgos_frost_breath_on_tank(
        PlayerbotAI* botAI) { return new KalecgosFrostBreathOnTankTrigger(botAI); }

    // 布鲁塔卢斯
    static Trigger* brutallus_pulling_boss(
        PlayerbotAI* botAI) { return new BrutallusPullingBossTrigger(botAI); }
    static Trigger* brutallus_boss_engaged_by_tanks(
        PlayerbotAI* botAI) { return new BrutallusBossEngagedByTanksTrigger(botAI); }
    static Trigger* brutallus_casting_meteor_slash(
        PlayerbotAI* botAI) { return new BrutallusCastingMeteorSlashTrigger(botAI); }
    static Trigger* brutallus_bot_has_burn(
        PlayerbotAI* botAI) { return new BrutallusBotHasBurnTrigger(botAI); }

    // 菲米丝
    static Trigger* felmyst_pulling_boss(
        PlayerbotAI* botAI) { return new FelmystPullingBossTrigger(botAI); }
    static Trigger* felmyst_boss_engaged_by_tanks(
        PlayerbotAI* botAI) { return new FelmystBossEngagedByTanksTrigger(botAI); }
    static Trigger* felmyst_casting_gas_nova(
        PlayerbotAI* botAI) { return new FelmystCastingGasNovaTrigger(botAI); }
    static Trigger* felmyst_casting_encapsulate(
        PlayerbotAI* botAI) { return new FelmystCastingEncapsulateTrigger(botAI); }
    static Trigger* felmyst_in_flight_phase(
        PlayerbotAI* botAI) { return new FelmystInFlightPhaseTrigger(botAI); }
    static Trigger* felmyst_need_to_manage_phase_timer(
        PlayerbotAI* botAI) { return new FelmystNeedToManagePhaseTimerTrigger(botAI); }

    // 艾瑞达双子
    static Trigger* eredar_twins_pulling_bosses(
        PlayerbotAI* botAI) { return new EredarTwinsPullingBossesTrigger(botAI); }
    static Trigger* eredar_twins_determining_kill_order(
        PlayerbotAI* botAI) { return new EredarTwinsDeterminingKillOrderTrigger(botAI); }
    static Trigger* eredar_twins_bot_has_dark_touched(
        PlayerbotAI* botAI) { return new EredarTwinsBotHasDarkTouchedTrigger(botAI); }
    static Trigger* eredar_twins_bot_has_flame_touched(
        PlayerbotAI* botAI) { return new EredarTwinsBotHasFlameTouchedTrigger(botAI); }
    static Trigger* eredar_twins_bot_has_conflagration(
        PlayerbotAI* botAI) { return new EredarTwinsBotHasConflagrationTrigger(botAI); }

    // 穆鲁
    static Trigger* muru_entropius_spawned(
        PlayerbotAI* botAI) { return new MuruEntropiusSpawnedTrigger(botAI); }
    static Trigger* muru_adds_spawned(
        PlayerbotAI* botAI) { return new MuruAddsSpawnedTrigger(botAI); }
    static Trigger* muru_void_sentinel_spawned(
        PlayerbotAI* botAI) { return new MuruVoidSentinelSpawnedTrigger(botAI); }
    static Trigger* muru_casting_darkness(
        PlayerbotAI* botAI) { return new MuruCastingDarknessTrigger(botAI); }
    static Trigger* muru_entropius_phase(
        PlayerbotAI* botAI) { return new MuruEntropiusPhaseTrigger(botAI); }

    // 基尔加丹
    static Trigger* kiljaeden_pulling_boss(
        PlayerbotAI* botAI) { return new KiljaedenPullingBossTrigger(botAI); }
    static Trigger* kiljaeden_casting_darkness_of_souls(
        PlayerbotAI* botAI) { return new KiljaedenCastingDarknessOfSoulsTrigger(botAI); }
    static Trigger* kiljaeden_casting_armageddon(
        PlayerbotAI* botAI) { return new KiljaedenCastingArmageddonTrigger(botAI); }
    static Trigger* kiljaeden_spawned_sinister_reflection(
        PlayerbotAI* botAI) { return new KiljaedenSpawnedSinisterReflectionTrigger(botAI); }
    static Trigger* kiljaeden_shield_orb_spawned(
        PlayerbotAI* botAI) { return new KiljaedenShieldOrbSpawnedTrigger(botAI); }
    static Trigger* kiljaeden_need_to_manage_phase(
        PlayerbotAI* botAI) { return new KiljaedenNeedToManagePhaseTrigger(botAI); }
    static Trigger* kiljaeden_boss_engaged_by_ranged(
        PlayerbotAI* botAI) { return new KiljaedenBossEngagedByRangedTrigger(botAI); }
};

#endif
