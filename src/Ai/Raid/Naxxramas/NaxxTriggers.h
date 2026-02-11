/*
 * This is Leewheel Script Project
 */

#ifndef _PLAYERBOT_NAXXTRIGGERS_H
#define _PLAYERBOT_NAXXTRIGGERS_H

#include "Trigger.h"

//By Leewheel 2026-02-11
class PlayerbotAI;

class InNaxxramasTrigger : public Trigger
{
public:
    InNaxxramasTrigger(PlayerbotAI* botAI) : Trigger(botAI, "in naxxramas") {}
    bool IsActive() override;
};

class AnubrekhanLocustSwarmTrigger : public Trigger
{
public:
    AnubrekhanLocustSwarmTrigger(PlayerbotAI* botAI) : Trigger(botAI, "anubrekhan locust swarm") {}
    bool IsActive() override;
};

class CryptGuardSpawnedTrigger : public Trigger
{
public:
    CryptGuardSpawnedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "crypt guard spawned") {}
    bool IsActive() override;
};

class FaerlinaFrenzyTrigger : public Trigger
{
public:
    FaerlinaFrenzyTrigger(PlayerbotAI* botAI) : Trigger(botAI, "faerlina frenzy") {}
    bool IsActive() override;
};

class FaerlinaWorshipperTrigger : public Trigger
{
public:
    FaerlinaWorshipperTrigger(PlayerbotAI* botAI) : Trigger(botAI, "faerlina worshipper") {}
    bool IsActive() override;
};

class WebWrapTargetTrigger : public Trigger
{
public:
    WebWrapTargetTrigger(PlayerbotAI* botAI) : Trigger(botAI, "web wrap target") {}
    bool IsActive() override;
};

class WebSprayTrigger : public Trigger
{
public:
    WebSprayTrigger(PlayerbotAI* botAI) : Trigger(botAI, "web spray") {}
    bool IsActive() override;
};

class SpiderlingsSpawnedTrigger : public Trigger
{
public:
    SpiderlingsSpawnedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "spiderlings spawned") {}
    bool IsActive() override;
};

class PatchwerkHatefulStrikeTrigger : public Trigger
{
public:
    PatchwerkHatefulStrikeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "patchwerk hateful strike") {}
    bool IsActive() override;
};

class PatchwerkBerserkTrigger : public Trigger
{
public:
    PatchwerkBerserkTrigger(PlayerbotAI* botAI) : Trigger(botAI, "patchwerk berserk") {}
    bool IsActive() override;
};

class GrobbulusInjectionTrigger : public Trigger
{
public:
    GrobbulusInjectionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "grobbulus injection") {}
    bool IsActive() override;
};

class PoisonCloudTrigger : public Trigger
{
public:
    PoisonCloudTrigger(PlayerbotAI* botAI) : Trigger(botAI, "poison cloud") {}
    bool IsActive() override;
};

class GluthDecimateTrigger : public Trigger
{
public:
    GluthDecimateTrigger(PlayerbotAI* botAI) : Trigger(botAI, "gluth decimate") {}
    bool IsActive() override;
};

class MortalWoundTrigger : public Trigger
{
public:
    MortalWoundTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mortal wound") {}
    bool IsActive() override;
};

class NothTeleportTrigger : public Trigger
{
public:
    NothTeleportTrigger(PlayerbotAI* botAI) : Trigger(botAI, "noth teleport") {}
    bool IsActive() override;
};

class SkeletonsSpawnedTrigger : public Trigger
{
public:
    SkeletonsSpawnedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "skeletons spawned") {}
    bool IsActive() override;
};

class HeiganDanceTrigger : public Trigger
{
public:
    HeiganDanceTrigger(PlayerbotAI* botAI) : Trigger(botAI, "heigan dance") {}
    bool IsActive() override;
};

class EruptionSoonTrigger : public Trigger
{
public:
    EruptionSoonTrigger(PlayerbotAI* botAI) : Trigger(botAI, "eruption soon") {}
    bool IsActive() override;
};

class LoathebNecroticAuraTrigger : public Trigger
{
public:
    LoathebNecroticAuraTrigger(PlayerbotAI* botAI) : Trigger(botAI, "loatheb necrotic aura") {}
    bool IsActive() override;
};

class SporeSpawnedTrigger : public Trigger
{
public:
    SporeSpawnedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "spore spawned") {}
    bool IsActive() override;
};

class RazuviousShoutTrigger : public Trigger
{
public:
    RazuviousShoutTrigger(PlayerbotAI* botAI) : Trigger(botAI, "razuvious shout") {}
    bool IsActive() override;
};

class DeathKnightUnderstudyTrigger : public Trigger
{
public:
    DeathKnightUnderstudyTrigger(PlayerbotAI* botAI) : Trigger(botAI, "death knight understudy") {}
    bool IsActive() override;
};

class GothikWaveTrigger : public Trigger
{
public:
    GothikWaveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "gothik wave") {}
    bool IsActive() override;
};

class GothikCentralTrigger : public Trigger
{
public:
    GothikCentralTrigger(PlayerbotAI* botAI) : Trigger(botAI, "gothik central") {}
    bool IsActive() override;
};

class HorsemanMarkTrigger : public Trigger
{
public:
    HorsemanMarkTrigger(PlayerbotAI* botAI) : Trigger(botAI, "horseman mark") {}
    bool IsActive() override;
};

class MeteorTrigger : public Trigger
{
public:
    MeteorTrigger(PlayerbotAI* botAI) : Trigger(botAI, "meteor") {}
    bool IsActive() override;
};

class SapphironFlightTrigger : public Trigger
{
public:
    SapphironFlightTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sapphiron flight") {}
    bool IsActive() override;
};

class ChillTrigger : public Trigger
{
public:
    ChillTrigger(PlayerbotAI* botAI) : Trigger(botAI, "chill") {}
    bool IsActive() override;
};

class KelthuzadFrostBlastTrigger : public Trigger
{
public:
    KelthuzadFrostBlastTrigger(PlayerbotAI* botAI) : Trigger(botAI, "kelthuzad frost blast") {}
    bool IsActive() override;
};

class KelthuzadChainsTrigger : public Trigger
{
public:
    KelthuzadChainsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "kelthuzad chains") {}
    bool IsActive() override;
};

class GuardiansSpawnedTrigger : public Trigger
{
public:
    GuardiansSpawnedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "guardians spawned") {}
    bool IsActive() override;
};

class ThaddiusPolarityShiftTrigger : public Trigger
{
public:
    ThaddiusPolarityShiftTrigger(PlayerbotAI* botAI) : Trigger(botAI, "thaddius polarity shift") {}
    bool IsActive() override;
};

class TeslaCoilTrigger : public Trigger
{
public:
    TeslaCoilTrigger(PlayerbotAI* botAI) : Trigger(botAI, "tesla coil") {}
    bool IsActive() override;
};
//End By Leewheel

#endif
