/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MCHELPERS_H
#define PLAYERBOTS_MCHELPERS_H

namespace MoltenCoreHelpers
{
enum MoltenCoreNPCs
{
    // Golemagg
    NPC_CORE_RAGER = 11672,

    // Core Hound (trash)
    NPC_CORE_HOUND = 11671,

    //By leewheel 2026年7月12日
    // 自定义Boss: Smolder (NPC 83001)
    NPC_SMOLDER = 83001,
    NPC_FLAME_TSUNAMI_FIRST = 83006,  // 火焰海啸NPC范围起始
    NPC_FLAME_TSUNAMI_LAST = 83016,   // 火焰海啸NPC范围结束
    NPC_CHEATER_KILLER = 83020,       // 作弊检测NPC（封锢入口）

    // 自定义Boss: Hazzrash (NPC 83000)
    NPC_HAZZRASH = 83000,
    NPC_SHAZZRAH = 12264,             // Hazzrash只在Shazzrah存活时存在
};
enum MoltenCoreSpells
{
    // Baron Geddon
    SPELL_INFERNO = 19695,
    SPELL_LIVING_BOMB = 20475,

    // Golemagg
    SPELL_GOLEMAGGS_TRUST = 20553,

    // 自定义Boss: Smolder
    SPELL_SMOLDER_BELLOWING_ROAR = 22686,  // AOE恐惧，33s间隔
    SPELL_SMOLDER_TAIL_SWEEP = 52144,       // 尾部扫击
    SPELL_SMOLDER_BOMB = 80001,             // Smolder炸弹，随机3目标
    SPELL_SMOLDER_SCORCH = 42858,           // 灼烧，对坦克
    SPELL_SMOLDER_SUMMON_ELEMENTAL = 364728,// 召唤火元素

    // 自定义Boss: Hazzrash
    SPELL_HAZZRASH_EVOCATION = 30254,       // 引导法术，暂停攻击20秒
    SPELL_HAZZRASH_ARCANE_BARRAGE = 44425,  // 奥术弹幕
    SPELL_HAZZRASH_ARCANE_BLAST = 30451,    // 奥术冲击
    SPELL_HAZZRASH_CHAIN_BURN = 8211,       // 连锁燃烧
};

// 自定义Boss名字（数据库中的creature_template.name）
static constexpr const char* BOSS_NAME_SMOLDER = "阴燃";
static constexpr const char* BOSS_NAME_HAZZRASH = "赫兹拉斯";
//End By leewheel
}

#endif
