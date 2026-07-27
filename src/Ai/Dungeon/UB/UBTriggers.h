/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

//By leewheel 2026-07-27 引入brighton-chi的UB(幽暗沼泽)副本策略
// 幽暗沼泽触发器定义头文件
// - UBFoulSporesTrigger: 检测Hungarfen是否正在施放恶臭孢子
// - UBSporeCloudDangerTrigger: 检测附近是否有危险的蘑菇（孢子云/高生长层数）
#ifndef PLAYERBOTS_UBTRIGGERS_H
#define PLAYERBOTS_UBTRIGGERS_H

#include "Trigger.h"

// 恶臭孢子触发器：Boss拥有恶臭孢子光环时激活
class UBFoulSporesTrigger : public Trigger
{
public:
    UBFoulSporesTrigger(PlayerbotAI* botAI) : Trigger(botAI, "ub foul spores") {}
    bool IsActive() override;
};

// 孢子云危险触发器：附近存在危险蘑菇时激活
class UBSporeCloudDangerTrigger : public Trigger
{
public:
    UBSporeCloudDangerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "ub spore cloud danger") {}
    bool IsActive() override;
};

#endif
