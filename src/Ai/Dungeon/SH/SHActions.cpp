/*
* This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
* information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
* or (at your option) any later version.
*/

#include "Playerbots.h"
#include "EncounterHelpers.h"
#include "GameObject.h"
#include "SHTriggers.h"
#include "SHActions.h"

using namespace EncounterHelpers;

// By leewheel 2026-09-16 破碎大厅（map 540）机器人动作实现

// 高阶术士奈瑟库斯：暗影旋风
// 30502 是原地 AoE（带击退），坦克之外的人留在近战圈里会被连续打。
// 做法与 Gundrak 的 Whirling Slash 一致：非坦克离开到安全半径之外。
bool NethekurseAvoidDarkSpinAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "16807");
    if (!boss)
        return false;

    if (botAI->IsTank(bot))
        return false;

    constexpr float safeRadius = 20.0f;
    constexpr float distanceExtra = 2.0f;
    float const distance = bot->GetExactDist2d(boss->GetPosition());

    if (distance < safeRadius + distanceExtra)
        return MoveAway(boss, safeRadius + distanceExtra - distance);

    return false;
}

// 血卫士波伦（英雄 20923）/ 血卫士（普通 17461）：地面烈焰
// 弓手的火焰箭会在随机玩家脚下留下烈焰（GO 181915）。踩在烈焰上会持续受火伤，
// 正确做法就是立刻离开烈焰范围 —— 用 FleePosition 从烈焰中心向外挪 8 码。
bool PorungMoveOutOfBlazeAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "20923");
    if (!boss)
        boss = AI_VALUE2(Unit*, "find target", "17461");
    if (!boss)
        return false;

    GameObject* blaze = bot->FindNearestGameObject(
        static_cast<uint32>(ShatteredHallsIDs::GO_BLAZE), 12.0f);
    if (!blaze)
        return false;

    return FleePosition(blaze->GetPosition(), 8.0f);
}

// 战争使者沃姆罗格：燃烧之锤阶段
// 燃烧阶段 = Fear(30584) + Burning Maul(30598) + 反复 Blast Wave(30600)。
// 非坦克离开近战圈可以同时躲掉 Blast Wave 的火焰冲击与燃烧之锤 DoT。
bool OmroggAvoidBurningMaulAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "16809");
    if (!boss)
        return false;

    if (botAI->IsTank(bot))
        return false;

    constexpr float safeRadius = 12.0f;
    constexpr float distanceExtra = 2.0f;
    float const distance = bot->GetExactDist2d(boss->GetPosition());

    if (distance < safeRadius + distanceExtra)
        return MoveAway(boss, safeRadius + distanceExtra - distance);

    return false;
}

// 酋长卡加斯·刃拳：刃舞
// 刃舞期间 BOSS 会连续 8 次朝 5~16 码内的落点冲锋（30751 + 30739 AoE）。
// 站在 20 码外是唯一稳的做法（贴到 5 码内也行，但机器人容易被随后的冲锋蹭到）。
bool KargathAvoidBladeDanceAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "16808");
    if (!boss)
        return false;

    if (botAI->IsTank(bot))
        return false;

    constexpr float safeRadius = 20.0f;
    constexpr float distanceExtra = 2.0f;
    float const distance = bot->GetExactDist2d(boss->GetPosition());

    if (distance < safeRadius + distanceExtra)
        return MoveAway(boss, safeRadius + distanceExtra - distance);

    return false;
}

// 酋长卡加斯：给破碎刺客挂骷髅
// 刺客会被反复召唤（死后 20 秒复活），全场集中火力秒掉比硬吃背刺更划算。
bool KargathMarkAssassinsAction::Execute(Event /*event*/)
{
    Unit* assassin = AI_VALUE2(Unit*, "find target", "17695");
    if (!assassin)
        return false;

    if (IsMechanicTrackerBot(bot, SH_MAP_ID) && MarkTargetWithSkull(bot, assassin))
        return true;

    return false;
}

// By leewheel 2026-09-16
// 下水道：主人跳下去了，机器人跟着跳下去。
// 参考 SWP 艾瑞达双子的 EredarTwinsMeleeJumpFromBalconyAction：
//   ① 先在水平面上走到"洞口边缘"（MoveTo）
//   ② 到位后 JumpTo 落点（MoveJump 让机器人真的跳下去，而不是瞬移）
// 全图只有一处垂直落差通道：上层 (120.9, 252.8, -14.6) → 下水道 (119.8, 252.1, -45.2)。
static Player* FindGroupMasterInSewer(Player* bot)
{
    if (bot->GetMapId() != SH_MAP_ID)
        return nullptr;

    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || member == bot || !member->IsInWorld())
            continue;

        if (member->GetMapId() != bot->GetMapId())
            continue;

        // 只认真实玩家
        if (member->GetSession() && member->GetSession()->IsBot())
            continue;

        if (member->GetPositionZ() > SH_SEWER_LOWER_Z)
            continue;

        if (bot->GetExactDist2d(member->GetPosition()) > SH_SEWER_MAX_HORIZONTAL)
            continue;

        return member;
    }

    return nullptr;
}

bool ShatteredHallsFollowMasterIntoSewerAction::Execute(Event /*event*/)
{
    // 已经在下层就不用再跳
    if (bot->GetPositionZ() < SH_SEWER_UPPER_Z)
        return false;

    if (!FindGroupMasterInSewer(bot))
        return false;

    // ① 先走到洞口上方的边缘点
    constexpr float arrivalDistance = 4.0f;
    if (bot->GetExactDist2d(SH_SEWER_EDGE_X, SH_SEWER_EDGE_Y) > arrivalDistance)
    {
        return MoveTo(SH_MAP_ID, SH_SEWER_EDGE_X, SH_SEWER_EDGE_Y, SH_SEWER_EDGE_Z,
                      false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    // ② 到了边缘就跳下去（MoveJump 会走真实的跳跃轨迹，落点在 30 码以下的下水道）
    return JumpTo(SH_MAP_ID, SH_SEWER_LAND_X, SH_SEWER_LAND_Y, SH_SEWER_LAND_Z,
                  MovementPriority::MOVEMENT_FORCED);
}
