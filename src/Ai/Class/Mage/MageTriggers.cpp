/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "MageTriggers.h"
#include "DynamicObject.h"
#include "Player.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "Value.h"

bool NoManaGemTrigger::IsActive()
{
    static const std::vector<uint32> gemIds = {
        33312,  // Mana Sapphire
        22044,  // Mana Emerald
        8008,   // Mana Ruby
        8007,   // Mana Citrine
        5513,   // Mana Jade
        5514    // Mana Agate
    };

    for (uint32 gemId : gemIds)
    {
        if (bot->GetItemCount(gemId, false) > 0)  // false = only in bags
            return false;
    }
    return true;
}

bool ArcaneIntellectTrigger::IsActive()
{
    return BuffTrigger::IsActive() && !botAI->HasAura("arcane brilliance", GetTarget());
}

bool MageArmorTrigger::IsActive()
{
    Unit* target = GetTarget();
    return botAI->HasSpell("mage armor") && !botAI->HasAura("mage armor", target) &&
           !botAI->HasAura("ice armor", target) && !botAI->HasAura("frost armor", target) &&
           !botAI->HasAura("molten armor", target);
}

bool MoltenArmorTrigger::IsActive()
{
    Unit* target = GetTarget();
    return botAI->HasSpell("molten armor") && !botAI->HasAura("molten armor", target) &&
           !botAI->HasAura("ice armor", target) && !botAI->HasAura("frost armor", target) &&
           !botAI->HasAura("mage armor", target);
}

bool FrostNovaOnTargetTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (!target || !target->IsAlive() || !target->IsInWorld())
        return false;

    return botAI->HasAura(spell, target);
}

bool FrostbiteOnTargetTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (!target || !target->IsAlive() || !target->IsInWorld())
        return false;

    return botAI->HasAura(spell, target);
}

bool NoFocusMagicTrigger::IsActive()
{
    constexpr uint32 SPELL_FOCUS_MAGIC = 54646;
    if (!bot->HasSpell(SPELL_FOCUS_MAGIC))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive())
            continue;

        if (member->HasAura(SPELL_FOCUS_MAGIC, bot->GetGUID()))
            return false;
    }
    return true;
}

bool DeepFreezeCooldownTrigger::IsActive()
{
    constexpr uint32 SPELL_DEEP_FREEZE = 44572;
    return !bot->HasSpell(SPELL_DEEP_FREEZE) ||
           SpellCooldownTrigger::IsActive();
}

// By leewheel 2026-09-01
// 寒冰屏障冷却经济学（移植 NPCBots bot_mage_ai.cpp:830-852 needFactor 加权思想）：
//   场景A 输出重置：冰脉(12472)+深结(44572)都在 CD → 重置冰系爆发循环；
//   场景B 生存重置：PVP 战斗中血<40% 且冰霜新星(122)在 CD 且有敌方玩家 15 码内贴脸
//          → 重置冰霜新星控场，接闪现远遁（NPCBots 法师循环的 CD 支撑）。
// End By leewheel
bool ColdSnapTrigger::IsActive()
{
    // 场景A：双大招 CD（原 TwoTriggers 语义，经上下文查已注册的 on-cd 触发器）
    Trigger* icyVeinsCd = botAI->GetAiObjectContext()->GetTrigger("icy veins on cd");
    Trigger* deepFreezeCd = botAI->GetAiObjectContext()->GetTrigger("deep freeze on cd");
    if (icyVeinsCd && deepFreezeCd && icyVeinsCd->IsActive() && deepFreezeCd->IsActive())
        return true;

    // 场景B：PVP 生存窗口
    constexpr uint32 SPELL_FROST_NOVA = 122;   // 冰霜新星（DBC 验证）
    constexpr uint32 SPELL_ICE_BLOCK = 11426;  // 寒冰屏障本体（正在无敌时不重复开）
    if (bot->GetHealthPct() < 40.f && bot->HasSpellCooldown(SPELL_FROST_NOVA) &&
        !bot->HasAura(SPELL_ICE_BLOCK))
    {
        GuidVector enemies = AI_VALUE(GuidVector, "nearest enemy players");
        for (ObjectGuid const& guid : enemies)
        {
            Unit* enemy = botAI->GetUnit(guid);
            if (enemy && enemy->IsAlive() && bot->GetDistance(enemy) < 15.f)
                return true;
        }
    }

    return false;
}

const std::unordered_set<uint32> FlamestrikeNearbyTrigger::FLAMESTRIKE_SPELL_IDS = {
    2120, 2121, 8422, 8423, 10215, 10216, 27086, 42925, 42926
};

bool FlamestrikeNearbyTrigger::IsActive()
{
    for (uint32 spellId : FLAMESTRIKE_SPELL_IDS)
    {
        Aura* aura = bot->GetAura(spellId, bot->GetGUID());
        if (!aura)
            continue;

        DynamicObject* dynObj = aura->GetDynobjOwner();
        if (!dynObj)
            continue;

        float dist = bot->GetDistance2d(dynObj->GetPositionX(), dynObj->GetPositionY());
        if (dist <= radius)
            return true;
    }
    return false;
}

bool ImprovedScorchTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (!target || !target->IsAlive() || !target->IsInWorld())
        return false;

    static const uint32 ImprovedScorchExclusiveDebuffs[] = {// Shadow Mastery
                                                            17794, 17797, 17798, 17799, 17800,
                                                            // Winter's Chill
                                                            12579,
                                                            // Improved Scorch
                                                            22959};

    for (uint32 spellId : ImprovedScorchExclusiveDebuffs)
    {
        if (target->HasAura(spellId))
            return false;
    }

    return DebuffTrigger::IsActive();
}

const std::unordered_set<uint32> BlizzardChannelCheckTrigger::BLIZZARD_SPELL_IDS = {
    10,     // Blizzard Rank 1
    6141,   // Blizzard Rank 2
    8427,   // Blizzard Rank 3
    10185,  // Blizzard Rank 4
    10186,  // Blizzard Rank 5
    10187,  // Blizzard Rank 6
    27085,  // Blizzard Rank 7
    42938,  // Blizzard Rank 8
    42939   // Blizzard Rank 9
};

bool BlizzardChannelCheckTrigger::IsActive()
{
    if (Spell* spell = bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
        spell && BLIZZARD_SPELL_IDS.count(spell->m_spellInfo->Id))
    {
        uint8 attackerCount = AI_VALUE(uint8, "attacker count");
        return attackerCount < minEnemies;
    }

    return false;
}
