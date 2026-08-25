/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "MCTriggers.h"
#include "MCHelpers.h"
#include "SharedDefines.h"
#include "SpellAuras.h"

using namespace MoltenCoreHelpers;

bool McLivingBombDebuffTrigger::IsActive()
{
    // No check for Baron Geddon, because bots may have the bomb even after Geddon died.
    return bot->HasAura(SPELL_LIVING_BOMB);
}

bool McBaronGeddonInfernoTrigger::IsActive()
{
    if (Unit* boss = AI_VALUE2(Unit*, "find target", "12056"))
        return boss->HasAura(SPELL_INFERNO);
    return false;
}
//End By leewheel

bool McShazzrahRangedTrigger::IsActive()
{
    // Only fire inside Arcane Explosion range. The move-away action no-ops
    // beyond it, so an unconditional trigger makes every already-safe ranged
    // bot attempt a failing move each tick.
    //By leewheel 2026-08-26 合并：采用对侧距离门控改进，查找串按entry规则转回(Shazzrah=12264)，并纳入对侧新增的岩浆/Golemagg触发器实现
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "12264");
    return boss && bot->GetDistance2d(boss) < ARCANE_EXPLOSION_DISTANCE;
}

bool McInLavaTrigger::IsActive()
{
    LiquidData const& liquid = bot->GetLiquidData();
    return (liquid.Flags & MAP_LIQUID_TYPE_MAGMA) &&
           (liquid.Status & (LIQUID_MAP_WATER_WALK | LIQUID_MAP_IN_WATER | LIQUID_MAP_UNDER_WATER));
}

bool McGolemaggMagmaSplashTrigger::IsActive()
{
    if (PlayerbotAI::IsTank(bot))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "11988");
    if (!boss)
        return false;

    // Below 10% the fight is a burn race (Earthquake pulses, Core Ragers
    // converge): stack management stops mattering, melee return to dps.
    if (boss->GetHealthPct() <= 10.0f)
        return false;

    Aura* splash = bot->GetAura(SPELL_MAGMA_SPLASH);
    if (!splash || splash->GetStackAmount() < MAGMA_SPLASH_BACK_OFF_STACKS)
        return false;

    // Only fire while still inside swing range; once the bot has backed off,
    // the multiplier keeps it from re-engaging until the stack expires.
    return bot->GetDistance2d(boss) < MAGMA_SPLASH_BACK_OFF_DISTANCE;
}

bool McGolemaggMarkBossTrigger::IsActive()
{
    // any tank may mark the boss
    return AI_VALUE2(Unit*, "find target", "11988") && PlayerbotAI::IsTank(bot);
}

bool McGolemaggIsMainTankTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "11988") && PlayerbotAI::IsMainTank(bot);
}

bool McGolemaggIsAssistTankTrigger::IsActive()
{
    //By leewheel 2026-08-26 合并：保留entry规则，并纳入对侧新增的治疗职业Golemagg触发器
    return AI_VALUE2(Unit*, "find target", "11988") && PlayerbotAI::IsAssistTank(bot);
}

bool McGolemaggIsHealerTrigger::IsActive()
{
    return PlayerbotAI::IsHeal(bot) && AI_VALUE2(Unit*, "find target", "11988");
}

bool McCoreHoundMarkTrigger::IsActive()
{
    return PlayerbotAI::IsMainTank(bot) && AI_VALUE2(Unit*, "find target", "11671");
}

//By leewheel 2026年7月12日
// 自定义Boss: Smolder

bool McSmolderFlameTsunamiTrigger::IsActive()
{
    // 检测附近是否有火焰海啸NPC（83006-83016）
    Unit* boss = AI_VALUE2(Unit*, "find target", MoltenCoreHelpers::BOSS_NAME_SMOLDER);
    if (!boss || !boss->IsAlive())
        return false;

    GuidVector const npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        uint32 entry = unit->GetEntry();
        if (entry >= MoltenCoreHelpers::NPC_FLAME_TSUNAMI_FIRST &&
            entry <= MoltenCoreHelpers::NPC_FLAME_TSUNAMI_LAST)
        {
            // 火焰海啸NPC在15码内时触发
            if (bot->GetDistance2d(unit) < 15.0f)
                return true;
        }
    }
    return false;
}
//End By leewheel

bool McSmolderFearWardTrigger::IsActive()
{
    // Smolder使用AOE恐惧（咆哮），牧师应在坦克身上保持反恐结界
    if (bot->getClass() != CLASS_PRIEST)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", MoltenCoreHelpers::BOSS_NAME_SMOLDER);
    if (!boss || !boss->IsAlive())
        return false;

    Unit* victim = boss->GetVictim();
    if (!victim)
        return false;

    return !botAI->HasAura("fear ward", victim);
}

// 自定义Boss: Hazzrash

bool McHazzrashEvocationTrigger::IsActive()
{
    // Hazzrash在48s/108s/176s时施放Evocation，暂停攻击20秒
    // Boss施放Evocation时机器人可以趁机重新定位/治疗
    Unit* boss = AI_VALUE2(Unit*, "find target", MoltenCoreHelpers::BOSS_NAME_HAZZRASH);
    if (!boss || !boss->IsAlive())
        return false;

    return boss->HasAura(MoltenCoreHelpers::SPELL_HAZZRASH_EVOCATION);
}

bool McHazzrashRangedSpreadTrigger::IsActive()
{
    // Hazzrash使用连锁燃烧（Chain Burn），远程需要散开
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", MoltenCoreHelpers::BOSS_NAME_HAZZRASH);
    if (!boss || !boss->IsAlive())
        return false;

    // 检查是否有其他远程机器人太近
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member == bot)
                continue;

            if (!PlayerbotAI::IsRanged(member))
                continue;

            // 如果另一个远程机器人在10码内，需要散开
            if (bot->GetDistance2d(member) < 10.0f)
                return true;
        }
    }
    return false;
}
//End By leewheel
