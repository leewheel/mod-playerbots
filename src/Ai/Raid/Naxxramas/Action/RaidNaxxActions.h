/*
 * 版权所有 (C) 2026 Leewheel
 * 
 * 文件功能：纳克萨玛斯团队副本动作头文件
 * 定义纳克萨玛斯副本各Boss战斗的动作类
 * 
 * By Leewheel 2026-02-14
 */

// RaidNaxxActions.h
#ifndef _PLAYERBOT_RAIDNAXXACTIONS_H_
#define _PLAYERBOT_RAIDNAXXACTIONS_H_

#include "Action.h"
#include "AttackAction.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"

class PlayerbotAI;

// ==========================================
// 帕奇维克动作
// ==========================================

// 将副坦克定位在帕奇维克侧面以应对憎恨打击
class NaxxPatchwerkOffTankPositionAction : public MovementAction
{
public:
    NaxxPatchwerkOffTankPositionAction(PlayerbotAI* botAI) 
        : MovementAction(botAI, "naxx patchwerk offtank position") {}
    bool Execute(Event event) override;
};

// 在狂乱或狂暴期间触发燃烧阶段（使用所有冷却技能）
class NaxxPatchwerkBurnPhaseAction : public AttackAction
{
public:
    NaxxPatchwerkBurnPhaseAction(PlayerbotAI* botAI) 
        : AttackAction(botAI, "naxx patchwerk burn") {}
    bool Execute(Event event) override;
};

// ==========================================
// 格罗布鲁斯动作
// ==========================================

// 当变异注射即将爆炸时将机器人移动到房间边缘
class NaxxGrobbulusMoveToEdgeAction : public MovementAction
{
public:
    NaxxGrobbulusMoveToEdgeAction(PlayerbotAI* botAI) 
        : MovementAction(botAI, "naxx grobbulus move to edge") {}
    bool Execute(Event event) override;
};

// 将机器人移离毒云区域
class NaxxGrobbulusAvoidPoisonCloudAction : public MovementAction
{
public:
    NaxxGrobbulusAvoidPoisonCloudAction(PlayerbotAI* botAI) 
        : MovementAction(botAI, "naxx grobbulus avoid poison cloud") {}
    bool Execute(Event event) override;
};

// ==========================================
// 阿努布雷坎动作
// ==========================================

// 在蝗虫群期间分散站位，与其他团队成员保持距离
class NaxxAnubRekhanSpreadOutAction : public MovementAction
{
public:
    NaxxAnubRekhanSpreadOutAction(PlayerbotAI* botAI) 
        : MovementAction(botAI, "naxx anubrekhan spread out") {}
    bool Execute(Event event) override;
};

// 优先攻击地穴守卫小怪
class NaxxAnubRekhanAttackCryptGuardAction : public AttackAction
{
public:
    NaxxAnubRekhanAttackCryptGuardAction(PlayerbotAI* botAI) 
        : AttackAction(botAI, "naxx anubrekhan attack crypt guard") {}
    bool Execute(Event event) override;
};

// ==========================================
// 费尔莉娜动作
// ==========================================

// 优先攻击纳克萨玛斯崇拜者以移除狂乱
class NaxxFaerlinaAttackWorshipperAction : public AttackAction
{
public:
    NaxxFaerlinaAttackWorshipperAction(PlayerbotAI* botAI) 
        : AttackAction(botAI, "naxx faerlina attack worshipper") {}
    bool Execute(Event event) override;
};

// ==========================================
// 迈克斯纳动作
// ==========================================

// 在被蛛网缠绕时等待救援
class NaxxMaexxnaWaitForRescueAction : public Action
{
public:
    NaxxMaexxnaWaitForRescueAction(PlayerbotAI* botAI) 
        : Action(botAI, "naxx maexxna wait for rescue") {}
    bool Execute(Event event) override;
};

// 在毒性冲击时停止施法
class NaxxMaexxnaStopCastingAction : public Action
{
public:
    NaxxMaexxnaStopCastingAction(PlayerbotAI* botAI) 
        : Action(botAI, "naxx maexxna stop casting") {}
    bool Execute(Event event) override;
};

// 攻击小蜘蛛
class NaxxMaexxnaAttackSpiderlingAction : public AttackAction
{
public:
    NaxxMaexxnaAttackSpiderlingAction(PlayerbotAI* botAI) 
        : AttackAction(botAI, "naxx maexxna attack spiderling") {}
    bool Execute(Event event) override;
};

// ==========================================
// 诺斯动作
// ==========================================

// 诺斯传送时切换到小怪
class NaxxNothSwitchToAddsAction : public AttackAction
{
public:
    NaxxNothSwitchToAddsAction(PlayerbotAI* botAI) 
        : AttackAction(botAI, "naxx noth switch to adds") {}
    bool Execute(Event event) override;
};

// 诺斯重新出现时切换回Boss
class NaxxNothSwitchToBossAction : public AttackAction
{
public:
    NaxxNothSwitchToBossAction(PlayerbotAI* botAI) 
        : AttackAction(botAI, "naxx noth switch to boss") {}
    bool Execute(Event event) override;
};

// ==========================================
// 洛欧塞布动作
// ==========================================

// 死灵光环激活时停止治疗
class NaxxLoathebStopHealingAction : public Action
{
public:
    NaxxLoathebStopHealingAction(PlayerbotAI* botAI) 
        : Action(botAI, "naxx loatheb stop healing") {}
    bool Execute(Event event) override;
};

// 死灵光环消失时爆发治疗
class NaxxLoathebBurstHealingAction : public Action
{
public:
    NaxxLoathebBurstHealingAction(PlayerbotAI* botAI) 
        : Action(botAI, "naxx loatheb burst healing") {}
    bool Execute(Event event) override;
};

// ==========================================
// 格拉斯动作
// ==========================================

// 风筝僵尸食尸鬼远离Boss
class NaxxGluthKiteZombiesAction : public MovementAction
{
public:
    NaxxGluthKiteZombiesAction(PlayerbotAI* botAI) 
        : MovementAction(botAI, "naxx gluth kite zombies") {}
    bool Execute(Event event) override;
};

// 击杀僵尸食尸鬼
class NaxxGluthKillZombiesAction : public AttackAction
{
public:
    NaxxGluthKillZombiesAction(PlayerbotAI* botAI) 
        : AttackAction(botAI, "naxx gluth kill zombies") {}
    bool Execute(Event event) override;
};

// ==========================================
// 海根动作
// ==========================================

// 执行海根跳舞机制（移动到安全区域）
class NaxxHeiganDanceAction : public MovementAction
{
public:
    NaxxHeiganDanceAction(PlayerbotAI* botAI) 
        : MovementAction(botAI, "naxx heigan dance") {}
    bool Execute(Event event) override;
};

// ==========================================
// 四骑士动作
// ==========================================

// 切换到标记层数最少的骑士
class NaxxFourHorsemenSwitchAction : public AttackAction
{
public:
    NaxxFourHorsemenSwitchAction(PlayerbotAI* botAI) 
        : AttackAction(botAI, "naxx four horsemen switch") {}
    bool Execute(Event event) override;
};

// ==========================================
// 塔迪乌斯动作
// ==========================================

// 移动到对应的极性区域
class NaxxThaddiusMoveToPolarityAction : public MovementAction
{
public:
    NaxxThaddiusMoveToPolarityAction(PlayerbotAI* botAI) 
        : MovementAction(botAI, "naxx thaddius move to polarity") {}
    bool Execute(Event event) override;
};

// ==========================================
// 戈提克动作
// ==========================================

// 第一阶段攻击生者侧小怪
class NaxxGothikAttackLivingSideAction : public AttackAction
{
public:
    NaxxGothikAttackLivingSideAction(PlayerbotAI* botAI) 
        : AttackAction(botAI, "naxx gothik attack living side") {}
    bool Execute(Event event) override;
};

// 第一阶段攻击亡者侧小怪
class NaxxGothikAttackDeadSideAction : public AttackAction
{
public:
    NaxxGothikAttackDeadSideAction(PlayerbotAI* botAI) 
        : AttackAction(botAI, "naxx gothik attack dead side") {}
    bool Execute(Event event) override;
};

// 第二阶段攻击戈提克
class NaxxGothikAttackBossAction : public AttackAction
{
public:
    NaxxGothikAttackBossAction(PlayerbotAI* botAI) 
        : AttackAction(botAI, "naxx gothik attack boss") {}
    bool Execute(Event event) override;
};

// ==========================================
// 拉祖维奥斯动作
// ==========================================

// 精神控制死亡骑士学徒
class NaxxRazuviousMindControlAction : public Action
{
public:
    NaxxRazuviousMindControlAction(PlayerbotAI* botAI) 
        : Action(botAI, "naxx razuvious mind control") {}
    bool Execute(Event event) override;
};

// 使用学徒嘲讽Boss
class NaxxRazuviousTauntAction : public Action
{
public:
    NaxxRazuviousTauntAction(PlayerbotAI* botAI) 
        : Action(botAI, "naxx razuvious taunt") {}
    bool Execute(Event event) override;
};

// 切换控制到另一个学徒
class NaxxRazuviousSwitchControlAction : public Action
{
public:
    NaxxRazuviousSwitchControlAction(PlayerbotAI* botAI) 
        : Action(botAI, "naxx razuvious switch control") {}
    bool Execute(Event event) override;
};

// ==========================================
// 萨菲隆动作
// ==========================================

// 躲在冰块后面
class NaxxSapphironHideBehindIceBlockAction : public MovementAction
{
public:
    NaxxSapphironHideBehindIceBlockAction(PlayerbotAI* botAI) 
        : MovementAction(botAI, "naxx sapphiron hide behind ice block") {}
    bool Execute(Event event) override;
};

// ==========================================
// 克尔苏加德动作
// ==========================================

// 第一阶段攻击小怪
class NaxxKelThuzadAttackAddsAction : public AttackAction
{
public:
    NaxxKelThuzadAttackAddsAction(PlayerbotAI* botAI) 
        : AttackAction(botAI, "naxx kelthuzad attack adds") {}
    bool Execute(Event event) override;
};

// 第二阶段攻击Boss
class NaxxKelThuzadAttackBossAction : public AttackAction
{
public:
    NaxxKelThuzadAttackBossAction(PlayerbotAI* botAI) 
        : AttackAction(botAI, "naxx kelthuzad attack boss") {}
    bool Execute(Event event) override;
};

// 第三阶段攻击守护者
class NaxxKelThuzadAttackGuardianAction : public AttackAction
{
public:
    NaxxKelThuzadAttackGuardianAction(PlayerbotAI* botAI) 
        : AttackAction(botAI, "naxx kelthuzad attack guardian") {}
    bool Execute(Event event) override;
};

// 被冰霜冲击冻结时等待解冻
class NaxxKelThuzadWaitForUnfreezeAction : public Action
{
public:
    NaxxKelThuzadWaitForUnfreezeAction(PlayerbotAI* botAI) 
        : Action(botAI, "naxx kelthuzad wait for unfreeze") {}
    bool Execute(Event event) override;
};

// 远离暗影裂隙
class NaxxKelThuzadMoveFissureAction : public MovementAction
{
public:
    NaxxKelThuzadMoveFissureAction(PlayerbotAI* botAI) 
        : MovementAction(botAI, "naxx kelthuzad move fissure") {}
    bool Execute(Event event) override;
};

#endif

// By Leewheel 2026-02-14
