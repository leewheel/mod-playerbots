/*
* This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
* information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
* or (at your option) any later version.
*/

#include "SHTriggers.h"
#include "AiObject.h"
#include "AiObjectContext.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"

using namespace EncounterHelpers;

// By leewheel 2026-09-16 破碎大厅触发条件实现

// 高阶术士奈瑟库斯：暗影旋风（30502）
// 脚本用 ScheduleHealthCheckEvent(25, ...) 在 25% 血量时 DoCastSelf，是一次性的原地 AoE。
// 判定同时看 aura 与"当前正在施放"，避免因为法术是瞬发/引导而在某一帧漏判。
bool NethekurseDarkSpinTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "16807");
    if (!boss)
        return false;

    auto const spell = static_cast<uint32>(ShatteredHallsIDs::SPELL_DARK_SPIN);
    return boss->HasAura(spell) || boss->FindCurrentSpellBySpellId(spell);
}

// 血卫士波伦（英雄 20923）/ 血卫士（普通 17461）：弓手火焰箭在地面留下的烈焰（181915）
// 机制：弓手每隔一段时间朝随机玩家射火焰箭，落点生成烈焰；同一个目标不会被连续点名两次，
//       且"脚下已经有烈焰"的玩家不会再被选中 —— 所以正确答案就是踩到烈焰后立刻挪窝。
bool PorungBlazeOnGroundTrigger::IsActive()
{
    // 只在波伦 / 血卫士的战斗中生效，避免 181915（通用的"烈焰"物件）在其他场景误触发
    Unit* boss = AI_VALUE2(Unit*, "find target", "20923");
    if (!boss)
        boss = AI_VALUE2(Unit*, "find target", "17461");
    if (!boss)
        return false;

    return bot->FindNearestGameObject(
               static_cast<uint32>(ShatteredHallsIDs::GO_BLAZE), 12.0f) != nullptr;
}

// 战争使者沃姆罗格：燃烧之锤（30598）
// 脚本的燃烧阶段会先 Fear 再套上 Burning Maul，随后反复 Blast Wave。
// 因此"身上有燃烧之锤"= 已经进入火焰阶段，非坦克应当离开近战圈。
bool OmroggBurningMaulTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "16809");
    return boss && boss->HasAura(static_cast<uint32>(ShatteredHallsIDs::SPELL_BURNING_MAUL));
}

// 酋长卡加斯·刃拳：刃舞
// 脚本 DoCastAOE(30738) 选目标 → 30751 冲锋 → 30739 伤害，期间 BOSS 处于 REACT_PASSIVE。
// 三个法术任一出现在 BOSS 身上都说明刃舞进行中（30739 是刃舞伤害光环，最稳定）。
bool KargathBladeDanceTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "16808");
    if (!boss)
        return false;

    return boss->HasAura(static_cast<uint32>(ShatteredHallsIDs::SPELL_BLADE_DANCE_TARGETING)) ||
           boss->HasAura(static_cast<uint32>(ShatteredHallsIDs::SPELL_BLADE_DANCE_DMG)) ||
           boss->HasAura(static_cast<uint32>(ShatteredHallsIDs::SPELL_BLADE_DANCE_CHARGE));
}

// 酋长卡加斯：破碎刺客（17695）会被反复召唤（杀死后 20 秒复活），把 DPS 导向它们是正解
bool KargathAssassinsAreActiveTrigger::IsActive()
{
    return botAI->IsDps(bot) && AI_VALUE2(Unit*, "find target", "17695") != nullptr;
}

// 下水道：队伍里的真实玩家已经跳下去（Z 在下水道层），而机器人还站在上层
// —— 参考 SWP 艾瑞达双子的做法：用 Z 坐标判定"还站在上面"，再决定跳下去。
bool ShatteredHallsMasterInSewerTrigger::IsActive()
{
    if (bot->GetMapId() != SH_MAP_ID)
        return false;

    // 机器人本身已经在下层就不用再跳了
    if (bot->GetPositionZ() < SH_SEWER_UPPER_Z)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || member == bot || !member->IsInWorld())
            continue;

        if (member->GetMapId() != bot->GetMapId())
            continue;

        // 只认真实玩家（机器人队友站在下面不构成"跳下去"的理由）
        if (member->GetSession() && member->GetSession()->IsBot())
            continue;

        if (member->GetPositionZ() > SH_SEWER_LOWER_Z)
            continue;

        // 水平太远说明两者不在同一个区域（比如他在别处），不做硬跳
        if (bot->GetExactDist2d(member) > SH_SEWER_MAX_HORIZONTAL)
            continue;

        return true;
    }

    return false;
}
