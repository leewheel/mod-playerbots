/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PVPSURVIVALACTIONS_H
#define PLAYERBOTS_PVPSURVIVALACTIONS_H

#include "MovementActions.h"
#include "UseItemAction.h"
// By leewheel 2026-09-01: 徽章解控复用 UseTrinketAction（物品使用协议流程）
#include "GenericSpellActions.h"

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

// By leewheel 2026-09-01
// PVP 徽章解控（移植 NPCBots 被控解徽章 + 留牌反打优化）：
//   1) 被硬控（眩晕/恐惧/定身/迷惑/魅惑/沉睡/变形/放逐）时主动使用身上的解控饰品（触发法术 42292）；
//   2) 留牌逻辑：长 CC（剩余>15 秒，如变形术/放逐/沉睡）且自身状态良好（血>70%、围攻玩家<2）时
//      不急着解，等第二段连控或爆发窗口再用（NPCBots 是即中即交，我们更聪明）；
//   3) 复用 UseTrinketAction 的物品使用协议流程（含饰品 CD/类别 CD 跟踪）。
// End By leewheel
class UseCcbreakTrinketAction : public UseTrinketAction
{
public:
    UseCcbreakTrinketAction(PlayerbotAI* botAI) : UseTrinketAction(botAI, "use ccbreak trinket") {}

    bool isUseful() override;
    bool Execute(Event event) override;

protected:
    // 查找身上失去控制类光环的最长剩余时间（毫秒），无硬控返回 0
    uint32 GetLossOfControlRemainingMs() const;
    // 从已装备饰品里找带解控法术（42292）的徽章饰品
    Item* FindCcbreakTrinket() const;
};

#endif
