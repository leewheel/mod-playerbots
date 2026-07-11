
#ifndef PLAYERBOTS_VANILLANAXXMULTIPLIERS_H
#define PLAYERBOTS_VANILLANAXXMULTIPLIERS_H

#include "Multiplier.h"
#include "VanillaNaxxBossHelper.h"

class VanillaGrobbulusMultiplier : public Multiplier
{
public:
    VanillaGrobbulusMultiplier(PlayerbotAI* ai) : Multiplier(ai, "grobbulus") {}

public:
    virtual float GetValue(Action* action);
};

class VanillaLoathebGenericMultiplier : public Multiplier
{
public:
    VanillaLoathebGenericMultiplier(PlayerbotAI* ai) : Multiplier(ai, "loatheb generic") {}

public:
    virtual float GetValue(Action* action);
};

class VanillaThaddiusGenericMultiplier : public Multiplier
{
public:
    VanillaThaddiusGenericMultiplier(PlayerbotAI* ai) : Multiplier(ai, "thaddius generic"), helper(ai) {}

public:
    virtual float GetValue(Action* action);

private:
    VanillaThaddiusBossHelper helper;
};

class VanillaSapphironGenericMultiplier : public Multiplier
{
public:
    VanillaSapphironGenericMultiplier(PlayerbotAI* ai) : Multiplier(ai, "sapphiron generic"), helper(ai) {}

    virtual float GetValue(Action* action);

private:
    VanillaSapphironBossHelper helper;
};

class VanillaInstructorRazuviousGenericMultiplier : public Multiplier
{
public:
    VanillaInstructorRazuviousGenericMultiplier(PlayerbotAI* ai) : Multiplier(ai, "instructor razuvious generic"), helper(ai) {}
    virtual float GetValue(Action* action);

private:
    VanillaRazuviousBossHelper helper;
};

class VanillaKelthuzadGenericMultiplier : public Multiplier
{
public:
    VanillaKelthuzadGenericMultiplier(PlayerbotAI* ai) : Multiplier(ai, "kelthuzad generic"), helper(ai) {}
    virtual float GetValue(Action* action);

private:
    VanillaKelthuzadBossHelper helper;
};

class VanillaAnubrekhanGenericMultiplier : public Multiplier
{
public:
    VanillaAnubrekhanGenericMultiplier(PlayerbotAI* ai) : Multiplier(ai, "anubrekhan generic") {}

public:
    virtual float GetValue(Action* action);
};

class VanillaFourHorsemenGenericMultiplier : public Multiplier
{
public:
    VanillaFourHorsemenGenericMultiplier(PlayerbotAI* ai) : Multiplier(ai, "four horsemen generic") {}

public:
    virtual float GetValue(Action* action);
};

class VanillaGluthGenericMultiplier : public Multiplier
{
public:
    VanillaGluthGenericMultiplier(PlayerbotAI* ai) : Multiplier(ai, "gluth generic"), helper(ai) {}
    float GetValue(Action* action) override;

private:
    VanillaGluthBossHelper helper;
};

#endif
