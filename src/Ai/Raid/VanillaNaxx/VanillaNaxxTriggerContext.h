#ifndef PLAYERBOTS_VANILLANAXXTRIGGERCONTEXT_H
#define PLAYERBOTS_VANILLANAXXTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "VanillaNaxxTriggers.h"

class RaidVanillaNaxxTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidVanillaNaxxTriggerContext()
    {
        creators["mutating injection melee"] = &RaidVanillaNaxxTriggerContext::mutating_injection_melee;
        creators["mutating injection ranged"] = &RaidVanillaNaxxTriggerContext::mutating_injection_ranged;
        creators["mutating injection removed"] = &RaidVanillaNaxxTriggerContext::mutating_injection_removed;
        creators["grobbulus cloud"] = &RaidVanillaNaxxTriggerContext::grobbulus_cloud;

        creators["thaddius phase pet"] = &RaidVanillaNaxxTriggerContext::thaddius_phase_pet;
        creators["thaddius phase pet lose aggro"] = &RaidVanillaNaxxTriggerContext::thaddius_phase_pet_lose_aggro;
        creators["thaddius phase transition"] = &RaidVanillaNaxxTriggerContext::thaddius_phase_transition;
        creators["thaddius phase thaddius"] = &RaidVanillaNaxxTriggerContext::thaddius_phase_thaddius;

        creators["razuvious tank"] = &RaidVanillaNaxxTriggerContext::razuvious_tank;
        creators["razuvious nontank"] = &RaidVanillaNaxxTriggerContext::razuvious_nontank;

        creators["four horsemen attractors"] = &RaidVanillaNaxxTriggerContext::four_horsemen_attractors;
        creators["four horsemen except attractors"] = &RaidVanillaNaxxTriggerContext::four_horsemen_except_attractors;

        creators["sapphiron ground"] = &RaidVanillaNaxxTriggerContext::sapphiron_ground;
        creators["sapphiron flight"] = &RaidVanillaNaxxTriggerContext::sapphiron_flight;

        creators["kel'thuzad"] = &RaidVanillaNaxxTriggerContext::kelthuzad;

        creators["anub'rekhan"] = &RaidVanillaNaxxTriggerContext::anubrekhan;
        creators["faerlina"] = &RaidVanillaNaxxTriggerContext::faerlina;
        creators["maexxna"] = &RaidVanillaNaxxTriggerContext::maexxna;

        creators["gluth"] = &RaidVanillaNaxxTriggerContext::gluth;
        creators["gluth main tank mortal wound"] = &RaidVanillaNaxxTriggerContext::gluth_main_tank_mortal_wound;

        creators["loatheb"] = &RaidVanillaNaxxTriggerContext::loatheb;
    }

private:
    static Trigger* mutating_injection_melee(PlayerbotAI* ai) { return new VanillaMutatingInjectionMeleeTrigger(ai); }
    static Trigger* mutating_injection_ranged(PlayerbotAI* ai) { return new VanillaMutatingInjectionRangedTrigger(ai); }
    static Trigger* mutating_injection_removed(PlayerbotAI* ai) { return new VanillaMutatingInjectionRemovedTrigger(ai); }
    static Trigger* grobbulus_cloud(PlayerbotAI* ai) { return new VanillaGrobbulusCloudTrigger(ai); }

    static Trigger* thaddius_phase_pet(PlayerbotAI* ai) { return new VanillaThaddiusPhasePetTrigger(ai); }
    static Trigger* thaddius_phase_pet_lose_aggro(PlayerbotAI* ai) { return new VanillaThaddiusPhasePetLoseAggroTrigger(ai); }
    static Trigger* thaddius_phase_transition(PlayerbotAI* ai) { return new VanillaThaddiusPhaseTransitionTrigger(ai); }
    static Trigger* thaddius_phase_thaddius(PlayerbotAI* ai) { return new VanillaThaddiusPhaseThaddiusTrigger(ai); }
    static Trigger* razuvious_tank(PlayerbotAI* ai) { return new VanillaRazuviousTankTrigger(ai); }
    static Trigger* razuvious_nontank(PlayerbotAI* ai) { return new VanillaRazuviousNontankTrigger(ai); }

    static Trigger* four_horsemen_attractors(PlayerbotAI* ai) { return new VanillaFourHorsemenAttractorsTrigger(ai); }
    static Trigger* four_horsemen_except_attractors(PlayerbotAI* ai) { return new VanillaFourHorsemenExceptAttractorsTrigger(ai); }

    static Trigger* sapphiron_ground(PlayerbotAI* ai) { return new VanillaSapphironGroundTrigger(ai); }
    static Trigger* sapphiron_flight(PlayerbotAI* ai) { return new VanillaSapphironFlightTrigger(ai); }
    static Trigger* kelthuzad(PlayerbotAI* ai) { return new VanillaKelthuzadTrigger(ai); }
    static Trigger* anubrekhan(PlayerbotAI* ai) { return new VanillaAnubrekhanTrigger(ai); }
    static Trigger* faerlina(PlayerbotAI* ai) { return new VanillaFaerlinaTrigger(ai); }
    static Trigger* maexxna(PlayerbotAI* ai) { return new VanillaMaexxnaTrigger(ai); }
    static Trigger* gluth(PlayerbotAI* ai) { return new VanillaGluthTrigger(ai); }
    static Trigger* gluth_main_tank_mortal_wound(PlayerbotAI* ai) { return new VanillaGluthMainTankMortalWoundTrigger(ai); }
    static Trigger* loatheb(PlayerbotAI* ai) { return new VanillaLoathebTrigger(ai); }
};

#endif
