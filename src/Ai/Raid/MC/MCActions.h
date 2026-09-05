/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MCACTIONS_H
#define PLAYERBOTS_MCACTIONS_H

#include "AttackAction.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

class McMoveFromGroupAction : public MovementAction
{
public:
    McMoveFromGroupAction(PlayerbotAI* botAI, std::string const name = "mc move from group")
        : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class McMoveFromBaronGeddonAction : public MovementAction
{
public:
    McMoveFromBaronGeddonAction(PlayerbotAI* botAI, std::string const name = "mc move from baron geddon")
        : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class McShazzrahMoveAwayAction : public MovementAction
{
public:
    McShazzrahMoveAwayAction(PlayerbotAI* botAI, std::string const name = "mc shazzrah move away")
        : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class McMoveFromLavaAction : public MovementAction
{
public:
    McMoveFromLavaAction(PlayerbotAI* botAI, std::string const name = "mc move from lava")
        : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class McGolemaggBackOffAction : public MovementAction
{
public:
    McGolemaggBackOffAction(PlayerbotAI* botAI, std::string const name = "mc golemagg back off")
        : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class McGolemaggMarkBossAction : public Action
{
public:
    McGolemaggMarkBossAction(PlayerbotAI* botAI, std::string const name = "mc golemagg mark boss")
        : Action(botAI, name) {};
    bool Execute(Event event) override;
};

class McGolemaggTankAction : public AttackAction
{
public:
    McGolemaggTankAction(PlayerbotAI* botAI, std::string const name)
        : AttackAction(botAI, name) {}
protected:
    bool MoveUnitToPosition(Unit* target, Position const& tankPosition, float maxDistance, float stepDistance = 3.0f);
    bool FindCoreRagers(Unit*& coreRager1, Unit*& coreRager2) const;
};

class McGolemaggMainTankAttackGolemaggAction : public McGolemaggTankAction
{
public:
    McGolemaggMainTankAttackGolemaggAction(PlayerbotAI* botAI, std::string const name = "mc golemagg main tank attack golemagg")
        : McGolemaggTankAction(botAI, name) {};
    bool Execute(Event event) override;
};

class McGolemaggAssistTankAttackCoreRagerAction : public McGolemaggTankAction
{
public:
    McGolemaggAssistTankAttackCoreRagerAction(PlayerbotAI* botAI, std::string const name = "mc golemagg assist tank attack core rager")
        : McGolemaggTankAction(botAI, name) {};
    bool Execute(Event event) override;
};

class McGolemaggHealerPositionAction : public MovementAction
{
public:
    McGolemaggHealerPositionAction(PlayerbotAI* botAI, std::string const name = "mc golemagg healer position")
        : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class McCoreHoundMarkAction : public Action
{
public:
    McCoreHoundMarkAction(PlayerbotAI* botAI, std::string const name = "mc core hound mark")
        : Action(botAI, name) {};
    Unit* GetTarget() override;
    bool Execute(Event event) override;
};

//By leewheel 2026年7月12日
// 自定义Boss: Smolder

class McSmolderAvoidFlameTsunamiAction : public MovementAction
{
public:
    McSmolderAvoidFlameTsunamiAction(PlayerbotAI* botAI, std::string const name = "mc smolder avoid flame tsunami")
        : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class McSmolderFearWardAction : public Action
{
public:
    McSmolderFearWardAction(PlayerbotAI* botAI, std::string const name = "mc smolder fear ward")
        : Action(botAI, name) {}
    bool Execute(Event event) override;
};

// 自定义Boss: Hazzrash

class McHazzrashEvocationAction : public Action
{
public:
    McHazzrashEvocationAction(PlayerbotAI* botAI, std::string const name = "mc hazzrash evocation")
        : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class McHazzrashRangedSpreadAction : public MovementAction
{
public:
    McHazzrashRangedSpreadAction(PlayerbotAI* botAI, std::string const name = "mc hazzrash ranged spread")
        : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};
//End By leewheel

#endif
