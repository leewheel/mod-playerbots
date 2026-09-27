/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SSCMultipliers.h"
#include "ChooseTargetActions.h"
#include "DKActions.h"
#include "DruidActions.h"
#include "DruidBearActions.h"
#include "DruidCatActions.h"
#include "DruidShapeshiftActions.h"
#include "EncounterHelpers.h"
#include "FollowActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "LootAction.h"
#include "MageActions.h"
#include "MoveSpline.h"
#include "NonCombatActions.h"
#include "PaladinActions.h"
#include "Playerbots.h"
#include "PriestActions.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "SSCActions.h"
#include "SSCHelpers.h"
#include "ShamanActions.h"
#include "Timer.h"
#include "WarlockActions.h"
#include "WarriorActions.h"
#include "WipeAction.h"
#include <algorithm>

using namespace SscHelpers;
using namespace EncounterHelpers;

namespace
{

bool IsRepositionAction(Player* bot, Action* action)
{
    return (bot->getClass() == CLASS_HUNTER && dynamic_cast<CastDisengageAction*>(action)) ||
        (bot->getClass() == CLASS_MAGE && dynamic_cast<CastBlinkBackAction*>(action));
}

bool IsBloodlustAction(Player* bot, Action* action)
{
    return bot->getClass() == CLASS_SHAMAN &&
        (dynamic_cast<CastBloodlustAction*>(action) || dynamic_cast<CastHeroismAction*>(action));
}

// The taunts that take more than the tank's own target
bool IsAoeTauntAction(Player* bot, Action* action)
{
    switch (bot->getClass())
    {
        case CLASS_DRUID:
            return dynamic_cast<CastChallengingRoarAction*>(action);
        case CLASS_PALADIN:
            return dynamic_cast<CastRighteousDefenseAction*>(action);
        case CLASS_WARRIOR:
            return dynamic_cast<CastChallengingShoutAction*>(action);
        default:
            return false;
    }
}

} // end anonymous namespace

// Trash

float UnderbogColossusEscapeToxicPoolMultiplier::GetValue(Action* action)
{
    if (bot->GetMapId() != SSC_MAP_ID)
        return 1.0f;

    // Don't sit and drink in a toxic pool. Come on...
    if (dynamic_cast<DrinkAction*>(action) || dynamic_cast<EatAction*>(action))
        return IsNearToxicPool(botAI, TOXIC_POOL_HOLDING_RADIUS) ? 0.0f : 1.0f;

    if (!dynamic_cast<MovementAction*>(action))
        return 1.0f;

    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<UnderbogColossusEscapeToxicPoolAction*>(action))
        return 1.0f;

    return IsNearToxicPool(botAI, TOXIC_POOL_HOLDING_RADIUS) ? 0.0f : 1.0f;
}

// Shared Bosses

float SscControlMisdirectionMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 1.0f;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "21212");
    if (vashj && GetLadyVashjPhase(vashj) != 1)
        return 0.0f;

    // By leewheel 2026-09-19 / 2026-09-22 合并上游 the-lab：采纳上游"先取莱欧瑟拉斯、再统一判定"的结构
    //   （块内需要用 leotheras 变量做 IsLeotherasChannelingWhirlwind 判定），并按规则第 97 条 entry 化：
    //   leotheras the blind=21215(盲眼者莱欧瑟拉斯)。
    if (Unit* leotheras = AI_VALUE2(Unit*, "find target", "21215"))
    {
        if (HasInnerDemon(bot))
            return 0.0f;

        if (IsLeotherasChannelingWhirlwind(leotheras))
            return 0.0f;

        if (GetActiveLeotherasDemon(botAI) && GetLeotherasWarlockTank(bot))
            return 0.0f;
    }

    return AI_VALUE2(Unit*, "find target", "21214") ||
        AI_VALUE2(Unit*, "find target", "21216") ? 0.0f : 1.0f;
    // End By leewheel
}

float SscDelayDpsCooldownsMultiplier::GetValue(Action* action)
{
    if (bot->GetMapId() != SSC_MAP_ID || botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!IsDpsCooldownAction(bot, action))
        return 1.0f;

    bool const isBloodlust = IsBloodlustAction(bot, action);

    // By leewheel 2026-09-19 合并上游 the-lab：本函数为上游新增，按规则第 97 条把首领名改为 entry：
    //   lady vashj=21212(瓦丝琪) / morogrim tidewalker=21213(莫洛格里·踏潮者)
    //   tidewalker lurker=21920(踏潮潜伏者) / fathom-lord karathress=21214(深水领主卡拉瑟雷斯)
    //   fathom-guard tidalvess=21965(深水卫士泰达维斯) / hydross the unstable=21216(不稳定的海度斯)
    //   the lurker below=21217(鱼斯拉) / leotheras the blind=21215(盲眼者莱欧瑟拉斯)。
    if (Unit* vashj = AI_VALUE2(Unit*, "find target", "21212"))
    {
        int8 const phase = GetLadyVashjPhase(vashj);
        if (phase == 3)
            return 1.0f;

        // Bloodlust/Heroism are phase 3 only; other dps cooldowns can be used from phase 2.
        return !isBloodlust && phase == 2 ? 1.0f : 0.0f;
    }

    if (Unit* tidewalker = AI_VALUE2(Unit*, "find target", "21213"))
    {
        // Bloodlust/Heroism are saved for the last phase, once the raid is stacked in the corner.
        if (isBloodlust)
            // By leewheel 2026-09-24 合并 brighton 999582f7：嗜血判定由「潜伏者 21920 存在」
            //   改为「踏潮者血量进入 P2 阈值」，采纳上游新设计（不再依赖 21920 存在性）。
            // End By leewheel
            return tidewalker->GetHealthPct() <= TIDEWALKER_PHASE_2_HEALTH_PCT ? 1.0f : 0.0f;

        return tidewalker->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT ? 0.0f : 1.0f;
    }

    if (AI_VALUE2(Unit*, "find target", "21214"))
    {
// Tidalvess is the first kill target; once he is down the rest of the fight is open.
        // By leewheel 2026-09-22 按规则第 97 条 entry 化：fathom-guard tidalvess = 21965。
        Unit* tidalvess = AI_VALUE2(Unit*, "find target", "21965");
        return tidalvess && tidalvess->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT ? 0.0f : 1.0f;
    }

    // By leewheel 2026-09-22 保留我方 entry 列表（上游为 the lurker below / hydross the unstable 两名，
    //   我方多列 21215(莱欧瑟拉斯) —— 与下方 GetLeotheras(bot) 分支等价、不改变行为，只是提前一层判定）。
    for (char const* name : { "21216", "21217", "21215" })
    {
        if (Unit* boss = AI_VALUE2(Unit*, "find target", name))
            return boss->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT ? 0.0f : 1.0f;
    }
    // End By leewheel

    if (Unit* leotheras = GetLeotheras(botAI))
        return leotheras->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT ? 0.0f : 1.0f;

    return 1.0f;
}

// Hydross the Unstable <Duke of Currents>

float HydrossTheUnstableDisableOffPhaseTankActionsMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!dynamic_cast<ReachTargetAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action) && !IsTauntAction(bot, action))
    {
        return 1.0f;
    }

    Unit* hydross = AI_VALUE2(Unit*, "find target", "21216");
    if (!hydross)
        return 1.0f;

    if (IsHydrossInFrostPhase(hydross) && PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
        return 0.0f;

    return IsHydrossInNaturePhase(hydross) && PlayerbotAI::IsMainTank(bot) ? 0.0f : 1.0f;
}

float HydrossTheUnstableDisablePhaseTankAssistMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<DpsAssistAction*>(action) && !dynamic_cast<TankAssistAction*>(action))
        return 1.0f;

    if (!IsHydrossPhaseTank(bot))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "21216") ? 0.0f : 1.0f;
}

// Phase changes reset threat. Hold DPS from 1s after Marks hit 100% until 5s post-phase change.
float HydrossTheUnstableWaitForDpsMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    if (dynamic_cast<HydrossTheUnstablePositionAndSwapTanksAction*>(action))
        return 1.0f;

    if (IsHydrossAddTank(bot))
        return 1.0f;

    Unit* hydross = AI_VALUE2(Unit*, "find target", "21216");
    if (!hydross)
        return 1.0f;

    bool const frostPhase = IsHydrossInFrostPhase(hydross);
    if (PlayerbotAI::IsTank(bot) &&
        (frostPhase ?
            PlayerbotAI::IsMainTank(bot) : PlayerbotAI::IsAssistTankOfIndex(bot, 0, true)))
    {
        return 1.0f;
    }

    std::unordered_map<uint32, uint32> const& phaseStartTimer =
        frostPhase ? hydrossFrostDpsWaitTimer : hydrossNatureDpsWaitTimer;
    std::unordered_map<uint32, uint32> const& handOverTimer =
        frostPhase ? hydrossChangeToNaturePhaseTimer : hydrossChangeToFrostPhaseTimer;

    uint32 const instanceId = hydross->GetInstanceId();
    uint32 const now = getMSTime();
    constexpr uint32 handOverWaitMs = 1 * IN_MILLISECONDS;
    constexpr uint32 phaseStartWaitMs = 5 * IN_MILLISECONDS;

    auto itStart = phaseStartTimer.find(instanceId);
    bool const justChanged =
        itStart == phaseStartTimer.end() || getMSTimeDiff(itStart->second, now) < phaseStartWaitMs;

    auto itHandOver = handOverTimer.find(instanceId);
    bool const aboutToChange =
        itHandOver != handOverTimer.end() &&
        getMSTimeDiff(itHandOver->second, now) >= handOverWaitMs;

    return justChanged || aboutToChange ? 0.0f : 1.0f;
}

// The Lurker Below

float TheLurkerBelowStayAwayFromSpoutMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action) &&
        !IsRepositionAction(bot, action) &&
        (bot->getClass() != CLASS_ROGUE || !dynamic_cast<CastKillingSpreeAction*>(action)))
    {
        return 1.0f;
    }

    if (dynamic_cast<TheLurkerBelowRunAroundBehindBossAction*>(action))
        return 1.0f;

    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    return IsLurkerSpouting(AI_VALUE2(Unit*, "find target", "21217")) ? 0.0f : 1.0f;
}

float TheLurkerBelowMaintainRangedSpreadMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsRanged(bot))
        return 1.0f;

    if (!dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<FleeAction*>(action) && !IsRepositionAction(bot, action))
    {
        return 1.0f;
    }

    return AI_VALUE2(Unit*, "find target", "21217") ? 0.0f : 1.0f;
}

float TheLurkerBelowTanksFocusAssignedGuardianMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!dynamic_cast<TankAssistAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action) &&
        !IsTauntAction(bot, action) && !IsAoeThreatAction(bot, action))
    {
        return 1.0f;
    }

    auto const instanceIt = lurkerGuardianTankAssignments.find(bot->GetInstanceId());
    if (instanceIt == lurkerGuardianTankAssignments.end())
        return 1.0f;

    std::vector<Player*> const tanks = GetLurkerGuardianTanks(bot);
    auto const myIt = std::find(tanks.begin(), tanks.end(), bot);
    if (myIt == tanks.end())
        return 1.0f;

    Unit* guardian = botAI->GetUnit(instanceIt->second[std::distance(tanks.begin(), myIt)]);
    return guardian && guardian->IsAlive() ? 0.0f : 1.0f;
}

// Leotheras the Blind

float LeotherasTheBlindAvoidWhirlwindMultiplier::GetValueInEncounter(Action* action)
{
    if (PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<LeotherasTheBlindRunAwayFromWhirlwindAction*>(action))
        return 1.0f;

    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (HasInnerDemon(bot))
        return 1.0f;

    Unit* leotheras = AI_VALUE2(Unit*, "find target", "21215");
    return leotheras && IsLeotherasChannelingWhirlwind(leotheras) ? 0.0f : 1.0f;
}

float LeotherasTheBlindDisableTankActionsMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsTank(bot) || HasInnerDemon(bot))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "21215"))
        return 1.0f;

    // Auto-attack only while the Warlock has him: abilities would spend the rage being banked for
    // an Inner Demon, and the target choosers would take the tanks off him
    if (GetPhase2LeotherasDemon(botAI) &&
        (dynamic_cast<TankAssistAction*>(action) ||
         (dynamic_cast<CastSpellAction*>(action) &&
          !dynamic_cast<CastDireBearFormAction*>(action) &&
          !dynamic_cast<CastBearFormAction*>(action))))
    {
        return 0.0f;
    }

    if (bot->getClass() == CLASS_WARRIOR && dynamic_cast<CastVigilanceAction*>(action) &&
        GetActiveLeotherasDemon(botAI))
    {
        Player* warlockTank = GetLeotherasWarlockTank(bot);
        if (warlockTank && action->GetTarget() == warlockTank)
            return 0.0f;
    }

    // Keep Berserk until Phase 3 in case the bear gets Inner Demon.
    if (bot->getClass() == CLASS_DRUID && dynamic_cast<CastBerserkAction*>(action) &&
        !GetPhase3LeotherasDemon(botAI))
    {
        return 0.0f;
    }

    return 1.0f;
}

float LeotherasTheBlindFocusOnInnerDemonMultiplier::GetValueInEncounter(Action* action)
{
    if (!HasInnerDemon(bot))
        return 1.0f;

    if (action->getThreatType() == Action::ActionThreatType::Aoe)
        return 0.0f;

    // Don't waste time moving. Just kill the Inner Demon asap.
    if (dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<LeotherasTheBlindDestroyInnerDemonAction*>(action) &&
        !dynamic_cast<LeotherasTheBlindMeleeRunAwayFromChaosBlastAction*>(action) &&
        !dynamic_cast<LeotherasTheBlindPositionRangedAction*>(action) &&
        !dynamic_cast<MeleeAction*>(action))
    {
        return 0.0f;
    }

    if (IsRepositionAction(bot, action))
        return 0.0f;

    // Per class: spells that prevent attacking or drop threat, AoEs with no threat type, useless
    // spells, and spells that need to be blocked to facilitate the custom Inner Demon action.
    switch (bot->getClass())
    {
        case CLASS_DRUID:
            if (dynamic_cast<CastTreeFormAction*>(action))
                return 0.0f;
            break;

        case CLASS_HUNTER:
            if (dynamic_cast<CastDeterrenceAction*>(action) ||
                dynamic_cast<CastFeignDeathAction*>(action) ||
                dynamic_cast<CastWingClipAction*>(action) ||
                dynamic_cast<CastFreezingTrap*>(action) ||
                dynamic_cast<CastExplosiveTrapAction*>(action) ||
                dynamic_cast<CastImmolationTrapAction*>(action) ||
                dynamic_cast<CastAspectOfTheHawkAction*>(action) ||
                dynamic_cast<CastAspectOfTheWildAction*>(action) ||
                dynamic_cast<CastAspectOfTheDragonhawkAction*>(action) ||
                dynamic_cast<CastAspectOfThePackAction*>(action) ||
                dynamic_cast<CastAspectOfTheCheetahAction*>(action) ||
                dynamic_cast<CastAspectOfTheMonkeyAction*>(action))
            {
                return 0.0f;
            }
            break;

        case CLASS_MAGE:
            if (dynamic_cast<CastIceBlockAction*>(action) ||
                dynamic_cast<CastInvisibilityAction*>(action))
            {
                return 0.0f;
            }
            break;

        case CLASS_PALADIN:
            if (dynamic_cast<CastDivineShieldAction*>(action))
                return 0.0f;
            break;

        case CLASS_PRIEST:
            if (dynamic_cast<CastFadeAction*>(action))
                return 0.0f;
            break;

        case CLASS_ROGUE:
            if (dynamic_cast<CastFeintAction*>(action) || dynamic_cast<CastVanishAction*>(action))
                return 0.0f;
            break;

        case CLASS_WARLOCK:
            if (dynamic_cast<CastCurseOfDoomAction*>(action))
                return 0.0f;
            break;

        case CLASS_WARRIOR:
            if (dynamic_cast<CastThunderClapAction*>(action) ||
                dynamic_cast<CastCleaveAction*>(action) ||
                dynamic_cast<CastChallengingShoutAction*>(action) ||
                dynamic_cast<CastDemoralizingShoutAction*>(action) ||
                dynamic_cast<CastDemoralizingShoutWithoutLifeTimeCheckAction*>(action) ||
                dynamic_cast<CastShockwaveAction*>(action) ||
                dynamic_cast<CastPiercingHowlAction*>(action) ||
                dynamic_cast<CastIntimidatingShoutAction*>(action) ||
                dynamic_cast<CastSweepingStrikesAction*>(action) ||
                dynamic_cast<CastVigilanceAction*>(action))
            {
                return 0.0f;
            }
            break;

        default:
            break;
    }

    // Exclude abilities with a target that isn't the bot or the Inner Demon, plus self heals.
    return dynamic_cast<DpsAssistAction*>(action) ||
        dynamic_cast<TankAssistAction*>(action) ||
        dynamic_cast<CastSnareSpellAction*>(action) ||
        dynamic_cast<CastHealingSpellAction*>(action) ||
        dynamic_cast<CastCureSpellAction*>(action) ||
        dynamic_cast<CurePartyMemberAction*>(action) ||
        dynamic_cast<ResurrectPartyMemberAction*>(action) ||
        dynamic_cast<PartyMemberActionNameSupport*>(action) ||
        dynamic_cast<MainTankActionNameSupport*>(action) ||
        dynamic_cast<GroupBuffSpellAction*>(action) ||
        dynamic_cast<CastProtectSpellAction*>(action) ||
        dynamic_cast<CastInnervateOnHealerAction*>(action) ||
        dynamic_cast<CastDebuffSpellOnAttackerAction*>(action) ||
        dynamic_cast<CastDebuffSpellOnMeleeAttackerAction*>(action) ? 0.0f : 1.0f;
}

float LeotherasTheBlindMeleeAvoidChaosBlastMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsMelee(bot))
        return 1.0f;

    if (!dynamic_cast<AttackAction*>(action) &&
        !dynamic_cast<ReachTargetAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action) &&
        (bot->getClass() != CLASS_ROGUE || !dynamic_cast<CastKillingSpreeAction*>(action)))
    {
        return 1.0f;
    }

    if (dynamic_cast<LeotherasTheBlindDestroyInnerDemonAction*>(action))
        return 1.0f;

    if (!HasTooManyChaosBlastStacks(bot))
        return 1.0f;

    Creature* leotherasDemon = GetActiveLeotherasDemon(botAI);
    return leotherasDemon && leotherasDemon->GetVictim() != bot ? 0.0f : 1.0f;
}

float LeotherasTheBlindWaitForDpsMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    Unit* leotheras = AI_VALUE2(Unit*, "find target", "21215");
    if (!leotheras)
        return 1.0f;

    if (HasInnerDemon(bot))
        return 1.0f;

    uint32 const instanceId = leotheras->GetInstanceId();
    uint32 const now = getMSTime();

    if (IsLeotherasHumanoidPhase(botAI))
    {
        if (PlayerbotAI::IsTank(bot))
            return 1.0f;

        auto it = leotherasHumanoidPhaseDpsWaitTimer.find(instanceId);
        if (it == leotherasHumanoidPhaseDpsWaitTimer.end() ||
            getMSTimeDiff(it->second, now) < LEOTHERAS_HUMANOID_DPS_WAIT_MS)
        {
            return 0.0f;
        }

        auto whirlwind = leotherasWhirlwindEndTime.find(instanceId);
        if (whirlwind == leotherasWhirlwindEndTime.end() || now < whirlwind->second)
            return 1.0f;

        return now - whirlwind->second < LEOTHERAS_HUMANOID_DPS_WAIT_MS ? 0.0f : 1.0f;
    }

    if (IsLeotherasDemonPhase(botAI))
    {
        if (IsLeotherasWarlockTank(bot))
            return 1.0f;

        if (PlayerbotAI::IsTank(bot) && !GetLeotherasWarlockTank(bot))
            return 1.0f;

        auto it = leotherasDemonPhaseDpsWaitTimer.find(instanceId);
        if (it == leotherasDemonPhaseDpsWaitTimer.end())
            return 0.0f;

        return getMSTimeDiff(it->second, now) < LEOTHERAS_DEMON_DPS_WAIT_MS ? 0.0f : 1.0f;
    }

    if (IsLeotherasFinalPhase(botAI))
    {
        if (PlayerbotAI::IsTank(bot) || IsLeotherasWarlockTank(bot))
            return 1.0f;

        auto it = leotherasFinalPhaseDpsWaitTimer.find(instanceId);
        if (it == leotherasFinalPhaseDpsWaitTimer.end())
            return 0.0f;

        return getMSTimeDiff(it->second, now) < LEOTHERAS_FINAL_DPS_WAIT_MS ? 0.0f : 1.0f;
    }

    return 1.0f;
}

// By leewheel 2026-09-22 合并brighton the-lab a5541bb4：上游本次【重新启用】本 multiplier
//   （新注释：Soulshatter 需至少两个攻击者才可放，终阶段正是如此，用来让术士T 不把暗影的仇恨丢掉）。
//   我方 2026-09-19 曾按"上游 SscDelayDpsCooldownsMultiplier 已覆盖"将其实现注释停用，
//   现随上游恢复（.h 声明与 SSCStrategy.cpp 注册已由本次合并自动带回）；
//   目标查找按规则第 97 条 entry 化（leotheras the blind = 21215）。
//   2026-09-23 上游 8a778540 再把类名由 ...DisableWarlockTankSoulshatterMultiplier 改为
//   ...DisableTankSoulshatterMultiplier（限制范围由"术士T"泛化为"坦克"），
//   触发名同步为 "leotheras the blind disable tank soulshatter"；.h 声明与 SSCStrategy.cpp
//   注册均已由本次自动合并带回，故此处定义必须同名，否则声明与定义不匹配。
float LeotherasTheBlindDisableTankSoulshatterMultiplier::GetValueInEncounter(
    Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_WARLOCK)
        return 1.0f;

    if (!dynamic_cast<CastSoulshatterAction*>(action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "21215"))
        return 1.0f;

    return GetActiveLeotherasDemon(botAI) && IsLeotherasWarlockTank(bot) ? 0.0f : 1.0f;
}

// Fathom-Lord Karathress

float FathomLordKarathressDisableTankActionsMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    // By leewheel 2026-09-22 按规则第 97 条 entry 化：fathom-lord karathress = 21214。
    if (!AI_VALUE2(Unit*, "find target", "21214"))
        return 1.0f;

    // By leewheel 2026-09-22 合并brighton the-lab a5541bb4：采纳上游新增的坦克动作豁免
    //   （阵型移动/躲 AOE 一律放行；AoE 仇恨与 AoE 嘲讽只在隔壁议会成员够近时才压住，
    //    单体嘲讽始终放行 —— 用于把跟错坦克的卫士、失控的宠物、被打飞的卫士拉回来）。
    if (dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<AvoidAoeAction*>(action))
        return 0.0f;

    // Hold AoE threat and taunts only when another tank's target is close enough to be hit.
    if (IsAoeThreatAction(bot, action) || IsAoeTauntAction(bot, action))
    {
        return IsAnotherCouncilMemberWithin(botAI, KARATHRESS_AOE_THREAT_CLEARANCE) ?
            0.0f : 1.0f;
    }

    return 1.0f;
}

float FathomLordKarathressDisableAutoTargetMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<DpsAssistAction*>(action) && !dynamic_cast<TankAssistAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "21214") ? 0.0f : 1.0f;
}

float FathomLordKarathressDisableAoeMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsDps(bot))
        return 1.0f;

    auto castSpellAction = dynamic_cast<CastSpellAction*>(action);
    if (!castSpellAction || castSpellAction->getThreatType() != Action::ActionThreatType::Aoe)
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "21214") ? 0.0f : 1.0f;
}

float FathomLordKarathressWaitForDpsMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsDps(bot))
        return 1.0f;

    if (!dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<AttackAction*>(action))
        return 1.0f;

    // By leewheel 2026-09-22 合并brighton the-lab a5541bb4：上游删去冗余的 CastHealingSpellAction 豁免
    //   （函数开头已对所有非 DPS/治疗者 return 1.0f，治疗者根本走不到这里），行为不变；
    //   目标查找按规则第 97 条 entry 化（fathom-lord karathress = 21214）。
    Unit* karathress = AI_VALUE2(Unit*, "find target", "21214");
    if (!karathress)
        return 1.0f;

    auto it = karathressDpsWaitTimer.find(karathress->GetInstanceId());
    if (it == karathressDpsWaitTimer.end())
        return 0.0f;

    return getMSTimeDiff(it->second, getMSTime()) < KARATHRESS_DPS_WAIT_MS ? 0.0f : 1.0f;
}

float FathomLordKarathressMaintainPositionMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    // By leewheel 2026-09-22 按规则第 97 条 entry 化：fathom-lord karathress = 21214。
    if (!AI_VALUE2(Unit*, "find target", "21214"))
        return 1.0f;

    // By leewheel 2026-09-22 合并brighton the-lab a5541bb4：采纳上游新增的站位维持豁免
    //   （跟随/逃跑放行；只有 Caribdis 的治疗者、且是 Reach 目标动作时才压住），
    //   并按规则第 97 条 entry 化：fathom-guard caribdis = 21964
    //   （上游原文误拼为 "fathom-guard caribis"，用 entry 后该笔误自然消除）。
    if (dynamic_cast<FollowAction*>(action) || dynamic_cast<FleeAction*>(action))
        return 0.0f;

    // Caribdis healer needs to maintain position.
    if (!PlayerbotAI::IsAssistHealOfIndex(bot, 0, true))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action) ||
        dynamic_cast<FathomLordKarathressPositionCaribdisTankHealerAction*>(action) ||
        dynamic_cast<FathomLordKarathressDropFromCycloneAction*>(action))
    {
        return 1.0f;
    }

    // By leewheel 2026-09-23 合并brighton the-lab 356c39f4（flk testing / more flk testing）：
    //   上游本处把拼错的 "fathom-guard caribis" 修正为 "fathom-guard caribdis"（BASE 里少一个 d，
    //   该行永远匹配不到目标 ⇒ 本 multiplier 恒返回 1.0f）。我方早在其前就已按规则第 97 条
    //   entry 化成 21964=深水卫士卡里布迪斯，语义与上游的修复完全一致，故沿用我方 entry 写法。
    //   上游同函数上方把 dynamic_cast<ReachTargetAction*> 扩成 MovementAction + 两个排除项的
    //   结构改动已原样采纳（见上方代码）。
    return AI_VALUE2(Unit*, "find target", "21964") ? 0.0f : 1.0f;
    // End By leewheel
}

// Hold casts through the Cyclone and the drop after it. Point moves stall mid-cast, so a cast on
// the way down leaves the bot stuck in the air.
float FathomLordKarathressNoCastingWhileLiftedMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<CastSpellAction*>(action))
        return 1.0f;

    if (bot->HasAura(Id(SscSpells::SPELL_CYCLONE)))
        return 0.0f;

    // Only a bot that is moving can be partway down, so the height is looked up for no one else
    if (bot->movespline->Finalized())
        return 1.0f;

    float const floorZ = bot->GetMapHeight(
        bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), true, MAX_FALL_DISTANCE);
    return floorZ > INVALID_HEIGHT && bot->GetPositionZ() - floorZ > CYCLONE_DROP_HEIGHT ?
        0.0f : 1.0f;
}

// Out of sight of Caribdis, only the walk to her and the Cyclone drop can move the bot. Spread
// and the stock reach on its old target would just pull it back.
float FathomLordKarathressApproachingCaribdisMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) ||
        dynamic_cast<FathomLordKarathressAssignDpsPriorityAction*>(action) ||
        dynamic_cast<FathomLordKarathressDropFromCycloneAction*>(action))
    {
        return 1.0f;
    }

    // Healers are ranged too, but the walk is not theirs and they need to move freely
    if (!PlayerbotAI::IsRanged(bot) || !PlayerbotAI::IsDps(bot))
        return 1.0f;

    // By leewheel 2026-09-23 合并 brighton the-lab 8a778540：本 multiplier 是上游本轮新增，
    //   按规则第 97 条 entry 化：fathom-guard caribdis = 21964（深水卫士卡里布迪斯）、
    //   fathom-guard tidalvess = 21965（深水卫士泰达维斯）。
    Unit* caribdis = AI_VALUE2(Unit*, "find target", "21964");
    if (!caribdis)
        return 1.0f;

    // Only while she is the kill target: a totem in reach and Tidalvess come before her
// By leewheel 2026-09-26 合并brighton: GetSpitfireTotem签名收botAI(与定义一致), 查找entry化(21965)
    if (ShouldAttackSpitfireTotem(bot, GetSpitfireTotem(botAI)) ||
        AI_VALUE2(Unit*, "find target", "21965"))
    // End By leewheel
    {
        return 1.0f;
    }

    return bot->IsWithinLOSInMap(caribdis) ? 1.0f : 0.0f;
}

// Keep the totem and Caribdis targeted when LoS breaks (the ledge, the walk out to her).
// Otherwise the bot drops them and grabs whatever it can see.
float FathomLordKarathressDontDropOutOfSightTargetMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<DropTargetAction*>(action))
        return 1.0f;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return 1.0f;

    if (target == GetSpitfireTotem(botAI))
        return 0.0f;

    // By leewheel 2026-09-23 合并brighton the-lab 356c39f4：上游把本 multiplier 由
    //   KeepSpitfireTotemTarget 扩成 KeepTargetOutOfSight —— 出视线的目标（本体在脚边的图腾、
    //   或拉到视线外的卡里布迪斯）都不该被 DropTargetAction 丢掉。按规则第 97 条 entry 化：
    //   fathom-guard caribdis = 21964（深水卫士卡里布迪斯）。
    return target == AI_VALUE2(Unit*, "find target", "21964") ? 0.0f : 1.0f;
    // End By leewheel
}

// Morogrim Tidewalker

// By leewheel 2026-09-19 合并上游 the-lab：上游统一的 SscDelayDpsCooldownsMultiplier
//   已覆盖「踏潮者（21213）的嗜血/英勇只在鱼人波(21920)开」，故按「有上游就不留自建」删除
//   本分支自建的 MorogrimTidewalkerDelayBloodlustAndHeroismMultiplier。
// End By leewheel
float MorogrimTidewalkerDisableTankActionsMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    // By leewheel 2026-09-23 合并 brighton the-lab 8a778540：上游把本 multiplier 的判定顺序
    //   反转为「先判动作类型、再查 boss」，并合并为单条 return，语义等价（我方原写法为
    //   「先查 boss 存在、再判动作类型」）。取上游结构，按规则第 97 条 entry 化：
    //   morogrim tidewalker = 21213（莫洛格里·踏潮者）。
    if (!dynamic_cast<CombatFormationMoveAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "21213") ? 0.0f : 1.0f;
    // End By leewheel
}

// By leewheel 2026-09-23 合并 brighton the-lab 8a778540：上游把本 multiplier 由
//   MorogrimTidewalkerMaintainPhase2StackingMultiplier 改名并重写为
//   MorogrimTidewalkerStayStackedMultiplier —— 判定顺序反转为「先判动作类型、再查 boss」，
//   并把硬编码的 25.0f 换成已有常量 TIDEWALKER_PHASE_2_HEALTH_PCT（SSCHelpers.h:334），
//   语义完全一致。取上游实现，按规则第 97 条 entry 化：morogrim tidewalker = 21213。
// By leewheel 2026-09-24 合并 brighton 999582f7：上游更新本 multiplier 语义并补充注释——
//   只有已在集合点的 bot 才被按住，还在路上的 bot 保留其原有移动，
//   避免无法执行的 stack 步骤把它卡死；判定改用 TIDEWALKER_PHASE_2_MOVE_HEALTH_PCT
//   （比 P2 血量阈值宽 2%）+ GetTidewalkerStackPoint 距离。
float MorogrimTidewalkerStayStackedMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsRanged(bot))
        return 1.0f;

    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<MorogrimTidewalkerStackRangedBehindBossAction*>(action))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action) && !IsRepositionAction(bot, action))
        return 1.0f;

    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "21213");
    if (!tidewalker || tidewalker->GetHealthPct() > TIDEWALKER_PHASE_2_MOVE_HEALTH_PCT)
        return 1.0f;

    return bot->GetExactDist(GetTidewalkerStackPoint(tidewalker)) <=
        TIDEWALKER_RANGED_STACK_RADIUS ? 0.0f : 1.0f;
    // End By leewheel
}

// Lady Vashj <Coilfang Matron>

// By leewheel 2026-09-19 合并上游 the-lab：上游统一的 SscDelayDpsCooldownsMultiplier
//   已覆盖「瓦丝琪（21212）阶段门控：3 阶段全开、2 阶段仅非嗜血大招」，故按「有上游就不留自建」删除
//   本分支自建的 LadyVashjDelayCooldownsMultiplier。
// End By leewheel
float LadyVashjSetGroundingTotemMultiplier::GetValueInEncounter(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

// By leewheel 2026-09-26 合并brighton: 采纳brighton图腾动作限定+声明vashj(公共尾部需用vashj), 查找entry化(21212)
    if (!dynamic_cast<CastWindfuryTotemAction*>(action) &&
        !dynamic_cast<SetWindfuryTotemAction*>(action) &&
        !dynamic_cast<CastWrathOfAirTotemAction*>(action) &&
        !dynamic_cast<SetWrathOfAirTotemAction*>(action) &&
        !dynamic_cast<CastNatureResistanceTotemAction*>(action) &&
        !dynamic_cast<SetNatureResistanceTotemAction*>(action))
    {
        return 1.0f;
    }

    Unit* vashj = AI_VALUE2(Unit*, "find target", "21212");
    if (!vashj)
        return 1.0f;
    // End By leewheel

    // Shock Blast is cast in phases 1 and 3 only
    int8 const phase = GetLadyVashjPhase(vashj);
    if (phase != 1 && phase != 3)
        return 1.0f;

    return GetVashjGroundingShaman(bot) == bot ? 0.0f : 1.0f;
}

float LadyVashjMaintainPhase1RangedSpreadMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsRanged(bot))
        return 1.0f;

    // By leewheel 2026-09-24 合并 brighton 999582f7(fix old vashj multiplier bug)：
    //   上游把本判定反转为「非 spread 类动作直接放行」，与旧写法逻辑等价但更直白，采纳上游结构。
    // End By leewheel
    if (!dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<FleeAction*>(action) && !IsRepositionAction(bot, action))
    {
        return 1.0f;
    }

    // By leewheel 2026-09-24 合并 brighton 999582f7：上游此行用英文名 "lady vashj" 检索，
    //   中文客户端下永远命中不到，按规则第 97 条 entry 化为 21212。
    // End By leewheel
    Unit* vashj = AI_VALUE2(Unit*, "find target", "21212");
    return vashj && GetLadyVashjPhase(vashj) == 1 ? 0.0f : 1.0f;
}

float LadyVashjStaticChargeStayAwayFromGroupMultiplier::GetValueInEncounter(Action* action)
{
    // Only melee need holding back from a charged tank. Everyone else still has to reach targets to
    // heal or cast, and ReachPartyMemberToHealAction is a ReachTargetAction.
    if (!PlayerbotAI::IsMelee(bot) && !HasStaticCharge(bot))
        return 1.0f;

// By leewheel 2026-09-26 合并brighton: 采纳动作类型限定(仅抑制移动/追杀类action)
    if (!dynamic_cast<ReachTargetAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<FollowAction*>(action) &&
        !dynamic_cast<CastKillingSpreeAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }
    // End By leewheel

    Unit* vashj = AI_VALUE2(Unit*, "find target", "21212");
    return vashj && ShouldAvoidVashjStaticCharge(bot, vashj) ? 0.0f : 1.0f;
}

// Bots should not loot the core with normal looting logic. Both the walk to a corpse and opening
// it: a bot in the non-combat engine mid-fight runs the loot strategy, and one already beside the
// corpse needs no walk.
float LadyVashjDoNotLootTheTaintedCoreMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<LootAction*>(action) && !dynamic_cast<OpenLootAction*>(action))
        return 1.0f;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "21212");
    return vashj && GetLadyVashjPhase(vashj) == 2 ? 0.0f : 1.0f;
}

float LadyVashjCorePassersPrioritizePositioningMultiplier::GetValueInEncounter(Action* action)
{
    if (Unit* vashj = AI_VALUE2(Unit*, "find target", "21212");
        !vashj || GetLadyVashjPhase(vashj) != 2)
    {
        return 1.0f;
    }

    if (dynamic_cast<WipeAction*>(action))
        return 1.0f;

    // Only the chain's start and catchers are held here
    VashjCoreChain const* chain = GetVashjCoreChain(bot);
    if (!chain)
        return 1.0f;

    int8 const index = GetVashjCoreCatcherIndex(*chain, bot);
    if (index < 0 && chain->start != bot->GetGUID())
        return 1.0f;

    bool const isPass = dynamic_cast<LadyVashjPassTheTaintedCoreAction*>(action);

    // A holder only passes, uses or destroys the core
    if (HasTaintedCore(bot))
    {
        return isPass || dynamic_cast<LadyVashjDestroyTaintedCoreAction*>(action) ?
            1.0f : 0.0f;
    }

    // The designated looter must stay on the Tainted Elemental until it has the core.
    if (dynamic_cast<LadyVashjAssignPhase2AndPhase3DpsPriorityAction*>(action) &&
        GetDesignatedCoreLooter(botAI, bot) == bot)
    {
        constexpr float corpseSearchRadius = 30.0f;
        if (AI_VALUE2(Unit*, "find target", "22009") ||
            bot->FindNearestCreature(Id(SscNpcs::NPC_TAINTED_ELEMENTAL), corpseSearchRadius, false))
            return 0.0f;
    }

    // By leewheel 2026-09-28 合并 brighton 219cd311（vashj 传核链重做）：采纳上游"就位中的包手不做其他
    //   移动"判定，取代我方旧版（首二传包手在拾取者贴近元素时锁定移动 + AnyRecentCoreInInventory）——
    //   新机制由 catcher.prepositions/readyDelay 统一表达，语义等价且不回退。
    // End By leewheel
    // A catcher on its way to, or standing on, its spot moves for nothing else
    if (!isPass && index >= 0 && dynamic_cast<MovementAction*>(action) &&
        IsVashjCoreCatcherActive(bot, *chain, index))
    {
        return 0.0f;
    }

    return 1.0f;
}

// All of phases 2 and 3 require a custom movement and targeting system
// So the standard target selection system must be disabled
float LadyVashjDisableAutoTargetAndMoveMultiplier::GetValueInEncounter(Action *action)
{
    // By leewheel 2026-09-27 合并 brighton fa573dd3：采纳上游新增的六类动作门控判定
    //   （assist/follow-flee/debuff-on-attacker/formation/reach/avoid-aoe，下游代码引用这些变量），
    //   并按规则第 97 条 entry 化：lady vashj = 21212（瓦丝琪）。
    // End By leewheel
    bool const isAssist =
        dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<TankAssistAction*>(action);
    bool const isFollowOrFlee =
        dynamic_cast<FollowAction*>(action) || dynamic_cast<FleeAction*>(action);
    bool const isDebuffOnAttacker = dynamic_cast<CastDebuffSpellOnAttackerAction*>(action);
    bool const isFormation = dynamic_cast<CombatFormationMoveAction*>(action);
    bool const isReach = dynamic_cast<ReachTargetAction*>(action);
    bool const isAvoidAoe = dynamic_cast<AvoidAoeAction*>(action);
    if (!isAssist && !isFollowOrFlee && !isDebuffOnAttacker && !isFormation && !isReach &&
        !isAvoidAoe && !dynamic_cast<CastHealingSpellAction*>(action) &&
        !IsRepositionAction(bot, action))
    {
        return 1.0f;
    }

    Unit* vashj = AI_VALUE2(Unit*, "find target", "21212");
    if (!vashj)
        return 1.0f;

    if (isAvoidAoe)
        return 0.0f;

    int8 const phase = GetLadyVashjPhase(vashj);

    if (phase == 2)
    {
        if (isAssist && botAI->GetState() == BOT_STATE_COMBAT)
            return 0.0f;

        if (dynamic_cast<FleeAction*>(action))
            return 0.0f;

        if (dynamic_cast<FollowAction*>(action) && bot->GetExactDist2d(vashj) < 60.0f)
            return 0.0f;

        if (!PlayerbotAI::IsHeal(bot) && dynamic_cast<CastHealingSpellAction*>(action))
            return 0.0f;

        // By leewheel 2026-09-27 合并 brighton fa573dd3：采纳上游的 isDebuffOnAttacker 分组结构，
        //   并按规则第 97 条 entry 化：enchanted elemental = 21958（魔化元素）。
        // End By leewheel
        if (isDebuffOnAttacker)
        {
            Unit* enchanted = AI_VALUE2(Unit*, "find target", "21958");
            if (enchanted && AI_VALUE(Unit*, "current target") == enchanted)
                return 0.0f;
        }

        // Cluster ranged shoot from their slots. Only those sent after a Tainted Elemental walk
        // to it, and those stepping in to cast range of a Strider.
        if ((isReach || isFormation || IsRepositionAction(bot, action)) &&
            PlayerbotAI::IsRangedDps(bot))
        {
            bool const stepsInToStrider =
                isReach && IsVashjStriderToStepInTo(bot, AI_VALUE(Unit*, "current target"));
            if (!stepsInToStrider)
            {
                // By leewheel 2026-09-27 合并 brighton fa573dd3 后补齐 entry 化：
                //   tainted elemental = 22009（被污染的元素，查库核定）。
                Unit* tainted = AI_VALUE2(Unit*, "find target", "22009");
                if (!tainted || !IsVashjTaintedElementalKiller(bot, tainted))
                    return 0.0f;
            }
        }

        // Cluster healers heal from their slots too
        if ((isReach || isFormation) && PlayerbotAI::IsHeal(bot) &&
            GetVashjClusterSlot(bot).cluster >= 0)
        {
            return 0.0f;
        }
    }

    if (phase == 3)
    {
        if (isAssist && botAI->GetState() == BOT_STATE_COMBAT)
            return 0.0f;

        if (!isFollowOrFlee && !isDebuffOnAttacker && !isFormation)
            return 1.0f;

        Unit* enchanted = AI_VALUE2(Unit*, "find target", "21958");
        Unit* strider = AI_VALUE2(Unit*, "find target", "22056");
        Unit* elite = AI_VALUE2(Unit*, "find target", "22055");
        if (enchanted || strider || elite)
        {
            if (isFollowOrFlee)
                return 0.0f;

            if (isDebuffOnAttacker && enchanted && AI_VALUE(Unit*, "current target") == enchanted)
                return 0.0f;
        }
        else if (isFormation)
            return 0.0f;
    }

    return 1.0f;
}

float LadyVashjSaveHandOfFreedomMultiplier::GetValueInEncounter(Action *action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_PALADIN)
        return 1.0f;

    if (!dynamic_cast<CastHandOfFreedomOnPartyAction*>(action))
        return 1.0f;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "21212");
    return vashj && GetLadyVashjPhase(vashj) == 3 ? 0.0f : 1.0f;
}

// Near a pool, only the melee spore action moves melee dps. Stock reach-melee would take them
// straight back through it.
float LadyVashjMeleeControlSporeAvoidanceMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<LadyVashjMeleeMoveAroundToxicSporesAction*>(action) ||
        dynamic_cast<LadyVashjAssignPhase2AndPhase3DpsPriorityAction*>(action) ||
        dynamic_cast<LadyVashjSetGroundingTotemInMainTankGroupAction*>(action))
    {
        return 1.0f;
    }

    if (!IsVashjRingMelee(bot))
        return 1.0f;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "21212");
    if (!vashj || GetLadyVashjPhase(vashj) != 3)
        return 1.0f;

    return IsNearToxicSpores(botAI, bot, TOXIC_SPORES_MELEE_CONTROL_RADIUS) ? 0.0f : 1.0f;
}

// Stock reach-spell for ranged dps, and reach-to-heal for healers, only while its straight walk is
// clear of pools; otherwise the ranged spore action goes round them.
float LadyVashjRangedDoNotReachThroughSporesMultiplier::GetValueInEncounter(Action* action)
{
    bool const isHealerReach = dynamic_cast<ReachPartyMemberToHealAction*>(action);
    if (!isHealerReach && !dynamic_cast<ReachSpellAction*>(action))
        return 1.0f;

    if (isHealerReach != PlayerbotAI::IsHeal(bot))
        return 1.0f;

    Unit* target;
    float range;
    return GetVashjReachBlockedBySpores(botAI, bot, target, range) ? 0.0f : 1.0f;
}
