/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ThreatHelpers.h"
#include "Creature.h"
#include "Player.h"
#include "ThreatManager.h"
// By leewheel 2026-11-04 entry 匹配需要 std::all_of / std::string / strtoul
#include <algorithm>
#include <cstdlib>
#include <string>
// End By leewheel

namespace
{

char LowerAscii(char c) { return c >= 'A' && c <= 'Z' ? static_cast<char>(c - 'A' + 'a') : c; }

bool EqualsIgnoringAsciiCase(std::string_view a, std::string_view b)
{
    if (a.size() != b.size())
        return false;

    for (size_t i = 0; i < a.size(); ++i)
    {
        if (LowerAscii(a[i]) != LowerAscii(b[i]))
            return false;
    }

    return true;
}

// By leewheel 2026-11-04 entry 铁律兜底：上游 the-lab 改用英文名 qualifier 调 FindThreatTarget
// （BT/SSC/Kara 共 165 个调用点），而本服实库生物名是中文 ⇒ unit->GetName() 与英文名恒不相等，
// 整套 raid 机制会静默失效。处置：不逐个改调用点（改了每轮合并都要重新解 190+ 冲突），
// 改在这里做一次英文名 → NPC entry 的登记表，命中即按 entry 精确匹配。
// 证据（仓内既定）：SSC 见核心 serpent_shrine.h；Kara 见核心 karazhan.h 与模块既有核定注释；
// BT 见核心 black_temple.h。上游将来新增英文名会打 WARN（见下方），按此表补一行即可。
struct ThreatNameEntry
{
    std::string_view name;
    uint32 entry;
};

constexpr ThreatNameEntry kThreatNameEntries[] = {
    // --- Serpent Shrine Cavern（核心 serpent_shrine.h） ---
    {"hydross the unstable", 21216},      // NPC_HYDROSS_THE_UNSTABLE
    {"the lurker below", 21217},          // NPC_THE_LURKER_BELOW
    {"leotheras the blind", 21215},       // NPC_LEOTHERAS_THE_BLIND
    {"fathom-lord karathress", 21214},    // NPC_FATHOM_LORD_KARATHRESS
    {"lady vashj", 21212},                // NPC_LADY_VASHJ
    {"fathom-guard sharkkis", 21966},     // NPC_FATHOM_GUARD_SHARKKIS
    {"fathom-guard tidalvess", 21965},    // NPC_FATHOM_GUARD_TIDALVESS
    {"fathom-guard caribdis", 21964},     // NPC_FATHOM_GUARD_CARIBDIS
    {"coilfang strider", 22056},          // NPC_COILFANG_STRIDER
    {"tainted elemental", 22009},         // NPC_TAINTED_ELEMENTAL
    {"greyheart tidecaller", 21229},      // 本核 2026-10-24 合并时既定（SSC 水元素图腾）
    {"morogrim tidewalker", 21213},       // 本核既有核定（SSC 莫洛格里·踏潮者）
    // --- Karazhan（核心 karazhan.h / 本核既有核定） ---
    {"midnight", 16151},                  // NPC_MIDNIGHT（阿图门坐骑）
    {"spectral charger", 15547},          // 本核既有核定（Kara 幽灵战马）
    {"astral flare", 17283},              // 本核既有核定（虚空幽龙）
    {"conjured elemental", 17167},        // NPC_CONJURED_ELEMENTAL（馆长）
    // 绿野仙踪（OZ_TARGETS 经 GetOperaActor 间接传入，核心 karazhan.h）
    {"dorothee", 17535},                  // NPC_DOROTHEE
    {"tito", 17548},                      // NPC_TITO
    {"roar", 17546},                      // NPC_ROAR
    {"strawman", 17543},                  // NPC_STRAWMAN
    {"tinhead", 17547},                   // NPC_TINHEAD
    {"the big bad wolf", 17521},          // 本核既有核定（大灰狼）
    // --- Black Temple（核心 black_temple.h） ---
    {"gathios the shatterer", 22949},     // NPC_GATHIOS_THE_SHATTERER
};

uint32 EntryForThreatName(std::string_view name)
{
    for (auto const& item : kThreatNameEntries)
    {
        if (EqualsIgnoringAsciiCase(item.name, name))
            return item.entry;
    }

    return 0;
}

} // end anonymous namespace

Unit* FindThreatTarget(Player* bot, std::string_view name)
{
    if (name.empty() || !bot->GetGroup())
        return nullptr;

    // By leewheel 2026-11-04 entry 铁律：纯数字 qualifier 直接按 Entry 精确匹配；
    // 英文名 qualifier 先查登记表折算成 entry。两者都不命中时才退回上游的名字比对。
    uint32 entry = 0;
    if (std::all_of(name.begin(), name.end(), [](char c) { return c >= '0' && c <= '9'; }))
        entry = static_cast<uint32>(strtoul(std::string(name).c_str(), nullptr, 10));
    else
        entry = EntryForThreatName(name);
    // End By leewheel

    for (auto const& [guid, ref] : bot->GetThreatMgr().GetThreatenedByMeList())
    {
        Unit* unit = ref->GetOwner();
        if (!unit)
            continue;

        // By leewheel 2026-11-04 优先按 Entry 匹配
        if (entry != 0)
        {
            if (unit->GetEntry() == entry)
                return unit->IsInWorld() ? unit : nullptr;
            continue;
        }
        // End By leewheel

        if (EqualsIgnoringAsciiCase(unit->GetName(), name))
            return unit->IsInWorld() ? unit : nullptr;
    }

    return nullptr;
}
