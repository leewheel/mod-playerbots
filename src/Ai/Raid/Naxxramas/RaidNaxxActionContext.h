/*
 * 版权所有 (C) 2026 Leewheel
 * 
 * 文件功能：纳克萨玛斯团队副本动作上下文头文件
 * 定义纳克萨玛斯副本动作的注册和创建器
 * 
 * By Leewheel 2026-02-14
 */

#ifndef _PLAYERBOT_RAIDNAXXACTIONS_CONTEXT_H
#define _PLAYERBOT_RAIDNAXXACTIONS_CONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "Action/RaidNaxxActions.h"

class RaidNaxxActionContext : public NamedObjectContext<Action>
{
public:
    RaidNaxxActionContext()
    {
        // 帕奇维克
        creators["naxx patchwerk offtank position"] = &RaidNaxxActionContext::naxx_patchwerk_offtank_position;
        creators["naxx patchwerk burn"] = &RaidNaxxActionContext::naxx_patchwerk_burn;
        
        // 格罗布鲁斯
        creators["naxx grobbulus move to edge"] = &RaidNaxxActionContext::naxx_grobbulus_move_to_edge;
        creators["naxx grobbulus avoid poison cloud"] = &RaidNaxxActionContext::naxx_grobbulus_avoid_poison_cloud;
        
        // 阿努布雷坎
        creators["naxx anubrekhan spread out"] = &RaidNaxxActionContext::naxx_anubrekhan_spread_out;
        creators["naxx anubrekhan attack crypt guard"] = &RaidNaxxActionContext::naxx_anubrekhan_attack_crypt_guard;
        
        // 费尔莉娜
        creators["naxx faerlina attack worshipper"] = &RaidNaxxActionContext::naxx_faerlina_attack_worshipper;
        
        // 迈克斯纳
        creators["naxx maexxna wait for rescue"] = &RaidNaxxActionContext::naxx_maexxna_wait_for_rescue;
        creators["naxx maexxna stop casting"] = &RaidNaxxActionContext::naxx_maexxna_stop_casting;
        creators["naxx maexxna attack spiderling"] = &RaidNaxxActionContext::naxx_maexxna_attack_spiderling;
        
        // 诺斯
        creators["naxx noth switch to adds"] = &RaidNaxxActionContext::naxx_noth_switch_to_adds;
        creators["naxx noth switch to boss"] = &RaidNaxxActionContext::naxx_noth_switch_to_boss;
        
        // 洛欧塞布
        creators["naxx loatheb stop healing"] = &RaidNaxxActionContext::naxx_loatheb_stop_healing;
        creators["naxx loatheb burst healing"] = &RaidNaxxActionContext::naxx_loatheb_burst_healing;
        
        // 格拉斯
        creators["naxx gluth kite zombies"] = &RaidNaxxActionContext::naxx_gluth_kite_zombies;
        creators["naxx gluth kill zombies"] = &RaidNaxxActionContext::naxx_gluth_kill_zombies;
        
        // 海根
        creators["naxx heigan dance"] = &RaidNaxxActionContext::naxx_heigan_dance;
        
        // 四骑士
        creators["naxx four horsemen switch"] = &RaidNaxxActionContext::naxx_four_horsemen_switch;
        
        // 塔迪乌斯
        creators["naxx thaddius move to polarity"] = &RaidNaxxActionContext::naxx_thaddius_move_to_polarity;
        
        // 戈提克
        creators["naxx gothik attack living side"] = &RaidNaxxActionContext::naxx_gothik_attack_living_side;
        creators["naxx gothik attack dead side"] = &RaidNaxxActionContext::naxx_gothik_attack_dead_side;
        creators["naxx gothik attack boss"] = &RaidNaxxActionContext::naxx_gothik_attack_boss;
        
        // 拉祖维奥斯
        creators["naxx razuvious mind control"] = &RaidNaxxActionContext::naxx_razuvious_mind_control;
        creators["naxx razuvious taunt"] = &RaidNaxxActionContext::naxx_razuvious_taunt;
        creators["naxx razuvious switch control"] = &RaidNaxxActionContext::naxx_razuvious_switch_control;
        
        // 萨菲隆
        creators["naxx sapphiron hide behind ice block"] = &RaidNaxxActionContext::naxx_sapphiron_hide_behind_ice_block;
        
        // 克尔苏加德
        creators["naxx kelthuzad attack adds"] = &RaidNaxxActionContext::naxx_kelthuzad_attack_adds;
        creators["naxx kelthuzad attack boss"] = &RaidNaxxActionContext::naxx_kelthuzad_attack_boss;
        creators["naxx kelthuzad attack guardian"] = &RaidNaxxActionContext::naxx_kelthuzad_attack_guardian;
        creators["naxx kelthuzad wait for unfreeze"] = &RaidNaxxActionContext::naxx_kelthuzad_wait_for_unfreeze;
        creators["naxx kelthuzad move fissure"] = &RaidNaxxActionContext::naxx_kelthuzad_move_fissure;
    }

private:
    // 帕奇维克
    static Action* naxx_patchwerk_offtank_position(PlayerbotAI* ai) { return new NaxxPatchwerkOffTankPositionAction(ai); }
    static Action* naxx_patchwerk_burn(PlayerbotAI* ai) { return new NaxxPatchwerkBurnPhaseAction(ai); }
    
    // 格罗布鲁斯
    static Action* naxx_grobbulus_move_to_edge(PlayerbotAI* ai) { return new NaxxGrobbulusMoveToEdgeAction(ai); }
    static Action* naxx_grobbulus_avoid_poison_cloud(PlayerbotAI* ai) { return new NaxxGrobbulusAvoidPoisonCloudAction(ai); }
    
    // 阿努布雷坎
    static Action* naxx_anubrekhan_spread_out(PlayerbotAI* ai) { return new NaxxAnubRekhanSpreadOutAction(ai); }
    static Action* naxx_anubrekhan_attack_crypt_guard(PlayerbotAI* ai) { return new NaxxAnubRekhanAttackCryptGuardAction(ai); }
    
    // 费尔莉娜
    static Action* naxx_faerlina_attack_worshipper(PlayerbotAI* ai) { return new NaxxFaerlinaAttackWorshipperAction(ai); }
    
    // 迈克斯纳
    static Action* naxx_maexxna_wait_for_rescue(PlayerbotAI* ai) { return new NaxxMaexxnaWaitForRescueAction(ai); }
    static Action* naxx_maexxna_stop_casting(PlayerbotAI* ai) { return new NaxxMaexxnaStopCastingAction(ai); }
    static Action* naxx_maexxna_attack_spiderling(PlayerbotAI* ai) { return new NaxxMaexxnaAttackSpiderlingAction(ai); }
    
    // 诺斯
    static Action* naxx_noth_switch_to_adds(PlayerbotAI* ai) { return new NaxxNothSwitchToAddsAction(ai); }
    static Action* naxx_noth_switch_to_boss(PlayerbotAI* ai) { return new NaxxNothSwitchToBossAction(ai); }
    
    // 洛欧塞布
    static Action* naxx_loatheb_stop_healing(PlayerbotAI* ai) { return new NaxxLoathebStopHealingAction(ai); }
    static Action* naxx_loatheb_burst_healing(PlayerbotAI* ai) { return new NaxxLoathebBurstHealingAction(ai); }
    
    // 格拉斯
    static Action* naxx_gluth_kite_zombies(PlayerbotAI* ai) { return new NaxxGluthKiteZombiesAction(ai); }
    static Action* naxx_gluth_kill_zombies(PlayerbotAI* ai) { return new NaxxGluthKillZombiesAction(ai); }
    
    // 海根
    static Action* naxx_heigan_dance(PlayerbotAI* ai) { return new NaxxHeiganDanceAction(ai); }
    
    // 四骑士
    static Action* naxx_four_horsemen_switch(PlayerbotAI* ai) { return new NaxxFourHorsemenSwitchAction(ai); }
    
    // 塔迪乌斯
    static Action* naxx_thaddius_move_to_polarity(PlayerbotAI* ai) { return new NaxxThaddiusMoveToPolarityAction(ai); }
    
    // 戈提克
    static Action* naxx_gothik_attack_living_side(PlayerbotAI* ai) { return new NaxxGothikAttackLivingSideAction(ai); }
    static Action* naxx_gothik_attack_dead_side(PlayerbotAI* ai) { return new NaxxGothikAttackDeadSideAction(ai); }
    static Action* naxx_gothik_attack_boss(PlayerbotAI* ai) { return new NaxxGothikAttackBossAction(ai); }
    
    // 拉祖维奥斯
    static Action* naxx_razuvious_mind_control(PlayerbotAI* ai) { return new NaxxRazuviousMindControlAction(ai); }
    static Action* naxx_razuvious_taunt(PlayerbotAI* ai) { return new NaxxRazuviousTauntAction(ai); }
    static Action* naxx_razuvious_switch_control(PlayerbotAI* ai) { return new NaxxRazuviousSwitchControlAction(ai); }
    
    // 萨菲隆
    static Action* naxx_sapphiron_hide_behind_ice_block(PlayerbotAI* ai) { return new NaxxSapphironHideBehindIceBlockAction(ai); }
    
    // 克尔苏加德
    static Action* naxx_kelthuzad_attack_adds(PlayerbotAI* ai) { return new NaxxKelThuzadAttackAddsAction(ai); }
    static Action* naxx_kelthuzad_attack_boss(PlayerbotAI* ai) { return new NaxxKelThuzadAttackBossAction(ai); }
    static Action* naxx_kelthuzad_attack_guardian(PlayerbotAI* ai) { return new NaxxKelThuzadAttackGuardianAction(ai); }
    static Action* naxx_kelthuzad_wait_for_unfreeze(PlayerbotAI* ai) { return new NaxxKelThuzadWaitForUnfreezeAction(ai); }
    static Action* naxx_kelthuzad_move_fissure(PlayerbotAI* ai) { return new NaxxKelThuzadMoveFissureAction(ai); }
};

#endif

// By Leewheel 2026-02-14
