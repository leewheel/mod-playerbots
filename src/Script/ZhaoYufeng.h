//By leewheel 2026-09-22
/*
 * ZhaoYufeng.h —— 特殊机器人「赵与风」
 *
 * 需求来源（老大）：
 *   "我想设计一个特别的机器人，名字叫赵与风，战士，玩家组随机本或者团本的时候，
 *    有25%会组到他，这个机器人需要一个独立的策略，那就是所有Roll的东西全需求。"
 *
 * 实现要点：
 *   1. 角色：名字「赵与风」，职业战士；联盟/部落各一位（characters.name 允许重名，已实查
 *      data/sql/base/db_characters/characters.sql —— name 只有普通 KEY idx_name，非 UNIQUE）。
 *      两位角色由本系统在服务器启动时从"随机机器人账号"中征用一名战士并改名而来，
 *      因此天然复用随机机器人的账号体系（IsInRandomAccountList 判定、装备/天赋/等级初始化）。
 *   2. 编入队伍：玩家组随机本（LFG 队列补位）或团本（快速组队 .10人团/.25人团/.40人团）时，
 *      按配置概率（默认 25%）把他编入队伍；他正在别的队伍里时不抢人。
 *   3. 独立策略："always need" —— 所有 Roll 一律投 NEED（见 AlwaysNeedStrategy.h + LootRollAction.cpp）。
 *
 * 说明：本系统只"征用并改名"已有随机机器人的角色，不新建账号/角色，
 *       以最大限度复用既有登录、装备、天赋、策略流程，降低对核心的侵入。
 */
//End By leewheel

#ifndef ZHAO_YUFENG_H
#define ZHAO_YUFENG_H

#include "ObjectGuid.h"

#include <ctime>
#include <string>

class Player;

// 赵与风条目（每个阵营一位）
struct ZhaoYufengEntry
{
    ObjectGuid guid;
    uint32 accountId = 0;
    uint8  race = 0;
    uint8  playerClass = 0;
    uint8  gender = 0;
    uint32 level = 0;
    bool   valid = false;
};

class ZhaoYufengMgr
{
public:
    static ZhaoYufengMgr& instance();

    // 服务器启动时调用（此时 sPlayerbotAIConfig.randomBotAccounts 已就绪）
    void Load();
    // 懒加载兜底：若启动时未能完成加载，首次使用时再试一次
    void EnsureLoaded();

    bool IsEnabled() const { return m_enabled; }
    uint32 GetChance() const { return m_chance; }
    std::string const& GetName() const { return m_name; }

    // 是否为赵与风
    bool IsZhaoYufeng(ObjectGuid guid) const;
    bool IsZhaoYufeng(Player* player) const;

    // 取某阵营的条目（teamId: 0=联盟 1=部落）
    ZhaoYufengEntry const* GetEntryForTeam(uint32 teamId) const;

    // 取某阵营"可编入队伍"的赵与风（在线、未入队、未在战斗中）；不可用返回 nullptr
    Player* FindAvailableForTeam(uint32 teamId);

    // 本次是否触发（按配置概率掷骰）
    bool RollChance() const;

    // 给他挂上"全需求"独立策略
    void ApplyAlwaysNeed(Player* bot);

    // 把他的等级/天赋/装备调整为指定等级，使其能进入目标队伍或副本
    void PrepareForLevel(Player* bot, uint32 level);

    // 状态查询命令用
    std::string BuildStatusText() const;

    // ---- 随机本（LFG）编入状态 ----
    // 说明：随机本的编入需要访问 RandomPlayerbotMgr.cpp 内部的文件级静态函数
    //       （IsBotIdleForLfg / SendLfgJoinPacket），因此判定与执行分离：
    //       BeginLfgPending 由 CheckLfgQueue 在"排队开始"时调用（掷骰），
    //       入队动作由 ForceBotsJoinLfg 读取这里的标记后执行。
    void BeginLfgPending(uint32 teamId, uint32 queuedLevel);
    bool IsLfgPending(uint32 teamId) const;
    ObjectGuid GetLfgTarget(uint32 teamId) const;
    uint32 GetLfgTargetLevel(uint32 teamId) const;
    bool HasLfgSummoned(uint32 teamId) const;
    void MarkLfgSummoned(uint32 teamId);
    bool IsLfgExpired(uint32 teamId) const;
    void ClearLfgPending(uint32 teamId);

private:
    ZhaoYufengMgr() = default;
    ~ZhaoYufengMgr() = default;
    ZhaoYufengMgr(const ZhaoYufengMgr&) = delete;
    ZhaoYufengMgr& operator=(const ZhaoYufengMgr&) = delete;

    void LoadConfig();
    void LoadEntriesFromDb();
    void RecruitIfMissing(uint32 teamId);

    ZhaoYufengEntry m_entries[2];
    std::string     m_name = "赵与风";
    bool            m_enabled = true;
    uint32          m_chance = 25;
    bool            m_loaded = false;

    // 随机本编入状态（按阵营：0=联盟 1=部落）
    bool       m_lfgPending[2]  = { false, false };
    ObjectGuid m_lfgTarget[2];
    uint32     m_lfgLevel[2]    = { 0, 0 };
    bool       m_lfgSummoned[2] = { false, false };
    time_t     m_lfgDeadline[2] = { 0, 0 };
};

#define sZhaoYufengMgr ZhaoYufengMgr::instance()

#endif // ZHAO_YUFENG_H
