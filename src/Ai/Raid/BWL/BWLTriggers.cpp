/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BWLTriggers.h"
#include "BWLHelpers.h"
#include "Playerbots.h"

using namespace BlackwingLairHelpers;

// General

bool BwlSuppressionDeviceTrigger::IsActive()
{
    // Until MoP, only rogues could disarm suppression devices.
    // If raid cheats are enabled, any bot can disarm the devices.
    if (botAI->HasCheat(BotCheatMask::raid) || bot->IsClass(CLASS_ROGUE))
    {
        GuidVector gos = AI_VALUE(GuidVector, "nearest game objects");
        for (auto i = gos.begin(); i != gos.end(); ++i)
        {
            GameObject const* go = botAI->GetGameObject(*i);
            if (IsActiveSuppressionDeviceInRange(go, bot))
                return true;
        }
    }
    return false;
}

// Razorgore the Untamed

bool BwlRazorgoreNotMindControlledTrigger::IsActive()
{
    if (Unit* boss = AI_VALUE2(Unit*, "find target", "12435"))
        return !boss->HasAura(static_cast<uint32>(BlackwingLairSpells::SPELL_MINDCONTROL));
    return false;
}

// Vaelastrasz the Corrupt

bool BwlVaelastraszPositioningTrigger::IsActive()
{
    // Prevent non-tanks from rotating the boss while the tanks gain thread.
    if (Unit* boss = AI_VALUE2(Unit*, "find target", "13020"))
        return boss->GetVictim() != bot;
    return false;
}

bool BwlVaelastraszBurningAdrenalineTrigger::IsActive()
{
    // No check for Vaelastrasz, because bots may still have burning adrenaline even after Vaelastrasz died.
    return bot->HasAura(static_cast<uint32>(BlackwingLairSpells::SPELL_BURNING_ADRENALINE));
}

// Chromaggus

bool BwlAfflictionBronzeTrigger::IsActive()
{
    return bot->HasAura(static_cast<uint32>(BlackwingLairSpells::SPELL_BROOD_AFFLICTION_BRONZE));
}

// Nefarian

bool BwlWildMagicTrigger::IsActive()
{
    return bot->getClass() == CLASS_MAGE &&
        bot->HasAura(static_cast<uint32>(BlackwingLairSpells::SPELL_WILD_MAGIC));
}

bool BwlNefarianFearWardTrigger::IsActive()
{
    if (bot->getClass() != CLASS_PRIEST)
        return false;

    Unit* nefarian = AI_VALUE2(Unit*, "find target", "11583");
    if (!nefarian || !nefarian->IsInCombat())
        return false;

    Unit* victim = nefarian->GetVictim();
    if (!victim)
        return false;

    return !botAI->HasAura("fear ward", victim);
}

// Trash

bool BwlDeathTalonWyrmguardTankTrigger::IsActive()
{
    return PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "12460");
}

bool BwlDeathTalonWyrmguardRangedTrigger::IsActive()
{
    return PlayerbotAI::IsRanged(bot) && AI_VALUE2(Unit*, "find target", "12460");
}

//By leewheel 2026年7月12日
// 自定义Boss: Valthorax

bool BwlValthoraxFrostBombTrigger::IsActive()
{
    // 检测Boss是否进入50%血量冰霜炸弹阶段（Boss定身自身）
    Unit* boss = AI_VALUE2(Unit*, "find target", BlackwingLairHelpers::BOSS_NAME_VALTHORAX);
    if (!boss || !boss->IsAlive())
        return false;

    // Boss定身自身时表示正在引导冰霜炸弹
    return boss->HasAura(static_cast<uint32>(BlackwingLairHelpers::ValthoraxSpells::SPELL_VALTHORAX_SELF_ROOT));
}

bool BwlValthoraxVabominationTrigger::IsActive()
{
    // 检测憎恶（Vabomination）是否存活且在附近
    // 憎盛会移动到Boss身边治疗Boss，必须优先击杀
    GuidVector const npcs = AI_VALUE(GuidVector, "possible targets no los");
    for (auto const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive() &&
            unit->GetEntry() == static_cast<uint32>(BlackwingLairHelpers::BlackwingLairNPCs::NPC_VABOMINATION))
            return true;
    }
    return false;
}
//End By leewheel

bool BwlValthoraxAddsTrigger::IsActive()
{
    // 检测Boss召唤的亡灵小怪是否存活
    Unit* boss = AI_VALUE2(Unit*, "find target", BlackwingLairHelpers::BOSS_NAME_VALTHORAX);
    if (!boss || !boss->IsAlive())
        return false;

    GuidVector const npcs = AI_VALUE(GuidVector, "possible targets no los");
    for (auto const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        uint32 entry = unit->GetEntry();
        if (entry == static_cast<uint32>(BlackwingLairHelpers::BlackwingLairNPCs::NPC_SKELETAL_WARRIOR) ||
            entry == static_cast<uint32>(BlackwingLairHelpers::BlackwingLairNPCs::NPC_GHOUL) ||
            entry == static_cast<uint32>(BlackwingLairHelpers::BlackwingLairNPCs::NPC_BANSHEE))
            return true;
    }
    return false;
}
//End By leewheel
