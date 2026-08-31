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

// By leewheel 2026-09-01
// PVP 交战循环·控制远遁（老大核心需求："杀不死对方使用控制类技能让对方进入昏迷、恐惧之类的
//   状态，自己迅速远遁、绷带之类的给自己快速回血，然后等待循环CD到了之后再来一轮"）：
//   1) 当前目标是玩家且打不死（目标血>20%）而自身状态不佳（血<70%）时，
//      对目标施放本职业硬控（CC_TABLE 硬控优先）；
//   2) 控制命中后用职业位移技能远遁（闪现/消失/逃脱/疾跑，ESCAPE_TABLE），
//      无位移技能则向远离目标方向后撤 20 码；
//   3) 远遁后进入绷带/药水恢复窗口（配合增强版 SafeToBandage：被控敌人不算威胁），
//      CD 转好后由常规输出策略自然再接敌（隐式循环，无状态机死锁风险）。
// End By leewheel
class CastCcDisengageAction : public MovementAction
{
public:
    CastCcDisengageAction(PlayerbotAI* botAI) : MovementAction(botAI, "pvp cc disengage") {}

    bool isUseful() override;
    bool Execute(Event event) override;

protected:
    // 对目标施放本职业可用的最强硬控（只放眩晕/恐惧/定身/变形/沉睡/沉默类，不放纯减速）
    bool CastHardCc(Unit* target);
    // 职业位移远遁（成功返回 true）；无位移技能或不可用返回 false
    bool EscapeFrom(Unit* threat);
};

#endif
