/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "TellLosAction.h"
#include "ChatHelper.h"
#include "Event.h"
#include "ItemTemplate.h"
#include "ObjectMgr.h"
#include "Playerbots.h"
#include "StatsWeightCalculator.h"
#include "World.h"
#include <charconv>
#include <iomanip>
#include <iterator>
#include <sstream>

bool TellLosAction::Execute(Event event)
{
    std::string const param = event.getParam();

    if (param.empty() || param == "targets")
    {
        ListUnits("--- 目标 ---", *context->GetValue<GuidVector>("possible targets"));
        ListUnits("--- 目标（全部）---", *context->GetValue<GuidVector>("all targets"));
    }

    if (param.empty() || param == "npcs")
    {
        ListUnits("--- NPC ---", *context->GetValue<GuidVector>("nearest npcs"));
    }

    if (param.empty() || param == "corpses")
    {
        ListUnits("--- 尸体 ---", *context->GetValue<GuidVector>("nearest corpses"));
    }

    if (param.empty() || param == "gos" || param == "game objects")
    {
        ListGameObjects("--- 游戏对象 ---", *context->GetValue<GuidVector>("nearest game objects"));
    }

    if (param.empty() || param == "players")
    {
        ListUnits("--- 友方玩家 ---", *context->GetValue<GuidVector>("nearest friendly players"));
    }

    if (param.empty() || param == "triggers")
    {
        ListUnits("--- 触发器 ---", *context->GetValue<GuidVector>("possible triggers"));
    }

    return true;
}

void TellLosAction::ListUnits(std::string const title, GuidVector units)
{
    botAI->TellMaster(title);

    for (ObjectGuid const guid : units)
    {
        if (Unit* unit = botAI->GetUnit(guid))
        {
            botAI->TellMaster(unit->GetNameForLocaleIdx(sWorld->GetDefaultDbcLocale()));
        }
    }
}
void TellLosAction::ListGameObjects(std::string const title, GuidVector gos)
{
    botAI->TellMaster(title);

    for (ObjectGuid const guid : gos)
    {
        if (GameObject* go = botAI->GetGameObject(guid))
            botAI->TellMaster(chat->FormatGameobject(go));
    }
}

bool TellAuraAction::Execute(Event /*event*/)
{
    botAI->TellMaster("--- 光环 ---");
    sLog->outMessage("playerbot", LOG_LEVEL_DEBUG, "--- 光环 ---");
    Unit::AuraApplicationMap& map = bot->GetAppliedAuras();
    for (Unit::AuraApplicationMap::iterator i = map.begin(); i != map.end(); ++i)
    {
        Aura* aura = i->second->GetBase();
        if (!aura)
            continue;
        const std::string auraName = aura->GetSpellInfo()->SpellName[0];
        sLog->outMessage("playerbot", LOG_LEVEL_DEBUG, "光环信息 - 名称: " + auraName);
        AuraObjectType type = aura->GetType();
        WorldObject* owner = aura->GetOwner();
        std::string owner_name = owner ? owner->GetName() : "未知";
        float distance = bot->GetDistance2d(owner);
        Unit* caster = aura->GetCaster();
        std::string caster_name = caster ? caster->GetName() : "未知";
        bool is_area = aura->IsArea();
        int32 duration = aura->GetDuration();
        int32 spellId = aura->GetSpellInfo()->Id;
        bool isPositive = aura->GetSpellInfo()->IsPositive();
        sLog->outMessage("playerbot", LOG_LEVEL_DEBUG,
                         "光环信息 - 名称: " + auraName + " 施法者: " + caster_name + " 类型: " +
                             std::to_string(type) + " 所有者: " + owner_name + " 距离: " + std::to_string(distance) +
                             " 是否区域: " + std::to_string(is_area) + " 持续时间: " + std::to_string(duration) +
                             " 法术ID: " + std::to_string(spellId) + " 是否增益: " + std::to_string(isPositive));

        botAI->TellMaster("光环信息 - 名称: " + auraName + " 施法者: " + caster_name + " 类型: " +
                          std::to_string(type) + " 所有者: " + owner_name + " 距离: " + std::to_string(distance) +
                          " 是否区域: " + std::to_string(is_area) + " 持续时间: " + std::to_string(duration) +
                          " 法术ID: " + std::to_string(spellId) + " 是否增益: " + std::to_string(isPositive));

        if (type == DYNOBJ_AURA_TYPE)
        {
            DynamicObject* dyn_owner = aura->GetDynobjOwner();
            float radius = dyn_owner->GetRadius();
            int32 spellId = dyn_owner->GetSpellId();
            int32 duration = dyn_owner->GetDuration();
            sLog->outMessage("playerbot", LOG_LEVEL_DEBUG,
                             std::string("Info of DynamicObject -") + " name: " + dyn_owner->GetName() +
                                 " radius: " + std::to_string(radius) + " spell id: " + std::to_string(spellId) +
                                 " duration: " + std::to_string(duration));

            botAI->TellMaster(std::string("动态对象信息 -") + " 名称: " + dyn_owner->GetName() +
                              " 范围: " + std::to_string(radius) + " 法术 ID: " + std::to_string(spellId) +
                              " 持续时间: " + std::to_string(duration));
        }
    }
    return true;
}

bool TellEstimatedDpsAction::Execute(Event /*event*/)
{
    float dps = AI_VALUE(float, "estimated group dps");
    botAI->TellMaster("预估团队 DPS: " + std::to_string(dps));
    return true;
}

bool TellCalculateItemAction::Execute(Event event)
{
    std::string const text = event.getParam();
    ItemWithRandomProperty item = chat->parseItemWithRandomProperty(text);
    if (!item.itemId && text.find("Hitem:") == std::string::npos)
    {
        // Accept only a complete bare item id; atol() would accept a numeric prefix.
        char const* begin = text.c_str();
        while (*begin == ' ' || *begin == '\t')
            ++begin;

        char const* digitsEnd = begin;
        while (*digitsEnd >= '0' && *digitsEnd <= '9')
            ++digitsEnd;
        char const* tail = digitsEnd;
        while (*tail == ' ' || *tail == '\t')
            ++tail;

        uint32 parsedId = 0;
        auto const [ptr, ec] = std::from_chars(begin, digitsEnd, parsedId);
        if (begin == digitsEnd || *tail != '\0' || ec != std::errc() || ptr != digitsEnd || parsedId == 0)
            return false;

        item.itemId = parsedId;
    }

    StatsWeightCalculator calculator(bot);

    if (!item.itemId)
        return false;

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(item.itemId);
    if (!proto)
        return false;

    float score = calculator.CalculateItem(item.itemId, item.randomPropertyId);

    std::ostringstream out;
    out << chat->FormatItem(proto) << " 的计算评分：" << score;
    botAI->TellMasterNoFacing(out.str());

    // Per-stat breakdown (value * weight = contribution) so score tuning is verifiable.
    // Keep in sync with enum StatsType in StatsCollector.h.
    // clang-format off
    static char const* statNames[] = {
        "agility",   "strength",   "intellect",     "spirit",          "stamina",
        "hit",       "crit",       "haste",         "armor",           "defense",
        "dodge",     "parry",      "block_value",   "block_rating",    "resilience",
        "health_regen", "spell_power", "spell_penetration", "heal_power", "mana_regen",
        "attack_power", "armor_penetration", "expertise", "melee_dps", "ranged_dps",
        "bonus"
    };
    // clang-format on
    static_assert(std::size(statNames) == STATS_TYPE_MAX, "statNames must stay in sync with StatsType");

    StatsCollector const* collector = calculator.GetCollector();
    float const* weights = calculator.GetStatWeights();
    float sum = 0.0f;
    std::vector<std::string> lines;
    std::string line;
    for (uint32 i = 0; i < STATS_TYPE_MAX; ++i)
    {
        if (collector->stats[i] == 0.0f)
            continue;

        float const contribution = collector->stats[i] * weights[i];
        sum += contribution;

        std::ostringstream term;
        term << std::fixed << std::setprecision(2) << statNames[i] << " " << collector->stats[i] << " * " << weights[i]
             << " = " << contribution;

        // Split into several whispers so no line exceeds the client limit.
        if (!line.empty() && line.size() + 2 + term.str().size() > PlayerbotAI::MAX_CHAT_MESSAGE_BYTES)
        {
            lines.push_back(line);
            line.clear();
        }

        if (!line.empty())
            line += ", ";
        line += term.str();
    }

    if (line.empty())
        return true;

    lines.push_back(line);

    std::ostringstream sumText;
    std::ostringstream multText;
    sumText << std::fixed << std::setprecision(2) << sum;
    multText << std::fixed << std::setprecision(2) << (sum != 0.0f ? score / sum : 0.0f);

    std::map<std::string, std::string> placeholders;
    placeholders["%sum"] = sumText.str();
    placeholders["%mult"] = multText.str();
    botAI->TellMasterNoFacing(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "calc_item_breakdown", "Breakdown: weighted sum %sum, multipliers x%mult", placeholders));

    for (std::string const& chunk : lines)
        botAI->TellMasterNoFacing(chunk);

    return true;
}
