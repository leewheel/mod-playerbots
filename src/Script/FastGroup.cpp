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
    if (mgr)
    {
        for (ObjectGuid botGuid : itr->second)
        {
            Player* bot = mgr->GetPlayerBot(botGuid);
            if (bot)
            {
                LOG_INFO("playerbots", "快速组队：玩家 {} 离队/下线，机器人 {} 正在下线。",
                    master->GetName(), bot->GetName());
                mgr->LogoutPlayerBot(botGuid);
            }
        }
    }

    m_fastGroupBots.erase(itr);
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

    // 一次性查询所有符合职业和阵营的离线角色
    QueryResult results = CharacterDatabase.Query(
        "SELECT guid, name, race, account, class FROM characters "
        "WHERE class IN ({}) AND online = 0 AND {} "
        "ORDER BY RAND()",
        classList, raceCondition);

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

    // 第一轮：优先从尚未被使用的职业中各取1个（确保每种职业最多1人）
    for (auto& [cls, list] : byClass)
    {
        if (candidates.size() >= neededCount)
            break;
        // 跳过已被其他角色定位使用的职业
        if (usedClasses.find(cls) != usedClasses.end())
            continue;
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
        for (auto& [cls, list] : byClass)
        {
            if (candidates.size() >= neededCount)
                break;
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
//  命令处理函数
// ============================================================

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

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable party5Table  = {{ "", HandleFastGroupParty5Command,  rbac::RBAC_PERM_COMMAND_RELOAD, Console::No }};
        static ChatCommandTable raid10Table  = {{ "", HandleFastGroupRaid10Command,  rbac::RBAC_PERM_COMMAND_RELOAD, Console::No }};
        static ChatCommandTable raid25Table  = {{ "", HandleFastGroupRaid25Command,  rbac::RBAC_PERM_COMMAND_RELOAD, Console::No }};
        static ChatCommandTable raid40Table  = {{ "", HandleFastGroupRaid40Command,  rbac::RBAC_PERM_COMMAND_RELOAD, Console::No }};
        static ChatCommandTable disbandTable = {{ "", HandleFastGroupDisbandCommand, rbac::RBAC_PERM_COMMAND_RELOAD, Console::No }};

        static ChatCommandTable commandTable = {
            { "5人队",        party5Table  },
            { "10人团",       raid10Table  },
            { "25人团",       raid25Table  },
            { "40人团",       raid40Table  },
            { "解散快速组队", disbandTable },
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
        PLAYERHOOK_ON_LOGOUT
    }) {}

    // 机器人上线时设置等级、天赋、装备
    void OnPlayerLogin(Player* player) override
    {
        if (!player)
            return;

        // 检查是否是快速组队待设置的机器人
        PendingBotSetup setup;
        if (!sFastGroupMgr.PopPendingSetup(player->GetGUID(), setup))
            return;

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

        // 装备
        factory.InitEquipment(true);
        factory.InitBags(true);
        factory.InitAmmo();
        if (targetLevel >= sPlayerbotAIConfig.minEnchantingBotLevel)
            factory.ApplyEnchantAndGemsNew();

        player->DurabilityRepairAll(false, 1.0f, false);

        LOG_INFO("playerbots", "快速组队：机器人 {}（{}-{}）已上线，等级 {}，为玩家 {} 服务。",
            player->GetName(), GetClassNameCN(cls), GetRoleNameCN(role), targetLevel, setup.masterName);
    }

    // 玩家下线时清理快速组队记录并下线机器人
    void OnPlayerLogout(Player* player) override
    {
        if (!player)
            return;

        // 如果是主控玩家（有快速组队机器人），下线所有机器人
        sFastGroupMgr.LogoutFastGroupBots(player);

        // 清除待设置记录
        sFastGroupMgr.ClearPendingSetups(player->GetGUID());
    }
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

    void OnRemoveMember(Group* /*group*/, ObjectGuid guid, RemoveMethod method, ObjectGuid /*kicker*/, const char* /*reason*/) override
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

        // 如果该玩家有快速组队机器人在线，下线它们
        sFastGroupMgr.LogoutFastGroupBots(player);
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
