/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license
 * This is Leewheel Script Project
 */

#include "NaxxActions.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "Event.h"
#include "Unit.h"

//By Leewheel 2026-02-11
bool SpreadOutAction::Execute(Event event)
{
    Unit* target = AI_VALUE2(Unit*, "find target", "current target");
    if (!target)
        return false;

    float distance = bot->GetDistance(target);
    if (distance < 10.0f)
    {
        float angle = bot->GetAngle(target) + M_PI;
        float x = bot->GetPositionX() + cos(angle) * 15.0f;
        float y = bot->GetPositionY() + sin(angle) * 15.0f;
        float z = bot->GetPositionZ();
        
        bot->GetMotionMaster()->MovePoint(0, x, y, z);
        return true;
    }
    return false;
}
//End By Leewheel

//By Leewheel 2026-02-11
bool AttackCryptGuardAction::Execute(Event event)
{
    Unit* guard = AI_VALUE2(Unit*, "find target", "crypt guard");
    if (guard)
    {
        Attack(guard);
        return true;
    }
    return false;
}
//End By Leewheel

//By Leewheel 2026-02-11
bool UseWidowsEmbraceAction::Execute(Event event)
{
    Unit* target = AI_VALUE2(Unit*, "find target", "grand widow faerlina");
    if (!target || target->GetEntry() != 15953) // Grand Widow Faerlina
        return false;

    Unit* worshipper = AI_VALUE2(Unit*, "find target", "worshipper");
    if (worshipper)
    {
        Attack(worshipper);
        return true;
    }
    return false;
}
//End By Leewheel

//By Leewheel 2026-02-11
bool KillWorshipperAction::Execute(Event event)
{
    Unit* worshipper = AI_VALUE2(Unit*, "find target", "worshipper");
    if (worshipper)
    {
        Attack(worshipper);
        return true;
    }
    return false;
}
//End By Leewheel

//By Leewheel 2026-02-11
bool FreeWebWrapAction::Execute(Event event)
{
    Unit* wrapped = AI_VALUE2(Unit*, "find friendly", "web wrapped");
    if (wrapped)
    {
        if (bot->getClass() == CLASS_ROGUE || bot->getClass() == CLASS_WARRIOR)
        {
            Attack(wrapped);
            return true;
        }
        else if (bot->getClass() == CLASS_PRIEST || bot->getClass() == CLASS_PALADIN)
        {
            // Cast dispel if available
            return true;
        }
    }
    return false;
}
//End By Leewheel

//By Leewheel 2026-02-11
bool DispelWebSprayAction::Execute(Event event)
{
    if (bot->HasAura(29484)) // Web Spray
    {
        // Try to dispel if capable
        if ((bot->getClass() == CLASS_PRIEST || bot->getClass() == CLASS_PALADIN) || bot->getClass() == CLASS_DRUID)
        {
            // Cast dispel
            return true;
        }
    }
    return false;
}
//End By Leewheel

//By Leewheel 2026-02-11
bool KillSpiderlingsAction::Execute(Event event)
{
    Unit* spiderling = AI_VALUE2(Unit*, "find target", "spiderling");
    if (spiderling)
    {
        Attack(spiderling);
        return true;
    }
    return false;
}
//End By Leewheel

//By Leewheel 2026-02-11
bool PositionForHatefulAction::Execute(Event event)
{
    Unit* target = AI_VALUE2(Unit*, "find target", "patchwerk");
    if (!target || target->GetEntry() != 16028) // Patchwerk
        return false;

    if (botAI->IsTank(bot))
    {
        float distance = bot->GetDistance(target);
        if (distance > 15.0f)
        {
            bot->GetMotionMaster()->MoveChase(target, 5.0f);
            return true;
        }
    }
    return false;
}
//End By Leewheel

//By Leewheel 2026-02-11
bool BurnCooldownsAction::Execute(Event event)
{
    Unit* target = AI_VALUE2(Unit*, "find target", "current target");
    if (!target)
        return false;

    // Use all available cooldowns
    botAI->CastSpell("berserk", target);
    botAI->CastSpell("power infusion", bot);
    botAI->CastSpell("adrenaline rush", target);
    botAI->CastSpell("berserking", target);
    
    return true;
}
//End By Leewheel

//By Leewheel 2026-02-11
bool MoveAwayAction::Execute(Event event)
{
    if (bot->HasAura(28169)) // Mutating Injection
    {
        float angle = frand(0, 2 * M_PI);
        float x = bot->GetPositionX() + cos(angle) * 20.0f;
        float y = bot->GetPositionY() + sin(angle) * 20.0f;
        float z = bot->GetPositionZ();
        
        bot->GetMotionMaster()->MovePoint(0, x, y, z);
        return true;
    }
    return false;
}
//End By Leewheel

//By Leewheel 2026-02-11
bool AvoidPoisonCloudAction::Execute(Event event)
{
    Unit* cloud = AI_VALUE2(Unit*, "find target", "poison cloud");
    if (cloud)
    {
        float distance = bot->GetDistance(cloud);
        if (distance < 10.0f)
        {
            float angle = bot->GetAngle(cloud) + M_PI;
            float x = bot->GetPositionX() + cos(angle) * 15.0f;
            float y = bot->GetPositionY() + sin(angle) * 15.0f;
            float z = bot->GetPositionZ();
            
            bot->GetMotionMaster()->MovePoint(0, x, y, z);
            return true;
        }
    }
    return false;
}
//End By Leewheel

//By Leewheel 2026-02-11
bool KillZombiesAction::Execute(Event event)
{
    Unit* zombie = AI_VALUE2(Unit*, "find target", "zombie chow");
    if (zombie)
    {
        Attack(zombie);
        return true;
    }
    return false;
}
//End By Leewheel

//By Leewheel 2026-02-11
bool TankRotationAction::Execute(Event event)
{
    if (bot->HasAura(25646) && bot->GetHealthPct() < 50) // Mortal Wound
    {
        // Call for tank rotation
        return true;
    }
    return false;
}
//End By Leewheel

//By Leewheel 2026-02-11
bool PositionNothAction::Execute(Event event)
{
    Unit* target = AI_VALUE2(Unit*, "find target", "noth the plaguebringer");
    if (!target || target->GetEntry() != 15954) // Noth
        return false;

    float distance = bot->GetDistance(target);
    if (distance > 50.0f) // Noth on balcony
    {
        // Position for skeleton waves
        return true;
    }
    return false;
}
//End By Leewheel

//By Leewheel 2026-02-11
bool AoeSkeletonsAction::Execute(Event event)
{
    Unit* skeleton = AI_VALUE2(Unit*, "find target", "skeleton");
    if (skeleton)
    {
        // Cast AoE spells
        botAI->CastSpell("blizzard", bot);
        botAI->CastSpell("flamestrike", bot);
        botAI->CastSpell("consecration", bot);
        return true;
    }
    return false;
}
//End By Leewheel

//By Leewheel 2026-02-11
bool DanceHeiganAction::Execute(Event event)
{
    Unit* target = AI_VALUE2(Unit*, "find target", "heigan the unclean");
    if (!target || target->GetEntry() != 15936) // Heigan
        return false;

    // Move to safe zone based on current phase
    float x = 2796.0f + frand(-5, 5);
    float y = -3707.0f + frand(-5, 5);
    float z = 276.0f;
    
    bot->GetMotionMaster()->MovePoint(0, x, y, z);
    return true;
}
//End By Leewheel

//By Leewheel 2026-02-11
bool MoveToSafeZoneAction::Execute(Event event)
{
    // Move to designated safe zone for eruption
    float x = 2796.0f + frand(-5, 5);
    float y = -3707.0f + frand(-5, 5);
    float z = 276.0f;
    
    bot->GetMotionMaster()->MovePoint(0, x, y, z);
    return true;
}
//End By Leewheel

//By Leewheel 2026-02-11
bool HealRotationAction::Execute(Event event)
{
    Unit* target = AI_VALUE2(Unit*, "find target", "loatheb");
    if (!target || target->GetEntry() != 16011) // Loatheb
        return false;

    // Coordinate healing rotation
    if ((bot->getClass() == CLASS_PRIEST || bot->getClass() == CLASS_DRUID || bot->getClass() == CLASS_PALADIN || bot->getClass() == CLASS_SHAMAN))
    {
        // Only heal during the brief window
        return true;
    }
    return false;
}
//End By Leewheel

//By Leewheel 2026-02-11
bool KillSporeAction::Execute(Event event)
{
    Unit* spore = AI_VALUE2(Unit*, "find target", "spore");
    if (spore)
    {
        Attack(spore);
        return true;
    }
    return false;
}
//End By Leewheel

//By Leewheel 2026-02-11
bool DisruptShoutAction::Execute(Event event)
{
    Unit* target = AI_VALUE2(Unit*, "find target", "instructor razuvious");
    if (!target || target->GetEntry() != 16061) // Razuvious
        return false;

    if (target->HasAura(29107)) // Disrupting Shout
    {
        // Use interrupt abilities
        botAI->CastSpell("silence", target);
        botAI->CastSpell("counterspell", target);
        botAI->CastSpell("spell lock", target);
        return true;
    }
    return false;
}
//End By Leewheel

//By Leewheel 2026-02-11
bool MindControlUnderstudyAction::Execute(Event event)
{
    Unit* understudy = AI_VALUE2(Unit*, "find target", "death knight understudy");
    if (understudy && bot->getClass() == CLASS_PRIEST)
    {
        botAI->CastSpell("mind control", understudy);
        return true;
    }
    return false;
}
//End By Leewheel

//By Leewheel 2026-02-11
bool PositionGothikAction::Execute(Event event)
{
    // Position for wave phase
    float x = 2665.0f + frand(-5, 5);
    float y = -3428.0f + frand(-5, 5);
    float z = 267.0f;
    
    bot->GetMotionMaster()->MovePoint(0, x, y, z);
    return true;
}
//End By Leewheel

//By Leewheel 2026-02-11
bool CentralRoomAction::Execute(Event event)
{
    // Move to central room
    float x = 2691.0f + frand(-5, 5);
    float y = -3428.0f + frand(-5, 5);
    float z = 267.0f;
    
    bot->GetMotionMaster()->MovePoint(0, x, y, z);
    return true;
}
//End By Leewheel

//By Leewheel 2026-02-11
bool SwitchTargetsAction::Execute(Event event)
{
    Unit* horseman = AI_VALUE2(Unit*, "find target", "four horsemen");
    if (horseman)
    {
        if (!bot->HasAura(28832) && !bot->HasAura(28833) && 
            !bot->HasAura(28834) && !bot->HasAura(28835))
        {
            Attack(horseman);
            return true;
        }
    }
    return false;
}
//End By Leewheel

//By Leewheel 2026-02-11
bool PositionIceBlockAction::Execute(Event event)
{
    // Position behind ice block for LoS
    float x = 3498.0f + frand(-5, 5);
    float y = -5349.0f + frand(-5, 5);
    float z = 137.0f;
    
    bot->GetMotionMaster()->MovePoint(0, x, y, z);
    return true;
}
//End By Leewheel

//By Leewheel 2026-02-11
bool MoveBehindIceBlockAction::Execute(Event event)
{
    Unit* iceBlock = AI_VALUE2(Unit*, "find target", "ice block");
    if (iceBlock)
    {
        float angle = bot->GetAngle(iceBlock) + M_PI;
        float x = iceBlock->GetPositionX() + cos(angle) * 5.0f;
        float y = iceBlock->GetPositionY() + sin(angle) * 5.0f;
        float z = iceBlock->GetPositionZ();
        
        bot->GetMotionMaster()->MovePoint(0, x, y, z);
        return true;
    }
    return false;
}
//End By Leewheel

//By Leewheel 2026-02-11
bool BreakChainsAction::Execute(Event event)
{
    if (bot->HasAura(28410)) // Chains of Kel'Thuzad
    {
        // Move to break chains
        Unit* target = AI_VALUE2(Unit*, "find target", "kel'thuzad");
        if (target)
        {
            float distance = bot->GetDistance(target);
            if (distance > 10.0f)
            {
                bot->GetMotionMaster()->MoveChase(target, 5.0f);
                return true;
            }
        }
    }
    return false;
}
//End By Leewheel

//By Leewheel 2026-02-11
bool KillGuardiansAction::Execute(Event event)
{
    Unit* guardian = AI_VALUE2(Unit*, "find target", "guardian of icecrown");
    if (guardian)
    {
        Attack(guardian);
        return true;
    }
    return false;
}
//End By Leewheel

//By Leewheel 2026-02-11
bool MoveToPolarityAction::Execute(Event event)
{
    Unit* target = AI_VALUE2(Unit*, "find target", "thaddius");
    if (!target || target->GetEntry() != 15928) // Thaddius
        return false;

    if (bot->HasAura(28059)) // Positive Charge
    {
        // Move to positive side
        float x = 3520.0f + frand(-10, 0);
        float y = -2930.0f + frand(-10, 10);
        float z = 302.0f;
        
        bot->GetMotionMaster()->MovePoint(0, x, y, z);
        return true;
    }
    else if (bot->HasAura(28084)) // Negative Charge
    {
        // Move to negative side
        float x = 3520.0f + frand(0, 10);
        float y = -2930.0f + frand(-10, 10);
        float z = 302.0f;
        
        bot->GetMotionMaster()->MovePoint(0, x, y, z);
        return true;
    }
    return false;
}

bool AttackTeslaCoilAction::Execute(Event event)
{
    Unit* coil = AI_VALUE2(Unit*, "find target", "tesla coil");
    if (coil)
    {
        Attack(coil);
        return true;
    }
    return false;
}
