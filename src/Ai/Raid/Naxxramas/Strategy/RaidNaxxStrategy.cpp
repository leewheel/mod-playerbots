// RaidNaxxStrategy.cpp
#include "RaidNaxxStrategy.h"

void RaidNaxxStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // ==================== Patchwerk ====================
    
    // Off-tank positioning - maintain melee range
    triggers.push_back(new TriggerNode(
        "naxx patchwerk offtank position",
        { NextAction("naxx patchwerk offtank position", ACTION_RAID + 3) }
    ));
    
    // Frenzy phase (5% HP) - burn phase
    triggers.push_back(new TriggerNode(
        "naxx patchwerk frenzy",
        { NextAction("naxx patchwerk burn", ACTION_RAID + 5) }
    ));
    
    // Berserk phase (6 minutes) - burn phase
    triggers.push_back(new TriggerNode(
        "naxx patchwerk berserk",
        { NextAction("naxx patchwerk burn", ACTION_RAID + 5) }
    ));
}

void RaidNaxxStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // Empty for now - can add multipliers later if needed
}
