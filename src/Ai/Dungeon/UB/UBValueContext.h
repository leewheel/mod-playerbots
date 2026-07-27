/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

//By leewheel 2026-07-27 引入brighton-chi的UB(幽暗沼泽)副本策略
// 幽暗沼泽值上下文：注册蘑菇列表计算值
// - ub mushrooms: 定期扫描附近的幽暗沼泽蘑菇并返回GUID列表
#ifndef PLAYERBOTS_UBVALUECONTEXT_H
#define PLAYERBOTS_UBVALUECONTEXT_H

#include "NamedObjectContext.h"
#include "UBShared.h"
#include "Value.h"

// 蘑菇列表计算值：每200ms扫描附近存活的蘑菇
class UnderbogMushroomsValue : public CalculatedValue<GuidVector>
{
public:
    UnderbogMushroomsValue(PlayerbotAI* botAI) : CalculatedValue<GuidVector>(botAI, "ub mushrooms", 200) {}

protected:
    GuidVector Calculate() override { return UnderbogHungarfen::FindMushroomGuids(bot); }
};

class TbcDungeonUnderbogValueContext : public NamedObjectContext<UntypedValue>
{
public:
    TbcDungeonUnderbogValueContext() { creators["ub mushrooms"] = &TbcDungeonUnderbogValueContext::ub_mushrooms; }

private:
    // 蘑菇列表值
    static UntypedValue* ub_mushrooms(PlayerbotAI* botAI) { return new UnderbogMushroomsValue(botAI); }
};

#endif
