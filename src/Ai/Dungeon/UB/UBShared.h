/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

//By leewheel 2026-07-27 引入brighton-chi的UB(幽暗沼泽)副本策略
// 幽暗沼泽共享工具头文件
// 定义 Hungarfen 遭遇战相关的常量（NPC ID、法术ID、安全距离等）
// 以及蘑菇检测、危险判定、撤退路径安全检查等工具函数
#ifndef PLAYERBOTS_UBSHARED_H
#define PLAYERBOTS_UBSHARED_H

#include "ObjectGuid.h"

class Creature;
class Player;

namespace UnderbogHungarfen
{
    // NPC
    constexpr uint32 NPC_UNDERBOG_MUSHROOM = 17990;  // 幽暗沼泽蘑菇

    // 法术
    constexpr uint32 SPELL_FOUL_SPORES = 31673;       // 恶臭孢子（Boss AOE）
    constexpr uint32 SPELL_GROW        = 31698;       // 生长（蘑菇叠加层数）
    constexpr uint32 SPELL_SPORE_CLOUD = 34168;       // 孢子云（蘑菇AOE）

    // 半径回退值（法术数据获取失败时使用）
    constexpr float SPORE_CLOUD_RADIUS_FALLBACK = 8.0f;   // 孢子云默认半径
    constexpr float FOUL_SPORES_RADIUS_FALLBACK = 20.0f;  // 恶臭孢子默认半径

    // 安全余量
    constexpr float SPORE_CLOUD_MARGIN = 1.5f;   // 孢子云安全余量
    constexpr float FOUL_SPORES_MARGIN = 2.0f;   // 恶臭孢子安全余量

    constexpr uint32 GROW_STACKS_DANGER = 8;     // 蘑菇生长层数达到此值时视为危险

    constexpr float MUSHROOM_SCAN_RANGE = 40.0f;  // 蘑菇扫描范围

    // 获取法术效果最大半径，失败时返回回退值
    float MaxEffectRadius(uint32 spellId, float fallback);

    // 计算蘑菇对机器人的危险范围
    float MushroomDangerRange(Player* bot);

    // 判断蘑菇是否处于危险状态（已施放孢子云或生长层数过高）
    bool IsMushroomDangerous(Creature* mushroom);

    // 查找附近所有存活蘑菇的GUID列表
    GuidVector FindMushroomGuids(Player* bot);

    // 获取最近的危险蘑菇
    Creature* GetNearestDangerousMushroom(Player* bot, GuidVector const& mushrooms, float range);

    // 检查从机器人当前位置到目标点的路线是否经过危险蘑菇
    bool RetreatPathUnsafe(Player* bot, GuidVector const& mushrooms, float destX, float destY);
}

#endif
