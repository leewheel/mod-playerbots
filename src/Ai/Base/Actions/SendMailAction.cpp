/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SendMailAction.h"

#include "ChatHelper.h"
#include "Event.h"
#include "ItemVisitors.h"
#include "Mail.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"

bool SendMailAction::Execute(Event event)
{
    uint32 account = bot->GetSession()->GetAccountId();
    bool randomBot = sPlayerbotAIConfig.IsInRandomAccountList(account);

    GuidVector gos = *context->GetValue<GuidVector>("nearest game objects");
    bool mailboxFound = false;
    for (ObjectGuid const guid : gos)
    {
        if (GameObject* go = botAI->GetGameObject(guid))
            if (go->GetGoType() == GAMEOBJECT_TYPE_MAILBOX)
            {
                mailboxFound = true;
                break;
            }
    }

    std::string const text = event.getParam();

    Player* receiver = GetMaster();
    Player* tellTo = receiver;

    if (!receiver)
        receiver = event.getOwner();

    if (!receiver || receiver == bot)
        return false;

    if (!tellTo)
        tellTo = receiver;

    if (!sPlayerbotAIConfig.botSendMailEnabled)
    {
        bot->Whisper(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                         "send_mail_disabled", "我无法发送邮件", {}),
                     LANG_UNIVERSAL, tellTo);
        return false;
    }

    if (!mailboxFound && !randomBot)
    {
        bot->Whisper(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                         "send_mail_no_mailbox_nearby", "附近没有邮箱", {}),
                     LANG_UNIVERSAL, tellTo);
        return false;
    }

    ItemIds ids = chat->parseItems(text);
    if (ids.size() > 1)
    {
        bot->Whisper(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                         "send_mail_one_item_only", "你不能请求超过一件物品", {}),
                     LANG_UNIVERSAL, tellTo);
        return false;
    }

    if (ids.empty())
    {
        uint32 money = chat->parseMoney(text);
        if (!money)
            return false;

        if (randomBot)
        {
            bot->Whisper(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                             "send_mail_cannot_send_money", "我无法发送金币", {}),
                         LANG_UNIVERSAL, tellTo);
            return false;
        }

        if (bot->GetMoney() < money)
        {
            botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "send_mail_not_enough_money", "我的金币不足", {}));
            return false;
        }

        std::ostringstream body;
        body << "你好，" << receiver->GetName() << "，\n";
        body << "\n";
        body << "这是你需要的金币。\n";
        body << "\n";
        body << "谢谢，\n";
        body << bot->GetName() << "\n";

        CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();

        MailDraft draft("你需要的金币", body.str());
        draft.AddMoney(money);
        bot->SetMoney(bot->GetMoney() - money);
        draft.SendMailTo(trans, MailReceiver(receiver), MailSender(bot));

        CharacterDatabase.CommitTransaction(trans);

        std::ostringstream out;
        botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "send_mail_sending_to",
            "正在发送邮件给 %receiver",
            {{"%receiver", receiver->GetName()}}));
        return true;
    }

    std::ostringstream body;
    body << "你好，" << receiver->GetName() << "，\n";
    body << "\n";
    body << "这是你需要的物品。\n";
    body << "\n";
    body << "谢谢，\n";
    body << bot->GetName() << "\n";

    MailDraft draft("你需要的物品", body.str());
    for (ItemIds::iterator i = ids.begin(); i != ids.end(); i++)
    {
        FindItemByIdVisitor visitor(*i);
        IterateItems(&visitor, ITERATE_ITEMS_IN_BAGS);

        std::vector<Item*> items = visitor.GetResult();
        for (Item* item : items)
        {
            if (item->IsSoulBound() || item->IsConjuredConsumable())
            {
                std::ostringstream out;
                out << PlayerbotTextMgr::instance().GetBotTextOrDefault(
                    "send_mail_cannot_send_item",
                    "无法发送 %item",
                    {{"%item", ChatHelper::FormatItem(item->GetTemplate())}});
                bot->Whisper(out.str(), LANG_UNIVERSAL, tellTo);
                continue;
            }

            ItemTemplate const* proto = item->GetTemplate();
            if (!proto)
                continue;

            if (randomBot)
            {
                uint32 price = item->GetCount() * proto->SellPrice;
                if (!price)
                {
                    std::ostringstream out;
                    out << PlayerbotTextMgr::instance().GetBotTextOrDefault(
                        "send_mail_item_not_for_sale",
                        "%item：此物品不可出售",
                        {{"%item", ChatHelper::FormatItem(item->GetTemplate())}});
                    bot->Whisper(out.str(), LANG_UNIVERSAL, tellTo);
                    return false;
                }

                draft.AddCOD(price);
            }

            CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();

            bot->MoveItemFromInventory(item->GetBagSlot(), item->GetSlot(), true);
            item->DeleteFromInventoryDB(trans);
            item->SetOwnerGUID(receiver->GetGUID());
            item->SaveToDB(trans);
            draft.AddItem(item);
            draft.SendMailTo(trans, MailReceiver(receiver), MailSender(bot));

            CharacterDatabase.CommitTransaction(trans);

            std::ostringstream out;
            out << PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "send_mail_sent_to",
                "已发送邮件给 %receiver",
                {{"%receiver", receiver->GetName()}});
            bot->Whisper(out.str(), LANG_UNIVERSAL, tellTo);
            return true;
        }
    }

    return false;
}
