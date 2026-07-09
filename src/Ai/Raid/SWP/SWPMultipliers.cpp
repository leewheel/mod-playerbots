//By leewheel 2026-07-08
/*
 * 太阳之井高地 (Sunwell Plateau) 乘数器实现
 * 作者: leewheel
 * 对照 BT 乘数器模式实现
 */
//End By leewheel

#include "SWPMultipliers.h"

#include "ChooseTargetActions.h"
#include "DKActions.h"
#include "DruidActions.h"
#include "DruidBearActions.h"
#include "DruidShapeshiftActions.h"
#include "FollowActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "PaladinActions.h"
#include "PriestActions.h"
#include "SWPActions.h"
#include "SWPHelpers.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "ShamanActions.h"
#include "WarlockActions.h"
#include "WarriorActions.h"
#include "WipeAction.h"

using namespace SunwellPlateauHelpers;

// 判断是否为DPS爆发技能（参考BT模式）
static bool IsDpsCooldownAction(Action* action)
{
    return dynamic_cast<CastHeroismAction*>(action) ||
           dynamic_cast<CastBloodlustAction*>(action) ||
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
           dynamic_cast<CastBloodFuryAction*>(action);
}

// ===== 卡雷苟斯 (Kalecgos) =====

float KalecgosDelayDpsCooldownsMultiplier::GetValue(Action* action)
{
    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    if (!kalecgos || kalecgos->GetHealthPct() < 95.0f)
        return 1.0f;

    if (IsDpsCooldownAction(action) ||
        (botAI->IsDps(bot) && dynamic_cast<UseTrinketAction*>(action)))
    {
        return 0.0f;
    }

    return 1.0f;
}

float KalecgosControlMovementMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "kalecgos"))
        return 1.0f;

    // 禁止默认阵型移动，由SWP策略接管
    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
    {
        return 0.0f;
    }

    if (dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

// ===== 布鲁塔卢斯 (Brutallus) =====

float BrutallusDelayDpsCooldownsMultiplier::GetValue(Action* action)
{
    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    if (!brutallus || brutallus->GetHealthPct() < 95.0f)
        return 1.0f;

    if (IsDpsCooldownAction(action) ||
        (botAI->IsDps(bot) && dynamic_cast<UseTrinketAction*>(action)))
    {
        return 0.0f;
    }

    return 1.0f;
}

float BrutallusControlMovementMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "brutallus"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
    {
        return 0.0f;
    }

    if (dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

// ===== 菲米丝 (Felmyst) =====

float FelmystDelayDpsCooldownsMultiplier::GetValue(Action* action)
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || felmyst->GetHealthPct() < 95.0f)
        return 1.0f;

    if (IsDpsCooldownAction(action) ||
        (botAI->IsDps(bot) && dynamic_cast<UseTrinketAction*>(action)))
    {
        return 0.0f;
    }

    return 1.0f;
}

float FelmystControlMovementMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "felmyst"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
    {
        return 0.0f;
    }

    if (dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

// ===== 艾瑞达双子 (Eredar Twins) =====

float EredarTwinsDelayDpsCooldownsMultiplier::GetValue(Action* action)
{
    Unit* sacrolash = AI_VALUE2(Unit*, "find target", "lady sacrolash");
    Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
    if (!sacrolash && !alythess)
        return 1.0f;

    // 双子存活时延迟爆发，等一个倒下后再开
    if (sacrolash && alythess)
    {
        float healthPct = std::min(sacrolash->GetHealthPct(), alythess->GetHealthPct());
        if (healthPct > 90.0f)
        {
            if (IsDpsCooldownAction(action) ||
                (botAI->IsDps(bot) && dynamic_cast<UseTrinketAction*>(action)))
            {
                return 0.0f;
            }
        }
    }

    return 1.0f;
}

float EredarTwinsControlMovementMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "lady sacrolash") &&
        !AI_VALUE2(Unit*, "find target", "grand warlock alythess"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
    {
        return 0.0f;
    }

    if (dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

// ===== 穆鲁 (Muru) =====

float MuruDelayDpsCooldownsMultiplier::GetValue(Action* action)
{
    Unit* muru = AI_VALUE2(Unit*, "find target", "muru");
    if (muru && muru->GetHealthPct() > 95.0f)
    {
        if (IsDpsCooldownAction(action) ||
            (botAI->IsDps(bot) && dynamic_cast<UseTrinketAction*>(action)))
        {
            return 0.0f;
        }
    }

    // 恩特罗皮乌斯阶段可以开爆发
    Unit* entropius = AI_VALUE2(Unit*, "find target", "entropius");
    if (entropius && entropius->GetHealthPct() > 95.0f)
    {
        if (IsDpsCooldownAction(action) ||
            (botAI->IsDps(bot) && dynamic_cast<UseTrinketAction*>(action)))
        {
            return 0.0f;
        }
    }

    return 1.0f;
}

float MuruControlMovementMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "muru") &&
        !AI_VALUE2(Unit*, "find target", "entropius"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
    {
        return 0.0f;
    }

    if (dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

// ===== 基尔加丹 (Kil'jaeden) =====

float KiljaedenDelayDpsCooldownsMultiplier::GetValue(Action* action)
{
    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden)
        return 1.0f;

    // 基尔加丹25%以下时全力输出
    if (kiljaeden->GetHealthPct() <= 25.0f)
        return 1.0f;

    if (kiljaeden->GetHealthPct() > 95.0f)
    {
        if (IsDpsCooldownAction(action) ||
            (botAI->IsDps(bot) && dynamic_cast<UseTrinketAction*>(action)))
        {
            return 0.0f;
        }
    }

    return 1.0f;
}

float KiljaedenControlMovementMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "kil'jaeden"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
    {
        return 0.0f;
    }

    if (dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}
