//By leewheel 2026-07-08
/*
 * 太阳之井高地 (Sunwell Plateau) 动作上下文
 * 作者: leewheel
 * 将动作名称映射到对应的Action类
 */
//End By leewheel

#ifndef PLAYERBOTS_SWPACTIONCONTEXT_H
#define PLAYERBOTS_SWPACTIONCONTEXT_H

#include "NamedObjectContext.h"
#include "SWPActions.h"

class RaidSunwellPlateauActionContext : public NamedObjectContext<Action>
{
public:
    RaidSunwellPlateauActionContext()
    {
        // 通用
        creators["sunwell erase timers and trackers"] =
            &RaidSunwellPlateauActionContext::sunwell_erase_timers_and_trackers;

        // 卡雷苟斯
        creators["kalecgos misdirect boss to main tank"] =
            &RaidSunwellPlateauActionContext::kalecgos_misdirect_boss_to_main_tank;
        creators["kalecgos tanks position boss"] =
            &RaidSunwellPlateauActionContext::kalecgos_tanks_position_boss;
        creators["kalecgos ranged disperse"] =
            &RaidSunwellPlateauActionContext::kalecgos_ranged_disperse;
        creators["kalecgos enter spectral realm"] =
            &RaidSunwellPlateauActionContext::kalecgos_enter_spectral_realm;
        creators["kalecgos attack sathrovarr"] =
            &RaidSunwellPlateauActionContext::kalecgos_attack_sathrovarr;
        creators["kalecgos health sync"] =
            &RaidSunwellPlateauActionContext::kalecgos_health_sync;
        creators["kalecgos manage arcane buffet"] =
            &RaidSunwellPlateauActionContext::kalecgos_manage_arcane_buffet;
        creators["kalecgos dispelling curse"] =
            &RaidSunwellPlateauActionContext::kalecgos_dispelling_curse;
        creators["kalecgos dispelling frost breath"] =
            &RaidSunwellPlateauActionContext::kalecgos_dispelling_frost_breath;

        // 布鲁塔卢斯
        creators["brutallus misdirect boss to main tank"] =
            &RaidSunwellPlateauActionContext::brutallus_misdirect_boss_to_main_tank;
        creators["brutallus tanks position boss"] =
            &RaidSunwellPlateauActionContext::brutallus_tanks_position_boss;
        creators["brutallus soak meteor slash"] =
            &RaidSunwellPlateauActionContext::brutallus_soak_meteor_slash;
        creators["brutallus burn move away"] =
            &RaidSunwellPlateauActionContext::brutallus_burn_move_away;

        // 菲米丝
        creators["felmyst misdirect boss to main tank"] =
            &RaidSunwellPlateauActionContext::felmyst_misdirect_boss_to_main_tank;
        creators["felmyst tanks position boss"] =
            &RaidSunwellPlateauActionContext::felmyst_tanks_position_boss;
        creators["felmyst disperse from gas nova"] =
            &RaidSunwellPlateauActionContext::felmyst_disperse_from_gas_nova;
        creators["felmyst avoid encapsulate"] =
            &RaidSunwellPlateauActionContext::felmyst_avoid_encapsulate;
        creators["felmyst flight phase spread"] =
            &RaidSunwellPlateauActionContext::felmyst_flight_phase_spread;
        creators["felmyst manage phase timer"] =
            &RaidSunwellPlateauActionContext::felmyst_manage_phase_timer;

        // 艾瑞达双子
        creators["eredar twins misdirect boss to tanks"] =
            &RaidSunwellPlateauActionContext::eredar_twins_misdirect_boss_to_tanks;
        creators["eredar twins assign kill order"] =
            &RaidSunwellPlateauActionContext::eredar_twins_assign_kill_order;
        creators["eredar twins move to flame source"] =
            &RaidSunwellPlateauActionContext::eredar_twins_move_to_flame_source;
        creators["eredar twins move to shadow source"] =
            &RaidSunwellPlateauActionContext::eredar_twins_move_to_shadow_source;
        creators["eredar twins avoid conflagration"] =
            &RaidSunwellPlateauActionContext::eredar_twins_avoid_conflagration;

        // 穆鲁
        creators["muru misdirect boss to main tank"] =
            &RaidSunwellPlateauActionContext::muru_misdirect_boss_to_main_tank;
        creators["muru handle adds"] =
            &RaidSunwellPlateauActionContext::muru_handle_adds;
        creators["muru handle void sentinel"] =
            &RaidSunwellPlateauActionContext::muru_handle_void_sentinel;
        creators["muru avoid darkness"] =
            &RaidSunwellPlateauActionContext::muru_avoid_darkness;
        creators["muru entropius phase"] =
            &RaidSunwellPlateauActionContext::muru_entropius_phase;

        // 基尔加丹
        creators["kil'jaeden misdirect to tank"] =
            &RaidSunwellPlateauActionContext::kiljaeden_misdirect_to_tank;
        creators["kil'jaeden avoid darkness"] =
            &RaidSunwellPlateauActionContext::kiljaeden_avoid_darkness;
        creators["kil'jaeden avoid armageddon"] =
            &RaidSunwellPlateauActionContext::kiljaeden_avoid_armageddon;
        creators["kil'jaeden handle sinister reflection"] =
            &RaidSunwellPlateauActionContext::kiljaeden_handle_sinister_reflection;
        creators["kil'jaeden handle shield orb"] =
            &RaidSunwellPlateauActionContext::kiljaeden_handle_shield_orb;
        creators["kil'jaeden manage phase"] =
            &RaidSunwellPlateauActionContext::kiljaeden_manage_phase;
        creators["kil'jaeden ranged disperse"] =
            &RaidSunwellPlateauActionContext::kiljaeden_ranged_disperse;
    }

private:
    // 通用
    static Action* sunwell_erase_timers_and_trackers(
        PlayerbotAI* botAI) { return new SunwellEraseTimersAndTrackersAction(botAI); }

    // 卡雷苟斯
    static Action* kalecgos_misdirect_boss_to_main_tank(
        PlayerbotAI* botAI) { return new KalecgosMisdirectBossToMainTankAction(botAI); }
    static Action* kalecgos_tanks_position_boss(
        PlayerbotAI* botAI) { return new KalecgosTanksPositionBossAction(botAI); }
    static Action* kalecgos_ranged_disperse(
        PlayerbotAI* botAI) { return new KalecgosRangedDisperseAction(botAI); }
    static Action* kalecgos_enter_spectral_realm(
        PlayerbotAI* botAI) { return new KalecgosEnterSpectralRealmAction(botAI); }
    static Action* kalecgos_attack_sathrovarr(
        PlayerbotAI* botAI) { return new KalecgosAttackSathrovarrAction(botAI); }
    static Action* kalecgos_health_sync(
        PlayerbotAI* botAI) { return new KalecgosHealthSyncAction(botAI); }
    static Action* kalecgos_manage_arcane_buffet(
        PlayerbotAI* botAI) { return new KalecgosManageArcaneBuffetAction(botAI); }
    static Action* kalecgos_dispelling_curse(
        PlayerbotAI* botAI) { return new KalecgosDispellingCurseAction(botAI); }
    static Action* kalecgos_dispelling_frost_breath(
        PlayerbotAI* botAI) { return new KalecgosDispellingFrostBreathAction(botAI); }

    // 布鲁塔卢斯
    static Action* brutallus_misdirect_boss_to_main_tank(
        PlayerbotAI* botAI) { return new BrutallusMisdirectBossToMainTankAction(botAI); }
    static Action* brutallus_tanks_position_boss(
        PlayerbotAI* botAI) { return new BrutallusTanksPositionBossAction(botAI); }
    static Action* brutallus_soak_meteor_slash(
        PlayerbotAI* botAI) { return new BrutallusSoakMeteorSlashAction(botAI); }
    static Action* brutallus_burn_move_away(
        PlayerbotAI* botAI) { return new BrutallusBurnMoveAwayAction(botAI); }

    // 菲米丝
    static Action* felmyst_misdirect_boss_to_main_tank(
        PlayerbotAI* botAI) { return new FelmystMisdirectBossToMainTankAction(botAI); }
    static Action* felmyst_tanks_position_boss(
        PlayerbotAI* botAI) { return new FelmystTanksPositionBossAction(botAI); }
    static Action* felmyst_disperse_from_gas_nova(
        PlayerbotAI* botAI) { return new FelmystDisperseFromGasNovaAction(botAI); }
    static Action* felmyst_avoid_encapsulate(
        PlayerbotAI* botAI) { return new FelmystAvoidEncapsulateAction(botAI); }
    static Action* felmyst_flight_phase_spread(
        PlayerbotAI* botAI) { return new FelmystFlightPhaseSpreadAction(botAI); }
    static Action* felmyst_manage_phase_timer(
        PlayerbotAI* botAI) { return new FelmystManagePhaseTimerAction(botAI); }

    // 艾瑞达双子
    static Action* eredar_twins_misdirect_boss_to_tanks(
        PlayerbotAI* botAI) { return new EredarTwinsMisdirectBossToTanksAction(botAI); }
    static Action* eredar_twins_assign_kill_order(
        PlayerbotAI* botAI) { return new EredarTwinsAssignKillOrderAction(botAI); }
    static Action* eredar_twins_move_to_flame_source(
        PlayerbotAI* botAI) { return new EredarTwinsMoveToFlameSourceAction(botAI); }
    static Action* eredar_twins_move_to_shadow_source(
        PlayerbotAI* botAI) { return new EredarTwinsMoveToShadowSourceAction(botAI); }
    static Action* eredar_twins_avoid_conflagration(
        PlayerbotAI* botAI) { return new EredarTwinsAvoidConflagrationAction(botAI); }

    // 穆鲁
    static Action* muru_misdirect_boss_to_main_tank(
        PlayerbotAI* botAI) { return new MuruMisdirectBossToMainTankAction(botAI); }
    static Action* muru_handle_adds(
        PlayerbotAI* botAI) { return new MuruHandleAddsAction(botAI); }
    static Action* muru_handle_void_sentinel(
        PlayerbotAI* botAI) { return new MuruHandleVoidSentinelAction(botAI); }
    static Action* muru_avoid_darkness(
        PlayerbotAI* botAI) { return new MuruAvoidDarknessAction(botAI); }
    static Action* muru_entropius_phase(
        PlayerbotAI* botAI) { return new MuruEntropiusPhaseAction(botAI); }

    // 基尔加丹
    static Action* kiljaeden_misdirect_to_tank(
        PlayerbotAI* botAI) { return new KiljaedenMisdirectToTankAction(botAI); }
    static Action* kiljaeden_avoid_darkness(
        PlayerbotAI* botAI) { return new KiljaedenAvoidDarknessAction(botAI); }
    static Action* kiljaeden_avoid_armageddon(
        PlayerbotAI* botAI) { return new KiljaedenAvoidArmageddonAction(botAI); }
    static Action* kiljaeden_handle_sinister_reflection(
        PlayerbotAI* botAI) { return new KiljaedenHandleSinisterReflectionAction(botAI); }
    static Action* kiljaeden_handle_shield_orb(
        PlayerbotAI* botAI) { return new KiljaedenHandleShieldOrbAction(botAI); }
    static Action* kiljaeden_manage_phase(
        PlayerbotAI* botAI) { return new KiljaedenManagePhaseAction(botAI); }
    static Action* kiljaeden_ranged_disperse(
        PlayerbotAI* botAI) { return new KiljaedenRangedDisperseAction(botAI); }
};

#endif
