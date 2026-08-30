/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ZAMultipliers.h"
#include "ChooseTargetActions.h"
#include "DKActions.h"
#include "DruidBearActions.h"
#include "EncounterHelpers.h"
#include "FollowActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "PaladinActions.h"
#include "Playerbots.h"
#include "PriestActions.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "ShamanActions.h"
#include "WarlockActions.h"
#include "WarriorActions.h"
#include "ZAActions.h"
#include "ZAHelpers.h"

using namespace ZaHelpers;
using namespace EncounterHelpers;

// 合并brighton 2026-08-26: 整个文件对齐brighton最新重构版(IsHazardousMovement统一危险行动判定等),
// 全部find target按项目entry规则转换: zul'jin->23863, hex lord malacrass->24239, jan'alai->23578,
// akil'zon->23574, nalorakk->23576, amani'shi hatcher->23818, amani dragonhawk hatchling->23598, halazzi->23577 --By leewheel 2026年8月26日

namespace
{

bool const IsHazardousMovement(Action* action)
{
    return (dynamic_cast<MovementAction*>(action) && !dynamic_cast<AttackAction*>(action)) ||
        dynamic_cast<CastReachTargetSpellAction*>(action) ||
        dynamic_cast<CastKillingSpreeAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action);
}

}

// General

float ZulAmanDelayDpsCooldownsMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->GetMapId() != ZA_MAP_ID) // In case strategy persists outside (e.g., server reset)
        return 1.0f;

    if (!IsDpsCooldownAction(bot, action))
        return 1.0f;

    // Every Zul'Aman boss, and nothing else in the instance, runs a BossAI.
    Unit* boss = AI_VALUE(Unit*, "boss target");
    if (!boss)
        return 1.0f;

    if (boss->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT)
        return 0.0f;

    // Further restrictions on Bloodlust for Zul'jin and Jan'alai below
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    if (!dynamic_cast<CastBloodlustAction*>(action) &&
        !dynamic_cast<CastHeroismAction*>(action))
    {
        return 1.0f;
    }

    // Zul'jin: hold until Phase 3 (or later)
    if (boss->GetEntry() == Id(ZaNpcs::NPC_ZULJIN))
    {
        return (boss->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_EAGLE)) ||
            boss->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_LYNX)) ||
            boss->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_DRAGONHAWK))) ? 1.0f : 0.0f;
    }

    // Jan'alai: hold until time to burn Hatchlings (see comments to the constants in ZAHelpers.h)
    if (boss->GetEntry() == Id(ZaNpcs::NPC_JANALAI))
    {
        if (boss->GetHealthPct() <= JANALAI_HATCH_ALL_HEALTH_PCT)
            return 1.0f;

        return CountAttackersByEntry(botAI, Id(ZaNpcs::NPC_AMANI_DRAGONHAWK_HATCHLING)) >=
            JANALAI_BLOODLUST_HATCHLING_COUNT ? 1.0f : 0.0f;
    }

    return 1.0f;
}

float ZulAmanAvoidWhirlwindMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<ReachTargetAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action) &&
        !dynamic_cast<CastKillingSpreeAction*>(action))
    {
        return 1.0f;
    }

    if (Unit* zuljin = AI_VALUE2(Unit*, "find target", "23863"))
    {
        if (zuljin->GetVictim() == bot)
            return 1.0f;

        if (!zuljin->HasAura(Id(ZaSpells::SPELL_ZULJIN_WHIRLWIND)))
            return 1.0f;

        return bot->GetDistance2d(zuljin) <= ZA_WHIRLWIND_SAFE_DISTANCE ? 0.0f : 1.0f;
    }

    if (Unit* malacrass = AI_VALUE2(Unit*, "find target", "24239"))
    {
        if (malacrass->GetVictim() == bot)
            return 1.0f;

        if (!malacrass->HasAura(Id(ZaSpells::SPELL_HEX_LORD_WHIRLWIND)))
            return 1.0f;

        return bot->GetDistance2d(malacrass) <= ZA_WHIRLWIND_SAFE_DISTANCE ? 0.0f : 1.0f;
    }

    return 1.0f;
}

float ZulAmanDisableCombatFormationMoveMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<CombatFormationMoveAction*>(action))
        return 1.0f;

    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "23578") ||
        AI_VALUE2(Unit*, "find target", "23574") ? 0.0f : 1.0f;
}

// Akil'zon <Eagle Avatar>

float AkilzonStayInEyeOfTheStormMultiplier::GetValue(Action* action)
{
    if (!IsHazardousMovement(action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "23574"))
        return 1.0f;

    if (dynamic_cast<AkilzonMoveToEyeOfTheStormAction*>(action))
        return 1.0f;

    auto it = akilzonStormTimer.find(bot->GetInstanceId());
    if (it == akilzonStormTimer.end())
        return 1.0f;

    return IsInStormWindow(it->second) ? 0.0f : 1.0f;
}

// Nalorakk <Bear Avatar>

float NalorakkDisableTankActionsMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    bool const isTankAction = dynamic_cast<TankAssistAction*>(action) ||
        dynamic_cast<TankFaceAction*>(action);

    if (!isTankAction && !IsTauntAction(bot, action))
        return 1.0f;

    Unit* nalorakk = AI_VALUE2(Unit*, "find target", "23576");
    if (!nalorakk)
        return 1.0f;

    if (isTankAction)
        return 0.0f;

    // IsTauntAction
    bool const isInBearForm = IsNalorakkInBearForm(nalorakk);

    if (!isInBearForm && PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
        return 0.0f;

    return isInBearForm && PlayerbotAI::IsMainTank(bot) ? 0.0f : 1.0f;
}

float NalorakkControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "23576") ? 0.0f : 1.0f;
}

// Jan'alai <Dragonhawk Avatar>

float JanalaiDisableTankActionsMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    bool const isTankFaceAction = dynamic_cast<TankFaceAction*>(action);
    if (!isTankFaceAction && !dynamic_cast<TankAssistAction*>(action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "23578"))
        return 1.0f;

    if (isTankFaceAction)
        return 0.0f;

    // TankAssistAction
    return PlayerbotAI::IsMainTank(bot) ? 0.0f : 1.0f;
}

float JanalaiStayAwayFromFireBombsMultiplier::GetValue(Action* action)
{
    if (!IsHazardousMovement(action))
        return 1.0f;

    // By leewheel 2026-08-30 合并上游：改用IsJanalaiBombing判定；entry规则查怪(23578=jan'alai)
    Unit* janalai = AI_VALUE2(Unit*, "find target", "23578");
    if (!janalai)
        return 1.0f;
    // End By leewheel

    if (dynamic_cast<JanalaiAvoidFireBombsAction*>(action))
        return 1.0f;

    return IsJanalaiBombing(janalai) ? 0.0f : 1.0f;
}

float JanalaiDoNotCrowdControlHatchersMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<CastCrowdControlSpellAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "23818") ? 0.0f : 1.0f;
}

// By leewheel 2026-08-30 合并上游：JanalaiDelayBloodlustAndHeroismMultiplier已被上游删除(策略改用
//   IsJanalaiBombing体系)，同步移除本服旧实现，声明与挂载点已随上游自动合并清除

// Halazzi <Lynx Avatar>

float HalazziDisableTankActionsMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!dynamic_cast<TankAssistAction*>(action) && !dynamic_cast<TankFaceAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "23577") ? 0.0f : 1.0f;
}

float HalazziControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "23577") ? 0.0f : 1.0f;
}

float HalazziDisableAutoDpsTargetingMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsDps(bot))
        return 1.0f;

    if (!dynamic_cast<DpsAssistAction*>(action) &&
        !dynamic_cast<CastDebuffSpellOnAttackerAction*>(action))
    {
        return 1.0f;
    }

    return AI_VALUE2(Unit*, "find target", "23577") ? 0.0f : 1.0f;
}


// Hex Lord Malacrass

// Unstable Affliction is considered a magic effect, not a curse.
float HexLordMalacrassUnstableAfflictionMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() == CLASS_PRIEST)
    {
        if (!dynamic_cast<CastDispelMagicOnPartyAction*>(action) &&
            !dynamic_cast<CastDispelMagicAction*>(action) &&
            !dynamic_cast<CastMassDispelAction*>(action))
        {
            return 1.0f;
        }
    }
    else if (bot->getClass() == CLASS_PALADIN)
    {
        if (!dynamic_cast<CastCleanseMagicOnPartyAction*>(action) &&
            !dynamic_cast<CastCleanseMagicAction*>(action))
        {
            return 1.0f;
        }
    }
    else
    {
        return 1.0f;
    }

    if (!AI_VALUE2(Unit*, "find target", "24239"))
        return 1.0f;

    Unit* target = AI_VALUE2(Unit*, "party member to dispel", DISPEL_MAGIC);
    return target && target->HasAura(Id(ZaSpells::SPELL_UNSTABLE_AFFLICTION)) ? 0.0f : 1.0f;
}

float HexLordMalacrassSpellReflectionMultiplier::GetValue(Action* action)
{
    if (!PlayerbotAI::IsCaster(bot))
        return 1.0f;

    if (dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    if (!dynamic_cast<CastSpellAction*>(action))
        return 1.0f;

    // By leewheel 2026-08-30 合并上游单行写法；entry规则查怪(24239=hex lord malacrass)
    Unit* malacrass = AI_VALUE2(Unit*, "find target", "24239");
    return malacrass &&
        malacrass->HasAura(Id(ZaSpells::SPELL_HEX_LORD_SPELL_REFLECTION)) ? 0.0f : 1.0f;
}

// Zul'jin

float ZuljinDisableTankFaceMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!dynamic_cast<TankFaceAction*>(action))
        return 1.0f;

    // By leewheel 2026-08-30 合并上游单行写法；entry规则查怪(23863=zul'jin)
    Unit* zuljin = AI_VALUE2(Unit*, "find target", "23863");
    return zuljin && zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_DRAGONHAWK)) ? 1.0f : 0.0f;
}

float ZuljinEagleDisableAvoidAoeMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<AvoidAoeAction*>(action))
        return 1.0f;

    // By leewheel 2026-08-30 合并上游：Eagle判定改单行写法；ZuljinDelayBloodlustAndHeroismMultiplier
    //   已被上游删除(声明与挂载点已随自动合并清除)，同步移除本服旧实现；entry规则查怪(23863=zul'jin)
    Unit* zuljin = AI_VALUE2(Unit*, "find target", "23863");
    return zuljin && zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_EAGLE)) ? 0.0f : 1.0f;
}
// End By leewheel
