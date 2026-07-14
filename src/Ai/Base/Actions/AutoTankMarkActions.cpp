/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AutoTankMarkActions.h"

#include "AttackersValue.h"
#include "Event.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"

// By leewheel 2026-07-15: 检查目标是否已被任何标记
static bool IsTargetAlreadyMarked(Group* group, ObjectGuid targetGuid)
{
    if (!group || targetGuid.IsEmpty())
        return false;

    for (uint8 i = 0; i < 8; i++)
    {
        if (group->GetTargetIcon(i) == targetGuid)
            return true;
    }
    return false;
}

// By leewheel 2026-07-15: 选择最优标记目标的辅助函数
// mode=0: 选择最大生命值目标（骷髅），mode=1: 选择第二大生命值目标（叉叉，排除skull）
static Unit* SelectMarkTarget(PlayerbotAI* botAI, Group* group, int mode)
{
    if (!botAI || !group)
        return nullptr;

    Player* bot = botAI->GetBot();
    if (!bot)
        return nullptr;

    GuidVector attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();
    ObjectGuid skullGuid = group->GetTargetIcon(RtiTargetValue::skullIndex);

    Unit* bestTarget = nullptr;
    uint32 bestMaxHealth = 0;

    for (ObjectGuid const guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || unit->IsPlayer())
            continue;

        // 已标记的跳过
        if (IsTargetAlreadyMarked(group, unit->GetGUID()))
            continue;

        // 叉叉模式：跳过骷髅标记的目标
        if (mode == 1 && unit->GetGUID() == skullGuid)
            continue;

        // 选择最大生命值的目标（通常是最危险的）
        uint32 maxHealth = unit->GetMaxHealth();
        if (maxHealth > bestMaxHealth)
        {
            bestMaxHealth = maxHealth;
            bestTarget = unit;
        }
    }

    // 如果没找到最大生命值的目标，退而选择任意未标记目标
    if (!bestTarget)
    {
        for (ObjectGuid const guid : attackers)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (!unit || unit->IsPlayer())
                continue;

            if (IsTargetAlreadyMarked(group, unit->GetGUID()))
                continue;

            if (mode == 1 && unit->GetGUID() == skullGuid)
                continue;

            bestTarget = unit;
            break;
        }
    }

    return bestTarget;
}

bool MarkSkullTargetAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    if (bot->InBattleground())
        return false;

    Unit* target = SelectMarkTarget(botAI, group, 0);
    if (!target)
        return false;

    // 设置骷髅标记 (index = 7)
    group->SetTargetIcon(RtiTargetValue::skullIndex, bot->GetGUID(), target->GetGUID());

    return true;
}

bool MarkCrossTargetAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    if (bot->InBattleground())
        return false;

    Unit* target = SelectMarkTarget(botAI, group, 1);
    if (!target)
        return false;

    // 设置叉叉标记 (index = 6)
    group->SetTargetIcon(RtiTargetValue::crossIndex, bot->GetGUID(), target->GetGUID());

    return true;
}
