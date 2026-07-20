//By leewheel 2026-07-20
/*
 * forplayerCommand.cpp - 玩家自用机器人辅助命令
 *
 * 命令：.机器人宠物嘲讽
 *   在副本/团队副本中，切换队伍/团队里所有机器人猎人宠物的嘲讽自动施放。
 *   第一次输入关闭嘲讽，第二次输入开启嘲讽。
 *   切换后向小队/团队广播消息。
 *
 * 作者: leewheel
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Chat.h"
#include "Group.h"
#include "Pet.h"
#include "SpellMgr.h"
#include "WorldSession.h"
#include "SharedDefines.h"
#include "Log.h"

#include <algorithm>

using namespace Acore::ChatCommands;

class ForPlayerCommandScript : public CommandScript
{
public:
    ForPlayerCommandScript() : CommandScript("ForPlayerCommandScript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable petTauntTable = {{ "", HandleBotPetTaunt, rbac::RBAC_PERM_COMMAND_RELOAD, Console::No }};

        static ChatCommandTable commandTable = {
            {"机器人宠物嘲讽", petTauntTable},
        };
        return commandTable;
    }

    // 机器人宠物嘲讽 — 切换队伍/团队中所有机器人猎人宠物的嘲讽自动施放
    static bool HandleBotPetTaunt(ChatHandler* handler)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
            return false;

        // 必须在副本或团队副本中
        Map* map = player->GetMap();
        if (!map || (!map->IsDungeon() && !map->IsRaid()))
        {
            handler->PSendSysMessage("此命令只能在副本或团队副本中使用。");
            return true;
        }

        // 必须在队伍或团队中
        Group* group = player->GetGroup();
        if (!group)
        {
            handler->PSendSysMessage("你不在队伍或团队中。");
            return true;
        }

        // 第一遍：确定切换方向（取第一个找到的机器人猎人宠物的嘲讽状态）
        bool disableTaunt = true; // 默认关闭嘲讽
        bool stateDetermined = false;

        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || !member->IsInWorld())
                continue;
            if (!member->GetSession() || !member->GetSession()->IsBot())
                continue;
            if (member->getClass() != CLASS_HUNTER)
                continue;

            Pet* pet = member->GetPet();
            if (!pet || !pet->IsAlive() || !pet->IsInWorld())
                continue;

            // 查找宠物的嘲讽技能（Growl，使用 SPELL_EFFECT_ATTACK_ME）
            for (auto const& [spellId, petSpell] : pet->m_spells)
            {
                SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
                if (!spellInfo)
                    continue;
                if (!spellInfo->IsAutocastable())
                    continue;
                if (!spellInfo->HasEffect(SPELL_EFFECT_ATTACK_ME))
                    continue;

                // 找到了嘲讽技能，判断当前是否开启了自动施放
                auto autoIt = std::find(pet->m_autospells.begin(), pet->m_autospells.end(), spellId);
                disableTaunt = (autoIt != pet->m_autospells.end()); // 当前开启则关闭，当前关闭则开启
                stateDetermined = true;
                break;
            }
            if (stateDetermined)
                break;
        }

        if (!stateDetermined)
        {
            handler->PSendSysMessage("队伍中没有找到拥有宠物的机器人猎人。");
            return true;
        }

        // 第二遍：对所有机器人猎人宠物执行切换
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || !member->IsInWorld())
                continue;
            if (!member->GetSession() || !member->GetSession()->IsBot())
                continue;
            if (member->getClass() != CLASS_HUNTER)
                continue;

            Pet* pet = member->GetPet();
            if (!pet || !pet->IsAlive() || !pet->IsInWorld())
                continue;

            for (auto const& [spellId, petSpell] : pet->m_spells)
            {
                SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
                if (!spellInfo)
                    continue;
                if (!spellInfo->IsAutocastable())
                    continue;
                if (!spellInfo->HasEffect(SPELL_EFFECT_ATTACK_ME))
                    continue;

                // 切换自动施放: disableTaunt=true 则关闭, false 则开启
                pet->ToggleAutocast(spellInfo, !disableTaunt);

                // 向队伍/团队广播消息
                std::string msg = Acore::StringFormat("{}的宠物 {} 的嘲讽已{}",
                    member->GetName(), pet->GetName(), disableTaunt ? "关闭" : "开启");

                for (GroupReference* msgItr = group->GetFirstMember(); msgItr != nullptr; msgItr = msgItr->next())
                {
                    Player* msgTarget = msgItr->GetSource();
                    if (msgTarget && msgTarget->IsInWorld() && msgTarget->GetSession())
                        ChatHandler(msgTarget->GetSession()).PSendSysMessage("{}", msg);
                }
                break; // 每个宠物只处理一个嘲讽技能
            }
        }

        return true;
    }
};

void AddSC_ForPlayerCommand()
{
    new ForPlayerCommandScript();
}
