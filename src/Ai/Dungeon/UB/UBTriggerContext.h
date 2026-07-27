/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

//By leewheel 2026-07-27 引入brighton-chi的UB(幽暗沼泽)副本策略
// 幽暗沼泽触发器上下文：注册Hungarfen遭遇战中的触发器
// - ub foul spores: 检测Boss是否施放恶臭孢子
// - ub spore cloud danger: 检测附近是否有危险蘑菇
#ifndef PLAYERBOTS_UBTRIGGERCONTEXT_H
#define PLAYERBOTS_UBTRIGGERCONTEXT_H

#include "AiObjectContext.h"
#include "TriggerContext.h"
#include "UBTriggers.h"

class TbcDungeonUnderbogTriggerContext : public NamedObjectContext<Trigger>
{
public:
    TbcDungeonUnderbogTriggerContext()
    {
        creators["ub foul spores"] = &TbcDungeonUnderbogTriggerContext::ub_foul_spores;
        creators["ub spore cloud danger"] = &TbcDungeonUnderbogTriggerContext::ub_spore_cloud_danger;
    }

private:
    // 恶臭孢子触发器
    static Trigger* ub_foul_spores(PlayerbotAI* botAI) { return new UBFoulSporesTrigger(botAI); }

    // 孢子云危险触发器
    static Trigger* ub_spore_cloud_danger(PlayerbotAI* botAI) { return new UBSporeCloudDangerTrigger(botAI); }
};

#endif
