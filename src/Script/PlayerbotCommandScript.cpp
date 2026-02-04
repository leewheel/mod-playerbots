/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "BattleGroundTactics.h"
#include "Chat.h"
#include "GuildTaskMgr.h"
#include "PerfMonitor.h"
#include "PlayerbotMgr.h"
#include "RandomPlayerbotMgr.h"
#include "ScriptMgr.h"
#include "Group.h"
#include "PlayerbotFactory.h"
#include <Playerbots.h>

using namespace Acore::ChatCommands;

class playerbots_commandscript : public CommandScript
{
public:
    playerbots_commandscript() : CommandScript("playerbots_commandscript") {}

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable playerbotsDebugCommandTable = {
            {"bg", HandleDebugBGCommand, SEC_GAMEMASTER, Console::Yes},
        };

        static ChatCommandTable playerbotsAccountCommandTable = {
            {"setKey", HandleSetSecurityKeyCommand, SEC_PLAYER, Console::No},
            {"link", HandleLinkAccountCommand, SEC_PLAYER, Console::No},
            {"linkedAccounts", HandleViewLinkedAccountsCommand, SEC_PLAYER, Console::No},
            {"unlink", HandleUnlinkAccountCommand, SEC_PLAYER, Console::No},
        };

        static ChatCommandTable playerbotsCommandTable = {
            {"bot", HandlePlayerbotCommand, SEC_PLAYER, Console::No},
            {"gtask", HandleGuildTaskCommand, SEC_GAMEMASTER, Console::Yes},
            {"pmon", HandlePerfMonCommand, SEC_GAMEMASTER, Console::Yes},
            {"rndbot", HandleRandomPlayerbotCommand, SEC_GAMEMASTER, Console::Yes},
            {"debug", playerbotsDebugCommandTable},
            {"account", playerbotsAccountCommandTable},
        };

        static ChatCommandTable commandTable = {
            {"playerbots", playerbotsCommandTable},
        };

        return commandTable;
    }

    static bool HandlePlayerbotCommand(ChatHandler* handler, char const* args)
    {
        return PlayerbotMgr::HandlePlayerbotMgrCommand(handler, args);
    }

    static bool HandleRandomPlayerbotCommand(ChatHandler* handler, char const* args)
    {
        return RandomPlayerbotMgr::HandlePlayerbotConsoleCommand(handler, args);
    }

    static bool HandleGuildTaskCommand(ChatHandler* handler, char const* args)
    {
        return GuildTaskMgr::HandleConsoleCommand(handler, args);
    }

    static bool HandlePerfMonCommand(ChatHandler* handler, char const* args)
    {
        if (!strcmp(args, "reset"))
        {
            sPerfMonitor.Reset();
            return true;
        }

        if (!strcmp(args, "tick"))
        {
            sPerfMonitor.PrintStats(true, false);
            return true;
        }

        if (!strcmp(args, "stack"))
        {
            sPerfMonitor.PrintStats(false, true);
            return true;
        }

        if (!strcmp(args, "toggle"))
        {
            sPlayerbotAIConfig.perfMonEnabled = !sPlayerbotAIConfig.perfMonEnabled;
            if (sPlayerbotAIConfig.perfMonEnabled)
                LOG_INFO("playerbots", "Performance monitor enabled");
            else
                LOG_INFO("playerbots", "Performance monitor disabled");
            return true;
        }

        sPerfMonitor.PrintStats();
        return true;
    }

    static bool HandleDebugBGCommand(ChatHandler* handler, char const* args)
    {
        return BGTactics::HandleConsoleCommand(handler, args);
    }

    static bool HandleSetSecurityKeyCommand(ChatHandler* handler, char const* args)
    {
        if (!args || !*args)
        {
            handler->PSendSysMessage("Usage: .playerbots account setKey <securityKey>");
            return false;
        }

        Player* player = handler->GetSession()->GetPlayer();
        std::string key = args;

        PlayerbotMgr* mgr = PlayerbotsMgr::instance().GetPlayerbotMgr(player);
        if (mgr)
        {
            mgr->HandleSetSecurityKeyCommand(player, key);
            return true;
        }
        else
        {
            handler->PSendSysMessage("PlayerbotMgr instance not found.");
            return false;
        }
    }

    static bool HandleLinkAccountCommand(ChatHandler* handler, char const* args)
    {
        if (!args || !*args)
            return false;

        char* accountName = strtok((char*)args, " ");
        char* key = strtok(nullptr, " ");

        if (!accountName || !key)
        {
            handler->PSendSysMessage("Usage: .playerbots account link <accountName> <securityKey>");
            return false;
        }

        Player* player = handler->GetSession()->GetPlayer();

        PlayerbotMgr* mgr = PlayerbotsMgr::instance().GetPlayerbotMgr(player);
        if (mgr)
        {
            mgr->HandleLinkAccountCommand(player, accountName, key);
            return true;
        }
        else
        {
            handler->PSendSysMessage("PlayerbotMgr instance not found.");
            return false;
        }
    }

    static bool HandleViewLinkedAccountsCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();

        PlayerbotMgr* mgr = PlayerbotsMgr::instance().GetPlayerbotMgr(player);
        if (mgr)
        {
            mgr->HandleViewLinkedAccountsCommand(player);
            return true;
        }
        else
        {
            handler->PSendSysMessage("PlayerbotMgr instance not found.");
            return false;
        }
    }

    static bool HandleUnlinkAccountCommand(ChatHandler* handler, char const* args)
    {
        if (!args || !*args)
            return false;

        char* accountName = strtok((char*)args, " ");
        if (!accountName)
        {
            handler->PSendSysMessage("Usage: .playerbots account unlink <accountName>");
            return false;
        }

        Player* player = handler->GetSession()->GetPlayer();

        PlayerbotMgr* mgr = PlayerbotsMgr::instance().GetPlayerbotMgr(player);
        if (mgr)
        {
            mgr->HandleUnlinkAccountCommand(player, accountName);
            return true;
        }
        else
        {
            handler->PSendSysMessage("PlayerbotMgr instance not found.");
            return false;
        }
    }
};

static std::string GetPlayerRole(Player* player)
{
    uint32 spec = player->GetSpec(player->GetActiveSpec());
    switch (spec)
    {
        case TALENT_TREE_WARRIOR_PROTECTION:
        case TALENT_TREE_PALADIN_PROTECTION:
        case TALENT_TREE_DEATH_KNIGHT_BLOOD:
            return "tank";
        case TALENT_TREE_PALADIN_HOLY:
        case TALENT_TREE_PRIEST_DISCIPLINE:
        case TALENT_TREE_PRIEST_HOLY:
        case TALENT_TREE_SHAMAN_RESTORATION:
        case TALENT_TREE_DRUID_RESTORATION:
            return "heal";
        default:
            return "dps";
    }
}

// 组队函数
struct RoleNeeds
{
    int tanks;
    int heals;
    int dps;
};

class playerbots_chscommandscript : public CommandScript
{
public:
    playerbots_chscommandscript() : CommandScript("playerbots_chscommandscript") {}

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable debugTable = {
            {"战场", HandleDebugBGCommand, SEC_GAMEMASTER, Console::Yes},
        };

        static ChatCommandTable accountTable = {
            {"设密钥", HandleSetSecurityKeyCommand, SEC_PLAYER, Console::No},
            {"关联", HandleLinkAccountCommand, SEC_PLAYER, Console::No},
            {"查看", HandleViewLinkedAccountsCommand, SEC_PLAYER, Console::No},
            {"取消关联", HandleUnlinkAccountCommand, SEC_PLAYER, Console::No},
        };

        static ChatCommandTable playerbotsSubCommands = {
            {"机器人", HandlePlayerbotCommand, SEC_PLAYER, Console::No},
            {"添加", HandleAddClassCHS, SEC_PLAYER, Console::No},
            {"公会任务", HandleGuildTaskCommand, SEC_GAMEMASTER, Console::Yes},
            {"性能监控", HandlePerfMonCommand, SEC_GAMEMASTER, Console::Yes},
            {"随机机器人", HandleRandomPlayerbotCommand, SEC_GAMEMASTER, Console::Yes},
            {"挂机", HandleMakeSelfBotCommand, SEC_PLAYER, Console::No},
            {"刷新装备", HandleRefreshequipmentCommand, SEC_PLAYER, Console::No},
            {"刷稀有装备", HandleinitrareCommand, SEC_PLAYER, Console::No},
            {"刷精良装备", HandleinitEpicCommand, SEC_PLAYER, Console::No},
            {"刷史诗装备", HandleinitLegendaryCommand, SEC_PLAYER, Console::No},
            {"刷毕业装备", HandleinitArtifactCommand, SEC_PLAYER, Console::No},
            {"5人队", HandleFastGroupCommand, SEC_PLAYER, Console::No},
            {"菜刀队", HandleSwordTeamCommand, SEC_PLAYER, Console::No},
            {"五火球神教", HandleFireBallTeamCommand, SEC_PLAYER, Console::No},
            {"10人团", HandleRaid10Command, SEC_PLAYER, Console::No},
            {"25人团", HandleRaid25Command, SEC_PLAYER, Console::No},
            {"40人团", HandleRaid40Command, SEC_PLAYER, Console::No},
            {"同步任务", HandleSyncQuestCommand, SEC_PLAYER, Console::Yes},
            //{"PVP5人队", HandlePVP5ManPartyCommand, SEC_PLAYER, Console::Yes},
            {"调试", debugTable},
            {"账户", accountTable},
        };

        static ChatCommandTable commandTable = {
            {"玩家机器人", playerbotsSubCommands},
        };

        return commandTable;
    }

    static bool HandlePlayerbotCommand(ChatHandler* handler, char const* args)
    {
        return PlayerbotMgr::HandlePlayerbotMgrCommand(handler, args);
    }

    static bool HandleRandomPlayerbotCommand(ChatHandler* handler, char const* args)
    {
        return RandomPlayerbotMgr::HandlePlayerbotConsoleCommand(handler, args);
    }

    static bool HandleGuildTaskCommand(ChatHandler* handler, char const* args)
    {
        return GuildTaskMgr::HandleConsoleCommand(handler, args);
    }

    static bool HandlePerfMonCommand(ChatHandler* handler, char const* args)
    {
        if (!args || !*args)
        {
            sPerfMonitor.PrintStats();
            return true;
        }

        std::string cmd = args;
        Acore::String::Trim(cmd);

        if (cmd == "reset")
        {
            sPerfMonitor.Reset();
            return true;
        }
        if (cmd == "tick")
        {
            sPerfMonitor.PrintStats(true, false);
            return true;
        }
        if (cmd == "stack")
        {
            sPerfMonitor.PrintStats(false, true);
            return true;
        }
        if (cmd == "toggle")
        {
            sPlayerbotAIConfig.perfMonEnabled = !sPlayerbotAIConfig.perfMonEnabled;
            LOG_INFO("playerbots", "Performance monitor {}",
                     sPlayerbotAIConfig.perfMonEnabled ? "enabled" : "disabled");
            return true;
        }

        return true;
    }

    static bool HandleDebugBGCommand(ChatHandler* handler, char const* args)
    {
        return BGTactics::HandleConsoleCommand(handler, args);
    }

    static bool HandleSetSecurityKeyCommand(ChatHandler* handler, char const* args)
    {
        if (!args || !*args)
        {
            handler->PSendSysMessage("用法: .玩家机器人 账户 设密钥 <key>");
            return false;
        }

        Player* player = handler->GetSession()->GetPlayer();
        PlayerbotMgr* mgr = sPlayerbotsMgr.GetPlayerbotMgr(player);
        if (mgr)
        {
            mgr->HandleSetSecurityKeyCommand(player, args);
            return true;
        }

        handler->PSendSysMessage("未找到机器人管理器");
        return false;
    }

    static bool HandleLinkAccountCommand(ChatHandler* handler, char const* args)
    {
        char* acc = strtok((char*)args, " ");
        char* key = strtok(nullptr, " ");
        if (!acc || !key)
        {
            handler->PSendSysMessage("用法: .玩家机器人 账户 关联 <账号> <密钥>");
            return false;
        }

        Player* player = handler->GetSession()->GetPlayer();
        if (auto* mgr = sPlayerbotsMgr.GetPlayerbotMgr(player))
        {
            mgr->HandleLinkAccountCommand(player, acc, key);
            return true;
        }

        handler->PSendSysMessage("未找到机器人管理器");
        return false;
    }

    static bool HandleViewLinkedAccountsCommand(ChatHandler* handler, char const*)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (auto* mgr = sPlayerbotsMgr.GetPlayerbotMgr(player))
        {
            mgr->HandleViewLinkedAccountsCommand(player);
            return true;
        }

        handler->PSendSysMessage("未找到机器人管理器");
        return false;
    }

    static bool HandleUnlinkAccountCommand(ChatHandler* handler, char const* args)
    {
        if (!args || !*args)
        {
            handler->PSendSysMessage("用法: .玩家机器人 账户 取消关联 <账号>");
            return false;
        }

        Player* player = handler->GetSession()->GetPlayer();
        if (auto* mgr = sPlayerbotsMgr.GetPlayerbotMgr(player))
        {
            mgr->HandleUnlinkAccountCommand(player, args);
            return true;
        }

        handler->PSendSysMessage("未找到机器人管理器");
        return false;
    }

    static bool HandleAddClassCHS(ChatHandler* handler, char const* args)
    {
        if (!args || !*args)
        {
            handler->PSendSysMessage("用法: .玩家机器人 添加 <职业>");
            return false;
        }

        std::string zhClass = args;
        Acore::String::Trim(zhClass);

        auto translated = TranslateClassName(zhClass);
        if (!translated)
        {
            handler->PSendSysMessage("不支持的职业: %s", zhClass.c_str());
            return false;
        }

        std::string command = "addclass " + *translated;
        return PlayerbotMgr::HandlePlayerbotMgrCommand(handler, command.c_str());
    }

    static std::optional<std::string> TranslateClassName(const std::string& zh)
    {
        static const std::unordered_map<std::string, std::string> map = {
            {"战士", "warrior"}, {"圣骑", "paladin"}, {"圣骑士", "paladin"}, {"猎人", "hunter"}, {"潜行者", "rogue"},
            {"盗贼", "rogue"},   {"牧师", "priest"},  {"萨满", "shaman"},    {"法师", "mage"},   {"术士", "warlock"},
            {"德鲁伊", "druid"}, {"死亡骑士", "dk"},  {"死骑", "dk"},        {"dk", "dk"},       {"DK", "dk"},
            {"Dk", "dk"},        {"dK", "dk"}};

        auto it = map.find(zh);
        if (it != map.end())
            return it->second;
        return std::nullopt;
    }

    static bool HandleMakeSelfBotCommand(ChatHandler* handler, char const* args)
    {
        std::string command = "self";
        return PlayerbotMgr::HandlePlayerbotMgrCommand(handler, command.c_str());
    }

    // HandleRefreshequipmentCommand
    static bool HandleRefreshequipmentCommand(ChatHandler* handler, char const* args)
    {
        std::string command = "init=auto";
        return PlayerbotMgr::HandlePlayerbotMgrCommand(handler, command.c_str());
    }

    // init=rare
    static bool HandleinitrareCommand(ChatHandler* handler, char const* args)
    {
        std::string command = "init=rare";
        return PlayerbotMgr::HandlePlayerbotMgrCommand(handler, command.c_str());
    }
    // HandleinitEpicCommand
    static bool HandleinitEpicCommand(ChatHandler* handler, char const* args)
    {
        std::string command = "init=Epic";
        return PlayerbotMgr::HandlePlayerbotMgrCommand(handler, command.c_str());
    }
    static bool HandleinitLegendaryCommand(ChatHandler* handler, char const* args)
    {
        std::string command = "init=Legendary";
        return PlayerbotMgr::HandlePlayerbotMgrCommand(handler, command.c_str());
    }
    // HandleinitArtifactCommand
    static bool HandleinitArtifactCommand(ChatHandler* handler, char const* args)
    {
        std::string command = "init=Artifact";
        return PlayerbotMgr::HandlePlayerbotMgrCommand(handler, command.c_str());
    }

    static std::string GetRoleBySpec(Player* p)
    {
        uint32 specId = p->GetSpec(p->GetActiveSpec());
        switch (specId)
        {
            case TALENT_TREE_WARRIOR_PROTECTION:
            case TALENT_TREE_PALADIN_PROTECTION:
            case TALENT_TREE_DRUID_FERAL_COMBAT:
            case TALENT_TREE_DEATH_KNIGHT_BLOOD:
                return "tank";

            case TALENT_TREE_PRIEST_HOLY:
            case TALENT_TREE_PRIEST_DISCIPLINE:
            case TALENT_TREE_PALADIN_HOLY:
            case TALENT_TREE_SHAMAN_RESTORATION:
            case TALENT_TREE_DRUID_RESTORATION:
                return "heal";

            default:
                return "dps";
        }
    }

    static Player* AddBotWithSpec(ChatHandler* handler, Player* owner, std::set<ObjectGuid>& knownGuids,
                                  const std::string& cls, const std::string& spec)
    {
        if (cls == "dk" && owner->GetLevel() < 55)
            return nullptr;

        PlayerbotMgr::HandlePlayerbotMgrCommand(handler, ("addclass " + cls).c_str());

        for (auto const& ref : owner->GetMap()->GetPlayers())
        {
            Player* bot = ref.GetSource();
            if (!bot || !sPlayerbotsMgr.GetPlayerbotAI(bot))
                continue;

            if (knownGuids.count(bot->GetGUID()))
                continue;

            knownGuids.insert(bot->GetGUID());

            if (!spec.empty())
            {
                PlayerbotMgr::HandlePlayerbotMgrCommand(handler, ("talents spec " + spec + " pve").c_str());
            }

            return bot;
        }

        return nullptr;
    }

    static bool HandleFastGroupCommand(ChatHandler* handler, char const*)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
            return false;

        if (player->GetGroup())
        {
            handler->PSendSysMessage("你已经在队伍中，无法使用该命令！");
            return false;
        }

        PlayerbotMgr* mgr = sPlayerbotsMgr.GetPlayerbotMgr(player);
        if (!mgr)
        {
            handler->PSendSysMessage("未找到机器人管理器");
            return false;
        }

        uint32 level = player->GetLevel();

        std::map<std::string, std::vector<std::pair<std::string, std::string>>> roleSpecMap = {
            {"tank", {{"warrior", "protection"}, {"paladin", "protection"}, {"druid", "bear"}, {"dk", "blood"}}},
            {"heal", {{"priest", "holy"}, {"paladin", "holy"}, {"shaman", "restoration"}, {"druid", "restoration"}}},
            {"dps",
             {{"mage", "frost"},
              {"warlock", "affliction"},
              {"rogue", "combat"},
              {"hunter", "marksman"},
              {"shaman", "enhancement"},
              {"priest", "shadow"},
              {"druid", "balance"},
              {"paladin", "retribution"},
              {"dk", "unholy"}}}};

        std::set<ObjectGuid> knownGuids;
        for (auto const& ref : player->GetMap()->GetPlayers())
            if (Player* p = ref.GetSource())
                if (sPlayerbotsMgr.GetPlayerbotAI(p))
                    knownGuids.insert(p->GetGUID());

        // 低等级直接随机，不追求结构
        if (level < 10)
        {
            std::vector<std::string> classes = {"warrior", "paladin", "priest", "rogue", "mage",
                                                "hunter",  "warlock", "shaman", "druid"};

            std::shuffle(classes.begin(), classes.end(), std::mt19937(std::random_device()()));

            int added = 0;
            for (auto& cls : classes)
            {
                if (AddBotWithSpec(handler, player, knownGuids, cls, ""))
                {
                    if (++added >= 4)
                        break;
                }
            }

            handler->PSendSysMessage("等级 < 15：已随机组建队伍，添加 %u 名机器人", added);
            return true;
        }

        // === 标准 5 人结构 ===
        std::string playerRole = GetRoleBySpec(player);

        int tank = (playerRole == "tank");
        int heal = (playerRole == "heal");
        int dps = (playerRole == "dps");

        std::mt19937 rng(std::random_device{}());

        int safety = 20;  // 防死循环
        while ((tank < 1 || heal < 1 || dps < 3) && safety-- > 0)
        {
            std::string need = tank < 1 ? "tank" : heal < 1 ? "heal" : "dps";

            auto& pool = roleSpecMap[need];
            std::shuffle(pool.begin(), pool.end(), rng);

            bool filled = false;
            for (auto& [cls, spec] : pool)
            {
                Player* bot = AddBotWithSpec(handler, player, knownGuids, cls, spec);
                if (!bot)
                    continue;

                std::string r = GetRoleBySpec(bot);
                if (r != need)
                    continue;

                if (r == "tank")
                    ++tank;
                else if (r == "heal")
                    ++heal;
                else
                    ++dps;

                filled = true;
                break;
            }

            if (!filled)
                break;
        }

        handler->PSendSysMessage("已组建标准队伍：Tank={}， Heal={}， DPS={}", tank, heal, dps);

        return true;
    }


    static bool HandleSwordTeamCommand(ChatHandler* handler, char const*)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
            return false;

        if (!player->HasMeleeSpec())
        {
            handler->PSendSysMessage("只有近战职业才能组建菜刀队！");
            return false;
        }

        if (player->GetGroup())
        {
            handler->PSendSysMessage("你已经在队伍中，无法使用该命令！");
            return false;
        }

        PlayerbotMgr* mgr = sPlayerbotsMgr.GetPlayerbotMgr(player);
        if (!mgr)
        {
            handler->PSendSysMessage("未找到机器人管理器");
            return false;
        }

        uint32 level = player->GetLevel();

        // 职业池：近战（含 DK，但仅限55级以上）
        std::vector<std::pair<std::string, std::string>> meleePool = {
            {"warrior", "arms"},       {"warrior", "fury"}, {"paladin", "retribution"}, {"rogue", "combat"},
            {"shaman", "enhancement"}, {"druid", "feral"},  {"dk", "unholy"},           {"dk", "frost"}};

        // 坦克专精（优先使用）
        std::vector<std::pair<std::string, std::string>> tankPool = {
            {"warrior", "protection"}, {"paladin", "protection"}, {"druid", "bear"}, {"dk", "blood"}};

        std::set<ObjectGuid> beforeGuids;
        for (auto const& ref : player->GetMap()->GetPlayers())
            if (Player* p = ref.GetSource())
                if (sPlayerbotsMgr.GetPlayerbotAI(p))
                    beforeGuids.insert(p->GetGUID());

        auto AddBot = [&](const std::string& className, const std::string& spec = "")
        {
            // <55 时跳过死亡骑士
            if (className == "dk" && level < 55)
                return;

            std::string cmd = "addclass " + className;
            PlayerbotMgr::HandlePlayerbotMgrCommand(handler, cmd.c_str());

            for (auto const& ref : player->GetMap()->GetPlayers())
            {
                Player* bot = ref.GetSource();
                if (!bot || !sPlayerbotsMgr.GetPlayerbotAI(bot))
                    continue;
                if (beforeGuids.count(bot->GetGUID()))
                    continue;

                beforeGuids.insert(bot->GetGUID());

                if (!spec.empty())
                {
                    std::string specCmd = "talents spec " + spec + " pve";
                    PlayerbotMgr::HandlePlayerbotMgrCommand(handler, specCmd.c_str());
                }
                break;
            }
        };

        // 添加坦克
        bool tankAdded = false;
        for (auto& [cls, spec] : tankPool)
        {
            if (cls == "dk" && level < 55)
                continue;

            AddBot(cls, spec);
            tankAdded = true;
            break;
        }

        if (!tankAdded)
        {
            handler->PSendSysMessage("无法添加坦克职业，组建失败");
            return false;
        }

        // 添加近战 DPS（排除奶）
        int addedDps = 0;
        for (auto& [cls, spec] : meleePool)
        {
            if (addedDps >= 4)
                break;

            if (cls == "dk" && level < 55)
                continue;

            // 跳过 tank 已添加的
            bool isTankSpec = false;
            for (auto& [tCls, tSpec] : tankPool)
            {
                if (cls == tCls && spec == tSpec)
                {
                    isTankSpec = true;
                    break;
                }
            }
            if (isTankSpec)
                continue;

            AddBot(cls, spec);
            ++addedDps;
        }

        handler->PSendSysMessage("已为你组建菜刀队，包含1坦克和 %d 个近战DPS", addedDps);
        return true;
    }

    static bool HandleFireBallTeamCommand(ChatHandler* handler, char const*)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
            return false;

        // 必须是火法
        if (player->getClass() != CLASS_MAGE || player->GetSpec() != TALENT_TREE_MAGE_FIRE)
        {
            handler->PSendSysMessage("你不是火法，无法召唤五火球神教！");
            return false;
        }

        if (player->GetGroup())
        {
            handler->PSendSysMessage("你已经在一个队伍中，无法使用此命令！");
            return false;
        }

        PlayerbotMgr* mgr = sPlayerbotsMgr.GetPlayerbotMgr(player);
        if (!mgr)
        {
            handler->PSendSysMessage("未找到机器人管理器");
            return false;
        }

        uint32 level = player->GetLevel();

        // 创建队伍
        Group* group = new Group();
        if (!group->Create(player))
        {
            delete group;
            handler->PSendSysMessage("队伍创建失败");
            return false;
        }

        // 获取当前地图中所有机器人 GUID（执行前）
        std::set<ObjectGuid> beforeGuids;
        for (auto const& ref : player->GetMap()->GetPlayers())
            if (Player* p = ref.GetSource())
                if (sPlayerbotsMgr.GetPlayerbotAI(p))
                    beforeGuids.insert(p->GetGUID());

        int toAdd = 5 - 1;  // 1为玩家自己
        int added = 0;

        for (int i = 0; i < toAdd; ++i)
        {
            PlayerbotMgr::HandlePlayerbotMgrCommand(handler, "addclass mage");
            ++added;
        }

        // 遍历地图上机器人，找到新增的机器人进行初始化
        for (auto const& ref : player->GetMap()->GetPlayers())
        {
            Player* bot = ref.GetSource();
            if (!bot || !sPlayerbotsMgr.GetPlayerbotAI(bot))
                continue;

            if (beforeGuids.count(bot->GetGUID()))
                continue;

            // 设置为火法专精
            if (level >= 15)
            {
                std::string specCmd = "talents spec fire pve";
                PlayerbotMgr::HandlePlayerbotMgrCommand(handler, specCmd.c_str());
            }
        }

        handler->PSendSysMessage("五火球神教已集结，信火者，得永生！");
        return true;
    }

    static bool HandleRaid10Command(ChatHandler* handler, char const*)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
            return false;

        if (player->GetLevel() < 60)
        {
            handler->PSendSysMessage("等级不足，至少需要60级才能组建10人团队！");
            return false;
        }

        if (player->GetGroup())
        {
            handler->PSendSysMessage("你已经在队伍中，无法使用该命令！");
            return false;
        }

        PlayerbotMgr* mgr = sPlayerbotsMgr.GetPlayerbotMgr(player);
        if (!mgr)
        {
            handler->PSendSysMessage("未找到机器人管理器");
            return false;
        }

        // 创建团队
        Group* raidGroup = new Group();
        if (!raidGroup->Create(player))
        {
            delete raidGroup;
            handler->PSendSysMessage("创建团队失败！");
            return false;
        }
        raidGroup->ConvertToRaid();

        // 职责 - 职业 + 天赋组合
        std::map<std::string, std::vector<std::pair<std::string, std::string>>> roleSpecMap = {
            {"tank", {{"dk", "blood"}, {"warrior", "protection"}, {"paladin", "protection"}, {"druid", "bear"}}},
            {"heal", {{"priest", "holy"}, {"paladin", "holy"}, {"shaman", "restoration"}, {"druid", "restoration"}}},
            {"dps",
             {{"mage", "frost"},
              {"warlock", "affliction"},
              {"rogue", "combat"},
              {"hunter", "marksman"},
              {"shaman", "enhancement"},
              {"priest", "shadow"},
              {"druid", "balance"},
              {"paladin", "retribution"},
              {"dk", "unholy"}}}};

        // 记录已有 bot GUID
        std::set<ObjectGuid> beforeGuids;
        for (auto const& ref : player->GetMap()->GetPlayers())
            if (Player* p = ref.GetSource())
                if (sPlayerbotsMgr.GetPlayerbotAI(p))
                    beforeGuids.insert(p->GetGUID());

        // ✅ 修复：使用 raidGroup 而不是 player->GetGroup()
        auto AddBot = [&](const std::string& cls, const std::string& spec)
        {
            std::string cmd = "addclass " + cls;
            PlayerbotMgr::HandlePlayerbotMgrCommand(handler, cmd.c_str());

            // 等机器人创建并进组（给一点延迟，防止 race condition）
            std::this_thread::sleep_for(std::chrono::milliseconds(200));

            for (auto const& ref : player->GetMap()->GetPlayers())
            {
                Player* bot = ref.GetSource();
                if (!bot || !sPlayerbotsMgr.GetPlayerbotAI(bot))
                    continue;
                if (beforeGuids.count(bot->GetGUID()))
                    continue;

                // ✅ 关键修复：使用 raidGroup 而不是 player->GetGroup()
                if (!raidGroup->IsMember(bot->GetGUID()))
                {
                    raidGroup->AddMember(bot);
                }

                if (!raidGroup->IsMember(bot->GetGUID()))
                    continue;

                beforeGuids.insert(bot->GetGUID());

                // 分配天赋
                std::string specCmd = "talents spec " + spec + " pve";
                PlayerbotMgr::HandlePlayerbotMgrCommand(handler, specCmd.c_str());
                break;
            }
        };

        auto AddRoleBots = [&](const std::string& role, int count)
        {
            auto candidates = roleSpecMap[role];
            std::shuffle(candidates.begin(), candidates.end(), std::mt19937(std::random_device()()));

            int added = 0;
            for (const auto& [cls, spec] : candidates)
            {
                AddBot(cls, spec);
                if (++added >= count)
                    break;
            }
        };

        // 固定分配，添加 9 个机器人
        std::string playerRole = GetPlayerRole(player);

        int botTank = 2;
        int botHeal = 2;
        int botDps = 6;

        if (playerRole == "tank")
            botTank--;
        else if (playerRole == "heal")
            botHeal--;
        else
            botDps--;

        botTank = std::max(0, botTank);
        botHeal = std::max(0, botHeal);
        botDps = std::max(0, botDps);

        // 加 bot
        AddRoleBots("tank", botTank);
        AddRoleBots("heal", botHeal);
        AddRoleBots("dps", botDps);

        handler->PSendSysMessage("10人团队组建完成：2坦 2奶 6DPS（含你）");
        return true;
    }

    static bool HandleRaid25Command(ChatHandler* handler, char const*)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player || player->GetLevel() < 60)
        {
            handler->PSendSysMessage("需要等级60以上才能组建25人团队！");
            return false;
        }

        if (player->GetGroup())
        {
            handler->PSendSysMessage("你已经在队伍中！");
            return false;
        }

        PlayerbotMgr* mgr = sPlayerbotsMgr.GetPlayerbotMgr(player);
        if (!mgr)
        {
            handler->PSendSysMessage("未找到机器人管理器！");
            return false;
        }

        // 创建一个团队（只包含玩家）
        Group* raidGroup = new Group();
        if (!raidGroup->Create(player))
        {
            delete raidGroup;
            handler->PSendSysMessage("创建团队失败！");
            return false;
        }
        raidGroup->ConvertToRaid();

        // 记录玩家职责
        std::string playerRole = GetPlayerRole(player);
        int needTank = 2 - (playerRole == "tank" ? 1 : 0);
        int needHeal = 5 - (playerRole == "heal" ? 1 : 0);
        int needDps = 18 - (playerRole == "dps" ? 1 : 0);

        // 职责对应的职业+天赋
        std::map<std::string, std::vector<std::pair<std::string, std::string>>> roleSpecMap = {
            {"tank", {{"warrior", "protection"}, {"paladin", "protection"}, {"druid", "bear"}, {"dk", "blood"}}},
            {"heal", {{"priest", "holy"}, {"paladin", "holy"}, {"shaman", "restoration"}, {"druid", "restoration"}}},
            {"dps",
             {{"mage", "frost"},
              {"warlock", "affliction"},
              {"rogue", "combat"},
              {"hunter", "marksman"},
              {"shaman", "enhancement"},
              {"priest", "shadow"},
              {"druid", "balance"},
              {"paladin", "retribution"},
              {"dk", "unholy"}}}};

        // ✅ 修复：记录现有机器人GUID
        std::set<ObjectGuid> beforeGuids;
        for (auto const& ref : player->GetMap()->GetPlayers())
            if (Player* p = ref.GetSource())
                if (sPlayerbotsMgr.GetPlayerbotAI(p))
                    beforeGuids.insert(p->GetGUID());

        auto AddBotsForRole = [&](const std::string& role, int count)
        {
            auto list = roleSpecMap[role];
            std::shuffle(list.begin(), list.end(), std::mt19937(std::random_device()()));

            for (int i = 0; i < count; ++i)
            {
                const auto& [cls, spec] = list[i % list.size()];

                std::string cmd = "addclass " + cls;
                PlayerbotMgr::HandlePlayerbotMgrCommand(handler, cmd.c_str());

                std::this_thread::sleep_for(std::chrono::milliseconds(200));

                // 找到新创建的机器人并加入raidGroup
                for (auto const& ref : player->GetMap()->GetPlayers())
                {
                    Player* bot = ref.GetSource();
                    if (!bot || !sPlayerbotsMgr.GetPlayerbotAI(bot))
                        continue;
                    if (beforeGuids.count(bot->GetGUID()))
                        continue;

                    // ✅ 关键修复：加入 raidGroup
                    if (!raidGroup->IsMember(bot->GetGUID()))
                    {
                        raidGroup->AddMember(bot);
                    }

                    beforeGuids.insert(bot->GetGUID());

                    // 设置天赋
                    std::string specCmd = "talents spec " + spec + " pve";
                    PlayerbotMgr::HandlePlayerbotMgrCommand(handler, specCmd.c_str());
                    break;
                }
            }
        };

        // 添加机器人
        AddBotsForRole("tank", needTank);
        AddBotsForRole("heal", needHeal);
        AddBotsForRole("dps", needDps);

        handler->PSendSysMessage("25人团队组建完成：2 坦 5 奶 18 DPS（包含你自己）");
        return true;
    }

    static bool HandleRaid40Command(ChatHandler* handler, char const*)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player || player->GetLevel() < 60)
        {
            handler->PSendSysMessage("等级不足，至少60级才能组建40人团队！");
            return false;
        }

        if (player->GetGroup())
        {
            handler->PSendSysMessage("你已经在队伍中！");
            return false;
        }

        PlayerbotMgr* mgr = sPlayerbotsMgr.GetPlayerbotMgr(player);
        if (!mgr)
        {
            handler->PSendSysMessage("无法获取机器人管理器！");
            return false;
        }

        Group* raidGroup = new Group();
        if (!raidGroup->Create(player))
        {
            delete raidGroup;
            handler->PSendSysMessage("队伍创建失败！");
            return false;
        }
        raidGroup->ConvertToRaid();

        std::set<ObjectGuid> beforeGuids;
        for (auto const& ref : player->GetMap()->GetPlayers())
            if (Player* p = ref.GetSource())
                if (sPlayerbotsMgr.GetPlayerbotAI(p))
                    beforeGuids.insert(p->GetGUID());

        std::map<std::string, std::vector<std::pair<std::string, std::string>>> roleSpecMap = {
            {"tank", {{"dk", "blood"}, {"warrior", "protection"}, {"paladin", "protection"}, {"druid", "bear"}}},
            {"heal", {{"priest", "holy"}, {"paladin", "holy"}, {"shaman", "restoration"}, {"druid", "restoration"}}},
            {"dps",
             {{"mage", "frost"},
              {"warlock", "affliction"},
              {"rogue", "combat"},
              {"hunter", "marksman"},
              {"shaman", "enhancement"},
              {"priest", "shadow"},
              {"druid", "balance"},
              {"paladin", "retribution"},
              {"dk", "unholy"}}}};

        auto AddBot = [&](const std::string& cls, const std::string& spec)
        {
            std::string cmd = "addclass " + cls;
            PlayerbotMgr::HandlePlayerbotMgrCommand(handler, cmd.c_str());

            // 等机器人创建并进组（给一点延迟，防止 race condition）
            std::this_thread::sleep_for(std::chrono::milliseconds(200));

            for (auto const& ref : player->GetMap()->GetPlayers())
            {
                Player* bot = ref.GetSource();
                if (!bot || !sPlayerbotsMgr.GetPlayerbotAI(bot))
                    continue;
                if (beforeGuids.count(bot->GetGUID()))
                    continue;

                // ✅ 关键修复：直接把机器人加入 raidGroup
                if (!raidGroup->IsMember(bot->GetGUID()))
                {
                    raidGroup->AddMember(bot);
                }

                if (!raidGroup->IsMember(bot->GetGUID()))
                    continue;

                beforeGuids.insert(bot->GetGUID());

                // 分配天赋
                std::string specCmd = "talents spec " + spec + " pve";
                PlayerbotMgr::HandlePlayerbotMgrCommand(handler, specCmd.c_str());
                break;
            }
        };

        std::string playerRole = GetPlayerRole(player);
        int needTank = 4 - (playerRole == "tank" ? 1 : 0);
        int needHeal = 8 - (playerRole == "heal" ? 1 : 0);
        int needDps = 27 - (playerRole == "dps" ? 1 : 0);  // 1 是玩家自己

        auto AddBotsForRole = [&](const std::string& role, int count)
        {
            auto list = roleSpecMap[role];
            std::shuffle(list.begin(), list.end(), std::mt19937(std::random_device()()));

            for (int i = 0; i < count; ++i)
            {
                const auto& [cls, spec] = list[i % list.size()];
                AddBot(cls, spec);
            }
        };

        AddBotsForRole("tank", needTank);
        AddBotsForRole("heal", needHeal);
        AddBotsForRole("dps", needDps);

        handler->PSendSysMessage("40人团队组建完成：4 坦 8 奶 27 DPS（包含你）");
        return true;
    }

    static bool HandleSyncQuestCommand(ChatHandler* handler, char const* args)
    {
        Player* master = handler->GetSession()->GetPlayer();
        if (!master)
            return false;

        Group* group = master->GetGroup();
        if (!group)
        {
            handler->SendSysMessage("你当前没有在队伍中，无法同步机器人进度。");
            return false;
        }

        PlayerbotMgr* mgr = sPlayerbotsMgr.GetPlayerbotMgr(master);
        if (!mgr)
        {
            handler->SendSysMessage("未找到机器人管理器");
            return false;
        }
        uint32 syncedBots = 0;

        // 遍历队伍中的成员
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || !member->IsInWorld() || member->IsBeingTeleported() || member->GetMap() != master->GetMap())
                continue;

            // 仅同步机器人
            if (!member->GetSession() || !member->GetSession()->IsBot())
                continue;

            // 同步任务状态
            for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
            {
                if (master->GetQuestSlotQuestId(slot) != 0)
                {
                    uint32 questId = master->GetQuestSlotQuestId(slot);
                    Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
                    if (!quest)
                        continue;

                    QuestStatus masterStatus = master->GetQuestStatus(questId);
                    QuestStatus memberStatus = member->GetQuestStatus(questId);

                    if (memberStatus == QUEST_STATUS_NONE)
                    {
                        // 机器人没有这个任务，添加它
                        member->AddQuest(quest, nullptr);
                        memberStatus = QUEST_STATUS_INCOMPLETE;
                    }

                    // 同步任务状态
                    if (memberStatus != masterStatus)
                    {
                        if (masterStatus == QUEST_STATUS_COMPLETE || masterStatus == QUEST_STATUS_REWARDED)
                        {
                            // 如果玩家已完成，设置机器人为完成状态
                            member->SetQuestStatus(questId, QUEST_STATUS_COMPLETE);
                        }
                        else if (masterStatus == QUEST_STATUS_INCOMPLETE)
                        {
                            // 如果玩家在进行中，设置机器人为进行中
                            member->SetQuestStatus(questId, QUEST_STATUS_INCOMPLETE);
                        }
                        // 其他状态保持不变
                    }

                    // 设置目标完成数量
                    for (uint8 i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
                    {
                        uint16 masterCount = master->GetQuestSlotCounter(slot, i);
                        member->SetQuestSlotCounter(slot, i, masterCount);
                    }

                    member->SendQuestUpdate(questId);
                }
            }

            ++syncedBots;
        }

        handler->PSendSysMessage("已成功将所有任务同步给 {} 个机器人。", syncedBots);
        return true;
    }
};



void AddPlayerbotsCommandscripts()
{
    new playerbots_commandscript();
    new playerbots_chscommandscript();
}
