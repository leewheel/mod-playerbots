#ifndef PLAYERBOTS_VANILLANAXXACTIONS_H
#define PLAYERBOTS_VANILLANAXXACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "VanillaNaxxBossHelper.h"
// By leewheel 2026-08-21: 上游 the-lab 删除 GenericActions 重构，此处不再需要引用该头文件 //End By leewheel

class VanillaGrobbulusGoBehindAction : public MovementAction
{
public:
    VanillaGrobbulusGoBehindAction(PlayerbotAI* ai, float distance = 24.0f, float delta_angle = M_PI / 8)
        : MovementAction(ai, "grobbulus go behind")
    {
        this->distance = distance;
        this->delta_angle = delta_angle;
    }
    virtual bool Execute(Event event);

protected:
    float distance, delta_angle;
};

class VanillaGrobbulusRotateAction : public RotateAroundTheCenterPointAction
{
public:
    VanillaGrobbulusRotateAction(PlayerbotAI* botAI)
        : RotateAroundTheCenterPointAction(botAI, "rotate grobbulus", 3281.23f, -3310.38f, 35.0f, 8, true, M_PI) {}
    virtual bool isUseful() override
    {
        return RotateAroundTheCenterPointAction::isUseful() && botAI->IsMainTank(bot) &&
               AI_VALUE2(bool, "has aggro", "boss target");
    }
    uint32 GetCurrWaypoint() override;
};

class VanillaGrobbulusMoveCenterAction : public MoveInsideAction
{
public:
    VanillaGrobbulusMoveCenterAction(PlayerbotAI* ai) : MoveInsideAction(ai, 3281.23f, -3310.38f, 5.0f) {}
};

class VanillaGrobbulusMoveAwayAction : public MovementAction
{
public:
    VanillaGrobbulusMoveAwayAction(PlayerbotAI* ai, float distance = 18.0f)
        : MovementAction(ai, "grobbulus move away"), distance(distance)
    {
    }
    bool Execute(Event event) override;

private:
    float distance;
};

class VanillaThaddiusAttackNearestPetAction : public AttackAction
{
public:
    VanillaThaddiusAttackNearestPetAction(PlayerbotAI* ai) : AttackAction(ai, "thaddius attack nearest pet"), helper(ai) {}
    virtual bool Execute(Event event);
    virtual bool isUseful();

private:
    VanillaThaddiusBossHelper helper;
};

class VanillaThaddiusMoveToPlatformAction : public MovementAction
{
public:
    VanillaThaddiusMoveToPlatformAction(PlayerbotAI* ai) : MovementAction(ai, "thaddius move to platform") {}
    virtual bool Execute(Event event);
    virtual bool isUseful();
};

class VanillaThaddiusMovePolarityAction : public MovementAction
{
public:
    VanillaThaddiusMovePolarityAction(PlayerbotAI* ai) : MovementAction(ai, "thaddius move polarity") {}
    virtual bool Execute(Event event);
    virtual bool isUseful();
};

class VanillaRazuviousUseObedienceCrystalAction : public MovementAction
{
public:
    VanillaRazuviousUseObedienceCrystalAction(PlayerbotAI* ai)
        : MovementAction(ai, "razuvious use obedience crystal"), helper(ai)
    {
    }
    bool Execute(Event event) override;

private:
    VanillaRazuviousBossHelper helper;
};

class VanillaRazuviousTargetAction : public AttackAction
{
public:
    VanillaRazuviousTargetAction(PlayerbotAI* ai) : AttackAction(ai, "razuvious target"), helper(ai) {}
    bool Execute(Event event) override;

private:
    VanillaRazuviousBossHelper helper;
};

class VanillaFourHorsemenAttractAlternativelyAction : public AttackAction
{
public:
    VanillaFourHorsemenAttractAlternativelyAction(PlayerbotAI* ai) : AttackAction(ai, "four horsemen attract alternatively"), helper(ai)
    {
    }
    bool Execute(Event event) override;

protected:
    VanillaFourHorsemenBossHelper helper;
};

class VanillaFourHorsemenAttackInOrderAction : public AttackAction
{
public:
    VanillaFourHorsemenAttackInOrderAction(PlayerbotAI* ai) : AttackAction(ai, "four horsemen attack in order"), helper(ai) {}
    bool Execute(Event event) override;

protected:
    VanillaFourHorsemenBossHelper helper;
};

class VanillaSapphironGroundPositionAction : public MovementAction
{
public:
    VanillaSapphironGroundPositionAction(PlayerbotAI* ai) : MovementAction(ai, "sapphiron ground position"), helper(ai) {}
    bool Execute(Event event) override;

protected:
    VanillaSapphironBossHelper helper;
};

class VanillaSapphironFlightPositionAction : public MovementAction
{
public:
    VanillaSapphironFlightPositionAction(PlayerbotAI* ai) : MovementAction(ai, "sapphiron flight position"), helper(ai) {}
    bool Execute(Event event) override;

protected:
    VanillaSapphironBossHelper helper;
    bool MoveToNearestIcebolt();
};

class VanillaKelthuzadChooseTargetAction : public AttackAction
{
public:
    VanillaKelthuzadChooseTargetAction(PlayerbotAI* ai) : AttackAction(ai, "kel'thuzad choose target"), helper(ai) {}
    virtual bool Execute(Event event);

private:
    VanillaKelthuzadBossHelper helper;
};

class VanillaKelthuzadPositionAction : public MovementAction
{
public:
    VanillaKelthuzadPositionAction(PlayerbotAI* ai) : MovementAction(ai, "kel'thuzad position"), helper(ai) {}
    virtual bool Execute(Event event);

private:
    VanillaKelthuzadBossHelper helper;
};

class VanillaAnubrekhanChooseTargetAction : public AttackAction
{
public:
    VanillaAnubrekhanChooseTargetAction(PlayerbotAI* ai) : AttackAction(ai, "anub'rekhan choose target") {}
    bool Execute(Event event) override;
};

class VanillaAnubrekhanPositionAction : public RotateAroundTheCenterPointAction
{
public:
    VanillaAnubrekhanPositionAction(PlayerbotAI* ai)
        : RotateAroundTheCenterPointAction(ai, "anub'rekhan position", 3272.49f, -3476.27f, 45.0f, 16) {}
    bool Execute(Event event) override;
};

class VanillaGluthChooseTargetAction : public AttackAction
{
public:
    VanillaGluthChooseTargetAction(PlayerbotAI* ai) : AttackAction(ai, "gluth choose target"), helper(ai) {}
    bool Execute(Event event) override;

private:
    VanillaGluthBossHelper helper;
};

class VanillaGluthPositionAction : public RotateAroundTheCenterPointAction
{
public:
    VanillaGluthPositionAction(PlayerbotAI* ai)
        : RotateAroundTheCenterPointAction(ai, "gluth position", 3293.61f, -3149.01f, 12.0f, 12), helper(ai) {}
    bool Execute(Event event) override;

private:
    VanillaGluthBossHelper helper;
};

class VanillaGluthSlowdownAction : public Action
{
public:
    VanillaGluthSlowdownAction(PlayerbotAI* ai) : Action(ai, "gluth slowdown"), helper(ai) {}
    bool Execute(Event event) override;

private:
    VanillaGluthBossHelper helper;
};

class VanillaLoathebPositionAction : public MovementAction
{
public:
    VanillaLoathebPositionAction(PlayerbotAI* ai) : MovementAction(ai, "loatheb position"), helper(ai) {}
    virtual bool Execute(Event event);

private:
    VanillaLoathebBossHelper helper;
};

class VanillaLoathebChooseTargetAction : public AttackAction
{
public:
    VanillaLoathebChooseTargetAction(PlayerbotAI* ai) : AttackAction(ai, "loatheb choose target"), helper(ai) {}
    virtual bool Execute(Event event);

private:
    VanillaLoathebBossHelper helper;
};

#endif
