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

class Player;
class ChatHandler;

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

    // ---- 快速组队机器人列表管理 ----
    void RegisterFastGroupBots(Player* master, const std::vector<ObjectGuid>& botGuids);
    void LogoutFastGroupBots(Player* master);

    // 检查玩家是否已有快速组队机器人
    bool HasFastGroupBots(ObjectGuid masterGuid);

    // By leewheel 2026-07-07
    // 获取玩家已注册的快速组队机器人GUID列表
    // 供 AutoJoinRaid 检查机器人是否全部上线后执行传送
    std::vector<ObjectGuid> GetFastGroupBotGuids(ObjectGuid masterGuid);
    // End By leewheel

private:
    FastGroupMgr() = default;
    ~FastGroupMgr() = default;
    FastGroupMgr(const FastGroupMgr&) = delete;
    FastGroupMgr& operator=(const FastGroupMgr&) = delete;

    // 主控玩家GUID -> 快速组队上线的机器人GUID列表
    std::unordered_map<ObjectGuid, std::vector<ObjectGuid>> m_fastGroupBots;

    // 机器人GUID -> 待设置信息（用于 OnPlayerLogin 时设置等级/天赋/装备）
    std::unordered_map<ObjectGuid, PendingBotSetup> m_pendingSetups;
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

#endif // FAST_GROUP_COMMON_H
