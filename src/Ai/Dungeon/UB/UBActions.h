/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

//By leewheel 2026-07-27 引入brighton-chi的UB(幽暗沼泽)副本策略
// 幽暗沼泽动作定义头文件
// - UBRetreatFromFoulSporesAction: Boss施放恶臭孢子时，机器人撤退到安全距离外
// - UBVacateSporeCloudAction: 蘑菇孢子云危险时，机器人远离危险蘑菇
#ifndef PLAYERBOTS_UBACTIONS_H
#define PLAYERBOTS_UBACTIONS_H

#include "MovementActions.h"

// 恶臭孢子撤退动作：计算安全距离，多角度寻找安全撤退路线
class UBRetreatFromFoulSporesAction : public MovementAction
{
public:
    UBRetreatFromFoulSporesAction(PlayerbotAI* botAI) : MovementAction(botAI, "ub retreat from foul spores") {}
    bool Execute(Event event) override;
};

// 孢子云规避动作：找到最近的危险蘑菇并远离
// 坦克拉怪时直接拉开距离，其他人逃跑到安全位置
class UBVacateSporeCloudAction : public MovementAction
{
public:
    UBVacateSporeCloudAction(PlayerbotAI* botAI) : MovementAction(botAI, "ub vacate spore cloud") {}
    bool Execute(Event event) override;
};

#endif
