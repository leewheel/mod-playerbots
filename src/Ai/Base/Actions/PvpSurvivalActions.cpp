/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PvpSurvivalActions.h"
#include "Event.h"
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
//   战士无硬控，用恫吓怒吼(范围恐惧)与断筋(减速)逃生；
//   术士的死亡缠绕兼具恐惧与自疗，是"控制期间打绷带"手法的最佳搭档，排在恐惧之前。
// End By leewheel
constexpr uint32 CC_TABLE[][8] = {
    // CLASS_WARRIOR: 恫吓怒吼 / 断筋
    {CLASS_WARRIOR, 5246, 1715, 0, 0, 0, 0, 0},
    // CLASS_PALADIN: 制裁之锤 / 忏悔(天赋)
    {CLASS_PALADIN, 853, 20066, 0, 0, 0, 0, 0},
    // CLASS_HUNTER: 驱散射击 / 震荡射击(减速)
    {CLASS_HUNTER, 19503, 5116, 0, 0, 0, 0, 0},
    // CLASS_ROGUE: 致盲 / 凿击
    {CLASS_ROGUE, 2094, 1776, 0, 0, 0, 0, 0},
    // CLASS_PRIEST: 心灵尖啸
    {CLASS_PRIEST, 8122, 0, 0, 0, 0, 0, 0},
    // CLASS_SHAMAN: 妖术 / 冰霜震击(减速)
    {CLASS_SHAMAN, 51514, 8056, 0, 0, 0, 0, 0},
    // CLASS_MAGE: 冰霜新星(定身) / 变形术
    {CLASS_MAGE, 122, 118, 0, 0, 0, 0, 0},
    // CLASS_WARLOCK: 死亡缠绕(恐惧+自疗) / 恐惧 / 恐怖嚎叫
    {CLASS_WARLOCK, 6789, 5782, 5484, 0, 0, 0, 0},
    // CLASS_DRUID: 缠绕 / 旋风(天赋)
    {CLASS_DRUID, 339, 33786, 0, 0, 0, 0, 0},
    // CLASS_DEATH_KNIGHT: 冰霜之链(减速) / 饥饿之寒(天赋冰冻)
    {CLASS_DEATH_KNIGHT, 45524, 51209, 0, 0, 0, 0, 0},
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
    // 血量低于 60% 才考虑打绷带
    if (bot->GetHealthPct() > 60.f)
        return false;

    // 打绷带条件：近战范围内无敌对玩家（安全窗口，绷带会被伤害打断）且背包里有绷带
    return FindNearestEnemyPlayerForSurvival(botAI, 12.f) == nullptr && FindBandage() != nullptr;
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
