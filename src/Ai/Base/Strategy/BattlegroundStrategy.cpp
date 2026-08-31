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

    // By leewheel 2026-08-29
    // PVP 自保触发链（参考 NPCBots 战场生存手法），优先级从高到低：
    //   1) 濒死被围攻（血<25% 且敌>=2）→ 强制反方向撤退（最高优先，保命第一）；
    //   2) 低血量被近身（血<40% 且敌近身）→ 施放本职业控制/逃生技能争取恢复窗口；
    //   3) 血量偏低且无敌近身（血<60%）→ 安全窗口打绷带恢复。
    //   三者在 ACTION_EMERGENCY/ACTION_MOVE 量级，保证在普通输出动作之前被调度。
    // End By leewheel
    triggers.push_back(new TriggerNode("pvp critical", { NextAction("pvp retreat", ACTION_EMERGENCY)}));
    triggers.push_back(new TriggerNode("low hp pvp", { NextAction("pvp cast cc escape", ACTION_EMERGENCY - 1)}));
    triggers.push_back(new TriggerNode("safe to bandage", { NextAction("pvp use bandage", ACTION_MOVE + 1)}));

    // By leewheel 2026-09-01
    // PVP 交战循环触发链（移植 NPCBots 法师/盗贼"控制→远遁→等CD→再来一轮"要素并推广到全职业）：
    //   1) 打不死（目标玩家血>20%）且自身血<70% → 对目标施放硬控并位移远遁；
    //   2) 恢复窗口（血<80% 且近身敌人全被控/无敌）→ 瞬发治疗石先手 + 打绷带续接快速回血；
    //   3) CD 转好后常规输出策略自然再接敌（隐式循环）。
    //   优先级低于 8/29 保命链（保命 > 战术循环 > 常规输出）。
    // End By leewheel
    triggers.push_back(new TriggerNode("pvp cycle disengage", { NextAction("pvp cc disengage", ACTION_EMERGENCY - 2)}));
    triggers.push_back(new TriggerNode("pvp cycle recover", {
        NextAction("healthstone", ACTION_MOVE + 3),
        NextAction("pvp use bandage", ACTION_MOVE + 2)}));
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
