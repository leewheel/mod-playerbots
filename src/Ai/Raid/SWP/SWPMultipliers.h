//By leewheel 2026-07-08
/*
 * 太阳之井高地 (Sunwell Plateau) 乘数器声明
 * 作者: leewheel
 * 控制BOSS战中各类动作的优先级和启用/禁用
 */
//End By leewheel

#ifndef PLAYERBOTS_SWPMULTIPLIERS_H
#define PLAYERBOTS_SWPMULTIPLIERS_H

#include "Multiplier.h"

// 卡雷苟斯 (Kalecgos)
class KalecgosDelayDpsCooldownsMultiplier : public Multiplier
{
public:
    KalecgosDelayDpsCooldownsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kalecgos delay dps cooldowns multiplier") {}
    virtual float GetValue(Action* action);
};

class KalecgosControlMovementMultiplier : public Multiplier
{
public:
    KalecgosControlMovementMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kalecgos control movement multiplier") {}
    virtual float GetValue(Action* action);
};

// 布鲁塔卢斯 (Brutallus)
class BrutallusDelayDpsCooldownsMultiplier : public Multiplier
{
public:
    BrutallusDelayDpsCooldownsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "brutallus delay dps cooldowns multiplier") {}
    virtual float GetValue(Action* action);
};

class BrutallusControlMovementMultiplier : public Multiplier
{
public:
    BrutallusControlMovementMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "brutallus control movement multiplier") {}
    virtual float GetValue(Action* action);
};

// 菲米丝 (Felmyst)
class FelmystDelayDpsCooldownsMultiplier : public Multiplier
{
public:
    FelmystDelayDpsCooldownsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "felmyst delay dps cooldowns multiplier") {}
    virtual float GetValue(Action* action);
};

class FelmystControlMovementMultiplier : public Multiplier
{
public:
    FelmystControlMovementMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "felmyst control movement multiplier") {}
    virtual float GetValue(Action* action);
};

// 艾瑞达双子 (Eredar Twins)
class EredarTwinsDelayDpsCooldownsMultiplier : public Multiplier
{
public:
    EredarTwinsDelayDpsCooldownsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "eredar twins delay dps cooldowns multiplier") {}
    virtual float GetValue(Action* action);
};

class EredarTwinsControlMovementMultiplier : public Multiplier
{
public:
    EredarTwinsControlMovementMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "eredar twins control movement multiplier") {}
    virtual float GetValue(Action* action);
};

// 穆鲁 (Muru)
class MuruDelayDpsCooldownsMultiplier : public Multiplier
{
public:
    MuruDelayDpsCooldownsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "muru delay dps cooldowns multiplier") {}
    virtual float GetValue(Action* action);
};

class MuruControlMovementMultiplier : public Multiplier
{
public:
    MuruControlMovementMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "muru control movement multiplier") {}
    virtual float GetValue(Action* action);
};

// 基尔加丹 (Kil'jaeden)
class KiljaedenDelayDpsCooldownsMultiplier : public Multiplier
{
public:
    KiljaedenDelayDpsCooldownsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kil'jaeden delay dps cooldowns multiplier") {}
    virtual float GetValue(Action* action);
};

class KiljaedenControlMovementMultiplier : public Multiplier
{
public:
    KiljaedenControlMovementMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kil'jaeden control movement multiplier") {}
    virtual float GetValue(Action* action);
};

#endif
