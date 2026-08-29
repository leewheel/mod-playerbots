/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PVPSURVIVALACTIONS_H
#define PLAYERBOTS_PVPSURVIVALACTIONS_H

#include "MovementActions.h"
#include "UseItemAction.h"

// By leewheel 2026-08-29
// PVP 自保动作组（参考 NPCBots 战场生存手法）：
//   1) 血量告急时对近身敌方玩家施放控制技能（按职业 entry 表硬控优先），争取恢复窗口；
//   2) 控制成功/敌人拉开后利用短暂时间打绷带恢复血量（背包按等级从高到低找绷带）；
//   3) 血量极低且被围攻时撤退逃跑（远离敌人方向强制移动）。
//   全部技能/物品一律使用 entry，运行时通过法术链自动匹配 bot 已学的最高等级。
// End By leewheel

// By leewheel 2026-08-29
// 查找 range 码内最近的存活敌方玩家（含敌方 bot）——自保动作组共用的辅助函数
// End By leewheel
Unit* FindNearestEnemyPlayerForSurvival(PlayerbotAI* botAI, float range);

// 低血量控制逃生：对近身敌方玩家施放本职业可用的控制/减速技能
class CastCcEscapeAction : public Action
{
public:
    CastCcEscapeAction(PlayerbotAI* botAI) : Action(botAI, "pvp cast cc escape") {}

    bool isUseful() override;
    bool Execute(Event event) override;

protected:
    // 返回 bot 在给定法术链（baseEntry 为 rank1 entry）上已学会的最高等级法术 entry；未学返回 0
    uint32 FindKnownTopRank(uint32 baseEntry);
};

// PVP 绷带自救：近战范围内无敌对玩家时打绷带恢复（复用 UseItemAction 的完整物品使用流程）
class UseBandageInPvpAction : public UseItemAction
{
public:
    UseBandageInPvpAction(PlayerbotAI* botAI) : UseItemAction(botAI, "pvp use bandage") {}

    bool isUseful() override;
    bool Execute(Event event) override;

protected:
    // 从背包中按等级从高到低查找第一个可用的绷带
    Item* FindBandage();
};

// PVP 濒死撤退：血量极低且被多名敌方玩家围攻时，向远离敌人方向强制移动
class PvpRetreatAction : public MovementAction
{
public:
    PvpRetreatAction(PlayerbotAI* botAI) : MovementAction(botAI, "pvp retreat") {}

    bool isUseful() override;
    bool Execute(Event event) override;
};

#endif
