/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SpellNameIdFallback.h"

// 数据量较大（一万余行），只在本文件包含一次。
#include "SpellNameIdFallbackData.h"

#include <algorithm>
#include <cctype>

// By leewheel 2026-09-10: 技能名 -> 技能ID 兜底映射的实现（详见 SpellNameIdFallback.h 说明）
namespace PlayerbotsSpellNameId
{
std::string NormalizeSpellName(std::string const& name)
{
    std::string out;
    out.reserve(name.size());

    bool pendingSpace = false;
    for (char ch : name)
    {
        unsigned char uch = static_cast<unsigned char>(ch);

        // 只折叠 ASCII 空白；非 ASCII（UTF-8 多字节）原样保留，避免破坏字节序列。
        if (uch <= 0x20 || uch == 0x7F)
        {
            if (!out.empty())
                pendingSpace = true;
            continue;
        }

        if (pendingSpace)
        {
            out.push_back(' ');
            pendingSpace = false;
        }

        // ASCII 大写转小写；多字节字符保持原样（技能名的键以英文为准）。
        out.push_back((uch < 0x80 && std::isupper(uch)) ? static_cast<char>(std::tolower(uch)) : ch);
    }

    return out;
}

std::vector<uint32_t> FindSpellIdsByName(std::string const& normalizedLowerName)
{
    std::vector<uint32_t> result;

    if (normalizedLowerName.empty() || SpellNameIdRowCount == 0)
        return result;

    // 数据按 (name, rank, spellId) 升序排列，先二分定位到第一段 name >= 目标 的位置。
    SpellNameIdRow const* begin = SpellNameIdRows;
    SpellNameIdRow const* end = SpellNameIdRows + SpellNameIdRowCount;

    SpellNameIdRow const* lower = std::lower_bound(
        begin, end, normalizedLowerName,
        [](SpellNameIdRow const& row, std::string const& key) { return row.name < key; });

    // 向后收集所有同名行；数据已按 rank 升序，故结果天然按 rank 升序。
    for (SpellNameIdRow const* it = lower; it != end && it->name == normalizedLowerName; ++it)
        result.push_back(it->spellId);

    return result;
}
}  // namespace PlayerbotsSpellNameId
// End By leewheel
