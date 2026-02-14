// RaidNaxxTriggers.cpp
#include "RaidNaxxTriggers.h"

#include "GenericTriggers.h"
#include "ObjectAccessor.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

// ==================== Patchwerk Triggers ====================

NaxxPatchwerkCombatTrigger::NaxxPatchwerkCombatTrigger(PlayerbotAI* botAI) 
    : Trigger(botAI, "naxx patchwerk combat") 
{
}

bool NaxxPatchwerkCombatTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "patchwerk");
    if (!boss || boss->GetEntry() != 16028)
        return false;
    
    return boss->IsInCombat();
}

NaxxPatchwerkFrenzyTrigger::NaxxPatchwerkFrenzyTrigger(PlayerbotAI* botAI) 
    : Trigger(botAI, "naxx patchwerk frenzy") 
{
}

bool NaxxPatchwerkFrenzyTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "patchwerk");
    if (!boss || boss->GetEntry() != 16028)
        return false;
    
    // Check if boss has Frenzy aura (28131)
    return boss->HasAura(28131);
}

NaxxPatchwerkBerserkTrigger::NaxxPatchwerkBerserkTrigger(PlayerbotAI* botAI) 
    : Trigger(botAI, "naxx patchwerk berserk") 
{
}

bool NaxxPatchwerkBerserkTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "patchwerk");
    if (!boss || boss->GetEntry() != 16028)
        return false;
    
    // Check if boss has Berserk aura (26662)
    return boss->HasAura(26662);
}

NaxxPatchwerkOffTankPositionTrigger::NaxxPatchwerkOffTankPositionTrigger(PlayerbotAI* botAI) 
    : Trigger(botAI, "naxx patchwerk offtank position") 
{
}

bool NaxxPatchwerkOffTankPositionTrigger::IsActive()
{
    // Only for off-tanks
    if (!botAI->IsTank(bot) || botAI->IsMainTank(bot))
        return false;
    
    Unit* boss = AI_VALUE2(Unit*, "find target", "patchwerk");
    if (!boss || boss->GetEntry() != 16028)
        return false;
    
    // Check if not in melee range
    return !bot->IsWithinMeleeRange(boss);
}
