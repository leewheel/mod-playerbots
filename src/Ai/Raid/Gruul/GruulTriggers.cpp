/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GruulTriggers.h"
#include "GruulHelpers.h"
#include "InstanceScript.h"
#include "Playerbots.h"

using namespace GruulHelpers;

// General

bool GruulsLairNoEncounterInProgress::IsActive()
{
    if (bot->GetMapId() != GRUUL_MAP_ID)
        return false;

    InstanceScript* instance = bot->GetInstanceScript();
    return instance && !instance->IsEncounterInProgress();
}

// High King Maulgar

bool HighKingMaulgarThreeOgresNeedMeleeTanksTrigger::IsActive()
{
    if (IsBlindeyeTank(bot))
        return AI_VALUE2(Unit*, "find target", "18831");

    // By leewheel 2026-08-29 合并：采用对侧完整岗位判断(BlindEye/Olm/Maulgar三坦克各自对应目标)，entry规则查找
    if (IsOlmTank(bot))
        return AI_VALUE2(Unit*, "find target", "18834");

    return IsMaulgarTank(bot) && AI_VALUE2(Unit*, "find target", "18836");
    // End By leewheel
}

bool HighKingMaulgarKroshNeedsMageTankTrigger::IsActive()
{
    // By leewheel 2026-08-29 合并：采用对侧IsKroshMageTank助手(职责单一)，entry规则查找
    return IsKroshMageTank(bot) && AI_VALUE2(Unit*, "find target", "18832");
    // End By leewheel
}

bool HighKingMaulgarKigglerNeedsMoonkinTankTrigger::IsActive()
{
    // By leewheel 2026-08-29 合并：采用对侧IsKigglerMoonkinTank助手，entry规则查找
    return IsKigglerMoonkinTank(bot) && AI_VALUE2(Unit*, "find target", "18835");
    // End By leewheel
}

bool HighKingMaulgarDeterminingKillOrderTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "18831"))
        return false;

    if (IsMaulgarTank(bot))
        return false;

    if (IsOlmTank(bot))
        return !AI_VALUE2(Unit*, "find target", "18834");

    if (IsBlindeyeTank(bot))
        return !AI_VALUE2(Unit*, "find target", "18836");

    // By leewheel 2026-08-29 合并：采用对侧IsKroshMageTank/IsKigglerMoonkinTank助手形态，entry规则查找
    if (IsKroshMageTank(bot))
        return !AI_VALUE2(Unit*, "find target", "18832");

    if (IsKigglerMoonkinTank(bot))
        return !AI_VALUE2(Unit*, "find target", "18835");
    // End By leewheel

    return true;
}

bool HighKingMaulgarBossChannelingWhirlwindTrigger::IsActive()
{
    // By leewheel 2026-08-29 修复：旋风斩是 High King Maulgar(18836) 的技能，
    // 旧代码误用 18831(Blindeye the Seer) 导致该触发器永远无法在正确的boss身上生效
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "18836");
    // End By leewheel
    if (!maulgar || !maulgar->HasAura(Id(GruulSpells::SPELL_WHIRLWIND)))
        return false;

    return !IsMaulgarTank(bot);
}

bool HighKingMaulgarKroshCastsBlastWaveTrigger::IsActive()
{
    // By leewheel 2026-08-29 合并：坦克与Krosh法师坦克放行(对侧新增)，entry规则查找
    if (PlayerbotAI::IsTank(bot) || IsKroshMageTank(bot))
        return false;

    return AI_VALUE2(Unit*, "find target", "18832");
    // End By leewheel
}

bool HighKingMaulgarWildFelStalkerSpawnedTrigger::IsActive()
{
    return bot->getClass() == CLASS_WARLOCK && AI_VALUE2(Unit*, "find target", "18847");
}

bool HighKingMaulgarPullingOgreCouncilTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    // By leewheel 2026-08-29 合并：修正entry错配(拉怪监控对象是Blindeye 18831而非Maulgar 18836)；
    //   2026-08-30 合并上游：HP常量统一为 BLINDEYE_ENGAGED_HEALTH_PCT
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "18831");
    return blindeye && blindeye->GetHealthPct() > BLINDEYE_ENGAGED_HEALTH_PCT;
}

bool HighKingMaulgarBossCastsIntimidatingRoarTrigger::IsActive()
{
    return bot->getClass() == CLASS_PRIEST && AI_VALUE2(Unit*, "find target", "18836");
    // End By leewheel
}

// Gruul the Dragonkiller

bool GruulTheDragonkillerShouldBeTankedTrigger::IsActive()
{
    return PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "19044");
}

bool GruulTheDragonkillerRangedShouldSpreadTrigger::IsActive()
{
    return PlayerbotAI::IsRanged(bot) && AI_VALUE2(Unit*, "find target", "19044");
}

bool GruulTheDragonkillerIncomingShatterTrigger::IsActive()
{
    return HasGroundSlam(bot);
}
