/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PvpSurvivalActions.h"
#include "Event.h"
#include "ItemCountValue.h"
#include "Playerbots.h"

// By leewheel 2026-08-29
// PVP 自保动作组实现。
//   设计要点：
//   1) 控制/减速技能全部使用 rank1 entry 定义，运行时经法术链(SpellChainNode)匹配 bot 已学的最高等级；
//   2) 绷带物品全部使用 entry，从高等级到低等级查找，避免依赖英文名；
//   3) 动作间按触发器优先级衔接：濒死撤退 > 低血量控制 > 安全绷带。
// End By leewheel

namespace
{
// By leewheel 2026-08-29
// 各职业 PVP 自保技能表（rank1 entry，硬控在前、减速/辅助在后）：
//   释放时按顺序尝试，第一个已学且可施放的生效。
// By leewheel 2026-09-01
//   扩充为 NPCBots 全职业 _spells_cc 清单（bot_warrior_ai.cpp:169 / bot_warlock_ai.cpp:180 /
//   bot_hunter_ai.cpp:169 / bot_death_knight_ai.cpp:173 / bot_druid_ai.cpp 等），
//   硬控（眩晕/恐惧/冰冻/变形/沉睡）在前，减速垫后；位移接敌类（冲锋/拦截/死亡之握）不列入逃生表。
//   全部 entry 已经 chs_dbc.db_spell_12340_eng 逐一验证（含 Mechanic 列核对）。
// End By leewheel
constexpr uint32 CC_TABLE[][8] = {
    // CLASS_WARRIOR: 冲击波(天赋眩晕) / 恫吓怒吼(范围恐惧) / 震荡猛击(天赋眩晕) / 断筋 / 穿刺哀嚎(减速)
    {CLASS_WARRIOR, 46968, 5246, 22427, 1715, 10576, 0, 0},
    // CLASS_PALADIN: 制裁之锤 / 忏悔(天赋) / 神圣愤怒(眩晕亡灵恶魔) / 驱散邪恶(恐惧亡灵恶魔)
    {CLASS_PALADIN, 853, 20066, 2812, 10326, 0, 0, 0},
    // CLASS_HUNTER: 冰冻陷阱 / 蝎毒(沉睡) / 驱散射击(眩晕) / 震荡射击(减速) / 翼击(减速)
    {CLASS_HUNTER, 3355, 19386, 19503, 5116, 2974, 0, 0},
    // CLASS_ROGUE: 致盲 / 肾击(需连击点) / 凿击 / 闷棍(需潜行) / 脚踢(打断)
    {CLASS_ROGUE, 2094, 408, 1776, 2070, 1766, 0, 0},
    // CLASS_PRIEST: 心灵尖啸(恐惧) / 束缚亡灵(亡灵怪定身)
    {CLASS_PRIEST, 8122, 9484, 0, 0, 0, 0, 0},
    // CLASS_SHAMAN: 妖术(变形) / 冰霜震击(减速)
    {CLASS_SHAMAN, 51514, 8056, 0, 0, 0, 0, 0},
    // CLASS_MAGE: 冰霜新星(定身) / 变形术 / 龙息术(天赋沉默)
    {CLASS_MAGE, 122, 118, 31661, 0, 0, 0, 0},
    // CLASS_WARLOCK: 死亡缠绕(眩晕+自疗) / 恐惧 / 恐怖嚎叫(群体恐惧) / 放逐(恶魔元素) / 语言诅咒(减速)
    {CLASS_WARLOCK, 6789, 5782, 5484, 710, 1714, 0, 0},
    // CLASS_DRUID: 缠绕(定身) / 旋风(天赋浮空) / 台风(天赋击退) / 熊锤(眩晕,巨熊形态) / 休眠(人形野兽龙类) / 猫扑(眩晕)
    {CLASS_DRUID, 339, 33786, 50516, 5211, 2637, 9005, 0},
    // CLASS_DEATH_KNIGHT: 心灵冰冻(打断) / 绞杀(天赋沉默) / 冰霜之链(减速) / 饥饿之寒(天赋冰冻)
    {CLASS_DEATH_KNIGHT, 47528, 48680, 45524, 51209, 0, 0, 0},
};

constexpr size_t CC_TABLE_SIZE = sizeof(CC_TABLE) / sizeof(CC_TABLE[0]);
constexpr size_t CC_TABLE_COLS = 8;

// By leewheel 2026-08-29
// 绷带 entry 表（从 80 级可用的最高等级到最低等级）：
//   厚灵纹 / 灵纹 / 厚魔纹 / 魔纹 / 厚丝质 / 丝质 / 厚亚麻 / 丝线 / 亚麻
// End By leewheel
constexpr uint32 BANDAGE_ENTRIES[] = {34721, 21990, 14529, 8545, 6451, 6450, 3530, 2581, 1251};
constexpr size_t BANDAGE_COUNT = sizeof(BANDAGE_ENTRIES) / sizeof(BANDAGE_ENTRIES[0]);
}  // namespace

Unit* FindNearestEnemyPlayerForSurvival(PlayerbotAI* botAI, float range)
{
    Player* bot = botAI->GetBot();
    GuidVector enemies = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest enemy players")->Get();
    Unit* nearest = nullptr;
    float bestDist = range;
    for (ObjectGuid const& guid : enemies)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        float dist = bot->GetDistance(unit);
        if (dist < bestDist)
        {
            bestDist = dist;
            nearest = unit;
        }
    }

    return nearest;
}

uint32 CastCcEscapeAction::FindKnownTopRank(uint32 baseEntry)
{
    // bot 实际学会的等级就是它能用的最高等级：
    // 通过链节点从全链最高等级向下找第一个已学的
    SpellInfo const* base = sSpellMgr->GetSpellInfo(baseEntry);
    if (!base)
        return 0;

    SpellChainNode const* node = sSpellMgr->GetSpellChainNode(baseEntry);
    if (!node)
        return bot->HasSpell(baseEntry) ? baseEntry : 0;

    // 从链尾（最高等级）向链头回溯，找 bot 已学的最高等级（链节点成员是 SpellInfo 指针）
    SpellInfo const* cur = node->last;
    while (cur)
    {
        if (bot->HasSpell(cur->Id))
            return cur->Id;

        cur = cur->ChainEntry ? cur->ChainEntry->prev : nullptr;
    }

    return 0;
}

bool CastCcEscapeAction::isUseful()
{
    // 血量低于 40% 且近身有敌方玩家（12 码）才进入控制逃生
    if (bot->GetHealthPct() > 40.f)
        return false;

    return FindNearestEnemyPlayerForSurvival(botAI, 12.f) != nullptr;
}

bool CastCcEscapeAction::Execute(Event /*event*/)
{
    Unit* enemy = FindNearestEnemyPlayerForSurvival(botAI, 12.f);
    if (!enemy)
        return false;

    // 在本职业的自保技能表里按顺序尝试
    for (size_t row = 0; row < CC_TABLE_SIZE; ++row)
    {
        if (CC_TABLE[row][0] != bot->getClass())
            continue;

        for (size_t col = 1; col < CC_TABLE_COLS; ++col)
        {
            uint32 baseEntry = CC_TABLE[row][col];
            if (!baseEntry)
                break;

            uint32 spellId = FindKnownTopRank(baseEntry);
            if (!spellId)
                continue;

            // CanCastSpell 校验冷却/资源/施法条件，通过即施放
            if (botAI->CanCastSpell(spellId, enemy, false))
            {
                if (botAI->CastSpell(spellId, enemy))
                {
                    LOG_DEBUG("playerbots", "PVP 自保：机器人 {} 对 {} 施放控制/逃生技能 (entry: {})",
                              bot->GetName(), enemy->GetName(), spellId);
                    return true;
                }
            }
        }
        break;  // 找到本职业行后不再继续
    }

    return false;
}

bool UseBandageInPvpAction::isUseful()
{
    // By leewheel 2026-09-01
    // 交战循环增强：血线从 60% 提到 80%（远遁后尽早回血）；
    // 安全判定从"12 码无敌"升级为"25 码内敌对玩家全部被硬控"——
    // 恐惧/变形/沉睡中的敌人不会来打断绷带，正是控制远遁后的恢复窗口
    // （NPCBots 无绷带机制，此为老大需求的自研超越点）。
    // End By leewheel
    if (bot->GetHealthPct() > 80.f)
        return false;

    if (!FindBandage())
        return false;

    constexpr uint64 locMask = IMMUNE_TO_MOVEMENT_IMPAIRMENT_AND_LOSS_CONTROL_MASK & ~(1ULL << MECHANIC_SNARE);
    GuidVector enemies = AI_VALUE(GuidVector, "nearest enemy players");
    for (ObjectGuid const& guid : enemies)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive() && bot->GetDistance(unit) < 25.f && !unit->HasAuraWithMechanic(locMask))
            return false;  // 近身有未被控的敌人，绷带会被打断，不打
    }

    return true;
}

Item* UseBandageInPvpAction::FindBandage()
{
    // 从高等级绷带到低等级遍历，找背包里第一个可用的
    for (size_t i = 0; i < BANDAGE_COUNT; ++i)
    {
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(BANDAGE_ENTRIES[i]);
        if (!proto)
            continue;

        std::vector<Item*> found = AI_VALUE2(std::vector<Item*>, "inventory items", chat->FormatItem(proto));
        // By leewheel 2026-09-04 防悬空崩溃: 过滤缓存列表中已失效的物品指针
        // End By leewheel
        found = InventoryItemValueBase::FilterLive(bot, found);
        if (!found.empty())
            return *found.begin();
    }

    return nullptr;
}

bool UseBandageInPvpAction::Execute(Event /*event*/)
{
    Item* bandage = FindBandage();
    if (!bandage)
        return false;

    // 复用 UseItemAction 的完整物品使用流程（含引导法术处理）
    return UseItem(bandage, ObjectGuid::Empty, nullptr, bot);
}

bool PvpRetreatAction::isUseful()
{
    // 血量低于 25% 且 15 码内敌方玩家 >= 2（被围攻）才触发撤退
    if (bot->GetHealthPct() > 25.f)
        return false;

    GuidVector enemies = AI_VALUE(GuidVector, "nearest enemy players");
    uint8 nearCount = 0;
    for (ObjectGuid const& guid : enemies)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive() && bot->GetDistance(unit) < 15.f)
            ++nearCount;
    }

    return nearCount >= 2;
}

bool PvpRetreatAction::Execute(Event /*event*/)
{
    // 计算所有近身敌方玩家的质心，向反方向强制移动 40 码（拉开距离等冷却/寻找机会打绷带）
    float sumX = 0.f, sumY = 0.f;
    uint8 count = 0;
    GuidVector enemies = AI_VALUE(GuidVector, "nearest enemy players");
    for (ObjectGuid const& guid : enemies)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        sumX += unit->GetPositionX();
        sumY += unit->GetPositionY();
        ++count;
    }

    if (!count)
        return false;

    float cx = sumX / count;
    float cy = sumY / count;
    float dx = bot->GetPositionX() - cx;
    float dy = bot->GetPositionY() - cy;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len < 0.1f)
        return false;

    // 反方向取 40 码处的落点
    float destX = bot->GetPositionX() + (dx / len) * 40.f;
    float destY = bot->GetPositionY() + (dy / len) * 40.f;

    LOG_DEBUG("playerbots", "PVP 自保：机器人 {} 被围攻，向反方向撤退", bot->GetName());
    return MoveTo(bot->GetMapId(), destX, destY, bot->GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_FORCED);
}

// By leewheel 2026-09-01
// PVP 徽章解控实现（移植 NPCBots 被控解徽章机制并加入留牌反打优化）。
//   NPCBots 参考：bot_ai.cpp:11395-11400（被硬控 20%/tick 即中即交伪法术 42292）；
//   我们的优化：长 CC（变形术/放逐/沉睡，剩余>15s）且状态良好时留牌，等第二段连控再用。
// End By leewheel

namespace
{
// 解控饰品触发法术：移除所有限制移动与失去控制效果（部落/联盟 PVP 徽章饰品通用）
constexpr uint32 CC_BREAK_TRINKET_SPELL = 42292;

// 失去控制类机制清单（徽章可解且值得解的类型；减速 SNARE 不列入——用徽章解减速是浪费）
// By leewheel 2026-09-01: 按本框架 Mechanics 枚举（SharedDefines.h L1336）与
//   SpellInfo::HasEffectMechanic API 修正；对齐引擎为 42292 定义的
//   IMMUNE_TO_MOVEMENT_IMPAIRMENT_AND_LOSS_CONTROL_MASK（去掉 SNARE）。
// End By leewheel
constexpr Mechanics const LOC_MECHANICS[] = {
    MECHANIC_STUN, MECHANIC_FEAR, MECHANIC_ROOT, MECHANIC_CHARM, MECHANIC_SLEEP,
    MECHANIC_POLYMORPH, MECHANIC_BANISH, MECHANIC_DISORIENTED, MECHANIC_KNOCKOUT,
    MECHANIC_FREEZE, MECHANIC_HORROR, MECHANIC_DAZE, MECHANIC_SAPPED, MECHANIC_SHACKLE,
};
}  // namespace

uint32 UseCcbreakTrinketAction::GetLossOfControlRemainingMs() const
{
    uint32 longest = 0;
    Unit::AuraApplicationMap const& auras = bot->GetAppliedAuras();
    for (auto const& [guid, aurApp] : auras)
    {
        Aura* aura = aurApp ? aurApp->GetBase() : nullptr;
        if (!aura)
            continue;

        SpellInfo const* info = aura->GetSpellInfo();
        if (!info || info->IsPositive())
            continue;

        for (Mechanics mechanic : LOC_MECHANICS)
        {
            if (info->HasEffectMechanic(mechanic))
            {
                int32 remaining = aura->GetDuration();
                if (remaining > 0 && static_cast<uint32>(remaining) > longest)
                    longest = static_cast<uint32>(remaining);
                break;
            }
        }
    }

    return longest;
}

Item* UseCcbreakTrinketAction::FindCcbreakTrinket() const
{
    // 只认已装备的饰品栏（徽章必须戴着才有意义，背包里的不临时换）
    for (uint8 slot : { EQUIPMENT_SLOT_TRINKET1, EQUIPMENT_SLOT_TRINKET2 })
    {
        Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (!item)
            continue;

        ItemTemplate const* proto = item->GetTemplate();
        if (!proto)
            continue;

        for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
        {
            if (proto->Spells[i].SpellId == static_cast<int32>(CC_BREAK_TRINKET_SPELL))
                return item;
        }
    }

    return nullptr;
}

bool UseCcbreakTrinketAction::isUseful()
{
    // 1) 身上必须有失去控制类硬控，且剩余时间值得解（<1s 的残血 CC 浪费徽章）
    uint32 ccRemaining = GetLossOfControlRemainingMs();
    if (ccRemaining < 1000)
        return false;

    // 2) 必须戴着解控饰品
    if (!FindCcbreakTrinket())
        return false;

    // 3) 留牌反打逻辑（超越 NPCBots 即中即交）：
    //    长 CC（>15s：变形术/放逐/沉睡类）且自身状态良好（血>70%、围攻玩家<2）→ 留牌等窗口；
    //    短 CC（眩晕/恐惧/定身）或被围攻或血量偏低 → 立即解。
    if (ccRemaining > 15000 && bot->GetHealthPct() > 70.f)
    {
        uint8 attackingPlayers = 0;
        Unit::AttackerSet const& attackers = bot->getAttackers();
        for (Unit const* attacker : attackers)
        {
            if (attacker && attacker->IsPlayer())
                ++attackingPlayers;
        }
        if (attackingPlayers < 2)
            return false;
    }

    return true;
}

bool UseCcbreakTrinketAction::Execute(Event /*event*/)
{
    Item* trinket = FindCcbreakTrinket();
    if (!trinket)
        return false;

    // 复用父类物品使用协议流程（含饰品自身 CD 与类别 CD 跟踪）
    if (UseTrinket(trinket))
    {
        LOG_DEBUG("playerbots", "PVP 徽章解控：机器人 {} 使用解控饰品 (entry: {})", bot->GetName(),
                  trinket->GetEntry());
        return true;
    }

    return false;
}

// By leewheel 2026-09-01
// PVP 交战循环·控制远遁实现（老大核心需求的直接落地）。
//   参考 NPCBots：法师背身闪现（bot_mage_ai.cpp:727-731）、盗贼盲接消失重潜
//   （bot_rogue_ai.cpp:399-407）、术士恐惧吸血循环（bot_warlock_ai.cpp:374-391）。
// End By leewheel

namespace
{
// 职业远遁位移技能表（rank1 entry，已经 chs_dbc 验证）：
//   法师闪现 1953 / 盗贼消失 1856+疾跑 2983 / 猎人逃脱 781；
//   其余职业无战斗位移技能，走"控制命中后反向移动"兜底。
constexpr uint32 ESCAPE_TABLE[][4] = {
    {CLASS_MAGE, 1953, 0, 0},
    {CLASS_ROGUE, 1856, 2983, 0},
    {CLASS_HUNTER, 781, 0, 0},
};
constexpr size_t ESCAPE_TABLE_SIZE = sizeof(ESCAPE_TABLE) / sizeof(ESCAPE_TABLE[0]);

// 取 bot 在给定 rank1 entry 上已学的最高等级（与 CastCcEscapeAction 同逻辑）
uint32 TopKnownRankOf(Player* bot, uint32 baseEntry)
{
    SpellInfo const* base = sSpellMgr->GetSpellInfo(baseEntry);
    if (!base)
        return 0;

    SpellChainNode const* node = sSpellMgr->GetSpellChainNode(baseEntry);
    if (!node)
        return bot->HasSpell(baseEntry) ? baseEntry : 0;

    SpellInfo const* cur = node->last;
    while (cur)
    {
        if (bot->HasSpell(cur->Id))
            return cur->Id;

        cur = cur->ChainEntry ? cur->ChainEntry->prev : nullptr;
    }

    return 0;
}
}  // namespace

bool CastCcDisengageAction::CastHardCc(Unit* target)
{
    // 目标已在硬控中则不浪费 GCD（NPCBots 防叠加原则）
    for (Mechanics mechanic : LOC_MECHANICS)
    {
        if (target->HasAuraWithMechanic(1ULL << mechanic))
            return true;  // 已控视为成功，可以远遁
    }

    for (size_t row = 0; row < CC_TABLE_SIZE; ++row)
    {
        if (CC_TABLE[row][0] != bot->getClass())
            continue;

        for (size_t col = 1; col < CC_TABLE_COLS; ++col)
        {
            uint32 baseEntry = CC_TABLE[row][col];
            if (!baseEntry)
                break;

            // 只施放硬控：法术的 effect mechanic 必须命中失去控制清单（排除纯减速/沉默垫后项）
            SpellInfo const* base = sSpellMgr->GetSpellInfo(baseEntry);
            if (!base)
                continue;

            bool hardCc = false;
            for (Mechanics mechanic : LOC_MECHANICS)
            {
                if (base->HasEffectMechanic(mechanic))
                {
                    hardCc = true;
                    break;
                }
            }
            if (!hardCc)
                continue;

            uint32 spellId = TopKnownRankOf(bot, baseEntry);
            if (!spellId)
                continue;

            if (botAI->CanCastSpell(spellId, target, false) && botAI->CastSpell(spellId, target))
            {
                LOG_DEBUG("playerbots", "PVP 循环：机器人 {} 对 {} 施放硬控 (entry: {}) 准备远遁", bot->GetName(),
                          target->GetName(), spellId);
                return true;
            }
        }
        break;
    }

    return false;
}

bool CastCcDisengageAction::EscapeFrom(Unit* threat)
{
    // 1) 职业位移技能
    for (size_t row = 0; row < ESCAPE_TABLE_SIZE; ++row)
    {
        if (ESCAPE_TABLE[row][0] != bot->getClass())
            continue;

        for (size_t col = 1; col < 4; ++col)
        {
            uint32 baseEntry = ESCAPE_TABLE[row][col];
            if (!baseEntry)
                break;

            uint32 spellId = TopKnownRankOf(bot, baseEntry);
            if (!spellId || bot->HasSpellCooldown(spellId))
                continue;

            // 法师闪现：先转身背对威胁再施放（NPCBots 背身 180° 闪现，避免闪到敌人脸上）
            if (baseEntry == 1953 && threat)
                bot->SetOrientation(bot->GetAngle(threat) + static_cast<float>(M_PI));

            if (botAI->CanCastSpell(spellId, bot, false) && botAI->CastSpell(spellId, bot))
            {
                LOG_DEBUG("playerbots", "PVP 循环：机器人 {} 使用位移技能远遁 (entry: {})", bot->GetName(), spellId);
                return true;
            }
        }
        break;
    }

    // 2) 无位移技能或不可用：向远离威胁方向后撤 20 码
    if (threat && threat->IsAlive())
    {
        float destX = bot->GetPositionX() + std::cos(bot->GetAngle(threat) + static_cast<float>(M_PI)) * 20.f;
        float destY = bot->GetPositionY() + std::sin(bot->GetAngle(threat) + static_cast<float>(M_PI)) * 20.f;
        return MoveTo(bot->GetMapId(), destX, destY, bot->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}

bool CastCcDisengageAction::isUseful()
{
    //By leewheel 2026-09-01 复核修正: 同触发器侧，远程职业 GetVictim 恒空，优先取 AI 当前目标
    Unit* victim = AI_VALUE(Unit*, "current target");
    if (!victim || !victim->IsPlayer() || !victim->IsAlive())
        victim = bot->GetVictim();
    //End By leewheel
    if (!victim || !victim->IsPlayer() || !victim->IsAlive())
        return false;

    // 打不死（目标血量健康>20%）且自身状态不佳（血<70%）→ 控制远遁
    return victim->GetHealthPct() > 20.f && bot->GetHealthPct() < 70.f;
}

bool CastCcDisengageAction::Execute(Event /*event*/)
{
    //By leewheel 2026-09-01 复核修正: 远程职业 GetVictim 恒空，优先取 AI 当前目标（与 isUseful 一致）
    Unit* victim = AI_VALUE(Unit*, "current target");
    if (!victim || !victim->IsPlayer() || !victim->IsAlive())
        victim = bot->GetVictim();
    //End By leewheel
    if (!victim || !victim->IsPlayer() || !victim->IsAlive())
        return false;

    // 控制命中（或目标已被控）才远遁；控不上则原地继续打（不白跑）
    if (!CastHardCc(victim))
        return false;

    return EscapeFrom(victim);
}
