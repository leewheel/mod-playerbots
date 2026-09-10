/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef _PLAYERBOTS_SPELL_NAME_ID_FALLBACK_H
#define _PLAYERBOTS_SPELL_NAME_ID_FALLBACK_H

#include <cstdint>
#include <string>
#include <vector>

// By leewheel 2026-09-10
// 背景：机器人技能系统（SpellIdValue）原本只遍历 bot 的 GetSpellMap()，用技能**全名等长匹配**
// 把策略里写的名字（如 "healing wave"）解析成 spellId。这条链有三个脆弱点：
//   1. bot 没学到该技能 -> 直接返回 0；
//   2. 魔改版 DBC 里技能名被改过 -> 名字对不上，返回 0；
//   3. Rank 文本若不是 "Rank N" 格式 -> atoi 失败，选错等级。
// 而返回 0 之后 CanCastSpell() 只是静默返回 false，日志里不留痕迹，表现就是
// “bot 站着不动、完全不放某个技能”，很难排查。
//
// 对策：以 DBC（db_spell_12340_eng，即服务端加载的 Spell.dbc）为准，生成一份
// 「规范化小写技能名 -> 该名字下全部 spellId（按 rank 升序）」的静态映射表，
// 作为名称解析失败时的第二道保障。以后修改技能 DBC，重新生成
// SpellNameIdFallbackData.h 即可。
// End By leewheel
namespace PlayerbotsSpellNameId
{
// 把任意技能名规范化成与映射表一致的键：去首尾空白、转小写、连续空白合并为单个空格。
std::string NormalizeSpellName(std::string const& name);

// 在静态映射表中查找规范化后的技能名。
// 命中时返回该名字下全部 spellId（已按 rank 升序，rank 相同的按 spellId 升序）；
// 未命中时返回空 vector。
std::vector<uint32_t> FindSpellIdsByName(std::string const& normalizedLowerName);
}  // namespace PlayerbotsSpellNameId

#endif
