/*
 * This is Leewheel Script Project
 */

#include "NaxxTriggers.h"
#include "Playerbots.h"
#include "ServerFacade.h"

//By Leewheel 2026-02-11
bool InNaxxramasTrigger::IsActive()
{
    return bot->GetMapId() == 533; // Naxxramas map ID
}
//End By Leewheel

//By Leewheel 2026-02-11
bool AnubrekhanLocustSwarmTrigger::IsActive()
{
    Unit* target = AI->GetTarget();
    if (!target || target->GetEntry() != 15956) // Anub'Rekhan entry
        return false;
    
    return target->HasAura(28785); // Locust Swarm spell ID
}
//End By Leewheel

//By Leewheel 2026-02-11
bool CryptGuardSpawnedTrigger::IsActive()
{
    Unit* guard = AI_VALUE2(Unit*, "find target", "crypt guard");
    return guard != nullptr;
}
//End By Leewheel

//By Leewheel 2026-02-11
bool FaerlinaFrenzyTrigger::IsActive()
{
    Unit* target = AI->GetTarget();
    if (!target || target->GetEntry() != 15953) // Grand Widow Faerlina entry
        return false;
    
    return target->HasAura(28798); // Frenzy spell ID
}
//End By Leewheel

//By Leewheel 2026-02-11
bool FaerlinaWorshipperTrigger::IsActive()
{
    Unit* worshipper = AI_VALUE2(Unit*, "find target", "worshipper");
    return worshipper != nullptr;
}
//End By Leewheel

//By Leewheel 2026-02-11
bool WebWrapTargetTrigger::IsActive()
{
    Unit* wrapped = AI_VALUE2(Unit*, "find friendly", "web wrapped");
    return wrapped != nullptr;
}
//End By Leewheel

//By Leewheel 2026-02-11
bool WebSprayTrigger::IsActive()
{
    Unit* target = AI->GetTarget();
    if (!target || target->GetEntry() != 15952) // Maexxna entry
        return false;
    
    return target->HasAura(29484); // Web Spray spell ID
}
//End By Leewheel

//By Leewheel 2026-02-11
bool SpiderlingsSpawnedTrigger::IsActive()
{
    Unit* spiderling = AI_VALUE2(Unit*, "find target", "spiderling");
    return spiderling != nullptr;
}
//End By Leewheel

//By Leewheel 2026-02-11
bool PatchwerkHatefulStrikeTrigger::IsActive()
{
    Unit* target = AI->GetTarget();
    if (!target || target->GetEntry() != 16028) // Patchwerk entry
        return false;
    
    return target->GetHealthPct() > 5; // Hateful Strike throughout fight
}
//End By Leewheel

//By Leewheel 2026-02-11
bool PatchwerkBerserkTrigger::IsActive()
{
    Unit* target = AI->GetTarget();
    if (!target || target->GetEntry() != 16028) // Patchwerk entry
        return false;
    
    return target->GetHealthPct() < 5; // Berserk at low health
}
//End By Leewheel

//By Leewheel 2026-02-11
bool GrobbulusInjectionTrigger::IsActive()
{
    return bot->HasAura(28169); // Mutating Injection spell ID
}
//End By Leewheel

//By Leewheel 2026-02-11
bool PoisonCloudTrigger::IsActive()
{
    Unit* cloud = AI_VALUE2(Unit*, "find target", "poison cloud");
    return cloud != nullptr;
}
//End By Leewheel

//By Leewheel 2026-02-11
bool GluthDecimateTrigger::IsActive()
{
    Unit* target = AI->GetTarget();
    if (!target || target->GetEntry() != 16030) // Gluth entry
        return false;
    
    return target->HasAura(28375); // Decimate spell ID
}
//End By Leewheel

//By Leewheel 2026-02-11
bool MortalWoundTrigger::IsActive()
{
    Unit* target = AI->GetTarget();
    if (!target || target->GetEntry() != 16030) // Gluth entry
        return false;
    
    return bot->HasAura(25646); // Mortal Wound spell ID
}
//End By Leewheel

//By Leewheel 2026-02-11
bool NothTeleportTrigger::IsActive()
{
    Unit* target = AI->GetTarget();
    if (!target || target->GetEntry() != 15954) // Noth the Plaguebringer entry
        return false;
    
    float distance = bot->GetDistance(target);
    return distance > 50.0f; // Noth teleported to balcony
}
//End By Leewheel

//By Leewheel 2026-02-11
bool SkeletonsSpawnedTrigger::IsActive()
{
    Unit* skeleton = AI_VALUE2(Unit*, "find target", "skeleton");
    return skeleton != nullptr;
}
//End By Leewheel

//By Leewheel 2026-02-11
bool HeiganDanceTrigger::IsActive()
{
    Unit* target = AI->GetTarget();
    if (!target || target->GetEntry() != 15936) // Heigan the Unclean entry
        return false;
    
    return target->HasAura(29371); // Spell Disruption aura
}
//End By Leewheel

//By Leewheel 2026-02-11
bool EruptionSoonTrigger::IsActive()
{
    Unit* target = AI->GetTarget();
    if (!target || target->GetEntry() != 15936) // Heigan the Unclean entry
        return false;
    
    return target->HasAura(29371); // During dance phase
}
//End By Leewheel

//By Leewheel 2026-02-11
bool LoathebNecroticAuraTrigger::IsActive()
{
    Unit* target = AI->GetTarget();
    if (!target || target->GetEntry() != 16011) // Loatheb entry
        return false;
    
    return target->HasAura(55593); // Necrotic Aura spell ID
}
//End By Leewheel

//By Leewheel 2026-02-11
bool SporeSpawnedTrigger::IsActive()
{
    Unit* spore = AI_VALUE2(Unit*, "find target", "spore");
    return spore != nullptr;
}
//End By Leewheel

//By Leewheel 2026-02-11
bool RazuviousShoutTrigger::IsActive()
{
    Unit* target = AI->GetTarget();
    if (!target || target->GetEntry() != 16061) // Instructor Razuvious entry
        return false;
    
    return target->HasAura(29107); // Disrupting Shout spell ID
}
//End By Leewheel

//By Leewheel 2026-02-11
bool DeathKnightUnderstudyTrigger::IsActive()
{
    Unit* understudy = AI_VALUE2(Unit*, "find target", "death knight understudy");
    return understudy != nullptr;
}
//End By Leewheel

//By Leewheel 2026-02-11
bool GothikWaveTrigger::IsActive()
{
    Unit* target = AI->GetTarget();
    if (!target || target->GetEntry() != 16060) // Gothik the Harvester entry
        return false;
    
    return target->GetHealthPct() > 30; // During wave phase
}
//End By Leewheel

//By Leewheel 2026-02-11
bool GothikCentralTrigger::IsActive()
{
    Unit* target = AI->GetTarget();
    if (!target || target->GetEntry() != 16060) // Gothik the Harvester entry
        return false;
    
    return target->GetHealthPct() <= 30; // Central room phase
}
//End By Leewheel

//By Leewheel 2026-02-11
bool HorsemanMarkTrigger::IsActive()
{
    return bot->HasAura(28832) || bot->HasAura(28833) || 
           bot->HasAura(28834) || bot->HasAura(28835); // Four Horsemen marks
}
//End By Leewheel

//By Leewheel 2026-02-11
bool MeteorTrigger::IsActive()
{
    Unit* target = AI->GetTarget();
    if (!target || target->GetEntry() != 16064) // Lady Blaumeux entry
        return false;
    
    return true; // Meteor ability
}
//End By Leewheel

//By Leewheel 2026-02-11
bool SapphironFlightTrigger::IsActive()
{
    Unit* target = AI->GetTarget();
    if (!target || target->GetEntry() != 15989) // Sapphiron entry
        return false;
    
    return target->HasAura(30130); // Icebolt spell ID
}
//End By Leewheel

//By Leewheel 2026-02-11
bool ChillTrigger::IsActive()
{
    Unit* target = AI->GetTarget();
    if (!target || target->GetEntry() != 15989) // Sapphiron entry
        return false;
    
    return bot->HasAura(28547); // Chill spell ID
}
//End By Leewheel

//By Leewheel 2026-02-11
bool KelthuzadFrostBlastTrigger::IsActive()
{
    Unit* target = AI->GetTarget();
    if (!target || target->GetEntry() != 15990) // Kel'Thuzad entry
        return false;
    
    return target->HasAura(27808); // Frost Blast spell ID
}
//End By Leewheel

//By Leewheel 2026-02-11
bool KelthuzadChainsTrigger::IsActive()
{
    return bot->HasAura(28410); // Chains of Kel'Thuzad spell ID
}
//End By Leewheel

//By Leewheel 2026-02-11
bool GuardiansSpawnedTrigger::IsActive()
{
    Unit* guardian = AI_VALUE2(Unit*, "find target", "guardian of icecrown");
    return guardian != nullptr;
}
//End By Leewheel

//By Leewheel 2026-02-11
bool ThaddiusPolarityShiftTrigger::IsActive()
{
    Unit* target = AI->GetTarget();
    if (!target || target->GetEntry() != 15928) // Thaddius entry
        return false;
    
    return bot->HasAura(28059) || bot->HasAura(28084); // Positive/Negative Charge
}
//End By Leewheel

//By Leewheel 2026-02-11
bool TeslaCoilTrigger::IsActive()
{
    Unit* coil = AI_VALUE2(Unit*, "find target", "tesla coil");
    return coil != nullptr;
}
//End By Leewheel
