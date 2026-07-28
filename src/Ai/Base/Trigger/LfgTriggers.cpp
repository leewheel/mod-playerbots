/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "LfgTriggers.h"

#include "AiFactory.h"
#include "Playerbots.h"

bool LfgProposalActiveTrigger::IsActive() { return AI_VALUE(uint32, "lfg proposal"); }

bool UnknownDungeonTrigger::IsActive()
{
    return botAI->HasActivePlayerMaster() && botAI->GetMaster() && botAI->GetMaster()->IsInWorld() &&
           botAI->GetMaster()->GetMap()->IsDungeon() && bot->GetMapId() == botAI->GetMaster()->GetMapId();
}

// By leewheel 2026-07-29
// 坦克/治疗 bot 优先触发 LFG 队列
// 1. 节流：与 RandomTrigger 一致，按 repeatDelay 节流避免每帧重算
// 2. 角色过滤：仅对坦克/治疗天赋 bot 激活，DPS bot 不会因此触发（避免抢队）
// 3. 概率更高：默认 probability=2，约为普通 "random" 触发器的 3.5 倍频率
// 4. 配置开关：通过 AiPlayerbot.RandomBotLfgRolePriority 控制（默认 1 启用）
bool LfgRolePriorityTrigger::IsActive()
{
    // By leewheel 2026-07-29: 允许通过配置关闭坦克优先
    if (!sPlayerbotAIConfig.randomBotJoinLfg || sPlayerbotAIConfig.randomBotLfgRolePriority == 0)
        return false;

    if (getMSTime() - lastCheck < sPlayerbotAIConfig.repeatDelay)
        return false;
    lastCheck = getMSTime();

    // 仅对坦克或治疗天赋 bot 激活。DPS bot 走 "random" 触发器即可。
    if (!botAI->IsTank(bot, true) && !botAI->IsHeal(bot, true))
        return false;

    int32 k = (int32)(probability / sPlayerbotAIConfig.randomChangeMultiplier);
    if (k < 1)
        k = 1;
    return (rand() % k) == 0;
}
