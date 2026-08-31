/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RANDOMPLAYERBOTMGR_H
#define PLAYERBOTS_RANDOMPLAYERBOTMGR_H

#include "GameTime.h"
#include "NewRpgInfo.h"
#include "ObjectGuid.h"
#include "PlayerbotCommandServer.h"
#include "PlayerbotMgr.h"
#include <unordered_set>

// By leewheel 2026-08-01
// 周期性卡顿修复：新增 <array>/<map>/<unordered_map> 头文件依赖，
// 用于 LFG 队列角色计数与入队时间追踪
#include <array>
#include <map>
#include <unordered_map>
// End By leewheel
// By leewheel 2026-08-09: 合并上游 #2633，currentBots 改为 unordered_set 提升性能
#include <unordered_set>
// End By leewheel

struct BattlegroundInfo
{
    std::vector<uint32> bgInstances;
    std::vector<uint32> ratedArenaInstances;
    std::vector<uint32> skirmishArenaInstances;
    uint32 bgInstanceCount = 0;
    uint32 ratedArenaInstanceCount = 0;
    uint32 skirmishArenaInstanceCount = 0;
    uint32 minLevel = 0;
    uint32 maxLevel = 0;
    uint32 activeRatedArenaQueue = 0;     // 0 = Inactive, 1 = Active
    uint32 activeSkirmishArenaQueue = 0;  // 0 = Inactive, 1 = Active
    uint32 activeBgQueue = 0;             // 0 = Inactive, 1 = Active

    // By leewheel 2026-07-07
    // 记录真实玩家开始排队的时间，用于超时强制加入机器人
    time_t bgQueueStartTime = 0;
    // End By leewheel

    // Bots (Arena)
    uint32 ratedArenaBotCount = 0;
    uint32 skirmishArenaBotCount = 0;

    // Bots (Battleground)
    uint32 bgHordeBotCount = 0;
    uint32 bgAllianceBotCount = 0;

    // Players (Arena)
    uint32 ratedArenaPlayerCount = 0;
    uint32 skirmishArenaPlayerCount = 0;

    // Players (Battleground)
    uint32 bgHordePlayerCount = 0;
    uint32 bgAlliancePlayerCount = 0;
};

class ChatHandler;
class PerfMonitorOperation;
class WorldLocation;

struct CachedEvent
{
    uint32 value = 0;
    uint32 lastChangeTime = 0;
    uint32 validIn = 0;
    std::string data;

    bool IsEmpty() const { return !lastChangeTime; }
};

struct BotEventCache
{
    bool loaded = false;
    std::unordered_map<std::string, CachedEvent> events;
};

// https://gist.github.com/bradley219/5373998

class botPIDImpl;
class botPID
{
public:
    // Kp -  proportional gain
    // Ki -  Integral gain
    // Kd -  derivative gain
    // dt -  loop interval time
    // max - maximum value of manipulated variable
    // min - minimum value of manipulated variable
    botPID(double dt, double max, double min, double Kp, double Ki, double Kd);
    void adjust(double Kp, double Ki, double Kd);
    void reset();

    double calculate(double setpoint, double pv);
    ~botPID();

private:
    botPIDImpl* pimpl;
};

class RandomPlayerbotMgr : public PlayerbotHolder
{
public:
    static RandomPlayerbotMgr& instance()
    {
        static RandomPlayerbotMgr instance;

        return instance;
    }

    void LogPlayerLocation();
    void UpdateAIInternal(uint32 elapsed, bool minimal = false) override;

    uint32 activeBots = 0;
    static bool HandlePlayerbotConsoleCommand(ChatHandler* handler, char const* args);
    bool IsRandomBot(Player* bot);
    bool IsRandomBot(ObjectGuid::LowType bot);
    bool IsAddclassBot(Player* bot);
    bool IsAddclassBot(ObjectGuid::LowType bot);
    void Randomize(Player* bot);
    void Clear(Player* bot);
    void RandomizeFirst(Player* bot);
    void RandomizeMin(Player* bot);
    void IncreaseLevel(Player* bot);
    void ScheduleTeleport(uint32 bot, uint32 time = 0);
    void ScheduleChangeStrategy(uint32 bot, uint32 time = 0);
    void HandleCommand(uint32 type, std::string const text, Player* fromPlayer, std::string channelName = "");
    std::string const HandleRemoteCommand(std::string const request);
    void OnPlayerLogout(Player* player);
    void OnPlayerLogin(Player* player);
    void OnPlayerLoginError(uint32 bot);
    Player* GetRandomPlayer();
    std::vector<Player*> GetPlayers() { return players; };
    PlayerBotMap GetAllBots() { return playerBots; };
    void InitArenaTeams();
    void PrintStats();
    double GetBuyMultiplier(Player* bot);
    double GetSellMultiplier(Player* bot);
    void AddTradeDiscount(Player* bot, Player* master, int32 value);
    void SetTradeDiscount(Player* bot, Player* master, uint32 value);
    uint32 GetTradeDiscount(Player* bot, Player* master);
    void Refresh(Player* bot);
    void RandomTeleportForLevel(Player* bot);
    void RandomTeleportGrindForLevel(Player* bot);
    void RandomTeleportForRpg(Player* bot);
    uint32 GetMaxAllowedBotCount();
    bool ProcessBot(Player* player);
    void Revive(Player* player);
    void ChangeStrategy(Player* player);
    void ChangeStrategyOnce(Player* player);
    uint32 GetValue(Player* bot, std::string const& type);
    uint32 GetValue(uint32 bot, std::string const& type);
    std::string GetData(uint32 bot, std::string const& type);
    void SetValue(uint32 bot, std::string const& type, uint32 value, std::string const& data = "");
    void SetValue(Player* bot, std::string const& type, uint32 value, std::string const& data = "");
    bool IsSpecPvp(uint32 bot, uint8 cls);
    void Remove(Player* bot);
    ObjectGuid GetBattleMasterGUID(Player* bot, BattlegroundTypeId bgTypeId);
    CreatureData const* GetCreatureDataByEntry(uint32 entry);
    void LoadBattleMastersCache();
    std::map<uint32, std::map<uint32, BattlegroundInfo>> BattlegroundData;
    std::map<uint32, std::map<uint32, std::map<TeamId, uint32>>> VisualBots;
    std::map<uint32, std::map<uint32, std::map<uint32, uint32>>> Supporters;
    std::map<TeamId, std::vector<uint32>> LfgDungeons;

    // By leewheel 2026-07-10
    // LFG排队追踪数据：记录每个阵营真实玩家开始排队的时间
    // 用于超时强制机器人加入LFG队列
    std::map<TeamId, time_t> lfgQueueStartTime;
    // End By leewheel

    void CheckBgQueue();
    void CheckLfgQueue();
    // By leewheel 2026-07-10
    // 强制机器人加入LFG队列（含天赋切换）
    void ForceBotsJoinLfg(TeamId teamId);
    // End By leewheel

    // By leewheel 2026-08-01
    // 周期性卡顿修复：向 AI 触发器暴露 LFG 队列角色计数（坦克/治疗/DPS），
    // 供 LfgRolePriorityTrigger 判断队列饱和度，避免坦克/治疗机器人无限自主排队。
    std::array<uint32, 3> GetLfgQueueRoleCount(TeamId teamId) const
    {
        auto it = lfgQueueRoleCount.find(teamId);
        return it != lfgQueueRoleCount.end() ? it->second : std::array<uint32, 3>{0, 0, 0};
    }
    // 记录机器人成功进入 LFG 队列的时间，用于滞留超时清理
    void RecordBotLfgJoinTime(ObjectGuid guid) { lfgBotJoinTime[guid] = time(nullptr); }
    time_t GetBotLfgJoinTime(ObjectGuid guid) const
    {
        auto it = lfgBotJoinTime.find(guid);
        return it != lfgBotJoinTime.end() ? it->second : 0;
    }
    void ClearBotLfgJoinTime(ObjectGuid guid) { lfgBotJoinTime.erase(guid); }
    // By leewheel 2026-08-01
    // 周期性卡顿修复：天赋切换节流——ForceBotsJoinLfg 补位时会对候选 bot 执行天赋重置
    // （InitTalentsBySpecNo + InitTalentsTree，重操作：技能/属性重算、大量数据包）。
    // 7月30日 版本无去重，CheckLfgQueue 每 30 秒刷新一次缺额，同一 bot 可能每轮都被反复
    // 切换天赋，与 8 秒撮合周期叠加加剧卡顿。这里对同一 bot 的切换做 30 秒节流。
    bool IsSpecSwitchThrottled(ObjectGuid guid) const
    {
        auto it = botLastSpecSwitchTime.find(guid);
        return it != botLastSpecSwitchTime.end() && time(nullptr) - it->second < 30;
    }
    void RecordSpecSwitchTime(ObjectGuid guid) { botLastSpecSwitchTime[guid] = time(nullptr); }
    // End By leewheel

    void CheckPlayers();
    void LogBattlegroundInfo();

    std::map<TeamId, std::map<BattlegroundTypeId, std::vector<uint32>>> getBattleMastersCache()
    {
        return BattleMastersCache;
    }

    float getActivityMod() { return activityMod; }
    float getActivityPercentage() { return activityMod * 100.0f; }
    void setActivityPercentage(float percentage) { activityMod = percentage / 100.0f; }
    static uint8 GetTeamClassIdx(bool isAlliance, uint8 claz) { return isAlliance * 20 + claz; }

    void PrepareAddclassCache();
    void Init();
    std::map<uint8, std::unordered_set<ObjectGuid>> addclassCache;

    // Account type management
    void AssignAccountTypes();
    bool IsAccountType(uint32 accountId, uint8 accountType);

protected:
    void OnBotLoginInternal(Player* const bot) override;

private:
    RandomPlayerbotMgr() : PlayerbotHolder()
    {
        this->playersLevel = sPlayerbotAIConfig.randombotStartingLevel;

        if (sPlayerbotAIConfig.enabled || sPlayerbotAIConfig.randomBotAutologin)
        {
            PlayerbotCommandServer::instance().Start();
        }

        BattlegroundData.clear();  // Clear here and here only.

        // Cleanup on server start: orphaned pet data that's often left behind by bot pets that no longer exist in the DB
        CharacterDatabase.Execute("DELETE FROM pet_aura WHERE guid NOT IN (SELECT id FROM character_pet)");
        CharacterDatabase.Execute("DELETE FROM pet_spell WHERE guid NOT IN (SELECT id FROM character_pet)");
        CharacterDatabase.Execute("DELETE FROM pet_spell_cooldown WHERE guid NOT IN (SELECT id FROM character_pet)");

        for (int bracket = BG_BRACKET_ID_FIRST; bracket < MAX_BATTLEGROUND_BRACKETS; ++bracket)
        {
            for (int queueType = BATTLEGROUND_QUEUE_AV; queueType < MAX_BATTLEGROUND_QUEUE_TYPES; ++queueType)
            {
                this->BattlegroundData[queueType][bracket] = BattlegroundInfo();
            }
        }

        this->BgCheckTimer = 0;
        this->LfgCheckTimer = 0;
        this->PlayersCheckTimer = 0;
        // By leewheel 2026-08-29 氛围组机器人：重抽计时器归零
        this->AmbienceReshuffleTimer = 0;
        // End By leewheel
        // By leewheel 2026-08-01
        // 周期性卡顿修复：初始化全服扫描限频计时器
        this->FullScanTimer = 0;
        // End By leewheel
    }

    ~RandomPlayerbotMgr() = default;

    RandomPlayerbotMgr(const RandomPlayerbotMgr&) = delete;
    RandomPlayerbotMgr& operator=(const RandomPlayerbotMgr&) = delete;

    RandomPlayerbotMgr(RandomPlayerbotMgr&&) = delete;
    RandomPlayerbotMgr& operator=(RandomPlayerbotMgr&&) = delete;

    // pid values are set in constructor
    botPID pid = botPID(1, 50, -50, 0, 0, 0);
    float activityMod = 0.25;
    bool _isBotInitializing = true;
    bool _isBotLogging = true;
    //By leewheel 2026-09-01 老大需求：随机机器人登录进度周期显示（每10秒一条INFO）
    time_t _loginProgressLastLog = 0;      // 上次进度日志时间
    uint32 _loginProgressLastCount = 0;    // 上次进度日志时的在线机器人数
    uint32 _loginProgressStalls = 0;       // 连续无增长周期数（>=6 视为已稳定）
    bool _loginProgressDone = false;       // 本轮登录进度播报是否结束（完成/稳定后不再刷屏）
    //End By leewheel
    NewRpgStatistic rpgStasticTotal;
    CachedEvent* FindEvent(uint32 bot, std::string const& event);
    uint32 GetEventValue(uint32 bot, std::string const& event);
    std::string GetEventData(uint32 bot, std::string const& event);
    uint32 SetEventValue(uint32 bot, std::string const& event, uint32 value, uint32 validIn,
                         std::string const& data = "");
    void GetBots();
    std::vector<uint32> GetBgBots(uint32 bracket);
    time_t BgCheckTimer;
    time_t LfgCheckTimer;
    time_t PlayersCheckTimer;
    // By leewheel 2026-08-01
    // 周期性卡顿修复：全服扫描限频计时器（CheckLfgQueue 兜底扫描用）
    time_t FullScanTimer;
    // 记录各阵营 LFG 队列中的角色计数（0坦/1奶/2DPS），供 AI 触发器做队列饱和度判断
    std::map<TeamId, std::array<uint32, 3>> lfgQueueRoleCount;
    // 记录机器人成功入队的时间，用于滞留超时强制离队清理
    std::unordered_map<ObjectGuid, time_t> lfgBotJoinTime;
    // 记录机器人上次天赋切换的时间，用于 ForceBotsJoinLfg 天赋重置去重节流
    std::unordered_map<ObjectGuid, time_t> botLastSpecSwitchTime;
    // By leewheel 2026-08-18
    // 各阵营最近一次"强制坦克兜底"的时间，用于超出5分钟仍无坦克时强制抓坦克职业机器人入队的节流计时
    std::map<TeamId, time_t> lfgForceTankTime;
    // End By leewheel
    // End By leewheel
    time_t RealPlayerLastTimeSeen = 0;
    time_t DelayLoginBotsTimer;
    time_t printStatsTimer;
    uint32 AddRandomBots();
    bool ProcessBot(uint32 bot);
    void ScheduleRandomize(uint32 bot, uint32 time);
    void RandomTeleport(Player* bot);
    void RandomTeleport(Player* bot, std::vector<WorldLocation>& locs, bool hearth = false);
    // By leewheel 2026-08-29 氛围组机器人
    void ReshuffleAmbienceBots();                                   // 周期重抽氛围组名单
    bool IsAmbienceBot(uint32 bot) const { return ambienceBots.count(bot) > 0; }
    bool RandomTeleportNearPlayer(Player* bot);                     // 传送到真实玩家附近的hub点
    // End By leewheel
    // By leewheel 2026-08-30 等级分布修复（清除已删除的 mod-rndbot-sync 对数据库的等级伤害）
    uint32 RollNativeBotLevel();                                    // 按模块原生分布掷一个等级
    bool FixBotLevel(Player* bot);                                  // 只升不降：把被拖低的bot重随机回原生分布
    void FixLevelDistribution();                                    // 遍历在线bot批量修复（控制台命令）
    // End By leewheel
    uint32 GetZoneLevel(uint16 mapId, float teleX, float teleY, float teleZ);
    typedef void (RandomPlayerbotMgr::*ConsoleCommandHandler)(Player*);
    std::vector<Player*> players;

    // std::map<uint32, std::vector<WorldLocation>> rpgLocsCache;
    std::map<uint32, std::map<uint32, std::vector<WorldLocation>>> rpgLocsCacheLevel;
    std::map<TeamId, std::map<BattlegroundTypeId, std::vector<uint32>>> BattleMastersCache;
    std::unordered_map<uint32, BotEventCache> eventCache;
    std::unordered_set<uint32> currentBots;
    // By leewheel 2026-08-29 氛围组机器人：名单周期重抽，目的地偏向真实玩家附近
    std::unordered_set<uint32> ambienceBots;
    time_t AmbienceReshuffleTimer;
    // End By leewheel
    uint32 playersLevel;

    // Account lists
    std::vector<uint32> rndBotTypeAccounts;             // Accounts marked as RNDbot (type 1)
    std::vector<uint32> addClassTypeAccounts;           // Accounts marked as AddClass (type 2)

    //void ScaleBotActivity();      // Deprecated function
    static inline uint32 NowSeconds() { return static_cast<uint32>(GameTime::GetGameTime().count()); }
};

#define sRandomPlayerbotMgr RandomPlayerbotMgr::instance()

#endif
