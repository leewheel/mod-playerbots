/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// === 外部代码引入记录 ===
// 2026-07-30 引入自 brighton-chi/mod-playerbots:
//   commit 2c578c7450a1185d540e4b829386ea596732e48a - TK static role members (botAI->IsXxx → PlayerbotAI::IsXxx)
// By leewheel
// End By leewheel

//By leewheel 2026-07-28 - 同步上游brighton-chi/mod-playerbots，按副本审核优化

#include "TKMultipliers.h"
#include "ChooseTargetActions.h"
#include "DKActions.h"
#include "DruidActions.h"
#include "DruidBearActions.h"
#include "EquipAction.h"
#include "FollowActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "PaladinActions.h"
#include "Playerbots.h"
#include "RogueActions.h"
#include "ShamanActions.h"
#include "TKActions.h"
#include "TKHelpers.h"
#include "TKKaelthasBossAI.h"
#include "WarlockActions.h"
#include "WarriorActions.h"

namespace
{

//By leewheel 2026-07-28 - 提取单目标嘲讽判断为公共辅助函数，避免重复代码
bool IsSingleTargetTaunt(Action* action)
{
    return dynamic_cast<CastTauntAction*>(action) ||
        dynamic_cast<CastGrowlAction*>(action) ||
        dynamic_cast<CastHandOfReckoningAction*>(action) ||
        dynamic_cast<CastDarkCommandAction*>(action) ||
        dynamic_cast<CastDeathGripAction*>(action);
}

}

// Al'ar <Phoenix God>

//By leewheel 2026-07-28 - 优化判断顺序：先过滤action类型，再查alar状态，减少不必要的AI_VALUE查询
float AlarMoveBetweenPlatformsMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<TankFaceAction*>(action) &&
        !dynamic_cast<CastKillingSpreeAction*>(action) &&
        !dynamic_cast<CastDisengageAction*>(action) &&
        !dynamic_cast<CastBlinkBackAction*>(action) &&
        !dynamic_cast<ReachTargetAction*>(action))
    {
        return 1.0f;
    }

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (alar && !isAlarInPhase2[alar->GetMap()->GetInstanceId()])
        return 0.0f;

    return 1.0f;
}

//By leewheel 2026-07-28 - 拆分Follow和Flee判断；phase2前禁止跟随，phase2中非战斗状态也禁止跟随
//By leewheel 2026-07-29 - 同步上游brighton-chi 4f20bd10：反转判断顺序，先做 cheap dynamic_cast 再查 alar
float AlarDisableDisperseMultiplier::GetValue(Action* action)
{
    bool const isCombatMoveBlocked = dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<TankFaceAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action);

    if (isCombatMoveBlocked || dynamic_cast<FleeAction*>(action))
        return AI_VALUE2(Unit*, "find target", "al'ar") ? 0.0f : 1.0f;

    if (!dynamic_cast<FollowAction*>(action))
        return 1.0f;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return 1.0f;

    if (!isAlarInPhase2[alar->GetMap()->GetInstanceId()])
        return 1.0f;

    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    return 0.0f;
}

//By leewheel 2026-07-28 - 重命名并优化：先判断非战斗状态和action类型，最后才查alar，减少查询
float AlarDisableAutomaticTargetingMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<TankAssistAction*>(action) && !dynamic_cast<DpsAssistAction*>(action))
        return 1.0f;

    if (AI_VALUE2(Unit*, "find target", "al'ar"))
        return 0.0f;

    return 1.0f;
}

//By leewheel 2026-07-28 - 重写远离复生逻辑：ranged/tank检查PASSIVE状态，近战检查血量>5%
//By leewheel 2026-07-29 - 同步上游brighton-chi 4f20bd10：先做 dynamic_cast 早返回，再查 alar；保留老大 ranged/tank PASSIVE + 近战血量检查
float AlarStayAwayFromRebirthMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) ||
        dynamic_cast<AlarMoveAwayFromRebirthAction*>(action))
    {
        return 1.0f;
    }

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar || isAlarInPhase2[alar->GetMap()->GetInstanceId()])
        return 1.0f;

    if (PlayerbotAI::IsRanged(bot) || PlayerbotAI::IsTank(bot))
    {
        Creature* alarCreature = alar->ToCreature();
        if (!alarCreature || alarCreature->GetReactState() != REACT_PASSIVE)
            return 1.0f;
    }
    else if (alar->GetHealthPct() > 5.0f)
    {
        return 1.0f;
    }

    return 0.0f;
}

//By leewheel 2026-07-28 - 使用IsSingleTargetTaunt辅助函数；优化判断顺序先过滤action再查状态
float AlarDontTauntBossIfArmorMeltedMultiplier::GetValue(Action* action)
{
    if (!IsSingleTargetTaunt(action))
        return 1.0f;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar || AI_VALUE(Unit*, "current target") != alar)
        return 1.0f;

    if (bot->HasAura(static_cast<uint32>(TkSpells::SPELL_MELT_ARMOR)))
        return 0.0f;

    return 1.0f;
}

// Void Reaver

//By leewheel 2026-07-29 - 同步上游brighton-chi 4f20bd10：先做 dynamic_cast 早返回，再查 void reaver
float VoidReaverMaintainPositionsMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<SetBehindTargetAction*>(action))
    {
        return 1.0f;
    }

    if (AI_VALUE2(Unit*, "find target", "void reaver"))
        return 0.0f;

    return 1.0f;
}

// High Astromancer Solarian

//By leewheel 2026-07-28 - 重写星术师站位逻辑：拆分各action类型的独立判断
//By leewheel 2026-07-29 - 同步上游brighton-chi 4f20bd10：拆分为 isCombatMoveBlocked/isBlinkDisengage/isNonHunterFlee 三组标志，
//                          战斗移动/Blink/Flee 走 fast-path，Reach spell 与 other movement 走 slow-path
float HighAstromancerSolarianMaintainPositionMultiplier::GetValue(Action* action)
{
    bool const isCombatMoveBlocked = dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action);
    bool const isBlinkDisengage = dynamic_cast<CastBlinkBackAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action);
    bool const isNonHunterFlee = bot->getClass() != CLASS_HUNTER &&
        dynamic_cast<FleeAction*>(action);

    if (isCombatMoveBlocked || isBlinkDisengage || isNonHunterFlee)
    {
        Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
        if (astromancer && !astromancer->HasAura(
                static_cast<uint32>(TkSpells::SPELL_SOLARIAN_TRANSFORM)))
        {
            return 0.0f;
        }
        return 1.0f;
    }

    bool const isReachSpell = dynamic_cast<CastReachTargetSpellAction*>(action);
    bool const isOtherMovement = dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<AttackAction*>(action) &&
        !dynamic_cast<HighAstromancerSolarianMoveAwayFromGroupAction*>(action);

    if (!isReachSpell && !isOtherMovement)
        return 1.0f;

    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!astromancer || astromancer->HasAura(
            static_cast<uint32>(TkSpells::SPELL_SOLARIAN_TRANSFORM)))
    {
        return 1.0f;
    }

    if (!HasWrathOfTheAstromancer(bot))
        return 1.0f;

    return 0.0f;
}

//By leewheel 2026-07-28 - 新增：星术师阶段禁止近战自动选目标（主坦除外且需PASSIVE状态）
//By leewheel 2026-07-29 - 同步上游brighton-chi 4f20bd10：把 TankAssist/DpsAssist 早返回移到 astromancer 查询之前
float HighAstromancerSolarianDisableMeleeTargetingMultiplier::GetValue(Action* action)
{
    if (!PlayerbotAI::IsMelee(bot))
        return 1.0f;

    if (!dynamic_cast<TankAssistAction*>(action) && !dynamic_cast<DpsAssistAction*>(action))
        return 1.0f;

    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!astromancer)
        return 1.0f;

    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (PlayerbotAI::IsMainTank(bot))
    {
        Creature* astromancerCreature = astromancer->ToCreature();
        if (astromancerCreature && astromancerCreature->GetReactState() != REACT_PASSIVE)
            return 0.0f;

        return 1.0f;
    }

    if (AI_VALUE2(Unit*, "find target", "solarium priest"))
        return 0.0f;

    return 1.0f;
}

// Kael'thas Sunstrider <Lord of the Blood Elves>

//By leewheel 2026-07-28 - 优化DPS等待逻辑：使用sentinel(-1)表示满血不计时，提取变量减少重复调用
//By leewheel 2026-07-29 - 同步上游brighton-chi 4f20bd10：先做 Misdirect/Heal/Attack-CastSpell 早返回，再查 kaelthas
float KaelthasSunstriderWaitForDpsMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<KaelthasSunstriderMisdirectAdvisorsToTanksAction*>(action))
        return 1.0f;

    if (dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    if (!dynamic_cast<AttackAction*>(action) &&
        !dynamic_cast<CastSpellAction*>(action))
    {
        return 1.0f;
    }

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI || kaelAI->GetPhase() != PHASE_SINGLE_ADVISOR)
        return 1.0f;

    time_t const now = std::time(nullptr);
    constexpr uint8 dpsWaitSeconds = 10;

    auto it = advisorDpsWaitTimer.find(kaelthas->GetMap()->GetInstanceId());
    if (it != advisorDpsWaitTimer.end())
    {
        //By leewheel 2026-07-28 - sentinel(-1)表示顾问满血不计倒计时；真实时间戳检查是否已过等待期
        if (it->second != -1 && (now - it->second) >= dpsWaitSeconds)
            return 1.0f;
    }

    Unit* sanguinar = AI_VALUE2(Unit*, "find target", "lord sanguinar");
    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    Unit* telonicus = AI_VALUE2(Unit*, "find target", "master engineer telonicus");

    auto isAdvisorActive = [](Unit* advisor)
    {
        return advisor && !advisor->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
            !IsFeigningDeath(advisor);
    };

    //By leewheel 2026-07-28 - 提取角色判断变量避免重复调用
    bool isMainTank = PlayerbotAI::IsMainTank(bot);
    bool isFirstAssistTank = PlayerbotAI::IsAssistTankOfIndex(bot, 0, false);
    bool isWarlockTank = GetCapernianTank(bot) == bot;

    if ((isAdvisorActive(sanguinar) && isMainTank) ||
        (isAdvisorActive(telonicus) && isFirstAssistTank) ||
        (isAdvisorActive(capernian) && (isMainTank || isWarlockTank)))
    {
        return 1.0f;
    }

    bool shouldHoldDps =
        (isAdvisorActive(sanguinar) && !isMainTank) ||
        (isAdvisorActive(telonicus) && !isFirstAssistTank) ||
        (isAdvisorActive(capernian) && !isMainTank && !isWarlockTank);

    if (shouldHoldDps)
        return 0.0f;

    return 1.0f;
}

//By leewheel 2026-07-28 - 删除SPELL_PERMANENT_FEIGN_DEATH检查，改用IsFeigningDeath
//By leewheel 2026-07-29 - 同步上游brighton-chi 4f20bd10：先做 MovementAction/Kite 早返回，再查 kaelthas
float KaelthasSunstriderKiteThaladredMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) ||
        dynamic_cast<KaelthasSunstriderKiteThaladredAction*>(action))
    {
        return 1.0f;
    }

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI)
        return 1.0f;

    if (PlayerbotAI::IsTank(bot) && kaelAI->GetPhase() == PHASE_ALL_ADVISORS)
        return 1.0f;

    Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladred the darkener");
    if (!thaladred || thaladred->GetVictim() != bot)
        return 1.0f;

    return 0.0f;
}

float KaelthasSunstriderControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI || kaelAI->GetPhase() == PHASE_FINAL)
        return 1.0f;

    if (dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 0.0f;

    return 1.0f;
}

//By leewheel 2026-07-28 - 用IsFeigningDeath替代SPELL_PERMANENT_FEIGN_DEATH；新增CastReachTargetSpellAction禁用
//By leewheel 2026-07-29 - 同步上游brighton-chi 4f20bd10：先做 ReachSpell/BlockedMovement 早返回，再查 kaelthas
float KaelthasSunstriderKeepDistanceFromCapernianMultiplier::GetValue(Action* action)
{
    bool const isReachSpell = dynamic_cast<CastReachTargetSpellAction*>(action);
    bool const isBlockedMovement = dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<AttackAction*>(action) &&
        !dynamic_cast<KaelthasSunstriderSpreadAndMoveAwayFromCapernianAction*>(action);

    if (!isReachSpell && !isBlockedMovement)
        return 1.0f;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI || kaelAI->GetPhase() != PHASE_SINGLE_ADVISOR)
        return 1.0f;

    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    if (!capernian || capernian->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) ||
        IsFeigningDeath(capernian))
    {
        return 1.0f;
    }

    return 0.0f;
}

//By leewheel 2026-07-28 - 只对主坦生效；用IsSingleTargetTaunt替代逐个判断；CastSwipeBearAction替代CastSwipeAction
float KaelthasSunstriderManageWeaponTankingMultiplier::GetValue(Action* action)
{
    if (!PlayerbotAI::IsMainTank(bot))
        return 1.0f;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI || kaelAI->GetPhase() != PHASE_WEAPONS)
        return 1.0f;

    if (kaelAI->GetPhase() != PHASE_WEAPONS &&
        dynamic_cast<TankFaceAction*>(action))
        return 0.0f;

    if (!botAI->IsMainTank(bot))
        return 1.0f;

    // Try to keep main tank from grabbing aggro on any weapon other than the axe
    if (kaelAI->GetPhase() == PHASE_WEAPONS &&
        (dynamic_cast<TankAssistAction*>(action) ||
         dynamic_cast<CastTauntAction*>(action) ||
         dynamic_cast<CastChallengingShoutAction*>(action) ||
         dynamic_cast<CastThunderClapAction*>(action) ||
         dynamic_cast<CastShockwaveAction*>(action) ||
         dynamic_cast<CastCleaveAction*>(action) ||
         dynamic_cast<CastGrowlAction*>(action) ||
         dynamic_cast<CastSwipeBearAction*>(action) ||
         dynamic_cast<CastChallengingRoarAction*>(action) ||
         dynamic_cast<CastHandOfReckoningAction*>(action) ||
         dynamic_cast<CastAvengersShieldAction*>(action) ||
         dynamic_cast<CastConsecrationAction*>(action) ||
         dynamic_cast<CastDarkCommandAction*>(action) ||
         dynamic_cast<CastDeathAndDecayAction*>(action) ||
         dynamic_cast<CastPestilenceAction*>(action) ||
         dynamic_cast<CastBloodBoilAction*>(action)))
        return 0.0f;

    return 1.0f;
}

//By leewheel 2026-07-28 - 优化判断顺序：先过滤action类型再查目标
float KaelthasSunstriderSuppressEquipUpgradeMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<EquipUpgradeAction*>(action) &&
        !dynamic_cast<EquipUpgradesPacketAction*>(action))
    {
        return 1.0f;
    }

    if (AI_VALUE2(Unit*, "find target", "kael'thas sunstrider"))
        return 0.0f;

    return 1.0f;
}

//By leewheel 2026-07-28 - 新增：凯子阶段自动选目标管理（DPS全部禁用，主坦/顾问阶段副坦禁用）
//By leewheel 2026-07-29 - 同步上游brighton-chi 4f20bd10：先做 DpsAssist/TankAssist 早返回，再查 kaelthas
float KaelthasSunstriderManageAutomaticTargetingMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<DpsAssistAction*>(action) &&
        !dynamic_cast<TankAssistAction*>(action))
    {
        return 1.0f;
    }

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI)
        return 1.0f;

    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (dynamic_cast<DpsAssistAction*>(action))
        return 0.0f;

    //By leewheel 2026-07-29 - TankAssist 分支：主坦/顾问阶段副坦都禁用自动选目标
    if (PlayerbotAI::IsMainTank(bot))
        return 0.0f;

    if (kaelAI->GetPhase() == PHASE_SINGLE_ADVISOR ||
        kaelAI->GetPhase() == PHASE_ALL_ADVISORS)
    {
        return 0.0f;
    }

    return 1.0f;
}

//By leewheel 2026-07-28 - 新增武器阶段TankFace禁用逻辑
//By leewheel 2026-07-29 - 同步上游brighton-chi 4f20bd10：先做 isCombatMoveBlocked 早返回，再做 TankFace 早返回，最后才查 kaelthas
float KaelthasSunstriderDisableDisperseMultiplier::GetValue(Action* action)
{
    bool const isCombatMoveBlocked = dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<TankFaceAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action);

    if (isCombatMoveBlocked)
        return AI_VALUE2(Unit*, "find target", "kael'thas sunstrider") ? 0.0f : 1.0f;

    if (!dynamic_cast<TankFaceAction*>(action))
        return 1.0f;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI || kaelAI->GetPhase() == PHASE_WEAPONS)
        return 1.0f;

    return 0.0f;
}

//By leewheel 2026-07-28 - 新增：阶段3准备（顾问复活/凯尔对话阶段，指定角色移动就位）
//By leewheel 2026-07-29 - 同步上游brighton-chi 4f20bd10：先做 MovementAction/HandleAdvisorRoles 早返回，再查 kaelthas
float KaelthasSunstriderPrepareForPhase3Multiplier::GetValue(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) ||
        dynamic_cast<KaelthasSunstriderHandleAdvisorRolesInPhase3Action*>(action))
    {
        return 1.0f;
    }

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI || kaelAI->GetPhase() != PHASE_ALL_ADVISORS)
        return 1.0f;

    //By leewheel 2026-07-28 - 用顾问不可选中状态作为复活/对话阶段的代理判断
    Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladred the darkener");
    if (!thaladred || !thaladred->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
        return 1.0f;

    if (PlayerbotAI::IsMainTank(bot) ||
        PlayerbotAI::IsAssistTankOfIndex(bot, 0, false) ||
        PlayerbotAI::IsAssistHealOfIndex(bot, 0, false) ||
        (bot->getClass() == CLASS_WARLOCK && GetCapernianTank(bot) == bot))
    {
        return 0.0f;
    }

    return 1.0f;
}

// Bloodlust/Heroism and other major cooldowns should be used at the start of Phase 3
//By leewheel 2026-07-28 - 拆分DPS条件，UseTrinketAction单独判断需IsDps
//By leewheel 2026-07-29 - 同步上游brighton-chi 4f20bd10：先做 isCooldown 标志计算与早返回，再查 kaelthas 阶段
float KaelthasSunstriderDelayCooldownsMultiplier::GetValue(Action* action)
{
    bool const isCooldown =
        (bot->getClass() == CLASS_SHAMAN &&
         (dynamic_cast<CastBloodlustAction*>(action) ||
          dynamic_cast<CastHeroismAction*>(action))) ||
        dynamic_cast<CastMetamorphosisAction*>(action) ||
        dynamic_cast<CastAdrenalineRushAction*>(action) ||
        dynamic_cast<CastBladeFlurryAction*>(action) ||
        dynamic_cast<CastIcyVeinsAction*>(action) ||
        dynamic_cast<CastColdSnapAction*>(action) ||
        dynamic_cast<CastArcanePowerAction*>(action) ||
        dynamic_cast<CastPresenceOfMindAction*>(action) ||
        dynamic_cast<CastCombustionAction*>(action) ||
        dynamic_cast<CastRapidFireAction*>(action) ||
        dynamic_cast<CastReadinessAction*>(action) ||
        dynamic_cast<CastAvengingWrathAction*>(action) ||
        dynamic_cast<CastElementalMasteryAction*>(action) ||
        dynamic_cast<CastFeralSpiritAction*>(action) ||
        dynamic_cast<CastFireElementalTotemAction*>(action) ||
        dynamic_cast<CastFireElementalTotemMeleeAction*>(action) ||
        dynamic_cast<CastForceOfNatureAction*>(action) ||
        dynamic_cast<CastArmyOfTheDeadAction*>(action) ||
        dynamic_cast<CastSummonGargoyleAction*>(action) ||
        dynamic_cast<CastBerserkingAction*>(action) ||
        dynamic_cast<CastBloodFuryAction*>(action) ||
        (PlayerbotAI::IsDps(bot) && dynamic_cast<UseTrinketAction*>(action));

    if (!isCooldown)
        return 1.0f;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI || kaelAI->GetPhase() == PHASE_ALL_ADVISORS || kaelAI->GetPhase() == PHASE_FINAL)
        return 1.0f;

    return 0.0f;
}

//By leewheel 2026-07-28 - 优化判断顺序：先放行攻击和空中分散action，再过滤非移动，最后检查重力失效
float KaelthasSunstriderStaySpreadDuringGravityLapseMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<KaelthasSunstriderSpreadOutInMidairAction*>(action))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action))
        return 1.0f;

    if (bot->HasAura(static_cast<uint32>(TkSpells::SPELL_GRAVITY_LAPSE)))
        return 0.0f;

    return 1.0f;
}
