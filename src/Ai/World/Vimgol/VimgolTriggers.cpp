/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option) any later version.
 */

#include "VimgolTriggers.h"
#include "Playerbots.h"
#include "PlayerbotMgr.h"

// Creature entries
static constexpr uint32 NPC_VIMGOL_CIRCLE_BUNNY = 23040;   // Vim'gol's Circle Bunny (fire ring)
static constexpr uint32 NPC_VIMGOL_SUMMON_BUNNY = 23081;   // Vim'gol's Circle Summon Visual Bunny
static constexpr uint32 NPC_VIMGOL_THE_VILE     = 22911;   // Vim'gol the Vile (boss)
static constexpr uint32 SPELL_UNHOLY_GROWTH     = 40545;   // Unholy Growth

// Quest
static constexpr uint32 QUEST_VIMGOLS_GRIMOIRE  = 10998;   // Vim'gol's Grimoire

// Distance thresholds
static constexpr float CIRCLE_DETECT_RADIUS = 80.0f;   // detect circle bunnies within this range
static constexpr float BOSS_DETECT_RADIUS   = 200.0f;  // detect boss within this range

//By leewheel 2026-07-09
// 统一前置条件宏：检查玩家(master)是否拥有任务10998且未完成，且玩家处于组队状态
// 与机器人自身是否拥有任务无关，唯一条件就是玩家有任务+组队了机器人
#define VIMGOL_CHECK_MASTER_QUEST()                                                 \
    Player* master = GetMaster();                                                   \
    if (!master)                                                                    \
        return false;                                                               \
    if (master->GetQuestStatus(QUEST_VIMGOLS_GRIMOIRE) != QUEST_STATUS_INCOMPLETE)  \
        return false;                                                               \
    if (!master->GetGroup())                                                        \
        return false;
//End By leewheel

bool VimgolNearCircleTrigger::IsActive()
{
    VIMGOL_CHECK_MASTER_QUEST();

    // 仅在外域(地图530)生效
    if (bot->GetMapId() != 530)
        return false;

    // 检查附近是否有火环兔子
    std::list<Creature*> bunnyList;
    bot->GetCreatureListWithEntryInGrid(bunnyList, NPC_VIMGOL_CIRCLE_BUNNY, CIRCLE_DETECT_RADIUS);
    return !bunnyList.empty();
}

bool VimgolSummoningPhaseTrigger::IsActive()
{
    VIMGOL_CHECK_MASTER_QUEST();

    // 仅在外域(地图530)生效
    if (bot->GetMapId() != 530)
        return false;

    // 必须靠近法阵
    std::list<Creature*> bunnyList;
    bot->GetCreatureListWithEntryInGrid(bunnyList, NPC_VIMGOL_CIRCLE_BUNNY, CIRCLE_DETECT_RADIUS);
    if (bunnyList.empty())
        return false;

    // 检查维姆高尔是否还未被召唤(200码内无存活的BOSS)
    std::list<Creature*> bossList;
    bot->GetCreatureListWithEntryInGrid(bossList, NPC_VIMGOL_THE_VILE, BOSS_DETECT_RADIUS);
    for (Creature* boss : bossList)
    {
        if (boss && boss->IsAlive())
            return false;  // BOSS已被召唤，非召唤阶段
    }

    return true;  // 附近无BOSS -> 需要召唤
}

bool VimgolUnholyGrowthTrigger::IsActive()
{
    VIMGOL_CHECK_MASTER_QUEST();

    // 仅在外域(地图530)生效
    if (bot->GetMapId() != 530)
        return false;

    // 查找附近的维姆高尔
    std::list<Creature*> bossList;
    bot->GetCreatureListWithEntryInGrid(bossList, NPC_VIMGOL_THE_VILE, BOSS_DETECT_RADIUS);
    for (Creature* boss : bossList)
    {
        if (boss && boss->IsAlive() && boss->HasAura(SPELL_UNHOLY_GROWTH))
            return true;
    }

    return false;
}
