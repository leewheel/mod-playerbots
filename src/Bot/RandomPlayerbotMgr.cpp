/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RandomPlayerbotMgr.h"

#include "AiFactory.h"
#include "Battleground.h"
#include "BattlegroundMgr.h"
#include "Cell.h"
#include "CellImpl.h"
#include "ChannelMgr.h"
#include "DBCStores.h"
#include "DBCStructure.h"
#include "DatabaseEnv.h"
#include "Define.h"
#include "FleeManager.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "LFGMgr.h"
#include "MapMgr.h"
#include "NewRpgInfo.h"
#include "NewRpgStrategy.h"
#include "ObjectGuid.h"
#include "Opcodes.h"
#include "PerfMonitor.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotFactory.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"
#include "Position.h"
#include "RaceMgr.h"
#include "Random.h"
#include "RandomPlayerbotFactory.h"
#include "ServerFacade.h"
#include "SharedDefines.h"
#include "TravelMgr.h"
#include "Unit.h"
#include "World.h"
#include "WorldSessionMgr.h"
#include "WorldPacket.h"
#include <algorithm>
#include <boost/thread/thread.hpp>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <random>

struct GuidClassRaceInfo
{
    ObjectGuid::LowType guid;
    uint32 rClass;
    uint32 rRace;
};

void PrintStatsThread() { sRandomPlayerbotMgr.PrintStats(); }

void activatePrintStatsThread()
{
    boost::thread t(PrintStatsThread);
    t.detach();
}

void CheckBgQueueThread() { sRandomPlayerbotMgr.CheckBgQueue(); }

void activateCheckBgQueueThread()
{
    boost::thread t(CheckBgQueueThread);
    t.detach();
}

void CheckLfgQueueThread() { sRandomPlayerbotMgr.CheckLfgQueue(); }

void activateCheckLfgQueueThread()
{
    boost::thread t(CheckLfgQueueThread);
    t.detach();
}

void CheckPlayersThread() { sRandomPlayerbotMgr.CheckPlayers(); }

void activateCheckPlayersThread()
{
    boost::thread t(CheckPlayersThread);
    t.detach();
}

class botPIDImpl
{
public:
    botPIDImpl(double dt, double max, double min, double Kp, double Ki, double Kd);
    ~botPIDImpl();
    double calculate(double setpoint, double pv);
    void adjust(double Kp, double Ki, double Kd)
    {
        _Kp = Kp;
        _Ki = Ki;
        _Kd = Kd;
    }
    void reset() { _integral = 0; }

private:
    double _dt;
    double _max;
    double _min;
    double _Kp;
    double _Ki;
    double _Kd;
    double _pre_error;
    double _integral;
};

botPID::botPID(double dt, double max, double min, double Kp, double Ki, double Kd)
{
    pimpl = new botPIDImpl(dt, max, min, Kp, Ki, Kd);
}
void botPID::adjust(double Kp, double Ki, double Kd) { pimpl->adjust(Kp, Ki, Kd); }
void botPID::reset() { pimpl->reset(); }
double botPID::calculate(double setpoint, double pv) { return pimpl->calculate(setpoint, pv); }
botPID::~botPID() { delete pimpl; }

/**
 * Implementation
 */
botPIDImpl::botPIDImpl(double dt, double max, double min, double Kp, double Ki, double Kd)
    : _dt(dt), _max(max), _min(min), _Kp(Kp), _Ki(Ki), _Kd(Kd), _pre_error(0), _integral(0)
{
}

double botPIDImpl::calculate(double setpoint, double pv)
{
    // Calculate error
    double error = setpoint - pv;

    // Proportional term
    double Pout = _Kp * error;

    // Integral term
    _integral += error * _dt;
    double Iout = _Ki * _integral;

    // Derivative term
    double derivative = (error - _pre_error) / _dt;
    double Dout = _Kd * derivative;

    // Calculate total output
    double output = Pout + Iout + Dout;

    // Restrict to max/min
    if (output > _max)
    {
        output = _max;
        _integral -= error * _dt;  // Stop integral buildup at max
    }
    else if (output < _min)
    {
        output = _min;
        _integral -= error * _dt;  // Stop integral buildup at min
    }

    // Save error to previous error
    _pre_error = error;

    return output;
}

botPIDImpl::~botPIDImpl() {}

uint32 RandomPlayerbotMgr::GetMaxAllowedBotCount() { return GetEventValue(0, "bot_count"); }

void RandomPlayerbotMgr::LogPlayerLocation()
{
    activeBots = 0;

    try
    {
        sPlayerbotAIConfig.openLog("player_location.csv", "w");

        if (sPlayerbotAIConfig.randomBotAutologin)
        {
            for (auto i : GetAllBots())
            {
                Player* bot = i.second;
                if (!bot)
                    continue;

                std::ostringstream out;
                out << sPlayerbotAIConfig.GetTimestampStr() << "+00,";
                out << "RND"
                    << ",";
                out << bot->GetName() << ",";
                out << std::fixed << std::setprecision(2);
                WorldPosition(bot).printWKT(out);
                out << bot->GetOrientation() << ",";
                out << std::to_string(bot->getRace()) << ",";
                out << std::to_string(bot->getClass()) << ",";
                out << bot->GetMapId() << ",";
                out << bot->GetLevel() << ",";
                out << bot->GetHealth() << ",";
                out << bot->GetPowerPct(bot->getPowerType()) << ",";
                out << bot->GetMoney() << ",";

                if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot))
                {
                    out << std::to_string(uint8(botAI->GetGrouperType())) << ",";
                    out << std::to_string(uint8(botAI->GetGuilderType())) << ",";
                    out << (botAI->AllowActivity(ALL_ACTIVITY) ? "active" : "inactive") << ",";
                    out << (botAI->IsActive() ? "active" : "delay") << ",";
                    out << botAI->HandleRemoteCommand("state") << ",";

                    if (botAI->AllowActivity(ALL_ACTIVITY))
                        activeBots++;
                }
                else
                {
                    out << 0 << "," << 0 << ",err,err,err,";
                }

                out << (bot->IsInCombat() ? "combat" : "safe") << ",";
                out << (bot->isDead() ? (bot->GetCorpse() ? "ghost" : "dead") : "alive");

                sPlayerbotAIConfig.log("player_location.csv", out.str().c_str());
            }

            for (auto i : GetPlayers())
            {
                Player* bot = i;
                if (!bot)
                    continue;

                std::ostringstream out;
                out << sPlayerbotAIConfig.GetTimestampStr() << "+00,";
                out << "PLR"
                    << ",";
                out << bot->GetName() << ",";
                out << std::fixed << std::setprecision(2);
                WorldPosition(bot).printWKT(out);
                out << bot->GetOrientation() << ",";
                out << std::to_string(bot->getRace()) << ",";
                out << std::to_string(bot->getClass()) << ",";
                out << bot->GetMapId() << ",";
                out << bot->GetLevel() << ",";
                out << bot->GetHealth() << ",";
                out << bot->GetPowerPct(bot->getPowerType()) << ",";
                out << bot->GetMoney() << ",";

                if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot))
                {
                    out << std::to_string(uint8(botAI->GetGrouperType())) << ",";
                    out << std::to_string(uint8(botAI->GetGuilderType())) << ",";
                    out << (botAI->AllowActivity(ALL_ACTIVITY) ? "active" : "inactive") << ",";
                    out << (botAI->IsActive() ? "active" : "delay") << ",";
                    out << botAI->HandleRemoteCommand("state") << ",";

                    if (botAI->AllowActivity(ALL_ACTIVITY))
                        activeBots++;
                }
                else
                {
                    out << 0 << "," << 0 << ",player,player,player,";
                }

                out << (bot->IsInCombat() ? "combat" : "safe") << ",";
                out << (bot->isDead() ? (bot->GetCorpse() ? "ghost" : "dead") : "alive");

                sPlayerbotAIConfig.log("player_location.csv", out.str().c_str());
            }
        }
    }
    catch (...)
    {
        return;
        // This is to prevent some thread-unsafeness. Crashes would happen if bots get added or removed.
        // We really don't care here. Just skip a log. Making this thread-safe is not worth the effort.
    }
}

void RandomPlayerbotMgr::UpdateAIInternal(uint32 /*elapsed*/, bool /*minimal*/)
{
    if (totalPmo)
        totalPmo->finish();

    totalPmo = sPerfMonitor.start(PERF_MON_TOTAL, "RandomPlayerbotMgr::FullTick");

    if (!sPlayerbotAIConfig.randomBotAutologin || !sPlayerbotAIConfig.enabled)
        return;

    /*if (sPlayerbotAIConfig.enablePrototypePerformanceDiff)
    {
        LOG_INFO("playerbots", "---------------------------------------");
        LOG_INFO("playerbots",
                 "原型：玩家机器人性能增强已启用，可能出现问题和不稳定。");
        LOG_INFO("playerbots", "---------------------------------------");
        ScaleBotActivity();
    }*/

    uint32 maxAllowedBotCount = GetEventValue(0, "bot_count");
    if (!maxAllowedBotCount || (maxAllowedBotCount < sPlayerbotAIConfig.minRandomBots ||
                                maxAllowedBotCount > sPlayerbotAIConfig.maxRandomBots))
    {
        maxAllowedBotCount = urand(sPlayerbotAIConfig.minRandomBots, sPlayerbotAIConfig.maxRandomBots);
        SetEventValue(0, "bot_count", maxAllowedBotCount,
                      urand(sPlayerbotAIConfig.randomBotCountChangeMinInterval,
                            sPlayerbotAIConfig.randomBotCountChangeMaxInterval));
    }

    GetBots();
    // Copied deliberately: ProcessBot() erases from currentBots while this is
    // being iterated below, so the loops must run over a snapshot.
    std::unordered_set<uint32> availableBots = currentBots;
    uint32 availableBotCount = availableBots.size();
    uint32 onlineBotCount = playerBots.size();

    uint32 onlineBotFocus = 75;
    if (onlineBotCount < (uint32)(sPlayerbotAIConfig.minRandomBots * 90 / 100))
        onlineBotFocus = 25;

    // only keep updating till initializing time has completed,
    // which prevents unneeded expensive GameTime calls.
    if (_isBotInitializing)
    {
        _isBotInitializing = GameTime::GetUptime().count() < sPlayerbotAIConfig.maxRandomBots * (0.11 + 0.4);
    }

    uint32 updateIntervalTurboBoost = _isBotInitializing ? 1 : sPlayerbotAIConfig.randomBotUpdateInterval;
    SetNextCheckDelay(updateIntervalTurboBoost * (onlineBotFocus + 25) * 10);

    PerfMonitorOperation* pmo = sPerfMonitor.start(
        PERF_MON_TOTAL,
        onlineBotCount < maxAllowedBotCount ? "RandomPlayerbotMgr::Login" : "RandomPlayerbotMgr::UpdateAIInternal");

    bool realPlayerIsLogged = false;
    if (sPlayerbotAIConfig.disabledWithoutRealPlayer)
    {
        if (sWorldSessionMgr->GetActiveAndQueuedSessionCount() > 0)
        {
            RealPlayerLastTimeSeen = time(nullptr);
            realPlayerIsLogged = true;

            if (DelayLoginBotsTimer == 0)
            {
                DelayLoginBotsTimer = time(nullptr) + sPlayerbotAIConfig.disabledWithoutRealPlayerLoginDelay;
            }
        }
        else
        {
            if (DelayLoginBotsTimer)
            {
                DelayLoginBotsTimer = 0;
            }

            if (RealPlayerLastTimeSeen != 0 && onlineBotCount > 0 &&
                time(nullptr) > RealPlayerLastTimeSeen + sPlayerbotAIConfig.disabledWithoutRealPlayerLogoutDelay)
            {
                LogoutAllBots();
                LOG_INFO("playerbots", "没有真实玩家会话，登出所有机器人。");
            }
        }

        if (availableBotCount < maxAllowedBotCount &&
            (sPlayerbotAIConfig.disabledWithoutRealPlayer == false ||
             (realPlayerIsLogged && DelayLoginBotsTimer != 0 && time(nullptr) >= DelayLoginBotsTimer)))
        {
            AddRandomBots();
        }
    }
    else if (availableBotCount < maxAllowedBotCount)
    {
        AddRandomBots();
    }

    if (sPlayerbotAIConfig.syncLevelWithPlayers && !players.empty())
    {
        if (time(nullptr) > (PlayersCheckTimer + 60))
            sRandomPlayerbotMgr.CheckPlayers();
    }

    if (sPlayerbotAIConfig.randomBotJoinBG /* && !players.empty()*/)
    {
        // By leewheel 2026-07-07
        // 缩短战场队列检查间隔：35秒 → 15秒，让机器人更快响应玩家排队
        if (time(nullptr) > (BgCheckTimer + 15))
            sRandomPlayerbotMgr.CheckBgQueue();
        // End By leewheel
    }

    if (sPlayerbotAIConfig.randomBotJoinLfg /* && !players.empty()*/)
    {
        if (time(nullptr) > (LfgCheckTimer + 30))
            sRandomPlayerbotMgr.CheckLfgQueue();
    }

    if (sPlayerbotAIConfig.randomBotAutologin && sPlayerbotAIConfig.randomBotPrintStatsInterval &&
        time(nullptr) > (printStatsTimer + sPlayerbotAIConfig.randomBotPrintStatsInterval))
    {
        if (!printStatsTimer)
        {
            printStatsTimer = time(nullptr);
        }
        else
        {
            sRandomPlayerbotMgr.PrintStats();
            // activatePrintStatsThread();
        }
    }
    uint32 updateBots = sPlayerbotAIConfig.randomBotsPerInterval * onlineBotFocus / 100;
    uint32 maxNewBots =
        onlineBotCount < maxAllowedBotCount &&
                (sPlayerbotAIConfig.disabledWithoutRealPlayer == false ||
                 (realPlayerIsLogged && DelayLoginBotsTimer != 0 && time(nullptr) >= DelayLoginBotsTimer))
            ? maxAllowedBotCount - onlineBotCount
            : 0;
    uint32 loginBots = std::min(sPlayerbotAIConfig.randomBotsPerInterval - updateBots, maxNewBots);

    if (!availableBots.empty())
    {
        // Update bots
        for (auto bot : availableBots)
        {
            if (!GetPlayerBot(bot))
                continue;

            if (ProcessBot(bot))
            {
                updateBots--;
            }

            if (!updateBots)
                break;
        }

        if (loginBots && botLoading.empty())
        {
            loginBots += updateBots;
            loginBots = std::min(loginBots, maxNewBots);

            LOG_DEBUG("playerbots", "{} 个新机器人准备登录", loginBots);

            // Log in bots
            for (auto bot : availableBots)
            {
                if (GetPlayerBot(bot))
                    continue;

                if (ProcessBot(bot))
                {
                    loginBots--;
                }

                if (!loginBots)
                    break;
            }

            DelayLoginBotsTimer = 0;
        }
    }

    if (pmo)
        pmo->finish();

    if (sPlayerbotAIConfig.hasLog("player_location.csv"))
    {
        LogPlayerLocation();
    }
}

// void RandomPlayerbotMgr::ScaleBotActivity()
//{
//     float activityPercentage = getActivityPercentage();
//
//     // if (activityPercentage >= 100.0f || activityPercentage <= 0.0f) pid.reset(); //Stop integer buildup during
//     // max/min activity
//
//     //    % increase/decrease                   wanted diff                                         , avg diff
//     float activityPercentageMod = pid.calculate(
//         sRandomPlayerbotMgr.GetPlayers().empty() ? sPlayerbotAIConfig.diffEmpty :
//         sPlayerbotAIConfig.diffWithPlayer, sWorldUpdateTime.GetAverageUpdateTime());
//
//     activityPercentage = activityPercentageMod + 50;
//
//     // Cap the percentage between 0 and 100.
//     activityPercentage = std::max(0.0f, std::min(100.0f, activityPercentage));
//
//     setActivityPercentage(activityPercentage);
// }

// Assigns accounts as RNDbot accounts (type 1) based on MaxRandomBots and EnablePeriodicOnlineOffline and its ratio,
// and assigns accounts as AddClass accounts (type 2) based AddClassAccountPoolSize. Type 1 and 2 assignments are
// permenant, unless MaxRandomBots or AddClassAccountPoolSize are set to 0. If so, their associated accounts will
// be unassigned (type 0)
void RandomPlayerbotMgr::AssignAccountTypes()
{
    LOG_INFO("playerbots", "正在为随机机器人账号分配账号类型...");

    // Clear existing filtered lists
    rndBotTypeAccounts.clear();
    addClassTypeAccounts.clear();

    // First, get ALL randombot accounts from the database
    std::vector<uint32> allRandomBotAccounts;
    QueryResult allAccounts = LoginDatabase.Query(
        "SELECT id FROM account WHERE username LIKE '{}%%' ORDER BY id",
        sPlayerbotAIConfig.randomBotAccountPrefix.c_str());

    if (allAccounts)
    {
        do
        {
            Field* fields = allAccounts->Fetch();
            uint32 accountId = fields[0].Get<uint32>();
            allRandomBotAccounts.push_back(accountId);
        } while (allAccounts->NextRow());
    }

    LOG_INFO("playerbots", "数据库中共找到 {} 个随机机器人账号", allRandomBotAccounts.size());

    // Check existing assignments
    QueryResult existingAssignments = PlayerbotsDatabase.Query("SELECT account_id, account_type FROM playerbots_account_type");
    std::map<uint32, uint8> currentAssignments;

    if (existingAssignments)
    {
        do
        {
            Field* fields = existingAssignments->Fetch();
            uint32 accountId = fields[0].Get<uint32>();
            uint8 accountType = fields[1].Get<uint8>();
            currentAssignments[accountId] = accountType;
        } while (existingAssignments->NextRow());
    }

    // Mark ALL randombot accounts as unassigned if not already assigned
    for (uint32 accountId : allRandomBotAccounts)
    {
        if (currentAssignments.find(accountId) == currentAssignments.end())
        {
            PlayerbotsDatabase.Execute("INSERT INTO playerbots_account_type (account_id, account_type) VALUES ({}, 0) ON DUPLICATE KEY UPDATE account_type = account_type", accountId);
            currentAssignments[accountId] = 0;
        }
    }

    // Calculate needed RNDbot accounts
    uint32 neededRndBotAccounts = 0;
    if (sPlayerbotAIConfig.maxRandomBots > 0)
    {
        int divisor = RandomPlayerbotFactory::CalculateAvailableCharsPerAccount();
        int maxBots = sPlayerbotAIConfig.maxRandomBots;

        // Take periodic online-offline into account
        if (sPlayerbotAIConfig.enablePeriodicOnlineOffline)
        {
            maxBots *= sPlayerbotAIConfig.periodicOnlineOfflineRatio;
        }

        // Calculate base accounts needed for RNDbots, ensuring round up for maxBots not cleanly divisible by the divisor
        neededRndBotAccounts = (maxBots + divisor - 1) / divisor;
    }

    // Count existing assigned accounts
    uint32 existingRndBotAccounts = 0;
    uint32 existingAddClassAccounts = 0;

    for (auto const& [accountId, accountType] : currentAssignments)
    {
        if (accountType == 1) existingRndBotAccounts++;
        else if (accountType == 2) existingAddClassAccounts++;
    }

    // Assign RNDbot accounts from lowest position if needed
    if (existingRndBotAccounts < neededRndBotAccounts)
    {
        uint32 toAssign = neededRndBotAccounts - existingRndBotAccounts;
        uint32 assigned = 0;

        for (uint32 i = 0; i < allRandomBotAccounts.size() && assigned < toAssign; i++)
        {
            uint32 accountId = allRandomBotAccounts[i];
            if (currentAssignments[accountId] == 0) // Unassigned
            {
                PlayerbotsDatabase.Execute("UPDATE playerbots_account_type SET account_type = 1, assignment_date = NOW() WHERE account_id = {}", accountId);
                currentAssignments[accountId] = 1;
                assigned++;
            }
        }

        if (assigned < toAssign)
        {
            LOG_ERROR("playerbots", "未分配账号不足以满足 RNDbot 需求，还需要 {} 个账号。", toAssign - assigned);
        }
    }

    // Assign AddClass accounts from highest position if needed
    uint32 neededAddClassAccounts = sPlayerbotAIConfig.addClassAccountPoolSize;

    if (existingAddClassAccounts < neededAddClassAccounts)
    {
        uint32 toAssign = neededAddClassAccounts - existingAddClassAccounts;
        uint32 assigned = 0;

        for (size_t idx = allRandomBotAccounts.size(); idx-- > 0 && assigned < toAssign;)
        {
            uint32 accountId = allRandomBotAccounts[idx];
            if (currentAssignments[accountId] == 0) // Unassigned
            {
                PlayerbotsDatabase.Execute("UPDATE playerbots_account_type SET account_type = 2, assignment_date = NOW() WHERE account_id = {}", accountId);
                currentAssignments[accountId] = 2;
                assigned++;
            }
        }

        if (assigned < toAssign)
        {
            LOG_ERROR("playerbots", "未分配账号不足以满足 AddClass 需求，还需要 {} 个账号。", toAssign - assigned);
        }
    }

    // Populate filtered account lists with ALL accounts of each type
    for (auto const& [accountId, accountType] : currentAssignments)
    {
        if (accountType == 1) rndBotTypeAccounts.push_back(accountId);
        else if (accountType == 2) addClassTypeAccounts.push_back(accountId);
    }

    LOG_INFO("playerbots", "账号类型分配完成：{} 个 RNDbot 账号，{} 个 AddClass 账号，{} 个未分配",
             rndBotTypeAccounts.size(), addClassTypeAccounts.size(),
             currentAssignments.size() - rndBotTypeAccounts.size() - addClassTypeAccounts.size());
}

bool RandomPlayerbotMgr::IsAccountType(uint32 accountId, uint8 accountType)
{
    QueryResult result = PlayerbotsDatabase.Query("SELECT 1 FROM playerbots_account_type WHERE account_id = {} AND account_type = {}", accountId, accountType);
    return result != nullptr;
}

// Logs-in bots in 4 phases. Phase 1 logs Alliance bots up to how much is expected according to the faction ratio,
// and Phase 2 logs-in the remainder Horde bots to reach the total maxAllowedBotCount. If maxAllowedBotCount is not
// reached after Phase 2, the function goes back to log-in Alliance bots and reach maxAllowedBotCount. This is done
// because not every account is guaranteed 5A/5H bots, so the true ratio might be skewed by few percentages. Finally,
// Phase 4 is reached if and only if the value of RandomBotAccountCount is lower than it should.
uint32 RandomPlayerbotMgr::AddRandomBots()
{
    uint32 maxAllowedBotCount = GetEventValue(0, "bot_count");
    static time_t missingBotsTimer = 0;

    if (currentBots.size() < maxAllowedBotCount)
    {
        // Calculate how many bots to add
        maxAllowedBotCount -= currentBots.size();
        maxAllowedBotCount = std::min(sPlayerbotAIConfig.randomBotsPerInterval, maxAllowedBotCount);

        // Single RNG instance for all shuffling
        std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());

        // Only need to track the Alliance count, as it's in Phase 1
        uint32 totalRatio = sPlayerbotAIConfig.randomBotAllianceRatio + sPlayerbotAIConfig.randomBotHordeRatio;
        uint32 allowedAllianceCount = maxAllowedBotCount * (sPlayerbotAIConfig.randomBotAllianceRatio) / totalRatio;

        uint32 remainder = maxAllowedBotCount * (sPlayerbotAIConfig.randomBotAllianceRatio) % totalRatio;

        // Fix #1082: Randomly add one based on reminder
        if (remainder && urand(1, totalRatio) <= remainder)
        {
            allowedAllianceCount++;
        }

        // Determine which accounts to use based on EnablePeriodicOnlineOffline
        std::vector<uint32> accountsToUse;
        if (sPlayerbotAIConfig.enablePeriodicOnlineOffline)
        {

            // Calculate how many accounts can be used
            // With enablePeriodicOnlineOffline, don't use all of rndBotTypeAccounts right away. Fraction results are rounded up
            uint32 accountsToUseCount = (rndBotTypeAccounts.size() + sPlayerbotAIConfig.periodicOnlineOfflineRatio - 1)
                                        / sPlayerbotAIConfig.periodicOnlineOfflineRatio;

            // Randomly select accounts
            std::vector<uint32> shuffledAccounts = rndBotTypeAccounts;
            std::shuffle(shuffledAccounts.begin(), shuffledAccounts.end(), rng);

            for (uint32 i = 0; i < accountsToUseCount && i < shuffledAccounts.size(); i++)
            {
                accountsToUse.push_back(shuffledAccounts[i]);
            }
        }
        else
        {
            accountsToUse = rndBotTypeAccounts;
        }

        // Pre-map all characters from selected accounts
        struct CharacterInfo
        {
            uint32 guid;
            uint8 rClass;
            uint8 rRace;
            uint32 accountId;
        };
        std::vector<CharacterInfo> allCharacters;

        for (uint32 accountId : accountsToUse)
        {
            // By leewheel 2026-08-29 - 原语句 CHAR_SEL_CHARS_BY_ACCOUNT_ID 已改为只查 guid 单列，
            //   继续按多列读取会越界读到野内存并在 Field::GetData<uint8>() 里解引用崩溃
            //   （worldserver 100% ACCESS_VIOLATION）。
            //   这里改用主项目核心已注册的三列版专用语句，列顺序 guid / class / race 与下方读取一一对应。
            CharacterDatabasePreparedStatement* stmt =
                CharacterDatabase.GetPreparedStatement(CHAR_SEL_PBOT_CHARS_CLASS_RACE_BY_ACCOUNT_ID);
            // End By leewheel
            stmt->SetData(0, accountId);
            PreparedQueryResult result = CharacterDatabase.Query(stmt);
            if (!result)
                continue;

            do
            {
                Field* fields = result->Fetch();
                CharacterInfo info;
                // By leewheel 2026-08-29 - 列索引与三列专用语句严格对应：0=guid, 1=class, 2=race
                info.guid = fields[0].Get<uint32>();
                info.rClass = fields[1].Get<uint8>();
                info.rRace = fields[2].Get<uint8>();
                // End By leewheel
                info.accountId = accountId;
                allCharacters.push_back(info);
            } while (result->NextRow());
        }

        // Shuffle for class balance
        std::shuffle(allCharacters.begin(), allCharacters.end(), rng);

        // Separate characters by faction for phased login
        std::vector<CharacterInfo> allianceChars;
        std::vector<CharacterInfo> hordeChars;

        for (auto const& charInfo : allCharacters)
        {
            if (IsAlliance(charInfo.rRace))
                allianceChars.push_back(charInfo);

            else
                hordeChars.push_back(charInfo);
        }

        // Lambda to handle bot login logic
        auto tryLoginBot = [&](const CharacterInfo& charInfo) -> bool
        {
            if (GetEventValue(charInfo.guid, "add") ||
                GetEventValue(charInfo.guid, "logout") ||
                GetPlayerBot(charInfo.guid) ||
                currentBots.contains(charInfo.guid) ||
                (sPlayerbotAIConfig.disableDeathKnightLogin && charInfo.rClass == CLASS_DEATH_KNIGHT))
            {
                return false;
            }

            uint32 add_time = sPlayerbotAIConfig.enablePeriodicOnlineOffline
                                ? urand(sPlayerbotAIConfig.minRandomBotInWorldTime,
                                        sPlayerbotAIConfig.maxRandomBotInWorldTime)
                                : sPlayerbotAIConfig.permanentlyInWorldTime;

            SetEventValue(charInfo.guid, "add", 1, add_time);
            SetEventValue(charInfo.guid, "logout", 0, 0);
            currentBots.insert(charInfo.guid);

            return true;
        };

        // PHASE 1: Log-in Alliance bots up to allowedAllianceCount
        for (auto const& charInfo : allianceChars)
        {
            if (!allowedAllianceCount)
                break;

            if (tryLoginBot(charInfo))
            {
                maxAllowedBotCount--;
                allowedAllianceCount--;
            }
        }

        // PHASE 2: Log-in Horde bots up to maxAllowedBotCount
        for (auto const& charInfo : hordeChars)
        {
            if (!maxAllowedBotCount)
                break;

            if (tryLoginBot(charInfo))
                maxAllowedBotCount--;
        }

        // PHASE 3: If maxAllowedBotCount wasn't reached, log-in more Alliance bots
        for (auto const& charInfo : allianceChars)
        {
            if (!maxAllowedBotCount)
                break;

            if (tryLoginBot(charInfo))
                maxAllowedBotCount--;
        }

        // PHASE 4: An error is given if maxAllowedBotCount is still not reached
        if (maxAllowedBotCount)
        {
            if (missingBotsTimer == 0)
                missingBotsTimer = time(nullptr);

            if (time(nullptr) - missingBotsTimer >= 10)
            {
                int divisor = RandomPlayerbotFactory::CalculateAvailableCharsPerAccount();
                uint32 moreAccountsNeeded = (maxAllowedBotCount + divisor - 1) / divisor;
                LOG_ERROR("playerbots",
                          "无法登录所有请求的机器人，请尝试在配置文件中增加 RandomBotAccountCount。还需要 {} 个账号。",
                          moreAccountsNeeded);
                missingBotsTimer = 0;    // Reset timer so error is not spammed every tick
            }
        }
        else
        {
            missingBotsTimer = 0;       // Reset timer if logins for this interval were successful
        }
    }
    else
    {
        missingBotsTimer = 0;           // Reset timer if there's enough bots
    }

    return currentBots.size();
}

void RandomPlayerbotMgr::LoadBattleMastersCache()
{
    BattleMastersCache.clear();

    LOG_INFO("playerbots", "正在加载战场管理员缓存...");

    QueryResult result = WorldDatabase.Query("SELECT `entry`,`bg_template` FROM `battlemaster_entry`");

    uint32 count = 0;

    if (!result)
    {
        return;
    }

    do
    {
        ++count;

        Field* fields = result->Fetch();

        uint32 entry = fields[0].Get<uint32>();
        uint32 bgTypeId = fields[1].Get<uint32>();

        CreatureTemplate const* bmaster = sObjectMgr->GetCreatureTemplate(entry);
        if (!bmaster)
            continue;

        FactionTemplateEntry const* bmFaction = sFactionTemplateStore.LookupEntry(bmaster->faction);
        uint32 bmFactionId = bmFaction->faction;
        FactionEntry const* bmParentFaction = sFactionStore.LookupEntry(bmFactionId);
        uint32 bmParentTeam = bmParentFaction->team;
        TeamId bmTeam = TEAM_NEUTRAL;
        if (bmParentTeam == 891)
            bmTeam = TEAM_ALLIANCE;

        if (bmFactionId == 189)
            bmTeam = TEAM_ALLIANCE;

        if (bmParentTeam == 892)
            bmTeam = TEAM_HORDE;

        if (bmFactionId == 66)
            bmTeam = TEAM_HORDE;

        BattleMastersCache[bmTeam][BattlegroundTypeId(bgTypeId)].insert(
            BattleMastersCache[bmTeam][BattlegroundTypeId(bgTypeId)].end(), entry);
        LOG_DEBUG("playerbots", "已缓存战场管理员 #{}，战场类型 {} ({})", entry, bgTypeId,
                  bmTeam == TEAM_ALLIANCE ? "Alliance"
                  : bmTeam == TEAM_HORDE  ? "Horde"
                                          : "Neutral");

    } while (result->NextRow());

    LOG_INFO("playerbots", ">> 已加载 {} 条战场管理员记录", count);
}

std::vector<uint32> parseBrackets(const std::string& str)
{
    std::vector<uint32> brackets;
    std::stringstream ss(str);
    std::string item;

    while (std::getline(ss, item, ','))
    {
        brackets.push_back(static_cast<uint32>(std::stoi(item)));
    }

    return brackets;
}

void RandomPlayerbotMgr::CheckBgQueue()
{
    if (!BgCheckTimer)
    {
        BgCheckTimer = time(nullptr);
        return;  // Exit immediately after initializing the timer
    }

    if (time(nullptr) < BgCheckTimer)
    {
        return;  // No need to proceed if the current time is less than the timer
    }

    // Update the timer to the current time
    BgCheckTimer = time(nullptr);

    LOG_DEBUG("playerbots", "正在检查战场队列...");

    // Initialize Battleground Data (do not clear here)

    // By leewheel 2026-07-07
    // 重置BattlegroundData时保留bgQueueStartTime，确保排队开始时间不被每次检查重置
    // 这样可以正确追踪真实玩家从开始排队到现在的总等待时间
    for (int bracket = BG_BRACKET_ID_FIRST; bracket < MAX_BATTLEGROUND_BRACKETS; ++bracket)
    {
        for (int queueType = BATTLEGROUND_QUEUE_AV; queueType < MAX_BATTLEGROUND_QUEUE_TYPES; ++queueType)
        {
            time_t savedQueueStartTime = BattlegroundData[queueType][bracket].bgQueueStartTime;
            BattlegroundData[queueType][bracket] = BattlegroundInfo();
            BattlegroundData[queueType][bracket].bgQueueStartTime = savedQueueStartTime;
        }
    }
    // End By leewheel

    // Process real players and populate Battleground Data with player/queue count
    // Opens a queue for bots to join
    for (Player* player : players)
    {
        // Skip player if not currently in a queue
        if (!player->InBattlegroundQueue())
            continue;

        Battleground* bg = player->GetBattleground();
        if (bg && bg->GetStatus() == STATUS_WAIT_LEAVE)
            continue;

        TeamId teamId = player->GetTeamId();

        for (uint8 queueType = 0; queueType < PLAYER_MAX_BATTLEGROUND_QUEUES; ++queueType)
        {
            BattlegroundQueueTypeId queueTypeId = player->GetBattlegroundQueueTypeId(queueType);
            if (queueTypeId == BATTLEGROUND_QUEUE_NONE)
                continue;

            // Check if real player is able to create/join this queue
            BattlegroundTypeId bgTypeId = sBattlegroundMgr->BGTemplateId(queueTypeId);
            uint32 mapId = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId)->GetMapId();
            PvPDifficultyEntry const* pvpDiff = GetBattlegroundBracketByLevel(mapId, player->GetLevel());
            if (!pvpDiff)
                continue;

            // If player is allowed, populate the BattlegroundData with the appropriate level requirements
            BattlegroundBracketId bracketId = pvpDiff->GetBracketId();
            BattlegroundData[queueTypeId][bracketId].minLevel = pvpDiff->minLevel;
            BattlegroundData[queueTypeId][bracketId].maxLevel = pvpDiff->maxLevel;

            // Arena logic
            bool isRated = false;
            if (BattlegroundMgr::BGArenaType(queueTypeId))
            {
                BattlegroundQueue& bgQueue = sBattlegroundMgr->GetBattlegroundQueue(queueTypeId);
                GroupQueueInfo ginfo;

                if (bgQueue.GetPlayerGroupInfoData(player->GetGUID(), &ginfo))
                {
                    isRated = ginfo.IsRated;
                }

                if (bgQueue.IsPlayerInvitedToRatedArena(player->GetGUID()) ||
                    (player->InArena() && player->GetBattleground()->isRated()))
                    isRated = true;

                if (isRated)
                    BattlegroundData[queueTypeId][bracketId].ratedArenaPlayerCount++;
                else
                    BattlegroundData[queueTypeId][bracketId].skirmishArenaPlayerCount++;
            }
            // BG Logic
            else
            {
                if (teamId == TEAM_ALLIANCE)
                    BattlegroundData[queueTypeId][bracketId].bgAlliancePlayerCount++;
                else
                    BattlegroundData[queueTypeId][bracketId].bgHordePlayerCount++;

                // If a player has joined the BG, update the instance count in BattlegroundData (for consistency)
                if (player->InBattleground())
                {
                    std::vector<uint32>* instanceIds = nullptr;
                    uint32 instanceId = player->GetBattleground()->GetInstanceID();

                    instanceIds = &BattlegroundData[queueTypeId][bracketId].bgInstances;
                    if (instanceIds &&
                        std::find(instanceIds->begin(), instanceIds->end(), instanceId) == instanceIds->end())
                        instanceIds->push_back(instanceId);

                    BattlegroundData[queueTypeId][bracketId].bgInstanceCount = instanceIds->size();
                }
            }

            if (!player->IsInvitedForBattlegroundInstance() && !player->InBattleground())
            {
                if (BattlegroundMgr::BGArenaType(queueTypeId))
                {
                    if (isRated)
                        BattlegroundData[queueTypeId][bracketId].activeRatedArenaQueue = 1;
                    else
                        BattlegroundData[queueTypeId][bracketId].activeSkirmishArenaQueue = 1;
                }
                else
                {
                    BattlegroundData[queueTypeId][bracketId].activeBgQueue = 1;
                    // By leewheel 2026-07-07
                    // 记录真实玩家开始排队的时间（仅首次记录），用于超时强制加入机器人
                    if (BattlegroundData[queueTypeId][bracketId].bgQueueStartTime == 0)
                        BattlegroundData[queueTypeId][bracketId].bgQueueStartTime = time(nullptr);
                    // End By leewheel
                }
            }
        }
    }

    // By leewheel 2026-07-07
    // 清理没有真实玩家排队的队列的bgQueueStartTime
    // 当activeBgQueue为0时，说明该队列没有真实玩家在等待，重置排队开始时间
    // 这样下次有新玩家排队时，计时器会重新开始
    for (int bracket = BG_BRACKET_ID_FIRST; bracket < MAX_BATTLEGROUND_BRACKETS; ++bracket)
    {
        for (int queueType = BATTLEGROUND_QUEUE_AV; queueType < MAX_BATTLEGROUND_QUEUE_TYPES; ++queueType)
        {
            if (BattlegroundData[queueType][bracket].activeBgQueue == 0 &&
                BattlegroundData[queueType][bracket].bgQueueStartTime != 0)
            {
                BattlegroundData[queueType][bracket].bgQueueStartTime = 0;
            }
        }
    }
    // End By leewheel

    // Process player bots
    for (auto& [guid, bot] : playerBots)
    {
        if (!bot || !bot->InBattlegroundQueue() || !bot->IsInWorld() || !IsRandomBot(bot))
            continue;

        Battleground* bg = bot->GetBattleground();
        if (bg && bg->GetStatus() == STATUS_WAIT_LEAVE)
            continue;

        TeamId teamId = bot->GetTeamId();

        for (uint8 queueType = 0; queueType < PLAYER_MAX_BATTLEGROUND_QUEUES; ++queueType)
        {
            BattlegroundQueueTypeId queueTypeId = bot->GetBattlegroundQueueTypeId(queueType);
            if (queueTypeId == BATTLEGROUND_QUEUE_NONE)
                continue;

            BattlegroundTypeId bgTypeId = sBattlegroundMgr->BGTemplateId(queueTypeId);
            uint32 mapId = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId)->GetMapId();
            PvPDifficultyEntry const* pvpDiff = GetBattlegroundBracketByLevel(mapId, bot->GetLevel());
            if (!pvpDiff)
                continue;

            BattlegroundBracketId bracketId = pvpDiff->GetBracketId();
            BattlegroundData[queueTypeId][bracketId].minLevel = pvpDiff->minLevel;
            BattlegroundData[queueTypeId][bracketId].maxLevel = pvpDiff->maxLevel;

            if (BattlegroundMgr::BGArenaType(queueTypeId))
            {
                bool isRated = false;
                BattlegroundQueue& bgQueue = sBattlegroundMgr->GetBattlegroundQueue(queueTypeId);
                GroupQueueInfo ginfo;

                if (bgQueue.GetPlayerGroupInfoData(guid, &ginfo))
                {
                    isRated = ginfo.IsRated;
                }

                if (bgQueue.IsPlayerInvitedToRatedArena(guid) || (bot->InArena() && bot->GetBattleground()->isRated()))
                    isRated = true;

                if (isRated)
                    BattlegroundData[queueTypeId][bracketId].ratedArenaBotCount++;
                else
                    BattlegroundData[queueTypeId][bracketId].skirmishArenaBotCount++;
            }
            else
            {
                if (teamId == TEAM_ALLIANCE)
                    BattlegroundData[queueTypeId][bracketId].bgAllianceBotCount++;
                else
                    BattlegroundData[queueTypeId][bracketId].bgHordeBotCount++;
            }

            if (bot->InBattleground())
            {
                std::vector<uint32>* instanceIds = nullptr;
                uint32 instanceId = bot->GetBattleground()->GetInstanceID();
                bool isArena = false;
                bool isRated = false;

                // Arena logic
                if (bot->InArena())
                {
                    isArena = true;
                    if (bot->GetBattleground()->isRated())
                    {
                        isRated = true;
                        instanceIds = &BattlegroundData[queueTypeId][bracketId].ratedArenaInstances;
                    }
                    else
                    {
                        instanceIds = &BattlegroundData[queueTypeId][bracketId].skirmishArenaInstances;
                    }
                }
                // BG Logic
                else
                {
                    instanceIds = &BattlegroundData[queueTypeId][bracketId].bgInstances;
                }

                if (instanceIds &&
                    std::find(instanceIds->begin(), instanceIds->end(), instanceId) == instanceIds->end())
                    instanceIds->push_back(instanceId);

                if (isArena)
                {
                    if (isRated)
                        BattlegroundData[queueTypeId][bracketId].ratedArenaInstanceCount = instanceIds->size();
                    else
                        BattlegroundData[queueTypeId][bracketId].skirmishArenaInstanceCount = instanceIds->size();
                }
                else
                {
                    BattlegroundData[queueTypeId][bracketId].bgInstanceCount = instanceIds->size();
                }
            }
        }
    }

    // If enabled, wait for all bots to have logged in before queueing for Arena's / BG's
    if (sPlayerbotAIConfig.randomBotAutoJoinBG && playerBots.size() >= GetMaxAllowedBotCount())
    {
        uint32 randomBotAutoJoinArenaBracket = sPlayerbotAIConfig.randomBotAutoJoinArenaBracket;
        uint32 randomBotAutoJoinBGRatedArena2v2Count = sPlayerbotAIConfig.randomBotAutoJoinBGRatedArena2v2Count;
        uint32 randomBotAutoJoinBGRatedArena3v3Count = sPlayerbotAIConfig.randomBotAutoJoinBGRatedArena3v3Count;
        uint32 randomBotAutoJoinBGRatedArena5v5Count = sPlayerbotAIConfig.randomBotAutoJoinBGRatedArena5v5Count;

        uint32 randomBotAutoJoinBGICCount = sPlayerbotAIConfig.randomBotAutoJoinBGICCount;
        uint32 randomBotAutoJoinBGEYCount = sPlayerbotAIConfig.randomBotAutoJoinBGEYCount;
        uint32 randomBotAutoJoinBGAVCount = sPlayerbotAIConfig.randomBotAutoJoinBGAVCount;
        uint32 randomBotAutoJoinBGABCount = sPlayerbotAIConfig.randomBotAutoJoinBGABCount;
        uint32 randomBotAutoJoinBGWSCount = sPlayerbotAIConfig.randomBotAutoJoinBGWSCount;

        std::vector<uint32> icBrackets = parseBrackets(sPlayerbotAIConfig.randomBotAutoJoinICBrackets);
        std::vector<uint32> eyBrackets = parseBrackets(sPlayerbotAIConfig.randomBotAutoJoinEYBrackets);
        std::vector<uint32> avBrackets = parseBrackets(sPlayerbotAIConfig.randomBotAutoJoinAVBrackets);
        std::vector<uint32> abBrackets = parseBrackets(sPlayerbotAIConfig.randomBotAutoJoinABBrackets);
        std::vector<uint32> wsBrackets = parseBrackets(sPlayerbotAIConfig.randomBotAutoJoinWSBrackets);

        // Check both bgInstanceCount / bgInstances.size
        // to help counter against potentional inconsistencies
        auto updateRatedArenaInstanceCount = [&](uint32 queueType, uint32 bracket, uint32 minCount)
        {
            if (BattlegroundData[queueType][bracket].activeRatedArenaQueue == 0 &&
                BattlegroundData[queueType][bracket].ratedArenaInstanceCount < minCount &&
                BattlegroundData[queueType][bracket].ratedArenaInstances.size() < minCount)
                BattlegroundData[queueType][bracket].activeRatedArenaQueue = 1;
        };

        auto updateBGInstanceCount = [&](uint32 queueType, std::vector<uint32> brackets, uint32 minCount)
        {
            for (uint32 bracket : brackets)
            {
                if (BattlegroundData[queueType][bracket].activeBgQueue == 0 &&
                    BattlegroundData[queueType][bracket].bgInstanceCount < minCount &&
                    BattlegroundData[queueType][bracket].bgInstances.size() < minCount)
                    BattlegroundData[queueType][bracket].activeBgQueue = 1;
            }
        };

        // Update rated arena instance counts
        updateRatedArenaInstanceCount(BATTLEGROUND_QUEUE_2v2, randomBotAutoJoinArenaBracket,
                                      randomBotAutoJoinBGRatedArena2v2Count);
        updateRatedArenaInstanceCount(BATTLEGROUND_QUEUE_3v3, randomBotAutoJoinArenaBracket,
                                      randomBotAutoJoinBGRatedArena3v3Count);
        updateRatedArenaInstanceCount(BATTLEGROUND_QUEUE_5v5, randomBotAutoJoinArenaBracket,
                                      randomBotAutoJoinBGRatedArena5v5Count);

        // Update battleground instance counts
        updateBGInstanceCount(BATTLEGROUND_QUEUE_IC, icBrackets, randomBotAutoJoinBGICCount);
        updateBGInstanceCount(BATTLEGROUND_QUEUE_EY, eyBrackets, randomBotAutoJoinBGEYCount);
        updateBGInstanceCount(BATTLEGROUND_QUEUE_AV, avBrackets, randomBotAutoJoinBGAVCount);
        updateBGInstanceCount(BATTLEGROUND_QUEUE_AB, abBrackets, randomBotAutoJoinBGABCount);
        updateBGInstanceCount(BATTLEGROUND_QUEUE_WS, wsBrackets, randomBotAutoJoinBGWSCount);
    }

    LogBattlegroundInfo();
}

void RandomPlayerbotMgr::LogBattlegroundInfo()
{
    for (auto const& queueTypePair : BattlegroundData)
    {
        uint8 queueType = queueTypePair.first;

        BattlegroundQueueTypeId queueTypeId = BattlegroundQueueTypeId(queueType);

        if (uint8 type = BattlegroundMgr::BGArenaType(queueTypeId))
        {
            for (auto const& bracketIdPair : queueTypePair.second)
            {
                auto& bgInfo = bracketIdPair.second;
                if (bgInfo.minLevel == 0)
                    continue;
                LOG_INFO("playerbots",
                         "竞技场:{} {}: 玩家（练习:{}, 评级:{}）机器人（练习:{}, 评级:{}）总计（练习:{} "
                         "评级:{}），实例（练习:{} 评级:{}）",
                         type == ARENA_TYPE_2v2   ? "2v2"
                         : type == ARENA_TYPE_3v3 ? "3v3"
                                                  : "5v5",
                         std::to_string(bgInfo.minLevel) + "-" + std::to_string(bgInfo.maxLevel),
                         bgInfo.skirmishArenaPlayerCount, bgInfo.ratedArenaPlayerCount, bgInfo.skirmishArenaBotCount,
                         bgInfo.ratedArenaBotCount, bgInfo.skirmishArenaPlayerCount + bgInfo.skirmishArenaBotCount,
                         bgInfo.ratedArenaPlayerCount + bgInfo.ratedArenaBotCount, bgInfo.skirmishArenaInstanceCount,
                         bgInfo.ratedArenaInstanceCount);
            }
            continue;
        }

        BattlegroundTypeId bgTypeId = BattlegroundMgr::BGTemplateId(queueTypeId);
        std::string _bgType;
        switch (bgTypeId)
        {
            case BATTLEGROUND_AV:
                _bgType = "AV";
                break;
            case BATTLEGROUND_WS:
                _bgType = "WSG";
                break;
            case BATTLEGROUND_AB:
                _bgType = "AB";
                break;
            case BATTLEGROUND_EY:
                _bgType = "EotS";
                break;
            case BATTLEGROUND_RB:
                _bgType = "Random";
                break;
            case BATTLEGROUND_SA:
                _bgType = "SotA";
                break;
            case BATTLEGROUND_IC:
                _bgType = "IoC";
                break;
            default:
                _bgType = "Other";
                break;
        }

        for (auto const& bracketIdPair : queueTypePair.second)
        {
            auto& bgInfo = bracketIdPair.second;
            if (bgInfo.minLevel == 0)
                continue;

            LOG_INFO("playerbots",
                     "战场:{} {}: 玩家（联盟:{} 部落:{}）机器人（联盟:{} 部落:{}）总计（联盟:{} 部落:{}），实例 {}，活跃队列: {}", _bgType,
                     std::to_string(bgInfo.minLevel) + "-" + std::to_string(bgInfo.maxLevel),
                     bgInfo.bgAlliancePlayerCount, bgInfo.bgHordePlayerCount, bgInfo.bgAllianceBotCount,
                     bgInfo.bgHordeBotCount, bgInfo.bgAlliancePlayerCount + bgInfo.bgAllianceBotCount,
                     bgInfo.bgHordePlayerCount + bgInfo.bgHordeBotCount, bgInfo.bgInstanceCount, bgInfo.activeBgQueue);
        }
    }
    LOG_DEBUG("playerbots", "战场队列检查完成");
}

// By leewheel 2026-07-10
// LFG超时强制加入辅助函数

// 检查机器人是否空闲可以加入LFG
static bool IsBotIdleForLfg(Player* bot)
{
    if (!bot || !bot->IsInWorld())
        return false;
    if (bot->GetLevel() < 15)
        return false;
    if (bot->InBattleground() || bot->InBattlegroundQueue())
        return false;
    if (bot->isDead())
        return false;
    if (bot->IsBeingTeleported())
        return false;
    if (bot->IsInCombat())
        return false;
    Map* map = bot->GetMap();
    if (map && map->Instanceable())
        return false;
    lfg::LfgState state = sLFGMgr->GetState(bot->GetGUID());
    if (state != lfg::LFG_STATE_NONE)
        return false;
    if (bot->GetGroup() && bot->GetGroup()->GetLeaderGUID() != bot->GetGUID())
        return false;
    // 非真实玩家
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI || IsRealPlayer(bot))
        return false;
    return true;
}

// 检查机器人当前是否为坦克天赋
// By leewheel 2026-07-29
// 修复德鲁伊坦克检测：原代码要求 HasAura(16931) 但此光环仅在熊形态下存在，
// Feral 德鲁伊在非熊形态下永远被识别为 DPS，导致 LFG 强制补位时永远不选 Feral 德鲁伊当坦。
// 根因：Druid 的 Feral 天赋页（spec==1）就是坦克专精，无论熊形态/猫形态/枭兽形态都属 Feral。
// 修复：移除形态/光环检测，仅按天赋页判断，spec==1 即为坦克。GetRoles() 同问题已同步修复。
static bool IsBotTank(Player* bot)
{
    uint8 spec = AiFactory::GetPlayerSpecTab(bot);
    switch (bot->getClass())
    {
        case CLASS_WARRIOR: return spec == 2;
        case CLASS_PALADIN: return spec == 1;
        case CLASS_DRUID: return spec == 1;  // Feral 天赋即坦克专精，形态无关
        case CLASS_DEATH_KNIGHT: return spec == 0;
        default: return false;
    }
}

// 检查机器人当前是否为治疗天赋
static bool IsBotHealer(Player* bot)
{
    uint8 spec = AiFactory::GetPlayerSpecTab(bot);
    switch (bot->getClass())
    {
        case CLASS_PALADIN: return spec == 0;
        case CLASS_DRUID: return spec == 2;
        case CLASS_PRIEST: return spec != 2;
        case CLASS_SHAMAN: return spec == 2;
        default: return false;
    }
}

// 检查职业是否可以当坦克
static bool ClassCanTank(uint8 cls)
{
    return cls == CLASS_WARRIOR || cls == CLASS_PALADIN ||
           cls == CLASS_DRUID || cls == CLASS_DEATH_KNIGHT;
}

// By leewheel 2026-07-29
// 将四个可坦职业映射到稳定的轮转槽位，避免 PlayerBotMap 按GUID固定顺序时长期只选中死亡骑士。
static int32 GetTankClassSlot(uint8 cls)
{
    switch (cls)
    {
        case CLASS_WARRIOR: return 0;
        case CLASS_PALADIN: return 1;
        case CLASS_DRUID: return 2;
        case CLASS_DEATH_KNIGHT: return 3;
        default: return -1;
    }
}

static char const* GetTankClassName(uint8 slot)
{
    switch (slot)
    {
        case 0: return "战士";
        case 1: return "圣骑士";
        case 2: return "德鲁伊";
        case 3: return "死亡骑士";
        default: return "未知职业";
    }
}
// End By leewheel

// 检查职业是否可以当治疗
static bool ClassCanHeal(uint8 cls)
{
    return cls == CLASS_PALADIN || cls == CLASS_DRUID ||
           cls == CLASS_PRIEST || cls == CLASS_SHAMAN;
}

// 获取坦克天赋页索引
static int32 GetTankSpecTab(uint8 cls)
{
    switch (cls)
    {
        case CLASS_WARRIOR: return 2;
        case CLASS_PALADIN: return 1;
        case CLASS_DRUID: return 1;
        case CLASS_DEATH_KNIGHT: return 0;
        default: return -1;
    }
}

// By leewheel 2026-08-11
// 获取坦克职业机器人的坦克专精天赋点数（用于"边缘判定"诊断）
// 背景：GetPlayerSpecTab 只返回天赋点最多的那一页，存在严重误判：
//   战士 30防护/31狂怒 → 判定狂怒(DPS)，本来是潜在坦克被错过
//   战士 25防护/25狂怒/15武器 → 同样判定为非坦克
// 此函数返回该 bot 在坦克专精页上的实际天赋点数，供 ForceBotsJoinLfg 判断是否为"潜在坦克"。
static uint32 GetTankSpecPoints(Player* bot)
{
    int32 const tankTab = GetTankSpecTab(bot->getClass());
    if (tankTab < 0)
        return 0;
    std::map<uint8, uint32> tabs = AiFactory::GetPlayerSpecTabs(bot);
    return tabs[tankTab];
}

// 判断坦克职业 bot 是否为"潜在坦克"——坦克专精天赋点数≥20但当前未被 IsBotTank 判定为坦克
// 用于修复 GetPlayerSpecTab 边缘误判导致的坦克缺位问题
static bool IsPotentialTank(Player* bot)
{
    if (!ClassCanTank(bot->getClass()))
        return false;
    if (IsBotTank(bot))
        return false;
    return GetTankSpecPoints(bot) >= 20;
}
// End By leewheel

// 获取治疗天赋页索引
static int32 GetHealerSpecTab(uint8 cls)
{
    switch (cls)
    {
        case CLASS_PALADIN: return 0;
        case CLASS_DRUID: return 2;
        case CLASS_PRIEST: return 1;
        case CLASS_SHAMAN: return 2;
        default: return -1;
    }
}

// 发送LFG加入数据包
// By leewheel 2026-07-29
// 返回机器人是否确实进入了LFG队列，调用方只能在成功后扣减职责缺额。
static bool SendLfgJoinPacket(Player* bot, const lfg::LfgDungeonSet& dungeons, uint8 role)
// End By leewheel
{
    // 按等级过滤副本
    lfg::LfgDungeonSet validDungeons;
    for (uint32 dungeonId : dungeons)
    {
        LFGDungeonEntry const* dungeon = sLFGDungeonStore.LookupEntry(dungeonId);
        if (!dungeon)
            continue;
        if (dungeon->TypeID != lfg::LFG_TYPE_RANDOM && dungeon->TypeID != lfg::LFG_TYPE_DUNGEON &&
            dungeon->TypeID != lfg::LFG_TYPE_HEROIC && dungeon->TypeID != lfg::LFG_TYPE_RAID)
            continue;
        auto const& botLevel = bot->GetLevel();
        if ((dungeon->MinLevel && (botLevel < dungeon->MinLevel || botLevel > dungeon->MaxLevel)) ||
            (botLevel > dungeon->MinLevel + 10 && dungeon->TypeID == lfg::LFG_TYPE_DUNGEON))
            continue;
        validDungeons.insert(dungeonId);
    }

    if (validDungeons.empty())
        return false;

    // By leewheel 2026-07-29
    // 改回直接调用 sLFGMgr->JoinLfg（参考 LiyunfanPlayerbotsBranch 的稳定实现）。
    // 根因：之前用 QueuePacket(CMSG_LFG_JOIN) 时，bot 入队后 state 立刻被清回 NONE，
    //       导致下次补位仍认为它 idle 并再次 join，循环几十次始终不能进 QUEUED 状态。
    //       直接调 LFGMgr::JoinLfg 是该函数被真实玩家 CMSG_LFG_JOIN handler 内部调用的同一段，
    //       state 设置可靠，能正常进入 LFG_STATE_QUEUED 走撮合流程。
    // By leewheel 2026-08-01
    // 周期性卡顿修复：诊断日志降级为 DEBUG，消除 30 秒周期路径上的日志 I/O 尖峰
    LOG_DEBUG("playerbots", "[LFG诊断] bot {} 准备JoinLfg role={} dungeons={} 当前state={}",
        bot->GetName().c_str(), (uint32)role, (uint32)validDungeons.size(),
        (uint32)sLFGMgr->GetState(bot->GetGUID()));
    sLFGMgr->JoinLfg(bot, role, validDungeons, std::to_string(GET_PLAYERBOT_AI(bot)->GetEquipGearScore(bot)));
    lfg::LfgState const stateAfterJoin = sLFGMgr->GetState(bot->GetGUID());
    LOG_DEBUG("playerbots", "[LFG诊断] bot {} JoinLfg后state={}",
        bot->GetName().c_str(), (uint32)stateAfterJoin);

    // By leewheel 2026-07-29
    // 只有 QUEUED/PROPOSAL 才表示真实加入成功，不能再把 state=NONE 的失败尝试计为已补位。
    // By leewheel 2026-08-01
    // 周期性卡顿修复：入队成功后记录入队时间，供 CheckLfgQueue 滞留超时清理使用
    if (stateAfterJoin == lfg::LFG_STATE_QUEUED)
        sRandomPlayerbotMgr.RecordBotLfgJoinTime(bot->GetGUID());
    // End By leewheel
    return stateAfterJoin == lfg::LFG_STATE_QUEUED || stateAfterJoin == lfg::LFG_STATE_PROPOSAL;
    // End By leewheel
}
// End By leewheel

void RandomPlayerbotMgr::CheckLfgQueue()
{
    if (!LfgCheckTimer || time(nullptr) > (LfgCheckTimer + 30))
        LfgCheckTimer = time(nullptr);

    LOG_DEBUG("playerbots", "正在检查 LFG 队列...");

    // Clear LFG list
    LfgDungeons[TEAM_ALLIANCE].clear();
    LfgDungeons[TEAM_HORDE].clear();

    // By leewheel 2026-07-10
    // 追踪真实玩家LFG排队时间
    bool teamHasQueuedPlayer[2] = {false, false};
    // End By leewheel

    // By leewheel 2026-07-29
    // 诊断日志：显示 players 向量大小，定位 teamHasQueuedPlayer 始终为 false 的根因
    // By leewheel 2026-08-01
    // 周期性卡顿修复：诊断日志降级为 DEBUG，消除每 30 秒一次的世界线程日志 I/O 尖峰
    LOG_DEBUG("playerbots", "[LFG诊断] CheckLfgQueue开始: players向量大小={}", players.size());
    // End By leewheel

    for (std::vector<Player*>::iterator i = players.begin(); i != players.end(); ++i)
    {
        Player* player = *i;
        if (!player || !player->IsInWorld())
            continue;

        Group* group = player->GetGroup();
        ObjectGuid guid = group ? group->GetGUID() : player->GetGUID();

        lfg::LfgState gState = sLFGMgr->GetState(guid);

        // By leewheel 2026-07-29
        // 双重GUID检查：同时检查组GUID和玩家自身GUID的LFG状态
        // 根因：AzerothCore LFG系统中，solo玩家排队时状态存储在玩家GUID下，
        //       但某些情况下（如先组队后排队、离开队伍后状态残留）
        //       组GUID和玩家GUID的状态可能不一致，需要双重检查
        lfg::LfgState pState = sLFGMgr->GetState(player->GetGUID());
        // By leewheel 2026-08-01
        // 周期性卡顿修复：逐玩家诊断日志降级为 DEBUG，消除每 30 秒一次的日志 I/O 尖峰
        LOG_DEBUG("playerbots", "[LFG诊断] 玩家 {} 组GUID状态={} 玩家GUID状态={} 在组={}",
                 player->GetName().c_str(), (uint32)gState, (uint32)pState, group ? "是" : "否");
        // End By leewheel

        if ((gState != lfg::LFG_STATE_NONE && gState < lfg::LFG_STATE_DUNGEON) ||
            (pState != lfg::LFG_STATE_NONE && pState < lfg::LFG_STATE_DUNGEON))
        // End By leewheel
        {
            // By leewheel 2026-07-10
            teamHasQueuedPlayer[player->GetTeamId()] = true;
            // End By leewheel

            lfg::LfgDungeonSet const& dList = sLFGMgr->GetSelectedDungeons(player->GetGUID());
            for (lfg::LfgDungeonSet::const_iterator itr = dList.begin(); itr != dList.end(); ++itr)
            {
                lfg::LFGDungeonData const* dungeon = sLFGMgr->GetLFGDungeon(*itr);
                if (!dungeon)
                    continue;

                LfgDungeons[player->GetTeamId()].push_back(dungeon->id);
            }
        }
    }

    // By leewheel 2026-07-29
    // 全服扫描兜底：如果 players 向量为空或未检测到排队玩家，
    // 扫描所有在线玩家检查LFG状态
    // 根因：players 向量可能因 OnPlayerLogin 未被调用而遗漏真实玩家
    // By leewheel 2026-08-01
    // 周期性卡顿修复：全服扫描兜底限频为 5 分钟一次（原来每 30 秒一次），
    // 且所有诊断日志降级为 DEBUG——世界线程每 30 秒全服遍历 + 逐人日志 I/O 是 30 秒周期卡顿的直接来源。
    // By leewheel 2026-08-11
    // 修复坦克不进组：全服扫描节流从 5 分钟改为 1 分钟。
    // 根因：players 向量检测不到真实玩家排队时，5 分钟内 teamHasQueuedPlayer 保持 false，
    //       lfgQueueStartTime 被反复重置为 0，ForceBotsJoinLfg 永远不触发；
    //       且 else 分支会立即清掉所有已入队的单人 bot，坦克无法保持入队状态。
    //       改为 60 秒后，最多 2 轮 CheckLfgQueue（60秒）即可检测到真实玩家排队。
    if (!teamHasQueuedPlayer[TEAM_ALLIANCE] && !teamHasQueuedPlayer[TEAM_HORDE] &&
        time(nullptr) > (FullScanTimer + 60))
    {
        FullScanTimer = time(nullptr);
        LOG_DEBUG("playerbots", "[LFG诊断] players向量未检测到排队玩家，启动全服扫描...");
        sWorldSessionMgr->DoForAllOnlinePlayers([&](Player* player)
        {
            if (!player || !player->IsInWorld())
                return;
            // 跳过随机机器人
            if (IsRandomBot(player))
                return;

            Group* group = player->GetGroup();
            ObjectGuid guid = group ? group->GetGUID() : player->GetGUID();
            lfg::LfgState gState = sLFGMgr->GetState(guid);
            lfg::LfgState pState = sLFGMgr->GetState(player->GetGUID());

            LOG_DEBUG("playerbots", "[LFG诊断] 全服扫描: 玩家 {} 组GUID状态={} 玩家GUID状态={}",
                     player->GetName().c_str(), (uint32)gState, (uint32)pState);

            if ((gState != lfg::LFG_STATE_NONE && gState < lfg::LFG_STATE_DUNGEON) ||
                (pState != lfg::LFG_STATE_NONE && pState < lfg::LFG_STATE_DUNGEON))
            {
                teamHasQueuedPlayer[player->GetTeamId()] = true;

                lfg::LfgDungeonSet const& dList = sLFGMgr->GetSelectedDungeons(player->GetGUID());
                for (lfg::LfgDungeonSet::const_iterator itr = dList.begin(); itr != dList.end(); ++itr)
                {
                    lfg::LFGDungeonData const* dungeon = sLFGMgr->GetLFGDungeon(*itr);
                    if (!dungeon)
                        continue;
                    LfgDungeons[player->GetTeamId()].push_back(dungeon->id);
                }

                LOG_DEBUG("playerbots", "[LFG诊断] 全服扫描发现排队玩家: {} 阵营={}",
                         player->GetName().c_str(), player->GetTeamId() == TEAM_ALLIANCE ? "联盟" : "部落");
            }
        });
    }

    LOG_DEBUG("playerbots", "[LFG诊断] 检查结果: 联盟排队={} 部落排队={}",
             teamHasQueuedPlayer[TEAM_ALLIANCE], teamHasQueuedPlayer[TEAM_HORDE]);
    // End By leewheel

    // By leewheel 2026-07-21
    // LFG排队超时强制机器人加入机制
    // 阈值从2/3改为1/6(默认180秒→30秒)，大幅缩短真实玩家等待时间
    uint32 forceThreshold = sPlayerbotAIConfig.randomBotLfgMaxQueueWaitTime / 6;
    if (forceThreshold < 15)
        forceThreshold = 15;  // 最低15秒
    // By leewheel 2026-07-29
    // 进一步缩短阈值：从 1/6 改为 1/8（约 22.5 秒），让真实玩家更早看到坦克 bot 入队。
    // 配合 "lfg role priority" 触发器，坦克 bot 主动入队 + 强制补位双重保险。
    uint32 forceThresholdFast = sPlayerbotAIConfig.randomBotLfgMaxQueueWaitTime / 8;
    if (forceThresholdFast < 10)
        forceThresholdFast = 10;  // 最低10秒
    uint32 const effectiveThreshold = forceThresholdFast < forceThreshold ? forceThresholdFast : forceThreshold;
    // End By leewheel
    for (int teamIdx = TEAM_ALLIANCE; teamIdx <= TEAM_HORDE; ++teamIdx)
    {
        TeamId teamId = (TeamId)teamIdx;

        // By leewheel 2026-08-01
        // 周期性卡顿修复：每轮刷新该阵营 LFG 队列中的角色计数（0坦/1奶/2DPS），
        // 供 LfgRolePriorityTrigger 做队列饱和度判断，避免坦克/治疗机器人无限自主排队
        std::array<uint32, 3> roleCount = {0, 0, 0};
        for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
        {
            Player* bot = it->second;
            if (!bot || bot->GetTeamId() != teamId || !IsRandomBot(bot))
                continue;
            lfg::LfgState state = sLFGMgr->GetState(bot->GetGUID());
            if (state == lfg::LFG_STATE_NONE || state >= lfg::LFG_STATE_DUNGEON)
                continue;
            uint8 roles = sLFGMgr->GetRoles(bot->GetGUID());
            if (roles & lfg::PLAYER_ROLE_TANK)
                roleCount[0]++;
            else if (roles & lfg::PLAYER_ROLE_HEALER)
                roleCount[1]++;
            else
                roleCount[2]++;
        }
        lfgQueueRoleCount[teamId] = roleCount;
        // End By leewheel

        if (teamHasQueuedPlayer[teamIdx])
        {
            // 记录排队开始时间（仅首次）
            if (lfgQueueStartTime[teamId] == 0)
                lfgQueueStartTime[teamId] = time(nullptr);

            uint32 waitTime = (uint32)(time(nullptr) - lfgQueueStartTime[teamId]);
            if (waitTime >= effectiveThreshold)
            {
                LOG_INFO("playerbots", "LFG排队超时强制机器人加入: 阵营={}, 等待时间={}秒, 阈值={}秒",
                         teamId == TEAM_ALLIANCE ? "联盟" : "部落", waitTime, effectiveThreshold);
                ForceBotsJoinLfg(teamId);
            }

            // By leewheel 2026-08-01
            // 周期性卡顿修复：有真实玩家排队期间同样清理滞留机器人——
            // 补位入队超过 randomBotLfgMaxQueueWaitTime 仍未撮合成功的单人 bot 强制离队，
            // 防止 8 秒撮合(UpdateQueueTimers)持续承受膨胀队列带来的周期性尖峰。
            for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
            {
                Player* bot = it->second;
                if (!bot || bot->GetTeamId() != teamId || !IsRandomBot(bot))
                    continue;
                // 跟真实玩家组队排队的 bot 状态挂在组 GUID 下，这里只处理单人排队的 bot
                if (bot->GetGroup())
                    continue;
                if (sLFGMgr->GetState(bot->GetGUID()) != lfg::LFG_STATE_QUEUED)
                    continue;
                time_t const joinTime = GetBotLfgJoinTime(bot->GetGUID());
                if (!joinTime)
                    continue;
                if (time(nullptr) - joinTime < sPlayerbotAIConfig.randomBotLfgMaxQueueWaitTime)
                    continue;
                sLFGMgr->LeaveLfg(bot->GetGUID());
                ClearBotLfgJoinTime(bot->GetGUID());
            }
            // End By leewheel
        }
        else
        {
            // By leewheel 2026-08-11
            // 修复坦克不进组：teamHasQueuedPlayer = false 时不再立即重置 lfgQueueStartTime 和清理所有单人 QUEUED bot。
            //
            // 根因（玩家反馈"超过10分钟没有坦克进组"）：
            //   teamHasQueuedPlayer 在 true/false 之间频繁切换时（players 向量中玩家短暂 IsInWorld()=false、
            //   全服扫描节流期间检测不到等），原逻辑每轮 false 都会：
            //     1. lfgQueueStartTime = 0 → waitTime 永远达不到 effectiveThreshold(22.5秒)，ForceBotsJoinLfg 永远不触发
            //     2. 立即清掉所有单人 QUEUED bot → 坦克 bot 通过 LfgRolePriorityTrigger 自主入队后 30 秒就被清掉，
            //        无法在队列中保持足够时间让 LFGQueue::UpdateQueueTimers(8秒周期)撮合成功
            //   两者叠加导致坦克 bot 在入队/离队之间循环，玩家端看到"超过10分钟没有坦克进组"。
            //
            // 修复策略（给宽限期，不立即清）：
            //   1. lfgQueueStartTime 保留 90 秒不重置——下一轮 teamHasQueuedPlayer=true 时 waitTime 可继续累计
            //   2. 已入队的单人 bot 只清理超时的（randomBotLfgMaxQueueWaitTime=180秒），不立即全部清掉
            //   3. 90 秒后仍无真实玩家排队，才真正重置 lfgQueueStartTime 并走原来的清理逻辑
            bool const queueExpired = (lfgQueueStartTime[teamId] != 0 &&
                                       time(nullptr) - lfgQueueStartTime[teamId] > 90);
            if (queueExpired)
                lfgQueueStartTime[teamId] = 0;
            // End By leewheel

            // By leewheel 2026-07-30
            // 清理滞留队列的机器人：真实玩家离开队列(或已进本)后，此前补位/自主排队的
            // 机器人会以 QUEUED 状态滞留在 LFG 队列中（ProcessBot 对 LFG 状态 bot 跳过
            // randomize/teleport，"seldom" 离队触发器要数分钟才轮到一次）。
            // LFGQueue::UpdateQueueTimers 每 8 秒(LFG_QUEUEUPDATE_INTERVAL)对全队列做
            // 撮合与状态推送，队列里滞留的 bot 越多，世界线程每 8 秒的尖峰越大，
            // 表现为玩家端每隔 7~8 秒规律性卡顿。没有真实玩家排队时 bot 留在队列毫无意义，
            // 这里立即让本阵营所有单人 QUEUED 状态的随机机器人离开队列。
            // By leewheel 2026-08-11
            // 修复坦克不进组：宽限期(90秒)内不清理单人 QUEUED bot，只清理超时的。
            // 宽限期过后(queueExpired=true)走原来的全量清理逻辑。
            for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
            {
                Player* bot = it->second;
                if (!bot || bot->GetTeamId() != teamId || !IsRandomBot(bot))
                    continue;
                // 跟真实玩家组队排队的 bot 状态挂在组 GUID 下，这里只处理单人排队的 bot
                if (bot->GetGroup())
                    continue;
                if (sLFGMgr->GetState(bot->GetGUID()) != lfg::LFG_STATE_QUEUED)
                    continue;
                // By leewheel 2026-08-11
                // 宽限期内只清理超时 bot，不立即全部清掉
                if (!queueExpired)
                {
                    time_t const joinTime = GetBotLfgJoinTime(bot->GetGUID());
                    if (!joinTime)
                        continue;
                    if (time(nullptr) - joinTime < sPlayerbotAIConfig.randomBotLfgMaxQueueWaitTime)
                        continue;
                }
                // End By leewheel
                sLFGMgr->LeaveLfg(bot->GetGUID());
                // By leewheel 2026-08-01
                // 同步清除入队时间记录，防止 lfgBotJoinTime 无限增长
                ClearBotLfgJoinTime(bot->GetGUID());
                // End By leewheel
            }
            // End By leewheel
        }
    }
    // End By leewheel

    LOG_DEBUG("playerbots", "LFG 队列检查完成");
}

// By leewheel 2026-07-21
// 强制机器人加入LFG队列（补位模式）
// 当真实玩家排队超时后，检查队列中各角色bot数量，按缺额补充：
// 目标: 2坦 + 2奶 + 3DPS (真实玩家+bot共5人即可成团，多出的bot作为候补)
// 1. 先统计队列中已有的bot角色(用sLFGMgr->GetRoles读排队角色)
// 2. 优先找对应天赋的空闲bot直接加入
// 3. 如果找不到，找符合职业条件的bot切换天赋后加入
// 4. 每次CheckLfgQueue(30秒)都会重新补位，掉线的bot自动被替换
void RandomPlayerbotMgr::ForceBotsJoinLfg(TeamId teamId)
{
    // 获取真实玩家排队的副本列表
    std::vector<uint32>& dungeonVec = LfgDungeons[teamId];
    if (dungeonVec.empty())
        return;

    // 构建LFG副本集合
    lfg::LfgDungeonSet dungeonSet;
    for (uint32 dungeonId : dungeonVec)
    {
        LFGDungeonEntry const* dungeon = sLFGDungeonStore.LookupEntry(dungeonId);
        if (!dungeon)
            continue;
        dungeonSet.insert(dungeon->ID);
    }
    if (dungeonSet.empty())
        return;

    // 目标：队列中保持的bot角色数量（不含真实玩家）
    const int TARGET_TANKS = 2;
    const int TARGET_HEALERS = 2;
    const int TARGET_DPS = 3;

    // By leewheel 2026-08-01
    // 周期性卡顿修复：单次遍历同时完成三件事（原实现需 5 次全 bot 遍历）：
    //   1. 统计已在队列中的 bot 角色数量（计算缺额）
    //   2. 收集空闲可用的候选 bot（后续轮转/补位直接在候选列表上操作）
    //   3. 统计空闲 bot 职业构成（诊断用，已降级为 DEBUG 日志）
    // 目标：消除 ForceBotsJoinLfg 内 O(12n) 级别的重复全遍历，降低世界线程尖峰。
    struct LfgBotCandidate
    {
        Player* bot;
        uint8 cls;
        int32 tankClassSlot;
        bool used = false;
    };
    std::vector<LfgBotCandidate> candidates;
    int tanksInQueue = 0, healersInQueue = 0, dpsInQueue = 0;
    int idleTanks = 0, idleHealers = 0, idleDps = 0, idleTotal = 0;
    std::array<int, 4> idleTankByClass = {};
    std::array<int, 4> switchableNonHealerTankByClass = {};
    std::array<int, 4> switchableHealerTankByClass = {};
    // By leewheel 2026-08-11
    // 潜在坦克误判统计：坦克专精天赋点数≥20但 GetPlayerSpecTab 误判为非坦克的 bot 数量
    // 用于诊断"明明是坦克天赋的机器人自认为DPS"问题
    std::array<int, 4> potentialTanksByClass = {};
    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* bot = it->second;
        if (!bot || bot->GetTeamId() != teamId)
            continue;
        bool const isRandom = IsRandomBot(bot);
        // 统计已在队列中的 bot 角色数量（用于计算缺额，与 bot 是否空闲无关）
        if (isRandom)
        {
            lfg::LfgState state = sLFGMgr->GetState(bot->GetGUID());
            if (state != lfg::LFG_STATE_NONE && state < lfg::LFG_STATE_DUNGEON)
            {
                uint8 roles = sLFGMgr->GetRoles(bot->GetGUID());
                if (roles & lfg::PLAYER_ROLE_TANK)
                    tanksInQueue++;
                else if (roles & lfg::PLAYER_ROLE_HEALER)
                    healersInQueue++;
                else
                    dpsInQueue++;
            }
        }
        // 收集空闲候选 bot，并顺带完成诊断统计
        if (!isRandom || !IsBotIdleForLfg(bot))
            continue;
        bool const isTank = IsBotTank(bot);
        bool const isHealer = IsBotHealer(bot);
        int32 const tankClassSlot = GetTankClassSlot(bot->getClass());
        candidates.push_back({bot, bot->getClass(), tankClassSlot, false});
        idleTotal++;
        if (isTank)
        {
            idleTanks++;
            if (tankClassSlot >= 0)
                idleTankByClass[tankClassSlot]++;
        }
        else
        {
            // 区分“非治疗可切坦”和“治疗可切坦”，避免圣骑士/德鲁伊治疗被轮转提前改坦导致奶缺额。
            // 治疗类职业必须且仅能在治疗已经满编、坦克仍缺的情况下才允许转坦。
            if (tankClassSlot >= 0)
            {
                if (isHealer)
                    switchableHealerTankByClass[tankClassSlot]++;
                else
                    switchableNonHealerTankByClass[tankClassSlot]++;
                // By leewheel 2026-08-11
                // 潜在坦克检测：坦克职业 bot 当前不是坦克天赋，但坦克专精天赋点数≥20
                // 说明 GetPlayerSpecTab 因天赋点分散而误判，该 bot 实际上是潜在坦克
                if (GetTankSpecPoints(bot) >= 20)
                    potentialTanksByClass[tankClassSlot]++;
                // End By leewheel
            }
            if (isHealer)
                idleHealers++;
            else
                idleDps++;
        }
    }
    // End By leewheel

    // 计算各角色缺额
    int needTanks = TARGET_TANKS - tanksInQueue;
    int needHealers = TARGET_HEALERS - healersInQueue;
    int needDps = TARGET_DPS - dpsInQueue;

    if (needTanks <= 0 && needHealers <= 0 && needDps <= 0)
    {
        LOG_DEBUG("playerbots", "LFG队列bot已满编: 坦{} 奶{} DPS{} (阵营={})",
                  tanksInQueue, healersInQueue, dpsInQueue,
                  teamId == TEAM_ALLIANCE ? "联盟" : "部落");
        return;
    }

    // By leewheel 2026-08-01
    // 周期性卡顿修复：补位/诊断日志全部降级为 DEBUG，消除世界线程日志 I/O 尖峰
    // By leewheel 2026-08-11
    // 玩家反馈"随机本坦克10分钟不进组"，根因之一是诊断日志全部 DEBUG，运维看不到任何信息
    // 升级关键诊断日志为 INFO，让运维能定位是"无坦克职业bot在线"还是"天赋误判"
    LOG_INFO("playerbots", "LFG补位开始: 队列中坦{}奶{}DPS{}, 需补坦{}奶{}DPS{} (阵营={})",
             tanksInQueue, healersInQueue, dpsInQueue,
             needTanks > 0 ? needTanks : 0, needHealers > 0 ? needHealers : 0, needDps > 0 ? needDps : 0,
             teamId == TEAM_ALLIANCE ? "联盟" : "部落");
    LOG_INFO("playerbots", "[LFG诊断] 空闲bot统计: 总计={} 坦克={} 治疗={} DPS={} (阵营={})",
             idleTotal, idleTanks, idleHealers, idleDps,
             teamId == TEAM_ALLIANCE ? "联盟" : "部落");
    LOG_INFO("playerbots", "[LFG诊断] 坦克职业候选: 战士(现成{} 非治疗可切{} 治疗可切{}) 圣骑士(现成{} 非治疗可切{} 治疗可切{}) 德鲁伊(现成{} 非治疗可切{} 治疗可切{}) 死亡骑士(现成{} 非治疗可切{} 治疗可切{})",
             idleTankByClass[0], switchableNonHealerTankByClass[0], switchableHealerTankByClass[0],
             idleTankByClass[1], switchableNonHealerTankByClass[1], switchableHealerTankByClass[1],
             idleTankByClass[2], switchableNonHealerTankByClass[2], switchableHealerTankByClass[2],
             idleTankByClass[3], switchableNonHealerTankByClass[3], switchableHealerTankByClass[3]);

    // By leewheel 2026-08-11
    // 无坦克职业bot在线告警：当整个阵营没有战士/圣骑/德鲁伊/DK的空闲bot时，明确告知运维
    // 这是玩家等不到坦克的根本原因之一——服务器根本没有坦克职业的机器人可调度
    if (needTanks > 0 &&
        idleTankByClass[0] == 0 && idleTankByClass[1] == 0 && idleTankByClass[2] == 0 && idleTankByClass[3] == 0 &&
        switchableNonHealerTankByClass[0] == 0 && switchableNonHealerTankByClass[1] == 0 &&
        switchableNonHealerTankByClass[2] == 0 && switchableNonHealerTankByClass[3] == 0 &&
        switchableHealerTankByClass[0] == 0 && switchableHealerTankByClass[1] == 0 &&
        switchableHealerTankByClass[2] == 0 && switchableHealerTankByClass[3] == 0)
    {
        LOG_ERROR("playerbots", "[LFG坦克告警] 阵营{} 缺坦克{}个, 但无任何坦克职业bot在线(战士/圣骑/德鲁伊/DK均无空闲候选), 玩家将一直等不到坦克! 请检查randomBotAccounts是否包含坦克职业账号或增加坦克职业bot数量",
            teamId == TEAM_ALLIANCE ? "联盟" : "部落", needTanks);
    }
    // End By leewheel

    // By leewheel 2026-08-11
    // 潜在坦克误判告警：坦克职业bot有≥20点坦克专精天赋，但 GetPlayerSpecTab 因天赋点分散误判为DPS
    // 这些bot会在phase1被强制切坦，但如果是"30防护/31狂怒"这种边缘情况，切换是浪费的
    // 告警让运维知道存在天赋点分配不合理的bot，可以从源头修复（调整randomClassSpecIndex配置）
    if (needTanks > 0 &&
        (potentialTanksByClass[0] > 0 || potentialTanksByClass[1] > 0 ||
         potentialTanksByClass[2] > 0 || potentialTanksByClass[3] > 0))
    {
        LOG_WARN("playerbots", "[LFG坦克告警] 阵营{} 存在潜在坦克被误判为DPS: 战士{} 圣骑{} 德鲁伊{} DK{} (坦克专精天赋≥20但GetPlayerSpecTab判定为非坦克, 将在phase1强制切坦)",
            teamId == TEAM_ALLIANCE ? "联盟" : "部落",
            potentialTanksByClass[0], potentialTanksByClass[1],
            potentialTanksByClass[2], potentialTanksByClass[3]);
    }
    // End By leewheel
    // End By leewheel

    // By leewheel 2026-07-29
    // 坦克职业轮转：PlayerBotMap 按GUID固定顺序遍历时，鲜血死亡骑士会长期占据最前面的坦克位，
    // 战士、圣骑士、德鲁伊即使存在，也可能永远等不到第二遍切换天赋。这里先按四职业轮转补坦：
    // phase0：各职业现成坦克天赋；phase1：非治疗可切坦；phase2：治疗可切坦（必须且仅能在治疗满编时启用）。
    // 治疗天赋绝不能早于治疗满编之前被轮转改成坦克，否则会把治疗缺额推到玩家身上。
    // 四职业轮转仍不足时才回落到通用扫描，兼顾职业公平、治疗职责保护和补位成功率。
    // By leewheel 2026-08-01
    // 周期性卡顿修复：改为在预筛候选列表上操作，消除每 phase×classSlot 一次的全 bot 遍历
    static std::array<uint8, 2> nextTankClassSlot = {0, 0};
    uint8 const teamSlot = teamId == TEAM_HORDE ? 1 : 0;
    uint8 const rotationStart = nextTankClassSlot[teamSlot];
    std::array<bool, 4> selectedTankClass = {};

    // phase 描述：0=现成坦克天赋，1=非治疗可切坦，2=治疗可切坦
    for (uint8 phase = 0; phase < 3 && needTanks > 0; ++phase)
    {
        // 治疗被改成坦克之前必须保证治疗已经满编，否则会把治疗缺额转嫁到真实玩家身上。
        if (phase == 2 && needHealers > 0)
            break;

        bool const requireCurrentTankSpec = phase == 0;
        bool const allowHealerReassign = phase == 2;
        for (uint8 offset = 0; offset < 4 && needTanks > 0; ++offset)
        {
            uint8 const classSlot = (rotationStart + offset) % 4;
            if (selectedTankClass[classSlot])
                continue;

            for (LfgBotCandidate& cand : candidates)
            {
                if (cand.used)
                    continue;
                if (cand.tankClassSlot != (int32)classSlot)
                    continue;
                bool const isHealer = IsBotHealer(cand.bot);
                if (requireCurrentTankSpec)
                {
                    if (!IsBotTank(cand.bot))
                        continue;
                }
                else
                {
                    if (IsBotTank(cand.bot))
                        continue;
                    // phase1 严禁挑选治疗天赋机器人；phase2 仅挑选治疗天赋机器人
                    if (allowHealerReassign != isHealer)
                        continue;
                }

                if (!requireCurrentTankSpec)
                {
                    int32 const specTab = GetTankSpecTab(cand.cls);
                    if (specTab < 0)
                        continue;

                    // By leewheel 2026-08-01
                    // 周期性卡顿修复：天赋切换节流——同一 bot 30 秒内不重复切换天赋，
                    // 避免 CheckLfgQueue 每 30 秒刷新缺额时反复重置同一批 bot 的天赋
                    // （InitTalentsBySpecNo + InitTalentsTree 是重操作：技能/属性重算、大量数据包）。
                    if (sRandomPlayerbotMgr.IsSpecSwitchThrottled(cand.bot->GetGUID()))
                        continue;
                    sRandomPlayerbotMgr.RecordSpecSwitchTime(cand.bot->GetGUID());
                    // End By leewheel

                    uint32 const specIndex = sPlayerbotAIConfig.randomClassSpecIndex[cand.cls][specTab];
                    PlayerbotFactory::InitTalentsBySpecNo(cand.bot, specIndex, true);
                    if (cand.bot->GetFreeTalentPoints() > 0)
                    {
                        PlayerbotFactory factory(cand.bot, cand.bot->GetLevel());
                        factory.InitTalentsTree(true, false, false);
                    }
                    // 天赋模板可能缺失或应用失败，切换后必须重新按实际天赋页验证，禁止把非坦克天赋机器人以坦克职责送入LFG。
                    if (!IsBotTank(cand.bot))
                    {
                        LOG_WARN("playerbots", "LFG坦克职业轮转失败: {}({}) 切换后仍不是坦克天赋，继续尝试该职业其他机器人",
                            cand.bot->GetName().c_str(), GetTankClassName(classSlot));
                        continue;
                    }
                }

                if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(cand.bot))
                    botAI->ResetStrategies(false);

                if (SendLfgJoinPacket(cand.bot, dungeonSet, lfg::PLAYER_ROLE_TANK))
                {
                    needTanks--;
                    cand.used = true;
                    selectedTankClass[classSlot] = true;
                    nextTankClassSlot[teamSlot] = (classSlot + 1) % 4;
                    char const* phaseDesc = "以现有坦克天赋";
                    if (phase == 1) phaseDesc = "切换非治疗坦克天赋后";
                    else if (phase == 2) phaseDesc = "切换治疗坦克天赋后";
                    LOG_DEBUG("playerbots", "LFG坦克职业轮转: {}({}) {}成功加入队列",
                        cand.bot->GetName().c_str(), GetTankClassName(classSlot), phaseDesc);
                    break;
                }

                LOG_WARN("playerbots", "LFG坦克职业轮转失败: {}({}) 未进入QUEUED/PROPOSAL，继续尝试该职业其他机器人",
                    cand.bot->GetName().c_str(), GetTankClassName(classSlot));
            }
        }
    }
    // End By leewheel

    // 第一遍：找对应天赋的空闲bot直接加入（职业轮转不足时的兜底）
    // By leewheel 2026-08-01
    // 周期性卡顿修复：改为在预筛候选列表上操作，消除一次全 bot 遍历；
    // 候选可能被坦克轮转段切换过天赋，因此兜底阶段一律按当前实际天赋重新判断
    for (LfgBotCandidate& cand : candidates)
    {
        if (cand.used)
            continue;

        Player* bot = cand.bot;

        // 需要坦克且bot是坦克天赋
        if (needTanks > 0 && IsBotTank(bot))
        {
            // By leewheel 2026-07-29
            if (SendLfgJoinPacket(bot, dungeonSet, lfg::PLAYER_ROLE_TANK))
            {
                needTanks--;
                cand.used = true;
                LOG_DEBUG("playerbots", "LFG补位: {} 以坦克身份成功加入队列", bot->GetName().c_str());
                if (needTanks <= 0 && needHealers <= 0 && needDps <= 0)
                    break;
            }
            else
            {
                LOG_WARN("playerbots", "LFG补位失败: {} 以坦克身份加入后未进入QUEUED/PROPOSAL，继续尝试其他机器人", bot->GetName().c_str());
            }
            // End By leewheel
            continue;
        }

        // 需要治疗且bot是治疗天赋
        if (needHealers > 0 && IsBotHealer(bot))
        {
            // By leewheel 2026-07-29
            if (SendLfgJoinPacket(bot, dungeonSet, lfg::PLAYER_ROLE_HEALER))
            {
                needHealers--;
                cand.used = true;
                LOG_DEBUG("playerbots", "LFG补位: {} 以治疗身份成功加入队列", bot->GetName().c_str());
                if (needTanks <= 0 && needHealers <= 0 && needDps <= 0)
                    break;
            }
            else
            {
                LOG_WARN("playerbots", "LFG补位失败: {} 以治疗身份加入后未进入QUEUED/PROPOSAL，继续尝试其他机器人", bot->GetName().c_str());
            }
            // End By leewheel
            continue;
        }

        // 需要DPS
        if (needDps > 0)
        {
            // By leewheel 2026-07-29
            if (SendLfgJoinPacket(bot, dungeonSet, lfg::PLAYER_ROLE_DAMAGE))
            {
                needDps--;
                cand.used = true;
                LOG_DEBUG("playerbots", "LFG补位: {} 以DPS身份成功加入队列", bot->GetName().c_str());
                if (needTanks <= 0 && needHealers <= 0 && needDps <= 0)
                    break;
            }
            else
            {
                LOG_WARN("playerbots", "LFG补位失败: {} 以DPS身份加入后未进入QUEUED/PROPOSAL，继续尝试其他机器人", bot->GetName().c_str());
            }
            // End By leewheel
            continue;
        }
    }
    // End By leewheel

    // 第二遍：如果还缺坦克/治疗，切换天赋
    // By leewheel 2026-08-01
    // 周期性卡顿修复：改为在预筛候选列表上操作，消除一次全 bot 遍历；
    // 候选可能被坦克轮转段切换过天赋，因此此处一律按当前实际天赋重新判断
    if (needTanks > 0 || needHealers > 0)
    {
        for (LfgBotCandidate& cand : candidates)
        {
            if (cand.used)
                continue;

            Player* bot = cand.bot;
            uint8 cls = cand.cls;

            // 需要坦克，bot可以当坦克但当前不是坦克天赋
            if (needTanks > 0 && ClassCanTank(cls) && !IsBotTank(bot))
            {
                // By leewheel 2026-07-29
                // 治疗天赋机器人转坦克必须保证治疗已满编，否则把治疗缺额转嫁给真实玩家。
                if (IsBotHealer(bot) && needHealers > 0)
                    continue;
                // End By leewheel
                int32 specTab = GetTankSpecTab(cls);
                if (specTab >= 0)
                {
                    // By leewheel 2026-08-01
                    // 周期性卡顿修复：天赋切换节流（同坦克职业轮转段），
                    // 被节流的 bot 本轮不参与切换，交给后续周期补位。
                    if (sRandomPlayerbotMgr.IsSpecSwitchThrottled(bot->GetGUID()))
                        continue;
                    sRandomPlayerbotMgr.RecordSpecSwitchTime(bot->GetGUID());
                    // End By leewheel
                    uint32 specIndex = sPlayerbotAIConfig.randomClassSpecIndex[cls][specTab];
                    PlayerbotFactory::InitTalentsBySpecNo(bot, specIndex, true);
                    if (bot->GetFreeTalentPoints() > 0)
                    {
                        PlayerbotFactory factory(bot, bot->GetLevel());
                        factory.InitTalentsTree(true, false, false);
                    }
                    // By leewheel 2026-07-29
                    // 天赋模板可能缺失或应用失败，切换后必须重新按实际天赋页验证。
                    if (!IsBotTank(bot))
                    {
                        LOG_WARN("playerbots", "LFG补位失败: {} 切换坦克天赋后仍不是坦克天赋，继续尝试其他机器人", bot->GetName().c_str());
                        continue;
                    }
                    // End By leewheel
                    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
                    if (botAI)
                        botAI->ResetStrategies(false);
                    // By leewheel 2026-07-29
                    if (SendLfgJoinPacket(bot, dungeonSet, lfg::PLAYER_ROLE_TANK))
                    {
                        needTanks--;
                        cand.used = true;
                        LOG_DEBUG("playerbots", "LFG补位: {} 切换天赋为坦克并成功加入队列", bot->GetName().c_str());
                        if (needTanks <= 0 && needHealers <= 0)
                            break;
                    }
                    else
                    {
                        LOG_WARN("playerbots", "LFG补位失败: {} 切换坦克天赋后未进入QUEUED/PROPOSAL，继续尝试其他机器人", bot->GetName().c_str());
                    }
                    // End By leewheel
                    continue;
                }
            }

            // 需要治疗，bot可以当治疗但当前不是治疗天赋
            if (needHealers > 0 && ClassCanHeal(cls) && !IsBotHealer(bot))
            {
                int32 specTab = GetHealerSpecTab(cls);
                if (specTab >= 0)
                {
                    // By leewheel 2026-08-01
                    // 周期性卡顿修复：天赋切换节流（同坦克职业轮转段）
                    if (sRandomPlayerbotMgr.IsSpecSwitchThrottled(bot->GetGUID()))
                        continue;
                    sRandomPlayerbotMgr.RecordSpecSwitchTime(bot->GetGUID());
                    // End By leewheel
                    uint32 specIndex = sPlayerbotAIConfig.randomClassSpecIndex[cls][specTab];
                    PlayerbotFactory::InitTalentsBySpecNo(bot, specIndex, true);
                    if (bot->GetFreeTalentPoints() > 0)
                    {
                        PlayerbotFactory factory(bot, bot->GetLevel());
                        factory.InitTalentsTree(true, false, false);
                    }
                    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
                    if (botAI)
                        botAI->ResetStrategies(false);
                    // By leewheel 2026-07-29
                    if (SendLfgJoinPacket(bot, dungeonSet, lfg::PLAYER_ROLE_HEALER))
                    {
                        needHealers--;
                        cand.used = true;
                        LOG_DEBUG("playerbots", "LFG补位: {} 切换天赋为治疗并成功加入队列", bot->GetName().c_str());
                        if (needTanks <= 0 && needHealers <= 0)
                            break;
                    }
                    else
                    {
                        LOG_WARN("playerbots", "LFG补位失败: {} 切换治疗天赋后未进入QUEUED/PROPOSAL，继续尝试其他机器人", bot->GetName().c_str());
                    }
                    // End By leewheel
                    continue;
                }
            }
        }
    }
    // End By leewheel

    // By leewheel 2026-08-18
    // 强制坦克兜底：解决"随机本20分钟无坦克入队"。常规补位（现成坦克/切换天赋）在候选池
    // 没有可用的空闲坦克职业机器人、或 SendLfgJoinPacket 失败时，会持续存在坦克缺额。
    // 此处在"超过5分钟仍缺坦克"时，强制抓取一个本阵营的坦克职业随机机器人：
    //   1. 强制切到坦克专精天赋（若当前不是）
    //   2. 刷新装备（保证战力）
    //   3. 强制加入 LFG 队列
    // 用 lfgForceTankTime 做 5 分钟节流，避免每 30 秒对同一 bot 反复刷新/入队造成资源浪费。
    if (needTanks > 0)
    {
        time_t const now = time(nullptr);
        time_t& lastForceTank = lfgForceTankTime[teamId];
        if (now - lastForceTank >= 300)
        {
            for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
            {
                Player* bot = it->second;
                if (!bot || bot->GetTeamId() != teamId || !IsRandomBot(bot))
                    continue;
                if (bot->GetLevel() < 15 || bot->InBattleground() || bot->InBattlegroundQueue() ||
                    bot->isDead() || bot->IsInCombat() || bot->IsBeingTeleported())
                    continue;
                Map* botMap = bot->GetMap();
                if (botMap && botMap->Instanceable())
                    continue;
                lfg::LfgState state = sLFGMgr->GetState(bot->GetGUID());
                if (state != lfg::LFG_STATE_NONE && state < lfg::LFG_STATE_DUNGEON)
                    continue;  // 已在队列，跳过
                if (!ClassCanTank(bot->getClass()))
                    continue;
                int32 const specTab = GetTankSpecTab(bot->getClass());
                if (specTab < 0)
                    continue;
                // 强制切到坦克天赋
                if (!IsBotTank(bot))
                {
                    uint32 const specIndex = sPlayerbotAIConfig.randomClassSpecIndex[bot->getClass()][specTab];
                    PlayerbotFactory::InitTalentsBySpecNo(bot, specIndex, true);
                    if (bot->GetFreeTalentPoints() > 0)
                    {
                        PlayerbotFactory factory(bot, bot->GetLevel());
                        factory.InitTalentsTree(true, false, false);
                    }
                    if (!IsBotTank(bot))
                        continue;  // 天赋模板缺失等仍非坦克，换下一个
                }
                // 刷新装备保证坦克战力
                PlayerbotFactory eqFactory(bot, bot->GetLevel());
                eqFactory.InitEquipment(false);
                if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot))
                    botAI->ResetStrategies(false);
                if (SendLfgJoinPacket(bot, dungeonSet, lfg::PLAYER_ROLE_TANK))
                {
                    needTanks--;
                    lastForceTank = now;
                    LOG_INFO("playerbots", "[LFG兜底] 强制抓取坦克职业机器人 {} 刷新天赋/装备并成功入队(阵营={}, 角色号={})",
                        bot->GetName().c_str(), teamId == TEAM_ALLIANCE ? "联盟" : "部落", specTab);
                    break;  // 每次只强制一个
                }
                LOG_WARN("playerbots", "[LFG兜底] 强制抓取坦克 {} 入队失败(state未进QUEUED/PROPOSAL)，尝试下一个",
                    bot->GetName().c_str());
            }
        }
    }
    // End By leewheel

    // By leewheel 2026-07-29
    // 只有所有职责缺额都归零才记录“补位完成”；仍有缺额时必须明确告警，禁止产生假成功日志。
    // By leewheel 2026-08-01
    // 周期性卡顿修复：成功路径降级为 DEBUG，避免每次补位完成都触发 INFO 日志 I/O 尖峰
    // By leewheel 2026-08-11
    // 玩家反馈"随机本坦克10分钟不进组"，需要运维能看到补位结果，升级回 INFO
    if (needTanks <= 0 && needHealers <= 0 && needDps <= 0)
    {
        LOG_INFO("playerbots", "LFG补位完成: 坦克、治疗和DPS职责均已补齐 (阵营={})",
            teamId == TEAM_ALLIANCE ? "联盟" : "部落");
    }
    else
    {
        LOG_WARN("playerbots", "LFG本轮补位结束但仍有缺额: 坦{}奶{}DPS{} (阵营={})",
            needTanks > 0 ? needTanks : 0, needHealers > 0 ? needHealers : 0, needDps > 0 ? needDps : 0,
            teamId == TEAM_ALLIANCE ? "联盟" : "部落");
    }
    // End By leewheel
}
// End By leewheel

void RandomPlayerbotMgr::CheckPlayers()
{
    if (!PlayersCheckTimer || time(nullptr) > (PlayersCheckTimer + 60))
        PlayersCheckTimer = time(nullptr);

    LOG_INFO("playerbots", "正在检查玩家...");

    if (!playersLevel)
        playersLevel = sPlayerbotAIConfig.randombotStartingLevel;

    for (std::vector<Player*>::iterator i = players.begin(); i != players.end(); ++i)
    {
        Player* player = *i;

        if (player->IsGameMaster())
            continue;

        // if (player->GetSession()->GetSecurity() > SEC_PLAYER)
        //     continue;

        if (player->GetLevel() > playersLevel)
            playersLevel = player->GetLevel() + 3;
    }

    LOG_INFO("playerbots", "最高玩家等级为 {}，机器人最高等级设为 {}", playersLevel - 3, playersLevel);
}

void RandomPlayerbotMgr::ScheduleRandomize(uint32 bot, uint32 time) { SetEventValue(bot, "randomize", 1, time); }

void RandomPlayerbotMgr::ScheduleTeleport(uint32 bot, uint32 time)
{
    if (!time)
        time = 60 + urand(sPlayerbotAIConfig.randomBotUpdateInterval, sPlayerbotAIConfig.randomBotUpdateInterval * 3);

    SetEventValue(bot, "teleport", 1, time);
}

void RandomPlayerbotMgr::ScheduleChangeStrategy(uint32 bot, uint32 time)
{
    if (!time)
        time = urand(sPlayerbotAIConfig.minRandomBotChangeStrategyTime,
                     sPlayerbotAIConfig.maxRandomBotChangeStrategyTime);

    SetEventValue(bot, "change_strategy", 1, time);
}

bool RandomPlayerbotMgr::ProcessBot(uint32 bot)
{
    ObjectGuid botGUID = ObjectGuid::Create<HighGuid::Player>(bot);
    Player* player = GetPlayerBot(botGUID);
    PlayerbotAI* botAI = player ? GET_PLAYERBOT_AI(player) : nullptr;

    uint32 isValid = GetEventValue(bot, "add");
    if (!isValid)
    {
        if (!player || !player->GetGroup())
        {
            if (player)
                LOG_DEBUG("playerbots", "机器人 #{} {}:{} <{}>: 登出", bot, IsAlliance(player->getRace()) ? "A" : "H",
                          player->GetLevel(), player->GetName().c_str());
            else
                LOG_DEBUG("playerbots", "机器人 #{}: 登出", bot);

            SetEventValue(bot, "add", 0, 0);
            currentBots.erase(bot);

            if (player)
                LogoutPlayerBot(botGUID);
        }

        return false;
    }

    uint32 randomTime;
    if (!player)
    {
        AddPlayerBot(botGUID, 0);
        randomTime = urand(1, 2);

        uint32 randomBotUpdateInterval = _isBotInitializing ? 1 : sPlayerbotAIConfig.randomBotUpdateInterval;
        randomTime = urand(std::max(5, static_cast<int>(randomBotUpdateInterval * 0.5)),
                           std::max(12, static_cast<int>(randomBotUpdateInterval * 2)));
        SetEventValue(bot, "update", 1, randomTime);

        // do not randomize or teleport immediately after server start (prevent lagging)
        if (!GetEventValue(bot, "randomize"))
        {
            randomTime = urand(3, std::max(4, static_cast<int>(randomBotUpdateInterval * 0.4)));
            ScheduleRandomize(bot, randomTime);
        }
        if (!GetEventValue(bot, "teleport"))
        {
            randomTime = urand(std::max(7, static_cast<int>(randomBotUpdateInterval * 0.7)),
                               std::max(14, static_cast<int>(randomBotUpdateInterval * 1.4)));
            ScheduleTeleport(bot, randomTime);
        }

        return true;
    }

    if (!player->IsInWorld())
        return false;

    if (player->GetGroup() || player->HasUnitState(UNIT_STATE_IN_FLIGHT))
        return false;

    // By leewheel 2026-07-29
    // LFG 状态保护：处于 LFG 队列（QUEUED）或更高级别（DUNGEON/BOOT/FINISHED）的机器人
    // 必须跳过 randomize / teleport 操作，否则会立即被踢出队列。
    // 根因：ForceBotsJoinLfg 补位后，bot 在单人状态（无 group）会被 ProcessBot 当作 idle bot
    //       执行 "传送以升级和刷新"，导致 sLFGMgr 内部状态被重置，bot 离开 LFG 队列。
    //       实际表现为：补位的 2 坦 + 2 奶在 30 秒内全部退出队列，下次检查时又变回坦0奶0DPS N。
    // 修复：处于 LFG_STATE_NONE 以外任何状态的 bot 都跳过 randomize/teleport。
    lfg::LfgState lfgState = sLFGMgr->GetState(player->GetGUID());
    if (lfgState != lfg::LFG_STATE_NONE)
    {
        // 重置所有事件以延后下一次执行，避免 bot 离开 LFG 后立刻再次被传送
        SetEventValue(bot, "update", 1, sPlayerbotAIConfig.randomBotUpdateInterval);
        return false;
    }
    // End By leewheel

    uint32 update = GetEventValue(bot, "update");
    if (!update)
    {
        if (botAI)
            botAI->GetAiObjectContext()->GetValue<bool>("random bot update")->Set(true);

        bool update = true;
        if (botAI)
        {
            // botAI->GetAiObjectContext()->GetValue<bool>("random bot update")->Set(true);
            if (!sRandomPlayerbotMgr.IsRandomBot(player))
                update = false;

            if (player->GetGroup() && botAI->GetGroupLeader())
            {
                PlayerbotAI* groupLeaderBotAI = GET_PLAYERBOT_AI(botAI->GetGroupLeader());
                if (!groupLeaderBotAI || IsSelfBot(botAI->GetGroupLeader()))
                    update = false;
            }

            // if (botAI->HasPlayerNearby(sPlayerbotAIConfig.grindDistance))
            //     update = false;
        }

        if (update)
            ProcessBot(player);

        randomTime = urand(sPlayerbotAIConfig.minRandomBotReviveTime, sPlayerbotAIConfig.maxRandomBotReviveTime);
        SetEventValue(bot, "update", 1, randomTime);

        return true;
    }

    uint32 logout = GetEventValue(bot, "logout");
    if (player && !logout && !isValid)
    {
        LOG_DEBUG("playerbots", "机器人 #{} {}:{} <{}>: 登出", bot, IsAlliance(player->getRace()) ? "A" : "H",
                  player->GetLevel(), player->GetName().c_str());
        LogoutPlayerBot(botGUID);
        currentBots.erase(bot);
        SetEventValue(bot, "logout", 1,
                      urand(sPlayerbotAIConfig.minRandomBotInWorldTime, sPlayerbotAIConfig.maxRandomBotInWorldTime));
        return true;
    }

    return false;
}

bool RandomPlayerbotMgr::ProcessBot(Player* bot)
{

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
        return false;

    if (bot->InBattleground())
        return false;

    if (bot->InBattlegroundQueue())
        return false;

    // By leewheel 2026-07-29
    // LFG 状态保护（防御层）：即使 ProcessBot(uint32) 中的检查被绕过，
    // 这里的 LFG 检查也能阻止 randomize/teleport 操作破坏 LFG 队列。
    if (sLFGMgr->GetState(bot->GetGUID()) != lfg::LFG_STATE_NONE)
        return false;
    // End By leewheel

     uint32 botId = bot->GetGUID().GetCounter();

    // if death revive
    if (bot->isDead())
    {
        if (!GetEventValue(botId, "dead"))
        {
            uint32 randomTime =
                urand(sPlayerbotAIConfig.minRandomBotReviveTime, sPlayerbotAIConfig.maxRandomBotReviveTime);
            LOG_DEBUG("playerbots", "标记机器人 {} 为死亡，将在 {} 秒后复活。", bot->GetName().c_str(),
                      randomTime);
            SetEventValue(botId, "dead", 1, sPlayerbotAIConfig.maxRandomBotInWorldTime);
            SetEventValue(botId, "revive", 1, randomTime);
            return false;
        }

        if (!GetEventValue(botId, "revive"))
        {
            Revive(bot);
            return true;
        }

        return false;
    }

    // leave group if leader is rndbot
    Group* group = bot->GetGroup();
    if (group && !group->isLFGGroup() && IsRandomBot(group->GetLeader()))
    {
        botAI->LeaveOrDisbandGroup();
        LOG_INFO("playerbots", "机器人 {} 因队长是随机机器人而从队伍中移除。", bot->GetName().c_str());
    }

    // only randomize and teleport idle bots
    bool idleBot = false;
    if (TravelTarget* target = botAI->GetAiObjectContext()->GetValue<TravelTarget*>("travel target")->Get())
    {
        if (target->getTravelState() == TravelState::TRAVEL_STATE_IDLE)
        {
            idleBot = true;
        }
    }
    else
    {
        idleBot = true;
    }

    if (idleBot)
    {
        // randomize
        uint32 randomize = GetEventValue(botId, "randomize");
        if (!randomize)
        {
            // bool randomiser = true;
            // if (player->GetGuildId())
            // {
            //     if (Guild* guild = sGuildMgr->GetGuildById(player->GetGuildId()))
            //     {
            //         if (guild->GetLeaderGUID() == player->GetGUID())
            //         {
            //             for (std::vector<Player*>::iterator i = players.begin(); i != players.end(); ++i)
            //                 GuildTaskMgr::instance().Update(*i, player);
            //         }

            //         uint32 accountId = sCharacterCache->GetCharacterAccountIdByGuid(guild->GetLeaderGUID());
            //         if (!sPlayerbotAIConfig.IsInRandomAccountList(accountId))
            //         {
            //             uint8 rank = player->GetRank();
            //             randomiser = rank < 4 ? false : true;
            //         }
            //     }
            // }
            // if (randomiser)
            // {
            Randomize(bot);
            LOG_DEBUG("playerbots", "机器人 #{} {}:{} <{}>: 已随机化", botId,
                      bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName());
            uint32 randomTime =
                urand(sPlayerbotAIConfig.minRandomBotRandomizeTime, sPlayerbotAIConfig.maxRandomBotRandomizeTime);
            ScheduleRandomize(botId, randomTime);
            return true;
        }

        // uint32 changeStrategy = GetEventValue(bot, "change_strategy");
        // if (!changeStrategy)
        // {
        //     LOG_INFO("playerbots", "Changing strategy for bot  #{} <{}>", bot, player->GetName().c_str());
        //     ChangeStrategy(player);
        //     return true;
        // }

        uint32 teleport = GetEventValue(botId, "teleport");
        if (!teleport)
        {
            LOG_DEBUG("playerbots", "机器人 #{} <{}>: 传送以升级和刷新", botId, bot->GetName());
            Refresh(bot);
            RandomTeleportForLevel(bot);
            uint32 time = urand(sPlayerbotAIConfig.minRandomBotTeleportInterval,
                                sPlayerbotAIConfig.maxRandomBotTeleportInterval);
            ScheduleTeleport(botId, time);
            return true;
        }
    }

    return false;
}

void RandomPlayerbotMgr::Revive(Player* player)
{
    uint32 bot = player->GetGUID().GetCounter();

    // LOG_INFO("playerbots", "Bot {} revived", player->GetName().c_str());
    SetEventValue(bot, "dead", 0, 0);
    SetEventValue(bot, "revive", 0, 0);

    Refresh(player);
    RandomTeleportGrindForLevel(player);
}

void RandomPlayerbotMgr::RandomTeleport(Player* bot, std::vector<WorldLocation>& locs, bool hearth)
{
    // ignore when alrdy teleported or not in the world yet.
    if (bot->IsBeingTeleported() || !bot->IsInWorld())
        return;

    // no teleport / movement update when rooted.
    if (bot->IsRooted())
        return;

    // ignore when in queue for battle grounds.
    if (bot->InBattlegroundQueue())
        return;

    // ignore when in battle grounds or arena.
    if (bot->InBattleground() || bot->InArena())
        return;

    // ignore when in group (e.g. world, dungeons, raids) and leader is not a player.
    if (bot->GetGroup() && !bot->GetGroup()->IsLeader(bot->GetGUID()))
        return;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (botAI)
    {
        // ignore when in when taxi with boat/zeppelin and has players nearby
        if (bot->HasUnitMovementFlag(MOVEMENTFLAG_ONTRANSPORT) && bot->HasUnitState(UNIT_STATE_IGNORE_PATHFINDING) &&
            botAI->HasPlayerNearby())
            return;
    }

    // if (sPlayerbotAIConfig.randomBotRpgChance < 0)
    //     return;

    if (locs.empty())
    {
        LOG_DEBUG("playerbots", "无法传送机器人 {} - 没有可用位置", bot->GetName().c_str());
        return;
    }

    std::vector<WorldPosition> tlocs;
    for (auto& loc : locs)
        tlocs.push_back(WorldPosition(loc));
    // Do not teleport to maps disabled in config
    tlocs.erase(std::remove_if(tlocs.begin(), tlocs.end(),
                               [](WorldPosition l)
                               {
                                   std::vector<uint32>::iterator i =
                                       find(sPlayerbotAIConfig.randomBotMaps.begin(),
                                            sPlayerbotAIConfig.randomBotMaps.end(), l.GetMapId());
                                   return i == sPlayerbotAIConfig.randomBotMaps.end();
                               }),
                tlocs.end());
    if (tlocs.empty())
    {
        LOG_DEBUG("playerbots", "无法传送机器人 {} - 所有位置被过滤器移除", bot->GetName().c_str());
        return;
    }

    PerfMonitorOperation* pmo = sPerfMonitor.start(PERF_MON_RNDBOT, "RandomTeleportByLocations");

    std::shuffle(std::begin(tlocs), std::end(tlocs), RandomEngine::Instance());
    for (uint32 i = 0; i < tlocs.size(); i++)
    {
        WorldLocation loc = tlocs[i];

        float x = loc.GetPositionX();  // + (attemtps > 0 ? urand(0, sPlayerbotAIConfig.grindDistance) -
                                       // sPlayerbotAIConfig.grindDistance / 2 : 0);
        float y = loc.GetPositionY();  // + (attemtps > 0 ? urand(0, sPlayerbotAIConfig.grindDistance) -
                                       // sPlayerbotAIConfig.grindDistance / 2 : 0);
        float z = loc.GetPositionZ();

        Map* map = sMapMgr->FindMap(loc.GetMapId(), 0);
        if (!map)
            continue;

        AreaTableEntry const* zone = sAreaTableStore.LookupEntry(map->GetZoneId(bot->GetPhaseMask(), x, y, z));
        if (!zone)
            continue;

        AreaTableEntry const* area = sAreaTableStore.LookupEntry(map->GetAreaId(bot->GetPhaseMask(), x, y, z));
        if (!area)
            continue;

        // Do not teleport to enemy zones if level is low
        if (zone->team == 4 && bot->GetTeamId() == TEAM_ALLIANCE)
            continue;

        if (zone->team == 2 && bot->GetTeamId() == TEAM_HORDE)
            continue;

        if (map->IsInWater(bot->GetPhaseMask(), x, y, z, bot->GetCollisionHeight()))
            continue;

        float ground = map->GetHeight(bot->GetPhaseMask(), x, y, z + 0.5f);
        if (ground <= INVALID_HEIGHT)
            continue;

        z = 0.05f + ground;

        if (!botAI->StarterLevelDistanceCheck(bot, loc, true))
            continue;

        const LocaleConstant& locale = sWorld->GetDefaultDbcLocale();
        LOG_DEBUG("playerbots",
                  "随机传送机器人 {}（等级 {}）到 地图: {} ({}) 区域: {} ({}) 子区域: {} ({}) 区域等级: {} "
                  "子区域等级: {} {},{},{} ({}/{} "
                  "个位置)",
                  bot->GetName().c_str(), bot->GetLevel(), map->GetId(), map->GetMapName(), zone->ID,
                  zone->area_name[locale], area->ID, area->area_name[locale], zone->area_level, area->area_level, x, y,
                  z, i + 1, tlocs.size());

        if (hearth)
        {
            bot->SetHomebind(loc, zone->ID);
        }

        // Prevent blink to be detected by visible real players
        if (botAI->HasPlayerNearby(150.0f))
        {
            break;
        }

        bot->GetMotionMaster()->Clear();
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (botAI)
            botAI->Reset(true);
        bot->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_TELEPORTED | AURA_INTERRUPT_FLAG_CHANGE_MAP);
        bot->TeleportTo(loc.GetMapId(), x, y, z, 0);
        bot->SendMovementFlagUpdate();

        if (pmo)
            pmo->finish();

        return;
    }

    if (pmo)
        pmo->finish();

    // LOG_ERROR("playerbots", "无法传送机器人 {} - 没有可用位置 ({} locations)", bot->GetName().c_str(),
    //           tlocs.size());
}

void RandomPlayerbotMgr::PrepareAddclassCache()
{
    // Using accounts marked as type 2 (AddClass)
    int32 collected = 0;

    for (uint32 accountId : addClassTypeAccounts)
    {
        for (uint8 claz = CLASS_WARRIOR; claz <= CLASS_DRUID; claz++)
        {
            if (claz == 10)
                continue;

            QueryResult results = CharacterDatabase.Query(
                "SELECT guid, race FROM characters "
                "WHERE account = {} AND class = '{}' AND online = 0",
                accountId, claz);

            if (results)
            {
                do
                {
                    Field* fields = results->Fetch();
                    ObjectGuid guid = ObjectGuid(HighGuid::Player, fields[0].Get<uint32>());
                    uint32 race = fields[1].Get<uint32>();
                    bool isAlliance = race == 1 || race == 3 || race == 4 || race == 7 || race == 11;
                    addclassCache[GetTeamClassIdx(isAlliance, claz)].insert(guid);
                    collected++;
                } while (results->NextRow());
            }
        }
    }

    LOG_INFO("playerbots", ">> 从 {} 个 AddClass 账号收集了 {} 个角色用于 addclass 命令。", collected, addClassTypeAccounts.size());
}

void RandomPlayerbotMgr::Init()
{
    if (sPlayerbotAIConfig.addClassCommand)
        sRandomPlayerbotMgr.PrepareAddclassCache();

    if (sPlayerbotAIConfig.randomBotJoinBG)
        sRandomPlayerbotMgr.LoadBattleMastersCache();

    PlayerbotsDatabase.Execute("DELETE FROM playerbots_random_bots WHERE event = 'add'");
}

void RandomPlayerbotMgr::InitArenaTeams()
{
    if (sPlayerbotAIConfig.deleteRandomBotArenaTeams)
    {
        RandomPlayerbotFactory::DeleteBotArenaTeams();
        return;
    }

    RandomPlayerbotFactory::LoadArenaTeamData();

    LOG_INFO("playerbots", "Bot arena teams: 2v2={}/{}, 3v3={}/{}, 5v5={}/{}",
             RandomPlayerbotFactory::GetBotArenaTeamCount(ARENA_TYPE_2v2), sPlayerbotAIConfig.randomBotArenaTeam2v2Count,
             RandomPlayerbotFactory::GetBotArenaTeamCount(ARENA_TYPE_3v3), sPlayerbotAIConfig.randomBotArenaTeam3v3Count,
             RandomPlayerbotFactory::GetBotArenaTeamCount(ARENA_TYPE_5v5), sPlayerbotAIConfig.randomBotArenaTeam5v5Count);
}

void RandomPlayerbotMgr::RandomTeleportForLevel(Player* bot)
{
    if (bot->InBattleground())
        return;

    if (bot->GetLevel() >= 10 && urand(0, 100) < sPlayerbotAIConfig.probTeleToBankers * 100)
    {
        std::vector<WorldLocation> locs = sTravelMgr.GetCityLocations(bot);
        if (!locs.empty())
        {
            RandomTeleport(bot, locs, true);
            return;
        }
    }
    std::vector<WorldLocation> locs = sTravelMgr.GetTeleportLocations(bot);
    if (!locs.empty())
    {
        RandomTeleport(bot, locs, false);
        return;
    }
}

void RandomPlayerbotMgr::RandomTeleportGrindForLevel(Player* bot)
{
    if (bot->InBattleground())
        return;

    std::vector<WorldLocation> locs = sTravelMgr.GetTeleportLocations(bot);
    LOG_DEBUG("playerbots", "为等级 {} 随机传送机器人 {}（{} 个可用位置）", bot->GetName().c_str(),
              bot->GetLevel(), locs.size());

    RandomTeleport(bot, locs);
}

void RandomPlayerbotMgr::RandomTeleport(Player* bot)
{
    if (bot->InBattleground())
        return;

    PerfMonitorOperation* pmo = sPerfMonitor.start(PERF_MON_RNDBOT, "RandomTeleport");
    std::vector<WorldLocation> locs;

    std::list<Unit*> targets;
    float range = sPlayerbotAIConfig.randomBotTeleportDistance;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, range);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, range);

    if (!targets.empty())
    {
        for (Unit* unit : targets)
        {
            bot->UpdatePosition(*unit);
            FleeManager manager(bot, sPlayerbotAIConfig.sightDistance, 0, true);
            float rx, ry, rz;
            if (manager.CalculateDestination(&rx, &ry, &rz))
            {
                WorldLocation loc(bot->GetMapId(), rx, ry, rz);
                locs.push_back(loc);
            }
        }
    }
    else
    {
        RandomTeleportForLevel(bot);
    }

    if (pmo)
        pmo->finish();

    Refresh(bot);
}

void RandomPlayerbotMgr::Randomize(Player* bot)
{
    if (bot->InBattleground())
        return;

    if (bot->GetLevel() < 3 || (bot->GetLevel() < 56 && bot->getClass() == CLASS_DEATH_KNIGHT))
    {
        RandomizeFirst(bot);
    }
    else if (bot->GetLevel() < sPlayerbotAIConfig.randomBotMaxLevel || !sPlayerbotAIConfig.downgradeMaxLevelBot)
    {
        uint8 level = bot->GetLevel();
        PlayerbotFactory factory(bot, level);
        factory.Randomize(true);
        // IncreaseLevel(bot);
    }
    else
    {
        RandomizeFirst(bot);
    }
}

void RandomPlayerbotMgr::IncreaseLevel(Player* bot)
{
    uint32 maxLevel = sPlayerbotAIConfig.randomBotMaxLevel;
    if (maxLevel > sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
        maxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);

    PerfMonitorOperation* pmo = sPerfMonitor.start(PERF_MON_RNDBOT, "IncreaseLevel");
    uint32 lastLevel = GetValue(bot, "level");
    uint8 level = bot->GetLevel() + 1;
    if (level > maxLevel)
    {
        level = maxLevel;
    }
    if (lastLevel != level)
    {
        PlayerbotFactory factory(bot, level);
        factory.Randomize(true);
    }

    if (pmo)
        pmo->finish();
}

void RandomPlayerbotMgr::RandomizeFirst(Player* bot)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
        return;

    uint32 maxLevel = sPlayerbotAIConfig.randomBotMaxLevel;
    if (maxLevel > sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
        maxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);

    // if lvl sync is enabled, max level is limited by online players lvl
    if (sPlayerbotAIConfig.syncLevelWithPlayers)
        maxLevel = std::max(sPlayerbotAIConfig.randomBotMinLevel,
                            std::min(playersLevel, sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL)));

    uint32 minLevel = sPlayerbotAIConfig.randomBotMinLevel;
    if (bot->getClass() == CLASS_DEATH_KNIGHT)
    {
        maxLevel = std::max(maxLevel, sWorld->getIntConfig(CONFIG_START_HEROIC_PLAYER_LEVEL));
        minLevel = std::max(minLevel, sWorld->getIntConfig(CONFIG_START_HEROIC_PLAYER_LEVEL));
    }

    PerfMonitorOperation* pmo = sPerfMonitor.start(PERF_MON_RNDBOT, "RandomizeFirst");

    uint32 level;

    if (sPlayerbotAIConfig.downgradeMaxLevelBot && bot->GetLevel() >= sPlayerbotAIConfig.randomBotMaxLevel)
    {
        if (bot->getClass() == CLASS_DEATH_KNIGHT)
        {
            level = sWorld->getIntConfig(CONFIG_START_HEROIC_PLAYER_LEVEL);
        }
        else
        {
            level = sPlayerbotAIConfig.randomBotMinLevel;
        }
    }
    else
    {
        uint32 roll = urand(1, 100);
        if (roll <= 100 * sPlayerbotAIConfig.randomBotMaxLevelChance)
        {
            level = maxLevel;
        }
        else if (roll <=
                 (100 * (sPlayerbotAIConfig.randomBotMaxLevelChance + sPlayerbotAIConfig.randomBotMinLevelChance)))
        {
            level = minLevel;
        }
        else
        {
            level = urand(minLevel, maxLevel);
        }
    }

    if (sPlayerbotAIConfig.disableRandomLevels)
    {
        level = bot->getClass() == CLASS_DEATH_KNIGHT ? std::max(sPlayerbotAIConfig.randombotStartingLevel,
                                                                 sWorld->getIntConfig(CONFIG_START_HEROIC_PLAYER_LEVEL))
                                                      : sPlayerbotAIConfig.randombotStartingLevel;
    }

    SetValue(bot, "level", level);
    PlayerbotFactory factory(bot, level);
    factory.Randomize(false);

    uint32 randomTime =
        urand(sPlayerbotAIConfig.minRandomBotRandomizeTime, sPlayerbotAIConfig.maxRandomBotRandomizeTime);
    uint32 inworldTime =
        urand(sPlayerbotAIConfig.minRandomBotInWorldTime, sPlayerbotAIConfig.maxRandomBotInWorldTime);

    PlayerbotsDatabasePreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_UPD_RANDOM_BOTS);
    stmt->SetData(0, randomTime);
    stmt->SetData(1, "bot_delete");
    stmt->SetData(2, bot->GetGUID().GetCounter());
    PlayerbotsDatabase.Execute(stmt);

    stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_UPD_RANDOM_BOTS);
    stmt->SetData(0, inworldTime);
    stmt->SetData(1, "logout");
    stmt->SetData(2, bot->GetGUID().GetCounter());
    PlayerbotsDatabase.Execute(stmt);

    // teleport to a random inn for bot level
    botAI->Reset(true);

    if (bot->GetGroup())
        botAI->LeaveOrDisbandGroup();

    if (pmo)
        pmo->finish();

    RandomTeleportForLevel(bot);
}

void RandomPlayerbotMgr::RandomizeMin(Player* bot)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
        return;

    PerfMonitorOperation* pmo = sPerfMonitor.start(PERF_MON_RNDBOT, "RandomizeMin");
    uint32 level = sPlayerbotAIConfig.randomBotMinLevel;
    SetValue(bot, "level", level);
    PlayerbotFactory factory(bot, level);
    factory.Randomize(false);

    uint32 randomTime =
        urand(sPlayerbotAIConfig.minRandomBotRandomizeTime, sPlayerbotAIConfig.maxRandomBotRandomizeTime);
    uint32 inworldTime =
        urand(sPlayerbotAIConfig.minRandomBotInWorldTime, sPlayerbotAIConfig.maxRandomBotInWorldTime);

    PlayerbotsDatabasePreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_UPD_RANDOM_BOTS);
    stmt->SetData(0, randomTime);
    stmt->SetData(1, "bot_delete");
    stmt->SetData(2, bot->GetGUID().GetCounter());
    PlayerbotsDatabase.Execute(stmt);

    stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_UPD_RANDOM_BOTS);
    stmt->SetData(0, inworldTime);
    stmt->SetData(1, "logout");
    stmt->SetData(2, bot->GetGUID().GetCounter());
    PlayerbotsDatabase.Execute(stmt);

    // teleport to a random inn for bot level
    botAI->Reset(true);

    if (bot->GetGroup())
        botAI->LeaveOrDisbandGroup();

    if (pmo)
        pmo->finish();
}

void RandomPlayerbotMgr::Clear(Player* bot)
{
    PlayerbotFactory factory(bot, bot->GetLevel());
    factory.ClearEverything();
}

uint32 RandomPlayerbotMgr::GetZoneLevel(uint16 mapId, float teleX, float teleY, float /*teleZ*/)
{
    uint32 maxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);

    uint32 level = 0;
    QueryResult results = WorldDatabase.Query(
        "SELECT AVG(t.minlevel) minlevel, AVG(t.maxlevel) maxlevel FROM creature c "
        "INNER JOIN creature_template t ON c.id = t.entry WHERE map = {} AND minlevel > 1 AND ABS(position_x - {}) < "
        "{} AND ABS(position_y - {}) < {}",
        mapId, teleX, sPlayerbotAIConfig.randomBotTeleportDistance / 2, teleY,
        sPlayerbotAIConfig.randomBotTeleportDistance / 2);

    if (results)
    {
        Field* fields = results->Fetch();
        uint8 minLevel = fields[0].Get<uint8>();
        uint8 maxLevel = fields[1].Get<uint8>();
        level = urand(minLevel, maxLevel);
        if (level > maxLevel)
            level = maxLevel;
    }
    else
    {
        level = urand(1, maxLevel);
    }

    return level;
}

void RandomPlayerbotMgr::Refresh(Player* bot)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
        return;

    if (bot->isDead())
    {
        bot->ResurrectPlayer(1.0f);
        bot->SpawnCorpseBones();
        botAI->ResetStrategies(false);
    }

    // if (sPlayerbotAIConfig.disableRandomLevels)
    //     return;

    if (bot->InBattleground())
        return;

    LOG_DEBUG("playerbots", "正在刷新机器人 {} <{}>", bot->GetGUID().ToString().c_str(), bot->GetName().c_str());

    PerfMonitorOperation* pmo = sPerfMonitor.start(PERF_MON_RNDBOT, "Refresh");

    botAI->Reset();

    bot->DurabilityRepairAll(false, 1.0f, false);
    bot->SetFullHealth();
    bot->SetPvP(sWorld->IsPvPRealm());
    PlayerbotFactory factory(bot, bot->GetLevel());
    factory.Refresh();

    if (bot->GetMaxPower(POWER_MANA) > 0)
        bot->SetPower(POWER_MANA, bot->GetMaxPower(POWER_MANA));

    if (bot->GetMaxPower(POWER_ENERGY) > 0)
        bot->SetPower(POWER_ENERGY, bot->GetMaxPower(POWER_ENERGY));

    uint32 money = bot->GetMoney();
    bot->SetMoney(money + 500 * sqrt(urand(1, bot->GetLevel() * 5)));

    if (bot->GetGroup())
        botAI->LeaveOrDisbandGroup();

    if (pmo)
        pmo->finish();
}

bool RandomPlayerbotMgr::IsRandomBot(Player* bot)
{
    if (bot && GET_PLAYERBOT_AI(bot))
    {
        if (IsSelfBot(bot))
            return false;
    }
    if (bot)
    {
        return IsRandomBot(bot->GetGUID().GetCounter());
    }

    return false;
}

bool RandomPlayerbotMgr::IsRandomBot(ObjectGuid::LowType bot)
{
    ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(bot);
    if (!sPlayerbotAIConfig.IsInRandomAccountList(sCharacterCache->GetCharacterAccountIdByGuid(guid)))
        return false;

    return currentBots.contains(bot);
}

bool RandomPlayerbotMgr::IsAddclassBot(Player* bot)
{
    if (bot && GET_PLAYERBOT_AI(bot))
    {
        if (IsSelfBot(bot))
            return false;
    }
    if (bot)
    {
        return IsAddclassBot(bot->GetGUID().GetCounter());
    }

    return false;
}

bool RandomPlayerbotMgr::IsAddclassBot(ObjectGuid::LowType bot)
{
    ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(bot);

    // Check the cache with faction considerations
    for (uint8 claz = CLASS_WARRIOR; claz <= CLASS_DRUID; claz++)
    {
        if (claz == 10)
            continue;

        for (uint8 isAlliance = 0; isAlliance <= 1; isAlliance++)
        {
            if (addclassCache[GetTeamClassIdx(isAlliance, claz)].find(guid) !=
                addclassCache[GetTeamClassIdx(isAlliance, claz)].end())
            {
                return true;
            }
        }
    }

    // If not in cache, check the account type
    uint32 accountId = sCharacterCache->GetCharacterAccountIdByGuid(guid);
    if (accountId && IsAccountType(accountId, 2)) // Type 2 = AddClass
    {
        return true;
    }

    return false;
}

void RandomPlayerbotMgr::GetBots()
{
    if (!currentBots.empty())
        return;

    PlayerbotsDatabasePreparedStatement* stmt =
        PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_RANDOM_BOTS_BY_OWNER_AND_EVENT);
    stmt->SetData(0, 0);
    stmt->SetData(1, "add");
    uint32 maxAllowedBotCount = GetEventValue(0, "bot_count");
    if (PreparedQueryResult result = PlayerbotsDatabase.Query(stmt))
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 bot = fields[0].Get<uint32>();
            if (GetEventValue(bot, "add"))
                currentBots.insert(bot);

            if (currentBots.size() >= maxAllowedBotCount)
                break;
        } while (result->NextRow());
    }
}

std::vector<uint32> RandomPlayerbotMgr::GetBgBots(uint32 bracket)
{
    // if (!currentBgBots.empty()) return currentBgBots;

    std::vector<uint32> BgBots;

    PlayerbotsDatabasePreparedStatement* stmt =
        PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_RANDOM_BOTS_BY_EVENT_AND_VALUE);
    stmt->SetData(0, "bg");
    stmt->SetData(1, bracket);
    if (PreparedQueryResult result = PlayerbotsDatabase.Query(stmt))
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 bot = fields[0].Get<uint32>();
            BgBots.push_back(bot);
        } while (result->NextRow());
    }

    return BgBots;
}

CachedEvent* RandomPlayerbotMgr::FindEvent(uint32 bot, std::string const& event)
{
    BotEventCache& cache = eventCache[bot];

    // Load once
    if (!cache.loaded)
    {
        cache.events.clear();

        PlayerbotsDatabasePreparedStatement* stmt =
            PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_RANDOM_BOTS_BY_OWNER_AND_BOT);
        stmt->SetData(0, 0);
        stmt->SetData(1, bot);

        if (PreparedQueryResult result = PlayerbotsDatabase.Query(stmt))
        {
            do
            {
                Field* fields = result->Fetch();

                CachedEvent e;
                e.value = fields[1].Get<uint32>();
                e.lastChangeTime = fields[2].Get<uint32>();
                e.validIn = fields[3].Get<uint32>();
                e.data = fields[4].Get<std::string>();

                cache.events.emplace(fields[0].Get<std::string>(), std::move(e));
            } while (result->NextRow());
        }

        cache.loaded = true;
    }

    auto it = cache.events.find(event);
    if (it == cache.events.end())
        return nullptr;

    CachedEvent& e = it->second;

    // remove expired events
    if (e.validIn && (NowSeconds() - e.lastChangeTime) >= e.validIn && event != "specNo" && event != "specLink")
    {
        cache.events.erase(it);
        return nullptr;
    }

    return &e;
}

bool RandomPlayerbotMgr::IsSpecPvp(uint32 bot, uint8 cls)
{
    uint32 stored = GetValue(bot, "specNo");
    if (!stored)
        return false;
    uint32 specIndex = stored - 1;
    std::string const& name = sPlayerbotAIConfig.premadeSpecName[cls][specIndex];
    return !name.empty() && name.find("pvp") != std::string::npos;
}

uint32 RandomPlayerbotMgr::GetEventValue(uint32 bot, std::string const& event)
{
    if (CachedEvent* e = FindEvent(bot, event))
        return e->value;

    return 0;
}

std::string RandomPlayerbotMgr::GetEventData(uint32 bot, std::string const& event)
{
    if (CachedEvent* e = FindEvent(bot, event))
        return e->data;

    return "";
}

uint32 RandomPlayerbotMgr::SetEventValue(uint32 bot, std::string const& event, uint32 value, uint32 validIn,
                                         std::string const& data)
{
    // By Leewheel 2026-07-08 - 性能优化：将 BeginTransaction/CommitTransaction
    // 改为异步 Execute。原事务方式会从连接池获取连接（BeginTransaction），
    // 当批量处理多个机器人时（每次更新循环可能处理数十个机器人），
    // 大量并发的事务请求会耗尽连接池导致主线程阻塞，造成随机 1s 卡顿。
    // 这些事件值都是临时机器人状态（更新计时器、传送计划等），
    // 即使 DELETE 和 INSERT 之间不保证原子性也不会造成问题。
    PlayerbotsDatabasePreparedStatement* stmt =
        PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_DEL_RANDOM_BOTS_BY_OWNER_AND_EVENT);
    stmt->SetData(0, 0);
    stmt->SetData(1, bot);
    stmt->SetData(2, event.c_str());
    PlayerbotsDatabase.Execute(stmt);

    if (value)
    {
        stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_INS_RANDOM_BOTS);
        stmt->SetData(0, 0);
        stmt->SetData(1, bot);
        stmt->SetData(2, NowSeconds());
        stmt->SetData(3, validIn);
        stmt->SetData(4, event.c_str());
        stmt->SetData(5, value);

        if (!data.empty())
            stmt->SetData(6, data.c_str());
        else
            stmt->SetData(6);  // NULL

        PlayerbotsDatabase.Execute(stmt);
    }

    // Update in-memory cache
    BotEventCache& cache = eventCache[bot];
    cache.loaded = true;

    if (!value)
    {
        cache.events.erase(event);
        return 0;
    }

    CachedEvent& e = cache.events[event];  // create-on-write is OK here
    e.value = value;
    e.lastChangeTime = NowSeconds();
    e.validIn = validIn;
    e.data = data;

    return value;
}

uint32 RandomPlayerbotMgr::GetValue(uint32 bot, std::string const& type) { return GetEventValue(bot, type); }

uint32 RandomPlayerbotMgr::GetValue(Player* bot, std::string const& type)
{
    return GetValue(bot->GetGUID().GetCounter(), type);
}

std::string RandomPlayerbotMgr::GetData(uint32 bot, std::string const& type) { return GetEventData(bot, type); }

void RandomPlayerbotMgr::SetValue(uint32 bot, std::string const& type, uint32 value, std::string const& data)
{
    SetEventValue(bot, type, value, sPlayerbotAIConfig.maxRandomBotInWorldTime, data);
}

void RandomPlayerbotMgr::SetValue(Player* bot, std::string const& type, uint32 value, std::string const& data)
{
    SetValue(bot->GetGUID().GetCounter(), type, value, data);
}

bool RandomPlayerbotMgr::HandlePlayerbotConsoleCommand(ChatHandler* /*handler*/, char const* args)
{
    if (!sPlayerbotAIConfig.enabled)
    {
        LOG_ERROR("playerbots", "玩家机器人系统当前已禁用！");
        return false;
    }

    if (!args || !*args)
    {
        LOG_ERROR("playerbots", "用法: rndbot stats/update/reset/init/refresh/add/remove");
        return false;
    }

    std::string const cmd = args;

    if (cmd == "reset")
    {
        PlayerbotsDatabase.Execute(PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_DEL_RANDOM_BOTS));
        sRandomPlayerbotMgr.eventCache.clear();
        LOG_INFO("playerbots", "所有玩家的随机机器人已重置，请重启服务器。");
        return true;
    }

    if (cmd == "stats")
    {
        sRandomPlayerbotMgr.PrintStats();
        // activatePrintStatsThread();
        return true;
    }

    if (cmd == "reload")
    {
        sPlayerbotAIConfig.Initialize();
        return true;
    }

    if (cmd == "update")
    {
        sRandomPlayerbotMgr.UpdateAIInternal(0);
        return true;
    }

    std::map<std::string, ConsoleCommandHandler> handlers;
    // handlers["initmin"] = &RandomPlayerbotMgr::RandomizeMin;
    handlers["init"] = &RandomPlayerbotMgr::RandomizeFirst;
    handlers["clear"] = &RandomPlayerbotMgr::Clear;
    handlers["levelup"] = handlers["level"] = &RandomPlayerbotMgr::IncreaseLevel;
    handlers["refresh"] = &RandomPlayerbotMgr::Refresh;
    handlers["teleport"] = &RandomPlayerbotMgr::RandomTeleportForLevel;
    // handlers["rpg"] = &RandomPlayerbotMgr::RandomTeleportForRpg;
    handlers["revive"] = &RandomPlayerbotMgr::Revive;
    handlers["grind"] = &RandomPlayerbotMgr::RandomTeleport;
    handlers["change_strategy"] = &RandomPlayerbotMgr::ChangeStrategy;

    for (std::map<std::string, ConsoleCommandHandler>::iterator j = handlers.begin(); j != handlers.end(); ++j)
    {
        std::string const prefix = j->first;
        if (cmd.find(prefix) != 0)
            continue;

        std::string const name = cmd.size() > prefix.size() + 1 ? cmd.substr(1 + prefix.size()) : "%";

        std::vector<uint32> botIds;
        for (std::vector<uint32>::iterator i = sPlayerbotAIConfig.randomBotAccounts.begin();
             i != sPlayerbotAIConfig.randomBotAccounts.end(); ++i)
        {
            uint32 account = *i;
            if (QueryResult results = CharacterDatabase.Query(
                    "SELECT guid FROM characters WHERE account = {} AND name like '{}'", account, name.c_str()))
            {
                do
                {
                    Field* fields = results->Fetch();

                    uint32 botId = fields[0].Get<uint32>();
                    ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(botId);
                    if (!sRandomPlayerbotMgr.IsRandomBot(guid.GetCounter()))
                    {
                        continue;
                    }
                    Player* bot = ObjectAccessor::FindPlayer(guid);
                    if (!bot)
                        continue;

                    botIds.push_back(botId);
                } while (results->NextRow());
            }
        }

        if (botIds.empty())
        {
            LOG_INFO("playerbots", "无需操作");
            return false;
        }

        uint32 processed = 0;
        for (std::vector<uint32>::iterator i = botIds.begin(); i != botIds.end(); ++i)
        {
            ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(*i);
            Player* bot = ObjectAccessor::FindPlayer(guid);
            if (!bot)
                continue;

            LOG_INFO("playerbots", "[{}/{}] 正在为机器人 {} 处理命令 {}", processed++, botIds.size(), cmd.c_str(),
                     bot->GetName().c_str());

            ConsoleCommandHandler handler = j->second;
            (sRandomPlayerbotMgr.*handler)(bot);
        }

        return true;
    }

    // std::vector<std::string> messages = sRandomPlayerbotMgr.HandlePlayerbotCommand(args);
    // for (std::vector<std::string>::iterator i = messages.begin(); i != messages.end(); ++i)
    // {
    //     LOG_INFO("playerbots", "{}", i->c_str());
    // }
    return true;
}

void RandomPlayerbotMgr::HandleCommand(uint32 type, std::string const text, Player* fromPlayer, std::string channelName)
{
    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        if (!bot)
            continue;

        if (!channelName.empty())
        {
            if (ChannelMgr* cMgr = ChannelMgr::forTeam(bot->GetTeamId()))
            {
                Channel* chn = cMgr->GetChannel(channelName, bot);
                if (!chn)
                    continue;
            }
        }

        GET_PLAYERBOT_AI(bot)->HandleCommand(type, text, fromPlayer);
    }
}

void RandomPlayerbotMgr::OnPlayerLogout(Player* player)
{
    DisablePlayerBot(player->GetGUID());

    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (botAI && player == botAI->GetMaster())
        {
            botAI->SetMaster(nullptr);
            if (!bot->InBattleground())
            {
                botAI->ResetStrategies();
            }
        }
    }

    std::vector<Player*>::iterator i = std::find(players.begin(), players.end(), player);
    if (i != players.end())
        players.erase(i);
}

void RandomPlayerbotMgr::OnBotLoginInternal(Player* const bot)
{
    if (_isBotLogging)
    {
        LOG_INFO("playerbots", "{}/{} 机器人 {} 已登录", playerBots.size(),
                 sRandomPlayerbotMgr.GetMaxAllowedBotCount(), bot->GetName().c_str());

        if (playerBots.size() == sRandomPlayerbotMgr.GetMaxAllowedBotCount())
        {
            _isBotLogging = false;
        }
    }

    // Run guild recovery/assignment at login to handle empty guild tables after restart.
    if (sPlayerbotAIConfig.randomBotGuildCount > 0)
    {
        PlayerbotFactory factory(bot, bot->GetLevel());
        factory.InitGuild();
    }

    RandomPlayerbotFactory::AssignBotToArenaTeam(bot);

    if (sPlayerbotAIConfig.randomBotFixedLevel)
    {
        bot->SetPlayerFlag(PLAYER_FLAGS_NO_XP_GAIN);
    }
    else
    {
        bot->RemovePlayerFlag(PLAYER_FLAGS_NO_XP_GAIN);
    }
}

void RandomPlayerbotMgr::OnPlayerLogin(Player* player)
{
    uint32 botsNearby = 0;

    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        if (player == bot /* || GET_PLAYERBOT_AI(player)*/)  // TEST
            continue;

        Cell playerCell(player->GetPositionX(), player->GetPositionY());
        Cell botCell(bot->GetPositionX(), bot->GetPositionY());

        // if (playerCell == botCell)
        // botsNearby++;

        Group* group = bot->GetGroup();
        if (!group)
            continue;

        for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
        {
            Player* member = gref->GetSource();
            PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
            if (botAI && member == player && (!botAI->GetMaster() || GET_PLAYERBOT_AI(botAI->GetMaster())))
            {
                if (!bot->InBattleground())
                {
                    botAI->SetMaster(player);
                    botAI->ResetStrategies();
                    botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                        "hello", "你好", {}));
                }

                break;
            }
        }
    }

    if (botsNearby > 100 && false)
    {
        WorldPosition botPos(player);

        // botPos.GetReachableRandomPointOnGround(player, sPlayerbotAIConfig.reactDistance * 2, true);

        // player->TeleportTo(botPos);
        // player->Relocate(botPos.coord_x, botPos.coord_y, botPos.coord_z, botPos.orientation);

        if (!player->GetFactionTemplateEntry())
        {
            botPos.GetReachableRandomPointOnGround(player, sPlayerbotAIConfig.reactDistance * 2, true);
        }
        else
        {
            std::vector<TravelDestination*> dests = TravelMgr::instance().getRpgTravelDestinations(player, true, true, 200000.0f);

            do
            {
                RpgTravelDestination* dest = (RpgTravelDestination*)dests[urand(0, dests.size() - 1)];
                CreatureTemplate const* cInfo = dest->GetCreatureTemplate();
                if (!cInfo)
                    continue;

                FactionTemplateEntry const* factionEntry = sFactionTemplateStore.LookupEntry(cInfo->faction);
                ReputationRank reaction = Unit::GetFactionReactionTo(player->GetFactionTemplateEntry(), factionEntry);

                if (reaction > REP_NEUTRAL && dest->nearestPoint(&botPos)->GetMapId() == player->GetMapId())
                {
                    botPos = *dest->nearestPoint(&botPos);
                    break;
                }
            } while (true);
        }

        player->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_TELEPORTED | AURA_INTERRUPT_FLAG_CHANGE_MAP);
        player->TeleportTo(botPos);

        // player->Relocate(botPos.getX(), botPos.getY(), botPos.getZ(), botPos.getO());
    }

    if (IsRandomBot(player))
    {
        // ObjectGuid::LowType guid = player->GetGUID().GetCounter(); //not used, conditional could be rewritten for
        // simplicity. line marked for removal.
        player->SetPvP(sWorld->IsPvPRealm());
    }
    else
    {
        players.push_back(player);
        LOG_DEBUG("playerbots", "将非随机机器人玩家 {} 纳入随机机器人更新", player->GetName().c_str());
    }
}

void RandomPlayerbotMgr::OnPlayerLoginError(uint32 bot)
{
    SetEventValue(bot, "add", 0, 0);
    currentBots.erase(bot);
}

Player* RandomPlayerbotMgr::GetRandomPlayer()
{
    if (players.empty())
        return nullptr;

    uint32 index = urand(0, players.size() - 1);
    return players[index];
}

void RandomPlayerbotMgr::PrintStats()
{
    printStatsTimer = time(nullptr);
    LOG_INFO("playerbots", "随机机器人统计：{} 在线", playerBots.size());

    std::map<uint8, uint32> alliance, horde;
    for (uint32 i = 0; i < 10; ++i)
    {
        alliance[i] = 0;
        horde[i] = 0;
    }

    std::map<uint8, uint32> perRace;
    std::map<uint8, uint32> perClass;

    std::map<uint8, uint32> lvlPerRace;
    std::map<uint8, uint32> lvlPerClass;
    for (uint8 race = RACE_HUMAN; race < sRaceMgr->GetMaxRaces(); ++race)
    {
        perRace[race] = 0;
        lvlPerRace[race] = 0;
    }

    for (uint8 cls = CLASS_WARRIOR; cls < MAX_CLASSES; ++cls)
    {
        perClass[cls] = 0;
        lvlPerClass[cls] = 0;
    }

    uint32 dps = 0;
    uint32 heal = 0;
    uint32 tank = 0;
    uint32 active = 0;
/*    uint32 update = 0;
    uint32 randomize = 0;
    uint32 teleport = 0;
    uint32 changeStrategy = 0;*/
    uint32 dead = 0;
    uint32 combat = 0;
    // uint32 revive = 0; //not used, line marked for removal.
    uint32 inFlight = 0;
    uint32 moving = 0;
    uint32 mounted = 0;
    uint32 inBg = 0;
    uint32 rest = 0;
    uint32 engine_noncombat = 0;
    uint32 engine_combat = 0;
    uint32 engine_dead = 0;
    std::unordered_map<NewRpgStatus, int> rpgStatusCount;
    // static NewRpgStatistic rpgStasticTotal;
    std::unordered_map<uint32, int> zoneCount;
    uint8 maxBotLevel = 0;
    for (PlayerBotMap::iterator i = playerBots.begin(); i != playerBots.end(); ++i)
    {
        Player* bot = i->second;
        if (IsAlliance(bot->getRace()))
            ++alliance[bot->GetLevel()];
        else
            ++horde[bot->GetLevel()];
        maxBotLevel = std::max(maxBotLevel, bot->GetLevel());

        ++perRace[bot->getRace()];
        ++perClass[bot->getClass()];

        lvlPerClass[bot->getClass()] += bot->GetLevel();
        lvlPerRace[bot->getRace()] += bot->GetLevel();

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (!botAI)
        {
            LOG_ERROR("playerbots", "玩家/机器人 {} 已在 sRandomPlayerbotMgr playerBots 中注册但没有机器人 AI！", bot->GetName().c_str());
            continue;
        }

        if (botAI->IsActivityAllowedCached())
            ++active;
        /* TODO: Review statistics on rpg merge
        if (botAI->GetAiObjectContext()->GetValue<bool>("random bot update")->Get())
            ++update;

        uint32 botId = bot->GetGUID().GetCounter();
        if (!GetEventValue(botId, "randomize"))
            ++randomize;

        if (!GetEventValue(botId, "teleport"))
            ++teleport;

        if (!GetEventValue(botId, "change_strategy"))
            ++changeStrategy;
        */
        if (bot->isDead())
        {
            ++dead;
            // if (!GetEventValue(botId, "dead"))
            //++revive;
        }
        if (bot->IsInCombat())
            ++combat;

        if (bot->isMoving())
            ++moving;

        if (bot->IsInFlight())
            ++inFlight;

        if (bot->IsMounted())
            ++mounted;

        if (bot->InBattleground() || bot->InArena())
            ++inBg;

        if (bot->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING))
            ++rest;

        if (botAI->GetState() == BOT_STATE_NON_COMBAT)
            ++engine_noncombat;

        else if (botAI->GetState() == BOT_STATE_COMBAT)
            ++engine_combat;

        else
            ++engine_dead;

        if (botAI->IsHeal(bot, false))
            ++heal;

        else if (botAI->IsTank(bot, false))
            ++tank;

        else
            ++dps;

        zoneCount[bot->GetZoneId()]++;

        if (sPlayerbotAIConfig.enableNewRpgStrategy)
        {
            rpgStatusCount[botAI->rpgInfo.GetStatus()]++;
            rpgStasticTotal += botAI->rpgStatistic;
            botAI->rpgStatistic = NewRpgStatistic();
        }
    }

    LOG_INFO("playerbots", "机器人等级：");
    // uint32 maxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);
    uint32_t currentAlliance = 0, currentHorde = 0;
    uint32_t step = std::max(1, static_cast<int>((maxBotLevel + 4) / 8));
    uint32_t from = 1;

    for (uint8 i = 1; i <= maxBotLevel; ++i)
    {
        currentAlliance += alliance[i];
        currentHorde += horde[i];

        if (((i + 1) % step == 0) || i == maxBotLevel)
        {
            if (currentAlliance || currentHorde)
                LOG_INFO("playerbots", "    {}..{}: {} 联盟, {} 部落", from, i, currentAlliance, currentHorde);
            currentAlliance = 0;
            currentHorde = 0;
            from = i + 1;
        }
    }

    LOG_INFO("playerbots", "机器人种族：");
    for (uint8 race = RACE_HUMAN; race < sRaceMgr->GetMaxRaces(); ++race)
    {
        if (perRace[race])
        {
            uint32 lvl = lvlPerRace[race] * 10 / perRace[race];
            float flvl = lvl / 10.0f;
            LOG_INFO("playerbots", "    {}: {}, 平均等级: {}", ChatHelper::FormatRaceLog(race).c_str(), perRace[race],
                     flvl);
        }
    }

    LOG_INFO("playerbots", "机器人职业：");
    for (uint8 cls = CLASS_WARRIOR; cls < MAX_CLASSES; ++cls)
    {
        if (perClass[cls])
        {
            uint32 lvl = lvlPerClass[cls] * 10 / perClass[cls];
            float flvl = lvl / 10.0f;
            LOG_INFO("playerbots", "    {}: {}, 平均等级: {}", ChatHelper::FormatClassLog(cls).c_str(), perClass[cls],
                     flvl);
        }
    }

    LOG_INFO("playerbots", "机器人职责：");
    LOG_INFO("playerbots", "    坦克: {}, 治疗: {}, 输出: {}", tank, heal, dps);

    LOG_INFO("playerbots", "机器人状态：");
    LOG_INFO("playerbots", "    活跃: {}", active);
    LOG_INFO("playerbots", "    移动中: {}", moving);

    // LOG_INFO("playerbots", "Bots to:");
    // LOG_INFO("playerbots", "    update: {}", update);
    // LOG_INFO("playerbots", "    randomize: {}", randomize);
    // LOG_INFO("playerbots", "    teleport: {}", teleport);
    // LOG_INFO("playerbots", "    change_strategy: {}", changeStrategy);
    // LOG_INFO("playerbots", "    revive: {}", revive);

    LOG_INFO("playerbots", "    飞行中: {}", inFlight);
    LOG_INFO("playerbots", "    骑乘中: {}", mounted);
    LOG_INFO("playerbots", "    战斗中: {}", combat);
    LOG_INFO("playerbots", "    战场中: {}", inBg);
    LOG_INFO("playerbots", "    休息中: {}", rest);
    LOG_INFO("playerbots", "    死亡: {}", dead);

    if (sPlayerbotAIConfig.enableNewRpgStrategy)
    {
        LOG_INFO("playerbots", "机器人 RPG 状态：");
        LOG_INFO("playerbots",
                 "    Idle: {}, Rest: {}, GoGrind: {}, GoCamp: {}, MoveRandom: {}, MoveNpc: {}, DoQuest: {}, "
                 "TravelFlight: {}, OutdoorPvP: {}",
                 rpgStatusCount[RPG_IDLE], rpgStatusCount[RPG_REST], rpgStatusCount[RPG_GO_GRIND],
                 rpgStatusCount[RPG_GO_CAMP], rpgStatusCount[RPG_WANDER_RANDOM], rpgStatusCount[RPG_WANDER_NPC],
                 rpgStatusCount[RPG_DO_QUEST], rpgStatusCount[RPG_TRAVEL_FLIGHT], rpgStatusCount[RPG_OUTDOOR_PVP]);

        LOG_INFO("playerbots", "机器人任务总数：");
        LOG_INFO("playerbots", "    已接受: {}, 已奖励: {}, 已放弃: {}", rpgStasticTotal.questAccepted,
                 rpgStasticTotal.questRewarded, rpgStasticTotal.questDropped);
    }

    LOG_INFO("playerbots", "机器人引擎：", dead);
    LOG_INFO("playerbots", "    非战斗: {}, 战斗: {}, 死亡: {}", engine_noncombat, engine_combat, engine_dead);
}

double RandomPlayerbotMgr::GetBuyMultiplier(Player* bot)
{
    uint32 id = bot->GetGUID().GetCounter();
    uint32 value = GetEventValue(id, "buymultiplier");
    if (!value)
    {
        value = urand(50, 120);
        uint32 validIn = urand(sPlayerbotAIConfig.minRandomBotsPriceChangeInterval,
                               sPlayerbotAIConfig.maxRandomBotsPriceChangeInterval);
        SetEventValue(id, "buymultiplier", value, validIn);
    }

    return (double)value / 100.0;
}

double RandomPlayerbotMgr::GetSellMultiplier(Player* bot)
{
    uint32 id = bot->GetGUID().GetCounter();
    uint32 value = GetEventValue(id, "sellmultiplier");
    if (!value)
    {
        value = urand(80, 250);
        uint32 validIn = urand(sPlayerbotAIConfig.minRandomBotsPriceChangeInterval,
                               sPlayerbotAIConfig.maxRandomBotsPriceChangeInterval);
        SetEventValue(id, "sellmultiplier", value, validIn);
    }

    return (double)value / 100.0;
}

void RandomPlayerbotMgr::AddTradeDiscount(Player* bot, Player* master, int32 value)
{
    if (!master)
        return;

    uint32 discount = GetTradeDiscount(bot, master);
    int32 result = (int32)discount + value;
    discount = (result < 0 ? 0 : result);

    SetTradeDiscount(bot, master, discount);
}

void RandomPlayerbotMgr::SetTradeDiscount(Player* bot, Player* master, uint32 value)
{
    if (!master)
        return;

    uint32 botId = bot->GetGUID().GetCounter();
    uint32 masterId = master->GetGUID().GetCounter();

    std::ostringstream name;
    name << "trade_discount_" << masterId;
    SetEventValue(botId, name.str(), value, sPlayerbotAIConfig.maxRandomBotInWorldTime);
}

uint32 RandomPlayerbotMgr::GetTradeDiscount(Player* bot, Player* master)
{
    if (!master)
        return 0;

    uint32 botId = bot->GetGUID().GetCounter();
    uint32 masterId = master->GetGUID().GetCounter();

    std::ostringstream name;
    name << "trade_discount_" << masterId;
    return GetEventValue(botId, name.str());
}

std::string const RandomPlayerbotMgr::HandleRemoteCommand(std::string const request)
{
    std::string::const_iterator pos = std::find(request.begin(), request.end(), ',');
    if (pos == request.end())
    {
        std::ostringstream out;
        out << "无效请求: " << request;
        return out.str();
    }

    std::string const command = std::string(request.begin(), pos);
    ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(atoi(std::string(pos + 1, request.end()).c_str()));
    Player* bot = GetPlayerBot(guid);
    if (!bot)
        return "无效的 GUID";

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
        return "无效的 GUID";

    return botAI->HandleRemoteCommand(command);
}

void RandomPlayerbotMgr::ChangeStrategy(Player* player)
{
    uint32 bot = player->GetGUID().GetCounter();

    if (frand(0.f, 100.f) > sPlayerbotAIConfig.randomBotRpgChance)
    {
        LOG_INFO("playerbots", "机器人 #{} <{}>: 已送往刷怪点", bot, player->GetName().c_str());
        ScheduleTeleport(bot, 30);
    }
    else
    {
        LOG_INFO("playerbots", "正在将机器人 #{} <{}> 的策略改为 RPG", bot, player->GetName().c_str());
        LOG_INFO("playerbots", "机器人 #{} <{}>: 已送往旅店", bot, player->GetName().c_str());
        RandomTeleportForLevel(player);
        SetEventValue(bot, "teleport", 1, sPlayerbotAIConfig.maxRandomBotInWorldTime);
    }

    ScheduleChangeStrategy(bot);
}

void RandomPlayerbotMgr::ChangeStrategyOnce(Player* player)
{
    uint32 bot = player->GetGUID().GetCounter();

    if (frand(0.f, 100.f) > sPlayerbotAIConfig.randomBotRpgChance)  // select grind / pvp
    {
        LOG_INFO("playerbots", "机器人 #{} <{}>: 已送往刷怪点", bot, player->GetName().c_str());
        RandomTeleportForLevel(player);
        Refresh(player);
    }
    else
    {
        LOG_INFO("playerbots", "机器人 #{} <{}>: 已送往旅店", bot, player->GetName().c_str());
        RandomTeleportForLevel(player);
    }
}

void RandomPlayerbotMgr::RandomTeleportForRpg(Player* bot)
{
    uint32 race = bot->getRace();
    uint32 level = bot->GetLevel();
    LOG_DEBUG("playerbots", "为 RPG 随机传送机器人 {}（{} 个可用位置）", bot->GetName().c_str(),
              rpgLocsCacheLevel[race].size());
    RandomTeleport(bot, rpgLocsCacheLevel[race][level], true);
}

void RandomPlayerbotMgr::Remove(Player* bot)
{
    ObjectGuid owner = bot->GetGUID();

    PlayerbotsDatabasePreparedStatement* stmt =
        PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_DEL_RANDOM_BOTS_BY_OWNER);
    stmt->SetData(0, 0);
    stmt->SetData(1, owner.GetCounter());
    PlayerbotsDatabase.Execute(stmt);

    uint32 botId = owner.GetCounter();
    eventCache.erase(botId);

    LogoutPlayerBot(owner);
}

CreatureData const* RandomPlayerbotMgr::GetCreatureDataByEntry(uint32 entry)
{
    if (entry != 0)
    {
        for (auto const& itr : sObjectMgr->GetAllCreatureData())
            if (itr.second.id == entry)
                return &itr.second;
    }

    return nullptr;
}

ObjectGuid RandomPlayerbotMgr::GetBattleMasterGUID(Player* bot, BattlegroundTypeId bgTypeId)
{
    ObjectGuid battleMasterGUID = ObjectGuid::Empty;

    TeamId team = bot->GetTeamId();
    std::vector<uint32> Bms;

    for (auto i = std::begin(BattleMastersCache[team][bgTypeId]); i != std::end(BattleMastersCache[team][bgTypeId]);
         ++i)
    {
        Bms.insert(Bms.end(), *i);
    }

    for (auto i = std::begin(BattleMastersCache[TEAM_NEUTRAL][bgTypeId]);
         i != std::end(BattleMastersCache[TEAM_NEUTRAL][bgTypeId]); ++i)
    {
        Bms.insert(Bms.end(), *i);
    }

    if (Bms.empty())
        return battleMasterGUID;

    float dist1 = FLT_MAX;

    for (auto i = begin(Bms); i != end(Bms); ++i)
    {
        CreatureData const* data = sRandomPlayerbotMgr.GetCreatureDataByEntry(*i);
        if (!data)
            continue;

        Unit* Bm = PlayerbotAI::GetUnit(data);
        if (!Bm)
            continue;

        if (bot->GetMapId() != Bm->GetMapId())
            continue;

        // return first available guid on map if queue from anywhere
        if (!BattlegroundMgr::IsArenaType(bgTypeId))
        {
            battleMasterGUID = Bm->GetGUID();
            break;
        }

        AreaTableEntry const* zone = sAreaTableStore.LookupEntry(Bm->GetZoneId());
        if (!zone)
            continue;

        if (zone->team == 4 && bot->GetTeamId() == TEAM_ALLIANCE)
            continue;

        if (zone->team == 2 && bot->GetTeamId() == TEAM_HORDE)
            continue;

        if (Bm->getDeathState() == DeathState::Dead)
            continue;

        float dist2 = ServerFacade::instance().GetDistance2d(bot, data->posX, data->posY);
        if (dist2 < dist1)
        {
            dist1 = dist2;
            battleMasterGUID = Bm->GetGUID();
        }
    }

    return battleMasterGUID;
}
