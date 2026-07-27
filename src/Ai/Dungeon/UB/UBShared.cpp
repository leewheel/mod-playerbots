/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

//By leewheel 2026-07-27 引入brighton-chi的UB(幽暗沼泽)副本策略
// 幽暗沼泽共享工具实现文件
// 提供蘑菇检测、危险判定、撤退路径安全检查等功能
#include "UBShared.h"
#include "Creature.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellMgr.h"

#include <algorithm>
#include <cmath>
#include <list>

namespace UnderbogHungarfen
{
    // 获取法术效果最大半径，失败时返回回退值
    float MaxEffectRadius(uint32 spellId, float fallback)
    {
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo)
            return fallback;

        float radius = 0.0f;
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            radius = std::max(radius, spellInfo->Effects[i].CalcRadius());

        return radius > 0.0f ? radius : fallback;
    }

    // 计算蘑菇对机器人的危险范围 = 孢子云半径 + 机器人战斗触及 + 安全余量
    float MushroomDangerRange(Player* bot)
    {
        return MaxEffectRadius(SPELL_SPORE_CLOUD, SPORE_CLOUD_RADIUS_FALLBACK) + bot->GetCombatReach() +
               SPORE_CLOUD_MARGIN;
    }

    // 判断蘑菇是否危险：已施放孢子云 或 生长层数 >= 8
    bool IsMushroomDangerous(Creature* mushroom)
    {
        if (!mushroom || !mushroom->IsAlive())
            return false;

        // 已施放孢子云的蘑菇直接视为危险
        if (mushroom->HasAura(SPELL_SPORE_CLOUD))
            return true;

        // 生长层数达到阈值时视为危险
        Aura* grow = mushroom->GetAura(SPELL_GROW);
        return grow && grow->GetStackAmount() >= GROW_STACKS_DANGER;
    }

    // 查找附近所有存活的蘑菇GUID
    GuidVector FindMushroomGuids(Player* bot)
    {
        std::list<Creature*> mushrooms;
        bot->GetCreatureListWithEntryInGrid(mushrooms, NPC_UNDERBOG_MUSHROOM, MUSHROOM_SCAN_RANGE);

        GuidVector guids;
        for (Creature* mushroom : mushrooms)
        {
            if (mushroom->IsAlive())
                guids.push_back(mushroom->GetGUID());
        }

        return guids;
    }

    // 获取指定范围内的最近危险蘑菇
    Creature* GetNearestDangerousMushroom(Player* bot, GuidVector const& mushrooms, float range)
    {
        Creature* best = nullptr;
        float bestDist = range;
        for (ObjectGuid guid : mushrooms)
        {
            Creature* mushroom = ObjectAccessor::GetCreature(*bot, guid);
            if (!IsMushroomDangerous(mushroom))
                continue;

            float const dist = bot->GetDistance2d(mushroom);
            if (dist <= bestDist)
            {
                bestDist = dist;
                best = mushroom;
            }
        }

        return best;
    }

    namespace
    {
        // 计算点到线段的2D距离（用于检查撤退路线是否经过蘑菇）
        float PointSegmentDist2d(float px, float py, float ax, float ay, float bx, float by)
        {
            float const dx = bx - ax;
            float const dy = by - ay;
            float const len2 = dx * dx + dy * dy;
            float t = (len2 < 1e-6f) ? 0.0f : ((px - ax) * dx + (py - ay) * dy) / len2;
            t = std::max(0.0f, std::min(1.0f, t));
            return std::hypot(px - (ax + t * dx), py - (ay + t * dy));
        }
    }

    // 检查撤退路线是否不安全：终点或路线经过危险蘑菇
    bool RetreatPathUnsafe(Player* bot, GuidVector const& mushrooms, float destX, float destY)
    {
        float const dangerRange = MushroomDangerRange(bot);
        float const startX = bot->GetPositionX();
        float const startY = bot->GetPositionY();

        for (ObjectGuid guid : mushrooms)
        {
            Creature* mushroom = ObjectAccessor::GetCreature(*bot, guid);
            if (!mushroom || !mushroom->IsAlive())
                continue;

            // 终点在蘑菇危险范围内
            if (mushroom->GetExactDist2d(destX, destY) < dangerRange)
                return true;

            // 撤退路线经过危险蘑菇
            if (IsMushroomDangerous(mushroom) &&
                PointSegmentDist2d(mushroom->GetPositionX(), mushroom->GetPositionY(), startX, startY, destX, destY) <
                    dangerRange)
                return true;
        }

        return false;
    }
}
