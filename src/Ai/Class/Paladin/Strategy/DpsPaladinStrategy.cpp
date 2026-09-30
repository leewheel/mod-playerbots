/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "DpsPaladinStrategy.h"
#include "Playerbots.h"
#include "Strategy.h"

class DpsPaladinStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    DpsPaladinStrategyActionNodeFactory()
    {
        creators["sanctity aura"] = &sanctity_aura;
        creators["retribution aura"] = &retribution_aura;
        creators["blessing of might"] = &blessing_of_might;
        creators["repentance"] = &repentance;
        creators["repentance on enemy healer"] = &repentance_on_enemy_healer;
        creators["repentance on snare target"] = &repentance_on_snare_target;
        creators["repentance of shield"] = &repentance_or_shield;
    }

private:
    static ActionNode* blessing_of_might([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "blessing of might",
            /*P*/ {},
            /*A*/ { NextAction("blessing of kings") },
            /*C*/ {}
        );
    }

    static ActionNode* repentance([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "repentance",
            /*P*/ {},
            /*A*/ { NextAction("hammer of justice") },
            /*C*/ {}
        );
    }

    static ActionNode* repentance_on_enemy_healer([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "repentance on enemy healer",
            /*P*/ {},
            /*A*/ { NextAction("hammer of justice on enemy healer") },
            /*C*/ {}
        );
    }

    static ActionNode* repentance_on_snare_target([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "repentance on snare target",
            /*P*/ {},
            /*A*/ { NextAction("hammer of justice on snare target") },
            /*C*/ {}
        );
    }

    static ActionNode* sanctity_aura([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "sanctity aura",
            /*P*/ {},
            /*A*/ { NextAction("retribution aura") },
            /*C*/ {}
        );
    }

    static ActionNode* retribution_aura([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "retribution aura",
            /*P*/ {},
            /*A*/ { NextAction("devotion aura") },
            /*C*/ {}
        );
    }

    static ActionNode* repentance_or_shield([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "repentance",
            /*P*/ {},
            /*A*/ { NextAction("divine shield") },
            /*C*/ {}
        );
    }
};

DpsPaladinStrategy::DpsPaladinStrategy(PlayerbotAI* botAI) : GenericPaladinStrategy(botAI)
{
    actionNodeFactories.Add(new DpsPaladinStrategyActionNodeFactory());
}

std::vector<NextAction> DpsPaladinStrategy::getDefaultActions()
{
    return {
        NextAction("hammer of wrath", ACTION_DEFAULT + 0.6f),
        NextAction("judgement of wisdom", ACTION_DEFAULT + 0.5f),
        NextAction("crusader strike", ACTION_DEFAULT + 0.4f),
        NextAction("divine storm", ACTION_DEFAULT + 0.3f),
        NextAction("consecration", ACTION_DEFAULT + 0.1f),
        NextAction("melee", ACTION_DEFAULT)
    };
}

void DpsPaladinStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericPaladinStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "art of war",
            {
                NextAction("exorcism", ACTION_DEFAULT + 0.2f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "seal",
            {
                NextAction("seal of corruption", ACTION_HIGH)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low mana",
            {
                NextAction("seal of wisdom", ACTION_HIGH + 5)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "avenging wrath",
            {
                NextAction("avenging wrath", ACTION_HIGH + 2)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "medium aoe",
            {
                NextAction("divine storm", ACTION_HIGH + 4),
                NextAction("consecration", ACTION_HIGH + 3)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "enemy out of melee",
            {
                NextAction("reach melee", ACTION_HIGH + 1)
            }
        )
    );

    // By leewheel 2026-09-28 玩家反馈：带小号练级时圣骑士 bot（默认 dps 策略）战斗中不治疗队友，
    // 只有脱战才加血——队伍治疗触发此前全在 heal/offheal 策略里，dps 策略一条没挂。
    // 此处对齐 OffhealRetPaladinStrategy 的治疗触发组（优先级同款）：战斗中队友危急/低血/中血
    // 均会读条治疗；10 级以下未学圣光闪现时，flash of light 动作经 ActionNodeFactory 自动回落圣光术
    triggers.push_back(
        new TriggerNode(
            "party member critical health",
            {
                NextAction("holy shock on party", ACTION_CRITICAL_HEAL + 6),
                NextAction("holy light on party", ACTION_CRITICAL_HEAL + 4)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member low health",
            {
                NextAction("holy light on party", ACTION_MEDIUM_HEAL + 5)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member medium health",
            {
                NextAction("flash of light on party", ACTION_LIGHT_HEAL + 8)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member to heal out of spell range",
            {
                NextAction("reach party member to heal", ACTION_EMERGENCY + 3)
            }
        )
    );
    // bot 自身低血也自保（与 OffhealRet 同款），避免战斗中自己残血只会硬扛
    triggers.push_back(
        new TriggerNode(
            "low health",
            {
                NextAction("holy light", ACTION_CRITICAL_HEAL + 2)
            }
        )
    );
    // End By leewheel
}
