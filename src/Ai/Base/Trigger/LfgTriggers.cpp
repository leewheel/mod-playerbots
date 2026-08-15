/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "LfgTriggers.h"

#include "AiFactory.h"
// By leewheel 2026-08-02
// 修复编译错误 C2065/C2039：LfgRolePriorityTrigger::IsActive 使用了 lfg::LfgState / sLFGMgr /
// lfg::LFG_STATE_NONE / lfg::LFG_STATE_DUNGEON，但本文件缺少核心 LFG 头文件，
// 编译器无法识别这些标识符。补充 include（LFGMgr.h 内部已包含 LFG.h 提供 LfgState 枚举）。
#include "LFGMgr.h"
// End By leewheel
#include "Playerbots.h"

bool LfgProposalActiveTrigger::IsActive() { return AI_VALUE(uint32, "lfg proposal"); }

bool UnknownDungeonTrigger::IsActive()
{
    return IsRealPlayer(botAI->GetMaster()) && botAI->GetMaster() && botAI->GetMaster()->IsInWorld() &&
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

    // By leewheel 2026-08-01
    // 周期性卡顿修复：bot 已在 LFG 队列中（QUEUED/PROPOSAL 等非 NONE、非 DUNGEON 状态）则不再触发。
    // 7月30日 版本中缺少该检查：已排队 bot 仍每约4秒触发一次 lfg join，每次 join 都会走
    // OnPlayerCanJoinLfg 钩子执行全地图遍历修复（FixAllDungeonRequirements + InitializeLockedDungeons），
    // 与 LFG 每8秒撮合周期(UpdateQueueTimers)叠加，是玩家端每 7~8 秒规律性卡顿的直接来源。
    lfg::LfgState const state = sLFGMgr->GetState(bot->GetGUID());
    if (state != lfg::LFG_STATE_NONE && state < lfg::LFG_STATE_DUNGEON)
        return false;
    // End By leewheel

    // By leewheel 2026-08-01
    // 周期性卡顿修复：队列饱和度检查——
    // 该阵营对应角色已有足够 bot 在 LFG 队列中时不再触发，防止坦克/治疗机器人无限自主排队，
    // 持续膨胀 LFG 队列、加重 8 秒撮合周期(UpdateQueueTimers)的周期性尖峰。
    // 计数由 CheckLfgQueue 每 30 秒刷新的 lfgQueueRoleCount 提供（索引 0=坦克 1=治疗 2=DPS），
    // 与 ForceBotsJoinLfg 的目标配额（2坦+2奶）保持一致。
    std::array<uint32, 3> const queued = sRandomPlayerbotMgr.GetLfgQueueRoleCount(bot->GetTeamId());
    if (botAI->IsTank(bot, true) && queued[0] >= 2)
        return false;
    if (botAI->IsHeal(bot, true) && queued[1] >= 2)
        return false;
    // End By leewheel

    int32 k = (int32)(probability / sPlayerbotAIConfig.randomChangeMultiplier);
    if (k < 1)
        k = 1;
    return (rand() % k) == 0;
}
