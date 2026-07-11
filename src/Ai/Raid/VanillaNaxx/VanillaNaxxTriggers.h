
#ifndef PLAYERBOTS_VANILLANAXXTRIGGERS_H
#define PLAYERBOTS_VANILLANAXXTRIGGERS_H

#include "EventMap.h"
#include "GenericTriggers.h"
#include "PlayerbotAIConfig.h"
#include "VanillaNaxxBossHelper.h"
#include "Trigger.h"

class VanillaMutatingInjectionTrigger : public HasAuraTrigger
{
public:
    VanillaMutatingInjectionTrigger(PlayerbotAI* ai) : HasAuraTrigger(ai, "mutating injection", 1) {}
};

class VanillaMutatingInjectionMeleeTrigger : public VanillaMutatingInjectionTrigger
{
public:
    VanillaMutatingInjectionMeleeTrigger(PlayerbotAI* ai) : VanillaMutatingInjectionTrigger(ai) {}
    bool IsActive() override;
};

class VanillaMutatingInjectionRangedTrigger : public VanillaMutatingInjectionTrigger
{
public:
    VanillaMutatingInjectionRangedTrigger(PlayerbotAI* ai) : VanillaMutatingInjectionTrigger(ai) {}
    bool IsActive() override;
};

class VanillaAuraRemovedTrigger : public Trigger
{
public:
    VanillaAuraRemovedTrigger(PlayerbotAI* botAI, std::string name) : Trigger(botAI, name, 1)
    {
        this->prev_check = false;
    }
    virtual bool IsActive() override;

protected:
    bool prev_check;
};

class VanillaMutatingInjectionRemovedTrigger : public HasNoAuraTrigger
{
public:
    VanillaMutatingInjectionRemovedTrigger(PlayerbotAI* ai) : HasNoAuraTrigger(ai, "mutating injection") {}
    virtual bool IsActive();
};

class VanillaGrobbulusCloudTrigger : public Trigger
{
public:
    VanillaGrobbulusCloudTrigger(PlayerbotAI* ai) : Trigger(ai, "grobbulus cloud event"), last_cloud_ms(0) {}
    bool IsActive() override;

private:
    uint32 last_cloud_ms;
    static constexpr uint32 CloudRotationDelayMs = 15000;
};

class VanillaRazuviousTankTrigger : public Trigger
{
public:
    VanillaRazuviousTankTrigger(PlayerbotAI* ai) : Trigger(ai, "instructor razuvious tank"), helper(ai) {}
    bool IsActive() override;

private:
    VanillaRazuviousBossHelper helper;
};

class VanillaRazuviousNontankTrigger : public Trigger
{
public:
    VanillaRazuviousNontankTrigger(PlayerbotAI* ai) : Trigger(ai, "instructor razuvious non-tank"), helper(ai) {}
    bool IsActive() override;

private:
    VanillaRazuviousBossHelper helper;
};

class VanillaKelthuzadTrigger : public Trigger
{
public:
    VanillaKelthuzadTrigger(PlayerbotAI* ai) : Trigger(ai, "kel'thuzad trigger"), helper(ai) {}
    bool IsActive() override;

private:
    VanillaKelthuzadBossHelper helper;
};

class VanillaAnubrekhanTrigger : public Trigger
{
public:
    VanillaAnubrekhanTrigger(PlayerbotAI* ai) : Trigger(ai, "anub'rekhan") {}
    bool IsActive() override;
};

class VanillaFaerlinaTrigger : public Trigger
{
public:
    VanillaFaerlinaTrigger(PlayerbotAI* ai) : Trigger(ai, "faerlina") {}
    bool IsActive() override;
};

class VanillaMaexxnaTrigger : public Trigger
{
public:
    VanillaMaexxnaTrigger(PlayerbotAI* ai) : Trigger(ai, "maexxna") {}
    bool IsActive() override;
};

class VanillaThaddiusPhasePetTrigger : public Trigger
{
public:
    VanillaThaddiusPhasePetTrigger(PlayerbotAI* ai) : Trigger(ai, "thaddius phase pet"), helper(ai) {}
    bool IsActive() override;

private:
    VanillaThaddiusBossHelper helper;
};

class VanillaThaddiusPhasePetLoseAggroTrigger : public VanillaThaddiusPhasePetTrigger
{
public:
    VanillaThaddiusPhasePetLoseAggroTrigger(PlayerbotAI* ai) : VanillaThaddiusPhasePetTrigger(ai) {}
    virtual bool IsActive()
    {
        Unit* target = AI_VALUE(Unit*, "current target");
        return VanillaThaddiusPhasePetTrigger::IsActive() && botAI->IsTank(bot) && target && target->GetVictim() != bot;
    }
};

class VanillaThaddiusPhaseTransitionTrigger : public Trigger
{
public:
    VanillaThaddiusPhaseTransitionTrigger(PlayerbotAI* ai) : Trigger(ai, "thaddius phase transition"), helper(ai) {}
    bool IsActive() override;

private:
    VanillaThaddiusBossHelper helper;
};

class VanillaThaddiusPhaseThaddiusTrigger : public Trigger
{
public:
    VanillaThaddiusPhaseThaddiusTrigger(PlayerbotAI* ai) : Trigger(ai, "thaddius phase thaddius"), helper(ai) {}
    bool IsActive() override;

private:
    VanillaThaddiusBossHelper helper;
};

class VanillaFourHorsemenAttractorsTrigger : public Trigger
{
public:
    VanillaFourHorsemenAttractorsTrigger(PlayerbotAI* ai) : Trigger(ai, "four horsemen attractors"), helper(ai) {}
    bool IsActive() override;

private:
    VanillaFourHorsemenBossHelper helper;
};

class VanillaFourHorsemenExceptAttractorsTrigger : public Trigger
{
public:
    VanillaFourHorsemenExceptAttractorsTrigger(PlayerbotAI* ai) : Trigger(ai, "four horsemen except attractors"), helper(ai) {}
    bool IsActive() override;

private:
    VanillaFourHorsemenBossHelper helper;
};

class VanillaSapphironGroundTrigger : public Trigger
{
public:
    VanillaSapphironGroundTrigger(PlayerbotAI* ai) : Trigger(ai, "sapphiron ground"), helper(ai) {}
    bool IsActive() override;

private:
    VanillaSapphironBossHelper helper;
};

class VanillaSapphironFlightTrigger : public Trigger
{
public:
    VanillaSapphironFlightTrigger(PlayerbotAI* ai) : Trigger(ai, "sapphiron flight"), helper(ai) {}
    bool IsActive() override;

private:
    VanillaSapphironBossHelper helper;
};

class VanillaGluthTrigger : public Trigger
{
public:
    VanillaGluthTrigger(PlayerbotAI* ai) : Trigger(ai, "gluth trigger"), helper(ai) {}
    bool IsActive() override;

private:
    VanillaGluthBossHelper helper;
};

class VanillaGluthMainTankMortalWoundTrigger : public Trigger
{
public:
    VanillaGluthMainTankMortalWoundTrigger(PlayerbotAI* ai) : Trigger(ai, "gluth main tank mortal wound trigger"), helper(ai) {}
    bool IsActive() override;

private:
    VanillaGluthBossHelper helper;
};

class VanillaLoathebTrigger : public Trigger
{
public:
    VanillaLoathebTrigger(PlayerbotAI* ai) : Trigger(ai, "loatheb"), helper(ai) {}
    bool IsActive() override;

private:
    VanillaLoathebBossHelper helper;
};

#endif
