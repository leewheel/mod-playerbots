/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "ChangeChatAction.h"

#include "Event.h"
#include "Playerbots.h"

bool ChangeChatAction::Execute(Event event)
{
    std::string const text = event.getParam();
    ChatMsg parsed = chat->parseChat(text);
    if (parsed == CHAT_MSG_SYSTEM)
    {
        std::ostringstream out;
        out << "当前聊天频道 " << chat->FormatChat(*context->GetValue<ChatMsg>("chat"));
        botAI->TellMaster(out);
    }
    else
    {
        context->GetValue<ChatMsg>("chat")->Set(parsed);

        std::ostringstream out;
        out << "聊天频道已设为 " << chat->FormatChat(parsed);
        botAI->TellMaster(out);
    }

    return true;
}
