#include "UnlockTradedItemAction.h"
#include "PlayerbotAI.h"
#include "TradeData.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "Log.h"
#include "Spell.h"

// By leewheel 2026-07-18
// 修复开锁交易物品的问题：第一次成功后后续全部失败
// 原因：PlayerbotAI::CastSpell 对 OPEN_LOCK 法术走 Spell::prepare 路径，
//       而交易物品开锁应使用 TradeData::SetSpell 路径（无需创建 Spell 对象）。
//       原代码传入 OPEN_LOCK 法术但 CastSpell 的 TARGET_FLAG_ITEM 快速路径
//       只对非 OPEN_LOCK 法术生效（line 3757 检查 Effect != OPEN_LOCK），
//       导致走入了 Spell::prepare 完整施法路径，可能因各种条件检查失败。
// 修复：直接使用 TradeData::SetSpell 设置开锁法术到交易窗口，
//       这是服务端原生的交易物品开锁机制，不经过 PlayerbotAI::CastSpell。
// End By leewheel

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
                out << "开锁技能不足（" << botSkill << "/" << requiredSkill << "），无法开锁："
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

    // By leewheel 2026-07-18
    // 修复：直接使用服务端的交易法术机制，不经过 PlayerbotAI::CastSpell
    // 原因：PlayerbotAI::CastSpell 对 OPEN_LOCK 法术走 Spell::prepare 完整路径，
    //       创建 Spell 对象后调用 prepare()，会检查各种施法条件。
    //       但交易物品开锁应通过 TradeData::SetSpell 触发，
    //       服务端会在交易确认时执行开锁效果，不需要手动施法。
    TradeData* tradeData = bot->GetTradeData();
    if (!tradeData)
    {
        botAI->TellError("施放开锁失败：无交易数据。");
        return;
    }

    // 设置交易法术为开锁(1804)
    // 这会让服务端在交易物品上执行开锁效果
    tradeData->SetSpell(PICK_LOCK_SPELL_ID, nullptr);

    std::ostringstream out;
    out << "正在开锁交易物品：" << item->GetTemplate()->Name1;
    botAI->TellMaster(out.str());

    LOG_INFO("playerbots", "开锁交易物品：机器人 {} 对交易物品 {} 施放开锁(1804)，技能值 {}。",
        bot->GetName(), item->GetTemplate()->Name1, bot->GetSkillValue(SKILL_LOCKPICKING));
    // End By leewheel
}
