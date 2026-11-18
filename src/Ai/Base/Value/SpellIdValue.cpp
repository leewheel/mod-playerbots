/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SpellIdValue.h"
#include "ChatHelper.h"
#include "Playerbots.h"
#include "SpellNameIdFallback.h"
#include "Vehicle.h"
#include <algorithm>

namespace
{
// A name can match a rank chain and unrelated chainless spells, such as the Blacksmithing
// recipe "Corruption"; the chain always wins.
bool IsBetterRank(SpellInfo const* candidate, SpellInfo const* best)
{
    if (candidate->IsRanked() != best->IsRanked())
        return candidate->IsRanked();

    return candidate->GetRank() > best->GetRank();
}
}  // namespace

#include <cctype>

namespace
{
bool SpellNameMatches(SpellInfo const* spellInfo, std::wstring const& wnamepart, wchar_t firstSymbol)
{
    for (uint8 loc = 0; loc < MAX_LOCALES; ++loc)
    {
        char const* spellName = spellInfo->SpellName[loc];
        if (!spellName || !*spellName)
            continue;

        // By leewheel 2026-08-19
        // 性能优化：首字符快速失败。
        // 原实现先执行 Utf8FitTo（UTF-8 转小写匹配）再做 utf8to16 + wstrToLower + 首字符检查，
        // 对每个技能、每个语言都付出昂贵的 UTF-8 转换代价（采样证实 SpellIdValue::Calculate
        // 是 worldserver 前十大 CPU 热点，整条 utf8 转换链占比约 10%）。
        // 绝大多数技能名与目标名的首字符（ASCII 字母）不一致，先做一次 ASCII tolower 比较即可
        // 直接跳过，行为与原逻辑完全一致（大小写不敏感匹配语义不变）。
        // 仅在双方首字符均为 ASCII（<0x80）时才做快速比较；任一方为多字节字符仍走原 utf8 逻辑。
        unsigned char firstByte = static_cast<unsigned char>(spellName[0]);
        if (firstByte < 0x80 && firstSymbol < 0x80)
        {
            if (std::tolower(firstByte) != std::tolower(static_cast<unsigned char>(firstSymbol)))
                continue;
        }
        // End By leewheel

        if (!Utf8FitTo(spellName, wnamepart))
            continue;

        std::wstring wspellName;
        if (!Utf8toWStr(spellName, wspellName))
            continue;

        wstrToLower(wspellName);
        if (wspellName.empty() || wspellName[0] != firstSymbol || wspellName.length() != wnamepart.length())
            continue;

        return true;
    }

    return false;
}

std::string GetLocalizedSpellName(SpellInfo const* spellInfo)
{
    LocaleConstant defaultLoc = sWorld->GetDefaultDbcLocale();
    if (spellInfo->SpellName[defaultLoc] && *spellInfo->SpellName[defaultLoc])
        return spellInfo->SpellName[defaultLoc];

    if (spellInfo->SpellName[LOCALE_enUS] && *spellInfo->SpellName[LOCALE_enUS])
        return spellInfo->SpellName[LOCALE_enUS];

    return spellInfo->SpellName[0] ? spellInfo->SpellName[0] : "";
}
}  // namespace
SpellIdValue::SpellIdValue(PlayerbotAI* botAI) : CalculatedValue<uint32>(botAI, "spell id", 20 * 1000) {}
// By leewheel 2026-09-27 合并brighton the-lab：brighton侧构造函数用 20*IN_MILLISECONDS，
//   与本分支 20*1000 语义完全等价（IN_MILLISECONDS==1），保留本地性能优化实现。
// End By leewheel

VehicleSpellIdValue::VehicleSpellIdValue(PlayerbotAI* botAI) : CalculatedValue<uint32>(botAI, "vehicle spell id") {}

uint32 SpellIdValue::Calculate()
{
    std::string namepart = qualifier;
    ItemIds itemIds = ChatHelper::parseItems(namepart);

    PlayerbotChatHandler handler(bot);
    uint32 extractedSpellId = handler.extractSpellId(namepart);
    if (extractedSpellId)
        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(extractedSpellId))
            namepart = GetLocalizedSpellName(spellInfo);

    std::wstring wnamepart;
    if (!Utf8toWStr(namepart, wnamepart))
        return 0;

    wstrToLower(wnamepart);
    if (wnamepart.empty())
        return 0;

    wchar_t firstSymbol = wnamepart[0];

    std::set<uint32> spellIds;
    for (PlayerSpellMap::iterator itr = bot->GetSpellMap().begin(); itr != bot->GetSpellMap().end(); ++itr)
    {
        uint32 spellId = itr->first;

        if (itr->second->State == PLAYERSPELL_REMOVED || !itr->second->Active)
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo || spellInfo->IsPassive())
            continue;

        if (spellInfo->Effects[0].Effect == SPELL_EFFECT_LEARN_SPELL)
            continue;

        bool useByItem = false;
        for (uint8 i = 0; i < 3; ++i)
        {
            if (spellInfo->Effects[i].Effect == SPELL_EFFECT_CREATE_ITEM &&
                itemIds.find(spellInfo->Effects[i].ItemType) != itemIds.end())
            {
                useByItem = true;
                break;
            }
        }

        if (!useByItem && !SpellNameMatches(spellInfo, wnamepart, firstSymbol))
            continue;

        spellIds.insert(spellId);
    }

    Pet* pet = bot->GetPet();
    if (spellIds.empty() && pet)
    {
        for (PetSpellMap::const_iterator itr = pet->m_spells.begin(); itr != pet->m_spells.end(); ++itr)
        {
            if (itr->second.state == PETSPELL_REMOVED)
                continue;

            uint32 spellId = itr->first;
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
            if (!spellInfo)
                continue;

            if (spellInfo->Effects[0].Effect == SPELL_EFFECT_LEARN_SPELL)
                continue;

            if (!SpellNameMatches(spellInfo, wnamepart, firstSymbol))
                continue;

            spellIds.insert(spellId);
        }
    }

    // By leewheel 2026-09-10
    // 名称在 bot 已学技能里匹配不到时，改用 DBC 静态映射表（SpellNameIdFallbackData.h）兜底。
    // 触发场景：魔改版技能名与策略里写的名字对不上，或 bot 尚未学到该技能。
    // 原有匹配逻辑一行未动，仅在它失败时才介入。
    if (spellIds.empty())
    {
        std::vector<uint32_t> const fallbackIds =
            PlayerbotsSpellNameId::FindSpellIdsByName(PlayerbotsSpellNameId::NormalizeSpellName(namepart));

        // 兜底表按 rank 升序返回，倒序取第一个 bot 已学会的，即为可用的最高级。
        for (auto it = fallbackIds.rbegin(); it != fallbackIds.rend(); ++it)
        {
            if (bot->HasSpell(*it))
            {
                spellIds.insert(*it);
                break;
            }
        }

        // bot 一个等级都没学会：把结论写进日志，避免像原来那样静默返回 0 无从排查。
        if (spellIds.empty() && !fallbackIds.empty())
        {
            LOG_DEBUG("playerbots",
                      "[SpellIdFallback] bot '{}' has no rank of spell '{}' (DBC has {} ranks, top spellId {})",
                      bot->GetName(), namepart, fallbackIds.size(), fallbackIds.back());
        }
    }
    // End By leewheel

    if (spellIds.empty())
        return 0;

    // By leewheel 2026-11-18：上游本批把 rank 选择从「解析 Rank 字符串 atoi」改成
    //   SpellInfo::IsRanked()/GetRank() 比较（IsBetterRank），是有意改进，照上游采纳；
    //   本核的 DBC 兜底段仍在其之前，兜底成功时 best 从兜底结果里挑最高 rank，语义一致。
    SpellInfo const* best = nullptr;
    for (uint32 spellId : spellIds)
    {
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (spellInfo && (!best || IsBetterRank(spellInfo, best)))
            best = spellInfo;
    }
    // End By leewheel

    if (!best)
        return 0;

    int32 const saveMana = static_cast<int32>(std::round(AI_VALUE(double, "mana save level")));
    if (saveMana <= 1)
        return best->Id;

    // Mana save level N picks the Nth-highest rank of the chosen spell, else its lowest.
    std::vector<SpellInfo const*> ranks;
    for (uint32 spellId : spellIds)
    {
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (spellInfo && spellInfo->IsRankOf(best))
            ranks.push_back(spellInfo);
    }

    std::sort(
        ranks.begin(), ranks.end(),
        [](SpellInfo const* a, SpellInfo const* b) { return a->GetRank() > b->GetRank(); });

    size_t const index = std::min<size_t>(saveMana - 1, ranks.size() - 1);
    return ranks[index]->Id;
}

uint32 VehicleSpellIdValue::Calculate()
{
    Vehicle* vehicle = bot->GetVehicle();
    if (!vehicle)
        return 0;

    // do not allow if no spells
    VehicleSeatEntry const* seat = vehicle->GetSeatForPassenger(bot);
    if (!seat || !(seat->m_flags & VEHICLE_SEAT_FLAG_CAN_CAST))
        return 0;

    Unit* vehicleBase = vehicle->GetBase();
    if (!vehicleBase->IsAlive())
        return 0;

    std::string namepart = qualifier;

    PlayerbotChatHandler handler(bot);
    uint32 extractedSpellId = handler.extractSpellId(namepart);
    if (extractedSpellId)
        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(extractedSpellId))
            namepart = GetLocalizedSpellName(spellInfo);

    std::wstring wnamepart;
    if (!Utf8toWStr(namepart, wnamepart))
        return 0;

    wstrToLower(wnamepart);
    if (wnamepart.empty())
        return 0;

    wchar_t firstSymbol = wnamepart[0];

    Creature* creature = vehicleBase->ToCreature();
    for (uint32 x = 0; x < MAX_CREATURE_SPELLS; ++x)
    {
        uint32 spellId = creature->m_spells[x];
        if (spellId == 2)
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo || spellInfo->IsPassive())
            continue;

        if (!SpellNameMatches(spellInfo, wnamepart, firstSymbol))
            continue;

        return spellId;
    }

    return 0;
}
