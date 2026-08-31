/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GenericDKStrategy.h"
#include "DKAiObjectContext.h"
#include "Playerbots.h"

class GenericDKStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericDKStrategyActionNodeFactory()
    {
        creators["killing machine"] = &killing_machine;
        creators["anti magic zone"] = &anti_magic_zone;
        creators["death grip"] = &death_grip;
    }

private:
    static ActionNode* death_grip([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("death grip",
                              /*P*/ {},
                              /*A*/ { NextAction("icy touch") },
                              /*C*/ {});
    }

    static ActionNode* killing_machine([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("killing machine",
                              /*P*/ {},
                              /*A*/ { NextAction("improved icy talons") },
                              /*C*/ {});
    }

    static ActionNode* anti_magic_zone([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("anti magic zone",
                              /*P*/ {},
                              /*A*/ { NextAction("anti magic shell") },
                              /*C*/ {});
    }
};

GenericDKStrategy::GenericDKStrategy(PlayerbotAI* botAI) : MeleeCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericDKStrategyActionNodeFactory());
}

void GenericDKStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    MeleeCombatStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode("mind freeze", { NextAction("mind freeze", ACTION_HIGH + 1) }));
    triggers.push_back(
        new TriggerNode("mind freeze on enemy healer",
                        { NextAction("mind freeze on enemy healer", ACTION_HIGH + 1) }));
    triggers.push_back(new TriggerNode(
        "horn of winter", { NextAction("horn of winter", ACTION_NORMAL + 1) }));
    triggers.push_back(new TriggerNode("critical health",
                                       { NextAction("raise dead", ACTION_HIGH + 6),
                                         NextAction("death pact", ACTION_HIGH + 5) }));

    triggers.push_back(
        new TriggerNode("low health", { NextAction("icebound fortitude", ACTION_HIGH + 5),
                                                        NextAction("rune tap", ACTION_HIGH + 4) }));
    triggers.push_back(
        new TriggerNode("medium aoe", { NextAction("death and decay", ACTION_HIGH + 9),
                                                        NextAction("pestilence", ACTION_NORMAL + 4),
                                                        NextAction("blood boil", ACTION_NORMAL + 3) }));
    triggers.push_back(
        new TriggerNode("pestilence glyph", { NextAction("pestilence", ACTION_HIGH + 9) }));
    triggers.push_back(
        new TriggerNode("no rune",
            {
                NextAction("empower rune weapon", ACTION_HIGH + 1)
            }
        )
    );

    // By leewheel 2026-09-01
    // DK 巫妖之躯解控（移植 NPCBots bot_death_knight_ai.cpp:499-509 BreakCC 重写）：
    //   被恐惧/魅惑/沉睡时主动解（复用现成 fear charm sleep 触发器）。
    //   优先级 EMERGENCY+2 压过亡灵意志(EMERGENCY+1)与解控饰品(EMERGENCY)——
    //   职业解控免费，把徽章留给下一轮控制（超越 NPCBots 的省饰品优化）。
    //   未点邪恶系巫妖之躯天赋时 spell id 解析为 0，动作自然不可用，无副作用。
    // End By leewheel
    triggers.push_back(
        new TriggerNode("fear charm sleep", { NextAction("lichborne", ACTION_EMERGENCY + 2) }));
}
