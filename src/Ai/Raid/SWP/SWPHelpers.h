//By leewheel 2026-07-08
/*
 * 太阳之井高地 (Sunwell Plateau) 策略辅助定义
 * 作者: leewheel
 * 包含: 法术ID、NPC ID、物品ID、游戏对象ID、坐标常量、辅助函数声明
 * 对照 Acore boss_kalecgos/brutallus/felmyst/eredar_twins/muru/kiljaeden 源码校验
 */
//End By leewheel

#ifndef PLAYERBOTS_SWPHELPERS_H
#define PLAYERBOTS_SWPHELPERS_H

#include <array>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "Common.h"
#include "ObjectGuid.h"
#include "Position.h"

class GameObject;
class Player;
class PlayerbotAI;
class Unit;

namespace SunwellPlateauHelpers
{

// 太阳之井高地法术ID
enum class SunwellSpells : uint32
{
    // ===== 卡雷苟斯 (Kalecgos) =====
    // 外场 - 卡雷苟斯龙形态
    SPELL_SPECTRAL_REALM            = 46021,  // 幽灵领域（进入恶魔位面的光环）
    SPELL_SPECTRAL_EXHAUSTION       = 44867,  // 幽灵力竭（退出后60秒内不能再进入）
    SPELL_CURSE_OF_BOUNDLESS_AGONY  = 45032,  // 无尽痛苦诅咒（BOSS施放在目标身上）
    SPELL_CURSE_OF_BOUNDLESS_AGONY_PLR = 45034, // 无尽痛苦诅咒（玩家身上的转移版本）
    SPELL_SPECTRAL_BLAST            = 44869,  // 幽灵冲击（BOSS随机传送玩家进内场）
    SPELL_SPECTRAL_BLAST_PORTAL     = 44866,  // 幽灵冲击传送门视觉
    SPELL_TELEPORT_SPECTRAL         = 46019,  // 传送到幽灵领域
    SPELL_TELEPORT_NORMAL_REALM     = 46020,  // 传送回现实位面
    SPELL_ARCANE_BUFFET             = 45018,  // 奥术冲击（外场每8秒AoE，叠加debuff）
    SPELL_FROST_BREATH              = 44799,  // 冰霜吐息（正面锥形，降攻速75%）
    SPELL_TAIL_LASH                 = 45122,  // 尾部鞭笞
    SPELL_BANISH                    = 44836,  // 放逐（1%血量触发）
    SPELL_CRAZED_RAGE               = 44807,  // 狂暴（10%血量触发，双方互触）
    // 内场 - 萨斯罗瓦尔
    SPELL_SHADOW_BOLT               = 45031,  // 暗影箭（锥形AoE ~5000伤害）
    SPELL_CORRUPTION_STRIKE         = 45029,  // 腐化冲击（秒T技能 ~9000+击倒+DoT）
    SPELL_CURSE_OF_BOUNDLESS_AGONY_REMOVE = 45050, // 诅咒移除
    // NPC卡雷苟斯人形态
    SPELL_REVITALIZE                = 45027,  // 恢复（内场NPC卡雷苟斯施放）
    SPELL_HEROIC_STRIKE             = 45026,  // 英雄打击（内场NPC卡雷苟斯施放）

    // ===== 布鲁塔卢斯 (Brutallus) =====
    SPELL_METEOR_SLASH              = 45150,  // 流星猛击（锥形分摊伤害）
    SPELL_BURN                      = 45141,  // 燃烧（BOSS施放的触发法术）
    SPELL_BURN_DAMAGE               = 46394,  // 燃烧伤害（玩家身上的DoT光环）
    SPELL_BERSERK                   = 26662,  // 狂暴（6分钟后）
    SPELL_STOMP                     = 45185,  // 踩踏（击倒+伤害）

    // ===== 菲米丝 (Felmyst) =====
    SPELL_GAS_NOVA                  = 45855,  // 毒气新星（需要驱散/跑开）
    SPELL_ENCAPSULATE_CHANNEL       = 45661,  // 包裹通道（地面AoE通道法术）
    SPELL_BERSERK_FELMYST           = 45078,  // 狂暴（10分钟后）
    SPELL_CORROSION                 = 45866,  // 腐蚀（减甲）
    SPELL_NOXIOUS_FUMES             = 47002,  // 有毒烟雾（开场AoE）
    SPELL_CLEAVE                    = 19983,  // 顺劈

    // ===== 艾瑞达双子 (Eredar Twins) =====
    // 萨洛拉尔·黎明破坏者 (Lady Sacrolash)
    SPELL_DARK_TOUCHED              = 45347,  // 暗影触碰（debuff）
    SPELL_SHADOW_NOVA               = 45329,  // 暗影新星
    SPELL_SHADOW_BLADES             = 45248,  // 暗影之刃
    SPELL_CONFOUNDING_BLOW          = 45256,  // 混淆打击
    // 高阶术士艾莉赛斯 (Grand Warlock Alythess)
    SPELL_FLAME_TOUCHED             = 45348,  // 火焰触碰（debuff）
    SPELL_FLAME_SEAR                = 46771,  // 火焰灼烧（5个目标）
    SPELL_BLAZE                     = 45235,  // 烈焰（地面火焰）
    SPELL_CONFLAGRATION             = 45342,  // 爆燃（点名玩家，昏迷+火焰伤害）
    // 共用
    SPELL_EMPOWERED                 = 45366,  // 充能（一个双子死后另一个获得）
    SPELL_ENRAGE_TWINS              = 46587,  // 狂暴（6分钟后）

    // ===== 穆鲁 (Muru) =====
    // 阶段1 - 穆鲁
    SPELL_DARKNESS_PERIODIC         = 45998,  // 黑暗周期性（穆鲁对自己施放，第2tick召唤暗魔）
    SPELL_NEGATIVE_ENERGY           = 46009,  // 负能量（周期性随机目标伤害）
    SPELL_SUMMON_BLOOD_ELVES_PERIODIC = 46041, // 召唤血精灵周期性
    SPELL_OPEN_PORTAL_PERIODIC      = 45994,  // 开门周期性（召唤虚空哨兵）
    SPELL_SUMMON_BERSERKER1         = 46037,  // 召唤狂暴者1
    SPELL_SUMMON_BERSERKER2         = 46040,  // 召唤狂暴者2
    SPELL_SUMMON_FURY_MAGE1         = 46038,  // 召唤怒火法师1
    SPELL_SUMMON_FURY_MAGE2         = 46039,  // 召唤怒火法师2
    SPELL_OPEN_ALL_PORTALS          = 46177,  // 穆鲁开门（阶段转换）
    SPELL_SUMMON_ENTROPIUS          = 46217,  // 召唤恩特罗皮乌斯
    SPELL_SUMMON_DARK_FIEND         = 46000,  // 召唤暗魔（46000-46007）
    // 阶段2 - 恩特罗皮乌斯
    SPELL_ENTROPIUS_COSMETIC_SPAWN  = 46223,  // 恩特罗皮乌斯出现视觉
    SPELL_NEGATIVE_ENERGY_PERIODIC  = 46284,  // 恩特罗皮乌斯的负能量（递增伤害）
    SPELL_BLACK_HOLE                = 46282,  // 黑洞（召唤奇点）
    SPELL_DARKNESS_ENTROPIUS        = 46269,  // 恩特罗皮乌斯的黑暗
    SPELL_BLACK_HOLE_EFFECT         = 46230,  // 黑洞吸引效果

    // ===== 基尔加丹 (Kil'jaeden) =====
    SPELL_SHADOW_SPIKE              = 46680,  // 暗影之刺（高额伤害）
    SPELL_FLAME_DART                = 45737,  // 火焰飞镖（远程持续伤害）
    SPELL_DARKNESS_OF_SOULS         = 46605,  // 千魂之暗（大招，需要护盾）
    SPELL_DARKNESS_OF_SOULS_DAMAGE  = 45657,  // 千魂之暗伤害
    SPELL_ARMAGEDDON_PERIODIC       = 45921,  // 末日审判周期性触发
    SPELL_ARMAGEDDON_MISSILE        = 45909,  // 末日审判（陨石导弹）
    SPELL_ARMAGEDDON_VISUAL         = 45911,  // 末日审判视觉
    SPELL_VENGEANCE_OF_THE_BLUE_FLIGHT = 45839, // 蓝龙之怒（玩家骑龙后获得的护盾光环）
    SPELL_SHIELD_OF_THE_BLUE        = 45848,  // 蓝龙之盾（护盾减伤）
    SPELL_SOUL_FLAY                 = 45442,  // 灵魂抽打
    SPELL_LEGION_LIGHTNING          = 45664,  // 军团闪电
    SPELL_FIRE_BLOOM                = 45641,  // 火焰之花
    SPELL_SINISTER_REFLECTION       = 45892,  // 邪恶映像（召唤玩家镜像）
    SPELL_FLAME_BURST               = 45779,  // 烈焰爆发（近战范围AoE）

    // 通用 - 猎人误导
    SPELL_MISDIRECTION              = 35079,
};

// 太阳之井高地NPC ID
enum class SunwellNpcs : uint32
{
    // 卡雷苟斯
    NPC_KALECGOS_DRAGON             = 24850,  // 卡雷苟斯（龙形态，现实位面）
    NPC_KALECGOS_HUMAN              = 24891,  // 卡雷苟斯（人形态，内场NPC）
    NPC_SATHROVARR                  = 24892,  // 萨斯罗瓦尔（恶魔位面BOSS）

    // 布鲁塔卢斯
    NPC_BRUTALLUS                   = 24882,

    // 菲米丝
    NPC_FELMYST                     = 25038,

    // 艾瑞达双子
    NPC_LADY_SACROLASH              = 25165,
    NPC_GRAND_WARLOCK_ALYTHESS      = 25166,

    // 穆鲁
    NPC_MURU                        = 25741,
    NPC_ENTROPIUS                   = 25840,
    NPC_VOID_SENTINEL               = 25772,
    NPC_VOID_SPAWN                  = 25824,
    NPC_BERSERKER                   = 25798,
    NPC_FURY_MAGE                   = 25799,
    NPC_DARK_FIEND                  = 25744,  // 暗魔（穆鲁黑暗召唤的小怪）
    NPC_SINGULARITY                 = 25855,  // 奇点/黑洞（恩特罗皮乌斯阶段）

    // 基尔加丹
    NPC_KILJAEDEN                   = 25315,
    NPC_HAND_OF_THE_DECEIVER        = 25588,  // 欺骗者之手（基尔加丹P1 ADD）
    NPC_SINISTER_REFLECTION         = 25708,  // 邪恶映像
    NPC_SHIELD_ORB                  = 25502,  // 护盾宝珠
    NPC_ARMAGEDDON_TARGET           = 25735,  // 末日审判目标标记
};

// 太阳之井高地物品ID
enum class SunwellItems : uint32
{
    // 暂无特殊物品需求
};

// 太阳之井高地游戏对象ID
enum class SunwellObjects : uint32
{
    GO_SPECTRAL_RIFT                = 187355,  // 幽灵裂缝（卡雷苟斯传送门视觉）
    GO_ICE_BARRIER                  = 188119,  // 冰屏障（布鲁塔卢斯场地入口）
    GO_FORCE_FIELD                  = 188421,  // 力场
    GO_FIRE_BARRIER                 = 188075,  // 火焰屏障
    GO_ORB_OF_THE_BLUE_DRAGONFLIGHT1 = 187869, // 蓝龙宝珠1
    GO_ORB_OF_THE_BLUE_DRAGONFLIGHT2 = 188114, // 蓝龙宝珠2
    GO_ORB_OF_THE_BLUE_DRAGONFLIGHT3 = 188115, // 蓝龙宝珠3
    GO_ORB_OF_THE_BLUE_DRAGONFLIGHT4 = 188116, // 蓝龙宝珠4
};

// 太阳之井高地地图ID
constexpr uint32 SUNWELL_MAP_ID = 580;

// ===== 卡雷苟斯 - 坐标定义 =====
extern const Position KALECGOS_TANK_POSITION;
extern const Position KALECGOS_RANGED_CENTER;
extern const Position KALECGOS_PORTAL_POSITION;

// 卡雷苟斯阶段追踪器
extern std::unordered_map<uint32, time_t> kalecgosPhaseTimer;
extern std::unordered_map<ObjectGuid, bool> kalecgosHasEnteredSpectral;

// ===== 布鲁塔卢斯 - 坐标定义 =====
extern const Position BRUTALLUS_TANK_POSITION;
extern const Position BRUTALLUS_OFFTANK_POSITION;
extern const std::array<Position, 3> BRUTALLUS_RANGED_POSITIONS;

extern std::unordered_map<ObjectGuid, time_t> brutallusBurnTimer;

// ===== 菲米丝 - 坐标定义 =====
extern const Position FELMYST_TANK_POSITION;
extern const Position FELMYST_RANGED_CENTER;

extern std::unordered_map<uint32, time_t> felmystPhaseTimer;
int GetFelmystPhase(Unit* felmyst);

// ===== 艾瑞达双子 - 坐标定义 =====
extern const Position SACROLASH_TANK_POSITION;
extern const Position ALYTHESS_TANK_POSITION;
extern const Position TWINS_RANGED_POSITION;

extern std::unordered_map<uint32, int> twinsKillOrder;

// ===== 穆鲁 - 坐标定义 =====
extern const Position MURU_TANK_POSITION;
extern const Position MURU_MELEE_POSITION;
extern const Position MURU_RANGED_POSITION;
extern const Position MURU_VOID_SPAWN_POSITION;

extern std::unordered_map<uint32, time_t> muruPhaseTimer;

// ===== 基尔加丹 - 坐标定义 =====
extern const Position KILJAEDEN_TANK_POSITION;
extern const Position KILJAEDEN_RANGED_CENTER;
extern const Position KILJAEDEN_SAFE_POSITION;

extern std::unordered_map<uint32, int> kiljaedenLastPhase;
extern std::unordered_map<uint32, time_t> kiljaedenPhaseTimer;

int GetKiljaedenPhase(Unit* kiljaeden);

// ===== 辅助函数 =====

// 检查机器人是否在幽灵领域中（卡雷苟斯战斗）
bool IsInSpectralRealm(Player* bot);

// 获取卡雷苟斯内外场血量差异（正值=外场低，负值=内场低）
// 返回FLT_MAX如果无法获取任一BOSS血量
float GetKalecgosHealthDifference(PlayerbotAI* botAI, Player* bot);

// 检查是否需要压低外场BOSS血量（外场血量明显高于内场）
bool NeedPushOuterRealmHealth(PlayerbotAI* botAI, Player* bot);

// 检查是否需要压低内场BOSS血量（内场血量明显高于外场）
bool NeedPushInnerRealmHealth(PlayerbotAI* botAI, Player* bot);

// 检查双方是否都接近狂暴阶段（都低于15%）
bool IsKalecgosEnrageImminent(PlayerbotAI* botAI, Player* bot);

// 获取机器人奥术冲击debuff叠加层数
uint32 GetArcaneBuffetStacks(Player* bot);

// 检查机器人是否需要因奥术冲击层数过高而进入幽灵领域
bool NeedEnterSpectralForArcaneBuffet(Player* bot, uint32 threshold = 4);

// 检查附近队友是否有无尽痛苦诅咒需要驱散
bool HasCurseOfBoundlessAgonyNearby(Player* bot);

// 获取被燃烧debuff影响的玩家列表（布鲁塔卢斯战斗）
std::vector<Player*> GetPlayersWithBurn(Player* bot);

// 检查是否有附近的包裹危险区域（菲米丝战斗）
bool HasEncapsulateNearby(PlayerbotAI* botAI, Player* bot);

// 获取最近的虚空哨兵（穆鲁战斗）
Unit* GetNearestVoidSentinel(PlayerbotAI* botAI, Player* bot);

// 检查基尔加丹是否在施放千魂之暗
bool IsKiljaedenCastingDarkness(Unit* kiljaeden);

// 获取基尔加丹的护盾宝珠
Unit* GetShieldOrb(PlayerbotAI* botAI, Player* bot);

// 获取邪恶映像列表
std::vector<Unit*> GetSinisterReflections(PlayerbotAI* botAI, Player* bot);

}  // namespace SunwellPlateauHelpers

#endif
