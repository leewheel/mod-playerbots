#include "VanillaNaxxActions.h"

#include "Playerbots.h"

bool VanillaLoathebPositionAction::Execute(Event /*event*/)
{
    if (!helper.UpdateBossAI())
        return false;

    if (botAI->IsTank(bot))
    {
        if (AI_VALUE2(bool, "has aggro", "boss target"))
            return MoveTo(VANILLA_NAXX_MAP_ID, helper.mainTankPos.first, helper.mainTankPos.second, bot->GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT);
    }
    else if (botAI->IsRanged(bot))
        return MoveInside(VANILLA_NAXX_MAP_ID, helper.rangePos.first, helper.rangePos.second, bot->GetPositionZ(), 1.0f,
                          MovementPriority::MOVEMENT_COMBAT);
    return false;
}

bool VanillaLoathebChooseTargetAction::Execute(Event /*event*/)
{
    if (!helper.UpdateBossAI())
        return false;

    GuidVector attackers = context->GetValue<GuidVector>("attackers")->Get();
    Unit* target = nullptr;
    Unit* target_boss = nullptr;
    Unit* target_spore = nullptr;
    for (auto i = attackers.begin(); i != attackers.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (!unit)
            continue;

        if (!unit->IsAlive())
            continue;

        if (botAI->EqualLowercaseName(unit->GetName(), "spore") || unit->GetEntry() == 16286)
            target_spore = unit;

        if (botAI->EqualLowercaseName(unit->GetName(), "loatheb") || unit->GetEntry() == 16011)
            target_boss = unit;
    }
    if (target_spore && bot->GetDistance2d(target_spore) <= 1.0f)
        target = target_spore;
    else
        target = target_boss;

    if (!target || context->GetValue<Unit*>("current target")->Get() == target)
        return false;

    return Attack(target);
}
