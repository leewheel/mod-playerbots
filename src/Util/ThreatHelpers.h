/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_THREATHELPERS_H
#define PLAYERBOTS_THREATHELPERS_H

#include <string_view>

class Player;
class Unit;

// The "find target" value without the value machinery. Like it, this sees only units that have the
// bot on their threat list, ignores case, and returns null if the bot has no group.
//
// By leewheel 2026-11-04 entry 铁律：qualifier 为纯数字时按 unit->GetEntry() 精确匹配。
// 上游原实现只做 GetName() 英文名比对（EqualsIgnoringAsciiCase），而实库生物名是中文
// （如"卡雷苟斯"），英文名 qualifier 恒不相等 ⇒ 全部 raid 机制静默失效。判据与本核
// 2026-08-21 对 FindTargetValue 的补丁一致：纯数字走 entry，否则保留原名字匹配。
Unit* FindThreatTarget(Player* bot, std::string_view name);

#endif
