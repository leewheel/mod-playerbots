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

bool KalecgosNeedEnterSpectralRealmTrigger::IsActive()
{
    if (botAI->IsTank(bot))
        return false;

    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    if (!kalecgos)
        return false;

    // 如果已经在幽灵领域中，不需要再次进入
    if (IsInSpectralRealm(bot))
        return false;

    // 如果有幽灵力竭debuff，不能再次进入
    if (bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_SPECTRAL_EXHAUSTION)))
        return false;

    // 检查是否有萨斯罗瓦尔还活着
    Unit* sathrovarr = AI_VALUE2(Unit*, "find target", "sathrovarr the corruptor");
    if (!sathrovarr || !sathrovarr->IsAlive())
        return false;

    // 奥术冲击层数过高时优先进入幽灵领域（层数>=4时进入以刷新debuff）
    // Acore源码: 奥术冲击(45018)每8秒全团AoE，叠加debuff每层增500奥伤，持续40秒
    // 进入幽灵领域可重置层数，避免后期层数过高致死
    if (NeedEnterSpectralForArcaneBuffet(bot, 4))
        return true;

    // DPS需要进入幽灵领域帮助卡雷苟斯对抗萨斯罗瓦尔
    if (botAI->IsDps(bot))
        return true;

    return false;
}

bool KalecgosInSpectralRealmTrigger::IsActive()
{
    if (!IsInSpectralRealm(bot))
        return false;

    Unit* sathrovarr = AI_VALUE2(Unit*, "find target", "sathrovarr the corruptor");
    return sathrovarr && sathrovarr->IsAlive();
}

// ===== 卡雷苟斯新增触发器 =====

bool KalecgosHealthNotSyncedTrigger::IsActive()
{
    // 不在卡雷苟斯战斗中则不触发
    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
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

bool KalecgosNeedArcaneBuffetResetTrigger::IsActive()
{
    // 坦克不需要为此进入幽灵领域
    if (botAI->IsTank(bot))
        return false;

    // 不在卡雷苟斯战斗中则不触发
    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    if (!kalecgos)
        return false;

    // 奥术冲击层数>=5且能进入幽灵领域时触发
    // Acore源码: 奥术冲击每8秒全团AoE，每层增500奥伤，持续40秒
    // 层数过高时必须进入幽灵领域刷新
    return NeedEnterSpectralForArcaneBuffet(bot, 5);
}

bool KalecgosCurseOfBoundlessAgonyTrigger::IsActive()
{
    // 不在卡雷苟斯战斗中则不触发
    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    if (!kalecgos)
        return false;

    // 检查附近是否有队友中了无尽痛苦诅咒
    return HasCurseOfBoundlessAgonyNearby(bot);
}

bool KalecgosFrostBreathOnTankTrigger::IsActive()
{
    // 不在卡雷苟斯战斗中则不触发
    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    if (!kalecgos)
        return false;

    // 检查主坦是否中了冰霜吐息（降攻速75%，可驱散）
    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    return mainTank->HasAura(static_cast<uint32>(SunwellSpells::SPELL_FROST_BREATH));
}

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
