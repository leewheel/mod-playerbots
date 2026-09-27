/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PLAYERBOTMGR_H
#define PLAYERBOTS_PLAYERBOTMGR_H

#include "ObjectGuid.h"
#include "Player.h"
#include "PlayerbotAIBase.h"
#include <ctime>

class ChatHandler;
class PlayerbotAI;
class PlayerbotLoginQueryHolder;
class WorldPacket;
class WorldSession;

typedef std::map<ObjectGuid, Player*> PlayerBotMap;
typedef std::map<std::string, std::set<std::string> > PlayerBotErrorMap;

struct PendingBotLogin
{
    uint32 masterAccountId = 0;
    WorldSession* session = nullptr;
    bool failed = false;  // torn down by UpdatePendingLogins(), never inside the session's own callback
};

class PlayerbotHolder : public PlayerbotAIBase
{
public:
    PlayerbotHolder();
    virtual ~PlayerbotHolder(){};

    void AddPlayerBot(ObjectGuid guid, uint32 masterAccountId);
    bool IsAccountLinked(uint32 accountId, uint32 masterAccountId);
    void HandlePlayerBotLoginCallback(PlayerbotLoginQueryHolder const& holder, WorldSession* botSession);

    void LogoutPlayerBot(ObjectGuid guid);
    void DisablePlayerBot(ObjectGuid guid);
    void RemoveFromPlayerbotsMap(ObjectGuid guid);
    Player* GetPlayerBot(ObjectGuid guid) const;
    Player* GetPlayerBot(ObjectGuid::LowType lowGuid) const;
    PlayerBotMap::const_iterator GetPlayerBotsBegin() const { return playerBots.begin(); }
    PlayerBotMap::const_iterator GetPlayerBotsEnd() const { return playerBots.end(); }

    void UpdateAIInternal([[maybe_unused]] uint32 elapsed, [[maybe_unused]] bool minimal = false) override{};
    void UpdateSessions();
    void HandleBotPackets(WorldSession* session);
    static void UpdatePendingLogins();  // world thread, once per tick
    static void ClearPendingLogins();   // shutdown, outside any callback

    void LogoutAllBots();
    void OnBotLogin(Player* const bot);

    std::vector<std::string> HandlePlayerbotCommand(char const* args, Player* master = nullptr);
    std::string const ProcessBotCommand(std::string const cmd, ObjectGuid guid, ObjectGuid masterguid, bool admin,
                                        uint32 masterAccountId, uint32 masterGuildId);
    uint32 GetAccountId(std::string const name);
    uint32 GetAccountId(ObjectGuid guid);
    std::string const ListBots(Player* master);
    std::string const LookupBots(Player* master);
    uint32 GetPlayerbotsCount() { return playerBots.size(); }
    uint32 GetPlayerbotsCountByClass(uint32 cls);

protected:
    virtual void OnBotLoginInternal(Player* const bot) = 0;

    PlayerBotMap playerBots;
    // By leewheel 2026-09-27 合并brighton the-lab：采纳上游新登录架构
    //   PendingBotLogin（记录 session + failed 由 UpdatePendingLogins 定时清理）。
    static void AbandonPendingLogin(ObjectGuid guid);
    static std::unordered_map<ObjectGuid, PendingBotLogin> botLoading;
    // 本地超时兜底（保留）：brighton 的 UpdatePendingLogins 只在 session 有可处理回调时
    //   才进展，对"异步查询回调异常/丢失、永不返回"的条目没有强制清除；此处记录入队时间，
    //   供 RandomPlayerbotMgr 定时把超时（60s）条目踢出 botLoading，避免登录循环卡死。
    // End By leewheel
    static std::unordered_map<ObjectGuid, time_t> botLoadingTime;
};

class PlayerbotMgr : public PlayerbotHolder
{
public:
    PlayerbotMgr(Player* const master);
    virtual ~PlayerbotMgr();

    static bool HandlePlayerbotMgrCommand(ChatHandler* handler, char const* args);
    void HandleMasterIncomingPacket(WorldPacket const& packet);
    void HandleMasterOutgoingPacket(WorldPacket const& packet);
    void HandleCommand(uint32 type, std::string const text);
    void OnPlayerLogin(Player* player);
    void CancelLogout();

    void UpdateAIInternal(uint32 elapsed, bool minimal = false) override;
    void TellError(std::string const botName, std::string const text);

    Player* GetMaster() const { return master; };

    void SaveToDB();

    void HandleSetSecurityKeyCommand(Player* player, std::string const& key);
    void HandleLinkAccountCommand(Player* player, std::string const& accountName, std::string const& key);
    void HandleViewLinkedAccountsCommand(Player* player);
    void HandleUnlinkAccountCommand(Player* player, std::string const& accountName);

protected:
    void OnBotLoginInternal(Player* const bot) override;
    void CheckTellErrors(uint32 elapsed);

private:
    Player* const master;
    PlayerBotErrorMap errors;
    time_t lastErrorTell;
};

class PlayerbotsMgr
{
public:
    static PlayerbotsMgr& instance()
    {
        static PlayerbotsMgr instance;
        return instance;
    }

    void AddPlayerbotData(Player* player, bool isBotAI);
    void RemovePlayerBotData(ObjectGuid const& guid, bool is_AI);

    PlayerbotAI* GetPlayerbotAI(Player* player);
    PlayerbotMgr* GetPlayerbotMgr(Player* player);

private:
    PlayerbotsMgr() = default;
    ~PlayerbotsMgr() = default;

    PlayerbotsMgr(PlayerbotsMgr const&) = delete;
    PlayerbotsMgr& operator=(PlayerbotsMgr const&) = delete;

    PlayerbotsMgr(PlayerbotsMgr&&) = delete;
    PlayerbotsMgr& operator=(PlayerbotsMgr&&) = delete;

    std::unordered_map<ObjectGuid, PlayerbotAIBase*> _playerbotsAIMap;
    std::unordered_map<ObjectGuid, PlayerbotAIBase*> _playerbotsMgrMap;
};

#define sPlayerbotsMgr PlayerbotsMgr::instance()

#endif
