/*
* This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
* information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
* or (at your option) any later version.
*/

#ifndef PLAYERBOTS_SHTRIGGERS_H
#define PLAYERBOTS_SHTRIGGERS_H

#include "Trigger.h"
#include "GenericTriggers.h"
#include "DungeonStrategyUtils.h"

// By leewheel 2026-09-16
// 破碎大厅（地狱火堡垒：破碎大厅，map 540）机器人策略 —— 触发条件定义
// 本次新增（此前 mod-playerbots 完全没有破碎大厅策略）：
//   * 四个首领（奈瑟库斯 / 波伦 / 沃姆罗格 / 卡加斯）的关键技能规避
//   * 「玩家跳下下水道、机器人跟着跳下去」——参考 SWP 艾瑞达双子（阳台跳下）的实现
//     即：用 Z 坐标判定"机器人还在上层"，再用 MoveTo(边缘) + JumpTo(落点) 下到下层。

// 破碎大厅关键法术 / 物件
enum class ShatteredHallsIDs : uint32
{
    // 高阶术士奈瑟库斯（16807）
    SPELL_DARK_SPIN             = 30502,   // 暗影旋风：25% 血量触发的原地 AoE，近战必须跑开

    // 战争使者沃姆罗格（16809）
    SPELL_BURNING_MAUL          = 30598,   // 燃烧之锤：进入"燃烧阶段"，随之而来的是火焰冲击波
    SPELL_BLAST_WAVE            = 30600,   // 火焰冲击波：燃烧阶段的 AoE
    SPELL_FEAR                  = 30584,   // 恐惧

    // 酋长卡加斯·刃拳（16808）—— 刃舞三件套
    SPELL_BLADE_DANCE_TARGETING = 30738,   // 刃舞：选点
    SPELL_BLADE_DANCE_DMG       = 30739,   // 刃舞：伤害
    SPELL_BLADE_DANCE_CHARGE    = 30751,   // 刃舞：冲锋

    // 血卫士波伦（20923 / 普通模式 17461 血卫士）—— 弓手火焰箭留下的地面烈焰
    GO_BLAZE                    = 181915,
};

// 破碎大厅地图 ID（地狱火堡垒：破碎大厅）
inline constexpr uint32 SH_MAP_ID = 540;

// 关键生物 entry（与 src/server/scripts/.../ShatteredHalls/shattered_halls.h 对齐）
enum class ShatteredHallsNPCs : uint32
{
    BOSS_NETHEKURSE = 16807,   // 高阶术士奈瑟库斯
    BOSS_OMROGG     = 16809,   // 战争使者沃姆罗格
    BOSS_KARGATH    = 16808,   // 酋长卡加斯·刃拳
    BOSS_PORUNG     = 20923,   // 血卫士波伦（英雄模式）
    BLOOD_GUARD     = 17461,   // 血卫士（普通模式下波伦的占位）
    ASSASSIN        = 17695,   // 破碎刺客（卡加斯召唤，死后 20 秒复活）
};

// By leewheel 2026-09-16
// 下水道"跳下去"的坐标常量。
// 全图唯一的垂直落差通道：上层 (120.9, 252.8, -14.6) → 下水道 (119.8, 252.1, -45.2)。
// 水平间距 1.3 码、垂直落差 30.65 码 —— 由世界库快照 world-20260903_update.sql 的
// 422 个 map 540 刷点做「垂直对齐」分析（2D<6 且 dz>18）得出，全图仅此一处。
// 上层落脚参考 17288（兰迪·维兹普罗克）刷点，下层落脚参考 17357 / 4075（淤泥怪 / 老鼠）刷点。
inline constexpr float SH_SEWER_EDGE_X  = 120.88f;
inline constexpr float SH_SEWER_EDGE_Y  = 252.78f;
inline constexpr float SH_SEWER_EDGE_Z  = -14.57f;
inline constexpr float SH_SEWER_LAND_X  = 119.80f;
inline constexpr float SH_SEWER_LAND_Y  = 252.08f;
inline constexpr float SH_SEWER_LAND_Z  = -45.22f;
// Z 分层判据：机器人 Z >= UPPER 视为"还在上层"，主人 Z <= LOWER 视为"已经跳下去了"
inline constexpr float SH_SEWER_UPPER_Z = -30.0f;
inline constexpr float SH_SEWER_LOWER_Z = -35.0f;
// 主人与机器人的水平距离上限：超过它说明两者根本不在同一个区域，不做硬跳
inline constexpr float SH_SEWER_MAX_HORIZONTAL = 60.0f;
// End By leewheel

// 高阶术士奈瑟库斯

class NethekurseDarkSpinTrigger : public Trigger
{
public:
    NethekurseDarkSpinTrigger(PlayerbotAI* botAI) : Trigger(botAI, "nethekurse dark spin") {}

    bool IsActive() override;
};

// 血卫士波伦（英雄）/ 血卫士（普通）—— 地面烈焰

class PorungBlazeOnGroundTrigger : public Trigger
{
public:
    PorungBlazeOnGroundTrigger(PlayerbotAI* botAI) : Trigger(botAI, "porung blaze on ground") {}

    bool IsActive() override;
};

// 战争使者沃姆罗格

class OmroggBurningMaulTrigger : public Trigger
{
public:
    OmroggBurningMaulTrigger(PlayerbotAI* botAI) : Trigger(botAI, "omrogg burning maul") {}

    bool IsActive() override;
};

// 酋长卡加斯·刃拳

class KargathBladeDanceTrigger : public Trigger
{
public:
    KargathBladeDanceTrigger(PlayerbotAI* botAI) : Trigger(botAI, "kargath blade dance") {}

    bool IsActive() override;
};

class KargathAssassinsAreActiveTrigger : public Trigger
{
public:
    KargathAssassinsAreActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "kargath assassins are active") {}

    bool IsActive() override;
};

// 下水道：主人跳下去了，机器人还在上层

class ShatteredHallsMasterInSewerTrigger : public Trigger
{
public:
    ShatteredHallsMasterInSewerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "shattered halls master in sewer") {}

    bool IsActive() override;
};

#endif
