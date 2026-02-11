/*
 * This is Leewheel Script Project
 */

#ifndef _PLAYERBOT_NAXXACTIONS_H
#define _PLAYERBOT_NAXXACTIONS_H

#include "Action.h"
#include "AttackAction.h"

//By Leewheel 2026-02-11
class PlayerbotAI;

class SpreadOutAction : public AttackAction
{
public:
    SpreadOutAction(PlayerbotAI* botAI) : AttackAction(botAI, "spread out") {}
    bool Execute(Event event) override;
};

class AttackCryptGuardAction : public AttackAction
{
public:
    AttackCryptGuardAction(PlayerbotAI* botAI) : AttackAction(botAI, "attack crypt guard") {}
    bool Execute(Event event) override;
};

class UseWidowsEmbraceAction : public AttackAction
{
public:
    UseWidowsEmbraceAction(PlayerbotAI* botAI) : AttackAction(botAI, "use widow's embrace") {}
    bool Execute(Event event) override;
};

class KillWorshipperAction : public AttackAction
{
public:
    KillWorshipperAction(PlayerbotAI* botAI) : AttackAction(botAI, "kill worshipper") {}
    bool Execute(Event event) override;
};

class FreeWebWrapAction : public AttackAction
{
public:
    FreeWebWrapAction(PlayerbotAI* botAI) : AttackAction(botAI, "free web wrap") {}
    bool Execute(Event event) override;
};

class DispelWebSprayAction : public AttackAction
{
public:
    DispelWebSprayAction(PlayerbotAI* botAI) : AttackAction(botAI, "dispel web spray") {}
    bool Execute(Event event) override;
};

class KillSpiderlingsAction : public AttackAction
{
public:
    KillSpiderlingsAction(PlayerbotAI* botAI) : AttackAction(botAI, "kill spiderlings") {}
    bool Execute(Event event) override;
};

class PositionForHatefulAction : public AttackAction
{
public:
    PositionForHatefulAction(PlayerbotAI* botAI) : AttackAction(botAI, "position for hateful") {}
    bool Execute(Event event) override;
};

class BurnCooldownsAction : public AttackAction
{
public:
    BurnCooldownsAction(PlayerbotAI* botAI) : AttackAction(botAI, "burn cooldowns") {}
    bool Execute(Event event) override;
};

class MoveAwayAction : public AttackAction
{
public:
    MoveAwayAction(PlayerbotAI* botAI) : AttackAction(botAI, "move away") {}
    bool Execute(Event event) override;
};

class AvoidPoisonCloudAction : public AttackAction
{
public:
    AvoidPoisonCloudAction(PlayerbotAI* botAI) : AttackAction(botAI, "avoid poison cloud") {}
    bool Execute(Event event) override;
};

class KillZombiesAction : public AttackAction
{
public:
    KillZombiesAction(PlayerbotAI* botAI) : AttackAction(botAI, "kill zombies") {}
    bool Execute(Event event) override;
};

class TankRotationAction : public AttackAction
{
public:
    TankRotationAction(PlayerbotAI* botAI) : AttackAction(botAI, "tank rotation") {}
    bool Execute(Event event) override;
};

class PositionNothAction : public AttackAction
{
public:
    PositionNothAction(PlayerbotAI* botAI) : AttackAction(botAI, "position noth") {}
    bool Execute(Event event) override;
};

class AoeSkeletonsAction : public AttackAction
{
public:
    AoeSkeletonsAction(PlayerbotAI* botAI) : AttackAction(botAI, "aoe skeletons") {}
    bool Execute(Event event) override;
};

class DanceHeiganAction : public AttackAction
{
public:
    DanceHeiganAction(PlayerbotAI* botAI) : AttackAction(botAI, "dance heigan") {}
    bool Execute(Event event) override;
};

class MoveToSafeZoneAction : public AttackAction
{
public:
    MoveToSafeZoneAction(PlayerbotAI* botAI) : AttackAction(botAI, "move to safe zone") {}
    bool Execute(Event event) override;
};

class HealRotationAction : public AttackAction
{
public:
    HealRotationAction(PlayerbotAI* botAI) : AttackAction(botAI, "heal rotation") {}
    bool Execute(Event event) override;
};

class KillSporeAction : public AttackAction
{
public:
    KillSporeAction(PlayerbotAI* botAI) : AttackAction(botAI, "kill spore") {}
    bool Execute(Event event) override;
};

class DisruptShoutAction : public AttackAction
{
public:
    DisruptShoutAction(PlayerbotAI* botAI) : AttackAction(botAI, "disrupt shout") {}
    bool Execute(Event event) override;
};

class MindControlUnderstudyAction : public AttackAction
{
public:
    MindControlUnderstudyAction(PlayerbotAI* botAI) : AttackAction(botAI, "mind control understudy") {}
    bool Execute(Event event) override;
};

class PositionGothikAction : public AttackAction
{
public:
    PositionGothikAction(PlayerbotAI* botAI) : AttackAction(botAI, "position gothik") {}
    bool Execute(Event event) override;
};

class CentralRoomAction : public AttackAction
{
public:
    CentralRoomAction(PlayerbotAI* botAI) : AttackAction(botAI, "central room") {}
    bool Execute(Event event) override;
};

class SwitchTargetsAction : public AttackAction
{
public:
    SwitchTargetsAction(PlayerbotAI* botAI) : AttackAction(botAI, "switch targets") {}
    bool Execute(Event event) override;
};

class PositionIceBlockAction : public AttackAction
{
public:
    PositionIceBlockAction(PlayerbotAI* botAI) : AttackAction(botAI, "position ice block") {}
    bool Execute(Event event) override;
};

class MoveBehindIceBlockAction : public AttackAction
{
public:
    MoveBehindIceBlockAction(PlayerbotAI* botAI) : AttackAction(botAI, "move behind ice block") {}
    bool Execute(Event event) override;
};

class BreakChainsAction : public AttackAction
{
public:
    BreakChainsAction(PlayerbotAI* botAI) : AttackAction(botAI, "break chains") {}
    bool Execute(Event event) override;
};

class KillGuardiansAction : public AttackAction
{
public:
    KillGuardiansAction(PlayerbotAI* botAI) : AttackAction(botAI, "kill guardians") {}
    bool Execute(Event event) override;
};

class MoveToPolarityAction : public AttackAction
{
public:
    MoveToPolarityAction(PlayerbotAI* botAI) : AttackAction(botAI, "move to polarity") {}
    bool Execute(Event event) override;
};

class AttackTeslaCoilAction : public AttackAction
{
public:
    AttackTeslaCoilAction(PlayerbotAI* botAI) : AttackAction(botAI, "attack tesla coil") {}
    bool Execute(Event event) override;
};
//End By Leewheel

#endif
