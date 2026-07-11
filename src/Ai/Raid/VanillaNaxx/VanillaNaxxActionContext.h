#ifndef PLAYERBOTS_VANILLANAXXACTIONCONTEXT_H
#define PLAYERBOTS_VANILLANAXXACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "VanillaNaxxActions.h"

class RaidVanillaNaxxActionContext : public NamedObjectContext<Action>
{
public:
    RaidVanillaNaxxActionContext()
    {
        creators["grobbulus go behind the boss"] = &RaidVanillaNaxxActionContext::go_behind_the_boss;
        creators["rotate grobbulus"] = &RaidVanillaNaxxActionContext::rotate_grobbulus;
        creators["grobbulus move center"] = &RaidVanillaNaxxActionContext::grobbulus_move_center;
        creators["grobbulus move away"] = &RaidVanillaNaxxActionContext::grobbulus_move_away;

        creators["thaddius attack nearest pet"] = &RaidVanillaNaxxActionContext::thaddius_attack_nearest_pet;
        creators["thaddius move to platform"] = &RaidVanillaNaxxActionContext::thaddius_move_to_platform;
        creators["thaddius move polarity"] = &RaidVanillaNaxxActionContext::thaddius_move_polarity;

        creators["razuvious use obedience crystal"] = &RaidVanillaNaxxActionContext::razuvious_use_obedience_crystal;
        creators["razuvious target"] = &RaidVanillaNaxxActionContext::razuvious_target;

        creators["four horsemen attract alternatively"] = &RaidVanillaNaxxActionContext::four_horsemen_attract_alternatively;
        creators["four horsemen attack in order"] = &RaidVanillaNaxxActionContext::four_horsemen_attack_in_order;

        creators["sapphiron ground position"] = &RaidVanillaNaxxActionContext::sapphiron_ground_position;
        creators["sapphiron flight position"] = &RaidVanillaNaxxActionContext::sapphiron_flight_position;

        creators["kel'thuzad choose target"] = &RaidVanillaNaxxActionContext::kelthuzad_choose_target;
        creators["kel'thuzad position"] = &RaidVanillaNaxxActionContext::kelthuzad_position;

        creators["anub'rekhan choose target"] = &RaidVanillaNaxxActionContext::anubrekhan_choose_target;
        creators["anub'rekhan position"] = &RaidVanillaNaxxActionContext::anubrekhan_position;

        creators["gluth choose target"] = &RaidVanillaNaxxActionContext::gluth_choose_target;
        creators["gluth position"] = &RaidVanillaNaxxActionContext::gluth_position;
        creators["gluth slowdown"] = &RaidVanillaNaxxActionContext::gluth_slowdown;

        creators["loatheb position"] = &RaidVanillaNaxxActionContext::loatheb_position;
        creators["loatheb choose target"] = &RaidVanillaNaxxActionContext::loatheb_choose_target;
    }

private:
    static Action* go_behind_the_boss(PlayerbotAI* ai) { return new VanillaGrobbulusGoBehindAction(ai); }
    static Action* rotate_grobbulus(PlayerbotAI* ai) { return new VanillaGrobbulusRotateAction(ai); }
    static Action* grobbulus_move_center(PlayerbotAI* ai) { return new VanillaGrobbulusMoveCenterAction(ai); }
    static Action* grobbulus_move_away(PlayerbotAI* ai) { return new VanillaGrobbulusMoveAwayAction(ai); }
    static Action* thaddius_attack_nearest_pet(PlayerbotAI* ai) { return new VanillaThaddiusAttackNearestPetAction(ai); }
    static Action* thaddius_move_to_platform(PlayerbotAI* ai) { return new VanillaThaddiusMoveToPlatformAction(ai); }
    static Action* thaddius_move_polarity(PlayerbotAI* ai) { return new VanillaThaddiusMovePolarityAction(ai); }
    static Action* razuvious_target(PlayerbotAI* ai) { return new VanillaRazuviousTargetAction(ai); }
    static Action* razuvious_use_obedience_crystal(PlayerbotAI* ai)
    {
        return new VanillaRazuviousUseObedienceCrystalAction(ai);
    }
    static Action* four_horsemen_attract_alternatively(PlayerbotAI* ai) { return new VanillaFourHorsemenAttractAlternativelyAction(ai); }
    static Action* four_horsemen_attack_in_order(PlayerbotAI* ai) { return new VanillaFourHorsemenAttackInOrderAction(ai); }
    static Action* sapphiron_ground_position(PlayerbotAI* ai) { return new VanillaSapphironGroundPositionAction(ai); }
    static Action* sapphiron_flight_position(PlayerbotAI* ai) { return new VanillaSapphironFlightPositionAction(ai); }
    static Action* kelthuzad_choose_target(PlayerbotAI* ai) { return new VanillaKelthuzadChooseTargetAction(ai); }
    static Action* kelthuzad_position(PlayerbotAI* ai) { return new VanillaKelthuzadPositionAction(ai); }
    static Action* anubrekhan_choose_target(PlayerbotAI* ai) { return new VanillaAnubrekhanChooseTargetAction(ai); }
    static Action* anubrekhan_position(PlayerbotAI* ai) { return new VanillaAnubrekhanPositionAction(ai); }
    static Action* gluth_choose_target(PlayerbotAI* ai) { return new VanillaGluthChooseTargetAction(ai); }
    static Action* gluth_position(PlayerbotAI* ai) { return new VanillaGluthPositionAction(ai); }
    static Action* gluth_slowdown(PlayerbotAI* ai) { return new VanillaGluthSlowdownAction(ai); }
    static Action* loatheb_position(PlayerbotAI* ai) { return new VanillaLoathebPositionAction(ai); }
    static Action* loatheb_choose_target(PlayerbotAI* ai) { return new VanillaLoathebChooseTargetAction(ai); }
};

#endif
