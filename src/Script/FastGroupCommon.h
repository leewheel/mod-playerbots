//By leewheel 2026-07-07
/*
 * FastGroupCommon.h - 快速组队系统公共头文件
 *
 * 功能说明：
 *   提供快速组队系统和自动加入团本系统共享的类型定义、
 *   FastGroupMgr 管理类声明以及公共辅助函数声明。
 *   供 FastGroup.cpp 和 AutoJoinRaid.cpp 共同使用。
 *
 * 作者: leewheel
 */
//End By leewheel

#ifndef FAST_GROUP_COMMON_H
#define FAST_GROUP_COMMON_H

#include "ObjectGuid.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>
#include <ctime>

class Player;
class ChatHandler;

// ============================================================
//  组队形式枚举 - By leewheel 2026-07-10
//  记录在 playerbots_fast_group_members 表中
//  用于区分机器人是快速组队、随机本还是邀请组队的
//  快速组队的机器人在退队时需要清理装备（装备是系统临时分配的）
//  随机本和邀请组队的机器人保留装备
// ============================================================
enum FastGroupType
{
    FG_TYPE_FAST_GROUP  = 0,  // 快速组队（.5人队/.10人团等命令）
    FG_TYPE_LFG         = 1,  // 随机本（LFG队列匹配）
    FG_TYPE_INVITE      = 2   // 邀请组队（玩家手动邀请）
};
// End By leewheel

// ============================================================
//  角色定位枚举
// ============================================================
enum FastGroupRole
{
    FG_ROLE_TANK  = 0,
    FG_ROLE_HEAL  = 1,
    FG_ROLE_DPS   = 2,
    FG_ROLE_MAX   = 3
};

// ============================================================
//  队伍配置结构
// ============================================================
struct FastGroupConfig
{
    uint32 totalMembers;  // 总人数（含玩家）
    uint32 tanks;          // 坦克数量（含玩家）
    uint32 heals;          // 治疗数量（含玩家）
    uint32 dps;            // 输出数量（含玩家）
};

// ============================================================
//  标准队伍配置表（定义在 FastGroup.cpp 中）
// ============================================================
enum FastGroupConfigIndex
{
    FG_CONFIG_PARTY_5  = 0,
    FG_CONFIG_RAID_10  = 1,
    FG_CONFIG_RAID_25  = 2,
    FG_CONFIG_RAID_40  = 3,
};

extern const FastGroupConfig FastGroupConfigs[];

// ============================================================
//  每个职业可担当的角色列表
//  说明：specTab 是天赋页索引（0/1/2），对应客户端天赋面板从左到右
// ============================================================
struct ClassRoleEntry
{
    uint8 playerClass;
    int   specTab;       // 天赋页索引，-1表示该职业所有天赋均可
    FastGroupRole role;
};

extern const ClassRoleEntry ClassRoleTable[];
extern const size_t ClassRoleTableSize;

// ============================================================
//  候选机器人信息结构
// ============================================================
struct BotCandidate
{
    ObjectGuid guid;
    std::string name;
    uint8 playerClass;
    uint8 race;
    uint32 accountId;
    FastGroupRole role;       // 该机器人要担当的角色
    int  specTab;             // 要设置的天赋页索引
};

// ============================================================
//  待设置的机器人信息（用于 OnPlayerLogin 时设置）
// ============================================================
struct PendingBotSetup
{
    uint32 targetLevel;
    FastGroupRole role;
    int specTab;
    ObjectGuid masterGuid;
    std::string botName;
    std::string masterName;
};

// ============================================================
//  小退保持：已保存的快速组队队伍构成 - By leewheel 2026-09-01
//  主控登出（小退）时记录机器人构成，主控再次登录时自动召回恢复。
//  仅存内存：服务器重启后丢失，重启场景由启动清理逻辑按旧行为处理。
// ============================================================
struct SavedFastGroupComposition
{
    std::vector<ObjectGuid> botGuids;   // 机器人GUID列表（保持召唤顺序）
    time_t savedAt = 0;                 // 保存时间戳（用于过期判断）
};

// 小退恢复待传送记录：机器人全部上线（或30秒超时）后传送到主控身边
struct FastGroupRestoreTeleport
{
    std::vector<ObjectGuid> botGuids;
    time_t createTime = 0;
};
// End By leewheel

// ============================================================
//  快速组队核心管理类
//  说明：方法实现在 FastGroup.cpp 中
// ============================================================
class FastGroupMgr
{
public:
    static FastGroupMgr& instance();

    // ---- 待设置机器人管理 ----
    void AddPendingSetup(ObjectGuid botGuid, const PendingBotSetup& setup);
    bool PopPendingSetup(ObjectGuid botGuid, PendingBotSetup& out);
    void ClearPendingSetups(ObjectGuid masterGuid);

    // ---- 快速组队机器人列表管理（内存） ----
    void RegisterFastGroupBots(Player* master, const std::vector<ObjectGuid>& botGuids);
    //By leewheel 2026-09-01 用户需求：副本中小退再进保持队伍状态
    // preserveForRelog=true（主控登出）：不清装备、不删DB记录、不解散队伍，
    //   仅让机器人下线，并把队伍构成存入内存，等主控登录时自动召回恢复。
    // preserveForRelog=false（显式退队/解散/重新组队）：保持原有彻底清理行为。
    void LogoutFastGroupBots(Player* master, bool preserveForRelog = false);
    //End By leewheel

    // 检查玩家是否已有快速组队机器人
    bool HasFastGroupBots(ObjectGuid masterGuid);

    // By leewheel 2026-07-07
    // 获取玩家已注册的快速组队机器人GUID列表
    // 供 AutoJoinRaid 检查机器人是否全部上线后执行传送
    std::vector<ObjectGuid> GetFastGroupBotGuids(ObjectGuid masterGuid);
    // End By leewheel

    // By leewheel 2026-07-10
    // ---- 数据库记录管理 ----
    // 向 playerbots_fast_group_members 表插入记录
    void DbAddFastGroupMember(uint32 masterGuid, uint32 botGuid, const std::string& botName, uint8 botClass, uint8 groupType);
    // 从表中删除指定机器人的记录
    void DbRemoveFastGroupMember(uint32 botGuid);
    // 删除指定主控玩家的所有记录
    void DbRemoveAllByMaster(uint32 masterGuid);
    // 查询指定主控玩家的所有快速组队机器人GUID列表
    std::vector<uint32> DbGetBotGuidsByMaster(uint32 masterGuid);
    // 检查指定机器人是否在表中（且为快速组队类型）
    bool DbIsFastGroupBot(uint32 botGuid);
    // 检查指定主控玩家是否有快速组队记录
    bool DbHasFastGroupBots(uint32 masterGuid);
    // 清理表中所有不在线的机器人记录（服务器启动时调用）
    void DbCleanupOfflineBots();
    // End By leewheel

    // By leewheel 2026-07-08
    // 角色分配跟踪：记录哪些主控玩家已经完成了LFG角色分配
    bool HasRolesAssigned(ObjectGuid masterGuid);
    void SetRolesAssigned(ObjectGuid masterGuid, bool assigned);
    // End By leewheel

    //By leewheel 2026-09-01 用户需求：副本中小退再进保持队伍状态
    // ---- 小退保持：队伍构成存取（内存） ----
    // 主控登出时保存队伍构成（由 LogoutFastGroupBots(preserve=true) 调用）
    void SaveCompositionOnLogout(ObjectGuid masterGuid, const std::vector<ObjectGuid>& botGuids);
    // 主控登录时取出并移除保存的队伍构成；超过有效期(24h)视为残留返回false
    bool PopSavedComposition(ObjectGuid masterGuid, std::vector<ObjectGuid>& outBotGuids);
    // 清除指定主控的保存构成（显式退队/解散时调用，防止误恢复）
    void ClearSavedComposition(ObjectGuid masterGuid);
    //End By leewheel

private:
    FastGroupMgr() = default;
    ~FastGroupMgr() = default;
    FastGroupMgr(const FastGroupMgr&) = delete;
    FastGroupMgr& operator=(const FastGroupMgr&) = delete;

    // 主控玩家GUID -> 快速组队上线的机器人GUID列表
    std::unordered_map<ObjectGuid, std::vector<ObjectGuid>> m_fastGroupBots;

    // 机器人GUID -> 待设置信息（用于 OnPlayerLogin 时设置等级/天赋/装备）
    std::unordered_map<ObjectGuid, PendingBotSetup> m_pendingSetups;

    // By leewheel 2026-07-08
    // 已完成LFG角色分配的主控玩家GUID集合
    std::set<ObjectGuid> m_rolesAssigned;
    // End By leewheel

    //By leewheel 2026-09-01 用户需求：副本中小退再进保持队伍状态
    // 主控玩家GUID -> 小退时保存的快速组队队伍构成
    std::unordered_map<ObjectGuid, SavedFastGroupComposition> m_savedCompositions;
    //End By leewheel
};

#define sFastGroupMgr FastGroupMgr::instance()

// ============================================================
//  公共辅助函数声明（实现在 FastGroup.cpp 中）
// ============================================================

// 获取玩家的天赋页索引（0/1/2），基于天赋点数最多的那一页
uint8 GetSpecTab(Player* player);

// 判断是否为坦克（纯天赋页判断，不依赖形态/光环）
bool IsTankBySpec(Player* player);

// 判断是否为治疗（纯天赋页判断）
bool IsHealBySpec(Player* player);

// 获取玩家角色定位
FastGroupRole GetPlayerRole(Player* player);

// 获取角色中文名
const char* GetRoleNameCN(FastGroupRole role);

// 获取职业中文名
const char* GetClassNameCN(uint8 playerClass);

// 按天赋页索引直接分配天赋点（不依赖预设天赋链接）
void InitTalentsByTab(Player* player, uint8 specTab);

// 从数据库中查找符合职业和阵营要求的离线机器人
std::vector<BotCandidate> FindOfflineBotsForRole(
    FastGroupRole role,
    uint8 teamId,
    uint32 neededCount,
    uint32 masterLevel,
    std::unordered_set<uint32>& usedGuids,
    std::set<uint8>& usedClasses);

// 主组队函数（ChatHandler 可选，为 nullptr 时不发送聊天消息）
bool DoFastGroup(Player* master, FastGroupConfigIndex configIndex, ChatHandler* handler = nullptr);

// By leewheel 2026-07-08
// 为队伍中所有成员分配LFG角色（坦克/治疗/输出），并发送 SMSG_LFG_ROLE_CHOSEN 包
// 使客户端显示角色图标
void AssignLfgRoles(Player* master);
// End By leewheel

#endif // FAST_GROUP_COMMON_H
