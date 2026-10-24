/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "FoodActions.h"
#include "Event.h"
#include "Playerbots.h"
#include <algorithm>

namespace
{
constexpr uint32 BG_WS_SPELL_WARSONG_FLAG = 23333;
constexpr uint32 BG_WS_SPELL_SILVERWING_FLAG = 23335;
constexpr uint32 BG_EY_NETHERSTORM_FLAG_SPELL = 34976;

// Graccu's Mince Meat Fruitcake: 5% health and mana per second for 20s. Mana regen is continuous,
// but health regen is applied in 2s lumps of 10%, timed by the server's update loop, not the aura.
constexpr uint32 SPELL_GRACCUS_MINCE_MEAT_FRUITCAKE = 25990;
constexpr uint32 RECOVERY_MAX_MS = 20 * IN_MILLISECONDS;
constexpr uint32 REGEN_TICK_MS = 2 * IN_MILLISECONDS;

bool IsFullMana(Player* bot)
{
    return bot->GetPower(POWER_MANA) >= bot->GetMaxPower(POWER_MANA);
}

// Rounded up since partial ticks provide no regen value.
uint32 RecoveryTicks(uint32 current, uint32 max)
{
    if (!max || current >= max)
        return 0;

    uint32 const missing = max - current;
    uint32 const ticksPerPool = RECOVERY_MAX_MS / REGEN_TICK_MS;
    return (missing * ticksPerPool + max - 1) / max;
}

// Larger gap of the two pools, plus a tick because the next health regen tick can be up to 2s away.
// Capped at the aura's max duration, so the bot never sits longer than a player could.
uint32 FoodCheatRecoveryDurationMs(Player* bot)
{
    //By leewheel 2026-10-25 合并 brighton-chi the-lab（4e5fe67b 一带）：上游把本函数改成
    //  RecoveryTicks() 助手 + "+1 tick" 封顶写法。我方上一轮在这里补的「maxHealth == 0 守卫」
    //  （GetMaxHealth() 为 0 时 missingShare 变 -inf，末尾 static_cast<uint32>(RECOVERY_MAX_MS * -inf)
    //  是 NaN→整型转换的 UB）**已被上游同一个防护覆盖** —— 看上方 RecoveryTicks() 开头：
    //      if (!max || current >= max) return 0;
    //  生命与法力两路都走它 ⇒ 判零口径一致且不重复。故取上游写法，我方不再另立一套。
    //End By leewheel
    uint32 ticks = RecoveryTicks(bot->GetHealth(), bot->GetMaxHealth());

    if (uint32 const maxMana = bot->GetMaxPower(POWER_MANA))
        ticks = std::max(ticks, RecoveryTicks(bot->GetPower(POWER_MANA), maxMana));

    return std::min((ticks + 1) * REGEN_TICK_MS, RECOVERY_MAX_MS);
}

uint32 StartFoodCheatRecovery(Player* bot)
{
    bot->ClearUnitState(UNIT_STATE_CHASE);
    bot->ClearUnitState(UNIT_STATE_FOLLOW);

    if (bot->isMoving())
        bot->StopMoving();

    bot->SetStandState(UNIT_STAND_STATE_SIT);
    bot->CastStop();

    uint32 const durationMs = FoodCheatRecoveryDurationMs(bot);
    bot->AddAura(SPELL_GRACCUS_MINCE_MEAT_FRUITCAKE, bot);
    return durationMs;
}

bool IsDisallowedShapeshiftForm(Player* bot)
{
    if (bot->getClass() == CLASS_DRUID)
    {
        ShapeshiftForm form = bot->GetShapeshiftForm();
        return form == FORM_TRAVEL || form == FORM_AQUA || form == FORM_CAT || form == FORM_BEAR ||
            form == FORM_DIREBEAR || form == FORM_FLIGHT || form == FORM_FLIGHT_EPIC;
    }

    return bot->getClass() == CLASS_PRIEST && bot->GetShapeshiftForm() == FORM_SPIRITOFREDEMPTION;
}
} // end anonymous namespace

bool DrinkAction::Execute(Event event)
{
    if (botAI->HasCheat(BotCheatMask::food))
    {
        SetDuration(StartFoodCheatRecovery(bot));
        return true;
    }

    return UseItemAction::Execute(event);
}

bool DrinkAction::isUseful()
{
    if (!bot->GetMaxPower(POWER_MANA)) // no mana pool to refill
        return false;

    return !IsFullMana(bot) || (botAI->HasCheat(BotCheatMask::food) && !bot->IsFullHealth());
}

bool DrinkAction::isPossible()
{
    if (bot->IsInCombat() || bot->IsMounted() || IsDisallowedShapeshiftForm(bot))
        return false;

    if (bot->HasAura(BG_WS_SPELL_WARSONG_FLAG) || bot->HasAura(BG_WS_SPELL_SILVERWING_FLAG) ||
        bot->HasAura(BG_EY_NETHERSTORM_FLAG_SPELL))
    {
        return false;
    }

    return botAI->HasCheat(BotCheatMask::food) || UseItemAction::isPossible();
}

bool EatAction::Execute(Event event)
{
    if (botAI->HasCheat(BotCheatMask::food))
    {
        SetDuration(StartFoodCheatRecovery(bot));
        return true;
    }

    return UseItemAction::Execute(event);
}

bool EatAction::isUseful()
{
    return !bot->IsFullHealth() || (botAI->HasCheat(BotCheatMask::food) && !IsFullMana(bot));
}

bool EatAction::isPossible()
{
    if (bot->IsInCombat() || bot->IsMounted() || IsDisallowedShapeshiftForm(bot))
        return false;

    if (bot->HasAura(BG_WS_SPELL_WARSONG_FLAG) || bot->HasAura(BG_WS_SPELL_SILVERWING_FLAG) ||
        bot->HasAura(BG_EY_NETHERSTORM_FLAG_SPELL))
    {
        return false;
    }

    return botAI->HasCheat(BotCheatMask::food) || UseItemAction::isPossible();
}
