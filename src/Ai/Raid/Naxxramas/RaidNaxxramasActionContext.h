/*
 * This is Leewheel Script Project
 */

#ifndef _PLAYERBOT_NAXXRAMASACTIONCONTEXT_H
#define _PLAYERBOT_NAXXRAMASACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "NaxxActions.h"

//By Leewheel 2026-02-11
class RaidNaxxramasActionContext : public NamedObjectContext<Action>
{
public:
    RaidNaxxramasActionContext()
    {
        creators["spread out"] = &RaidNaxxramasActionContext::spread_out;
        creators["attack crypt guard"] = &RaidNaxxramasActionContext::attack_crypt_guard;
        creators["use widow's embrace"] = &RaidNaxxramasActionContext::use_widows_embrace;
        creators["kill worshipper"] = &RaidNaxxramasActionContext::kill_worshipper;
        creators["free web wrap"] = &RaidNaxxramasActionContext::free_web_wrap;
        creators["dispel web spray"] = &RaidNaxxramasActionContext::dispel_web_spray;
        creators["kill spiderlings"] = &RaidNaxxramasActionContext::kill_spiderlings;
        creators["position for hateful"] = &RaidNaxxramasActionContext::position_for_hateful;
        creators["burn cooldowns"] = &RaidNaxxramasActionContext::burn_cooldowns;
        creators["move away"] = &RaidNaxxramasActionContext::move_away;
        creators["avoid poison cloud"] = &RaidNaxxramasActionContext::avoid_poison_cloud;
        creators["kill zombies"] = &RaidNaxxramasActionContext::kill_zombies;
        creators["tank rotation"] = &RaidNaxxramasActionContext::tank_rotation;
        creators["position noth"] = &RaidNaxxramasActionContext::position_noth;
        creators["aoe skeletons"] = &RaidNaxxramasActionContext::aoe_skeletons;
        creators["dance heigan"] = &RaidNaxxramasActionContext::dance_heigan;
        creators["move to safe zone"] = &RaidNaxxramasActionContext::move_to_safe_zone;
        creators["heal rotation"] = &RaidNaxxramasActionContext::heal_rotation;
        creators["kill spore"] = &RaidNaxxramasActionContext::kill_spore;
        creators["disrupt shout"] = &RaidNaxxramasActionContext::disrupt_shout;
        creators["mind control understudy"] = &RaidNaxxramasActionContext::mind_control_understudy;
        creators["position gothik"] = &RaidNaxxramasActionContext::position_gothik;
        creators["central room"] = &RaidNaxxramasActionContext::central_room;
        creators["switch targets"] = &RaidNaxxramasActionContext::switch_targets;
        creators["position ice block"] = &RaidNaxxramasActionContext::position_ice_block;
        creators["move behind ice block"] = &RaidNaxxramasActionContext::move_behind_ice_block;
        creators["break chains"] = &RaidNaxxramasActionContext::break_chains;
        creators["kill guardians"] = &RaidNaxxramasActionContext::kill_guardians;
        creators["move to polarity"] = &RaidNaxxramasActionContext::move_to_polarity;
        creators["attack tesla coil"] = &RaidNaxxramasActionContext::attack_tesla_coil;
    }

private:
    static Action* spread_out(PlayerbotAI* botAI) { return new SpreadOutAction(botAI); }
    static Action* attack_crypt_guard(PlayerbotAI* botAI) { return new AttackCryptGuardAction(botAI); }
    static Action* use_widows_embrace(PlayerbotAI* botAI) { return new UseWidowsEmbraceAction(botAI); }
    static Action* kill_worshipper(PlayerbotAI* botAI) { return new KillWorshipperAction(botAI); }
    static Action* free_web_wrap(PlayerbotAI* botAI) { return new FreeWebWrapAction(botAI); }
    static Action* dispel_web_spray(PlayerbotAI* botAI) { return new DispelWebSprayAction(botAI); }
    static Action* kill_spiderlings(PlayerbotAI* botAI) { return new KillSpiderlingsAction(botAI); }
    static Action* position_for_hateful(PlayerbotAI* botAI) { return new PositionForHatefulAction(botAI); }
    static Action* burn_cooldowns(PlayerbotAI* botAI) { return new BurnCooldownsAction(botAI); }
    static Action* move_away(PlayerbotAI* botAI) { return new MoveAwayAction(botAI); }
    static Action* avoid_poison_cloud(PlayerbotAI* botAI) { return new AvoidPoisonCloudAction(botAI); }
    static Action* kill_zombies(PlayerbotAI* botAI) { return new KillZombiesAction(botAI); }
    static Action* tank_rotation(PlayerbotAI* botAI) { return new TankRotationAction(botAI); }
    static Action* position_noth(PlayerbotAI* botAI) { return new PositionNothAction(botAI); }
    static Action* aoe_skeletons(PlayerbotAI* botAI) { return new AoeSkeletonsAction(botAI); }
    static Action* dance_heigan(PlayerbotAI* botAI) { return new DanceHeiganAction(botAI); }
    static Action* move_to_safe_zone(PlayerbotAI* botAI) { return new MoveToSafeZoneAction(botAI); }
    static Action* heal_rotation(PlayerbotAI* botAI) { return new HealRotationAction(botAI); }
    static Action* kill_spore(PlayerbotAI* botAI) { return new KillSporeAction(botAI); }
    static Action* disrupt_shout(PlayerbotAI* botAI) { return new DisruptShoutAction(botAI); }
    static Action* mind_control_understudy(PlayerbotAI* botAI) { return new MindControlUnderstudyAction(botAI); }
    static Action* position_gothik(PlayerbotAI* botAI) { return new PositionGothikAction(botAI); }
    static Action* central_room(PlayerbotAI* botAI) { return new CentralRoomAction(botAI); }
    static Action* switch_targets(PlayerbotAI* botAI) { return new SwitchTargetsAction(botAI); }
    static Action* position_ice_block(PlayerbotAI* botAI) { return new PositionIceBlockAction(botAI); }
    static Action* move_behind_ice_block(PlayerbotAI* botAI) { return new MoveBehindIceBlockAction(botAI); }
    static Action* break_chains(PlayerbotAI* botAI) { return new BreakChainsAction(botAI); }
    static Action* kill_guardians(PlayerbotAI* botAI) { return new KillGuardiansAction(botAI); }
    static Action* move_to_polarity(PlayerbotAI* botAI) { return new MoveToPolarityAction(botAI); }
    static Action* attack_tesla_coil(PlayerbotAI* botAI) { return new AttackTeslaCoilAction(botAI); }
};
//End By Leewheel

#endif
