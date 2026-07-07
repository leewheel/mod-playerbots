//By leewheel 2026-07-07
/*
 * AutoJoinRaid.cpp - 自动加入团本系统
 *
 * 功能说明：
 *   当玩家处于团队浏览器（Raid Browser / LFR）中时，
 *   机器人自动响应加入玩家想去的团本，
 *   按照玩家选择的团本规定实际人数入团，
 *   并满足团队坦克、治疗、DPS的配置。
 *
 *   配置参数：Playerbot.Auto.Join.Raid（默认开启）
 *
 * 工作原理：
 *   1. 通过 OnPlayerAfterUpdate 钩子定期检测玩家 LFR 状态
 *   2. 当检测到玩家处于 LFG_STATE_RAIDBROWSER 时，
 *      获取玩家选择的团本副本，根据副本地图的 maxPlayers 确定人数
 *   3. 调用 DoFastGroup 自动召唤机器人组建标准配置的团队
 *   4. 玩家离开 LFR 或下线时，自动清理已召唤的机器人
 *
 *   机器人上线后的等级/天赋/装备设置由 FastGroupPlayerScript
 *   的 OnPlayerLogin 钩子统一处理，无需在此重复实现。
 *
 * 作者: leewheel
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Chat.h"
#include "Group.h"
#include "ObjectAccessor.h"
#include "Log.h"
#include "SharedDefines.h"
#include "WorldSession.h"

// LFG 相关头文件
#include "LFGMgr.h"
#include "LFG.h"

// DBC 相关头文件
#include "DBCStores.h"
#include "DBCStructure.h"

// 配置管理
#include "Config.h"

// Playerbots 模块头文件
#include "Playerbots.h"

// 共享头文件（FastGroupMgr、DoFastGroup 等）
#include "FastGroupCommon.h"

#include <unordered_map>
#include <set>
#include <ctime>

// ============================================================
//  辅助函数：判断配置是否开启
// ============================================================
static bool IsAutoJoinRaidEnabled()
{
    return sConfigMgr->GetOption<bool>("Playerbot.Auto.Join.Raid", true);
}

// ============================================================
//  辅助函数：根据 LFR 选择的副本确定团本人数
//  返回值：maxPlayers（10/25/40），0 表示无法确定
// ============================================================
static uint32 GetRaidMaxPlayersFromDungeons(Player* player)
{
    if (!player)
        return 0;

    // 获取玩家在 LFR 中选择的副本列表
    lfg::LfgDungeonSet const& dungeons = sLFGMgr->GetSelectedDungeons(player->GetGUID());
    if (dungeons.empty())
        return 0;

    // 遍历所有选择的副本，找到第一个有效的团本地图
    for (uint32 dungeonId : dungeons)
    {
        // 获取副本数据
        lfg::LFGDungeonData const* dungeon = sLFGMgr->GetLFGDungeon(dungeonId);
        if (!dungeon)
            continue;

        // 只处理团本类型（LFG_TYPE_RAID = 2）
        if (dungeon->type != lfg::LFG_TYPE_RAID)
            continue;

        // 通过地图 ID 查找 MapEntry，获取 maxPlayers
        MapEntry const* mapEntry = sMapStore.LookupEntry(dungeon->map);
        if (mapEntry && mapEntry->maxPlayers > 0)
        {
            return mapEntry->maxPlayers;
        }

        // 如果 MapEntry 没有有效的 maxPlayers，使用难度来判断
        // RAID_DIFFICULTY_25MAN_NORMAL = 1, RAID_DIFFICULTY_25MAN_HEROIC = 3
        // RAID_DIFFICULTY_MASK_25MAN = 1
        if (dungeon->difficulty & RAID_DIFFICULTY_MASK_25MAN)
            return 25;
        else
            return 10;
    }

    return 0;
}

// ============================================================
//  辅助函数：根据 maxPlayers 确定队伍配置索引
// ============================================================
static FastGroupConfigIndex GetConfigIndexByMaxPlayers(uint32 maxPlayers)
{
    if (maxPlayers <= 5)
        return FG_CONFIG_PARTY_5;
    else if (maxPlayers <= 10)
        return FG_CONFIG_RAID_10;
    else if (maxPlayers <= 25)
        return FG_CONFIG_RAID_25;
    else
        return FG_CONFIG_RAID_40;
}

// ============================================================
//  PlayerScript：自动检测 LFR 状态并响应加入团本
// ============================================================
class AutoJoinRaidPlayerScript : public PlayerScript
{
public:
    AutoJoinRaidPlayerScript() : PlayerScript("AutoJoinRaidPlayerScript", {
        PLAYERHOOK_ON_AFTER_UPDATE,
        PLAYERHOOK_ON_LOGOUT
    }) {}

    // 定期检测玩家是否在团本浏览器中
    void OnPlayerAfterUpdate(Player* player, uint32 /*p_time*/) override
    {
        if (!player)
            return;

        // 跳过机器人，只处理真实玩家
        if (GET_PLAYERBOT_AI(player))
            return;

        // 检查配置是否开启
        if (!IsAutoJoinRaidEnabled())
            return;

        // 只处理满级或高等级玩家（等级太低没有团本）
        if (player->GetLevel() < 10)
            return;

        ObjectGuid guid = player->GetGUID();
        time_t now = time(nullptr);

        // 冷却检查：每 5 秒检测一次，避免每帧都查询 LFG 状态
        auto it = m_nextCheck.find(guid);
        if (it != m_nextCheck.end() && now < it->second)
            return;

        m_nextCheck[guid] = now + 5;  // 下次检测在 5 秒后

        // 检查玩家是否处于团本浏览器状态
        lfg::LfgState lfgState = sLFGMgr->GetState(guid);

        if (lfgState != lfg::LFG_STATE_RAIDBROWSER)
        {
            // 玩家不在团本浏览器中，清除已处理标记
            // （这样玩家再次进入 LFR 时可以重新触发）
            m_processed.erase(guid);
            return;
        }

        // 已经处理过了，不重复触发
        if (m_processed.find(guid) != m_processed.end())
            return;

        // 如果玩家已有快速组队机器人在线，不重复召唤
        if (sFastGroupMgr.HasFastGroupBots(guid))
            return;

        // 战斗中不触发，等下次检测
        if (player->IsInCombat())
            return;

        // 获取团本人数
        uint32 maxPlayers = GetRaidMaxPlayersFromDungeons(player);
        if (maxPlayers == 0)
            return;  // 无法确定团本人数，等下次检测

        // 确定队伍配置
        FastGroupConfigIndex configIndex = GetConfigIndexByMaxPlayers(maxPlayers);

        // 标记为已处理
        m_processed.insert(guid);

        // 创建 ChatHandler 用于发送消息给玩家
        ChatHandler handler(player->GetSession());
        handler.PSendSysMessage("|cff00ff00[自动加入团本] 检测到您在团本浏览器中，正在自动组建 {} 人团队...|r", maxPlayers);

        // 调用 DoFastGroup 组建团队
        bool success = DoFastGroup(player, configIndex, &handler);

        if (success)
        {
            LOG_INFO("playerbots", "自动加入团本：玩家 {} 在团本浏览器中选择了 {} 人团本，已自动召唤机器人组建团队。",
                player->GetName(), maxPlayers);
        }
        else
        {
            // 组队失败，清除已处理标记，允许下次重试
            m_processed.erase(guid);
            handler.PSendSysMessage("|cffffcc00[自动加入团本] 自动组建团队失败，稍后将重试。|r");
        }
    }

    // 玩家下线时清理状态
    void OnPlayerLogout(Player* player) override
    {
        if (!player)
            return;

        ObjectGuid guid = player->GetGUID();
        m_nextCheck.erase(guid);
        m_processed.erase(guid);

        // 注意：机器人下线由 FastGroupPlayerScript::OnPlayerLogout 处理
        // 这里只需要清理 AutoJoinRaid 自己的状态
    }

private:
    // 玩家 GUID -> 下次检测时间
    std::unordered_map<ObjectGuid, time_t> m_nextCheck;

    // 已处理的玩家 GUID 集合（防止重复触发）
    std::set<ObjectGuid> m_processed;
};

// ============================================================
//  脚本注册入口
// ============================================================
void AddSC_AutoJoinRaid()
{
    new AutoJoinRaidPlayerScript();
}
//End By leewheel
