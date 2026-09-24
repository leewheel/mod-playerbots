//By leewheel 2026-09-22
/*
 * ZhaoYufeng.cpp —— 特殊机器人「赵与风」实现
 *
 * 设计说明见 ZhaoYufeng.h。
 * 本文件负责：
 *   1. 加载/征用两位「赵与风」（联盟一名、部落一名，均为战士）
 *   2. 提供"是否可编入队伍"的查询与"按等级整备"的工具
 *   3. 给他挂上独立策略 "always need"（全需求）
 *   4. 提供 .赵与风 状态查询命令
 *
 * ⛔ 本文件只做"叠加"，不改动既有系统；编入队伍的钩子分别落在
 *    FastGroup.cpp（团本/快速组队）与 RandomPlayerbotMgr.cpp（随机本 LFG）。
 */
//End By leewheel

#include "ZhaoYufeng.h"

#include "CharacterCache.h"
#include "Chat.h"
#include "Config.h"
#include "DBCStores.h"
#include "DBCStructure.h"
#include "DatabaseEnv.h"
#include "FastGroupCommon.h"
#include "Group.h"
#include "Log.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotFactory.h"
#include "Playerbots.h"
#include "Random.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"

#include <sstream>
#include <string>
#include <vector>

// 命令表类型 ChatCommandTable 定义在 Acore::ChatCommands 命名空间内（与 FastGroup.cpp 一致）
using namespace Acore::ChatCommands;

// ============================================================
//  单例
// ============================================================
ZhaoYufengMgr& ZhaoYufengMgr::instance()
{
    static ZhaoYufengMgr inst;
    return inst;
}

// ============================================================
//  配置读取
//  说明：配置项未写入 .conf 时一律走这里的默认值，保证开箱可用。
// ============================================================
void ZhaoYufengMgr::LoadConfig()
{
    m_enabled = sConfigMgr->GetOption<bool>("AiPlayerbot.ZhaoYufeng.Enabled", true);
    m_chance  = sConfigMgr->GetOption<uint32>("AiPlayerbot.ZhaoYufeng.Chance", 25);
    m_name    = sConfigMgr->GetOption<std::string>("AiPlayerbot.ZhaoYufeng.Name", "赵与风");

    if (m_chance > 100)
        m_chance = 100;
}

// ============================================================
//  从数据库加载已有的「赵与风」
//  按种族 TeamID 归入联盟(0)/部落(1)
// ============================================================
void ZhaoYufengMgr::LoadEntriesFromDb()
{
    QueryResult result = CharacterDatabase.Query(
        "SELECT guid, account, race, class, gender, level FROM characters WHERE name = '{}'", m_name);

    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();
        uint32 guidLow = fields[0].Get<uint32>();
        uint8  race    = fields[2].Get<uint8>();

        ChrRacesEntry const* raceEntry = sChrRacesStore.LookupEntry(race);
        // TeamID: 7 = 联盟, 1 = 部落（与 Player::TeamIdForRace 一致）
        uint32 teamIdx = (raceEntry && raceEntry->TeamID == 7) ? 0 : 1;

        if (m_entries[teamIdx].valid)
            continue;   // 该阵营已有，忽略多余的同名角色

        m_entries[teamIdx].guid        = ObjectGuid(HighGuid::Player, guidLow);
        m_entries[teamIdx].accountId   = fields[1].Get<uint32>();
        m_entries[teamIdx].race        = race;
        m_entries[teamIdx].playerClass = fields[3].Get<uint8>();
        m_entries[teamIdx].gender      = fields[4].Get<uint8>();
        m_entries[teamIdx].level       = fields[5].Get<uint32>();
        m_entries[teamIdx].valid       = true;

        LOG_INFO("playerbots", "赵与风：已从数据库加载 {} 阵营的「{}」(GUID:{}, 账号:{}, 等级:{})。",
            teamIdx == 0 ? "联盟" : "部落", m_name, guidLow, m_entries[teamIdx].accountId, m_entries[teamIdx].level);
    } while (result->NextRow());
}

// ============================================================
//  缺少某阵营的「赵与风」时，从随机机器人账号里征用一名战士并改名
//  ⛔ 严格限定在 randomBotAccounts 内，绝不触碰真实玩家角色
// ============================================================
void ZhaoYufengMgr::RecruitIfMissing(uint32 teamId)
{
    if (sPlayerbotAIConfig.randomBotAccounts.empty())
    {
        LOG_WARN("playerbots", "赵与风：随机机器人账号列表为空（{} 阵营暂不征用），将在后续重试。",
            teamId == 0 ? "联盟" : "部落");
        return;
    }

    // 收集该阵营的全部种族（动态遍历 ChrRaces，与快速组队的做法保持一致）
    uint32 const wantTeamId = (teamId == 0) ? 7 : 1;
    std::string raceCondition = "race IN (";
    bool firstRace = true;
    for (uint32 i = 0; i < sChrRacesStore.GetNumRows(); ++i)
    {
        ChrRacesEntry const* re = sChrRacesStore.LookupEntry(i);
        if (!re || re->TeamID != wantTeamId)
            continue;
        if (!firstRace)
            raceCondition += ",";
        raceCondition += std::to_string(re->RaceID);
        firstRace = false;
    }
    raceCondition += ")";

    if (firstRace)
    {
        LOG_ERROR("playerbots", "赵与风：{} 阵营没有任何可用种族，无法征用角色。",
            teamId == 0 ? "联盟" : "部落");
        return;
    }

    // 随机机器人账号 IN 条件
    std::string accountCondition = "account IN (";
    bool firstAccount = true;
    for (uint32 accountId : sPlayerbotAIConfig.randomBotAccounts)
    {
        if (!firstAccount)
            accountCondition += ",";
        accountCondition += std::to_string(accountId);
        firstAccount = false;
    }
    accountCondition += ")";

    QueryResult result = CharacterDatabase.Query(
        "SELECT guid, account, race, class, gender, level FROM characters "
        "WHERE class = {} AND online = 0 AND {} AND {} AND name <> '{}' "
        "ORDER BY RAND() LIMIT 1",
        uint32(CLASS_WARRIOR), raceCondition, accountCondition, m_name);

    if (!result)
    {
        LOG_ERROR("playerbots", "赵与风：{} 阵营未能征用到战士角色（随机账号池中无可用离线战士）。",
            teamId == 0 ? "联盟" : "部落");
        return;
    }

    Field* fields = result->Fetch();
    uint32 guidLow    = fields[0].Get<uint32>();
    uint32 accountId  = fields[1].Get<uint32>();
    uint8  race       = fields[2].Get<uint8>();
    uint8  playerCls  = fields[3].Get<uint8>();
    uint8  gender     = fields[4].Get<uint8>();
    uint32 level      = fields[5].Get<uint32>();

    // 改名（同步执行，确保后续角色缓存加载/查询已是新名字）
    CharacterDatabase.DirectExecute(
        "UPDATE characters SET name = '" + m_name + "' WHERE guid = " + std::to_string(guidLow));

    ObjectGuid botGuid = ObjectGuid(HighGuid::Player, guidLow);

    m_entries[teamId].guid        = botGuid;
    m_entries[teamId].accountId   = accountId;
    m_entries[teamId].race        = race;
    m_entries[teamId].playerClass = playerCls;
    m_entries[teamId].gender      = gender;
    m_entries[teamId].level       = level;
    m_entries[teamId].valid       = true;

    // 同步角色缓存，避免运行期靠旧名字/空名字查找
    sCharacterCache->AddCharacterCacheEntry(botGuid, accountId, m_name, gender, race, playerCls, level);

    LOG_INFO("playerbots", "赵与风：已为 {} 阵营征用角色并改名 —— GUID:{}, 账号:{}, 原等级:{}，现在名字为「{}」。",
        teamId == 0 ? "联盟" : "部落", guidLow, accountId, level, m_name);
}

// ============================================================
//  加载入口
// ============================================================
void ZhaoYufengMgr::Load()
{
    LoadConfig();

    if (!m_enabled)
    {
        LOG_INFO("playerbots", "赵与风系统：已按配置禁用（AiPlayerbot.ZhaoYufeng.Enabled = 0）。");
        m_loaded = true;
        return;
    }

    LoadEntriesFromDb();

    bool incomplete = false;
    for (uint32 teamIdx = 0; teamIdx < 2; ++teamIdx)
    {
        if (m_entries[teamIdx].valid)
            continue;

        RecruitIfMissing(teamIdx);

        if (!m_entries[teamIdx].valid)
            incomplete = true;
    }

    // 只有两位都就位才算加载完成；否则保留 m_loaded = false，供 EnsureLoaded 重试
    m_loaded = !incomplete;

    LOG_INFO("playerbots", "赵与风系统：加载{}（出现概率 {}%）。",
        m_loaded ? "完成" : "未完整（稍后重试）", m_chance);
}

void ZhaoYufengMgr::EnsureLoaded()
{
    if (m_loaded)
        return;

    Load();
}

// ============================================================
//  查询
// ============================================================
bool ZhaoYufengMgr::IsZhaoYufeng(ObjectGuid guid) const
{
    for (uint32 i = 0; i < 2; ++i)
    {
        if (m_entries[i].valid && m_entries[i].guid == guid)
            return true;
    }
    return false;
}

bool ZhaoYufengMgr::IsZhaoYufeng(Player* player) const
{
    return player && IsZhaoYufeng(player->GetGUID());
}

ZhaoYufengEntry const* ZhaoYufengMgr::GetEntryForTeam(uint32 teamId) const
{
    if (teamId > 1)
        return nullptr;

    return m_entries[teamId].valid ? &m_entries[teamId] : nullptr;
}

Player* ZhaoYufengMgr::FindAvailableForTeam(uint32 teamId)
{
    ZhaoYufengEntry const* entry = GetEntryForTeam(teamId);
    if (!entry)
        return nullptr;

    Player* bot = ObjectAccessor::FindConnectedPlayer(entry->guid);
    if (!bot || !bot->IsInWorld())
        return nullptr;

    // 已经在别人的队伍里就不抢人（保证"赵与风"同一时间只在一支队伍中）
    if (bot->GetGroup())
        return nullptr;

    if (bot->IsInCombat())
        return nullptr;

    return bot;
}

bool ZhaoYufengMgr::RollChance() const
{
    if (!m_enabled || m_chance == 0)
        return false;

    return urand(0, 99) < m_chance;
}

// ============================================================
//  随机本（LFG）编入状态
// ============================================================
void ZhaoYufengMgr::BeginLfgPending(uint32 teamId, uint32 queuedLevel)
{
    if (teamId > 1)
        return;

    // 新一次排队：先清掉上一轮残留标记
    ClearLfgPending(teamId);

    if (!m_enabled || !RollChance())
        return;

    ZhaoYufengEntry const* entry = GetEntryForTeam(teamId);
    if (!entry)
        return;

    m_lfgPending[teamId]  = true;
    m_lfgTarget[teamId]   = entry->guid;
    m_lfgLevel[teamId]    = queuedLevel;
    m_lfgSummoned[teamId] = false;
    m_lfgDeadline[teamId] = time(nullptr) + 60;   // 60 秒内未成功入队则放弃本次

    LOG_INFO("playerbots", "赵与风：{} 阵营本次随机本排队命中 {}% 概率，将把「赵与风」编入队伍（目标等级 {}）。",
        teamId == 0 ? "联盟" : "部落", m_chance, queuedLevel);
}

bool ZhaoYufengMgr::IsLfgPending(uint32 teamId) const
{
    return teamId < 2 && m_lfgPending[teamId];
}

ObjectGuid ZhaoYufengMgr::GetLfgTarget(uint32 teamId) const
{
    return teamId < 2 ? m_lfgTarget[teamId] : ObjectGuid();
}

uint32 ZhaoYufengMgr::GetLfgTargetLevel(uint32 teamId) const
{
    return teamId < 2 ? m_lfgLevel[teamId] : 0;
}

bool ZhaoYufengMgr::HasLfgSummoned(uint32 teamId) const
{
    return teamId < 2 && m_lfgSummoned[teamId];
}

void ZhaoYufengMgr::MarkLfgSummoned(uint32 teamId)
{
    if (teamId < 2)
        m_lfgSummoned[teamId] = true;
}

bool ZhaoYufengMgr::IsLfgExpired(uint32 teamId) const
{
    return teamId >= 2 || (m_lfgDeadline[teamId] != 0 && time(nullptr) > m_lfgDeadline[teamId]);
}

void ZhaoYufengMgr::ClearLfgPending(uint32 teamId)
{
    if (teamId > 1)
        return;

    m_lfgPending[teamId]  = false;
    m_lfgTarget[teamId].Clear();
    m_lfgLevel[teamId]    = 0;
    m_lfgSummoned[teamId] = false;
    m_lfgDeadline[teamId] = 0;
}

// ============================================================
//  独立策略：全需求
// ============================================================
void ZhaoYufengMgr::ApplyAlwaysNeed(Player* bot)
{
    if (!bot)
        return;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
        return;

    if (!botAI->HasStrategy("always need", BOT_STATE_NON_COMBAT))
    {
        botAI->ChangeStrategy("+always need", BOT_STATE_NON_COMBAT);
        LOG_INFO("playerbots", "赵与风：已为 {} 启用独立策略「全需求」(always need)。", bot->GetName().c_str());
    }
}

// ============================================================
//  按目标等级整备（等级 / 天赋 / 技能 / 装备）
//  说明：赵与风是战士，统一走狂暴（天赋页 1）输出路线。
//        整备流程与快速组队的机器人初始化保持一致。
// ============================================================
void ZhaoYufengMgr::PrepareForLevel(Player* bot, uint32 level)
{
    if (!bot || !level)
        return;

    if (bot->GetLevel() != level)
    {
        bot->GiveLevel(level);
        bot->InitStatsForLevel(true);
        bot->SetUInt32Value(PLAYER_XP, 0);
    }

    if (bot->isDead())
        bot->ResurrectPlayer(1.0f, false);

    uint8 const specTab = 1;   // 战士：0=武器 1=狂暴 2=防护，赵与风走狂暴输出
    InitTalentsByTab(bot, specTab);

    uint32 quality = level >= 60 ? ITEM_QUALITY_EPIC : ITEM_QUALITY_RARE;
    PlayerbotFactory factory(bot, level, quality);
    factory.SetExcludeHeirloom(true);
    factory.InitClassSpells();
    factory.InitAvailableSpells();
    factory.InitSkills();
    factory.ClearAllItems();
    factory.InitEquipment(false);
    factory.InitBags(true);
    factory.InitAmmo();
    if (level >= sPlayerbotAIConfig.minEnchantingBotLevel)
        factory.ApplyEnchantAndGemsNew();

    bot->DurabilityRepairAll(false, 1.0f, false);

    // 重置策略会清掉自定义策略，因此"全需求"必须在重置之后重新挂上
    if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot))
        botAI->ResetStrategies(false);

    ApplyAlwaysNeed(bot);

    LOG_INFO("playerbots", "赵与风：{} 已整备到等级 {}（狂暴输出路线）。", bot->GetName().c_str(), level);
}

// ============================================================
//  状态文本
// ============================================================
std::string ZhaoYufengMgr::BuildStatusText() const
{
    std::ostringstream out;
    out << "赵与风系统：" << (m_enabled ? "启用" : "禁用")
        << "，出现概率 " << m_chance << "%"
        << "，名字「" << m_name << "」\n";

    char const* teamName[2] = { "联盟", "部落" };
    for (uint32 t = 0; t < 2; ++t)
    {
        out << "  " << teamName[t] << "：";
        if (!m_entries[t].valid)
        {
            out << "未就位（未征用到角色）\n";
            continue;
        }

        Player* bot = ObjectAccessor::FindConnectedPlayer(m_entries[t].guid);
        out << "GUID " << m_entries[t].guid.ToString()
            << "，账号 " << m_entries[t].accountId
            << "，等级 " << m_entries[t].level
            << "，" << (bot && bot->IsInWorld() ? "在线" : "离线");
        if (bot && bot->GetGroup())
            out << "（已在队伍中）";
        if (bot)
        {
            PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
            if (botAI && botAI->HasStrategy("always need", BOT_STATE_NON_COMBAT))
                out << "，策略[全需求]已启用";
        }
        out << "\n";
    }

    return out.str();
}

// ============================================================
//  命令：.赵与风
// ============================================================
class zhaoyufeng_commandscript : public CommandScript
{
public:
    zhaoyufeng_commandscript() : CommandScript("zhaoyufeng_commandscript") {}

    static bool HandleZhaoYufengStatusCommand(ChatHandler* handler)
    {
        handler->SendSysMessage(sZhaoYufengMgr.BuildStatusText().c_str());
        return true;
    }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable commandTable =
        {
            { "赵与风", HandleZhaoYufengStatusCommand, SEC_PLAYER, Console::No },
        };

        return commandTable;
    }
};

// ============================================================
//  启动脚本：在世界初始化之前加载/征用两位「赵与风」
//  时机说明：本脚本的注册位置在 PlayerbotsWorldScript 之后，
//            因此执行时 sPlayerbotAIConfig.Initialize() 已完成、
//            sPlayerbotAIConfig.randomBotAccounts 已就绪；
//            同时早于 characters 角色缓存加载，改名结果会被缓存正确读取。
// ============================================================
class ZhaoYufengWorldScript : public WorldScript
{
public:
    ZhaoYufengWorldScript() : WorldScript("ZhaoYufengWorldScript", {
        WORLDHOOK_ON_BEFORE_WORLD_INITIALIZED
    }) {}

    void OnBeforeWorldInitialized() override
    {
        sZhaoYufengMgr.Load();
    }
};

void AddSC_ZhaoYufeng()
{
    new zhaoyufeng_commandscript();
    new ZhaoYufengWorldScript();
}
//End By leewheel
