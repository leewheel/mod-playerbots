//By leewheel 2026-07-08
/*
 * 太阳之井高地 (Sunwell Plateau) 触发器声明
 * 作者: leewheel
 * 每个 BOSS 的每个关键机制对应一个 Trigger 类
 * 类名和触发器名称与 SWPTriggerContext.h / SWPStrategy.cpp 保持一致
 */
//End By leewheel

#ifndef PLAYERBOTS_SWPTRIGGERS_H
#define PLAYERBOTS_SWPTRIGGERS_H

#include "Trigger.h"

// ===== 通用 =====

class SunwellBotIsNotInCombatTrigger : public Trigger
{
public:
    SunwellBotIsNotInCombatTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "sunwell bot is not in combat") {}
    bool IsActive() override;
};

// ===== 入口小怪 (Entrance Trash) =====

class SwpTrashTankPullTrigger : public Trigger
{
public:
    SwpTrashTankPullTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "swp trash tank pull") {}
    bool IsActive() override;
};

class SwpTrashGroupHoldTrigger : public Trigger
{
public:
    SwpTrashGroupHoldTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "swp trash group hold") {}
    bool IsActive() override;
};

class SwpDeadPartyMemberWaitingTrigger : public Trigger
{
public:
    SwpDeadPartyMemberWaitingTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "swp dead party member waiting") {}
    bool IsActive() override;
};

// ===== 卡雷苟斯 (Kalecgos) =====

class KalecgosPullingBossTrigger : public Trigger
{
public:
    KalecgosPullingBossTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kalecgos pulling boss") {}
    bool IsActive() override;
};

class KalecgosBossEngagedByTanksTrigger : public Trigger
{
public:
    KalecgosBossEngagedByTanksTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kalecgos boss engaged by tanks") {}
    bool IsActive() override;
};

class KalecgosBossEngagedByRangedTrigger : public Trigger
{
public:
    KalecgosBossEngagedByRangedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kalecgos boss engaged by ranged") {}
    bool IsActive() override;
};

class KalecgosNeedEnterSpectralRealmTrigger : public Trigger
{
public:
    KalecgosNeedEnterSpectralRealmTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kalecgos need enter spectral realm") {}
    bool IsActive() override;
};

class KalecgosInSpectralRealmTrigger : public Trigger
{
public:
    KalecgosInSpectralRealmTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kalecgos in spectral realm") {}
    bool IsActive() override;
};

// 卡雷苟斯内外场血量同步触发器
// 当内外场BOSS血量差异>10%且未进入狂暴阶段时触发
class KalecgosHealthNotSyncedTrigger : public Trigger
{
public:
    KalecgosHealthNotSyncedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kalecgos health not synced") {}
    bool IsActive() override;
};

// 奥术冲击层数过高需要进入幽灵领域刷新触发器
class KalecgosNeedArcaneBuffetResetTrigger : public Trigger
{
public:
    KalecgosNeedArcaneBuffetResetTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kalecgos need arcane buffet reset") {}
    bool IsActive() override;
};

// 附近有无尽痛苦诅咒需要驱散触发器
class KalecgosCurseOfBoundlessAgonyTrigger : public Trigger
{
public:
    KalecgosCurseOfBoundlessAgonyTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kalecgos curse of boundless agony") {}
    bool IsActive() override;
};

// 主坦中了冰霜吐息需要驱散触发器
class KalecgosFrostBreathOnTankTrigger : public Trigger
{
public:
    KalecgosFrostBreathOnTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kalecgos frost breath on tank") {}
    bool IsActive() override;
};

// ===== 布鲁塔卢斯 (Brutallus) =====

class BrutallusPullingBossTrigger : public Trigger
{
public:
    BrutallusPullingBossTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "brutallus pulling boss") {}
    bool IsActive() override;
};

class BrutallusBossEngagedByTanksTrigger : public Trigger
{
public:
    BrutallusBossEngagedByTanksTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "brutallus boss engaged by tanks") {}
    bool IsActive() override;
};

class BrutallusCastingMeteorSlashTrigger : public Trigger
{
public:
    BrutallusCastingMeteorSlashTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "brutallus casting meteor slash") {}
    bool IsActive() override;
};

class BrutallusBotHasBurnTrigger : public Trigger
{
public:
    BrutallusBotHasBurnTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "brutallus bot has burn") {}
    bool IsActive() override;
};

// ===== 菲米丝 (Felmyst) =====

class FelmystPullingBossTrigger : public Trigger
{
public:
    FelmystPullingBossTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "felmyst pulling boss") {}
    bool IsActive() override;
};

class FelmystBossEngagedByTanksTrigger : public Trigger
{
public:
    FelmystBossEngagedByTanksTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "felmyst boss engaged by tanks") {}
    bool IsActive() override;
};

class FelmystCastingGasNovaTrigger : public Trigger
{
public:
    FelmystCastingGasNovaTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "felmyst casting gas nova") {}
    bool IsActive() override;
};

class FelmystCastingEncapsulateTrigger : public Trigger
{
public:
    FelmystCastingEncapsulateTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "felmyst casting encapsulate") {}
    bool IsActive() override;
};

class FelmystInFlightPhaseTrigger : public Trigger
{
public:
    FelmystInFlightPhaseTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "felmyst in flight phase") {}
    bool IsActive() override;
};

class FelmystNeedToManagePhaseTimerTrigger : public Trigger
{
public:
    FelmystNeedToManagePhaseTimerTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "felmyst need to manage phase timer") {}
    bool IsActive() override;
};

// ===== 艾瑞达双子 (Eredar Twins) =====

class EredarTwinsPullingBossesTrigger : public Trigger
{
public:
    EredarTwinsPullingBossesTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "eredar twins pulling bosses") {}
    bool IsActive() override;
};

class EredarTwinsDeterminingKillOrderTrigger : public Trigger
{
public:
    EredarTwinsDeterminingKillOrderTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "eredar twins determining kill order") {}
    bool IsActive() override;
};

class EredarTwinsBotHasDarkTouchedTrigger : public Trigger
{
public:
    EredarTwinsBotHasDarkTouchedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "eredar twins bot has dark touched") {}
    bool IsActive() override;
};

class EredarTwinsBotHasFlameTouchedTrigger : public Trigger
{
public:
    EredarTwinsBotHasFlameTouchedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "eredar twins bot has flame touched") {}
    bool IsActive() override;
};

class EredarTwinsBotHasConflagrationTrigger : public Trigger
{
public:
    EredarTwinsBotHasConflagrationTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "eredar twins bot has conflagration") {}
    bool IsActive() override;
};

// ===== 穆鲁 (Muru) =====

class MuruEntropiusSpawnedTrigger : public Trigger
{
public:
    MuruEntropiusSpawnedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "muru entropius spawned") {}
    bool IsActive() override;
};

class MuruAddsSpawnedTrigger : public Trigger
{
public:
    MuruAddsSpawnedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "muru adds spawned") {}
    bool IsActive() override;
};

class MuruVoidSentinelSpawnedTrigger : public Trigger
{
public:
    MuruVoidSentinelSpawnedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "muru void sentinel spawned") {}
    bool IsActive() override;
};

class MuruCastingDarknessTrigger : public Trigger
{
public:
    MuruCastingDarknessTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "muru casting darkness") {}
    bool IsActive() override;
};

class MuruEntropiusPhaseTrigger : public Trigger
{
public:
    MuruEntropiusPhaseTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "muru entropius phase") {}
    bool IsActive() override;
};

// ===== 基尔加丹 (Kil'jaeden) =====

class KiljaedenPullingBossTrigger : public Trigger
{
public:
    KiljaedenPullingBossTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kil'jaeden pulling boss") {}
    bool IsActive() override;
};

class KiljaedenCastingDarknessOfSoulsTrigger : public Trigger
{
public:
    KiljaedenCastingDarknessOfSoulsTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kil'jaeden casting darkness of souls") {}
    bool IsActive() override;
};

class KiljaedenCastingArmageddonTrigger : public Trigger
{
public:
    KiljaedenCastingArmageddonTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kil'jaeden casting armageddon") {}
    bool IsActive() override;
};

class KiljaedenSpawnedSinisterReflectionTrigger : public Trigger
{
public:
    KiljaedenSpawnedSinisterReflectionTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kil'jaeden spawned sinister reflection") {}
    bool IsActive() override;
};

class KiljaedenShieldOrbSpawnedTrigger : public Trigger
{
public:
    KiljaedenShieldOrbSpawnedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kil'jaeden shield orb spawned") {}
    bool IsActive() override;
};

class KiljaedenNeedToManagePhaseTrigger : public Trigger
{
public:
    KiljaedenNeedToManagePhaseTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kil'jaeden need to manage phase") {}
    bool IsActive() override;
};

class KiljaedenBossEngagedByRangedTrigger : public Trigger
{
public:
    KiljaedenBossEngagedByRangedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kil'jaeden boss engaged by ranged") {}
    bool IsActive() override;
};

#endif
