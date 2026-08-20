#include "VanillaNaxxMultipliers.h"

#include "ChooseTargetActions.h"
#include "DKActions.h"
#include "DruidActions.h"
#include "DruidBearActions.h"
#include "FollowActions.h"
#include "PetsAction.h" // By leewheel 2026-08-21: 上游删除 GenericActions 重构, PetAttackAction 迁移至 PetsAction //End By leewheel
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "MovementActions.h"
#include "PaladinActions.h"
#include "PriestActions.h"
#include "VanillaNaxxActions.h"
#include "VanillaNaxxSpellIds.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "ScriptedCreature.h"
#include "ShamanActions.h"
#include "Spell.h"
#include "UseMeetingStoneAction.h"
#include "WarriorActions.h"

float VanillaGrobbulusMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "grobbulus");
    if (!boss)
        return 1.0f;

    if (dynamic_cast<AvoidAoeAction*>(action))
        return botAI->IsMainTank(bot) ? 0.0f : 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action))
        return 0.0f;

    return 1.0f;
}

float VanillaLoathebGenericMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "loatheb");
    if (!boss)
        return 1.0f;

    context->GetValue<bool>("neglect threat")->Set(true);
    if (botAI->GetState() == BOT_STATE_COMBAT &&
        (dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<TankAssistAction*>(action) ||
         dynamic_cast<CastDebuffSpellOnAttackerAction*>(action) || dynamic_cast<FleeAction*>(action) ||
         dynamic_cast<CombatFormationMoveAction*>(action)))
    {
        return 0.0f;
    }
    if (!dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    // Vanilla Naxx Loatheb uses Corrupted Mind (29201) instead of Necrotic Aura (55593)
    // Check for both for safety, plus name-based fallback
    Aura* aura = VanillaNaxxSpellIds::GetAnyAura(bot, {VanillaNaxxSpellIds::CorruptedMind, VanillaNaxxSpellIds::NecroticAura});
    if (!aura)
    {
        // Fallback to name for custom spell data
        aura = botAI->GetAura("corrupted mind", bot);
        if (!aura)
            aura = botAI->GetAura("necrotic aura", bot);
    }
    if (!aura || aura->GetDuration() <= 1500)
        return 1.0f;

    return 0.0f;
}

float VanillaThaddiusGenericMultiplier::GetValue(Action* action)
{
    if (!helper.UpdateBossAI())
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action))
        return 0.0f;
    // pet phase
    if (helper.IsPhasePet() &&
        (dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<TankAssistAction*>(action) ||
         dynamic_cast<CastDebuffSpellOnAttackerAction*>(action) ||
         dynamic_cast<ReachPartyMemberToHealAction*>(action) || dynamic_cast<BuffOnMainTankAction*>(action)))
    {
        return 0.0f;
    }
    // die at the same time
    Unit* target = AI_VALUE(Unit*, "current target");
    Unit* feugen = AI_VALUE2(Unit*, "find target", "feugen");
    Unit* stalagg = AI_VALUE2(Unit*, "find target", "stalagg");
    if (helper.IsPhasePet() && target && feugen && stalagg && target->GetHealthPct() <= 40 &&
        (feugen->GetHealthPct() >= target->GetHealthPct() + 3 || stalagg->GetHealthPct() >= target->GetHealthPct() + 3))
    {
        if (dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<CastHealingSpellAction*>(action))
            return 0.0f;
    }
    return 1.0f;
}

float VanillaSapphironGenericMultiplier::GetValue(Action* action)
{
    if (!helper.UpdateBossAI())
        return 1.0f;

    if (dynamic_cast<CastDeathGripAction*>(action) || dynamic_cast<CombatFormationMoveAction*>(action))
        return 0.0f;

    return 1.0f;
}

float VanillaInstructorRazuviousGenericMultiplier::GetValue(Action* action)
{
    if (!helper.UpdateBossAI())
        return 1.0f;

    context->GetValue<bool>("neglect threat")->Set(true);
    if (botAI->GetState() == BOT_STATE_COMBAT &&
        (dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<TankAssistAction*>(action) ||
         dynamic_cast<CastTauntAction*>(action) || dynamic_cast<CastDarkCommandAction*>(action) ||
         dynamic_cast<CastHandOfReckoningAction*>(action) || dynamic_cast<CastGrowlAction*>(action)))
    {
        return 0.0f;
    }
    return 1.0f;
}

float VanillaKelthuzadGenericMultiplier::GetValue(Action* action)
{
    if (!helper.UpdateBossAI())
        return 1.0f;

    if ((dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<TankAssistAction*>(action) ||
         dynamic_cast<CastDebuffSpellOnAttackerAction*>(action) || dynamic_cast<FleeAction*>(action)))
    {
        return 0.0f;
    }
    if (helper.IsPhaseOne())
    {
        if (dynamic_cast<CastTotemAction*>(action) || dynamic_cast<CastShadowfiendAction*>(action) ||
            dynamic_cast<CastRaiseDeadAction*>(action) || dynamic_cast<CastFeignDeathAction*>(action) ||
            dynamic_cast<CastInvisibilityAction*>(action) || dynamic_cast<CastVanishAction*>(action) ||
            dynamic_cast<PetAttackAction*>(action))
        {
            return 0.0f;
        }
    }
    if (helper.IsPhaseTwo())
    {
        if (dynamic_cast<CastBlizzardAction*>(action) || dynamic_cast<CastFrostNovaAction*>(action))
            return 0.0f;

    }
    return 1.0f;
}

float VanillaAnubrekhanGenericMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "anub'rekhan");
    if (!boss)
        return 1.0f;

    if (VanillaNaxxSpellIds::HasAnyAura(boss, {VanillaNaxxSpellIds::LocustSwarm}) ||
        botAI->HasAura("locust swarm", boss))
    {
        if (dynamic_cast<FleeAction*>(action))
            return 0.0f;
    }
    return 1.0f;
}

float VanillaFourHorsemenGenericMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sir zeliek");
    if (!boss)
        return 1.0f;

    context->GetValue<bool>("neglect threat")->Set(true);
    if ((dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<TankAssistAction*>(action)))
        return 0.0f;

    return 1.0f;
}

float VanillaGluthGenericMultiplier::GetValue(Action* action)
{
    if (!helper.UpdateBossAI())
        return 1.0f;

    if ((dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<TankAssistAction*>(action) ||
         dynamic_cast<FleeAction*>(action) || dynamic_cast<CastDebuffSpellOnAttackerAction*>(action) ||
         dynamic_cast<CastStarfallAction*>(action)))
    {
        return 0.0f;
    }

    if (botAI->IsMainTank(bot))
    {
        Aura* aura = VanillaNaxxSpellIds::GetAnyAura(bot, {VanillaNaxxSpellIds::MortalWound});
        if (!aura)
        {
            aura = botAI->GetAura("mortal wound", bot, false, true);
        }
        if (aura && aura->GetStackAmount() >= 5)
        {
            if (dynamic_cast<CastTauntAction*>(action) || dynamic_cast<CastDarkCommandAction*>(action) ||
                dynamic_cast<CastHandOfReckoningAction*>(action) || dynamic_cast<CastGrowlAction*>(action))
            {
                return 0.0f;
            }
        }
    }
    if (dynamic_cast<PetAttackAction*>(action))
    {
        Unit* target = AI_VALUE(Unit*, "current target");
        if (helper.IsZombieChow(target))
            return 0.0f;
    }
    return 1.0f;
}
