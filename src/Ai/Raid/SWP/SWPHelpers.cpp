//By leewheel 2026-07-08
/*
 * 太阳之井高地 (Sunwell Plateau) 策略辅助函数实现
 * 作者: leewheel
 */
//By leewheel 2026-07-09 修复AI_VALUE2宏在自由函数中不可用的问题，改用botAI->GetAiObjectContext()直接访问
//                修复using namespace导致的函数重载歧义，改为namespace块包裹定义（与其他RaidHelper一致）
//                修复SPELL_ENCAPSULATE枚举名错误，改为SPELL_ENCAPSULATE_CHANNEL
//End By leewheel 2026-07-09
//End By leewheel

#include "SWPHelpers.h"

#include "Playerbots.h"
#include "RaidBossHelpers.h"

namespace SunwellPlateauHelpers
{

// ===== 卡雷苟斯 - 坐标定义 =====
const Position KALECGOS_TANK_POSITION      = { 1704.0f, 930.0f, -74.5f, 0.0f };
const Position KALECGOS_RANGED_CENTER      = { 1700.0f, 950.0f, -74.5f, 0.0f };
const Position KALECGOS_PORTAL_POSITION    = { 1705.0f, 940.0f, -74.5f, 0.0f };

std::unordered_map<uint32, time_t> kalecgosPhaseTimer;
std::unordered_map<ObjectGuid, bool> kalecgosHasEnteredSpectral;

// ===== 布鲁塔卢斯 - 坐标定义 =====
const Position BRUTALLUS_TANK_POSITION     = { 1450.0f, 600.0f, 18.0f, 0.0f };
const Position BRUTALLUS_OFFTANK_POSITION  = { 1450.0f, 615.0f, 18.0f, 0.0f };
const std::array<Position, 3> BRUTALLUS_RANGED_POSITIONS = {
    Position{ 1470.0f, 580.0f, 18.0f, 0.0f },
    Position{ 1470.0f, 620.0f, 18.0f, 0.0f },
    Position{ 1470.0f, 600.0f, 18.0f, 0.0f }
};

std::unordered_map<ObjectGuid, time_t> brutallusBurnTimer;

// ===== 菲米丝 - 坐标定义 =====
const Position FELMYST_TANK_POSITION       = { 1450.0f, 700.0f, 18.0f, 0.0f };
const Position FELMYST_RANGED_CENTER       = { 1450.0f, 670.0f, 18.0f, 0.0f };

std::unordered_map<uint32, time_t> felmystPhaseTimer;

// ===== 艾瑞达双子 - 坐标定义 =====
const Position SACROLASH_TANK_POSITION     = { 1800.0f, 600.0f, 75.0f, 0.0f };
const Position ALYTHESS_TANK_POSITION      = { 1800.0f, 650.0f, 75.0f, 0.0f };
const Position TWINS_RANGED_POSITION       = { 1800.0f, 625.0f, 75.0f, 0.0f };

std::unordered_map<uint32, int> twinsKillOrder;

// ===== 穆鲁 - 坐标定义 =====
const Position MURU_TANK_POSITION          = { 1800.0f, 400.0f, 0.0f, 0.0f };
const Position MURU_MELEE_POSITION         = { 1800.0f, 420.0f, 0.0f, 0.0f };
const Position MURU_RANGED_POSITION        = { 1780.0f, 400.0f, 0.0f, 0.0f };
const Position MURU_VOID_SPAWN_POSITION    = { 1800.0f, 380.0f, 0.0f, 0.0f };

std::unordered_map<uint32, time_t> muruPhaseTimer;

// ===== 基尔加丹 - 坐标定义 =====
const Position KILJAEDEN_TANK_POSITION     = { 1700.0f, 350.0f, 28.0f, 0.0f };
const Position KILJAEDEN_RANGED_CENTER     = { 1700.0f, 380.0f, 28.0f, 0.0f };
const Position KILJAEDEN_SAFE_POSITION     = { 1720.0f, 400.0f, 28.0f, 0.0f };

std::unordered_map<uint32, int> kiljaedenLastPhase;
std::unordered_map<uint32, time_t> kiljaedenPhaseTimer;

// ===== 入口区域 - 坐标定义 =====
// 坐标1: 门口第一群怪的位置（坦克引怪点）
const Position SWP_ENTRANCE_TRASH_GROUP1_POS = { 1655.14f, 923.453f, 15.6266f, 0.0f };
// 坐标2: 坦克拉怪目的地 / 团队等待参战位置
const Position SWP_ENTRANCE_ENGAGE_POS = { 1720.8975f, 876.3326f, 15.572174f, 0.0f };

// ===== 辅助函数实现 =====

// 获取菲米丝当前阶段（0=地面阶段, 1=飞行阶段）
// Acore中没有专门的飞行光环，通过检查菲米丝是否在近战范围内来判断
int GetFelmystPhase(Unit* felmyst)
{
    if (!felmyst)
        return 0;

    // 飞行阶段时菲米丝不在近战范围内（无法被近战攻击）
    // 检查菲米丝是否有受害者且不在近战范围内
    Unit* victim = felmyst->GetVictim();
    if (victim && !felmyst->IsWithinMeleeRange(victim))
        return 1;

    // 检查菲米丝是否在空中（Z坐标明显高于地面）
    // 菲米丝地面阶段Z约在18附近，飞行阶段会显著升高
    if (felmyst->GetPositionZ() > 30.0f)
        return 1;

    return 0;
}

// 检查机器人是否在幽灵领域中（卡雷苟斯战斗）
bool IsInSpectralRealm(Player* bot)
{
    return bot && bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_SPECTRAL_REALM));
}

// 获取被燃烧debuff影响的玩家列表（布鲁塔卢斯战斗）
std::vector<Player*> GetPlayersWithBurn(Player* bot)
{
    std::vector<Player*> result;
    if (!bot)
        return result;

    Group* group = bot->GetGroup();
    if (!group)
        return result;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
            continue;

        if (member->HasAura(static_cast<uint32>(SunwellSpells::SPELL_BURN_DAMAGE)))
            result.push_back(member);
    }

    return result;
}

// 检查是否有附近的包裹危险区域（菲米丝战斗）
bool HasEncapsulateNearby(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !bot)
        return false;

    // 查找正在施放包裹的菲米丝
    // By leewheel 2026-07-09 自由函数中不能使用AI_VALUE2宏，改用botAI->GetAiObjectContext()直接访问
    Unit* felmyst = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "felmyst")->Get();
    // End By leewheel 2026-07-09
    if (!felmyst)
        return false;

    // 检查菲米丝是否正在施放包裹法术（通道法术，用FindCurrentSpellBySpellId检测）
    if (felmyst->FindCurrentSpellBySpellId(static_cast<uint32>(SunwellSpells::SPELL_ENCAPSULATE_CHANNEL)))
    {
        // 如果菲米丝的目标是附近玩家，或自己在范围内
        if (felmyst->GetVictim() && bot->GetExactDist2d(felmyst->GetVictim()) < 20.0f)
            return true;
    }

    return false;
}

// 获取最近的虚空哨兵（穆鲁战斗）
Unit* GetNearestVoidSentinel(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !bot)
        return nullptr;

    // By leewheel 2026-07-09 自由函数中不能使用AI_VALUE2宏
    return botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "void sentinel")->Get();
    // End By leewheel 2026-07-09
}

// 检查基尔加丹是否在施放千魂之暗
bool IsKiljaedenCastingDarkness(Unit* kiljaeden)
{
    if (!kiljaeden)
        return false;

    return kiljaeden->FindCurrentSpellBySpellId(
        static_cast<uint32>(SunwellSpells::SPELL_DARKNESS_OF_SOULS)) != nullptr;
}

// 获取基尔加丹的护盾宝珠
Unit* GetShieldOrb(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !bot)
        return nullptr;

    // By leewheel 2026-07-09 自由函数中不能使用AI_VALUE2宏
    return botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "shield orb")->Get();
    // End By leewheel 2026-07-09
}

// 获取邪恶映像列表
std::vector<Unit*> GetSinisterReflections(PlayerbotAI* botAI, Player* bot)
{
    std::vector<Unit*> result;
    if (!botAI || !bot)
        return result;

    auto const& npcs =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();
    for (auto const& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (unit && unit->IsAlive() &&
            unit->GetEntry() == static_cast<uint32>(SunwellNpcs::NPC_SINISTER_REFLECTION))
        {
            result.push_back(unit);
        }
    }

    return result;
}

// 获取基尔加丹当前阶段
int GetKiljaedenPhase(Unit* kiljaeden)
{
    if (!kiljaeden)
        return 0;

    // 根据血量判断阶段
    float hp = kiljaeden->GetHealthPct();
    if (hp > 85.0f)
        return 0;  // 阶段1
    else if (hp > 55.0f)
        return 1;  // 阶段2
    else if (hp > 25.0f)
        return 2;  // 阶段3
    else
        return 3;  // 阶段4（最终）
}

// ===== 卡雷苟斯辅助函数实现 =====

// 获取卡雷苟斯内外场血量差异（正值=外场高，负值=内场高）
// 返回FLT_MAX如果无法获取任一BOSS血量
float GetKalecgosHealthDifference(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !bot)
        return FLT_MAX;

    // By leewheel 2026-07-09 自由函数中不能使用AI_VALUE2宏，改用botAI->GetAiObjectContext()直接访问
    Unit* kalecgos = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "kalecgos")->Get();
    Unit* sathrovarr = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "sathrovarr the corruptor")->Get();
    // End By leewheel 2026-07-09

    if (!kalecgos || !sathrovarr)
        return FLT_MAX;

    // 正值=外场(卡雷苟斯)血量更高，负值=内场(萨斯罗瓦尔)血量更高
    return kalecgos->GetHealthPct() - sathrovarr->GetHealthPct();
}

// 检查是否需要压低外场BOSS血量（外场血量明显高于内场）
bool NeedPushOuterRealmHealth(PlayerbotAI* botAI, Player* bot)
{
    float diff = GetKalecgosHealthDifference(botAI, bot);
    if (diff == FLT_MAX)
        return false;

    // 外场血量比内场高10%以上时需要压低外场
    return diff > 10.0f;
}

// 检查是否需要压低内场BOSS血量（内场血量明显高于外场）
bool NeedPushInnerRealmHealth(PlayerbotAI* botAI, Player* bot)
{
    float diff = GetKalecgosHealthDifference(botAI, bot);
    if (diff == FLT_MAX)
        return false;

    // 内场血量比外场高10%以上时需要压低内场
    return diff < -10.0f;
}

// 检查双方是否都接近狂暴阶段（都低于15%）
// 注：Acore源码中双方在10%时互触CRAZED_RAGE(44807)狂暴
bool IsKalecgosEnrageImminent(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !bot)
        return false;

    // By leewheel 2026-07-09 自由函数中不能使用AI_VALUE2宏，改用botAI->GetAiObjectContext()直接访问
    Unit* kalecgos = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "kalecgos")->Get();
    Unit* sathrovarr = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "sathrovarr the corruptor")->Get();
    // End By leewheel 2026-07-09

    if (!kalecgos || !sathrovarr)
        return false;

    // 狂暴在10%触发，15%作为预警阈值
    return kalecgos->GetHealthPct() <= 15.0f && sathrovarr->GetHealthPct() <= 15.0f;
}

// 获取机器人奥术冲击debuff叠加层数
uint32 GetArcaneBuffetStacks(Player* bot)
{
    if (!bot)
        return 0;

    Aura* aura = bot->GetAura(static_cast<uint32>(SunwellSpells::SPELL_ARCANE_BUFFET));
    if (!aura)
        return 0;

    return aura->GetStackAmount();
}

// 检查机器人是否需要因奥术冲击层数过高而进入幽灵领域
// 进入幽灵领域后奥术冲击debuff会被清除（换位面）
bool NeedEnterSpectralForArcaneBuffet(Player* bot, uint32 threshold)
{
    if (!bot)
        return false;

    // 已在幽灵领域中则不需要
    if (IsInSpectralRealm(bot))
        return false;

    // 有幽灵力竭debuff则不能进入
    if (bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_SPECTRAL_EXHAUSTION)))
        return false;

    return GetArcaneBuffetStacks(bot) >= threshold;
}

// 检查附近队友是否有无尽痛苦诅咒需要驱散
// 该诅咒驱散后会转移给邻近玩家(45034)，需专人驱散
bool HasCurseOfBoundlessAgonyNearby(Player* bot)
{
    if (!bot)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
            continue;

        // 检查原始诅咒(45032)和转移版本(45034)
        if (member->HasAura(static_cast<uint32>(SunwellSpells::SPELL_CURSE_OF_BOUNDLESS_AGONY)) ||
            member->HasAura(static_cast<uint32>(SunwellSpells::SPELL_CURSE_OF_BOUNDLESS_AGONY_PLR)))
        {
            if (bot->GetExactDist2d(member) < 30.0f)
                return true;
        }
    }

    return false;
}

// ===== 入口小怪辅助函数实现 =====

// 检查NPC entry是否为SWP入口小怪
bool IsSunwellEntranceTrash(uint32 entry)
{
    switch (entry)
    {
        case static_cast<uint32>(SunwellTrashNpcs::NPC_SUNBLADE_CABALIST):
        case static_cast<uint32>(SunwellTrashNpcs::NPC_SUNBLADE_ARCH_MAGE):
        case static_cast<uint32>(SunwellTrashNpcs::NPC_SUNBLADE_SLAYER):
        case static_cast<uint32>(SunwellTrashNpcs::NPC_SUNBLADE_VINDICATOR):
        case static_cast<uint32>(SunwellTrashNpcs::NPC_SUNBLADE_DUSK_PRIEST):
        case static_cast<uint32>(SunwellTrashNpcs::NPC_SUNBLADE_DAWN_PRIEST):
        case static_cast<uint32>(SunwellTrashNpcs::NPC_SUNBLADE_SCOUT):
        case static_cast<uint32>(SunwellTrashNpcs::NPC_SUNBLADE_PROTECTOR):
            return true;
        default:
            return false;
    }
}

// 检查是否有存活的入口小怪
bool HasAliveEntranceTrash(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !bot)
        return false;

    auto const& npcs =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();
    for (auto const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive() && IsSunwellEntranceTrash(unit->GetEntry()))
            return true;
    }

    return false;
}

// 获取最近的存活入口小怪
Unit* GetNearestAliveEntranceTrash(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !bot)
        return nullptr;

    Unit* nearest = nullptr;
    float bestDist = std::numeric_limits<float>::max();

    auto const& npcs =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();
    for (auto const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        if (!IsSunwellEntranceTrash(unit->GetEntry()))
            continue;

        float dist = bot->GetExactDist2d(unit);
        if (dist < bestDist)
        {
            bestDist = dist;
            nearest = unit;
        }
    }

    return nearest;
}

// 检查是否有入口小怪已经到达参战位置附近
bool HasTrashNearEngagePos(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !bot)
        return false;

    auto const& npcs =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();
    for (auto const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        if (!IsSunwellEntranceTrash(unit->GetEntry()))
            continue;

        float dist = unit->GetExactDist2d(
            SWP_ENTRANCE_ENGAGE_POS.GetPositionX(),
            SWP_ENTRANCE_ENGAGE_POS.GetPositionY());
        if (dist <= SWP_ENGAGE_POS_RADIUS)
            return true;
    }

    return false;
}

// 检查是否有任何BOSS正在被战斗
bool IsAnySwBossPresent(PlayerbotAI* botAI)
{
    if (!botAI)
        return false;

    static const char* bossNames[] = {
        "kalecgos",
        "brutallus",
        "felmyst",
        "lady sacrolash",
        "grand warlock alythess",
        "muru",
        "entropius",
        "kil'jaeden"
    };

    for (const char* name : bossNames)
    {
        Unit* boss = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", name)->Get();
        if (boss)
            return true;
    }

    return false;
}

// 检查是否有尚未复活的死亡队友（幽灵状态）
bool HasDeadPartyMemberNotResurrected(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !bot)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot)
            continue;

        // 检查是否死亡且尚未接受复活
        if (!member->IsAlive() && !member->isResurrectRequested())
            return true;
    }

    return false;
}

}  // namespace SunwellPlateauHelpers
