/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "LfgStrategy.h"

#include "Playerbots.h"

void LfgStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("random", { NextAction("lfg join", relevance) }));
    // By leewheel 2026-07-21
    // 移除"seldom"离队触发器：原设计每~10分钟随机离队，导致坦克/治疗频繁掉出队列，
    // 真实玩家排随机本时长时间组不到人。bot不再主动离队，仅在变为不可用状态
    // (战斗/副本/死亡/被传送等)时由LfgLeaveAction的isUseful或状态检查自然退出。
    // End By leewheel
    triggers.push_back(new TriggerNode(
        "unknown dungeon", { NextAction("give leader in dungeon", relevance) }));
}

LfgStrategy::LfgStrategy(PlayerbotAI* botAI) : PassThroughStrategy(botAI) {}
