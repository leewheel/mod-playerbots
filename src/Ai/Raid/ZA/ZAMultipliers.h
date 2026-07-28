/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ZAMULTIPLIERS_H
#define PLAYERBOTS_ZAMULTIPLIERS_H

#include "Multiplier.h"

// Akil'zon <Eagle Avatar>

class AkilzonDisableCombatFormationMoveMultiplier : public Multiplier
{
public:
    AkilzonDisableCombatFormationMoveMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "akil'zon disable combat formation move") {}
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    float GetValue(Action* action) override;
};

class AkilzonStayInEyeOfTheStormMultiplier : public Multiplier
{
public:
    AkilzonStayInEyeOfTheStormMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "akil'zon stay in eye of the storm") {}
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    float GetValue(Action* action) override;
};

// Nalorakk <Bear Avatar>

class NalorakkDisableTankActionsMultiplier : public Multiplier
{
public:
    NalorakkDisableTankActionsMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "nalorakk disable tank actions") {}
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    float GetValue(Action* action) override;
};

class NalorakkControlMisdirectionMultiplier : public Multiplier
{
public:
    NalorakkControlMisdirectionMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "nalorakk control misdirection") {}
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    float GetValue(Action* action) override;
};

// Jan'alai <Dragonhawk Avatar>

class JanalaiDisableTankActionsMultiplier : public Multiplier
{
public:
    JanalaiDisableTankActionsMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "jan'alai disable tank actions") {}
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    float GetValue(Action* action) override;
};

class JanalaiDisableCombatFormationMoveMultiplier : public Multiplier
{
public:
    JanalaiDisableCombatFormationMoveMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "jan'alai disable combat formation move") {}
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    float GetValue(Action* action) override;
};

class JanalaiStayAwayFromFireBombsMultiplier : public Multiplier
{
public:
    JanalaiStayAwayFromFireBombsMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "jan'alai stay away from fire bombs") {}
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    float GetValue(Action* action) override;
};

class JanalaiDoNotCrowdControlHatchersMultiplier : public Multiplier
{
public:
    JanalaiDoNotCrowdControlHatchersMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "jan'alai do not crowd control hatchers") {}
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    float GetValue(Action* action) override;
};

class JanalaiDelayBloodlustAndHeroismMultiplier : public Multiplier
{
public:
    JanalaiDelayBloodlustAndHeroismMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "jan'alai delay bloodlust and heroism") {}
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    float GetValue(Action* action) override;
};

// Halazzi <Lynx Avatar>

class HalazziDisableTankActionsMultiplier : public Multiplier
{
public:
    HalazziDisableTankActionsMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "halazzi disable tank actions") {}
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    float GetValue(Action* action) override;
};

class HalazziControlMisdirectionMultiplier : public Multiplier
{
public:
    HalazziControlMisdirectionMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "halazzi control misdirection") {}
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    float GetValue(Action* action) override;
};

// Hex Lord Malacrass

class HexLordMalacrassAvoidWhirlwindMultiplier : public Multiplier
{
public:
    HexLordMalacrassAvoidWhirlwindMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "hex lord malacrass avoid whirlwind") {}
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    float GetValue(Action* action) override;
};

class HexLordMalacrassDoNotDispelUnstableAfflictionMultiplier : public Multiplier
{
public:
    HexLordMalacrassDoNotDispelUnstableAfflictionMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "hex lord malacrass do not dispel unstable affliction") {}
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    float GetValue(Action* action) override;
};

class HexLordMalacrassStopAttackingDuringSpellReflectionMultiplier : public Multiplier
{
public:
    HexLordMalacrassStopAttackingDuringSpellReflectionMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "hex lord malacrass stop attacking during spell reflection") {}
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    float GetValue(Action* action) override;
};

// Zul'jin

class ZuljinDisableTankFaceMultiplier : public Multiplier
{
public:
    ZuljinDisableTankFaceMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "zul'jin disable tank face") {}
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    float GetValue(Action* action) override;
};

class ZuljinAvoidWhirlwindMultiplier : public Multiplier
{
public:
    ZuljinAvoidWhirlwindMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "zul'jin avoid whirlwind") {}
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    float GetValue(Action* action) override;
};

class ZuljinDisableAvoidAoeMultiplier : public Multiplier
{
public:
    ZuljinDisableAvoidAoeMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "zul'jin disable avoid aoe") {}
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    float GetValue(Action* action) override;
};

class ZuljinDelayBloodlustAndHeroismMultiplier : public Multiplier
{
public:
    ZuljinDelayBloodlustAndHeroismMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "zul'jin delay bloodlust and heroism") {}
//By leewheel 2026-07-28 - 使用override替代virtual，遵循C++11最佳实践
    float GetValue(Action* action) override;
};

#endif
