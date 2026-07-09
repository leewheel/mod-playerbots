/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option) any later version.
 */

#ifndef PLAYERBOTS_VIMGOLSTRATEGY_H
#define PLAYERBOTS_VIMGOLSTRATEGY_H

#include "Strategy.h"

class VimgolStrategy : public Strategy
{
public:
    VimgolStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "vimgol"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
