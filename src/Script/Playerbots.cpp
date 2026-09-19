/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "Playerbots.h"
#include "BattleGroundTactics.h"
#include "BgWanderGraph.h"
#include "BattlefieldScript.h"
#include "Channel.h"
#include "CheckMountStateAction.h"
#include "Config.h"
#include "BuiltInConfig.h"
#include "DBUpdater.h"
#include "DatabaseEnv.h"
// By leewheel 2026-09-19 合并上游 the-lab（2b15a2fb..799274b4）：本核的 PlayerbotsDatabase 仍是
//   核心自建的 DatabaseWorkerPool<PlayerbotsDatabaseConnection>（DatabaseEnv.h:45 声明 /
//   DatabaseEnv.cpp:25 定义），玩家机器人库的加载走【核心 DatabaseLoader 通道】，
//   故必须引入 DatabaseLoader.h。
//   上游新增的 "PlayerbotsDatabase.h" 在本核解析为
//   src/server/database/Database/Implementation/PlayerbotsDatabase.h（同一份；
//   模块侧那份同名副本因「类型重定义」已删除，见 PlayerbotsDatabase.h 内中文说明）。
#include "DatabaseLoader.h"
#include "PlayerbotsDatabase.h"
// End By leewheel
#include <mysqld_error.h>
#include "GuildTaskMgr.h"
#include "PlayerScript.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotCommandScript.h"
#include "PlayerbotGuildMgr.h"
#include "PlayerbotSpellRepository.h"
#include "PlayerbotWorldThreadProcessor.h"
#include "RandomPlayerbotMgr.h"
#include "ScriptMgr.h"
#include "cmath"
// By leewheel 2026-09-14：OnDatabaseGetDBRevision 改用上游的 map<string,string>& 签名，显式引入 <map>
#include <map>
// End By leewheel

//By leewheel 2026-07-20 - 新手玩家向高级机器人求助互动
extern bool HandleBotBeggingInteraction(Player* sender, Player* receiver, const std::string& msg);
//End By leewheel

class PlayerbotsDatabaseScript : public DatabaseScript
{
public:
    PlayerbotsDatabaseScript() : DatabaseScript("PlayerbotsDatabaseScript") {}

    // By leewheel 2026-09-14 适配 Acore e1823bb2（enable modules to own their database）：
    //   上游官方本提交引入"模块自有数据库"钩子 OnModuleDatabasesLoading / OnModuleDatabasesKeepAlive
    //   / OnModuleDatabasesClosing，与本模块原有自研钩子 OnDatabasesLoading / OnDatabasesKeepAlive
    //   / OnDatabasesClosing 语义完全相同但命名不同。不同步改名有两个后果：
    //   ① 基类里已无旧名，override 直接编译失败；
    //   ② 即便去掉 override，新钩子也只会走基类空实现 —— PlayerbotsDatabase 将不会被打开、保活与关闭。
    //   故此处整体对齐上游命名。OnDatabaseGetDBRevision 同时改为上游的 map<string,string>& 签名，
    //   按模块名上报版本(供 .server info 汇总显示)。OnDatabaseSelectIndexLogout 为上游所无，保留。
    // By leewheel 2026-09-15 合并brighton b2f6e460：本轮确认 brighton 侧 f7de76c4
    //   （"fixes to allow compile after AC update"）做了完全相同的改名
    //   （OnModuleDatabasesLoading/KeepAlive/Closing + OnDatabaseGetDBRevision 的 map 签名 + 写入
    //   revisions["Playerbots"]），两方已收敛到同一套实现，此处不再有任何差异。
    // By leewheel 2026-09-19 合并上游 the-lab：本函数体【有意偏离上游，保留本核自建仓基线的接线】。
    //   上游 #2793 把玩家机器人库的所有权收进模块（ModuleDatabasePool + ModuleDBUpdater），
    //   其写法要求 DoPrepareStatements 里【每一条】语句都是 CONNECTION_SYNCH ——
    //   ModuleDatabasePool.h:38-41 原文：
    //     "DoPrepareStatements must mark every statement CONNECTION_SYNCH: a CONNECTION_ASYNC
    //      one is skipped on these connections and asserts on first use."
    //   而本核 PlayerbotsDatabaseConnection::DoPrepareStatements() 共 71 条语句，其中
    //   【24 条是 CONNECTION_ASYNC】（如 PLAYERBOTS_INS_RANDOM_BOTS / PLAYERBOTS_DEL_RANDOM_BOTS* /
    //   PLAYERBOTS_INS_*_CACHE / PLAYERBOTS_INS_*TRAVELNODE* / PLAYERBOTS_INS_EQUIP_CACHE_NEW 等）。
    //   若照搬上游那套写法，这 24 条会在首次使用时断言，随机机器人库、旅行节点、装备/稀有度/传送
    //   缓存等全部失效 —— 故不能采纳。
    //   ⇒ 本核维持【自建仓基线】的既有接线：PlayerbotsDatabase 是核心的
    //   DatabaseWorkerPool<PlayerbotsDatabaseConnection>（DatabaseEnv.h:45 声明 / DatabaseEnv.cpp:25 定义），
    //   库的创建、填充、更新由核心 DatabaseLoader + DBUpdater<PlayerbotsDatabaseConnection> 承担：
    //     · DatabaseLoader.cpp:235-245 已为本核显式实例化 AddDatabase<PlayerbotsDatabaseConnection>；
    //     · DBUpdater.cpp:204-235 提供本核的 GetSourceDirectory()/GetBaseFilesDirectory()/GetTableName()/
    //       IsEnabled() 特化，其中 IsEnabled() 用 DatabaseLoader::DATABASE_PLAYERBOTS 判定
    //       （DatabaseLoader.h:52，枚举值 8）。
    //   语义与上游 ModuleDBUpdater 等价，但支持异步语句，且是本项目长期在跑的路径。
    bool OnModuleDatabasesLoading() override
    {
        DatabaseLoader playerbotLoader("server.playerbots");
        playerbotLoader.SetUpdateFlags(sConfigMgr->GetOption<bool>("Playerbots.Updates.EnableDatabases", true)
                                           ? DatabaseLoader::DATABASE_PLAYERBOTS
                                           : 0);
        playerbotLoader.AddDatabase(PlayerbotsDatabase, "Playerbots");

        return playerbotLoader.Load();
    }
    // End By leewheel

    void OnModuleDatabasesKeepAlive() override { PlayerbotsDatabase.KeepAlive(); }

    void OnModuleDatabasesClosing() override { PlayerbotsDatabase.Close(); }

    void OnDatabaseWarnAboutSyncQueries(bool apply) override { PlayerbotsDatabase.WarnAboutSyncQueries(apply); }

    // By leewheel 2026-09-19 合并上游 the-lab：本 override 【必须保留】——
    //   上游 799274b4 已无此函数，但它的共同祖先 2b15a2fb 仍然有，是 upstream 的
    //   af078829 "Use core modular database functionality (#2793)" 重写整块时【一并去掉】的
    //   （该提交主题是库所有权，删此钩子并非其目的 ⇒ 属顺带丢失，非有意废弃）。
    //   而本核的 DatabaseScript 仍挂这个钩子（DatabaseScript.h:108 虚函数 + ScriptMgr.h:754 +
    //   DatabaseScript.cpp:60，调用点 WorldSession.cpp:895-897）：
    //       uint32 statementIndex = CHAR_UPD_ACCOUNT_ONLINE;   // 核心默认值
    //       uint32 statementParam = GetAccountId();
    //       sScriptMgr->OnDatabaseSelectIndexLogout(_player, statementIndex, statementParam);
    //   没有本 override 时会走核心默认值，即只把【账号】标为在线状态；
    //   本 override 改发 CHAR_UPD_CHAR_OFFLINE + 角色 GUID（CharacterDatabase.h:285），
    //   正是 mod-playerbots 用来把机器人【角色】标记为离线的语句
    //   （本核 DatabaseScript.cpp:59 的注释也明确写着该钩子的用途）。
    void OnDatabaseSelectIndexLogout(Player* player, uint32& statementIndex, uint32& statementParam) override
    {
        statementIndex = CHAR_UPD_CHAR_OFFLINE;
        statementParam = player->GetGUID().GetCounter();
    }
    // End By leewheel

    void OnDatabaseGetDBRevision(std::map<std::string, std::string>& revisions) override
    {
        std::string revision;

        if (QueryResult resultPlayerbot =
                PlayerbotsDatabase.Query("SELECT date FROM version_db_playerbots ORDER BY date DESC LIMIT 1"))
        {
            Field* fields = resultPlayerbot->Fetch();
            revision = fields[0].Get<std::string>();
        }

        if (revision.empty())
            revision = "Unknown Playerbots Database Revision";

        revisions["Playerbots"] = revision;
    }
    // End By leewheel
};

class PlayerbotsPlayerScript : public PlayerScript
{
public:
    PlayerbotsPlayerScript() : PlayerScript("PlayerbotsPlayerScript", {
        PLAYERHOOK_ON_LOGIN,
        PLAYERHOOK_ON_AFTER_UPDATE,
        PLAYERHOOK_ON_BEFORE_CRITERIA_PROGRESS,
        PLAYERHOOK_ON_BEFORE_ACHI_COMPLETE,
        PLAYERHOOK_CAN_PLAYER_USE_PRIVATE_CHAT,
        PLAYERHOOK_CAN_PLAYER_USE_GROUP_CHAT,
        PLAYERHOOK_CAN_PLAYER_USE_GUILD_CHAT,
        PLAYERHOOK_CAN_PLAYER_USE_CHANNEL_CHAT,
        PLAYERHOOK_ON_GIVE_EXP,
        PLAYERHOOK_ON_BEFORE_TELEPORT
    }) {}

    void OnPlayerLogin(Player* player) override
    {
        if (!player->GetSession()->IsBot())
        {
            PlayerbotsMgr::instance().AddPlayerbotData(player, false);
            sRandomPlayerbotMgr.OnPlayerLogin(player);

            // Before modifying the following messages, please make sure it does not violate the GNU GPLv2
            // license especially if you are distributing a repack or hosting a public server
            // e.g. you can replace the URL with your own repository,
            // but it should be publicly accessible and include all modifications you've made
            if (sPlayerbotAIConfig.enabled)
            {
                ChatHandler(player->GetSession()).SendSysMessage(
                    "|cff00ff00本服务器运行 |cff00ccffmod-playerbots|r 模块 "
                    "|cffcccccchttps://github.com/mod-playerbots/mod-playerbots|r");
            }

            if (sPlayerbotAIConfig.enabled || sPlayerbotAIConfig.randomBotAutologin)
            {
                std::string maxAllowedBotCount = std::to_string(sRandomPlayerbotMgr.GetMaxAllowedBotCount());

                ChatHandler(player->GetSession()).SendSysMessage(
                    "|cff00ff00玩家机器人：|r 本服务器配置了 " + maxAllowedBotCount + " 个机器人。");
            }
        }
    }

    bool OnPlayerBeforeTeleport(Player* /*player*/, uint32 /*mapid*/, float /*x*/, float /*y*/, float /*z*/,
                                float /*orientation*/, uint32 /*options*/, Unit* /*target*/) override
    {
        /* for now commmented out until proven its actually required
        * havent seen any proof CleanVisibilityReferences() is needed

        // If the player is not safe to touch, do nothing
        if (!player)
            return true;

        // If same map or not in world do nothing
        if (!player->IsInWorld() || player->GetMapId() == mapid)
            return true;

        // If this is a selfbot, do nothing
        PlayerbotAI* ai = GET_PLAYERBOT_AI(player);
        if (!ai || IsSelfBot(player))
            return true;

        // Cross-map bot teleport: defer visibility reference cleanup.
        // CleanVisibilityReferences() erases this bot's GUID from other objects' visibility containers.
        // This is intentionally done via the event queue (instead of directly here) because erasing
        // from other players' visibility maps inside the teleport call stack can hit unsafe re-entrancy
        // or iterator invalidation while visibility updates are in progress
        ObjectGuid guid = player->GetGUID();
        player->m_Events.AddEventAtOffset(
            [guid, mapid]()
            {
                // do nothing, if the player is not safe to touch
                Player* p = ObjectAccessor::FindPlayer(guid);
                if (!p || !p->IsInWorld() || p->IsDuringRemoveFromWorld())
                    return;

                // do nothing if we are already on the target map
                if (p->GetMapId() == mapid)
                    return;

                p->GetObjectVisibilityContainer().CleanVisibilityReferences();
            },
            Milliseconds(0));

        */

        return true;
    }

    void OnPlayerAfterUpdate(Player* player, uint32 diff) override
    {
        PlayerbotAI* const botAI = PlayerbotsMgr::instance().GetPlayerbotAI(player);

        if (botAI != nullptr)
        {
            botAI->UpdateAI(diff);
        }

        if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
        {
            playerbotMgr->UpdateAI(diff);
        }
    }

    using PlayerScript::OnPlayerCanUseChat;  // keep the base overloads visible

    bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 /*lang*/, std::string& msg, Player* receiver) override
    {
        if (type != CHAT_MSG_WHISPER)
        {
            return true;
        }

        PlayerbotAI* const botAI = PlayerbotsMgr::instance().GetPlayerbotAI(receiver);

        if (botAI == nullptr)
        {
            return true;
        }

        //By leewheel 2026-07-20 - 新手玩家向高级机器人求助互动（要金币/背包）
        if (HandleBotBeggingInteraction(player, receiver, msg))
            return false;
        //End By leewheel

        botAI->HandleCommand(type, msg, player);

        // hotfix; otherwise the server will crash when whispering logout
        // https://github.com/mod-playerbots/mod-playerbots/pull/1838
        // TODO: find the root cause and solve it. (does not happen in party chat)
        if (msg == "logout")
            return false;

        return true;
    }

    bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 /*lang*/, std::string& msg, Group* group) override
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* const member = itr->GetSource();

            if (member == nullptr)
                continue;

            PlayerbotAI* const botAI = PlayerbotsMgr::instance().GetPlayerbotAI(member);

            if (botAI == nullptr)
                continue;

            botAI->HandleCommand(type, msg, player);
        }

        return true;
    }

    bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 /*lang*/, std::string& msg, Guild* /*guild*/) override
    {
        if (type != CHAT_MSG_GUILD)
            return true;

        PlayerbotMgr* playerbotMgr = PlayerbotsMgr::instance().GetPlayerbotMgr(player);

        if (playerbotMgr == nullptr)
            return true;

        for (PlayerBotMap::const_iterator it = playerbotMgr->GetPlayerBotsBegin(); it != playerbotMgr->GetPlayerBotsEnd(); ++it)
        {
            Player* const bot = it->second;

            if (bot == nullptr)
                continue;

            if (bot->GetGuildId() != player->GetGuildId())
                continue;

            PlayerbotsMgr::instance().GetPlayerbotAI(bot)->HandleCommand(type, msg, player);
        }

        return true;
    }

    bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 /*lang*/, std::string& msg, Channel* channel) override
    {
        PlayerbotMgr* const playerbotMgr = PlayerbotsMgr::instance().GetPlayerbotMgr(player);

        if (playerbotMgr != nullptr && channel->GetFlags() & 0x18)
            playerbotMgr->HandleCommand(type, msg);

        sRandomPlayerbotMgr.HandleCommand(type, msg, player);

        return true;
    }

    bool OnPlayerBeforeAchievementComplete(Player* player, AchievementEntry const* achievement) override
    {
        if ((sRandomPlayerbotMgr.IsRandomBot(player) || sRandomPlayerbotMgr.IsAddclassBot(player)) &&
            (achievement->flags & (ACHIEVEMENT_FLAG_REALM_FIRST_REACH | ACHIEVEMENT_FLAG_REALM_FIRST_KILL)))
        {
            return false;
        }

        return true;
    }

    void OnPlayerGiveXP(Player* player, uint32& amount, Unit* /*victim*/, uint8 /*xpSource*/) override
    {
        // early return
        if (sPlayerbotAIConfig.randomBotXPRate == 1.0 || !player)
            return;

        // no XP multiplier, when player is no bot.
        if (!player->GetSession()->IsBot() || !sRandomPlayerbotMgr.IsRandomBot(player))
            return;

        // no XP multiplier, when bot is in a group with a real player.
        if (Group* group = player->GetGroup())
        {
            for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
            {
                Player* member = gref->GetSource();
                if (!member)
                    continue;

                if (!member->GetSession()->IsBot())
                    return;
            }
        }

        // otherwise apply bot XP multiplier.
        amount = static_cast<uint32>(std::round(static_cast<float>(amount) * sPlayerbotAIConfig.randomBotXPRate));
    }
};

class PlayerbotsMiscScript : public MiscScript
{
public:
    PlayerbotsMiscScript() : MiscScript("PlayerbotsMiscScript", {MISCHOOK_ON_DESTRUCT_PLAYER}) {}

    void OnDestructPlayer(Player* player) override
    {
        PlayerbotAI* botAI = PlayerbotsMgr::instance().GetPlayerbotAI(player);

        if (botAI != nullptr)
            delete botAI;

        if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
            delete playerbotMgr;
    }
};

class PlayerbotsServerScript : public ServerScript
{
public:
    PlayerbotsServerScript() : ServerScript("PlayerbotsServerScript", {
        SERVERHOOK_CAN_PACKET_RECEIVE
    }) {}

    void OnPacketReceived(WorldSession* session, WorldPacket const& packet) override
    {
        if (Player* player = session->GetPlayer())
            if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
                playerbotMgr->HandleMasterIncomingPacket(packet);
    }
};

class PlayerbotsWorldScript : public WorldScript
{
public:
    PlayerbotsWorldScript() : WorldScript("PlayerbotsWorldScript", {
        WORLDHOOK_ON_BEFORE_WORLD_INITIALIZED,
        WORLDHOOK_ON_UPDATE
    }) {}

    void OnBeforeWorldInitialized() override
    {
        // Before modifying the following messages, please make sure it does not violate the GNU GPLv2
        // license especially if you are distributing a repack or hosting a public server
        // e.g. you can replace the URL with your own repository,
        // but it should be publicly accessible and include all modifications you've made
        // By leewheel 2026-08-30 合并上游：许可证标注更正为GNU GPLv2(上游134c709ca统一)
        LOG_INFO("server.loading", "╔══════════════════════════════════════════════════════════╗");
        LOG_INFO("server.loading", "║                                                          ║");
        LOG_INFO("server.loading", "║              AzerothCore Playerbots Module               ║");
        LOG_INFO("server.loading", "║                                                          ║");
        LOG_INFO("server.loading", "╟──────────────────────────────────────────────────────────╢");
        LOG_INFO("server.loading", "║     mod-playerbots is a community-driven open-source     ║");
        LOG_INFO("server.loading", "║  project based on AzerothCore, licensed under GNU GPLv2  ║");
        LOG_INFO("server.loading", "╟──────────────────────────────────────────────────────────╢");
        LOG_INFO("server.loading", "║     https://github.com/mod-playerbots/mod-playerbots     ║");
        LOG_INFO("server.loading", "╚══════════════════════════════════════════════════════════╝");
        // End By leewheel

        uint32 oldMSTime = getMSTime();

        LOG_INFO("server.loading", " ");
        LOG_INFO("server.loading", "Load Playerbots Config...");

        sPlayerbotAIConfig.Initialize();

        LOG_INFO("server.loading", ">> Loaded playerbots config in {} ms", GetMSTimeDiffToNow(oldMSTime));
        LOG_INFO("server.loading", " ");

        PlayerbotSpellRepository::Instance().Initialize();
        CheckMountStateAction::LoadPreferredMounts();

        // By leewheel 2026-09-03 加载战场游走节点图(移植自NPCBots, 供战场策略按节点决策)
        BgWanderGraph::instance()->Load();
        // End By leewheel

        LOG_INFO("server.loading", "Playerbots World Thread Processor initialized");
    }

    void OnUpdate(uint32 diff) override
    {
        PlayerbotWorldThreadProcessor::instance().Update(diff);
        sRandomPlayerbotMgr.UpdateAI(diff);  // World thread only
    }
};

class PlayerbotsScript : public PlayerbotScript
{
public:
    PlayerbotsScript() : PlayerbotScript("PlayerbotsScript") {}

    bool OnPlayerbotCheckLFGQueue(lfg::Lfg5Guids const& guidsList) override
    {
        bool nonBotFound = false;

        for (ObjectGuid const& guid : guidsList.guids)
        {
            Player* player = ObjectAccessor::FindPlayer(guid);

            if (guid.IsGroup() || IsRealPlayer(player) || IsSelfBot(player))
            {
                nonBotFound = true;
                break;
            }
        }

        return nonBotFound;
    }

    void OnPlayerbotCheckKillTask(Player* player, Unit* victim) override
    {
        if (player)
            GuildTaskMgr::instance().CheckKillTask(player, victim);
    }

    void OnPlayerbotCheckPetitionAccount(Player* player, bool& found) override
    {
        if (!found)
            return;

        if (PlayerbotsMgr::instance().GetPlayerbotAI(player) != nullptr)
            found = false;
    }

    bool OnPlayerbotCheckUpdatesToSend(Player* player) override
    {
        PlayerbotAI* botAI = PlayerbotsMgr::instance().GetPlayerbotAI(player);

        if (botAI == nullptr)
            return true;

        return IsSelfBot(player);
    }

    void OnPlayerbotPacketSent(Player* player, WorldPacket const* packet) override
    {
        if (player == nullptr)
            return;

        PlayerbotAI* botAI = PlayerbotsMgr::instance().GetPlayerbotAI(player);

        if (botAI != nullptr)
            botAI->HandleBotOutgoingPacket(*packet);

        if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
            playerbotMgr->HandleMasterOutgoingPacket(*packet);
    }

    void OnPlayerbotUpdate(uint32 /*diff*/) override
    {
        sRandomPlayerbotMgr.UpdateSessions();  // Per-bot updates only
    }

    void OnPlayerbotUpdateSessions(Player* player) override
    {
        if (player)
            if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
                playerbotMgr->UpdateSessions();
    }

    void OnPlayerbotLogout(Player* player) override
    {
        if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
        {
            PlayerbotAI* botAI = PlayerbotsMgr::instance().GetPlayerbotAI(player);

            if (botAI == nullptr || IsSelfBot(player))
                playerbotMgr->LogoutAllBots();
        }

        sRandomPlayerbotMgr.OnPlayerLogout(player);
    }

    void OnPlayerbotLogoutBots() override
    {
        LOG_INFO("playerbots", "Logging out all bots...");
        sRandomPlayerbotMgr.LogoutAllBots();
    }
};

class PlayerBotsBGScript : public BGScript
{
public:
    PlayerBotsBGScript() : BGScript("PlayerBotsBGScript") {}

    void OnBattlegroundStart(Battleground* bg) override
    {
        BGStrategyData data;

        switch (bg->GetBgTypeID())
        {
            case BATTLEGROUND_WS:
                data.allianceStrategy = urand(0, WS_STRATEGY_MAX - 1);
                data.hordeStrategy = urand(0, WS_STRATEGY_MAX - 1);
                break;
            case BATTLEGROUND_AB:
                data.allianceStrategy = urand(0, AB_STRATEGY_MAX - 1);
                data.hordeStrategy = urand(0, AB_STRATEGY_MAX - 1);
                break;
            case BATTLEGROUND_AV:
                data.allianceStrategy = urand(0, AV_STRATEGY_MAX - 1);
                data.hordeStrategy = urand(0, AV_STRATEGY_MAX - 1);
                break;
            case BATTLEGROUND_EY:
                data.allianceStrategy = urand(0, EY_STRATEGY_MAX - 1);
                data.hordeStrategy = urand(0, EY_STRATEGY_MAX - 1);
                break;
            default:
                break;
        }

        bgStrategies[bg->GetInstanceID()] = data;
    }

    void OnBattlegroundEnd(Battleground* bg, TeamId /*winnerTeam*/) override { bgStrategies.erase(bg->GetInstanceID()); }
};

// Workaround for missing InitEnabledHooksIfNeeded for new BattlefieldScript in ScriptMgr
class PlayerbotsBattlefieldScript : public BattlefieldScript
{
public:
    PlayerbotsBattlefieldScript() : BattlefieldScript("PlayerbotsBattlefieldScript") { }
};

void AddPlayerbotsSecureLoginScripts();
void AddPlayerbotsSelfBotAfkScripts();
void AddSC_MagtheridonBotScripts();
void AddSC_TempestKeepBotScripts();
void AddSC_HyjalBotScripts();
void AddSC_SunwellBotScripts();
void AddSC_IcecrownBotScripts();
void AddSC_RubySanctumBotScripts();
//By leewheel 2026-08-01 - 太阳之井高地机器人脚本注册声明
//  根因：AddSC_SunwellPlateauBotScripts 只定义未注册，导致 SWP 全部技能监听失效——
//  Kalecgos 传送门状态永远为空(bot不进内场无法击杀)、Kil'jaeden 末日决战规避失效、
//  Felmyst封装/双子点燃/千魂之暗预警和打断辅助全部缺失。
void AddSC_SunwellPlateauBotScripts();
//End By leewheel

//By leewheel 2026-07-06 - 快速组队系统
void AddSC_FastGroup();
//End By leewheel

//By leewheel 2026-07-07 - 自动加入团本系统
void AddSC_AutoJoinRaid();
//End By leewheel

//By leewheel 2026-07-17 - 路过增益系统
void AddSC_PassByBuffScripts();
//End By leewheel

//By leewheel 2026-07-18 - 机器人副本进入条件自动补全系统
void AddSC_BotInstanceEntryFixScripts();
//End By leewheel

//By leewheel 2026-07-20 - 玩家自用机器人辅助命令
void AddSC_ForPlayerCommand();
//End By leewheel

//By leewheel 2026-07-26 - 组队钓鱼策略
void AddSC_FishingParty();
//End By leewheel

void AddSC_randombot_level_mgr();

void AddPlayerbotsScripts()
{
    new PlayerbotsBattlefieldScript();
    new PlayerbotsDatabaseScript();
    new PlayerbotsPlayerScript();
    new PlayerbotsMiscScript();
    new PlayerbotsServerScript();
    new PlayerbotsWorldScript();
    new PlayerbotsScript();
    new PlayerBotsBGScript();

    // 组队钓鱼策略
    AddSC_FishingParty();

    AddPlayerbotsSecureLoginScripts();
    AddPlayerbotsSelfBotAfkScripts();
    AddPlayerbotsCommandscripts();
    PlayerBotsGuildValidationScript();
    AddSC_MagtheridonBotScripts();
    AddSC_TempestKeepBotScripts();
    AddSC_HyjalBotScripts();
    AddSC_SunwellBotScripts();
    AddSC_IcecrownBotScripts();
    AddSC_RubySanctumBotScripts();
    //By leewheel 2026-08-18 - 删除重复注册：brighton-the-lab 版本已在上方 (AddSC_SunwellPlateauBotScripts) 正式注册此脚本，此处旧补丁（2026-08-01 曾修复"漏注册"）与之重复，会令 SWP 的 6 个 ListenerScript/BossUpdateScript/TargetTracker 脚本对象被 new 两次并重复注册，导致 Spell 事件回调触发两次
    //End By leewheel

    //By leewheel 2026-07-06 - 快速组队系统
    AddSC_FastGroup();
    //End By leewheel

    //By leewheel 2026-07-07 - 自动加入团本系统
    AddSC_AutoJoinRaid();
    //End By leewheel

    //By leewheel 2026-07-17 - 路过增益系统
    AddSC_PassByBuffScripts();
    //End By leewheel

    //By leewheel 2026-07-18 - 机器人副本进入条件自动补全系统
    AddSC_BotInstanceEntryFixScripts();
    //End By leewheel

    //By leewheel 2026-07-20 - 玩家自用机器人辅助命令
    AddSC_ForPlayerCommand();
    //End By leewheel

    AddSC_randombot_level_mgr();
}
