/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "HireAction.h"

#include "Event.h"
#include "RandomPlayerbotMgr.h"
#include "PlayerbotAI.h"

bool HireAction::Execute(Event /*event*/)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    if (!RandomPlayerbotMgr::instance().IsRandomBot(bot))
        return false;

    uint32 account = master->GetSession()->GetAccountId();
    QueryResult results = CharacterDatabase.Query("SELECT COUNT(*) FROM characters WHERE account = {}", account);

    uint32 charCount = 10;
    if (results)
    {
        Field* fields = results->Fetch();
        charCount = uint32(fields[0].Get<uint64>());
    }

    if (charCount >= 10)
    {
        botAI->TellMaster("你已达到角色数量上限");
        return false;
    }

    if (bot->GetLevel() > master->GetLevel())
    {
        botAI->TellMaster("你无法雇佣等级高于你的角色");
        return false;
    }

    uint32 discount = RandomPlayerbotMgr::instance().GetTradeDiscount(bot, master);
    uint32 m = 1 + (bot->GetLevel() / 10);
    uint32 moneyReq = m * 5000 * bot->GetLevel();
    if (discount < moneyReq)
    {
        std::ostringstream out;
        out << "你还不能雇佣我——我们还不够熟。请确保交易折扣至少达到 " << chat->formatMoney(moneyReq);
        botAI->TellMaster(out.str());
        return false;
    }

    botAI->TellMaster("我将在你下次登录时加入");

    bot->SetMoney(moneyReq);
    RandomPlayerbotMgr::instance().Remove(bot);
    CharacterDatabase.Execute("UPDATE characters SET account = {} WHERE guid = {}", account,
                              bot->GetGUID().GetCounter());

    return true;
}
