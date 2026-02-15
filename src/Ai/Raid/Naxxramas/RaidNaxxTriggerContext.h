/*
 * 版权所有 (C) 2026 Leewheel
 * 
 * 文件功能：纳克萨玛斯团队副本触发器上下文头文件
 * 定义纳克萨玛斯副本触发器的注册和创建器
 * 
 * By Leewheel 2026-02-14
 */

#ifndef _PLAYERBOT_RAIDNAXXTRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDNAXXTRIGGERCONTEXT_H

#include "AiObjectContext.h"
#include "NamedObjectContext.h"
#include "Trigger/RaidNaxxTriggers.h"

class RaidNaxxTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidNaxxTriggerContext()
    {
        // 帕奇维克
        creators["naxx patchwerk combat"] = &RaidNaxxTriggerContext::naxx_patchwerk_combat;
        creators["naxx patchwerk frenzy"] = &RaidNaxxTriggerContext::naxx_patchwerk_frenzy;
        creators["naxx patchwerk berserk"] = &RaidNaxxTriggerContext::naxx_patchwerk_berserk;
        creators["naxx patchwerk offtank position"] = &RaidNaxxTriggerContext::naxx_patchwerk_offtank_position;
        
        // 格罗布鲁斯
        creators["naxx grobbulus mutating injection"] = &RaidNaxxTriggerContext::naxx_grobbulus_mutating_injection;
        creators["naxx grobbulus poison cloud"] = &RaidNaxxTriggerContext::naxx_grobbulus_poison_cloud;
        
        // 阿努布雷坎
        creators["naxx anubrekhan locust swarm"] = &RaidNaxxTriggerContext::naxx_anubrekhan_locust_swarm;
        
        // 费尔莉娜
        creators["naxx faerlina frenzy"] = &RaidNaxxTriggerContext::naxx_faerlina_frenzy;
        
        // 迈克斯纳
        creators["naxx maexxna web spray"] = &RaidNaxxTriggerContext::naxx_maexxna_web_spray;
        creators["naxx maexxna web wrap"] = &RaidNaxxTriggerContext::naxx_maexxna_web_wrap;
        creators["naxx maexxna poison shock"] = &RaidNaxxTriggerContext::naxx_maexxna_poison_shock;
        
        // 诺斯
        creators["naxx noth teleport"] = &RaidNaxxTriggerContext::naxx_noth_teleport;
        creators["naxx noth reappear"] = &RaidNaxxTriggerContext::naxx_noth_reappear;
        
        // 洛欧塞布
        creators["naxx loatheb necrotic aura active"] = &RaidNaxxTriggerContext::naxx_loatheb_necrotic_aura_active;
        creators["naxx loatheb necrotic aura inactive"] = &RaidNaxxTriggerContext::naxx_loatheb_necrotic_aura_inactive;
        
        // 格拉斯
        creators["naxx gluth decimate"] = &RaidNaxxTriggerContext::naxx_gluth_decimate;
        creators["naxx gluth zombie chow nearby"] = &RaidNaxxTriggerContext::naxx_gluth_zombie_chow_nearby;
        
        // 海根
        creators["naxx heigan dance"] = &RaidNaxxTriggerContext::naxx_heigan_dance;
        
        // 四骑士
        creators["naxx four horsemen mark high"] = &RaidNaxxTriggerContext::naxx_four_horsemen_mark_high;
        
        // 塔迪乌斯
        creators["naxx thaddius polarity shift"] = &RaidNaxxTriggerContext::naxx_thaddius_polarity_shift;
        
        // 戈提克
        creators["naxx gothik phase one"] = &RaidNaxxTriggerContext::naxx_gothik_phase_one;
        creators["naxx gothik phase two"] = &RaidNaxxTriggerContext::naxx_gothik_phase_two;
        
        // 拉祖维奥斯
        creators["naxx razuvious need control"] = &RaidNaxxTriggerContext::naxx_razuvious_need_control;
        creators["naxx razuvious control expiring"] = &RaidNaxxTriggerContext::naxx_razuvious_control_expiring;
        
        // 萨菲隆
        creators["naxx sapphiron air phase"] = &RaidNaxxTriggerContext::naxx_sapphiron_air_phase;
        
        // 克尔苏加德
        creators["naxx kelthuzad phase one"] = &RaidNaxxTriggerContext::naxx_kelthuzad_phase_one;
        creators["naxx kelthuzad phase two"] = &RaidNaxxTriggerContext::naxx_kelthuzad_phase_two;
        creators["naxx kelthuzad phase three"] = &RaidNaxxTriggerContext::naxx_kelthuzad_phase_three;
        creators["naxx kelthuzad frost blast"] = &RaidNaxxTriggerContext::naxx_kelthuzad_frost_blast;
        creators["naxx kelthuzad shadow fissure"] = &RaidNaxxTriggerContext::naxx_kelthuzad_shadow_fissure;
    }

private:
    // 帕奇维克
    static Trigger* naxx_patchwerk_combat(PlayerbotAI* ai) { return new NaxxPatchwerkCombatTrigger(ai); }
    static Trigger* naxx_patchwerk_frenzy(PlayerbotAI* ai) { return new NaxxPatchwerkFrenzyTrigger(ai); }
    static Trigger* naxx_patchwerk_berserk(PlayerbotAI* ai) { return new NaxxPatchwerkBerserkTrigger(ai); }
    static Trigger* naxx_patchwerk_offtank_position(PlayerbotAI* ai) { return new NaxxPatchwerkOffTankPositionTrigger(ai); }
    
    // 格罗布鲁斯
    static Trigger* naxx_grobbulus_mutating_injection(PlayerbotAI* ai) { return new NaxxGrobbulusMutatingInjectionTrigger(ai); }
    static Trigger* naxx_grobbulus_poison_cloud(PlayerbotAI* ai) { return new NaxxGrobbulusPoisonCloudTrigger(ai); }
    
    // 阿努布雷坎
    static Trigger* naxx_anubrekhan_locust_swarm(PlayerbotAI* ai) { return new NaxxAnubRekhanLocustSwarmTrigger(ai); }
    
    // 费尔莉娜
    static Trigger* naxx_faerlina_frenzy(PlayerbotAI* ai) { return new NaxxFaerlinaFrenzyTrigger(ai); }
    
    // 迈克斯纳
    static Trigger* naxx_maexxna_web_spray(PlayerbotAI* ai) { return new NaxxMaexxnaWebSprayTrigger(ai); }
    static Trigger* naxx_maexxna_web_wrap(PlayerbotAI* ai) { return new NaxxMaexxnaWebWrapTrigger(ai); }
    static Trigger* naxx_maexxna_poison_shock(PlayerbotAI* ai) { return new NaxxMaexxnaPoisonShockTrigger(ai); }
    
    // 诺斯
    static Trigger* naxx_noth_teleport(PlayerbotAI* ai) { return new NaxxNothTeleportTrigger(ai); }
    static Trigger* naxx_noth_reappear(PlayerbotAI* ai) { return new NaxxNothReappearTrigger(ai); }
    
    // 洛欧塞布
    static Trigger* naxx_loatheb_necrotic_aura_active(PlayerbotAI* ai) { return new NaxxLoathebNecroticAuraActiveTrigger(ai); }
    static Trigger* naxx_loatheb_necrotic_aura_inactive(PlayerbotAI* ai) { return new NaxxLoathebNecroticAuraInactiveTrigger(ai); }
    
    // 格拉斯
    static Trigger* naxx_gluth_decimate(PlayerbotAI* ai) { return new NaxxGluthDecimateTrigger(ai); }
    static Trigger* naxx_gluth_zombie_chow_nearby(PlayerbotAI* ai) { return new NaxxGluthZombieChowNearbyTrigger(ai); }
    
    // 海根
    static Trigger* naxx_heigan_dance(PlayerbotAI* ai) { return new NaxxHeiganDanceTrigger(ai); }
    
    // 四骑士
    static Trigger* naxx_four_horsemen_mark_high(PlayerbotAI* ai) { return new NaxxFourHorsemenMarkHighTrigger(ai); }
    
    // 塔迪乌斯
    static Trigger* naxx_thaddius_polarity_shift(PlayerbotAI* ai) { return new NaxxThaddiusPolarityShiftTrigger(ai); }
    
    // 戈提克
    static Trigger* naxx_gothik_phase_one(PlayerbotAI* ai) { return new NaxxGothikPhaseOneTrigger(ai); }
    static Trigger* naxx_gothik_phase_two(PlayerbotAI* ai) { return new NaxxGothikPhaseTwoTrigger(ai); }
    
    // 拉祖维奥斯
    static Trigger* naxx_razuvious_need_control(PlayerbotAI* ai) { return new NaxxRazuviousNeedControlTrigger(ai); }
    static Trigger* naxx_razuvious_control_expiring(PlayerbotAI* ai) { return new NaxxRazuviousControlExpiringTrigger(ai); }
    
    // 萨菲隆
    static Trigger* naxx_sapphiron_air_phase(PlayerbotAI* ai) { return new NaxxSapphironAirPhaseTrigger(ai); }
    
    // 克尔苏加德
    static Trigger* naxx_kelthuzad_phase_one(PlayerbotAI* ai) { return new NaxxKelThuzadPhaseOneTrigger(ai); }
    static Trigger* naxx_kelthuzad_phase_two(PlayerbotAI* ai) { return new NaxxKelThuzadPhaseTwoTrigger(ai); }
    static Trigger* naxx_kelthuzad_phase_three(PlayerbotAI* ai) { return new NaxxKelThuzadPhaseThreeTrigger(ai); }
    static Trigger* naxx_kelthuzad_frost_blast(PlayerbotAI* ai) { return new NaxxKelThuzadFrostBlastTrigger(ai); }
    static Trigger* naxx_kelthuzad_shadow_fissure(PlayerbotAI* ai) { return new NaxxKelThuzadShadowFissureTrigger(ai); }
};

#endif

// By Leewheel 2026-02-14
