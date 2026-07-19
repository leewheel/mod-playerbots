/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "HelpAction.h"

#include "ChatActionContext.h"
#include "Event.h"
#include "AiObjectContext.h"

HelpAction::HelpAction(PlayerbotAI* botAI) : Action(botAI, "help") { chatContext = new ChatActionContext(); }

HelpAction::~HelpAction() { delete chatContext; }

bool HelpAction::Execute(Event /*event*/)
{
    TellChatCommands();
    TellStrategies();
    return true;
}

void HelpAction::TellChatCommands()
{
    std::ostringstream out;
    out << "可密语以下命令（支持中文别名）: ";
    out << CombineSupported(chatContext->supports());
    out << "，或发送 [物品]、[任务]、[物体] 链接";
    botAI->TellError(out.str());
}

void HelpAction::TellStrategies()
{
    std::ostringstream out;
    out << "可用策略（co/战斗/nc/非战斗/de/死亡 命令）: ";
    out << CombineSupported(botAI->GetAiObjectContext()->GetSupportedStrategies());
    botAI->TellError(out.str());
}

std::string const HelpAction::CombineSupported(std::set<std::string> commands)
{
    std::ostringstream out;

    for (std::set<std::string>::iterator i = commands.begin(); i != commands.end();)
    {
        out << *i;
        if (++i != commands.end())
            out << ", ";
    }

    return out.str();
}
