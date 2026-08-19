/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_POSSIBLETARGETSVALUE_H
#define PLAYERBOTS_POSSIBLETARGETSVALUE_H

#include "NearestUnitsValue.h"
#include "PlayerbotAIConfig.h"

class PlayerbotAI;

class PossibleTargetsValue : public NearestUnitsValue
{
public:
    // By leewheel 2026-08-19 - CPU 优化：目标扫描加 1000ms 缓存。
    // 原实现 checkInterval 默认 1（每 tick 全图 Cell::VisitObjects 扫描 + FNV/LOS）。
    // 500 个随机 bot 自动登录后，每个活跃 bot 每 tick 多次触发全图目标扫描 → 单核100%~200%卡死。
    // 缓存到 1000ms：目标感知延迟 1 秒完全无感（bot 无需 <1s 刷新目标），
    // 但把 CPU 从爆炸降到可接受，且不牺牲任何 bot 行为/游戏性能。
    // End By leewheel
    PossibleTargetsValue(PlayerbotAI* botAI, std::string const name = "possible targets",
                         float range = sPlayerbotAIConfig.sightDistance, bool ignoreLos = false)
        : NearestUnitsValue(botAI, name, range, ignoreLos, 1000)
    {
    }

protected:
    void FindUnits(std::list<Unit*>& targets) override;
    bool AcceptUnit(Unit* unit) override;
};

class AllTargetsValue : public PossibleTargetsValue
{
public:
    AllTargetsValue(PlayerbotAI* botAI, float range = sPlayerbotAIConfig.sightDistance)
        : PossibleTargetsValue(botAI, "all targets", range, true)
    {
    }
};

class PossibleTriggersValue : public NearestUnitsValue
{
public:
    PossibleTriggersValue(PlayerbotAI* botAI, std::string const name = "possible triggers", float range = 15.0f,
                          bool ignoreLos = true)
        : NearestUnitsValue(botAI, name, range, ignoreLos, 1 * 1000)
    {
    }

protected:
    void FindUnits(std::list<Unit*>& targets) override;
    bool AcceptUnit(Unit* unit) override;
};
#endif
