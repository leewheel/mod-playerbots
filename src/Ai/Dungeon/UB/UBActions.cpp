/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

//By leewheel 2026-07-27 引入brighton-chi的UB(幽暗沼泽)副本策略
// 幽暗沼泽动作实现文件
// UBRetreatFromFoulSporesAction: 恶臭孢子施放时，计算安全距离并多角度寻找安全撤退路线
// UBVacateSporeCloudAction: 找到最近的危险蘑菇，坦克拉开距离/其他人逃跑
#include "UBActions.h"
#include "Playerbots.h"
#include "UBShared.h"

#include <cmath>

using namespace UnderbogHungarfen;

// 恶臭孢子撤退：计算Boss技能半径+安全余量，从多个角度尝试找到不经过蘑菇的安全路线
bool UBRetreatFromFoulSporesAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "hungarfen");
    if (!boss)
        return false;

    // 计算安全距离 = 技能最大半径 + 安全余量
    float const safeDistance = MaxEffectRadius(SPELL_FOUL_SPORES, FOUL_SPORES_RADIUS_FALLBACK) + FOUL_SPORES_MARGIN;
    float const currentDistance = bot->GetDistance2d(boss);
    if (currentDistance >= safeDistance)
        return false;

    float const moveDist = safeDistance - currentDistance + 1.0f;
    float const awayAngle = boss->GetAngle(bot);

    GuidVector const& mushrooms = AI_VALUE_REF(GuidVector, "ub mushrooms");

    // 从远离Boss的方向开始，多个角度尝试找到不经过蘑菇的安全路线
    for (float delta : { 0.0f, float(M_PI / 8), float(-M_PI / 8), float(M_PI / 4), float(-M_PI / 4),
                         float(3 * M_PI / 8), float(-3 * M_PI / 8), float(M_PI / 2), float(-M_PI / 2) })
    {
        float const angle = awayAngle + delta;
        float dx = bot->GetPositionX() + cos(angle) * moveDist;
        float dy = bot->GetPositionY() + sin(angle) * moveDist;
        float dz = bot->GetPositionZ();
        // 检查碰撞和地形有效性
        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                            bot->GetPositionZ(), dx, dy, dz))
            continue;

        // 检查撤退路线是否经过危险蘑菇
        if (RetreatPathUnsafe(bot, mushrooms, dx, dy))
            continue;

        if (MoveTo(bot->GetMapId(), dx, dy, dz, false, false, true, true,
                   MovementPriority::MOVEMENT_COMBAT))
            return true;
    }

    // 所有角度都不可行时，直接远离Boss
    return MoveAway(boss, moveDist);
}

// 孢子云规避：找到最近的危险蘑菇并远离
// 坦克如果是当前目标的目标，直接拉开距离；其他人逃跑到安全位置
bool UBVacateSporeCloudAction::Execute(Event /*event*/)
{
    float const dangerRange = MushroomDangerRange(bot);
    GuidVector const& mushrooms = AI_VALUE_REF(GuidVector, "ub mushrooms");
    Creature* mushroom = GetNearestDangerousMushroom(bot, mushrooms, dangerRange);
    if (!mushroom)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "hungarfen");
    // 坦克正在被Boss攻击时，直接拉开与蘑菇的距离
    if (botAI->IsTank(bot) && boss && boss->GetVictim() == bot)
    {
        float const currentDistance = bot->GetDistance2d(mushroom);
        return MoveAway(mushroom, dangerRange - currentDistance + 2.0f);
    }

    // 其他人逃跑到蘑菇危险范围之外
    return FleePosition(mushroom->GetPosition(), dangerRange);
}
