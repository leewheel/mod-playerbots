/*
 * 版权所有 (C) 2026 Leewheel
 * 
 * 文件功能：纳克萨玛斯团队副本触发器头文件
 * 定义纳克萨玛斯副本各Boss的触发器类和相关常量
 * 
 * By Leewheel 2026-02-14
 */

#ifndef _PLAYERBOT_RAIDNAXXTRIGGERS_H_
#define _PLAYERBOT_RAIDNAXXTRIGGERS_H_

#include "PlayerbotAI.h"
#include "Trigger.h"

// 纳克萨玛斯Boss和法术常量
enum NaxxramasNPCs
{
    // 蜘蛛区
    NPC_ANUBREKHAN          = 15956,
    NPC_FAERLINA            = 15953,
    NPC_MAEXXNA             = 15952,
    
    // 瘟疫区
    NPC_NOTH                = 15954,
    NPC_HEIGAN              = 15936,
    NPC_LOATHEB             = 16011,
    
    // 军事区
    NPC_RAZUVIOUS           = 16061,
    NPC_GOTHIK              = 16060,
    NPC_ZELIEK              = 16063,
    NPC_BLAUMEUX            = 16065,
    NPC_RIVENDARE           = 30549,
    NPC_KORTHAZZ            = 16064,
    
    // 构造区
    NPC_PATCHWERK           = 16028,
    NPC_GROBBULUS           = 15931,
    NPC_GLUTH               = 15932,
    NPC_THADDIUS            = 15928,
    
    // 冰龙巢穴
    NPC_SAPPHIRON           = 15989,
    NPC_KELTHUZAD           = 15990,
    
    // 其他NPC
    NPC_CRYPT_GUARD         = 16573,
    NPC_NAXXRAMAS_WORSHIPPER = 16506,
    NPC_NAXXRAMAS_FOLLOWER  = 16505,
    NPC_SPIDERLING          = 17055,
    NPC_PLAGUED_WARRIOR     = 16984,
    NPC_PLAGUED_CHAMPION    = 16983,
    NPC_ZOMBIE_CHOW         = 16360,
    NPC_DEATHKNIGHT_UNDERSTUDY = 16803,
    NPC_SOLDIER_OF_THE_FROZEN_WASTES = 16427,
    NPC_UNSTOPPABLE_ABOMINATION = 16428,
    NPC_GUARDIAN_OF_ICECROWN = 16441
};

enum NaxxramasSpells
{
    // 帕奇维克
    SPELL_HATEFUL_STRIKE_10     = 41926,
    SPELL_HATEFUL_STRIKE_25     = 59192,
    SPELL_FRENZY                = 28131,
    SPELL_BERSERK               = 26662,
    SPELL_SLIME_BOLT            = 32309,
    
    // 海根
    SPELL_SPELL_DISRUPTION      = 29310,
    SPELL_DECREPIT_FEVER_10     = 29998,
    SPELL_DECREPIT_FEVER_25     = 55011,
    SPELL_PLAGUE_CLOUD          = 29350,
    
    // 四骑士
    SPELL_MARK_OF_ZELIEK        = 28835,
    SPELL_MARK_OF_BLAUMEUX      = 28833,
    SPELL_MARK_OF_RIVENDARE     = 28834,
    SPELL_MARK_OF_KORTHAZZ      = 28832,
    
    // 塔迪乌斯
    SPELL_POLARITY_SHIFT        = 28089,
    SPELL_POSITIVE_CHARGE       = 28062,
    SPELL_NEGATIVE_CHARGE       = 28085,
    
    // 格罗布鲁斯
    SPELL_MUTATING_INJECTION    = 28169,
    SPELL_POISON_CLOUD          = 28240,
    
    // 萨菲隆
    SPELL_ICEBOLT               = 28522,
    SPELL_FROST_BREATH          = 29318,
    SPELL_FROST_AURA            = 28531,
    
    // 阿努布雷坎
    SPELL_LOCUST_SWARM          = 28785,
    SPELL_IMPALE                = 28783,
    
    // 费尔默
    SPELL_FAERLINA_FRENZY       = 28798,
    SPELL_POISON_BOLT_VOLLEY    = 28796,
    
    // 迈克斯纳
    SPELL_WEB_SPRAY             = 29484,
    SPELL_WEB_WRAP              = 28622,
    SPELL_POISON_SHOCK          = 28741,
    
    // 诺斯
    SPELL_NOTH_TELEPORT         = 29211,
    SPELL_CRIPPLE               = 29212,
    
    // 洛欧塞布
    SPELL_NECROTIC_AURA         = 55593,
    SPELL_INEVITABLE_DOOM       = 29204,
    
    // 格拉斯
    SPELL_DECIMATE              = 28374,
    SPELL_TERRIFYING_ROAR       = 29685,
    
    // 拉祖维奥斯
    SPELL_DISRUPTING_SHOUT      = 29107,
    SPELL_JAGGED_KNIFE          = 55550,
    
    // 克尔苏加德
    SPELL_FROST_BLAST           = 27808,
    SPELL_SHADOW_FISSURE        = 27810,
    SPELL_MANA_DETONATION       = 27819,
    SPELL_CHAINS_OF_KELTHUZAD   = 28410
};

enum NaxxramasData
{
    DATA_HEIGAN_ERUPTION        = 101,
    DATA_DANCE_FAIL             = 122,
    DATA_IMMORTAL_FAIL          = 123,
    DATA_THADDIUS_BOSS          = 107,
    DATA_STALAGG_BOSS           = 108,
    DATA_FEUGEN_BOSS            = 109
};

// ==========================================
// 帕奇维克触发器
// ==========================================

// 检测帕奇维克是否进入战斗
class NaxxPatchwerkCombatTrigger : public Trigger
{
public:
    NaxxPatchwerkCombatTrigger(PlayerbotAI* ai) : Trigger(ai, "naxx patchwerk combat") {}
    bool IsActive() override;
};

// 检测帕奇维克是否进入狂乱状态（≤5% 生命值）
class NaxxPatchwerkFrenzyTrigger : public Trigger
{
public:
    NaxxPatchwerkFrenzyTrigger(PlayerbotAI* ai) : Trigger(ai, "naxx patchwerk frenzy") {}
    bool IsActive() override;
};

// 检测帕奇维克是否进入狂暴状态（6分钟）
class NaxxPatchwerkBerserkTrigger : public Trigger
{
public:
    NaxxPatchwerkBerserkTrigger(PlayerbotAI* ai) : Trigger(ai, "naxx patchwerk berserk") {}
    bool IsActive() override;
};

// 检测副坦克是否需要定位到帕奇维克侧面
class NaxxPatchwerkOffTankPositionTrigger : public Trigger
{
public:
    NaxxPatchwerkOffTankPositionTrigger(PlayerbotAI* ai) : Trigger(ai, "naxx patchwerk offtank position") {}
    bool IsActive() override;
};

// ==========================================
// 格罗布鲁斯触发器
// ==========================================

// 检测机器人是否有变异注射减益效果
class NaxxGrobbulusMutatingInjectionTrigger : public Trigger
{
public:
    NaxxGrobbulusMutatingInjectionTrigger(PlayerbotAI* ai) : Trigger(ai, "naxx grobbulus mutating injection") {}
    bool IsActive() override;
};

// 检测机器人是否靠近毒云区域
class NaxxGrobbulusPoisonCloudTrigger : public Trigger
{
public:
    NaxxGrobbulusPoisonCloudTrigger(PlayerbotAI* ai) : Trigger(ai, "naxx grobbulus poison cloud") {}
    bool IsActive() override;
};

// ==========================================
// 阿努布雷坎触发器
// ==========================================

// 检测阿努布雷坎是否施放蝗虫群
class NaxxAnubRekhanLocustSwarmTrigger : public Trigger
{
public:
    NaxxAnubRekhanLocustSwarmTrigger(PlayerbotAI* ai) : Trigger(ai, "naxx anubrekhan locust swarm") {}
    bool IsActive() override;
};

// ==========================================
// 费尔莉娜触发器
// ==========================================

// 检测费尔莉娜是否进入狂乱状态
class NaxxFaerlinaFrenzyTrigger : public Trigger
{
public:
    NaxxFaerlinaFrenzyTrigger(PlayerbotAI* ai) : Trigger(ai, "naxx faerlina frenzy") {}
    bool IsActive() override;
};

// ==========================================
// 迈克斯纳触发器
// ==========================================

// 检测迈克斯纳是否施放蛛网喷射
class NaxxMaexxnaWebSprayTrigger : public Trigger
{
public:
    NaxxMaexxnaWebSprayTrigger(PlayerbotAI* ai) : Trigger(ai, "naxx maexxna web spray") {}
    bool IsActive() override;
};

// 检测机器人是否被蛛网缠绕
class NaxxMaexxnaWebWrapTrigger : public Trigger
{
public:
    NaxxMaexxnaWebWrapTrigger(PlayerbotAI* ai) : Trigger(ai, "naxx maexxna web wrap") {}
    bool IsActive() override;
};

// 检测迈克斯纳是否施放毒性冲击
class NaxxMaexxnaPoisonShockTrigger : public Trigger
{
public:
    NaxxMaexxnaPoisonShockTrigger(PlayerbotAI* ai) : Trigger(ai, "naxx maexxna poison shock") {}
    bool IsActive() override;
};

// ==========================================
// 诺斯触发器
// ==========================================

// 检测诺斯是否传送（消失）
class NaxxNothTeleportTrigger : public Trigger
{
public:
    NaxxNothTeleportTrigger(PlayerbotAI* ai) : Trigger(ai, "naxx noth teleport") {}
    bool IsActive() override;
};

// 检测诺斯是否重新出现
class NaxxNothReappearTrigger : public Trigger
{
public:
    NaxxNothReappearTrigger(PlayerbotAI* ai) : Trigger(ai, "naxx noth reappear") {}
    bool IsActive() override;
};

// ==========================================
// 洛欧塞布触发器
// ==========================================

// 检测死灵光环是否激活
class NaxxLoathebNecroticAuraActiveTrigger : public Trigger
{
public:
    NaxxLoathebNecroticAuraActiveTrigger(PlayerbotAI* ai) : Trigger(ai, "naxx loatheb necrotic aura active") {}
    bool IsActive() override;
};

// 检测死灵光环是否消失
class NaxxLoathebNecroticAuraInactiveTrigger : public Trigger
{
public:
    NaxxLoathebNecroticAuraInactiveTrigger(PlayerbotAI* ai) : Trigger(ai, "naxx loatheb necrotic aura inactive") {}
    bool IsActive() override;
};

// ==========================================
// 格拉斯触发器
// ==========================================

// 检测格拉斯是否施放毁灭
class NaxxGluthDecimateTrigger : public Trigger
{
public:
    NaxxGluthDecimateTrigger(PlayerbotAI* ai) : Trigger(ai, "naxx gluth decimate") {}
    bool IsActive() override;
};

// 检测附近是否有僵尸食尸鬼
class NaxxGluthZombieChowNearbyTrigger : public Trigger
{
public:
    NaxxGluthZombieChowNearbyTrigger(PlayerbotAI* ai) : Trigger(ai, "naxx gluth zombie chow nearby") {}
    bool IsActive() override;
};

// ==========================================
// 海根触发器
// ==========================================

// 检测海根跳舞机制（地板喷发）
class NaxxHeiganDanceTrigger : public Trigger
{
public:
    NaxxHeiganDanceTrigger(PlayerbotAI* ai) : Trigger(ai, "naxx heigan dance") {}
    bool IsActive() override;
};

// ==========================================
// 四骑士触发器
// ==========================================

// 检测骑士标记层数是否过高
class NaxxFourHorsemenMarkHighTrigger : public Trigger
{
public:
    NaxxFourHorsemenMarkHighTrigger(PlayerbotAI* ai) : Trigger(ai, "naxx four horsemen mark high") {}
    bool IsActive() override;
};

// ==========================================
// 塔迪乌斯触发器
// ==========================================

// 检测塔迪乌斯是否施放极性转换
class NaxxThaddiusPolarityShiftTrigger : public Trigger
{
public:
    NaxxThaddiusPolarityShiftTrigger(PlayerbotAI* ai) : Trigger(ai, "naxx thaddius polarity shift") {}
    bool IsActive() override;
};

// ==========================================
// 戈提克触发器
// ==========================================

// 检测戈提克第一阶段（双区域战斗）
class NaxxGothikPhaseOneTrigger : public Trigger
{
public:
    NaxxGothikPhaseOneTrigger(PlayerbotAI* ai) : Trigger(ai, "naxx gothik phase one") {}
    bool IsActive() override;
};

// 检测戈提克第二阶段（Boss可攻击）
class NaxxGothikPhaseTwoTrigger : public Trigger
{
public:
    NaxxGothikPhaseTwoTrigger(PlayerbotAI* ai) : Trigger(ai, "naxx gothik phase two") {}
    bool IsActive() override;
};

// ==========================================
// 拉祖维奥斯触发器
// ==========================================

// 检测是否需要控制死亡骑士学徒
class NaxxRazuviousNeedControlTrigger : public Trigger
{
public:
    NaxxRazuviousNeedControlTrigger(PlayerbotAI* ai) : Trigger(ai, "naxx razuvious need control") {}
    bool IsActive() override;
};

// 检测控制是否即将到期
class NaxxRazuviousControlExpiringTrigger : public Trigger
{
public:
    NaxxRazuviousControlExpiringTrigger(PlayerbotAI* ai) : Trigger(ai, "naxx razuvious control expiring") {}
    bool IsActive() override;
};

// ==========================================
// 萨菲隆触发器
// ==========================================

// 检测萨菲隆是否进入空中阶段
class NaxxSapphironAirPhaseTrigger : public Trigger
{
public:
    NaxxSapphironAirPhaseTrigger(PlayerbotAI* ai) : Trigger(ai, "naxx sapphiron air phase") {}
    bool IsActive() override;
};

// ==========================================
// 克尔苏加德触发器
// ==========================================

// 检测克尔苏加德第一阶段（小怪阶段）
class NaxxKelThuzadPhaseOneTrigger : public Trigger
{
public:
    NaxxKelThuzadPhaseOneTrigger(PlayerbotAI* ai) : Trigger(ai, "naxx kelthuzad phase one") {}
    bool IsActive() override;
};

// 检测克尔苏加德第二阶段（Boss战斗）
class NaxxKelThuzadPhaseTwoTrigger : public Trigger
{
public:
    NaxxKelThuzadPhaseTwoTrigger(PlayerbotAI* ai) : Trigger(ai, "naxx kelthuzad phase two") {}
    bool IsActive() override;
};

// 检测克尔苏加德第三阶段（守护者出现）
class NaxxKelThuzadPhaseThreeTrigger : public Trigger
{
public:
    NaxxKelThuzadPhaseThreeTrigger(PlayerbotAI* ai) : Trigger(ai, "naxx kelthuzad phase three") {}
    bool IsActive() override;
};

// 检测机器人是否被冰霜冲击冻结
class NaxxKelThuzadFrostBlastTrigger : public Trigger
{
public:
    NaxxKelThuzadFrostBlastTrigger(PlayerbotAI* ai) : Trigger(ai, "naxx kelthuzad frost blast") {}
    bool IsActive() override;
};

// 检测附近是否有暗影裂隙
class NaxxKelThuzadShadowFissureTrigger : public Trigger
{
public:
    NaxxKelThuzadShadowFissureTrigger(PlayerbotAI* ai) : Trigger(ai, "naxx kelthuzad shadow fissure") {}
    bool IsActive() override;
};

#endif

// By Leewheel 2026-02-14
