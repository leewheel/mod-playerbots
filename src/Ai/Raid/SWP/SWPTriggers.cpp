//By leewheel 2026-07-08
/*
 * 太阳之井高地 (Sunwell Plateau) 触发器实现
 * 作者: leewheel
 * 对照 Acore 源码修正法术ID和检测逻辑
 */
//End By leewheel

#include "SWPTriggers.h"

#include "AiFactory.h"
#include "Playerbots.h"
#include "SWPHelpers.h"
#include "RaidBossHelpers.h"
#include "SharedDefines.h"

using namespace SunwellPlateauHelpers;

// ===== 通用 =====

bool SunwellBotIsNotInCombatTrigger::IsActive()
{
    return !bot->IsInCombat() && bot->GetMapId() == SUNWELL_MAP_ID;
}

// ===== 入口小怪 (Entrance Trash) =====
// By leewheel 2026年7月9日
// 恢复入口小怪触发器实现，解决链接器LNK2001错误
// 策略层面(SWPStrategy.cpp)已注释掉TriggerNode注册，不会在运行时触发
// 但Context仍注册了这些类，必须提供实现以满足链接器要求
// End By leewheel

bool SwpTrashTankPullTrigger::IsActive()
{
    if (!botAI->IsTank(bot))
        return false;
    if (bot->GetMapId() != SUNWELL_MAP_ID)
        return false;
    if (IsAnySwBossPresent(botAI))
        return false;
    if (IsEntranceStrategyDone(botAI, bot))
        return false;
    if (!HasAliveEntranceTrash(botAI, bot))
        return false;
    float distToEntrance = bot->GetExactDist2d(
        SWP_ENTRANCE_TRASH_GROUP1_POS.GetPositionX(),
        SWP_ENTRANCE_TRASH_GROUP1_POS.GetPositionY());
    if (distToEntrance > SWP_ENTRANCE_DETECT_RADIUS)
        return false;
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    if (!currentTarget && !bot->IsInCombat())
        return false;
    return true;
}

bool SwpTrashTankWaitTrigger::IsActive()
{
    if (!botAI->IsTank(bot))
        return false;
    if (bot->GetMapId() != SUNWELL_MAP_ID)
        return false;
    if (IsAnySwBossPresent(botAI))
        return false;
    if (IsEntranceStrategyDone(botAI, bot))
        return false;
    if (!HasAliveEntranceTrash(botAI, bot))
        return false;
    float distToEntrance = bot->GetExactDist2d(
        SWP_ENTRANCE_TRASH_GROUP1_POS.GetPositionX(),
        SWP_ENTRANCE_TRASH_GROUP1_POS.GetPositionY());
    if (distToEntrance > SWP_ENTRANCE_DETECT_RADIUS)
        return false;
    if (bot->IsInCombat())
        return false;
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    if (currentTarget)
        return false;
    return true;
}

bool SwpTrashHealerEscortTrigger::IsActive()
{
    if (!botAI->IsHeal(bot))
        return false;
    if (!IsHealerEscort(botAI, bot))
        return false;
    if (bot->GetMapId() != SUNWELL_MAP_ID)
        return false;
    if (IsAnySwBossPresent(botAI))
        return false;
    if (IsEntranceStrategyDone(botAI, bot))
        return false;
    if (!HasAliveEntranceTrash(botAI, bot))
        return false;
    float distToEntrance = bot->GetExactDist2d(
        SWP_ENTRANCE_TRASH_GROUP1_POS.GetPositionX(),
        SWP_ENTRANCE_TRASH_GROUP1_POS.GetPositionY());
    if (distToEntrance > SWP_ENTRANCE_DETECT_RADIUS)
        return false;
    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank || !mainTank->IsAlive())
        return false;
    float distToTank = bot->GetExactDist2d(mainTank);
    if (distToTank <= SWP_HEALER_FOLLOW_RANGE)
        return false;
    return true;
}

bool SwpTrashGroupHoldTrigger::IsActive()
{
    if (botAI->IsTank(bot))
        return false;
    if (IsHealerEscort(botAI, bot))
        return false;
    if (bot->GetMapId() != SUNWELL_MAP_ID)
        return false;
    if (IsAnySwBossPresent(botAI))
        return false;
    if (IsEntranceStrategyDone(botAI, bot))
        return false;
    if (!HasAliveEntranceTrash(botAI, bot))
        return false;
    float distToEntrance = bot->GetExactDist2d(
        SWP_ENTRANCE_TRASH_GROUP1_POS.GetPositionX(),
        SWP_ENTRANCE_TRASH_GROUP1_POS.GetPositionY());
    if (distToEntrance > SWP_ENTRANCE_DETECT_RADIUS)
        return false;
    if (HasTrashNearEngagePos(botAI, bot))
        return false;
    return true;
}

bool SwpDeadPartyMemberWaitingTrigger::IsActive()
{
    if (bot->GetMapId() != SUNWELL_MAP_ID)
        return false;
    if (!bot->IsAlive())
        return false;
    if (bot->IsInCombat())
        return false;
    return HasDeadPartyMemberNotResurrected(botAI, bot);
}

// ===== 卡雷苟斯 (Kalecgos) =====

bool KalecgosPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    return kalecgos && kalecgos->GetHealthPct() > 95.0f;
}

bool KalecgosBossEngagedByTanksTrigger::IsActive()
{
    return botAI->IsTank(bot) &&
           AI_VALUE2(Unit*, "find target", "kalecgos");
}

bool KalecgosBossEngagedByRangedTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    if (!kalecgos)
        return false;

    // 检查奥术冲击叠加层数，超过3层需要分散
    Aura* buffet = bot->GetAura(static_cast<uint32>(SunwellSpells::SPELL_ARCANE_BUFFET));
    if (buffet && buffet->GetStackAmount() >= 3)
        return true;

    // 基础分散检查
    constexpr float safeDistFromPlayer = 8.0f;
    return GetNearestPlayerInRadius(bot, safeDistFromPlayer) != nullptr;
}

//By leewheel 2026-07-09 重写进入幽灵领域触发器
// 原问题：
// 1. 用 find target 查找萨斯罗瓦尔 -> 外场机器人没有仇恨，永远返回nullptr
// 2. 所有DPS都试图进入 -> 不打BOSS全跑去点门
// 3. 用 AI_VALUE2 find target 查找卡雷苟斯 -> 部分机器人不在仇恨表中，触发器不激活
// 4. GO_SPECTRAL_RIFT entry错误(187355→187055) -> 机器人找不到传送门GO
// 修复：
// 1. 使用 FindKalecgosBoss 替代 AI_VALUE2
// 2. 奥术冲击层数>=3时主动进入（降低阈值，3层=1500额外奥伤/tick已经很危险）
// 3. 添加内外场人数平衡检查，避免过多机器人同时进入内场
// 4. BOSS每20-30秒自动传送随机玩家，不需要全员主动进入
bool KalecgosNeedEnterSpectralRealmTrigger::IsActive()
{
    if (botAI->IsTank(bot))
        return false;

    // 使用FindKalecgosBoss替代AI_VALUE2，解决部分机器人不在仇恨表中的问题
    Unit* kalecgos = FindKalecgosBoss(botAI, bot);
    if (!kalecgos)
        return false;

    // 如果已经在幽灵领域中，不需要再次进入
    if (IsInSpectralRealm(bot))
        return false;

    // 如果有幽灵力竭debuff，不能再次进入
    if (bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_SPECTRAL_EXHAUSTION)))
        return false;

    // 奥术冲击层数>=3时才主动进入幽灵领域刷新debuff
    // Acore源码: 奥术冲击(45018)每8秒全团AoE，叠加debuff每层增500奥伤，持续40秒
    // 3层=1500额外奥伤/tick，对于非坦克职业已经很危险
    if (!NeedEnterSpectralForArcaneBuffet(bot, 3))
        return false;

    // 内外场人数平衡检查：
    // 内场人数不应超过小队总人数的1/3
    // BOSS每20-30秒自动传送1名玩家，不需要太多人主动进入
    uint32 inSpectral = CountGroupMembersInSpectralRealm(bot);
    uint32 groupSize = bot->GetGroup() ? bot->GetGroup()->GetMembersCount() : 1;
    uint32 maxInSpectral = std::max(1u, groupSize / 3);

    if (inSpectral >= maxInSpectral)
        return false;

    return true;
}
//End By leewheel

//By leewheel 2026-07-09 去掉Sathrovarr检查
// 只要机器人在幽灵领域中就触发，不需要额外检查Sathrovarr
// Sathrovarr的查找放到Action中处理（因为find target在外场找不到内场BOSS）
bool KalecgosInSpectralRealmTrigger::IsActive()
{
    return IsInSpectralRealm(bot);
}
//End By leewheel

// ===== 卡雷苟斯新增触发器 =====

//By leewheel 2026-07-09 使用FindKalecgosBoss替代find target
bool KalecgosHealthNotSyncedTrigger::IsActive()
{
    // 不在卡雷苟斯战斗中则不触发
    Unit* kalecgos = FindKalecgosBoss(botAI, bot);
    if (!kalecgos)
        return false;

    // 狂暴阶段不限制DPS节奏，全力输出
    if (IsKalecgosEnrageImminent(botAI, bot))
        return false;

    // 内外场血量差异>10%时触发
    float diff = GetKalecgosHealthDifference(botAI, bot);
    if (diff == FLT_MAX)
        return false;

    return std::abs(diff) > 10.0f;
}
//End By leewheel

//By leewheel 2026-07-09 使用FindKalecgosBoss替代find target
// 此触发器作为紧急后备：奥术冲击层数>=5时无视人数平衡强制进入
// 正常情况下KalecgosNeedEnterSpectralRealmTrigger(阈值3)会先触发
bool KalecgosNeedArcaneBuffetResetTrigger::IsActive()
{
    // 坦克不需要为此进入幽灵领域
    if (botAI->IsTank(bot))
        return false;

    // 不在卡雷苟斯战斗中则不触发
    Unit* kalecgos = FindKalecgosBoss(botAI, bot);
    if (!kalecgos)
        return false;

    // 奥术冲击层数>=5时紧急触发（此时每tick 2500额外奥伤，再不进入会死）
    // 此触发器作为紧急后备，不考虑人数平衡
    return NeedEnterSpectralForArcaneBuffet(bot, 5);
}
//End By leewheel

//By leewheel 2026-07-09 使用FindKalecgosBoss替代find target
bool KalecgosCurseOfBoundlessAgonyTrigger::IsActive()
{
    // 不在卡雷苟斯战斗中则不触发
    Unit* kalecgos = FindKalecgosBoss(botAI, bot);
    if (!kalecgos)
        return false;

    // 检查附近是否有队友中了无尽痛苦诅咒
    return HasCurseOfBoundlessAgonyNearby(bot);
}
//End By leewheel

//By leewheel 2026-07-09 使用FindKalecgosBoss替代find target
bool KalecgosFrostBreathOnTankTrigger::IsActive()
{
    // 不在卡雷苟斯战斗中则不触发
    Unit* kalecgos = FindKalecgosBoss(botAI, bot);
    if (!kalecgos)
        return false;

    // 检查主坦是否中了冰霜吐息（降攻速75%，可驱散）
    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    return mainTank->HasAura(static_cast<uint32>(SunwellSpells::SPELL_FROST_BREATH));
}
//End By leewheel

// ===== 布鲁塔卢斯 (Brutallus) =====

bool BrutallusPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    return brutallus && brutallus->GetHealthPct() > 95.0f;
}

bool BrutallusBossEngagedByTanksTrigger::IsActive()
{
    return botAI->IsTank(bot) &&
           AI_VALUE2(Unit*, "find target", "brutallus");
}

bool BrutallusCastingMeteorSlashTrigger::IsActive()
{
    if (botAI->IsHeal(bot))
        return false;

    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    if (!brutallus)
        return false;

    // 检查布鲁塔卢斯是否正在施放或刚施放流星猛击
    if (brutallus->FindCurrentSpellBySpellId(static_cast<uint32>(SunwellSpells::SPELL_METEOR_SLASH)))
        return true;

    // 检查自己是否有流星猛击debuff（被击中的标志）
    if (bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_METEOR_SLASH)))
        return true;

    return false;
}

bool BrutallusBotHasBurnTrigger::IsActive()
{
    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    if (!brutallus)
        return false;

    // 检查自己是否被燃烧点名（玩家身上的DoT光环是SPELL_BURN_DAMAGE = 46394）
    return bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_BURN_DAMAGE));
}

// ===== 菲米丝 (Felmyst) =====

bool FelmystPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    return felmyst && felmyst->GetHealthPct() > 95.0f;
}

bool FelmystBossEngagedByTanksTrigger::IsActive()
{
    return botAI->IsTank(bot) &&
           AI_VALUE2(Unit*, "find target", "felmyst");
}

bool FelmystCastingGasNovaTrigger::IsActive()
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    // 检查菲米丝是否正在施放毒气新星
    return felmyst->FindCurrentSpellBySpellId(static_cast<uint32>(SunwellSpells::SPELL_GAS_NOVA)) != nullptr;
}

bool FelmystCastingEncapsulateTrigger::IsActive()
{
    return HasEncapsulateNearby(botAI, bot);
}

bool FelmystInFlightPhaseTrigger::IsActive()
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    return GetFelmystPhase(felmyst) == 1;
}

bool FelmystNeedToManagePhaseTimerTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "felmyst") != nullptr;
}

// ===== 艾瑞达双子 (Eredar Twins) =====

bool EredarTwinsPullingBossesTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* sacrolash = AI_VALUE2(Unit*, "find target", "lady sacrolash");
    Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");

    return (sacrolash && sacrolash->GetHealthPct() > 95.0f) ||
           (alythess && alythess->GetHealthPct() > 95.0f);
}

bool EredarTwinsDeterminingKillOrderTrigger::IsActive()
{
    Unit* sacrolash = AI_VALUE2(Unit*, "find target", "lady sacrolash");
    Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");

    return (sacrolash && sacrolash->IsAlive()) ||
           (alythess && alythess->IsAlive());
}

bool EredarTwinsBotHasDarkTouchedTrigger::IsActive()
{
    // 如果有暗影触碰debuff，需要去火焰源头附近消除
    Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
    if (!alythess)
        return false;

    return bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_DARK_TOUCHED));
}

bool EredarTwinsBotHasFlameTouchedTrigger::IsActive()
{
    // 如果有火焰触碰debuff，需要去暗影源头附近消除
    Unit* sacrolash = AI_VALUE2(Unit*, "find target", "lady sacrolash");
    if (!sacrolash)
        return false;

    return bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_FLAME_TOUCHED));
}

bool EredarTwinsBotHasConflagrationTrigger::IsActive()
{
    // Acore源码: 爆燃(45342)初始由艾莉赛斯施放
    // 当萨洛拉尔死后，艾莉赛斯获得充能(45366)，继续施放爆燃
    // 当艾莉赛斯死后，萨洛拉尔获得充能(45366)，获得爆燃能力
    Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
    Unit* sacrolash = AI_VALUE2(Unit*, "find target", "lady sacrolash");

    // 检查艾莉赛斯是否正在施放爆燃
    if (alythess && alythess->FindCurrentSpellBySpellId(static_cast<uint32>(SunwellSpells::SPELL_CONFLAGRATION)))
        return true;

    // 检查萨洛拉尔是否在充能状态下施放爆燃（艾莉赛斯死后萨洛拉尔获得爆燃能力）
    if (sacrolash && sacrolash->HasAura(static_cast<uint32>(SunwellSpells::SPELL_EMPOWERED)) &&
        sacrolash->FindCurrentSpellBySpellId(static_cast<uint32>(SunwellSpells::SPELL_CONFLAGRATION)))
        return true;

    // 也检查玩家自身是否有爆燃debuff
    if (bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_CONFLAGRATION)))
        return true;

    return false;
}

// ===== 穆鲁 (Muru) =====

bool MuruEntropiusSpawnedTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    // 恩特罗皮乌斯出现后需要误导到坦克
    Unit* entropius = AI_VALUE2(Unit*, "find target", "entropius");
    return entropius && entropius->GetHealthPct() > 95.0f;
}

bool MuruAddsSpawnedTrigger::IsActive()
{
    Unit* muru = AI_VALUE2(Unit*, "find target", "muru");
    if (!muru)
        return false;

    // 检查是否有需要优先处理的ADD（狂暴者、怒火法师）
    Unit* berserker = AI_VALUE2(Unit*, "find target", "shadowsword berserker");
    Unit* furyMage = AI_VALUE2(Unit*, "find target", "shadowsword fury mage");

    return (berserker && berserker->IsAlive()) ||
           (furyMage && furyMage->IsAlive());
}

bool MuruVoidSentinelSpawnedTrigger::IsActive()
{
    Unit* muru = AI_VALUE2(Unit*, "find target", "muru");
    if (!muru)
        return false;

    Unit* voidSentinel = AI_VALUE2(Unit*, "find target", "void sentinel");
    return voidSentinel && voidSentinel->IsAlive();
}

bool MuruCastingDarknessTrigger::IsActive()
{
    Unit* muru = AI_VALUE2(Unit*, "find target", "muru");
    if (!muru)
        return false;

    // Acore源码: Muru对自己施放SPELL_DARKNESS_PERIODIC(45998)作为周期性光环
    // 该光环第2tick会召唤暗魔(Dark Fiend)
    // 应该用HasAura检测，而非FindCurrentSpellBySpellId
    return muru->HasAura(static_cast<uint32>(SunwellSpells::SPELL_DARKNESS_PERIODIC));
}

bool MuruEntropiusPhaseTrigger::IsActive()
{
    Unit* entropius = AI_VALUE2(Unit*, "find target", "entropius");
    return entropius && entropius->IsAlive();
}

// ===== 基尔加丹 (Kil'jaeden) =====

bool KiljaedenPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    return kiljaeden && kiljaeden->GetHealthPct() > 95.0f;
}

bool KiljaedenCastingDarknessOfSoulsTrigger::IsActive()
{
    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden)
        return false;

    return IsKiljaedenCastingDarkness(kiljaeden);
}

bool KiljaedenCastingArmageddonTrigger::IsActive()
{
    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden)
        return false;

    // 检查基尔加丹是否有末日审判周期性光环（阶段4开始施放）
    if (kiljaeden->HasAura(static_cast<uint32>(SunwellSpells::SPELL_ARMAGEDDON_PERIODIC)))
        return true;

    return false;
}

bool KiljaedenSpawnedSinisterReflectionTrigger::IsActive()
{
    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden)
        return false;

    // 检查是否有邪恶映像存在
    auto reflections = GetSinisterReflections(botAI, bot);
    return !reflections.empty();
}

bool KiljaedenShieldOrbSpawnedTrigger::IsActive()
{
    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden)
        return false;

    Unit* orb = GetShieldOrb(botAI, bot);
    return orb && orb->IsAlive();
}

bool KiljaedenNeedToManagePhaseTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "kil'jaeden") != nullptr;
}

bool KiljaedenBossEngagedByRangedTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden)
        return false;

    // 基尔加丹阶段2+会施放火焰飞镖，远程需要分散
    int phase = GetKiljaedenPhase(kiljaeden);
    if (phase < 1)
        return false;

    constexpr float safeDistFromPlayer = 8.0f;
    return GetNearestPlayerInRadius(bot, safeDistFromPlayer) != nullptr;
}
