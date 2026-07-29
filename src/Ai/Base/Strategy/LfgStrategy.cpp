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
    // By leewheel 2026-07-29
    // 恢复 "seldom" 离队触发器（参考 LiyunfanPlayerbotsBranch 的稳定实现）。
    // 原因：之前移除它是为了防止坦克/治疗频繁掉出队列，但实际把整段 LfgLeaveAction
    //       干掉了，导致 bot 永远不退队列。QueuePacket 模式下的 LfgLeaveAction
    //       配合 LFG_STATE_QUEUED 检查会在成功匹配后自然留下，只有 QUEUED 状态才离队，
    //       因此这里恢复 seldom 离队是安全的。
    triggers.push_back(
        new TriggerNode("seldom", { NextAction("lfg leave", relevance) }));
    // End By leewheel
    triggers.push_back(new TriggerNode(
        "unknown dungeon", { NextAction("give leader in dungeon", relevance) }));

    // By leewheel 2026-07-29
    // 坦克/治疗 bot 优先 LFG 触发器
    // 原因：原 "random" 触发器对所有 bot 是 1/7 概率（约 14 秒/次），
    //       坦克天赋 bot 仅占池子的 ~10%，导致真实玩家排队时几乎看不到坦克 bot。
    // 修复：新增 "lfg role priority" 触发器，仅对坦克/治疗 bot 激活，
    //       概率 1/2（约 4 秒/次，约为默认的 3.5 倍频率）。
    // 效果：坦克/治疗 bot 入队速度提升约 3 倍，DPS bot 频率不变。
    triggers.push_back(new TriggerNode("lfg role priority", { NextAction("lfg join", relevance) }));
    // End By leewheel
}

LfgStrategy::LfgStrategy(PlayerbotAI* botAI) : PassThroughStrategy(botAI) {}
