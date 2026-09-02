/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SSCMULTIPLIERS_H
#define PLAYERBOTS_SSCMULTIPLIERS_H

#include "Multiplier.h"

// Trash

class UnderbogColossusEscapeToxicPoolMultiplier : public Multiplier
{
public:
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    UnderbogColossusEscapeToxicPoolMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "underbog colossus escape toxic pool") {}
    float GetValue(Action* action) override;
};

// Hydross the Unstable <Duke of Currents>

class HydrossTheUnstableDisableTankActionsMultiplier : public Multiplier
{
public:
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    HydrossTheUnstableDisableTankActionsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "hydross the unstable disable tank actions") {}
    float GetValue(Action* action) override;
};

class HydrossTheUnstableWaitForDpsMultiplier : public Multiplier
{
public:
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    HydrossTheUnstableWaitForDpsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "hydross the unstable wait for dps") {}
    float GetValue(Action* action) override;
};

class HydrossTheUnstableControlMisdirectionMultiplier : public Multiplier
{
public:
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    HydrossTheUnstableControlMisdirectionMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "hydross the unstable control misdirection") {}
    float GetValue(Action* action) override;
};

// The Lurker Below

class TheLurkerBelowStayAwayFromSpoutMultiplier : public Multiplier
{
public:
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    TheLurkerBelowStayAwayFromSpoutMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "the lurker below stay away from spout") {}
    float GetValue(Action* action) override;
};

class TheLurkerBelowMaintainRangedSpreadMultiplier : public Multiplier
{
public:
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    TheLurkerBelowMaintainRangedSpreadMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "the lurker below maintain ranged spread") {}
    float GetValue(Action* action) override;
};

class TheLurkerBelowDisableTankAssistMultiplier : public Multiplier
{
public:
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    TheLurkerBelowDisableTankAssistMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "the lurker below disable tank assist") {}
    float GetValue(Action* action) override;
};

// Leotheras the Blind

class LeotherasTheBlindAvoidWhirlwindMultiplier : public Multiplier
{
public:
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    LeotherasTheBlindAvoidWhirlwindMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "leotheras the blind avoid whirlwind") {}
    float GetValue(Action* action) override;
};

class LeotherasTheBlindDisableTankActionsMultiplier : public Multiplier
{
public:
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    LeotherasTheBlindDisableTankActionsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "leotheras the blind disable tank actions") {}
    float GetValue(Action* action) override;
};

class LeotherasTheBlindMeleeDpsAvoidChaosBlastMultiplier : public Multiplier
{
public:
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    LeotherasTheBlindMeleeDpsAvoidChaosBlastMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "leotheras the blind melee dps avoid chaos blast") {}
    float GetValue(Action* action) override;
};

class LeotherasTheBlindFocusOnInnerDemonMultiplier : public Multiplier
{
public:
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    LeotherasTheBlindFocusOnInnerDemonMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "leotheras the blind focus on inner demon") {}
    float GetValue(Action* action) override;
};

class LeotherasTheBlindWaitForDpsMultiplier : public Multiplier
{
public:
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    LeotherasTheBlindWaitForDpsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "leotheras the blind wait for dps") {}
    float GetValue(Action* action) override;
};

class LeotherasTheBlindDelayBloodlustAndHeroismMultiplier : public Multiplier
{
public:
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    LeotherasTheBlindDelayBloodlustAndHeroismMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "leotheras the blind delay bloodlust and heroism") {}
    float GetValue(Action* action) override;
};

// Fathom-Lord Karathress

class FathomLordKarathressDisableTankActionsMultiplier : public Multiplier
{
public:
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    FathomLordKarathressDisableTankActionsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "fathom-lord karathress disable tank actions") {}
    float GetValue(Action* action) override;
};

class FathomLordKarathressDisableAoeMultiplier : public Multiplier
{
public:
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    FathomLordKarathressDisableAoeMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "fathom-lord karathress disable aoe") {}
    float GetValue(Action* action) override;
};

class FathomLordKarathressControlMisdirectionMultiplier : public Multiplier
{
public:
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    FathomLordKarathressControlMisdirectionMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "fathom-lord karathress control misdirection") {}
    float GetValue(Action* action) override;
};

class FathomLordKarathressWaitForDpsMultiplier : public Multiplier
{
public:
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    FathomLordKarathressWaitForDpsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "fathom-lord karathress wait for dps") {}
    float GetValue(Action* action) override;
};

class FathomLordKarathressCaribdisTankHealerMaintainPositionMultiplier : public Multiplier
{
public:
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    FathomLordKarathressCaribdisTankHealerMaintainPositionMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "fathom-lord karathress caribdis tank healer maintain position") {}
    float GetValue(Action* action) override;
};

// Morogrim Tidewalker

class MorogrimTidewalkerDelayBloodlustAndHeroismMultiplier : public Multiplier
{
public:
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    MorogrimTidewalkerDelayBloodlustAndHeroismMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "morogrim tidewalker delay bloodlust and heroism") {}
    float GetValue(Action* action) override;
};

class MorogrimTidewalkerDisableTankActionsMultiplier : public Multiplier
{
public:
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    MorogrimTidewalkerDisableTankActionsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "morogrim tidewalker disable tank actions") {}
    float GetValue(Action* action) override;
};

class MorogrimTidewalkerMaintainPhase2StackingMultiplier : public Multiplier
{
public:
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    MorogrimTidewalkerMaintainPhase2StackingMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "morogrim tidewalker maintain phase2 stacking") {}
    float GetValue(Action* action) override;
};

// Lady Vashj <Coilfang Matron>

class LadyVashjDelayCooldownsMultiplier : public Multiplier
{
public:
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    LadyVashjDelayCooldownsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "lady vashj delay cooldowns") {}
    float GetValue(Action* action) override;
};

class LadyVashjMainTankGroupShamanUseGroundingTotemMultiplier : public Multiplier
{
public:
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    LadyVashjMainTankGroupShamanUseGroundingTotemMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "lady vashj main tank group shaman use grounding totem") {}
    float GetValue(Action* action) override;
};

class LadyVashjMaintainPhase1RangedSpreadMultiplier : public Multiplier
{
public:
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    LadyVashjMaintainPhase1RangedSpreadMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "lady vashj maintain phase1 ranged spread") {}
    float GetValue(Action* action) override;
};

class LadyVashjStaticChargeStayAwayFromGroupMultiplier : public Multiplier
{
public:
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    LadyVashjStaticChargeStayAwayFromGroupMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "lady vashj static charge stay away from group") {}
    float GetValue(Action* action) override;
};

class LadyVashjDoNotLootTheTaintedCoreMultiplier : public Multiplier
{
public:
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    LadyVashjDoNotLootTheTaintedCoreMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "lady vashj do not loot the tainted core") {}
    float GetValue(Action* action) override;
};

class LadyVashjCorePassersPrioritizePositioningMultiplier : public Multiplier
{
public:
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    LadyVashjCorePassersPrioritizePositioningMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "lady vashj core passers prioritize positioning") {}
    float GetValue(Action* action) override;
};

class LadyVashjDisableAutomaticTargetingAndMovementModifier : public Multiplier
{
public:
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    LadyVashjDisableAutomaticTargetingAndMovementModifier(PlayerbotAI* botAI)
        : Multiplier(botAI, "lady vashj disable automatic targeting and movement") {}
    float GetValue(Action* action) override;
};

#endif
