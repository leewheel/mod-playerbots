/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SSCTriggers.h"
#include "EncounterHelpers.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "SSCActions.h"
#include "SSCHelpers.h"
#include "TemporarySummon.h"
#include <algorithm>
#include <vector>

using namespace SscHelpers;
using namespace EncounterHelpers;

// General

bool SscNoEncounterInProgressTrigger::IsActive()
{
    return !IsEncounterInProgress(bot, SSC_MAP_ID);
}

// Trash Mobs

bool UnderbogColossusInToxicPoolTrigger::IsActive()
{
    return IsInToxicPool(botAI);
}

bool GreyheartTidecallerWaterElementalTotemSpawnedTrigger::IsActive()
{
// By leewheel 2026-09-22 合并brighton the-lab a5541bb4：采纳上游把判定改为"机制跟踪者"
    //   （IsMechanicTrackerBot），并按规则第 97 条 entry 化：greyheart tidecaller = 21229(暗心唤潮者)。
    return IsMechanicTrackerBot(bot, SSC_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "21229");
    // End By leewheel
}

// Hydross the Unstable <Duke of Currents>

bool HydrossTheUnstableShouldBeTankedByFrostTankTrigger::IsActiveInEncounter()
{
    // By leewheel 2026-09-29 合并brighton 07e47c61: 上游改用 IsHydrossFrostTank/IsHydrossNatureTank
    //   语义化助手取代裸 IsMainTank/IsAssistTankOfIndex；保留 rule81 entry 化 21216。
    // End By leewheel
    return IsHydrossFrostTank(bot) && AI_VALUE2(Unit*, "find target", "21216");
}

bool HydrossTheUnstableShouldBeTankedByNatureTankTrigger::IsActiveInEncounter()
{
    return IsHydrossNatureTank(bot) && AI_VALUE2(Unit*, "find target", "21216");
}

bool HydrossTheUnstableRangedShouldSpreadInFrostPhaseTrigger::IsActiveInEncounter()
{
// By leewheel 2026-09-26 合并brighton: 采纳冰霜阶段判定, entry化(21216)
    return PlayerbotAI::IsRanged(bot) &&
        IsHydrossInFrostPhase(AI_VALUE2(Unit*, "find target", "21216"));
    // End By leewheel
}

bool HydrossTheUnstableShouldMisdirectUponPhaseChangeTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    // By leewheel 2026-09-29 合并brighton 07e47c61: 上游新增"相位切换 15 秒内无当前相位印记"
    //   判定；保留 rule81 entry 化 21216。
    // End By leewheel
    Unit* hydross = AI_VALUE2(Unit*, "find target", "21216");
    if (!hydross)
        return false;

    // No Mark of the current phase yet means the phase began less than 15s ago.
    return IsHydrossInFrostPhase(hydross) ? HasNoMarkOfHydross(bot) : HasNoMarkOfCorruption(bot);
}

bool HydrossTheUnstableAggroResetsUponPhaseChangeTrigger::IsActiveInEncounter()
{
    // By leewheel 2026-09-29 合并brighton 07e47c61: 上游改用 GetHydrossDpsHoldWindow 判定
    //   停手窗口（猎人仅相位切换后放行以误导），吸收我方旧"排除猎人"判定；entry 化 21216。
    // End By leewheel
    if (!PlayerbotAI::IsDps(bot))
        return false;

    HydrossDpsHoldWindow const window =
        GetHydrossDpsHoldWindow(AI_VALUE2(Unit*, "find target", "21216"));

    // Hunters keep going after the change to misdirect Hydross to the new tank.
    return window == HydrossDpsHoldWindow::BeforePhaseChange ||
        (window == HydrossDpsHoldWindow::AfterPhaseChange && bot->getClass() != CLASS_HUNTER);
}

bool HydrossTheUnstableShouldManagePhaseTimersTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, SSC_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "21216");
}

// The Lurker Below

bool TheLurkerBelowSpoutIsActiveTrigger::IsActiveInEncounter()
{
    // By leewheel 2026-09-13 合并brighton 8c96a663: 采纳上游 e8ac948a 放弃"站位/下潜"方案
    // （TheLurkerBelowRangedShouldHoldStationTrigger / ...ShouldDiveTrigger 已被上游删除），
    // 保留 Spout 判定并按 AGENTS.md 第81条 entry 化（21217 = 深水领主卡拉瑟雷斯 The Lurker Below）
    return IsLurkerSpouting(AI_VALUE2(Unit*, "find target", "21217"));
}

bool TheLurkerBelowShouldBeTankedTrigger::IsActiveInEncounter()
{
    // By leewheel 2026-09-29 合并brighton 07e47c61: 上游放宽为 IsTank 且再加 IsMainTank 收口
    //   （允许可替换主坦者提前走位）；保留 rule81 entry 化 21217。
    // End By leewheel
    return PlayerbotAI::IsTank(bot) &&
        IsLurkerSurfacedAndCalm(AI_VALUE2(Unit*, "find target", "21217")) &&
        PlayerbotAI::IsMainTank(bot);
}

bool TheLurkerBelowRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsRanged(bot) &&
        IsLurkerSurfacedAndCalm(AI_VALUE2(Unit*, "find target", "21217"));
}

bool TheLurkerBelowGuardiansShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* lurker = AI_VALUE2(Unit*, "find target", "21217");
    if (!lurker || lurker->getStandState() != UNIT_STAND_STATE_SUBMERGED)
        return false;

    std::vector<Player*> const tanks = GetLurkerGuardianTanks(bot);
    return std::find(tanks.begin(), tanks.end(), bot) != tanks.end();
}

// Bots are unable to move across the water via ReachMeleeAction. Only bots with charge moves can
// cross onto the islets to attack Ambushers during the submerge phase. They are then stuck there
// until their charge comes off of cooldown. To resolve, issue a direct move to a land position.
bool TheLurkerBelowMeleeCannotReachTargetTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot))
        return false;

    // Consider the bot stuck if it is not moving or casting even with a target out of melee range.
    if (bot->isMoving() || bot->IsNonMeleeSpellCast(false))
        return false;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || bot->IsWithinMeleeRange(target))
        return false;

    // By leewheel 2026-09-19 合并上游 the-lab：规则第 97 条，机器人策略里的 boss 名称必须用 entry。
    //   "the lurker below" ⇒ 21217（同文件 139 行的 Leotheras 已是 "21215"，此处补齐一致性）
    Unit* lurker = AI_VALUE2(Unit*, "find target", "21217");
    // End By leewheel
    return lurker && !IsLurkerSpouting(lurker);
}

// Leotheras the Blind

bool LeotherasTheBlindWarlockShouldTankDemonFormTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "21215"))
        return false;

    if (HasInnerDemon(bot) || !GetActiveLeotherasDemon(botAI))
        return false;

    return IsLeotherasWarlockTank(bot);
}

bool LeotherasTheBlindOnlyWarlockShouldTankDemonFormTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "21215"))
        return false;

    if (HasInnerDemon(bot) || !GetPhase2LeotherasDemon(botAI))
        return false;

    // If there is no Warlock tank, then traditional tanks will have to tank the demon form.
    return GetLeotherasWarlockTank(bot);
}

bool LeotherasTheBlindRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* leotheras = AI_VALUE2(Unit*, "find target", "21215");
    if (!leotheras || IsSpellbinderPhase(leotheras))
        return false;

    return !IsLeotherasChannelingWhirlwind(leotheras);
}

bool LeotherasTheBlindChannelingWhirlwindTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsTank(bot))
        return false;

    // By leewheel 2026-09-29 合并brighton 07e47c61: leotheras 变量后续无引用，上游内联为
    //   IsLeotherasChannelingWhirlwind 直判；保留 rule81 entry 化 21215。
    // End By leewheel
    if (!IsLeotherasChannelingWhirlwind(AI_VALUE2(Unit*, "find target", "21215")))
        return false;

    return !HasInnerDemon(bot);
}

bool LeotherasTheBlindTooManyChaosBlastStacksTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsRanged(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "21215"))
        return false;

    if (!HasTooManyChaosBlastStacks(bot))
        return false;

    Creature* leotherasDemon = GetActiveLeotherasDemon(botAI);
    return leotherasDemon && leotherasDemon->GetVictim() != bot;
}

bool LeotherasTheBlindInnerDemonHasAwakenedTrigger::IsActiveInEncounter()
{
    return HasInnerDemon(bot);
}

bool LeotherasTheBlindInFinalPhaseTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsHeal(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "21215"))
        return false;

    if (HasInnerDemon(bot) || !IsLeotherasFinalPhase(botAI))
        return false;

    return !IsLeotherasWarlockTank(bot);
}

bool LeotherasTheBlindHunterShouldMisdirectDemonFormTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "21215"))
        return false;

    if (HasInnerDemon(bot))
        return false;

    return GetActiveLeotherasDemon(botAI);
}

bool LeotherasTheBlindShouldManageDpsWaitTimersTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, SSC_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "21215");
}

// Fathom-Lord Karathress

bool FathomLordKarathressTargetsShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot) &&
        AI_VALUE2(Unit*, "find target", "21214");
}

bool FathomLordKarathressShouldHealCaribdisTankTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsAssistHealOfIndex(bot, 0, true) &&
        AI_VALUE2(Unit*, "find target", "21964");
}

bool FathomLordKarathressPullingBossesTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* tidalvess = AI_VALUE2(Unit*, "find target", "21965");
    return tidalvess && tidalvess->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT;
}

bool FathomLordKarathressShouldAssignDpsPriorityTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsHeal(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "21214"))
        return false;

    if (PlayerbotAI::IsDps(bot))
        return true;

    if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, false))
        return !AI_VALUE2(Unit*, "find target", "21964");

    // By leewheel 2026-09-22 合并brighton the-lab a5541bb4：采纳上游用 GetSharkkisTankTarget()
    //   （先接手咬住本坦的深水潜伏者/孢子蝠，再回落到沙克基斯），并把 indexLivingOnly 由 true 改回
    //   上游的 false；目标查找按规则第 97 条 entry 化：fathom-guard tidalvess = 21965。
    if (PlayerbotAI::IsAssistTankOfIndex(bot, 1, false))
        return !GetSharkkisTankTarget(botAI);

    if (PlayerbotAI::IsAssistTankOfIndex(bot, 2, false))
        return !AI_VALUE2(Unit*, "find target", "21965");

    return false;
}

bool FathomLordKarathressShouldManageDpsTimerTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, SSC_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "21214");
}

bool FathomLordKarathressRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

// By leewheel 2026-09-23 合并brighton the-lab 356c39f4：上游新增本触发器 —— 旋风会点名
    //   卡里布迪斯施法距离内的随机玩家、并波及自身 4 码，只有 45 码内的范围职业需要散开。
    //   按规则第 97 条 entry 化：fathom-guard caribdis = 21964（深水卫士卡里布迪斯）。
    Unit* caribdis = AI_VALUE2(Unit*, "find target", "21964");
    // End By leewheel
    return caribdis && bot->IsWithinDist(caribdis, CARIBDIS_CYCLONE_SUMMON_RANGE);
    // By leewheel 2026-09-26
}

// A bot left hanging still has the knockback's generator in its controlled slot once the tosses
// are over; while the aura is up, more tosses are coming and the arc is left to run
bool FathomLordKarathressLiftedByCycloneTrigger::IsActiveInEncounter()
{
    if (bot->HasAura(Id(SscSpells::SPELL_CYCLONE)) ||
        bot->GetMotionMaster()->GetMotionSlotType(MOTION_SLOT_CONTROLLED) != EFFECT_MOTION_TYPE)
    {
        return false;
    }

    // Only a bot left well off the floor. Any other knockback, such as Knock Away from Sharkkis's
    // pets (a flat shove topping out under half a yard), is left to run its course.
    float const floorZ = bot->GetMapHeight(
        bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), true, MAX_FALL_DISTANCE);
    if (floorZ <= INVALID_HEIGHT || bot->GetPositionZ() - floorZ <= CYCLONE_DROP_HEIGHT)
        return false;

    return AI_VALUE2(Unit*, "find target", "21214");
}

// Morogrim Tidewalker

bool MorogrimTidewalkerPullingBossTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "21213");
    return tidewalker && tidewalker->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT;
}

bool MorogrimTidewalkerShouldBeTankedTrigger::IsActiveInEncounter()
{
    // By leewheel 2026-09-29 合并brighton 07e47c61: 上游 IsTank 前置 + IsMainTank 收口；
    // 保留 rule81 entry 化 21213。
    return PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "21213") &&
        PlayerbotAI::IsMainTank(bot);
}

bool MorogrimTidewalkerRangedShouldStackTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

// By leewheel 2026-09-24 合并 brighton 999582f7：采纳上游——阈值改用
    //   TIDEWALKER_PHASE_2_MOVE_HEALTH_PCT（比 P2 血量阈值宽 2%、<= 比较），
    //   并新增 P1 治疗集合距离 trigger；英文名按规则第 97 条 entry 化：morogrim tidewalker = 21213。
    //   Ranged set off with the tank: behind him, they cannot get in front of him on the way
    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "21213");
    // End By leewheel
    return tidewalker && tidewalker->GetHealthPct() <= TIDEWALKER_PHASE_2_MOVE_HEALTH_PCT;
}

// Phase 1 only. To keep bots from chasing murlocs across the room, which is particularly prone to
// happening with bots that leave Watery Graves right as murlocs spawn.
bool MorogrimTidewalkerTooFarFromBossTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsTank(bot))
        return false;

    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "21213");
    return tidewalker && tidewalker->GetHealthPct() > TIDEWALKER_PHASE_2_MOVE_HEALTH_PCT &&
        bot->GetExactDist(tidewalker) >= TIDEWALKER_MAX_DISTANCE_FROM_BOSS;
}

// Lady Vashj <Coilfang Matron>

bool LadyVashjShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "21212");
    if (!vashj)
        return false;

    int8 const phase = GetLadyVashjPhase(vashj);
    return (phase == 1 || phase == 3) && PlayerbotAI::IsMainTank(bot);
}

bool LadyVashjRangedShouldSpreadInPhase1Trigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

// By leewheel 2026-09-26 合并brighton: 采纳brighton拆分设计(Phase1 spread/簇位抱簇/Phase3定位4触发器), vashj查找entry化(21212)
    Unit* vashj = AI_VALUE2(Unit*, "find target", "21212");
    if (!vashj || GetLadyVashjPhase(vashj) != 1)
        return false;

    return !HasVashjStaticCharge(bot);
}

bool LadyVashjClusterSlotsNeedHoldersTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, SSC_MAP_ID))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "21212");
    return vashj && GetLadyVashjPhase(vashj) == 2 && HasVashjClusterVacancy(bot);
}

bool LadyVashjShouldHoldClusterInPhase2Trigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRangedDps(bot) && !PlayerbotAI::IsHeal(bot))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "21212");
    return vashj && GetLadyVashjPhase(vashj) == 2;
}

// Hunters are left free to go after Sporebats, and the Static Charge action moves a holder on its
// own.
bool LadyVashjRangedShouldPositionInPhase3Trigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsCaster(bot) || HasVashjStaticCharge(bot))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "21212");
    return vashj && GetLadyVashjPhase(vashj) == 3;
    // End By leewheel
}

bool LadyVashjShamanShouldGroundShockBlastTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_SHAMAN)
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "21212");
    if (!vashj)
        return false;

    int8 const phase = GetLadyVashjPhase(vashj);
    if (phase != 1 && phase != 3)
        return false;

    return GetVashjGroundingShaman(bot) == bot;
}

bool LadyVashjStaticChargeOnGroupMemberTrigger::IsActiveInEncounter()
{
    // By leewheel 2026-09-29 合并brighton 07e47c61: 按规则第 97 条 entry 化 = 21212
    return ShouldAvoidVashjStaticCharge(bot, AI_VALUE2(Unit*, "find target", "21212"));
}

bool LadyVashjPullingBossTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

// By leewheel 2026-09-26 合并brighton: 采纳简化健康判定(BOSS_ENGAGED_HEALTH_PCT), 查找entry化(21212)
    Unit* vashj = AI_VALUE2(Unit*, "find target", "21212");
    return vashj && vashj->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT;
    // End By leewheel
}

// Healers too. Healer dps and a priest's wand get a target the tiers allow, never a Sporebat,
// which walks them up into the air, and in phase 2 the target keeps them in their combat engine.
// The phase 2 multiplier keeps them from walking to it.
bool LadyVashjAddsSpawnInPhase2AndPhase3Trigger::IsActiveInEncounter()
{
// By leewheel 2026-09-27 合并brighton the-lab：brighton 用名字 "lady vashj"，
    //   按规则第97条 entry 化保留 "21212"（Lady Vashj）。
    if (PlayerbotAI::IsHeal(bot))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "21212");
    // End By leewheel
    if (!vashj)
        return false;

    int8 const phase = GetLadyVashjPhase(vashj);
    return phase == 2 || phase == 3;
}

// Strider actions are predicated on the fact that you will have only one Strider up at once.
// If you have more than one up at a time, you likely do not have the DPS to complete the fight.
bool LadyVashjCoilfangStriderIsApproachingTrigger::IsActiveInEncounter()
{
// By leewheel 2026-09-26 合并brighton: 采纳IsTank门槛, 查找entry化(22056)
    return PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "22056");
    // End By leewheel
}

bool LadyVashjCoilfangEliteShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

// By leewheel 2026-09-26 合并brighton: 采纳"当前目标为coilfang精英且打bot"语义
    Unit* elite = AI_VALUE(Unit*, "current target");
    if (!elite || elite->GetEntry() != Id(SscNpcs::NPC_COILFANG_ELITE) || elite->GetVictim() != bot)
        return false;
    Unit* vashj = AI_VALUE2(Unit*, "find target", "21212");
    return vashj && GetLadyVashjPhase(vashj) == 2;
}

// Idle means not on an Elite, a Strider, or an Enchanted near her.
bool LadyVashjTankIsIdleAwayFromTheMiddleTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

// By leewheel 2026-09-26 合并brighton: 采纳"phase2且距boss≥VASHJ_IDLE_TANK_DISTANCE即空闲"语义(与上方注释一致), 查找entry化(21212)
    Unit* vashj = AI_VALUE2(Unit*, "find target", "21212");
    if (!vashj || GetLadyVashjPhase(vashj) != 2 ||
        bot->GetExactDist(vashj) < VASHJ_IDLE_TANK_DISTANCE)
    {
        return false;
    }
    // End By leewheel

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || !target->IsAlive())
        return true;

    switch (target->GetEntry())
    {
        case Id(SscNpcs::NPC_COILFANG_ELITE):
        case Id(SscNpcs::NPC_COILFANG_STRIDER):
            return false;
        case Id(SscNpcs::NPC_ENCHANTED_ELEMENTAL):
            return vashj->GetExactDist2d(target) > VASHJ_ENCHANTED_NEAR_HER_DISTANCE;
        default:
            return true;
    }
}

// Only a new elemental, or a looter who died on the way, needs a looter chosen.
// Phase 2 only, as are the attack and loot triggers below: a core looted in phase 3 has no
// generator left, and its Paralyze would root the looter.
bool LadyVashjTaintedElementalNeedsLooterTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, SSC_MAP_ID))
        return false;

    // By leewheel 2026-09-27 合并 brighton fa573dd3 后补齐 entry 化（本文件共 6 处）：
    //   lady vashj = 21212（瓦丝琪）、tainted elemental = 22009（被污染的元素）、
    //   fathom-lord karathress = 21214（深水领主卡拉瑟雷斯），均查库核定。
    Unit* vashj = AI_VALUE2(Unit*, "find target", "21212");
    if (!vashj || GetLadyVashjPhase(vashj) != 2)
        return false;

    Unit* tainted = AI_VALUE2(Unit*, "find target", "22009");
    if (!tainted)
        return false;

    auto it = vashjTaintedCoreLooter.find(bot->GetInstanceId());
    if (it == vashjTaintedCoreLooter.end() || it->second.tainted != tainted->GetGUID())
        return true;

    Player* looter = ObjectAccessor::GetPlayer(*bot, it->second.looter);
    return !looter || !looter->IsAlive();
}

// The ranged dps of the cluster nearest the elemental. Its looter waits beside it instead (see
// the loot action).
bool LadyVashjBotShouldAttackTaintedElementalTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsTank(bot))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "21212");
    if (!vashj || GetLadyVashjPhase(vashj) != 2)
        return false;

    // By leewheel 2026-09-29 合并brighton 07e47c61: 判定函数随上游重命名为
    //   IsAssignedToAttackTaintedElemental；保留 rule81 entry 化 22009。
    Unit* tainted = AI_VALUE2(Unit*, "find target", "22009");
    return tainted && IsAssignedToAttackTaintedElemental(bot, tainted);
}

// From the looter's pick until the core is taken from the corpse.
bool LadyVashjBotIsTaintedCoreLooterTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsTank(bot) || GetDesignatedCoreLooter(bot) != bot)
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "21212");
    if (!vashj || GetLadyVashjPhase(vashj) != 2)
        return false;

    Creature* tainted = GetAssignedTaintedElemental(bot);

    // TEMP LOG
    bool const hasCore = HasTaintedCore(bot);
    if (hasCore && TaintedLogFirstTime(bot, "core"))
    {
        LOG_INFO("playerbots", "[SSC tainted] +{}ms looter {} has the core",
            TaintedLogElapsedMs(bot), bot->GetName());
    }
    if (!tainted && TaintedLogFirstTime(bot, "gone"))
    {
        LOG_INFO("playerbots", "[SSC tainted] +{}ms elemental gone, core looted: {}",
            TaintedLogElapsedMs(bot), TaintedLogSeen(bot, "core") ? "yes" : "NO");
    }

    return tainted && IsTaintedCoreStillToLoot(tainted);
}

// A core with nowhere to go: in phase 3, with no generator left; from a chain that found no way; or
// still held when the next core is ready to loot. Its Paralyze roots the holder until it leaves the
// bags.
bool LadyVashjBotShouldDestroyTaintedCoreTrigger::IsActiveInEncounter()
{
    // By leewheel 2026-09-28 合并 brighton 219cd311（vashj 传核链重做）：采纳上游"核无处可去"判定
    //   ——阶段3 或无路可走的链、或下一颗核已可拾取时仍持核者销毁；按规则第 97 条 entry 化：lady vashj = 21212。
    // End By leewheel
    Unit* vashj = AI_VALUE2(Unit*, "find target", "21212");
    int8 const phase = GetLadyVashjPhase(vashj);
    if (phase == 3)
        return HasTaintedCore(bot);

    if (phase != 2)
        return false;

    // In phase 2 only a member of this chain or the one it replaced can hold a core
    VashjCoreChain const* chain = GetVashjCoreChain(bot);
    if (!chain)
        return false;

    ObjectGuid const guid = bot->GetGUID();
    bool const member = chain->start == guid || GetVashjCoreCatcherIndex(*chain, bot) >= 0 ||
        std::find(chain->earlier.begin(), chain->earlier.end(), guid) != chain->earlier.end();
    if (!member)
        return false;

    if (!chain->failed)
    {
        Creature* nextTainted = GetAssignedTaintedElemental(bot);
        if (!nextTainted || nextTainted->IsAlive() || GetTaintedCoreLootSlot(nextTainted) < 0)
            return false;
    }

    return HasTaintedCore(bot);
}

// The chain's start or one of its catchers, while it holds the core or is due at its spot.
bool LadyVashjBotIsInTaintedCoreChainTrigger::IsActiveInEncounter()
{
    // By leewheel 2026-09-28 合并 brighton 219cd311（vashj 传核链重做）：整体改用上游 VashjCoreChain 判定。
    //   我方上一版"首二传包手就位"早退块（击杀者之外的包手提前就位 + 就位前不移动）已由上游
    //   catcher.prepositions（非击杀者 = 预先就位）+ readyDelay（urand(1000,2000) 放行）等价覆盖，
    //   语义不回退；上游 TaintedLogGenerators 临时调试调用保留。规则第 97 条：lady vashj = 21212。
    // End By leewheel
    VashjCoreChain const* chain = GetVashjCoreChain(bot);
    if (!chain || chain->failed)
        return false;

    int8 const index = GetVashjCoreCatcherIndex(*chain, bot);
    if (index < 0 && chain->start != bot->GetGUID())
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "21212");
    if (!vashj || GetLadyVashjPhase(vashj) != 2)
        return false;

    TaintedLogGenerators(bot); // TEMP LOG

    if (HasTaintedCore(bot))
        return true;

    return index >= 0 && IsVashjCoreCatcherActive(bot, *chain, index);
}

bool LadyVashjPetShouldSwitchTargetTrigger::IsActiveInEncounter()
{
    Guardian* pet = bot->GetGuardianPet();
    if (!pet || !pet->IsAlive() || pet->HasReactState(REACT_PASSIVE))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "21212");
    if (!vashj)
        return false;

    int8 const phase = GetLadyVashjPhase(vashj);
    if (phase != 2 && phase != 3)
        return false;

    if (Unit* target = GetVashjPetTarget(botAI, pet, vashj))
        return pet->GetVictim() != target;

    // Nothing worth attacking, so only a pet still on an immune Vashj needs calling back
    return pet->GetVictim() == vashj;
}

// Bots going after Sporebats sometimes walk up into the air, or end up on the pipes above the
// dais. A bot never falls on its own, so it stays up there.
bool LadyVashjBotIsAboveTheGroundTrigger::IsActiveInEncounter()
{
// By leewheel 2026-09-26 合并brighton: 采纳"高于地面高度"判定+BotIsInToxicSpores/MeleeNearToxicSpores新增触发器, vashj查找entry化(21212)
    Unit* vashj = AI_VALUE2(Unit*, "find target", "21212");
    if (!vashj || GetLadyVashjPhase(vashj) != 3)
        return false;

    // Search down from the dais, not from the bot, so a bot on the pipes still reads as high
    float const floorZ = bot->GetMapHeight(
        bot->GetPositionX(), bot->GetPositionY(), VASHJ_PLATFORM_CENTER_POSITION.GetPositionZ());
    return floorZ > INVALID_HEIGHT && bot->GetPositionZ() - floorZ > 1.5f;
}

// Melee dps have their own trigger, below.
bool LadyVashjBotIsInToxicSporesTrigger::IsActiveInEncounter()
{
    // By leewheel 2026-09-29 合并brighton 07e47c61: IsVashjRingMelee 改为 (bot, vashj) 签名
    //   并并入相位判定；保留 rule81 entry 化 21212。
    Unit* vashj = AI_VALUE2(Unit*, "find target", "21212");
    if (!vashj || GetLadyVashjPhase(vashj) != 3 || IsVashjRingMelee(bot, vashj))
        return false;

    bool const tanking = vashj->GetVictim() == bot;

    // Shielded bots walk through on their way; her tank's radius is for the melee behind her
    if (!tanking && bot->isMoving() && CanWalkThroughToxicSpores(bot))
        return false;

    float const radius = tanking ? TOXIC_SPORES_TANK_AVOID_RADIUS : TOXIC_SPORES_AVOID_RADIUS;
    return IsNearToxicSpores(botAI, bot, radius);
}

bool LadyVashjMeleeNearToxicSporesTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot) || PlayerbotAI::IsTank(bot))
        return false;

    // By leewheel 2026-09-29 合并brighton 07e47c61: 上游新增 IsVashjRingMelee 前置
    //   （仅近战圈成员触发）；保留 rule81 entry 化 21212。
    Unit* vashj = AI_VALUE2(Unit*, "find target", "21212");
    return vashj && GetLadyVashjPhase(vashj) == 3 && IsVashjRingMelee(bot, vashj) &&
        IsNearToxicSpores(botAI, bot, TOXIC_SPORES_MELEE_CONTROL_RADIUS);
    // End By leewheel
}

bool LadyVashjRangedReachBlockedByToxicSporesTrigger::IsActiveInEncounter()
{
    // By leewheel 2026-09-29 合并brighton 07e47c61: 按规则第 97 条 entry 化 = 21212
    Unit* vashj = AI_VALUE2(Unit*, "find target", "21212");
    if (!vashj || GetLadyVashjPhase(vashj) != 3)
        return false;

    Unit* target;
    float range;
    return GetVashjReachBlockedBySpores(botAI, bot, target, range);
}

bool LadyVashjEntangleOnMeleeTrigger::IsActiveInEncounter()
{
// By leewheel 2026-09-26 合并brighton: 采纳Paladin门槛+phase1/3允许(与注释一致), vashj查找entry化(21212)
    if (bot->getClass() != CLASS_PALADIN)
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "21212");
    if (!vashj)
        return false;

    int8 const phase = GetLadyVashjPhase(vashj);
    if (phase != 1 && phase != 3)
        return false;
    // End By leewheel

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // In phase 1 only a melee holding Static Charge needs freeing, and never her target, who
    // doesn't move for it. The stock Hand of Freedom takes the nearest rooted member otherwise.
    Unit* vashjVictim = vashj->GetVictim();
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->HasAura(Id(SscSpells::SPELL_ENTANGLE)))
            continue;

        if (phase == 1 && (member == vashjVictim || !HasVashjStaticCharge(member)))
            continue;

        if (PlayerbotAI::IsMelee(member))
            return true;
    }

    return false;
}

bool LadyVashjRogueHasStaticChargeTrigger::IsActiveInEncounter()
{
    return bot->getClass() == CLASS_ROGUE && HasVashjStaticCharge(bot);
}
