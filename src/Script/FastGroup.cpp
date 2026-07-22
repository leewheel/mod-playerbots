//By leewheel 2026-07-06
/*
 * FastGroup.cpp - 快速组队系统
 *
 * 功能说明：
 *   通过命令 .5人队 / .10人团 / .25人团 / .40人团 快速组建标准配置的队伍。
 *   系统会根据玩家自身的职业和天赋角色（坦克/治疗/输出），自动从离线机器人中
 *   筛选合适职业天赋的角色上线、设置等级、穿最佳装备、设置天赋并加入队伍。
 *   玩家离队或下线时，由快速组队系统上线的机器人会自动全部下线。
 *
 * 组队配置标准：
 *   5人队  : 1坦克 + 1治疗 + 3输出（含玩家，即玩家+4机器人）
 *   10人团 : 2坦克 + 3治疗 + 5输出（含玩家）
 *   25人团 : 2坦克 + 5治疗 + 18输出（含玩家）
 *   40人团 : 2坦克 + 8治疗 + 30输出（含玩家）
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
#include "DatabaseEnv.h"

#include "Playerbots.h"
#include "AiFactory.h"
#include "PlayerbotFactory.h"
#include "DBCStores.h"
#include "DBCStructure.h"

// By leewheel 2026-07-08 - LFG角色分配所需头文件
#include "LFGMgr.h"
#include "LFG.h"
#include "WorldPacket.h"
#include "Opcodes.h"
// End By leewheel

// By leewheel 2026-07-07 - 引入共享头文件，供 AutoJoinRaid.cpp 共同使用
#include "FastGroupCommon.h"
// End By leewheel

#include <unordered_set>
#include <map>
#include <set>
#include <string>

using namespace Acore::ChatCommands;

// ============================================================
//  标准队伍配置表定义
// ============================================================
const FastGroupConfig FastGroupConfigs[] =
{
    { 5,  1, 1,  3 },   // 5人队
    { 10, 2, 3,  5 },   // 10人团
    { 25, 2, 5, 18 },   // 25人团
    { 40, 2, 8, 30 },   // 40人团
};

// ============================================================
//  每个职业可担当的角色列表定义
//  说明：specTab 是天赋页索引（0/1/2），对应客户端天赋面板从左到右
// ============================================================
const ClassRoleEntry ClassRoleTable[] =
{
    // 战士：武器(0)=DPS, 狂暴(1)=DPS, 防护(2)=TANK
    { CLASS_WARRIOR,      2, FG_ROLE_TANK },
    { CLASS_WARRIOR,      0, FG_ROLE_DPS  },
    { CLASS_WARRIOR,      1, FG_ROLE_DPS  },
    // 圣骑士：神圣(0)=HEAL, 防护(1)=TANK, 惩戒(2)=DPS
    { CLASS_PALADIN,      0, FG_ROLE_HEAL },
    { CLASS_PALADIN,      1, FG_ROLE_TANK },
    { CLASS_PALADIN,      2, FG_ROLE_DPS  },
    // 猎人：全部DPS
    { CLASS_HUNTER,      -1, FG_ROLE_DPS  },
    // 盗贼：全部DPS
    { CLASS_ROGUE,       -1, FG_ROLE_DPS  },
    // 牧师：戒律(0)=HEAL, 神圣(1)=HEAL, 暗影(2)=DPS
    { CLASS_PRIEST,       0, FG_ROLE_HEAL },
    { CLASS_PRIEST,       1, FG_ROLE_HEAL },
    { CLASS_PRIEST,       2, FG_ROLE_DPS  },
    // 死骑：鲜血(0)=TANK, 冰霜(1)=DPS, 邪恶(2)=DPS
    // 注：WLK中冰霜DK主要做DPS，与IsTankBySpec只判鲜血保持一致
    { CLASS_DEATH_KNIGHT, 0, FG_ROLE_TANK },
    { CLASS_DEATH_KNIGHT, 1, FG_ROLE_DPS  },
    { CLASS_DEATH_KNIGHT, 2, FG_ROLE_DPS  },
    // 萨满：元素(0)=DPS, 增强(1)=DPS, 恢复(2)=HEAL
    { CLASS_SHAMAN,       0, FG_ROLE_DPS  },
    { CLASS_SHAMAN,       1, FG_ROLE_DPS  },
    { CLASS_SHAMAN,       2, FG_ROLE_HEAL },
    // 法师：全部DPS
    { CLASS_MAGE,        -1, FG_ROLE_DPS  },
    // 术士：全部DPS
    { CLASS_WARLOCK,     -1, FG_ROLE_DPS  },
    // 德鲁伊：平衡(0)=DPS, 野性(1)=TANK, 恢复(2)=HEAL
    { CLASS_DRUID,        0, FG_ROLE_DPS  },
    { CLASS_DRUID,        1, FG_ROLE_TANK },
    { CLASS_DRUID,        2, FG_ROLE_HEAL },
};

const size_t ClassRoleTableSize = sizeof(ClassRoleTable) / sizeof(ClassRoleTable[0]);

// ============================================================
//  FastGroupMgr 方法实现
// ============================================================

FastGroupMgr& FastGroupMgr::instance()
{
    static FastGroupMgr inst;
    return inst;
}

// ============================================================
//  数据库记录管理方法 - By leewheel 2026-07-10
//  操作 playerbots_fast_group_members 表
//  用于持久化记录快速组队的机器人信息
// ============================================================

void FastGroupMgr::DbAddFastGroupMember(uint32 masterGuid, uint32 botGuid, const std::string& botName, uint8 botClass, uint8 groupType)
{
    CharacterDatabase.Execute(
        "INSERT INTO playerbots_fast_group_members (master_guid, bot_guid, bot_name, bot_class, group_type) "
        "VALUES ({}, {}, '{}', {}, {}) "
        "ON DUPLICATE KEY UPDATE master_guid = {}, bot_name = '{}', bot_class = {}, group_type = {}",
        masterGuid, botGuid, botName, botClass, groupType,
        masterGuid, botName, botClass, groupType);
}

void FastGroupMgr::DbRemoveFastGroupMember(uint32 botGuid)
{
    CharacterDatabase.Execute("DELETE FROM playerbots_fast_group_members WHERE bot_guid = {}", botGuid);
}

void FastGroupMgr::DbRemoveAllByMaster(uint32 masterGuid)
{
    CharacterDatabase.Execute("DELETE FROM playerbots_fast_group_members WHERE master_guid = {}", masterGuid);
}

std::vector<uint32> FastGroupMgr::DbGetBotGuidsByMaster(uint32 masterGuid)
{
    std::vector<uint32> result;
    QueryResult res = CharacterDatabase.Query(
        "SELECT bot_guid FROM playerbots_fast_group_members WHERE master_guid = {} AND group_type = {}",
        masterGuid, FG_TYPE_FAST_GROUP);
    if (res)
    {
        do
        {
            result.push_back(res->Fetch()[0].Get<uint32>());
        } while (res->NextRow());
    }
    return result;
}

bool FastGroupMgr::DbIsFastGroupBot(uint32 botGuid)
{
    QueryResult res = CharacterDatabase.Query(
        "SELECT 1 FROM playerbots_fast_group_members WHERE bot_guid = {} AND group_type = {} LIMIT 1",
        botGuid, FG_TYPE_FAST_GROUP);
    return res != nullptr;
}

bool FastGroupMgr::DbHasFastGroupBots(uint32 masterGuid)
{
    QueryResult res = CharacterDatabase.Query(
        "SELECT 1 FROM playerbots_fast_group_members WHERE master_guid = {} AND group_type = {} LIMIT 1",
        masterGuid, FG_TYPE_FAST_GROUP);
    return res != nullptr;
}

void FastGroupMgr::DbCleanupOfflineBots()
{
    // By leewheel 2026-07-10
    // 服务器启动时调用，此时所有机器人都已下线
    // 不能简单清空表！如果服务器崩溃，OnPlayerLogout 未被触发，
    // 机器人的装备没被清理，表记录也还在
    // 正确做法：读取表中的记录，直接用SQL清空这些机器人在数据库中的装备，
    //           然后再清空表
    QueryResult res = CharacterDatabase.Query(
        "SELECT bot_guid, bot_name FROM playerbots_fast_group_members WHERE group_type = {}",
        FG_TYPE_FAST_GROUP);
    
    if (res)
    {
        uint32 count = 0;
        do
        {
            Field* fields = res->Fetch();
            uint32 botGuid = fields[0].Get<uint32>();
            std::string botName = fields[1].Get<std::string>();

            // 直接在数据库中清空该角色的所有物品
            // character_inventory 记录物品位置和entry
            // item_instance 记录物品详细数据
            CharacterDatabase.Execute("DELETE FROM character_inventory WHERE guid = {}", botGuid);
            CharacterDatabase.Execute("DELETE FROM item_instance WHERE owner_guid = {}", botGuid);

            ++count;
            LOG_INFO("playerbots", "快速组队：服务器启动清理 - 机器人 {} (GUID:{}) 的残留装备已清除。", botName, botGuid);
        } while (res->NextRow());

        LOG_INFO("playerbots", "快速组队：服务器启动清理完成，共清理 {} 个机器人的残留装备。", count);
    }

    // 清理完装备后清空表
    CharacterDatabase.Execute("TRUNCATE TABLE playerbots_fast_group_members");
    LOG_INFO("playerbots", "快速组队：已清空快速组队记录表。");
    // End By leewheel
}
// End By leewheel

void FastGroupMgr::AddPendingSetup(ObjectGuid botGuid, const PendingBotSetup& setup)
{
    m_pendingSetups[botGuid] = setup;
}

bool FastGroupMgr::PopPendingSetup(ObjectGuid botGuid, PendingBotSetup& out)
{
    auto itr = m_pendingSetups.find(botGuid);
    if (itr == m_pendingSetups.end())
        return false;
    out = itr->second;
    m_pendingSetups.erase(itr);
    return true;
}

void FastGroupMgr::ClearPendingSetups(ObjectGuid masterGuid)
{
    for (auto it = m_pendingSetups.begin(); it != m_pendingSetups.end(); )
    {
        if (it->second.masterGuid == masterGuid)
            it = m_pendingSetups.erase(it);
        else
            ++it;
    }
}

void FastGroupMgr::RegisterFastGroupBots(Player* master, const std::vector<ObjectGuid>& botGuids)
{
    m_fastGroupBots[master->GetGUID()] = botGuids;
}

void FastGroupMgr::LogoutFastGroupBots(Player* master)
{
    if (!master)
        return;

    auto itr = m_fastGroupBots.find(master->GetGUID());
    if (itr == m_fastGroupBots.end())
        return;

    PlayerbotMgr* mgr = GET_PLAYERBOT_MGR(master);

    // By leewheel 2026-07-08
    // 获取队伍指针，用于移除不在线的机器人
    // 原因：玩家下线时 OnPlayerbotLogout 先执行 LogoutAllBots 下线了所有机器人，
    //       然后 OnPlayerLogout 中 GetPlayerBot 返回 null，但机器人的 GUID 可能还在队伍中
    //       （raid group 不会自动移除成员），需要手动从队伍中移除
    // By leewheel 2026-07-11 修订：
    //       LogoutFastGroupBots 已移到 OnPlayerBeforeLogout 中执行，
    //       在 LogoutAllBots 之前调用，此时机器人应该还在线。
    //       但保留 else 分支作为兜底，防止其他调用路径下机器人不在线的情况。
    Group* group = master->GetGroup();
    // End By leewheel

    for (ObjectGuid botGuid : itr->second)
    {
        Player* bot = nullptr;
        if (mgr)
            bot = mgr->GetPlayerBot(botGuid);

        if (bot)
        {
            // By leewheel 2026-07-10
            // 快速组队的机器人都是Rndbot，装备是系统临时分配的，退队时必须清理
            // 原因：如果不清理，下次快速组队可能选到穿着旧等级装备的机器人
            // Altbot不会出现在快速组队的内存列表中，无需额外判断
            PlayerbotFactory clearFactory(bot, bot->GetLevel(), 0);
            clearFactory.ClearAllItems();

            bot->SaveToDB(false, false);

            LOG_INFO("playerbots", "快速组队：玩家 {} 离队/下线，机器人 {} 正在下线（已清除装备）。",
                master->GetName(), bot->GetName());

            if (mgr)
                mgr->LogoutPlayerBot(botGuid);
            // End By leewheel
        }
        // By leewheel 2026-07-08
        // 机器人不在线（可能已被 LogoutAllBots 下线），从队伍中移除 GUID
        // By leewheel 2026-07-11 修订：
        // 增加数据库清理装备逻辑作为兜底
        // 原因：如果机器人已被 LogoutAllBots 下线，SaveToDB 已将装备保存到数据库，
        //       必须通过 SQL 直接删除数据库中的装备记录，否则装备会残留
        // By leewheel 2026-07-15 修订：
        // 不再调用 group->RemoveMember，避免在 OnRemoveMember 回调中递归调用
        // RemoveMember 导致 use-after-free 崩溃。
        // 原因：当玩家退组触发 OnRemoveMember → LogoutFastGroupBots 时，
        //   如果对离线机器人调用 group->RemoveMember(botGuid)，
        //   会递归进入 RemoveMember，可能触发 Disband() 销毁 group 对象，
        //   随后 LogoutFastGroupBots 循环继续使用已释放的 group 指针 → 崩溃。
        // 修复：离线机器人不需要主动从队伍中移除，队伍的 Disband() 或
        //   RemoveMember 流程会自然清理所有成员。
        //   只清理数据库中的装备记录即可。
        else if (group)
        {
            // 直接在数据库中清空该机器人的装备
            uint32 botGuidRaw = botGuid.GetCounter();
            CharacterDatabase.Execute("DELETE FROM character_inventory WHERE guid = {}", botGuidRaw);
            CharacterDatabase.Execute("DELETE FROM item_instance WHERE owner_guid = {}", botGuidRaw);

            LOG_INFO("playerbots", "快速组队：玩家 {} 离队/下线，不在线的机器人 GUID {} 已清理数据库装备。",
                master->GetName(), botGuid.ToString());
        }
        // End By leewheel

        // By leewheel 2026-07-10 - 从数据库删除记录
        sFastGroupMgr.DbRemoveFastGroupMember(botGuid.GetCounter());
        // End By leewheel
    }

    m_fastGroupBots.erase(itr);

    // By leewheel 2026-07-08 - 清除角色分配标记
    m_rolesAssigned.erase(master->GetGUID());
    // End By leewheel

    // By leewheel 2026-07-10 - 确保数据库中该玩家的所有快速组队记录都已清除
    sFastGroupMgr.DbRemoveAllByMaster(master->GetGUID().GetCounter());
    // End By leewheel

    // By leewheel 2026-07-11
    // 解散只剩主控玩家的空队伍
    // 原因：快速组队的机器人都被移除后，如果队伍中只剩主控玩家自己，
    //       这个空队伍没有意义，应该解散。
    //       否则玩家再次上线时会发现自己在一个空队伍中，影响后续操作。
    if (group)
    {
        // 重新获取队伍指针（前面的 RemoveMember 可能已经修改了队伍状态）
        Group* currentGroup = master->GetGroup();
        if (currentGroup)
        {
            // 统计队伍中在线成员数量
            uint32 onlineCount = 0;
            for (GroupReference* itr2 = currentGroup->GetFirstMember(); itr2 != nullptr; itr2 = itr2->next())
            {
                Player* member = itr2->GetSource();
                if (member && member->GetSession() && !member->GetSession()->isLogingOut())
                    ++onlineCount;
            }

            // 如果只有主控玩家自己在线，离开队伍
            if (onlineCount <= 1)
            {
                currentGroup->RemoveMember(master->GetGUID(), GROUP_REMOVEMETHOD_LEAVE);
                LOG_INFO("playerbots", "快速组队：玩家 {} 的快速组队机器人已全部下线，空队伍已解散。",
                    master->GetName());
            }
        }
    }
    // End By leewheel
}

// By leewheel 2026-07-07 - 添加 HasFastGroupBots 方法，供 AutoJoinRaid 使用
bool FastGroupMgr::HasFastGroupBots(ObjectGuid masterGuid)
{
    return m_fastGroupBots.find(masterGuid) != m_fastGroupBots.end();
}

// By leewheel 2026-07-07 - 获取已注册的机器人GUID列表，供 AutoJoinRaid 检查上线状态后执行传送
std::vector<ObjectGuid> FastGroupMgr::GetFastGroupBotGuids(ObjectGuid masterGuid)
{
    auto itr = m_fastGroupBots.find(masterGuid);
    if (itr == m_fastGroupBots.end())
        return {};
    return itr->second;
}
// End By leewheel

// By leewheel 2026-07-08 - LFG角色分配跟踪
bool FastGroupMgr::HasRolesAssigned(ObjectGuid masterGuid)
{
    return m_rolesAssigned.find(masterGuid) != m_rolesAssigned.end();
}

void FastGroupMgr::SetRolesAssigned(ObjectGuid masterGuid, bool assigned)
{
    if (assigned)
        m_rolesAssigned.insert(masterGuid);
    else
        m_rolesAssigned.erase(masterGuid);
}
// End By leewheel

// ============================================================
//  辅助函数实现：基于天赋页判断角色定位
//  说明：不依赖 PlayerbotAI 对象，对真实玩家和机器人都适用
// ============================================================

// 获取玩家的天赋页索引（0/1/2），基于天赋点数最多的那一页
uint8 GetSpecTab(Player* player)
{
    if (!player || player->GetLevel() < 10)
        return 0;
    return AiFactory::GetPlayerSpecTab(player);
}

// 判断是否为坦克（纯天赋页判断，不依赖形态/光环）
bool IsTankBySpec(Player* player)
{
    if (!player)
        return false;
    uint8 tab = GetSpecTab(player);
    switch (player->getClass())
    {
        case CLASS_WARRIOR:      return tab == WARRIOR_TAB_PROTECTION;
        case CLASS_PALADIN:      return tab == PALADIN_TAB_PROTECTION;
        case CLASS_DEATH_KNIGHT: return tab == DEATH_KNIGHT_TAB_BLOOD;
        case CLASS_DRUID:        return tab == DRUID_TAB_FERAL;
        default:                 return false;
    }
}

// 判断是否为治疗（纯天赋页判断）
bool IsHealBySpec(Player* player)
{
    if (!player)
        return false;
    uint8 tab = GetSpecTab(player);
    switch (player->getClass())
    {
        case CLASS_PRIEST:  return tab == PRIEST_TAB_DISCIPLINE || tab == PRIEST_TAB_HOLY;
        case CLASS_PALADIN: return tab == PALADIN_TAB_HOLY;
        case CLASS_SHAMAN:  return tab == SHAMAN_TAB_RESTORATION;
        case CLASS_DRUID:   return tab == DRUID_TAB_RESTORATION;
        default:            return false;
    }
}

// 获取玩家角色定位
FastGroupRole GetPlayerRole(Player* player)
{
    if (IsTankBySpec(player))
        return FG_ROLE_TANK;
    if (IsHealBySpec(player))
        return FG_ROLE_HEAL;
    return FG_ROLE_DPS;
}

// 获取角色中文名
const char* GetRoleNameCN(FastGroupRole role)
{
    switch (role)
    {
        case FG_ROLE_TANK: return "坦克";
        case FG_ROLE_HEAL: return "治疗";
        case FG_ROLE_DPS:  return "输出";
        default:           return "未知";
    }
}

// 获取职业中文名
const char* GetClassNameCN(uint8 playerClass)
{
    switch (playerClass)
    {
        case CLASS_WARRIOR:      return "战士";
        case CLASS_PALADIN:      return "圣骑士";
        case CLASS_HUNTER:       return "猎人";
        case CLASS_ROGUE:        return "盗贼";
        case CLASS_PRIEST:       return "牧师";
        case CLASS_DEATH_KNIGHT: return "死亡骑士";
        case CLASS_SHAMAN:       return "萨满";
        case CLASS_MAGE:         return "法师";
        case CLASS_WARLOCK:      return "术士";
        case CLASS_DRUID:        return "德鲁伊";
        default:                 return "未知";
    }
}

// ============================================================
//  按天赋页索引直接分配天赋点（不依赖预设天赋链接）
//  作者: leewheel 2026-07-07
//  说明：InitTalentsBySpecNo 依赖 parsedSpecLinkOrder 预设天赋链接，
//        如果配置中没有则什么都不做。本函数直接按天赋页 tabpage 分配天赋点，
//        逻辑与 PlayerbotFactory::InitTalents 一致。
// ============================================================
void InitTalentsByTab(Player* player, uint8 specTab)
{
    if (!player || player->GetLevel() < 10)
        return;

    // 先重置所有天赋
    player->resetTalents(true);

    //By leewheel 2026-07-22
    // 修复：死亡骑士在Ebon Hold地图(609)登录时，CalculateTalentsPoints()只给 level-55 点天赋
    // （该逻辑是为DK新手村逐步升级设计的），快速组队直接拉到目标等级时天赋点严重不足
    // （例如60级只有5点，应为51点）。强制修正为标准的 level-9 天赋点数。
    uint32 expectedPoints = player->GetLevel() < 10 ? 0 : player->GetLevel() - 9;
    if (player->GetFreeTalentPoints() < expectedPoints)
        player->SetFreeTalentPoints(expectedPoints);
    //End By leewheel

    uint32 classMask = player->getClassMask();

    // 按行收集指定天赋页的所有天赋
    std::map<uint32, std::vector<TalentEntry const*>> spells;
    for (uint32 i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        TalentEntry const* talentInfo = sTalentStore.LookupEntry(i);
        if (!talentInfo)
            continue;

        TalentTabEntry const* talentTabInfo = sTalentTabStore.LookupEntry(talentInfo->TalentTab);
        if (!talentTabInfo || talentTabInfo->tabpage != specTab)
            continue;

        if ((classMask & talentTabInfo->ClassMask) == 0)
            continue;

        spells[talentInfo->Row].push_back(talentInfo);
    }

    // 逐行随机分配天赋点
    for (auto i = spells.begin(); i != spells.end(); ++i)
    {
        std::vector<TalentEntry const*>& spells_row = i->second;
        if (spells_row.empty())
            continue;

        int attemptCount = 0;
        while (!spells_row.empty() && (int)player->GetFreeTalentPoints() > 0 && attemptCount++ < 3)
        {
            int index = urand(0, spells_row.size() - 1);
            TalentEntry const* talentInfo = spells_row[index];
            int maxRank = 0;
            for (uint32 rank = 0; rank < std::min((uint32)MAX_TALENT_RANK, player->GetFreeTalentPoints()); ++rank)
            {
                uint32 spellId = talentInfo->RankID[rank];
                if (!spellId)
                    continue;
                maxRank = rank;
            }
            if (talentInfo->DependsOn)
            {
                player->LearnTalent(talentInfo->DependsOn,
                                    std::min(talentInfo->DependsOnRank, player->GetFreeTalentPoints() - 1));
            }
            player->LearnTalent(talentInfo->TalentID, maxRank);
            spells_row.erase(spells_row.begin() + index);
        }
    }

    player->SendTalentsInfoData(false);
}

// ============================================================
//  从数据库中查找符合职业和阵营要求的离线机器人
//  修复说明（By leewheel 2026-07-07）：
//    1. 增加全局 usedGuids 参数，防止同一角色被分配到不同角色定位
//    2. 一次性查询所有可用职业，按职业分组后轮流取用，确保职业均匀分布
//    3. 增加全局 usedClasses 参数，跨角色定位去重职业
//       5人队每种职业最多1人，10人团以此类推
//       当需求超过可用职业种类数时，才允许同职业重复
// ============================================================
std::vector<BotCandidate> FindOfflineBotsForRole(
    FastGroupRole role,
    uint8 teamId,
    uint32 neededCount,
    uint32 masterLevel,
    std::unordered_set<uint32>& usedGuids,
    std::set<uint8>& usedClasses)
{
    std::vector<BotCandidate> candidates;

    // 收集该角色定位下所有不同的职业（去重），并记录每个职业对应的天赋页
    std::map<uint8, int> classToSpecTab;
    for (size_t i = 0; i < ClassRoleTableSize; ++i)
    {
        auto const& entry = ClassRoleTable[i];
        if (entry.role != role)
            continue;

        // 死骑需要主控玩家等级 >= 55
        if (entry.playerClass == CLASS_DEATH_KNIGHT && masterLevel < 55)
            continue;

        // 同一职业只取第一个匹配项的天赋页
        if (classToSpecTab.find(entry.playerClass) == classToSpecTab.end())
            classToSpecTab[entry.playerClass] = entry.specTab;
    }

    if (classToSpecTab.empty())
        return candidates;

    // 构建阵营种族条件
    std::string raceCondition;
    if (teamId == TEAM_ALLIANCE)
    {
        raceCondition = "race IN (1, 3, 4, 7, 11)";
    }
    else
    {
        raceCondition = "race IN (2, 5, 6, 8, 10)";
    }

    // 构建职业 IN 条件
    std::string classList;
    bool first = true;
    for (auto const& [cls, tab] : classToSpecTab)
    {
        if (!first)
            classList += ",";
        classList += std::to_string(cls);
        first = false;
    }

    // By leewheel 2026-07-10
    // 只从随机机器人账号中筛选，避免选中玩家手动创建的Altbot角色
    // Altbot应由玩家手动添加（addbot），不应被快速组队系统自动选中并修改
    //
    // By leewheel 2026-07-11 修复：
    // 原代码 INNER JOIN account a ON c.account = a.id 试图在角色数据库中JOIN account表，
    // 但 account 表在登录数据库(acore_auth)中，不在角色数据库(acore_characters)中，
    // 导致SQL报错 "Table 'acore_characters.account' doesn't exist"，查询返回空结果。
    // 修复方案：使用 sPlayerbotAIConfig.randomBotAccounts 内存列表（服务器启动时已加载）
    //          构建账号ID的 IN 条件，替代跨库JOIN。
    std::string accountCondition;
    if (sPlayerbotAIConfig.randomBotAccounts.empty())
    {
        // 兜底：如果内存列表为空，直接返回（不应该发生）
        LOG_ERROR("playerbots", "快速组队：randomBotAccounts 列表为空，无法查找离线机器人。");
        return candidates;
    }

    bool acctFirst = true;
    for (uint32 acctId : sPlayerbotAIConfig.randomBotAccounts)
    {
        if (!acctFirst)
            accountCondition += ",";
        accountCondition += std::to_string(acctId);
        acctFirst = false;
    }

    QueryResult results = CharacterDatabase.Query(
        "SELECT guid, name, race, account, class FROM characters "
        "WHERE class IN ({}) AND online = 0 AND {} "
        "AND account IN ({}) "
        "ORDER BY RAND()",
        classList, raceCondition, accountCondition);
    // End By leewheel

    if (!results)
        return candidates;

    // 按职业分组（排除已被其他角色定位使用的角色）
    std::map<uint8, std::vector<BotCandidate>> byClass;
    do
    {
        Field* fields = results->Fetch();
        uint32 guidLow = fields[0].Get<uint32>();

        // 跳过已被其他角色定位使用的角色
        if (usedGuids.find(guidLow) != usedGuids.end())
            continue;

        BotCandidate candidate;
        candidate.guid = ObjectGuid(HighGuid::Player, guidLow);
        candidate.name = fields[1].Get<std::string>();
        candidate.race = fields[2].Get<uint8>();
        candidate.accountId = fields[3].Get<uint32>();
        candidate.playerClass = fields[4].Get<uint8>();
        candidate.role = role;
        candidate.specTab = classToSpecTab[candidate.playerClass];

        byClass[candidate.playerClass].push_back(candidate);
    } while (results->NextRow());

    // By leewheel 2026-07-21：随机打乱职业挑选顺序。
    // 原实现遍历 std::map(按职业ID升序)，配合第一轮"取第一个可用职业"的规则，
    // 导致每次选中的职业固定为编号最小的几个职业——解散再组只有名字变、职业完全不变。
    // 这里用 Fisher-Yates 洗牌(复用已有 urand)随机打乱职业顺序，使每次组队职业随机；
    // usedClasses 去重逻辑保持不变(不重复职业、不与玩家重复)。
    std::vector<uint8> classOrder;
    classOrder.reserve(byClass.size());
    for (auto const& [cls, list] : byClass)
        classOrder.push_back(cls);
    for (size_t i = classOrder.size(); i > 1; --i)
    {
        size_t j = urand(0, static_cast<uint32>(i - 1));
        std::swap(classOrder[i - 1], classOrder[j]);
    }
    // End By leewheel

    // 第一轮：优先从尚未被使用的职业中各取1个（确保每种职业最多1人）
    for (uint8 cls : classOrder)
    {
        if (candidates.size() >= neededCount)
            break;
        // 跳过已被其他角色定位使用的职业
        if (usedClasses.find(cls) != usedClasses.end())
            continue;
        auto& list = byClass[cls];
        if (!list.empty())
        {
            candidates.push_back(list.back());
            usedGuids.insert(list.back().guid.GetCounter());
            usedClasses.insert(cls);
            list.pop_back();
        }
    }

    // 第二轮：如果数量不够，轮流从所有职业（含已使用的）中取用
    while (candidates.size() < neededCount)
    {
        bool added = false;
        for (uint8 cls : classOrder)
        {
            if (candidates.size() >= neededCount)
                break;
            auto& list = byClass[cls];
            if (!list.empty())
            {
                candidates.push_back(list.back());
                usedGuids.insert(list.back().guid.GetCounter());
                usedClasses.insert(cls);
                list.pop_back();
                added = true;
            }
        }
        if (!added)
            break;  // 所有职业都取完了
    }

    return candidates;
}

// ============================================================
//  主组队函数
//  By leewheel 2026-07-07：从 ExecuteFastGroup 重命名为 DoFastGroup，
//  ChatHandler 参数改为可选（默认 nullptr），供 AutoJoinRaid 调用。
// ============================================================
bool DoFastGroup(Player* master, FastGroupConfigIndex configIndex, ChatHandler* handler)
{
    if (!master)
        return false;

    const FastGroupConfig& config = FastGroupConfigs[configIndex];

    // 战斗中无法使用
    if (master->IsInCombat())
    {
        if (handler)
            handler->PSendSysMessage("|cffff0000[快速组队] 战斗中无法使用快速组队。|r");
        return false;
    }

    // 如果已有快速组队的机器人在线，先下线它们
    sFastGroupMgr.LogoutFastGroupBots(master);

    // 清除旧的待设置记录
    sFastGroupMgr.ClearPendingSetups(master->GetGUID());

    // 如果玩家已在队伍中，先离开
    if (Group* oldGroup = master->GetGroup())
    {
        oldGroup->RemoveMember(master->GetGUID(), GROUP_REMOVEMETHOD_LEAVE);
    }

    if (handler)
        handler->PSendSysMessage("|cff00ff00[快速组队] 正在组建 {} 人队伍...|r", config.totalMembers);

    // 分析玩家自身的角色定位
    FastGroupRole playerRole = GetPlayerRole(master);
    uint8 teamId = master->GetTeamId(true);
    uint32 targetLevel = master->GetLevel();

    if (handler)
        handler->PSendSysMessage("|cff00ccff[快速组队] 玩家 {} 角色定位：{}，目标等级：{}|r",
            master->GetName(), GetRoleNameCN(playerRole), targetLevel);

    // 计算需要补充的各角色数量（扣除玩家自身）
    uint32 needTanks = config.tanks;
    uint32 needHeals = config.heals;
    uint32 needDps   = config.dps;

    switch (playerRole)
    {
        case FG_ROLE_TANK: needTanks = (needTanks > 0) ? needTanks - 1 : 0; break;
        case FG_ROLE_HEAL: needHeals = (needHeals > 0) ? needHeals - 1 : 0; break;
        case FG_ROLE_DPS:  needDps   = (needDps > 0)   ? needDps - 1   : 0; break;
    }

    uint32 totalBotsNeeded = needTanks + needHeals + needDps;
    if (handler)
        handler->PSendSysMessage("|cff00ccff[快速组队] 需要机器人：{} 坦克 + {} 治疗 + {} 输出 = {} 个|r",
            needTanks, needHeals, needDps, totalBotsNeeded);

    // 招募各角色机器人（使用全局usedGuids防止同一角色被分配到多个定位）
    // By leewheel 2026-07-07：增加 usedClasses 跨定位职业去重，5人队每种职业最多1人
    std::vector<BotCandidate> allBots;
    std::unordered_set<uint32> usedGuids;
    std::set<uint8> usedClasses;

    // 玩家自身的职业也算已使用，避免机器人与玩家同职业
    usedClasses.insert(master->getClass());

    if (needTanks > 0)
    {
        auto tanks = FindOfflineBotsForRole(FG_ROLE_TANK, teamId, needTanks, targetLevel, usedGuids, usedClasses);
        for (auto& t : tanks)
            allBots.push_back(t);
    }

    if (needHeals > 0)
    {
        auto heals = FindOfflineBotsForRole(FG_ROLE_HEAL, teamId, needHeals, targetLevel, usedGuids, usedClasses);
        for (auto& h : heals)
            allBots.push_back(h);
    }

    if (needDps > 0)
    {
        auto dps = FindOfflineBotsForRole(FG_ROLE_DPS, teamId, needDps, targetLevel, usedGuids, usedClasses);
        for (auto& d : dps)
            allBots.push_back(d);
    }

    if (allBots.empty())
    {
        if (handler)
            handler->PSendSysMessage("|cffff0000[快速组队] 没有找到任何可用的离线机器人，组队失败。|r");
        return false;
    }

    if (allBots.size() < totalBotsNeeded)
    {
        if (handler)
            handler->PSendSysMessage("|cffffcc00[快速组队] 警告：可用机器人不足，仅找到 {} 个（需要 {} 个）。|r",
                allBots.size(), totalBotsNeeded);
    }

    // 获取 PlayerbotMgr
    PlayerbotMgr* mgr = GET_PLAYERBOT_MGR(master);
    if (!mgr)
    {
        if (handler)
            handler->PSendSysMessage("|cffff0000[快速组队] 错误：无法获取 PlayerbotMgr。|r");
        return false;
    }

    // 逐个添加机器人（AddPlayerBot 是异步的，实际设置在 OnPlayerLogin 中完成）
    std::vector<ObjectGuid> addedBotGuids;
    uint32 masterAccountId = master->GetSession()->GetAccountId();

    for (auto const& candidate : allBots)
    {
        // 检查是否已在线
        if (ObjectAccessor::FindConnectedPlayer(candidate.guid))
        {
            if (handler)
                handler->PSendSysMessage("|cffffcc00[快速组队] 机器人 {} 已在线，跳过。|r", candidate.name);
            continue;
        }

        if (handler)
            handler->PSendSysMessage("|cff00ccff[快速组队] 正在召唤：{}（{}-{}）...|r",
                candidate.name, GetClassNameCN(candidate.playerClass), GetRoleNameCN(candidate.role));

        // 记录待设置信息，供 OnPlayerLogin 使用
        PendingBotSetup setup;
        setup.targetLevel = targetLevel;
        setup.role = candidate.role;
        setup.specTab = candidate.specTab;
        setup.masterGuid = master->GetGUID();
        setup.botName = candidate.name;
        setup.masterName = master->GetName();
        sFastGroupMgr.AddPendingSetup(candidate.guid, setup);

        // 异步上线机器人
        mgr->AddPlayerBot(candidate.guid, masterAccountId);
        addedBotGuids.push_back(candidate.guid);

        // By leewheel 2026-07-10
        // 向数据库记录快速组队信息，用于退队时区分组队方式
        sFastGroupMgr.DbAddFastGroupMember(
            master->GetGUID().GetCounter(),
            candidate.guid.GetCounter(),
            candidate.name,
            candidate.playerClass,
            FG_TYPE_FAST_GROUP);
        // End By leewheel
    }

    if (addedBotGuids.empty())
    {
        if (handler)
            handler->PSendSysMessage("|cffff0000[快速组队] 没有机器人被成功召唤，组队终止。|r");
        return false;
    }

    // 注册快速组队机器人列表（用于后续离队自动下线）
    sFastGroupMgr.RegisterFastGroupBots(master, addedBotGuids);

    if (handler)
    {
        handler->PSendSysMessage("|cff00ff00[快速组队] 已召唤 {} 个机器人，正在设置等级和装备...|r", addedBotGuids.size());
        handler->PSendSysMessage("|cff00ccff[快速组队] 机器人将自动加入队伍。玩家离队时所有机器人将自动下线。|r");
    }

    LOG_INFO("playerbots", "快速组队：玩家 {} 组建 {} 人队伍，召唤了 {} 个机器人。",
        master->GetName(), config.totalMembers, addedBotGuids.size());

    return true;
}

// ============================================================
//  LFG角色分配函数
//  By leewheel 2026-07-08
//  为队伍中所有成员分配LFG角色（坦克/治疗/输出），
//  并发送 SMSG_LFG_ROLE_CHOSEN 包使客户端显示角色图标。
//  原理：WoW 3.3.5中队伍角色图标通过LFG系统显示，
//  需要设置 sLFGMgr->SetRoles 并广播 SMSG_LFG_ROLE_CHOSEN 包。
// ============================================================
void AssignLfgRoles(Player* master)
{
    if (!master)
        return;

    Group* group = master->GetGroup();
    if (!group)
        return;

    // 收集每个成员的角色
    std::map<ObjectGuid, uint8> roleMap;

    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member)
            continue;

        // 基于天赋页判断角色定位
        FastGroupRole role = GetPlayerRole(member);
        uint8 roleMask = 0;
        switch (role)
        {
            case FG_ROLE_TANK: roleMask = lfg::PLAYER_ROLE_TANK; break;
            case FG_ROLE_HEAL: roleMask = lfg::PLAYER_ROLE_HEALER; break;
            case FG_ROLE_DPS:  roleMask = lfg::PLAYER_ROLE_DAMAGE; break;
            default: break;
        }

        // 队长额外加上队长标记
        if (member->GetGUID() == group->GetLeaderGUID())
            roleMask |= lfg::PLAYER_ROLE_LEADER;

        roleMap[member->GetGUID()] = roleMask;

        // 设置 LFG 角色
        sLFGMgr->SetRoles(member->GetGUID(), roleMask);

        LOG_DEBUG("playerbots", "快速组队：角色分配 - {} -> {}",
            member->GetName(), GetRoleNameCN(role));
    }

    // 向每个成员发送所有成员的角色信息
    // SMSG_LFG_ROLE_CHOSEN 格式: guid(uint64), ready(uint8), roles(uint32)
    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* receiver = itr->GetSource();
        if (!receiver || !receiver->GetSession())
            continue;

        for (auto const& [guid, roles] : roleMap)
        {
            WorldPacket data(SMSG_LFG_ROLE_CHOSEN, 8 + 1 + 4);
            data << guid;
            data << uint8(roles > 0 ? 1 : 0);
            data << uint32(roles);
            receiver->GetSession()->SendPacket(&data);
        }
    }

    LOG_INFO("playerbots", "快速组队：玩家 {} 的队伍已完成LFG角色分配（{}人）。",
        master->GetName(), roleMap.size());
}
// End By leewheel

// By leewheel 2026-07-18
// 确保机器人的所有技能达到当前等级对应的最大值
// 包括：武器技能、防御技能、开锁技能、专业制作技能等
// 修复问题：快速组队的机器人可能技能值很低（如开锁1/350），
//          导致副本内无法开箱、专业技能制作失败等问题
// 原理：
//   - 等级依赖技能（武器、防御、开锁）：maxValue = level * 5
//   - 专业制作技能（锻造、炼金等）：maxValue 由专业技能等级上限决定
//   - 通过 GetPureMaxSkillValue() 获取每个技能的正确 max 值
//   - 然后用 SetSkill() 把 value 设为 max
void EnsureBotSkillsMaximized(Player* bot)
{
    if (!bot)
        return;

    // 1. 先调用 UpdateSkillsForLevel 更新等级依赖技能的上限
    //    这会把武器技能、防御技能的 max 更新为 level * 5
    bot->UpdateSkillsForLevel();

    // 2. 遍历所有已有技能，把 value 设为 max
    //    使用 UpdateSkillsToMaxSkillsForLevel 会跳过专业和骑术技能
    //    所以我们手动遍历，确保所有技能（包括专业制作技能）都满
    //    注意：SKILL_NONE(0) 到 SKILL_MAX 范围内遍历
    for (uint32 skillId = 0; skillId < MAX_SKILL_TYPE; ++skillId)
    {
        // 检查机器人是否已有这个技能
        if (!bot->HasSkill(skillId))
            continue;

        // 获取当前技能的 max 值
        uint16 maxVal = bot->GetPureMaxSkillValue(skillId);

        // 跳过 max <= 1 的技能（如骑术、双持等特殊技能）
        if (maxVal <= 1)
            continue;

        // 获取当前技能值
        uint16 currVal = bot->GetPureSkillValue(skillId);

        // 如果技能值已满，跳过
        if (currVal >= maxVal)
            continue;

        // 获取技能 step（用于专业技能的等级阶段）
        uint16 step = bot->GetSkillStep(skillId);
        if (step == 0)
            step = 1;

        // 设置技能值为 max
        bot->SetSkill(skillId, step, maxVal, maxVal);

        LOG_DEBUG("playerbots", "技能补满：机器人 {} 的技能(ID:{}) 从 {} 提升到 {}。",
            bot->GetName(), skillId, currVal, maxVal);
    }

    // 3. 额外确保盗贼的开锁技能满级
    //    开锁技能（SKILL_LOCKPICKING）是盗贼专属技能
    //    必须达到 level * 5，否则副本内的锁箱无法打开
    if (bot->getClass() == CLASS_ROGUE && bot->HasSkill(SKILL_LOCKPICKING))
    {
        uint16 maxLockpicking = bot->GetLevel() * 5;
        uint16 currLockpicking = bot->GetPureSkillValue(SKILL_LOCKPICKING);
        if (currLockpicking < maxLockpicking)
        {
            bot->SetSkill(SKILL_LOCKPICKING, 1, maxLockpicking, maxLockpicking);
            LOG_INFO("playerbots", "技能补满：机器人 {}(盗贼) 的开锁技能从 {} 提升到 {}。",
                bot->GetName(), currLockpicking, maxLockpicking);
        }
    }
}
// End By leewheel

// By leewheel 2026-07-17
// 检查并学习坐骑技能
// 快速组队的机器人在上线时自动学习坐骑技能：
// - 到达地面坐骑等级(默认20级)时，如果没有地面坐骑，随机学一个地面坐骑
// - 到达飞行坐骑等级(默认60级)时，如果没有飞行坐骑，随机学一个飞行坐骑
// 坐骑技能从1700001-1700412范围内随机选取
// 注意：每次快速组队都要检查，确保机器人一定会坐骑
//      大于等于飞行坐骑等级后必须学会一个地面坐骑和一个飞行坐骑
void EnsureBotHasMounts(Player* bot)
{
    if (!bot)
        return;

    uint32 botLevel = bot->GetLevel();
    uint32 groundMountMinLevel = sPlayerbotAIConfig.useGroundMountAtMinLevel;
    uint32 flyMountMinLevel = sPlayerbotAIConfig.useFlyMountAtMinLevel;

    // By leewheel 2026-07-18
    // 重写：改用硬编码数组替代数据库查询，不再依赖 spell_dbc 表和静态缓存
    // 原因：
    //   1. 数据库查询+静态缓存方式存在隐患（首次查询可能失败、缓存可能为空）
    //   2. 上一轮添加的 removeSpell+learnSpell 骑术法术逻辑破坏了 InitSkills
    //      原本正常工作的骑术技能（removeSpell 会级联移除法术链中的法术），
    //      导致飞行坐骑全部不能用。现已删除该逻辑，骑术技能完全交给
    //      InitSkills 处理（InitSkills 中的 SetSkill+learnSpell 是正常工作的）。
    //   3. 本函数只负责：补充缺失的骑术法术（不删除已有法术）+ 学习坐骑法术
    // 地面坐骑：EffectAura_2=32(SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED)
    // 飞行坐骑：EffectAura_2=207(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED)
    static const uint32 groundMountSpells[] = {
        1700012, 1700013, 1700014, 1700015, 1700016, 1700024, 1700025, 1700026,
        1700027, 1700028, 1700029, 1700033, 1700038, 1700039, 1700040, 1700041,
        1700042, 1700043, 1700044, 1700045, 1700046, 1700047, 1700049, 1700054,
        1700056, 1700057, 1700058, 1700060, 1700069, 1700070, 1700071, 1700073,
        1700074, 1700075, 1700076, 1700077, 1700078, 1700079, 1700080, 1700081,
        1700082, 1700083, 1700084, 1700085, 1700086, 1700087, 1700088, 1700089,
        1700095, 1700096, 1700097, 1700098, 1700099, 1700100, 1700101, 1700103,
        1700104, 1700105, 1700106, 1700107, 1700109, 1700110, 1700111, 1700113,
        1700114, 1700115, 1700116, 1700128, 1700129, 1700130, 1700135, 1700136,
        1700137, 1700138, 1700140, 1700141, 1700142, 1700147, 1700148, 1700149,
        1700150, 1700151, 1700152, 1700153, 1700154, 1700155, 1700156, 1700163,
        1700165, 1700166, 1700167, 1700168, 1700169, 1700170, 1700171, 1700172,
        1700173, 1700174, 1700175, 1700179, 1700184, 1700197, 1700198, 1700199,
        1700200, 1700201, 1700202, 1700203, 1700204, 1700205, 1700206, 1700207,
        1700208, 1700217, 1700218, 1700219, 1700220, 1700226, 1700227, 1700228,
        1700229, 1700230, 1700231, 1700232, 1700235, 1700236, 1700237, 1700238,
        1700239, 1700240, 1700241, 1700242, 1700243, 1700244, 1700245, 1700246,
        1700247, 1700248, 1700249, 1700250, 1700251, 1700252, 1700253, 1700254,
        1700255, 1700256, 1700257, 1700258, 1700260, 1700261, 1700262, 1700263,
        1700264, 1700265, 1700266, 1700267, 1700268, 1700269, 1700270, 1700271,
        1700272, 1700273, 1700274, 1700275, 1700276, 1700280, 1700281, 1700282,
        1700287, 1700288, 1700289, 1700290, 1700291, 1700292, 1700293, 1700294,
        1700307, 1700308, 1700318, 1700319, 1700320, 1700321, 1700323, 1700326,
        1700328, 1700329, 1700331, 1700332, 1700333, 1700337, 1700338, 1700339,
        1700340, 1700341, 1700342, 1700344, 1700345, 1700348, 1700349, 1700359,
        1700360, 1700361, 1700362, 1700363, 1700364, 1700365, 1700366, 1700367,
        1700368, 1700369, 1700370, 1700371, 1700372, 1700373, 1700374, 1700375,
        1700376, 1700377, 1700378, 1700379, 1700380, 1700381, 1700387, 1700388,
        1700389, 1700390, 1700391, 1700392, 1700393, 1700394, 1700395, 1700396,
        1700397, 1700398, 1700399, 1700400, 1700401
    };
    static const uint32 flightMountSpells[] = {
        1700001, 1700002, 1700003, 1700004, 1700005, 1700006, 1700007, 1700008,
        1700009, 1700010, 1700011, 1700017, 1700018, 1700019, 1700020, 1700021,
        1700022, 1700023, 1700030, 1700031, 1700032, 1700034, 1700035, 1700036,
        1700037, 1700048, 1700050, 1700051, 1700052, 1700053, 1700055, 1700059,
        1700061, 1700062, 1700063, 1700064, 1700065, 1700066, 1700067, 1700068,
        1700072, 1700090, 1700091, 1700092, 1700093, 1700094, 1700102, 1700108,
        1700112, 1700117, 1700118, 1700119, 1700120, 1700121, 1700122, 1700123,
        1700124, 1700125, 1700126, 1700127, 1700131, 1700132, 1700133, 1700134,
        1700139, 1700143, 1700144, 1700145, 1700146, 1700157, 1700158, 1700159,
        1700160, 1700161, 1700162, 1700164, 1700176, 1700177, 1700178, 1700180,
        1700181, 1700182, 1700183, 1700185, 1700186, 1700187, 1700188, 1700189,
        1700190, 1700192, 1700193, 1700194, 1700195, 1700196, 1700209, 1700210,
        1700211, 1700212, 1700213, 1700214, 1700215, 1700216, 1700221, 1700222,
        1700223, 1700224, 1700225, 1700233, 1700234, 1700259, 1700277, 1700278,
        1700279, 1700283, 1700284, 1700285, 1700286, 1700295, 1700296, 1700297,
        1700298, 1700299, 1700300, 1700301, 1700302, 1700303, 1700304, 1700305,
        1700306, 1700309, 1700310, 1700311, 1700312, 1700313, 1700314, 1700315,
        1700316, 1700317, 1700322, 1700324, 1700325, 1700327, 1700330, 1700334,
        1700335, 1700336, 1700343, 1700346, 1700347, 1700350, 1700351, 1700352,
        1700353, 1700354, 1700355, 1700356, 1700357, 1700358, 1700382, 1700383,
        1700384, 1700385, 1700386, 1700402, 1700403, 1700404, 1700405, 1700406,
        1700407, 1700408, 1700409, 1700410, 1700411, 1700412
    };
    static const uint32 groundMountCount = sizeof(groundMountSpells) / sizeof(groundMountSpells[0]);
    static const uint32 flightMountCount = sizeof(flightMountSpells) / sizeof(flightMountSpells[0]);

    // 第一步：确保骑术法术被学习（只补充缺失的，不删除已有法术）
    // 骑术法术由 InitSkills 负责主要学习，此处仅做补充兜底
    // 注意：绝不使用 removeSpell，避免破坏 InitSkills 已正确设置的骑术技能
    if (botLevel >= groundMountMinLevel && !bot->HasSpell(33388))
        bot->learnSpell(33388);  // Apprentice Riding (地面骑术75)
    if (botLevel >= sPlayerbotAIConfig.useFastGroundMountAtMinLevel && !bot->HasSpell(33391))
        bot->learnSpell(33391);  // Journeyman Riding (地面骑术150)
    if (botLevel >= flyMountMinLevel && !bot->HasSpell(34090))
        bot->learnSpell(34090);  // Expert Riding (飞行骑术225)
    if (botLevel >= sPlayerbotAIConfig.useFastFlyMountAtMinLevel && !bot->HasSpell(34091))
        bot->learnSpell(34091);  // Artisan Riding (飞行骑术300)
    if (botLevel >= 77 && !bot->HasSpell(54197))
        bot->learnSpell(54197);  // Cold Weather Flying (寒冷天气飞行)

    // 第二步：检查机器人是否已拥有地面坐骑和飞行坐骑法术
    bool hasGroundMount = false;
    bool hasFlightMount = false;

    for (auto const& [spellId, spellPtr] : bot->GetSpellMap())
    {
        if (spellPtr->State == PLAYERSPELL_REMOVED || !spellPtr->Active)
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo || spellInfo->IsPassive())
            continue;

        // 检查是否为坐骑法术(EffectAura_1 == SPELL_AURA_MOUNTED = 78)
        if (spellInfo->Effects[0].ApplyAuraName != SPELL_AURA_MOUNTED)
            continue;

        // 检查飞行坐骑时必须同时检查 Effects[1] 和 Effects[2]
        if (spellInfo->Effects[1].ApplyAuraName == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED ||
            spellInfo->Effects[2].ApplyAuraName == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED ||
            spellInfo->Id == 54729)  // Winged Steed of the Ebon Blade 特殊处理
            hasFlightMount = true;
        else
            hasGroundMount = true;
    }

    // 第三步：随机学习一个地面坐骑（如果没有的话）
    // 学习失败就重新再学，直到学会
    if (botLevel >= groundMountMinLevel && !hasGroundMount && groundMountCount > 0)
    {
        for (uint32 attempt = 0; attempt < 10; ++attempt)
        {
            uint32 index = urand(0, groundMountCount - 1);
            uint32 spell = groundMountSpells[index];
            if (bot->HasSpell(spell))
                break;
            bot->learnSpell(spell);
            if (bot->HasSpell(spell))
            {
                LOG_INFO("playerbots", "快速组队：机器人 {} 已学习地面坐骑技能 {}。", bot->GetName(), spell);
                break;
            }
        }
    }

    // 第四步：随机学习一个飞行坐骑（如果没有的话）
    // 学习失败就重新再学，直到学会
    if (botLevel >= flyMountMinLevel && !hasFlightMount && flightMountCount > 0)
    {
        for (uint32 attempt = 0; attempt < 10; ++attempt)
        {
            uint32 index = urand(0, flightMountCount - 1);
            uint32 spell = flightMountSpells[index];
            if (bot->HasSpell(spell))
                break;
            bot->learnSpell(spell);
            if (bot->HasSpell(spell))
            {
                LOG_INFO("playerbots", "快速组队：机器人 {} 已学习飞行坐骑技能 {}。", bot->GetName(), spell);
                break;
            }
        }
    }
}
// End By leewheel

static bool HandleFastGroupParty5Command(ChatHandler* handler)
{
    Player* master = handler->GetSession()->GetPlayer();
    if (!master)
        return false;
    return DoFastGroup(master, FG_CONFIG_PARTY_5, handler);
}

static bool HandleFastGroupRaid10Command(ChatHandler* handler)
{
    Player* master = handler->GetSession()->GetPlayer();
    if (!master)
        return false;
    return DoFastGroup(master, FG_CONFIG_RAID_10, handler);
}

static bool HandleFastGroupRaid25Command(ChatHandler* handler)
{
    Player* master = handler->GetSession()->GetPlayer();
    if (!master)
        return false;
    return DoFastGroup(master, FG_CONFIG_RAID_25, handler);
}

static bool HandleFastGroupRaid40Command(ChatHandler* handler)
{
    Player* master = handler->GetSession()->GetPlayer();
    if (!master)
        return false;
    return DoFastGroup(master, FG_CONFIG_RAID_40, handler);
}

static bool HandleFastGroupDisbandCommand(ChatHandler* handler)
{
    Player* master = handler->GetSession()->GetPlayer();
    if (!master)
        return false;
    sFastGroupMgr.LogoutFastGroupBots(master);
    handler->PSendSysMessage("|cff00ff00[快速组队] 所有快速组队机器人已下线。|r");
    return true;
}

// ============================================================
//  命令脚本注册
// ============================================================
class fastgroup_commandscript : public CommandScript
{
public:
    fastgroup_commandscript() : CommandScript("fastgroup_commandscript") {}

    // By leewheel 20260714 添加 .重置全体天赋 命令
    // 用于重置所有在线机器人的天赋，使天赋配置变更后立即生效
    // 普通玩家也可用，挑战模式下也可用
    static bool HandleResetAllBotTalentsCommand(ChatHandler* handler)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
            return false;

        PlayerBotMap allBots = sRandomPlayerbotMgr.GetAllBots();
        if (allBots.empty())
        {
            handler->PSendSysMessage("|cffffcc00[重置天赋]|r 当前没有任何在线的机器人，无需重置。");
            return true;
        }

        handler->PSendSysMessage("|cff00ff00[重置天赋]|r 正在重置 {} 个在线机器人的天赋，请稍候...", allBots.size());

        uint32 successCount = 0;
        uint32 failCount = 0;

        for (auto const& [guid, bot] : allBots)
        {
            if (!bot || !bot->IsInWorld())
            {
                ++failCount;
                continue;
            }

            // 先重置天赋点
            bot->resetTalents(true);

            // 使用PlayerbotFactory重新初始化天赋树
            PlayerbotFactory factory(bot, bot->GetLevel());
            factory.InitTalentsTree(true, false, false);

            // 重置AI策略以适应新天赋
            PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
            if (botAI)
                botAI->ResetStrategies(false);

            ++successCount;
        }

        handler->PSendSysMessage("|cff00ff00[重置天赋]|r 已重置 {} 个在线机器人的天赋（失败 {} 个）。", successCount, failCount);
        LOG_INFO("playerbots", "玩家 {} 执行了 .重置全体天赋 命令，成功重置 {} 个机器人天赋，失败 {} 个",
            player->GetName(), successCount, failCount);
        return true;
    }
    // End By leewheel

    // By leewheel 20260714 添加 .机器人天赋 命令
    // 列出在线机器人的天赋种类统计（坦克/治疗/输出）
    static bool HandleBotTalentListCommand(ChatHandler* handler)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
            return false;

        PlayerBotMap allBots = sRandomPlayerbotMgr.GetAllBots();
        if (allBots.empty())
        {
            handler->PSendSysMessage("|cffffcc00[机器人天赋]|r 当前没有任何在线的机器人。");
            return true;
        }

        uint32 tankCount = 0;
        uint32 healCount = 0;
        uint32 dpsCount = 0;
        uint32 totalCount = 0;

        for (auto const& [guid, bot] : allBots)
        {
            if (!bot || !bot->IsInWorld())
                continue;

            ++totalCount;
            FastGroupRole role = GetPlayerRole(bot);
            switch (role)
            {
                case FG_ROLE_TANK: ++tankCount; break;
                case FG_ROLE_HEAL: ++healCount; break;
                case FG_ROLE_DPS:  ++dpsCount;  break;
                default: break;
            }
        }

        handler->PSendSysMessage("|cff00ff00[机器人天赋]|r 在线 {} 个机器人，坦克 {} 个，治疗 {} 个，输出 {} 个。",
            totalCount, tankCount, healCount, dpsCount);
        return true;
    }
    // End By leewheel

    ChatCommandTable GetCommands() const override
    {
        // By leewheel 20260713: 将权限从RBAC_PERM_COMMAND_RELOAD改为SEC_PLAYER，使普通玩家也可使用快速组队命令
        static ChatCommandTable party5Table  = {{ "", HandleFastGroupParty5Command,  SEC_PLAYER, Console::No }};
        static ChatCommandTable raid10Table  = {{ "", HandleFastGroupRaid10Command,  SEC_PLAYER, Console::No }};
        static ChatCommandTable raid25Table  = {{ "", HandleFastGroupRaid25Command,  SEC_PLAYER, Console::No }};
        static ChatCommandTable raid40Table  = {{ "", HandleFastGroupRaid40Command,  SEC_PLAYER, Console::No }};
        static ChatCommandTable disbandTable = {{ "", HandleFastGroupDisbandCommand, SEC_PLAYER, Console::No }};
        // End By leewheel

        static ChatCommandTable commandTable = {
            { "5人队",        party5Table  },
            { "10人团",       raid10Table  },
            { "25人团",       raid25Table  },
            { "40人团",       raid40Table  },
            { "解散快速组队", disbandTable },
            // By leewheel 20260714 添加重置全体天赋命令
            { "重置全体天赋", HandleResetAllBotTalentsCommand, SEC_PLAYER, Console::No },
            // End By leewheel
            // By leewheel 20260714 添加机器人天赋统计命令
            { "机器人天赋", HandleBotTalentListCommand, SEC_PLAYER, Console::No },
            // End By leewheel
        };

        return commandTable;
    }
};

// ============================================================
//  PlayerScript：机器人上线时设置等级/天赋/装备，玩家下线时清理
// ============================================================
class FastGroupPlayerScript : public PlayerScript
{
public:
    FastGroupPlayerScript() : PlayerScript("FastGroupPlayerScript", {
        PLAYERHOOK_ON_LOGIN,
        PLAYERHOOK_ON_BEFORE_LOGOUT,
        PLAYERHOOK_ON_LOGOUT,
        PLAYERHOOK_ON_AFTER_UPDATE
    }) {}

    // By leewheel 2026-07-11
    // 玩家下线前处理：在 LogoutAllBots 之前执行
    // 原因：WorldSession::LogoutPlayer 中 OnPlayerbotLogout(LogoutAllBots) 在第658行执行，
    //       OnPlayerLogout 在第789行执行。如果放在 OnPlayerLogout 中，
    //       机器人已被 LogoutAllBots 下线，GetPlayerBot 返回 null，无法清理装备。
    //       移到 OnPlayerBeforeLogout（第653行）可以在 LogoutAllBots 之前执行，
    //       此时机器人还在线，可以正常清理装备并下线。
    void OnPlayerBeforeLogout(Player* player) override
    {
        if (!player)
            return;

        // 在 LogoutAllBots 之前下线快速组队机器人并清理装备
        sFastGroupMgr.LogoutFastGroupBots(player);
    }
    // End By leewheel

    // 机器人上线时设置等级、天赋、装备
    void OnPlayerLogin(Player* player) override
    {
        if (!player)
            return;

        // By leewheel 2026-07-11
        // 主控玩家上线时的残留队伍和数据库记录检查
        // 原因：如果服务器崩溃或上次下线异常，可能残留快速组队记录和空队伍
        if (!GET_PLAYERBOT_AI(player))
        {
            CleanupMasterLogin(player);
        }
        // End By leewheel

        // 检查是否是快速组队待设置的机器人
        PendingBotSetup setup;
        if (!sFastGroupMgr.PopPendingSetup(player->GetGUID(), setup))
            return;

        // By leewheel 2026-07-10
        // Altbot（玩家手动创建的小号机器人）不执行任何设置
        // Altbot保留玩家手动设置的等级、天赋、装备，不做任何修改
        // 原因：Altbot是玩家自己练的小号，已有自己的装备和天赋，不应被快速组队系统覆盖
        //
        // By leewheel 2026-07-11 修复：
        // 原代码使用 sRandomPlayerbotMgr.IsRandomBot(player) 判断是否为Altbot，
        // 但 IsRandomBot 有两个条件：1) 账号在randomBotAccounts中 2) 角色在currentBots列表中。
        // FastGroup 通过 SQL 从所有随机账号的离线角色中选取，很多角色不在 currentBots 中，
        // 导致 IsRandomBot 返回 false，被误判为 Altbot，跳过了全部装备设置逻辑，
        // 机器人裸体进副本！
        // 修复：只检查账号是否在随机账号列表中，与代码库中其他地方区分Altbot的方式一致。
        uint32 botAccountId = player->GetSession()->GetAccountId();
        if (!sPlayerbotAIConfig.IsInRandomAccountList(botAccountId))
        {
            LOG_INFO("playerbots", "快速组队：Altbot {} 已上线，保留原有装备和设置，为玩家 {} 服务。",
                player->GetName(), setup.masterName);
            return;
        }
        // End By leewheel

        uint32 targetLevel = setup.targetLevel;
        int specTab = setup.specTab;
        FastGroupRole role = setup.role;

        // 设置等级
        if (player->GetLevel() != targetLevel)
        {
            player->GiveLevel(targetLevel);
            player->InitStatsForLevel(true);
            player->SetUInt32Value(PLAYER_XP, 0);
        }

        // 如果已死亡，复活
        if (player->isDead())
            player->ResurrectPlayer(1.0f, false);

        // 使用 PlayerbotFactory 设置天赋和装备
        uint32 quality = targetLevel >= 60 ? ITEM_QUALITY_EPIC : ITEM_QUALITY_RARE;
        PlayerbotFactory factory(player, targetLevel, quality);
        factory.SetExcludeHeirloom(true);

        // 强制重置并按指定天赋页设置天赋（直接按tabpage分配，不依赖预设天赋链接）
        // By leewheel 2026-07-07：修复 specTab=-1 导致数组越界 + InitTalentsBySpecNo 不生效的问题
        uint32 cls = player->getClass();
        uint8 actualTab = (specTab >= 0 && specTab <= 2) ? (uint8)specTab : 0;  // specTab=-1 时用第0页
        InitTalentsByTab(player, actualTab);

        // 学习法术
        factory.InitClassSpells();
        factory.InitAvailableSpells();

        // By leewheel 2026-07-18
        // 技能等级补满：确保所有技能（武器技能、防御技能、开锁技能、专业制作技能等）
        //               达到当前等级对应的最大值
        // 修复问题：快速组队的机器人可能技能值很低（如盗贼开锁1/350），
        //          导致副本内无法开箱、武器技能不足导致命中率低等问题
        // InitSkills 会通过 SetRandomSkill 设置武器技能和开锁为 level*5
        // EnsureBotSkillsMaximized 会遍历所有已有技能补满（包括专业制作技能）
        factory.InitSkills();
        EnsureBotSkillsMaximized(player);
        // End By leewheel

        // By leewheel 2026-07-08
        // 装备：先清除所有旧装备，再以非增量模式安装当前等级最佳装备
        // 双保险：虽然下线时已清除，但防止数据库残留或其他路径带入旧装备
        factory.ClearAllItems();
        factory.InitEquipment(false);
        // End By leewheel
        factory.InitBags(true);
        factory.InitAmmo();
        if (targetLevel >= sPlayerbotAIConfig.minEnchantingBotLevel)
            factory.ApplyEnchantAndGemsNew();

        player->DurabilityRepairAll(false, 1.0f, false);

        // By leewheel 2026-07-17
        // 检查并学习坐骑技能
        // 快速组队的机器人可能在之前没有学习坐骑技能，导致无法上坐骑
        // 每次快速组队都必须检查：
        // - 到达地面坐骑等级(20)必须学会一个地面坐骑
        // - 到达飞行坐骑等级(60)必须学会一个地面坐骑和一个飞行坐骑
        EnsureBotHasMounts(player);
        // End By leewheel

        // By leewheel 2026-07-15
        // 天赋变更后必须重置AI策略，否则策略仍基于旧天赋导致 AI 行为错误
        // 原因：PlayerbotAI 在机器人上线时根据当前天赋初始化策略，
        //       但此时 FastGroup 的 InitTalentsByTab 还未修改天赋。
        //       InitTalentsByTab 修改天赋后，AI策略仍是基于旧天赋的，
        //       导致 Bot 行为不正确（例如：防护战士不拉怪、神圣骑士不治疗）。
        // 注意：LFG角色判断已通过 GetRoles() 使用 bySpec=true 修复，
        //       但 AI 行为（拉怪、治疗等）仍依赖策略，所以 ResetStrategies 仍然必须调用。
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
        if (botAI)
            botAI->ResetStrategies(false);
        // End By leewheel

        LOG_INFO("playerbots", "快速组队：机器人 {}（{}-{}）已上线，等级 {}，为玩家 {} 服务。",
            player->GetName(), GetClassNameCN(cls), GetRoleNameCN(role), targetLevel, setup.masterName);
    }

    // By leewheel 2026-07-11
    // 玩家下线时清理待设置记录和检查冷却
    // 注意：LogoutFastGroupBots 已移到 OnPlayerBeforeLogout 中执行
    //       原因：需要在 LogoutAllBots 之前执行，否则机器人已下线无法清理装备
    void OnPlayerLogout(Player* player) override
    {
        if (!player)
            return;

        // 清除待设置记录
        sFastGroupMgr.ClearPendingSetups(player->GetGUID());

        // By leewheel 2026-07-08 - 清除遗留机器人检查冷却
        m_nextOrphanCheck.erase(player->GetGUID());
        // End By leewheel
    }
    // End By leewheel

    // By leewheel 2026-07-11
    // 主控玩家上线时的残留清理
    // 处理服务器崩溃或异常下线后遗留的快速组队记录和空队伍
    void CleanupMasterLogin(Player* master)
    {
        if (!master)
            return;

        uint32 masterGuid = master->GetGUID().GetCounter();

        // 1. 检查数据库中是否有残留的快速组队记录
        //    正常下线时 OnPlayerBeforeLogout 已清理，此处处理异常情况
        if (sFastGroupMgr.DbHasFastGroupBots(masterGuid))
        {
            LOG_INFO("playerbots", "快速组队：玩家 {} 上线时发现残留快速组队记录，正在清理...", master->GetName());

            // 获取所有残留的快速组队机器人GUID
            std::vector<uint32> botGuids = sFastGroupMgr.DbGetBotGuidsByMaster(masterGuid);
            for (uint32 botGuid : botGuids)
            {
                // 直接在数据库中清空这些机器人的装备
                CharacterDatabase.Execute("DELETE FROM character_inventory WHERE guid = {}", botGuid);
                CharacterDatabase.Execute("DELETE FROM item_instance WHERE owner_guid = {}", botGuid);
            }

            // 删除所有残留记录
            sFastGroupMgr.DbRemoveAllByMaster(masterGuid);

            LOG_INFO("playerbots", "快速组队：玩家 {} 的残留快速组队记录已清理（共 {} 个机器人）。",
                master->GetName(), botGuids.size());
        }

        // 2. 检查玩家是否在残留的空队伍中
        //    如果队伍中只有玩家自己（其他成员都已下线），则离开队伍
        Group* group = master->GetGroup();
        if (group)
        {
            // 统计队伍中在线成员数量
            uint32 onlineCount = 0;
            for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
            {
                Player* member = itr->GetSource();
                if (member && member->GetSession() && !member->GetSession()->isLogingOut())
                    ++onlineCount;
            }

            // 如果只有玩家自己在线，离开队伍
            if (onlineCount <= 1)
            {
                group->RemoveMember(master->GetGUID(), GROUP_REMOVEMETHOD_LEAVE);
                LOG_INFO("playerbots", "快速组队：玩家 {} 上线时发现残留空队伍，已自动离开。", master->GetName());
            }
        }
    }
    // End By leewheel

    // By leewheel 2026-07-08
    // 定期检测机器人是否全部进组，全部进组后分配LFG角色
    // 同时检测遗留机器人（上次快速组队遗留的机器人）
    void OnPlayerAfterUpdate(Player* player, uint32 /*p_time*/) override
    {
        if (!player)
            return;

        // 跳过机器人
        if (GET_PLAYERBOT_AI(player))
            return;

        ObjectGuid guid = player->GetGUID();

        // By leewheel 2026-07-08
        // 遗留机器人检查：如果玩家在队伍中，且队伍中有在线的快速组队机器人，
        // 但 m_fastGroupBots 内存中没有记录，重新注册
        // By leewheel 2026-07-10 修订：
        // 使用数据库表判断是否为快速组队的机器人，而非仅判断 IsRandomBot
        // 原因：随机本的Rndbot也是IsRandomBot，但不应被纳入快速组队管理
        if (!sFastGroupMgr.HasFastGroupBots(guid))
        {
            // 冷却检查：每 5 秒检测一次
            time_t now = time(nullptr);
            auto checkIt = m_nextOrphanCheck.find(guid);
            if (checkIt != m_nextOrphanCheck.end() && now < checkIt->second)
                return;
            m_nextOrphanCheck[guid] = now + 5;

            Group* group = player->GetGroup();
            if (group)
            {
                std::vector<ObjectGuid> orphanBots;
                for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
                {
                    Player* member = itr->GetSource();
                    // By leewheel 2026-07-10
                    // 只注册数据库表中记录为快速组队的机器人
                    // 随机本的Rndbot和Altbot都不纳入快速组队管理
                    if (member && GET_PLAYERBOT_AI(member) &&
                        sFastGroupMgr.DbIsFastGroupBot(member->GetGUID().GetCounter()))
                        orphanBots.push_back(member->GetGUID());
                    // End By leewheel
                }

                if (!orphanBots.empty())
                {
                    sFastGroupMgr.RegisterFastGroupBots(player, orphanBots);
                    LOG_INFO("playerbots", "快速组队：玩家 {} 发现 {} 个遗留快速组队机器人，已重新纳入管理。",
                        player->GetName(), orphanBots.size());

                    ChatHandler handler(player->GetSession());
                    handler.PSendSysMessage("|cff00ccff[快速组队] 检测到 {} 个遗留快速组队机器人，已重新纳入管理。|r", orphanBots.size());
                }
            }
        }
        // End By leewheel

        // 只处理有快速组队机器人的主控玩家
        if (!sFastGroupMgr.HasFastGroupBots(guid))
            return;

        // 如果角色已分配，不再重复
        if (sFastGroupMgr.HasRolesAssigned(guid))
            return;

        // 获取已注册的机器人列表
        std::vector<ObjectGuid> botGuids = sFastGroupMgr.GetFastGroupBotGuids(guid);
        if (botGuids.empty())
            return;

        // 检查玩家是否在队伍中
        Group* group = player->GetGroup();
        if (!group)
            return;

        // 检查所有机器人是否已进组
        uint32 inGroupCount = 0;
        for (ObjectGuid botGuid : botGuids)
        {
            if (group->IsMember(botGuid))
                ++inGroupCount;
        }

        // 全部进组则分配角色
        if (inGroupCount == botGuids.size())
        {
            AssignLfgRoles(player);
            sFastGroupMgr.SetRolesAssigned(guid, true);

            ChatHandler handler(player->GetSession());
            handler.PSendSysMessage("|cff00ff00[快速组队] 所有成员已进组，LFG角色分配完成。|r");
        }
    }
    // End By leewheel

private:
    // By leewheel 2026-07-08
    // 遗留机器人检查冷却：玩家GUID -> 下次检查时间
    std::unordered_map<ObjectGuid, time_t> m_nextOrphanCheck;
    // End By leewheel
};

// ============================================================
//  GroupScript：玩家离队时自动下线快速组队机器人
// ============================================================
class FastGroupGroupScript : public GroupScript
{
public:
    FastGroupGroupScript() : GroupScript("FastGroupGroupScript", {
        GROUPHOOK_ON_REMOVE_MEMBER,
        GROUPHOOK_ON_DISBAND
    }) {}

    void OnRemoveMember(Group* group, ObjectGuid guid, RemoveMethod method, ObjectGuid /*kicker*/, const char* /*reason*/) override
    {
        // 只处理玩家主动离队或被踢出
        if (method != GROUP_REMOVEMETHOD_LEAVE && method != GROUP_REMOVEMETHOD_KICK)
            return;

        Player* player = ObjectAccessor::FindConnectedPlayer(guid);
        if (!player)
            return;

        // 只处理真实玩家（非机器人）
        if (GET_PLAYERBOT_AI(player))
            return;

        // By leewheel 2026-07-10
        // 基于数据库表判断是否为快速组队
        // 数据库记录持久化，不受内存记录清除影响，也不受服务器重启影响
        // 只有在 playerbots_fast_group_members 表中记录为 FG_TYPE_FAST_GROUP 的机器人才需要清理装备
        bool hadFastGroupBots = sFastGroupMgr.DbHasFastGroupBots(guid.GetCounter());

        // 如果该玩家有快速组队机器人在线，下线它们（清理注册Rndbot的装备）
        sFastGroupMgr.LogoutFastGroupBots(player);

        // 备用检查：只有数据库中有快速组队记录时才执行
        // 处理服务器重启等场景导致内存记录丢失后，遗留的快速组队Rndbot
        // 随机本的Rndbot和Altbot完全不受影响
        if (hadFastGroupBots && group)
        {
            PlayerbotMgr* mgr = GET_PLAYERBOT_MGR(player);
            if (mgr)
            {
                for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
                {
                    Player* member = itr->GetSource();
                    if (member && GET_PLAYERBOT_AI(member))
                    {
                        // By leewheel 2026-07-10
                        // 用数据库表精确判断：只有在表中记录为快速组队的机器人才清理装备并下线
                        // 随机本的Rndbot和Altbot完全不受影响
                        if (sFastGroupMgr.DbIsFastGroupBot(member->GetGUID().GetCounter()))
                        {
                            // 快速组队的Rndbot：清理装备并下线
                            PlayerbotFactory clearFactory(member, member->GetLevel(), 0);
                            clearFactory.ClearAllItems();
                            member->SaveToDB(false, false);
                            mgr->LogoutPlayerBot(member->GetGUID());
                            sFastGroupMgr.DbRemoveFastGroupMember(member->GetGUID().GetCounter());
                            LOG_INFO("playerbots", "快速组队：玩家 {} 退队，快速组队机器人 {} 已下线（已清除装备）。",
                                player->GetName(), member->GetName());
                        }
                        // End By leewheel
                    }
                }
            }
        }
        // End By leewheel
    }

    void OnDisband(Group* /*group*/) override
    {
        // 团队解散时，主控玩家离队会触发 OnRemoveMember，这里不需要额外处理
    }
};

// ============================================================
//  脚本注册入口
// ============================================================
void AddSC_FastGroup()
{
    new fastgroup_commandscript();
    new FastGroupPlayerScript();
    new FastGroupGroupScript();
}
//End By leewheel
