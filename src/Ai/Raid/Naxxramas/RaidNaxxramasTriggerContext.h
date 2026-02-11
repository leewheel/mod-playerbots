/*
 * This is Leewheel Script Project
 */

#ifndef _PLAYERBOT_NAXXRAMASTRIGGERCONTEXT_H
#define _PLAYERBOT_NAXXRAMASTRIGGERCONTEXT_H

#include "Trigger.h"
#include "NamedObjectContext.h"
#include "NaxxTriggers.h"

//By Leewheel 2026-02-11
class RaidNaxxramasTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidNaxxramasTriggerContext()
    {
        creators["in naxxramas"] = &RaidNaxxramasTriggerContext::in_naxxramas;
        creators["anubrekhan locust swarm"] = &RaidNaxxramasTriggerContext::anubrekhan_locust_swarm;
        creators["crypt guard spawned"] = &RaidNaxxramasTriggerContext::crypt_guard_spawned;
        creators["faerlina frenzy"] = &RaidNaxxramasTriggerContext::faerlina_frenzy;
        creators["faerlina worshipper"] = &RaidNaxxramasTriggerContext::faerlina_worshipper;
        creators["web wrap target"] = &RaidNaxxramasTriggerContext::web_wrap_target;
        creators["web spray"] = &RaidNaxxramasTriggerContext::web_spray;
        creators["spiderlings spawned"] = &RaidNaxxramasTriggerContext::spiderlings_spawned;
        creators["patchwerk hateful strike"] = &RaidNaxxramasTriggerContext::patchwerk_hateful_strike;
        creators["patchwerk berserk"] = &RaidNaxxramasTriggerContext::patchwerk_berserk;
        creators["grobbulus injection"] = &RaidNaxxramasTriggerContext::grobbulus_injection;
        creators["poison cloud"] = &RaidNaxxramasTriggerContext::poison_cloud;
        creators["gluth decimate"] = &RaidNaxxramasTriggerContext::gluth_decimate;
        creators["mortal wound"] = &RaidNaxxramasTriggerContext::mortal_wound;
        creators["noth teleport"] = &RaidNaxxramasTriggerContext::noth_teleport;
        creators["skeletons spawned"] = &RaidNaxxramasTriggerContext::skeletons_spawned;
        creators["heigan dance"] = &RaidNaxxramasTriggerContext::heigan_dance;
        creators["eruption soon"] = &RaidNaxxramasTriggerContext::eruption_soon;
        creators["loatheb necrotic aura"] = &RaidNaxxramasTriggerContext::loatheb_necrotic_aura;
        creators["spore spawned"] = &RaidNaxxramasTriggerContext::spore_spawned;
        creators["razuvious shout"] = &RaidNaxxramasTriggerContext::razuvious_shout;
        creators["death knight understudy"] = &RaidNaxxramasTriggerContext::death_knight_understudy;
        creators["gothik wave"] = &RaidNaxxramasTriggerContext::gothik_wave;
        creators["gothik central"] = &RaidNaxxramasTriggerContext::gothik_central;
        creators["horseman mark"] = &RaidNaxxramasTriggerContext::horseman_mark;
        creators["meteor"] = &RaidNaxxramasTriggerContext::meteor;
        creators["sapphiron flight"] = &RaidNaxxramasTriggerContext::sapphiron_flight;
        creators["chill"] = &RaidNaxxramasTriggerContext::chill;
        creators["kelthuzad frost blast"] = &RaidNaxxramasTriggerContext::kelthuzad_frost_blast;
        creators["kelthuzad chains"] = &RaidNaxxramasTriggerContext::kelthuzad_chains;
        creators["guardians spawned"] = &RaidNaxxramasTriggerContext::guardians_spawned;
        creators["thaddius polarity shift"] = &RaidNaxxramasTriggerContext::thaddius_polarity_shift;
        creators["tesla coil"] = &RaidNaxxramasTriggerContext::tesla_coil;
    }

private:
    static Trigger* in_naxxramas(PlayerbotAI* botAI) { return new InNaxxramasTrigger(botAI); }
    static Trigger* anubrekhan_locust_swarm(PlayerbotAI* botAI) { return new AnubrekhanLocustSwarmTrigger(botAI); }
    static Trigger* crypt_guard_spawned(PlayerbotAI* botAI) { return new CryptGuardSpawnedTrigger(botAI); }
    static Trigger* faerlina_frenzy(PlayerbotAI* botAI) { return new FaerlinaFrenzyTrigger(botAI); }
    static Trigger* faerlina_worshipper(PlayerbotAI* botAI) { return new FaerlinaWorshipperTrigger(botAI); }
    static Trigger* web_wrap_target(PlayerbotAI* botAI) { return new WebWrapTargetTrigger(botAI); }
    static Trigger* web_spray(PlayerbotAI* botAI) { return new WebSprayTrigger(botAI); }
    static Trigger* spiderlings_spawned(PlayerbotAI* botAI) { return new SpiderlingsSpawnedTrigger(botAI); }
    static Trigger* patchwerk_hateful_strike(PlayerbotAI* botAI) { return new PatchwerkHatefulStrikeTrigger(botAI); }
    static Trigger* patchwerk_berserk(PlayerbotAI* botAI) { return new PatchwerkBerserkTrigger(botAI); }
    static Trigger* grobbulus_injection(PlayerbotAI* botAI) { return new GrobbulusInjectionTrigger(botAI); }
    static Trigger* poison_cloud(PlayerbotAI* botAI) { return new PoisonCloudTrigger(botAI); }
    static Trigger* gluth_decimate(PlayerbotAI* botAI) { return new GluthDecimateTrigger(botAI); }
    static Trigger* mortal_wound(PlayerbotAI* botAI) { return new MortalWoundTrigger(botAI); }
    static Trigger* noth_teleport(PlayerbotAI* botAI) { return new NothTeleportTrigger(botAI); }
    static Trigger* skeletons_spawned(PlayerbotAI* botAI) { return new SkeletonsSpawnedTrigger(botAI); }
    static Trigger* heigan_dance(PlayerbotAI* botAI) { return new HeiganDanceTrigger(botAI); }
    static Trigger* eruption_soon(PlayerbotAI* botAI) { return new EruptionSoonTrigger(botAI); }
    static Trigger* loatheb_necrotic_aura(PlayerbotAI* botAI) { return new LoathebNecroticAuraTrigger(botAI); }
    static Trigger* spore_spawned(PlayerbotAI* botAI) { return new SporeSpawnedTrigger(botAI); }
    static Trigger* razuvious_shout(PlayerbotAI* botAI) { return new RazuviousShoutTrigger(botAI); }
    static Trigger* death_knight_understudy(PlayerbotAI* botAI) { return new DeathKnightUnderstudyTrigger(botAI); }
    static Trigger* gothik_wave(PlayerbotAI* botAI) { return new GothikWaveTrigger(botAI); }
    static Trigger* gothik_central(PlayerbotAI* botAI) { return new GothikCentralTrigger(botAI); }
    static Trigger* horseman_mark(PlayerbotAI* botAI) { return new HorsemanMarkTrigger(botAI); }
    static Trigger* meteor(PlayerbotAI* botAI) { return new MeteorTrigger(botAI); }
    static Trigger* sapphiron_flight(PlayerbotAI* botAI) { return new SapphironFlightTrigger(botAI); }
    static Trigger* chill(PlayerbotAI* botAI) { return new ChillTrigger(botAI); }
    static Trigger* kelthuzad_frost_blast(PlayerbotAI* botAI) { return new KelthuzadFrostBlastTrigger(botAI); }
    static Trigger* kelthuzad_chains(PlayerbotAI* botAI) { return new KelthuzadChainsTrigger(botAI); }
    static Trigger* guardians_spawned(PlayerbotAI* botAI) { return new GuardiansSpawnedTrigger(botAI); }
    static Trigger* thaddius_polarity_shift(PlayerbotAI* botAI) { return new ThaddiusPolarityShiftTrigger(botAI); }
    static Trigger* tesla_coil(PlayerbotAI* botAI) { return new TeslaCoilTrigger(botAI); }
};
//End By Leewheel

#endif
