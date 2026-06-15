#include "UnlockTradedItemAction.h"
#include "PlayerbotAI.h"
#include "TradeData.h"
#include "SpellInfo.h"

inline constexpr uint32_t PICK_LOCK_SPELL_ID = 1804;

bool UnlockTradedItemAction::Execute(Event /*event*/)
{
    Player* trader = bot->GetTrader();
    if (!trader)
        return false; // No active trade session

    TradeData* tradeData = trader->GetTradeData();
    if (!tradeData)
        return false; // No trade data available

    Item* lockbox = tradeData->GetItem(TRADE_SLOT_NONTRADED);
    if (!lockbox)
    {
        botAI->TellError("请勿交易栏中没有物品。");
        return false;
    }

    if (!CanUnlockItem(lockbox))
    {
        botAI->TellError("无法解锁此物品。");
        return false;
    }

    UnlockItem(lockbox);
    return true;
}

bool UnlockTradedItemAction::CanUnlockItem(Item* item)
{
    if (!item)
        return false;

    ItemTemplate const* itemTemplate = item->GetTemplate();
    if (!itemTemplate)
        return false;

    // Ensure the bot is a rogue and has Lockpicking skill
    if (bot->getClass() != CLASS_ROGUE || !botAI->HasSkill(SKILL_LOCKPICKING))
        return false;

    // Ensure the item is actually locked
    if (itemTemplate->LockID == 0 || !item->IsLocked())
        return false;

    // Check if the bot's Lockpicking skill is high enough
    uint32 lockId = itemTemplate->LockID;
    LockEntry const* lockInfo = sLockStore.LookupEntry(lockId);
    if (!lockInfo)
        return false;

    uint32 botSkill = bot->GetSkillValue(SKILL_LOCKPICKING);
    for (uint8 j = 0; j < 8; ++j)
    {
        if (lockInfo->Type[j] == LOCK_KEY_SKILL && SkillByLockType(LockType(lockInfo->Index[j])) == SKILL_LOCKPICKING)
        {
            uint32 requiredSkill = lockInfo->Skill[j];
            if (botSkill >= requiredSkill)
                return true;
            else
            {
                std::ostringstream out;
                out << "Lockpicking skill too low (" << botSkill << "/" << requiredSkill << ") to unlock: "
                    << item->GetTemplate()->Name1;
                botAI->TellMaster(out.str());
            }
        }
    }

    return false;
}

void UnlockTradedItemAction::UnlockItem(Item* item)
{
    if (!bot->HasSpell(PICK_LOCK_SPELL_ID))
    {
        botAI->TellError("无法解锁，缺少开锁技能。");
        return;
    }

    // Use CastSpell to unlock the item
    if (botAI->CastSpell(PICK_LOCK_SPELL_ID, bot->GetTrader(), item)) // Unit target is trader
    {
        std::ostringstream out;
        out << "Picking Lock on traded item: " << item->GetTemplate()->Name1;
        botAI->TellMaster(out.str());
    }
    else
    {
        botAI->TellError("施放开锁失败。");
    }
}
