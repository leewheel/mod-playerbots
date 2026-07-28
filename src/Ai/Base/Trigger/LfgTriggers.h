/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_LFGTRIGGERS_H
#define PLAYERBOTS_LFGTRIGGERS_H

#include "Trigger.h"

class PlayerbotAI;

class LfgProposalActiveTrigger : public Trigger
{
public:
    LfgProposalActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "lfg proposal active", 20 * 2000) {}

    bool IsActive() override;
};

class UnknownDungeonTrigger : public Trigger
{
public:
    UnknownDungeonTrigger(PlayerbotAI* botAI) : Trigger(botAI, "unknown dungeon", 20 * 2000) {}

    bool IsActive() override;
};

// By leewheel 2026-07-29
// 坦克/治疗机器人 LFG 优先触发器：
// 原因：原 LfgStrategy 仅依赖 "random" 触发器（默认 1/7 概率，约 14 秒/次），
//       坦克天赋 bot 仅占池子的 ~10%，导致真实玩家排队时几乎看不到坦克 bot。
// 修复：坦克/治疗 bot 走专用触发器，probability=2（约 4 秒/次，是默认的 3.5 倍），
//       非坦克/治疗 bot 仍走 "random" 触发器，避免无意义抢队。
//       触发器仅在 bot 满足空闲条件下激活（沿用 LfgJoinAction::isUseful 检查），
//       不会导致 bot 在战斗/死亡/副本内时错误排队。
class LfgRolePriorityTrigger : public Trigger
{
public:
    // probability 越小，触发越频繁。默认 2 表示每 2 个 repeatDelay 周期 1 次（约 4 秒）
    LfgRolePriorityTrigger(PlayerbotAI* botAI, std::string const name, int32 probability = 2)
        : Trigger(botAI, name), probability(probability), lastCheck(0) {}

    bool IsActive() override;

private:
    int32 probability;
    uint32 lastCheck;
};

#endif
