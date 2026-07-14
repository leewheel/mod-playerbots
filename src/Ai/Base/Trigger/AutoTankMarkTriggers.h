/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_AUTOTANKMARKTRIGGERS_H
#define PLAYERBOTS_AUTOTANKMARKTRIGGERS_H

#include "Trigger.h"

class PlayerbotAI;

// By leewheel 2026-07-15: 主坦克自动标记骷髅触发器
// 条件：配置开启 + 在副本/团本中 + 是主坦克 + 有未标记骷髅的敌人
class MainTankMarkSkullTrigger : public Trigger
{
public:
    MainTankMarkSkullTrigger(PlayerbotAI* botAI) : Trigger(botAI, "main tank can mark skull") {}

    bool IsActive() override;
};

// By leewheel 2026-07-15: 副坦克自动标记叉叉触发器
// 条件：配置开启 + 在副本/团本中 + 是副坦克 + 有未标记叉叉的敌人
class OffTankMarkCrossTrigger : public Trigger
{
public:
    OffTankMarkCrossTrigger(PlayerbotAI* botAI) : Trigger(botAI, "off tank can mark cross") {}

    bool IsActive() override;
};

// By leewheel 2026-07-15: 5人副本主坦克兼任标记叉叉触发器
// 条件：配置开启 + 在副本/团本中 + 是主坦克 + 队伍中没有其他坦克（5人场景）
// + 叉叉未被占用 + 有至少2个未标记敌人
class MainTankMarkCrossTrigger : public Trigger
{
public:
    MainTankMarkCrossTrigger(PlayerbotAI* botAI) : Trigger(botAI, "main tank can mark cross") {}

    bool IsActive() override;
};

#endif
