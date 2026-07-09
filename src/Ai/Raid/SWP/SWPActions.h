//By leewheel 2026-07-08
/*
 * 太阳之井高地 (Sunwell Plateau) 动作声明
 * 作者: leewheel
 * 每个 Trigger 对应一个或多个 Action 类
 */
//End By leewheel

#ifndef PLAYERBOTS_SWPACTIONS_H
#define PLAYERBOTS_SWPACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "SWPHelpers.h"

// ===== 通用 =====

class SunwellEraseTimersAndTrackersAction : public Action
{
public:
    SunwellEraseTimersAndTrackersAction(
        PlayerbotAI* botAI) : Action(botAI, "sunwell erase timers and trackers") {}
    bool Execute(Event event) override;
};

// ===== 入口小怪 (Entrance Trash) =====

class SwpTrashTankPullAction : public AttackAction
{
public:
    SwpTrashTankPullAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "swp trash tank pull") {}
    bool Execute(Event event) override;
};

class SwpTrashGroupHoldAction : public MovementAction
{
public:
    SwpTrashGroupHoldAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "swp trash group hold") {}
    bool Execute(Event event) override;
};

class SwpWaitForDeadPartyMembersAction : public Action
{
public:
    SwpWaitForDeadPartyMembersAction(
        PlayerbotAI* botAI) : Action(botAI, "swp wait for dead party members") {}
    bool Execute(Event event) override;
};

// ===== 卡雷苟斯 (Kalecgos) =====

class KalecgosMisdirectBossToMainTankAction : public AttackAction
{
public:
    KalecgosMisdirectBossToMainTankAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "kalecgos misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class KalecgosTanksPositionBossAction : public AttackAction
{
public:
    KalecgosTanksPositionBossAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "kalecgos tanks position boss") {}
    bool Execute(Event event) override;
};

class KalecgosRangedDisperseAction : public MovementAction
{
public:
    KalecgosRangedDisperseAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "kalecgos ranged disperse") {}
    bool Execute(Event event) override;
};

class KalecgosEnterSpectralRealmAction : public MovementAction
{
public:
    KalecgosEnterSpectralRealmAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "kalecgos enter spectral realm") {}
    bool Execute(Event event) override;
};

class KalecgosAttackSathrovarrAction : public AttackAction
{
public:
    KalecgosAttackSathrovarrAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "kalecgos attack sathrovarr") {}
    bool Execute(Event event) override;
};

// 卡雷苟斯内外场血量同步动作
// 当内外场血量差异过大时控制DPS节奏
class KalecgosHealthSyncAction : public Action
{
public:
    KalecgosHealthSyncAction(
        PlayerbotAI* botAI) : Action(botAI, "kalecgos health sync") {}
    bool Execute(Event event) override;
};

// 奥术冲击层数管理动作（进入幽灵领域刷新层数）
class KalecgosManageArcaneBuffetAction : public MovementAction
{
public:
    KalecgosManageArcaneBuffetAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "kalecgos manage arcane buffet") {}
    bool Execute(Event event) override;
};

// 驱散无尽痛苦诅咒动作
class KalecgosDispellingCurseAction : public Action
{
public:
    KalecgosDispellingCurseAction(
        PlayerbotAI* botAI) : Action(botAI, "kalecgos dispelling curse") {}
    bool Execute(Event event) override;
};

// 驱散冰霜吐息动作（从主坦身上驱散）
class KalecgosDispellingFrostBreathAction : public Action
{
public:
    KalecgosDispellingFrostBreathAction(
        PlayerbotAI* botAI) : Action(botAI, "kalecgos dispelling frost breath") {}
    bool Execute(Event event) override;
};

// ===== 布鲁塔卢斯 (Brutallus) =====

class BrutallusMisdirectBossToMainTankAction : public AttackAction
{
public:
    BrutallusMisdirectBossToMainTankAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "brutallus misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class BrutallusTanksPositionBossAction : public AttackAction
{
public:
    BrutallusTanksPositionBossAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "brutallus tanks position boss") {}
    bool Execute(Event event) override;
};

class BrutallusSoakMeteorSlashAction : public MovementAction
{
public:
    BrutallusSoakMeteorSlashAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "brutallus soak meteor slash") {}
    bool Execute(Event event) override;
};

class BrutallusBurnMoveAwayAction : public MovementAction
{
public:
    BrutallusBurnMoveAwayAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "brutallus burn move away") {}
    bool Execute(Event event) override;
};

// ===== 菲米丝 (Felmyst) =====

class FelmystMisdirectBossToMainTankAction : public AttackAction
{
public:
    FelmystMisdirectBossToMainTankAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "felmyst misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class FelmystTanksPositionBossAction : public AttackAction
{
public:
    FelmystTanksPositionBossAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "felmyst tanks position boss") {}
    bool Execute(Event event) override;
};

class FelmystDisperseFromGasNovaAction : public MovementAction
{
public:
    FelmystDisperseFromGasNovaAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "felmyst disperse from gas nova") {}
    bool Execute(Event event) override;
};

class FelmystAvoidEncapsulateAction : public MovementAction
{
public:
    FelmystAvoidEncapsulateAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "felmyst avoid encapsulate") {}
    bool Execute(Event event) override;
};

class FelmystFlightPhaseSpreadAction : public MovementAction
{
public:
    FelmystFlightPhaseSpreadAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "felmyst flight phase spread") {}
    bool Execute(Event event) override;
};

class FelmystManagePhaseTimerAction : public Action
{
public:
    FelmystManagePhaseTimerAction(
        PlayerbotAI* botAI) : Action(botAI, "felmyst manage phase timer") {}
    bool Execute(Event event) override;
};

// ===== 艾瑞达双子 (Eredar Twins) =====

class EredarTwinsMisdirectBossToTanksAction : public AttackAction
{
public:
    EredarTwinsMisdirectBossToTanksAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "eredar twins misdirect boss to tanks") {}
    bool Execute(Event event) override;
};

class EredarTwinsAssignKillOrderAction : public AttackAction
{
public:
    EredarTwinsAssignKillOrderAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "eredar twins assign kill order") {}
    bool Execute(Event event) override;
};

class EredarTwinsMoveToFlameSourceAction : public MovementAction
{
public:
    EredarTwinsMoveToFlameSourceAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "eredar twins move to flame source") {}
    bool Execute(Event event) override;
};

class EredarTwinsMoveToShadowSourceAction : public MovementAction
{
public:
    EredarTwinsMoveToShadowSourceAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "eredar twins move to shadow source") {}
    bool Execute(Event event) override;
};

class EredarTwinsAvoidConflagrationAction : public MovementAction
{
public:
    EredarTwinsAvoidConflagrationAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "eredar twins avoid conflagration") {}
    bool Execute(Event event) override;
};

// ===== 穆鲁 (Muru) =====

class MuruMisdirectBossToMainTankAction : public AttackAction
{
public:
    MuruMisdirectBossToMainTankAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "muru misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class MuruHandleAddsAction : public AttackAction
{
public:
    MuruHandleAddsAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "muru handle adds") {}
    bool Execute(Event event) override;
};

class MuruHandleVoidSentinelAction : public AttackAction
{
public:
    MuruHandleVoidSentinelAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "muru handle void sentinel") {}
    bool Execute(Event event) override;
};

class MuruAvoidDarknessAction : public MovementAction
{
public:
    MuruAvoidDarknessAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "muru avoid darkness") {}
    bool Execute(Event event) override;
};

class MuruEntropiusPhaseAction : public AttackAction
{
public:
    MuruEntropiusPhaseAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "muru entropius phase") {}
    bool Execute(Event event) override;
};

// ===== 基尔加丹 (Kil'jaeden) =====

class KiljaedenMisdirectToTankAction : public AttackAction
{
public:
    KiljaedenMisdirectToTankAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "kiljaeden misdirect to tank") {}
    bool Execute(Event event) override;
};

class KiljaedenAvoidDarknessAction : public MovementAction
{
public:
    KiljaedenAvoidDarknessAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "kiljaeden avoid darkness") {}
    bool Execute(Event event) override;
};

class KiljaedenAvoidArmageddonAction : public MovementAction
{
public:
    KiljaedenAvoidArmageddonAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "kiljaeden avoid armageddon") {}
    bool Execute(Event event) override;
};

class KiljaedenHandleSinisterReflectionAction : public AttackAction
{
public:
    KiljaedenHandleSinisterReflectionAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "kiljaeden handle sinister reflection") {}
    bool Execute(Event event) override;
};

class KiljaedenHandleShieldOrbAction : public AttackAction
{
public:
    KiljaedenHandleShieldOrbAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "kiljaeden handle shield orb") {}
    bool Execute(Event event) override;
};

class KiljaedenManagePhaseAction : public Action
{
public:
    KiljaedenManagePhaseAction(
        PlayerbotAI* botAI) : Action(botAI, "kiljaeden manage phase") {}
    bool Execute(Event event) override;
};

class KiljaedenRangedDisperseAction : public MovementAction
{
public:
    KiljaedenRangedDisperseAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "kiljaeden ranged disperse") {}
    bool Execute(Event event) override;
};

#endif
