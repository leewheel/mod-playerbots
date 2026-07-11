#include "VanillaNaxxTriggers.h"

#include "Playerbots.h"
#include "VanillaNaxxSpellIds.h"
#include "Timer.h"
#include "Trigger.h"

bool VanillaMutatingInjectionMeleeTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "grobbulus");
    if (!boss)
        return false;

    return VanillaMutatingInjectionTrigger::IsActive() && !botAI->IsRanged(bot);
}

bool VanillaMutatingInjectionRangedTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "grobbulus");
    if (!boss)
        return false;

    return VanillaMutatingInjectionTrigger::IsActive() && botAI->IsRanged(bot);
}

bool VanillaAuraRemovedTrigger::IsActive()
{
    bool check = botAI->HasAura(name, bot, false, false, -1, true);
    bool ret = false;
    if (prev_check && !check)
        ret = true;

    prev_check = check;
    return ret;
}

bool VanillaMutatingInjectionRemovedTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "grobbulus");
    if (!boss)
        return false;

    return HasNoAuraTrigger::IsActive() && botAI->GetState() == BOT_STATE_COMBAT && botAI->IsRanged(bot);
}

bool VanillaGrobbulusCloudTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "grobbulus");
    if (!boss)
        return false;

    if (!botAI->IsMainTank(bot))
        return false;

    if (!AI_VALUE2(bool, "has aggro", "boss target"))
        return false;

    uint32 now = getMSTime();
    bool poison_cloud_casting = false;
    if (boss->HasUnitState(UNIT_STATE_CASTING))
    {
        Spell* spell = boss->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        if (!spell)
            spell = boss->GetCurrentSpell(CURRENT_CHANNELED_SPELL);

        if (spell)
            poison_cloud_casting = VanillaNaxxSpellIds::MatchesAnySpellId(spell->GetSpellInfo(), {VanillaNaxxSpellIds::PoisonCloud});

    }
    if (!poison_cloud_casting && last_cloud_ms != 0 && now - last_cloud_ms < CloudRotationDelayMs)
        return false;

    last_cloud_ms = now;
    return true;
}

bool VanillaRazuviousTankTrigger::IsActive()
{
    // Vanilla Naxx40 uses RAID_DIFFICULTY_10MAN_HEROIC as difficulty.
    // In 40-man mode there are NO obedience crystals — priests must use
    // Mind Control on the Death Knight Understudies (same as WotLK 25-man).
    // Only RAID_DIFFICULTY_10MAN_NORMAL would use the tank+crystal path.
    Difficulty diff = bot->GetRaidDifficulty();
    if (diff == RAID_DIFFICULTY_10MAN_NORMAL)
        return helper.UpdateBossAI() && botAI->IsTank(bot);

    // 40-man (RAID_DIFFICULTY_10MAN_HEROIC) and 25-man: priests mind-control understudies
    return helper.UpdateBossAI() && bot->getClass() == CLASS_PRIEST;
}

bool VanillaRazuviousNontankTrigger::IsActive()
{
    // Mirror of VanillaRazuviousTankTrigger:
    // 40-man mode (RAID_DIFFICULTY_10MAN_HEROIC) uses priest mind-control,
    // so non-priests (not non-tanks) are the "nontank" group.
    Difficulty diff = bot->GetRaidDifficulty();
    if (diff == RAID_DIFFICULTY_10MAN_NORMAL)
        return helper.UpdateBossAI() && !(botAI->IsTank(bot));

    return helper.UpdateBossAI() && !(bot->getClass() == CLASS_PRIEST);
}

bool VanillaFourHorsemenAttractorsTrigger::IsActive()
{
    if (!helper.UpdateBossAI())
        return false;

    return helper.IsAttracter(bot);
}

bool VanillaFourHorsemenExceptAttractorsTrigger::IsActive()
{
    if (!helper.UpdateBossAI())
        return false;

    return !helper.IsAttracter(bot);
}

bool VanillaSapphironGroundTrigger::IsActive()
{
    if (!helper.UpdateBossAI())
        return false;

    return helper.IsPhaseGround();
}

bool VanillaSapphironFlightTrigger::IsActive()
{
    if (!helper.UpdateBossAI())
        return false;

    return helper.IsPhaseFlight();
}

bool VanillaGluthTrigger::IsActive() { return helper.UpdateBossAI(); }

bool VanillaGluthMainTankMortalWoundTrigger::IsActive()
{
    if (!helper.UpdateBossAI())
        return false;

    if (!botAI->IsAssistTankOfIndex(bot, 0))
        return false;

    Unit* mt = AI_VALUE(Unit*, "main tank");
    if (!mt)
        return false;

    Aura* aura = VanillaNaxxSpellIds::GetAnyAura(mt, {VanillaNaxxSpellIds::MortalWound});
    if (!aura)
    {
        aura = botAI->GetAura("mortal wound", mt, false, true);
    }
    if (!aura || aura->GetStackAmount() < 5)
        return false;

    return true;
}

bool VanillaKelthuzadTrigger::IsActive() { return helper.UpdateBossAI(); }

bool VanillaAnubrekhanTrigger::IsActive() {
    Unit* boss = AI_VALUE2(Unit*, "find target", "anub'rekhan");
    if (!boss)
        return false;

    return true;
}

bool VanillaFaerlinaTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "grand widow faerlina");
    if (!boss)
        return false;

    return true;
}

bool VanillaMaexxnaTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "maexxna");
    if (!boss)
        return false;

    return !botAI->IsTank(bot);
}

bool VanillaLoathebTrigger::IsActive() { return helper.UpdateBossAI(); }

bool VanillaThaddiusPhasePetTrigger::IsActive()
{
    if (!helper.UpdateBossAI())
        return false;

    return helper.IsPhasePet();
}

bool VanillaThaddiusPhaseTransitionTrigger::IsActive()
{
    if (!helper.UpdateBossAI())
        return false;

    return helper.IsPhaseTransition();
}

bool VanillaThaddiusPhaseThaddiusTrigger::IsActive()
{
    if (!helper.UpdateBossAI())
        return false;

    return helper.IsPhaseThaddius();
}
