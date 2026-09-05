/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BWLHELPERS_H
#define PLAYERBOTS_BWLHELPERS_H

#include "Player.h"
#include "PlayerbotAI.h"

namespace BlackwingLairHelpers
{
    enum class BlackwingLairSpells : uint32
    {
        // General
        SPELL_ONYXIA_SCALE_CLOAK = 22683,

        // Razorgore the Untamed
        SPELL_MINDCONTROL = 19832,

        // Vaelastrasz the Corrupt
        SPELL_BURNING_ADRENALINE = 18173,

        // Chromaggus
        SPELL_BROOD_AFFLICTION_BRONZE = 23170,
        SPELL_HOURGLASS_SAND = 23645,

        // Nefarian
        SPELL_WILD_MAGIC = 23410
    };

    enum class BlackwingLairGameObjects : uint32
    {
        // General
        GO_SUPPRESSION_DEVICE = 179784,

        // Razorgore the Untamed
        GO_BLACK_DRAGON_EGG = 177807
    };

    enum class BlackwingLairNPCs : uint32
    {
        // Trash
        NPC_DEATH_TALON_WYRMGUARD = 12460,

    //By leewheel 2026年7月12日
    // 自定义Boss: Valthorax
    NPC_VALTHORAX = 100184,
        NPC_VABOMINATION = 400153,      // 憎恶治疗小怪，移动到Boss身边治疗后消失
        NPC_SKELETAL_WARRIOR = 400150, // 骷髅战士
        NPC_GHOUL = 400151,            // 食尸鬼
        NPC_BANSHEE = 400152,          // 女妖/织魂者
    };

    // 自定义Boss: Valthorax — 法术ID
    // By leewheel 2026-09-04 合并brighton-chi/the-lab: 保留本分支自定义Boss定义, 函数签名随上游east-const风格。
    enum ValthoraxSpells
    {
        SPELL_VALTHORAX_FROSTBOMB = 80031,     // 50%血量时对自身施放
        SPELL_VALTHORAX_SELF_ROOT = 80000,      // 50%血量时定身自身15秒
        SPELL_VALTHORAX_FROSTBREATH = 21099,    // 冰霜吐息 (victim)
        SPELL_VALTHORAX_SHADOW_BOLT_VOLLEY = 20741,
        SPELL_VALTHORAX_NECROTIC_AURA = 80030,  // 亡灵光环
        SPELL_VALTHORAX_ENRAGE = 28468,         // 15%血量狂暴
    };

    // 自定义Boss名字（数据库中的creature_template.name）
    static constexpr const char* BOSS_NAME_VALTHORAX = "死亡使者瓦索拉克斯";
    //End By leewheel

    bool IsActiveSuppressionDeviceInRange(GameObject const* go, Player const* bot);
    bool AreRazorgoreEggsAlive(PlayerbotAI* botAI);
    bool IsRazorgoreOffTank(Player* bot);
    bool IsNonBABotNearPosition(Player const* bot, Position const& position, float distance);
}

#endif
