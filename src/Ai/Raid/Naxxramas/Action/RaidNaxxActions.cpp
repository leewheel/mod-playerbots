// RaidNaxxActions.cpp
#include "RaidNaxxActions.h"

#include "GenericSpellActions.h"
#include "LastMovementValue.h"
#include "MovementActions.h"
#include "Playerbots.h"
#include "PositionAction.h"

// ==================== Patchwerk Actions ====================

bool NaxxPatchwerkOffTankPositionAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "patchwerk");
    if (!boss)
        return false;
    
    // Move to boss side in melee range
    float angle = boss->GetOrientation() + M_PI_2; // 90 degrees to the right
    float distance = 5.0f; // Melee range
    
    float x = boss->GetPositionX() + distance * cos(angle);
    float y = boss->GetPositionY() + distance * sin(angle);
    
    return MoveTo(boss->GetMapId(), x, y, boss->GetPositionZ(), 
                  false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT);
}

bool NaxxPatchwerkBurnPhaseAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "patchwerk");
    if (!boss)
        return false;
    
    // Full DPS - attack the boss
    // The AI will automatically use cooldowns based on other strategies
    return Attack(boss);
}
