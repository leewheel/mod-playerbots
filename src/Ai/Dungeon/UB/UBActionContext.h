/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

//By leewheel 2026-07-27 引入brighton-chi的UB(幽暗沼泽)副本策略
// 幽暗沼泽动作上下文：注册Hungarfen遭遇战中的机器人动作
// - ub retreat from foul spores: 恶臭孢子时撤退到安全距离
// - ub vacate spore cloud: 蘑菇孢子云危险时远离
#ifndef PLAYERBOTS_UBACTIONCONTEXT_H
#define PLAYERBOTS_UBACTIONCONTEXT_H

#include "Action.h"
#include "AiObjectContext.h"
#include "UBActions.h"

class TbcDungeonUnderbogActionContext : public NamedObjectContext<Action>
{
public:
    TbcDungeonUnderbogActionContext()
    {
        creators["ub retreat from foul spores"] = &TbcDungeonUnderbogActionContext::ub_retreat_from_foul_spores;
        creators["ub vacate spore cloud"] = &TbcDungeonUnderbogActionContext::ub_vacate_spore_cloud;
    }

private:
    // 恶臭孢子撤退动作
    static Action* ub_retreat_from_foul_spores(PlayerbotAI* botAI) { return new UBRetreatFromFoulSporesAction(botAI); }

    // 孢子云规避动作
    static Action* ub_vacate_spore_cloud(PlayerbotAI* botAI) { return new UBVacateSporeCloudAction(botAI); }
};

#endif
