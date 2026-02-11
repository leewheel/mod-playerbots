/*
 * This is Leewheel Script Project
 */

#ifndef _PLAYERBOT_NAXXSTRATEGY_H
#define _PLAYERBOT_NAXXSTRATEGY_H

#include "Strategy.h"

//By Leewheel 2026-02-11
class PlayerbotAI;

class NaxxramasStrategy : public Strategy
{
public:
    NaxxramasStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    const std::string getName() override { return "naxx"; }
    uint32 GetType() const override { return STRATEGY_TYPE_COMBAT; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class NaxxAnubrekhanStrategy : public Strategy
{
public:
    NaxxAnubrekhanStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    const std::string getName() override { return "naxx anubrekhan"; }
    uint32 GetType() const override { return STRATEGY_TYPE_COMBAT; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class NaxxFaerlinaStrategy : public Strategy
{
public:
    NaxxFaerlinaStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    const std::string getName() override { return "naxx faerlina"; }
    uint32 GetType() const override { return STRATEGY_TYPE_COMBAT; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class NaxxMaexxnaStrategy : public Strategy
{
public:
    NaxxMaexxnaStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    const std::string getName() override { return "naxx maexxna"; }
    uint32 GetType() const override { return STRATEGY_TYPE_COMBAT; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class NaxxPatchwerkStrategy : public Strategy
{
public:
    NaxxPatchwerkStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    const std::string getName() override { return "naxx patchwerk"; }
    uint32 GetType() const override { return STRATEGY_TYPE_COMBAT; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class NaxxGrobbulusStrategy : public Strategy
{
public:
    NaxxGrobbulusStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    const std::string getName() override { return "naxx grobbulus"; }
    uint32 GetType() const override { return STRATEGY_TYPE_COMBAT; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class NaxxGluthStrategy : public Strategy
{
public:
    NaxxGluthStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    const std::string getName() override { return "naxx gluth"; }
    uint32 GetType() const override { return STRATEGY_TYPE_COMBAT; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class NaxxNothStrategy : public Strategy
{
public:
    NaxxNothStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    const std::string getName() override { return "naxx noth"; }
    uint32 GetType() const override { return STRATEGY_TYPE_COMBAT; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class NaxxHeiganStrategy : public Strategy
{
public:
    NaxxHeiganStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    const std::string getName() override { return "naxx heigan"; }
    uint32 GetType() const override { return STRATEGY_TYPE_COMBAT; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class NaxxLoathebStrategy : public Strategy
{
public:
    NaxxLoathebStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    const std::string getName() override { return "naxx loatheb"; }
    uint32 GetType() const override { return STRATEGY_TYPE_COMBAT; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class NaxxRazuviousStrategy : public Strategy
{
public:
    NaxxRazuviousStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    const std::string getName() override { return "naxx razuvious"; }
    uint32 GetType() const override { return STRATEGY_TYPE_COMBAT; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class NaxxGothikStrategy : public Strategy
{
public:
    NaxxGothikStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    const std::string getName() override { return "naxx gothik"; }
    uint32 GetType() const override { return STRATEGY_TYPE_COMBAT; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class NaxxFourHorsemenStrategy : public Strategy
{
public:
    NaxxFourHorsemenStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    const std::string getName() override { return "naxx four horsemen"; }
    uint32 GetType() const override { return STRATEGY_TYPE_COMBAT; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class NaxxSapphironStrategy : public Strategy
{
public:
    NaxxSapphironStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    const std::string getName() override { return "naxx sapphiron"; }
    uint32 GetType() const override { return STRATEGY_TYPE_COMBAT; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class NaxxKelthuzadStrategy : public Strategy
{
public:
    NaxxKelthuzadStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    const std::string getName() override { return "naxx kelthuzad"; }
    uint32 GetType() const override { return STRATEGY_TYPE_COMBAT; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class NaxxThaddiusStrategy : public Strategy
{
public:
    NaxxThaddiusStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    const std::string getName() override { return "naxx thaddius"; }
    uint32 GetType() const override { return STRATEGY_TYPE_COMBAT; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};
//End By Leewheel

#endif
