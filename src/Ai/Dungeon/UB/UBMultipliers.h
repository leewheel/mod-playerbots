/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

//By leewheel 2026-07-27 引入brighton-chi的UB(幽暗沼泽)副本策略
// 幽暗沼泽乘数头文件
// - HungarfenFoulSporesMultiplier: 恶臭孢子期间压制非必要动作
// - HungarfenMushroomIgnoreMultiplier: DPS不应对蘑菇施放AOE
#ifndef PLAYERBOTS_UBMULTIPLIERS_H
#define PLAYERBOTS_UBMULTIPLIERS_H

#include "Multiplier.h"

// 恶臭孢子乘数：Boss施放恶臭孢子时，压制移动和接近目标类动作
class HungarfenFoulSporesMultiplier : public Multiplier
{
public:
    HungarfenFoulSporesMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "hungarfen foul spores") {}
    float GetValue(Action* action) override;
};

// 蘑菇忽略乘数：DPS在与Hungarfen战斗时不应对蘑菇施放AOE法术
class HungarfenMushroomIgnoreMultiplier : public Multiplier
{
public:
    HungarfenMushroomIgnoreMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "hungarfen mushroom ignore") {}
    float GetValue(Action* action) override;
};

#endif
