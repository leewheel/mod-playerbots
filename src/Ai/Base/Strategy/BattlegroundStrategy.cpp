/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BattlegroundStrategy.h"
#include "Playerbots.h"

void BGStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("often", { NextAction("bg join", relevance)}));
    triggers.push_back(new TriggerNode("bg invite active", { NextAction("bg status check", relevance)}));
    triggers.push_back(new TriggerNode("timer", { NextAction("bg strategy check", relevance)}));
}

BGStrategy::BGStrategy(PlayerbotAI* botAI) : PassThroughStrategy(botAI) {}

void BattlegroundStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("bg waiting", { NextAction("bg move to start", ACTION_BG)}));
    triggers.push_back(new TriggerNode("bg active", { NextAction("bg move to objective", ACTION_BG)}));
    triggers.push_back(new TriggerNode("often", { NextAction("bg check objective", ACTION_BG + 1)}));
    triggers.push_back(new TriggerNode("dead", { NextAction("bg reset objective force", ACTION_EMERGENCY)}));
}

// By leewheel 2026-09-01
// PVP 交战循环策略（修复挂载点缺陷，详见头文件注释）：
//   原六条链挂在 BattlegroundStrategy（NONCOMBAT 类型，只在脱战引擎生效），
//   战斗状态（被控/被打）全部不生效——本策略 GENERIC 类型，战斗/非战斗双引擎挂载。
//   优先级从高到低：
//   1) 濒死被围攻（血<25% 且敌>=2）→ 强制反方向撤退（保命第一）；
//   2) 低血量被近身（血<40% 且敌近身）→ 施放本职业控制/逃生技能争取恢复窗口；
//   3) 打不死（目标玩家血>20%）且自身血<70% → 硬控远遁进入循环；
//   4) 恢复窗口（血<80% 且近身敌人全被控/无敌）→ 瞬发治疗石先手 + 绷带续接；
//   5) 血量偏低且无敌近身（血<60%）→ 安全窗口打绷带。
//   触发器自带战场/竞技场守卫，野外副本挂载无副作用。
// End By leewheel
void PvpCycleStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    //By leewheel 2026-09-01 复核修正: 徽章链原只在 RacialsStrategy（仅战斗引擎挂载），
    //  BG 脱战引擎（被远程风筝/定身且身边无攻击者时）永不解控，违背"解控饰品是必需品"
    //  的核心需求。本策略双引擎挂载，把徽章链补到脱战状态；战斗状态与 racials 链重复
    //  注册同一"触发器→动作"对，第二次评估时 CC 已解、isUseful 自动为 false，无副作用。
    triggers.push_back(new TriggerNode("cc victim", { NextAction("use ccbreak trinket", ACTION_EMERGENCY)}));
    //End By leewheel
    triggers.push_back(new TriggerNode("pvp critical", { NextAction("pvp retreat", ACTION_EMERGENCY)}));
    triggers.push_back(new TriggerNode("low hp pvp", { NextAction("pvp cast cc escape", ACTION_EMERGENCY - 1)}));
    triggers.push_back(new TriggerNode("pvp cycle disengage", { NextAction("pvp cc disengage", ACTION_EMERGENCY - 2)}));
    triggers.push_back(new TriggerNode("pvp cycle recover", {
        NextAction("healthstone", ACTION_MOVE + 3),
        NextAction("pvp use bandage", ACTION_MOVE + 2)}));
    triggers.push_back(new TriggerNode("safe to bandage", { NextAction("pvp use bandage", ACTION_MOVE + 1)}));
}

void WarsongStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("bg active", { NextAction("bg check flag", ACTION_EMERGENCY )}));
    triggers.push_back(new TriggerNode("enemy flagcarrier near", { NextAction("attack enemy flag carrier", ACTION_RAID + 1.0f)}));
    triggers.push_back(new TriggerNode("team flagcarrier near", { NextAction("bg protect fc", ACTION_RAID)}));
    triggers.push_back(new TriggerNode("often", { NextAction("bg use buff", ACTION_BG)}));
    triggers.push_back(new TriggerNode("low health", { NextAction("bg use buff", ACTION_MOVE)}));
    triggers.push_back(new TriggerNode("low mana", { NextAction("bg use buff", ACTION_MOVE)}));
    triggers.push_back(new TriggerNode("player has flag", { NextAction("bg move to objective", ACTION_EMERGENCY)}));
    triggers.push_back(new TriggerNode("timer bg", { NextAction("bg reset objective force", ACTION_EMERGENCY)}));
}

void AlteracStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("alliance no snowfall gy", { NextAction("bg move to objective", ACTION_EMERGENCY)}));
    triggers.push_back(new TriggerNode("timer bg", { NextAction("bg reset objective force", ACTION_EMERGENCY)}));
}

void ArathiStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("bg active", { NextAction("bg check flag", ACTION_EMERGENCY)}));
    triggers.push_back(new TriggerNode("often", { NextAction("bg use buff", ACTION_BG)}));
    triggers.push_back(new TriggerNode("low health", { NextAction("bg use buff", ACTION_MOVE)}));
    triggers.push_back(new TriggerNode("low mana", { NextAction("bg use buff", ACTION_MOVE)}));
}

void EyeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("bg active", { NextAction("bg check flag", ACTION_EMERGENCY)}));
    triggers.push_back(new TriggerNode("often", { NextAction("bg use buff", ACTION_BG)}));
    triggers.push_back(new TriggerNode("low health", { NextAction("bg use buff", ACTION_MOVE)}));
    triggers.push_back(new TriggerNode("low mana", { NextAction("bg use buff", ACTION_MOVE)}));
    triggers.push_back(new TriggerNode("enemy flagcarrier near", { NextAction("attack enemy flag carrier", ACTION_RAID)}));
    triggers.push_back(new TriggerNode("player has flag",{ NextAction("bg move to objective", ACTION_EMERGENCY)}));
}

//TODO: Do Priorities
void IsleStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("bg active", { NextAction("bg check flag", ACTION_MOVE)}));
    triggers.push_back(new TriggerNode("timer", { NextAction("enter vehicle", ACTION_MOVE + 8.0f)}));
    triggers.push_back(new TriggerNode("random", { NextAction("leave vehicle", ACTION_MOVE + 7.0f)}));
    triggers.push_back(new TriggerNode("in vehicle", { NextAction("hurl boulder", ACTION_MOVE + 9.0f)}));
    triggers.push_back(new TriggerNode("in vehicle", { NextAction("fire cannon", ACTION_MOVE + 9.0f)}));
    triggers.push_back(new TriggerNode("in vehicle", { NextAction("napalm", ACTION_MOVE + 9.0f)}));
    triggers.push_back(new TriggerNode("enemy is close", { NextAction("steam blast", ACTION_MOVE + 9.0f)}));
    triggers.push_back(new TriggerNode("in vehicle", { NextAction("ram", ACTION_MOVE + 9.0f)}));
    triggers.push_back(new TriggerNode("enemy is close", { NextAction("ram", ACTION_MOVE + 9.1f)}));
    triggers.push_back(new TriggerNode("enemy out of melee", { NextAction("steam rush", ACTION_MOVE + 9.2f)}));
    triggers.push_back(new TriggerNode("in vehicle", { NextAction("incendiary rocket", ACTION_MOVE + 9.0f)}));
    triggers.push_back(new TriggerNode("in vehicle", { NextAction("rocket blast", ACTION_MOVE + 9.0f)}));
    // this is bugged: it doesn't work, and stops glaive throw working (which is needed to take down gate)
    // triggers.push_back(new TriggerNode("in vehicle", { NextAction("blade salvo", ACTION_MOVE + 9.0f)}));
    triggers.push_back(new TriggerNode("in vehicle", { NextAction("glaive throw", ACTION_MOVE + 9.0f)}));
}

void ArenaStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("no possible targets", { NextAction("arena tactics", ACTION_BG)}));
}
