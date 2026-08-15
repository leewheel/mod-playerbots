/*
 * This file is part of the mod-playerbots module for AzerothCore.
 * Released under GNU GPL v2 license.
 */

// By leewheel 2026-07-18
// 机器人副本进入条件自动补全系统
// 当玩家带快速组队的机器人进入副本/团本时，机器人可能因为各种条件不满足而进不去
// 本系统在玩家传送进副本前，自动检测并修复机器人的进入条件：
//   1. 不是团队 → 将队伍转为团队（团本要求）
//   2. 机器人已死亡 → 复活机器人
//   3. 副本难度不匹配 → 设置与主控玩家相同的难度
//   4. 绑定了不同的副本进度 → 解绑旧进度
//   5. 缺少钥匙（物品） → 给机器人添加钥匙
//   6. 缺少前置任务 → 给机器人完成任务
//   7. 缺少成就 → 给机器人完成成就
//   8. 等级不够 → 提升等级（与主控玩家相同）
// 不修复的情况（合理的限制，不应绕过）：
//   - 人数已满 → 副本确实满了
//   - Boss战进行中 → 等Boss战结束
//   - 进入副本次数过多 → 等待冷却
// End By leewheel

#include "Playerbots.h"

#include "AchievementMgr.h"
#include "DBCStores.h"
#include "FastGroupCommon.h"
#include "Group.h"
#include "InstanceSaveMgr.h"
#include "Item.h"
#include "LFGMgr.h"
#include "Map.h"
#include "MapMgr.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "PlayerScript.h"
#include "PlayerbotAI.h"
#include "PlayerbotMgr.h"
#include "QuestDef.h"
#include "RandomPlayerbotMgr.h"
#include "ScriptMgr.h"
#include "WorldSession.h"

#include <unordered_map>
#include <vector>

// By leewheel 2026-08-01
// 周期性卡顿修复：FixAllDungeonRequirements + InitializeLockedDungeons 每次执行都会遍历
// 全部 900+ 张副本地图并逐项检查钥匙/任务/成就/等级，是重操作。在 7月30日 打包版本中，
// 坦克/治疗 bot 通过 LfgRolePriorityTrigger 每约4秒触发一次 LFG join，每次 join 都走
// OnPlayerCanJoinLfg 钩子执行全地图遍历 + lockmap 日志写盘，与 LFG 8秒撮合周期叠加，
// 形成每 7~8 秒规律性卡顿。这里增加每 bot 节流：同一 bot 在 30 秒内只执行一次完整修复。
static std::unordered_map<ObjectGuid, time_t> sLastDungeonFixTime;

static bool IsDungeonFixThrottled(Player* bot)
{
    if (!bot)
        return true;

    time_t const now = time(nullptr);

    // By leewheel 2026-08-01
    // 定期清理过期条目：随机 bot 登出/登入会不断产生新 GUID，map 长期运行会无限增长。
    // 超过 60 秒的条目对 30 秒节流已无任何意义，map 超过阈值时全量清理。
    if (sLastDungeonFixTime.size() > 10000)
    {
        for (auto it = sLastDungeonFixTime.begin(); it != sLastDungeonFixTime.end();)
        {
            if (now - it->second > 60)
                it = sLastDungeonFixTime.erase(it);
            else
                ++it;
        }
    }
    // End By leewheel

    auto it = sLastDungeonFixTime.find(bot->GetGUID());
    if (it != sLastDungeonFixTime.end() && now - it->second < 30)
        return true;

    sLastDungeonFixTime[bot->GetGUID()] = now;
    return false;
}
// End By leewheel

// ============================================================================
// 核心函数：检查并修复 DungeonProgressionRequirements 中的条件
// 包括：物品（钥匙）、任务、成就、等级
// ============================================================================

// By leewheel 2026-07-18
// 遍历所有副本地图，给机器人补全所有缺少的进入条件（钥匙/任务/成就/等级）
// 用于LFG排队前刷新缓存：不确定会排到哪个副本，所以全部补全
static void FixAllDungeonRequirements(Player* bot, Player* master = nullptr)
{
    if (!bot)
        return;

    // By leewheel 2026-07-29
    // LFGMgr::InitializeLockedDungeons 对死亡骑士有不在 dungeon_access_requirements 表内的硬编码检查：
    // 必须已奖励联盟任务 13188“王者之城”或部落任务 13189“酋长的祝福”。
    // 随机死亡骑士机器人通常跳过新手任务线，若不补齐该条件，所有可用随机地下城都会被标记为
    // LFG_LOCKSTATUS_QUEST_NOT_COMPLETED，JoinLfg 返回后仍是 LFG_STATE_NONE，导致坦克始终无法进队列。
    if (bot->IsClass(CLASS_DEATH_KNIGHT) &&
        !bot->IsQuestRewarded(13188) && !bot->IsQuestRewarded(13189))
    {
        uint32 const finalQuestId = bot->GetTeamId() == TEAM_ALLIANCE ? 13188 : 13189;
        bot->SetRewardedQuest(finalQuestId);
        LOG_INFO("playerbots", "LFG排队修复：死亡骑士机器人 {} 缺少新手任务线最终任务(任务ID:{})，已标记为已奖励。",
            bot->GetName(), finalQuestId);
    }
    // End By leewheel

    // 遍历所有地图，检查有进入要求的副本
    // dungeon_access_template 表中的所有记录都检查一遍
    for (uint32 mapId = 0; mapId < sMapStore.GetNumRows(); ++mapId)
    {
        MapEntry const* entry = sMapStore.LookupEntry(mapId);
        if (!entry || !entry->IsDungeon())
            continue;

        // 检查普通和英雄两种难度
        for (uint8 diff = 0; diff < 2; ++diff)
        {
            Difficulty difficulty = (Difficulty)diff;
            DungeonProgressionRequirements const* ar = sObjectMgr->GetAccessRequirement(mapId, difficulty);
            if (!ar)
                continue;

            // 检查物品（钥匙）
            for (ProgressionRequirement const* itemReq : ar->items)
            {
                if (itemReq->checkLeaderOnly)
                    continue;

                if (itemReq->faction != TEAM_NEUTRAL && itemReq->faction != bot->GetTeamId(true))
                    continue;

                if (!bot->HasItemCount(itemReq->id, 1))
                {
                    bot->AddItem(itemReq->id, 1);
                    LOG_INFO("playerbots", "LFG排队修复：机器人 {} 缺少钥匙(物品ID:{})，已自动添加。",
                        bot->GetName(), itemReq->id);
                }
            }

            // 检查成就
            for (ProgressionRequirement const* achReq : ar->achievements)
            {
                if (achReq->checkLeaderOnly)
                    continue;

                if (achReq->faction != TEAM_NEUTRAL && achReq->faction != bot->GetTeamId(true))
                    continue;

                if (!bot->HasAchieved(achReq->id))
                {
                    AchievementEntry const* achEntry = sAchievementStore.LookupEntry(achReq->id);
                    if (achEntry)
                    {
                        bot->CompletedAchievement(achEntry);
                        LOG_INFO("playerbots", "LFG排队修复：机器人 {} 缺少成就(成就ID:{})，已自动完成。",
                            bot->GetName(), achReq->id);
                    }
                }
            }

            // 检查任务
            for (ProgressionRequirement const* questReq : ar->quests)
            {
                if (questReq->checkLeaderOnly)
                    continue;

                if (questReq->faction != TEAM_NEUTRAL && questReq->faction != bot->GetTeamId(true))
                    continue;

                // By leewheel 2026-07-29
                // 修复：原 bot->CompleteQuest(questId) 对非 QUEST_FLAGS_TRACKING 任务
                //      不会触发 RewardQuest，导致 m_RewardedQuests 不含 questId。
                //      而 LFGMgr::InitializeLockedDungeons 用 GetQuestRewardStatus（=IsQuestRewarded）
                //      检查，对非重复任务仅查 m_RewardedQuests，于是死循环报"已自动完成"
                //      但 LFG 永远 LFG_LOCKSTATUS_QUEST_NOT_COMPLETED，dungeons 清空，
                //      JoinLfg 返回 LFG_JOIN_NOT_MEET_REQS，机器人永远入不了队。
                // 改用 SetRewardedQuest 直接写入 m_RewardedQuests（含 SaveMap，next save 持久化），
                //     立即让 GetQuestRewardStatus 返回 true，cache 重生成后通过 LFG 检查。
                if (!bot->GetQuestRewardStatus(questReq->id))
                {
                    Quest const* qInfo = sObjectMgr->GetQuestTemplate(questReq->id);
                    if (qInfo)
                    {
                        bot->SetRewardedQuest(questReq->id);
                        LOG_INFO("playerbots", "LFG排队修复：机器人 {} 缺少前置任务(任务ID:{})，已标记为已奖励。",
                            bot->GetName(), questReq->id);
                    }
                }
                // End By leewheel
            }
        }
    }
}
// End By leewheel

// By leewheel 2026-07-18
static void FixProgressionRequirements(Player* bot, uint32 mapId, Player* master)
{
    if (!bot || !master)
        return;

    MapEntry const* entry = sMapStore.LookupEntry(mapId);
    if (!entry || !entry->IsDungeon())
        return;

    // 获取当前难度下的副本进入要求
    bool isRaid = entry->IsRaid();
    Difficulty difficulty = bot->GetDifficulty(isRaid);
    DungeonProgressionRequirements const* ar = sObjectMgr->GetAccessRequirement(mapId, difficulty);
    if (!ar)
        return;

    // ---- 1. 等级检查 ----
    if (ar->levelMin && bot->GetLevel() < ar->levelMin)
    {
        uint32 targetLevel = std::max<uint32>(master->GetLevel(), ar->levelMin);
        bot->GiveLevel(targetLevel);
        bot->InitStatsForLevel(true);
        LOG_INFO("playerbots", "副本进入修复：机器人 {} 等级不足(最低需{})，已提升到等级 {}。",
            bot->GetName(), ar->levelMin, targetLevel);
    }

    // ---- 2. 物品（钥匙）检查 ----
    for (ProgressionRequirement const* itemReq : ar->items)
    {
        // 只检查非 leader_only 的物品（leader_only 的物品由队长满足）
        if (itemReq->checkLeaderOnly)
            continue;

        // 阵营检查
        if (itemReq->faction != TEAM_NEUTRAL && itemReq->faction != bot->GetTeamId(true))
            continue;

        // 检查机器人是否已有该物品
        if (!bot->HasItemCount(itemReq->id, 1))
        {
            bot->AddItem(itemReq->id, 1);
            LOG_INFO("playerbots", "副本进入修复：机器人 {} 缺少钥匙(物品ID:{})，已自动添加。",
                bot->GetName(), itemReq->id);
        }
    }

    // ---- 3. 成就检查 ----
    for (ProgressionRequirement const* achReq : ar->achievements)
    {
        if (achReq->checkLeaderOnly)
            continue;

        if (achReq->faction != TEAM_NEUTRAL && achReq->faction != bot->GetTeamId(true))
            continue;

        if (!bot->HasAchieved(achReq->id))
        {
            AchievementEntry const* achEntry = sAchievementStore.LookupEntry(achReq->id);
            if (achEntry)
            {
                bot->CompletedAchievement(achEntry);
                LOG_INFO("playerbots", "副本进入修复：机器人 {} 缺少成就(成就ID:{})，已自动完成。",
                    bot->GetName(), achReq->id);
            }
        }
    }

    // ---- 4. 任务检查 ----
    for (ProgressionRequirement const* questReq : ar->quests)
    {
        if (questReq->checkLeaderOnly)
            continue;

        if (questReq->faction != TEAM_NEUTRAL && questReq->faction != bot->GetTeamId(true))
            continue;

        // By leewheel 2026-07-29
        // 同 FixAllDungeonReasons：必须 SetRewardedQuest，不能仅 CompleteQuest，
        // 否则 LFG 缓存仍判定 quest 未奖励 → 入队失败。
        if (!bot->GetQuestRewardStatus(questReq->id))
        {
            Quest const* qInfo = sObjectMgr->GetQuestTemplate(questReq->id);
            if (qInfo)
            {
                bot->SetRewardedQuest(questReq->id);
                LOG_INFO("playerbots", "副本进入修复：机器人 {} 缺少前置任务(任务ID:{})，已标记为已奖励。",
                    bot->GetName(), questReq->id);
            }
        }
        // End By leewheel
    }
}
// End By leewheel

// ============================================================================
// 核心函数：确保机器人可以进入指定地图
// 返回true表示可以进入或已修复，false表示无法修复（不进入）
// ============================================================================

// By leewheel 2026-07-18
bool EnsureBotCanEnterMap(Player* bot, uint32 mapId, Player* master)
{
    if (!bot || !master)
        return false;

    MapEntry const* entry = sMapStore.LookupEntry(mapId);
    if (!entry)
        return false;

    // 非副本地图直接放行
    if (!entry->IsDungeon())
        return true;

    // 第一步：检查并修复 DungeonProgressionRequirements（钥匙/任务/成就/等级）
    FixProgressionRequirements(bot, mapId, master);

    // 第二步：检查 MapMgr::PlayerCannotEnter 的结果
    Map::EnterState cannotEnter = sMapMgr->PlayerCannotEnter(mapId, bot, false);

    if (cannotEnter == Map::CAN_ENTER)
        return true;

    // 根据不同的拒绝原因逐个修复
    switch (cannotEnter)
    {
        // 1. 不在团队中（团本要求）→ 队伍转团队
        case Map::CANNOT_ENTER_NOT_IN_RAID:
        {
            Group* group = bot->GetGroup();
            if (group && !group->isRaidGroup())
            {
                group->ConvertToRaid();
                LOG_INFO("playerbots", "副本进入修复：玩家 {} 的队伍已转为团队，以进入团本。",
                    master->GetName());
            }
            break;
        }

        // 2. 机器人死亡且尸体不在目标副本 → 复活
        case Map::CANNOT_ENTER_CORPSE_IN_DIFFERENT_INSTANCE:
        {
            bot->ResurrectPlayer(1.0f, false);
            LOG_INFO("playerbots", "副本进入修复：机器人 {} 已死亡，已复活以进入副本。",
                bot->GetName());
            break;
        }

        // 3. 难度不可用 → 调整为与主控玩家相同的难度
        case Map::CANNOT_ENTER_DIFFICULTY_UNAVAILABLE:
        {
            bool isRaid = entry->IsRaid();
            Difficulty masterDiff = master->GetDifficulty(isRaid);
            if (isRaid)
                bot->SetRaidDifficulty(masterDiff);
            else
                bot->SetDungeonDifficulty(masterDiff);

            LOG_INFO("playerbots", "副本进入修复：机器人 {} 的难度已调整为与主控玩家一致。",
                bot->GetName());
            break;
        }

        // 4. 绑定了不同的副本进度 → 解绑
        case Map::CANNOT_ENTER_INSTANCE_BIND_MISMATCH:
        {
            bool isRaid = entry->IsRaid();
            Difficulty diff = bot->GetDifficulty(isRaid);
            sInstanceSaveMgr->PlayerUnbindInstance(bot->GetGUID(), mapId, diff, true, bot);

            LOG_INFO("playerbots", "副本进入修复：机器人 {} 的副本进度绑定已清除，以进入新副本。",
                bot->GetName());
            break;
        }

        // ===== 以下情况不修复，是合理的限制 =====

        // 5. 实例人数已满 → 不进入
        case Map::CANNOT_ENTER_MAX_PLAYERS:
        {
            LOG_WARN("playerbots", "副本进入修复：副本人数已满，机器人 {} 无法进入。",
                bot->GetName());
            return false;
        }

        // 6. Boss战进行中 → 不进入
        case Map::CANNOT_ENTER_ZONE_IN_COMBAT:
        {
            LOG_WARN("playerbots", "副本进入修复：副本内有Boss战进行中，机器人 {} 暂时无法进入。",
                bot->GetName());
            return false;
        }

        // 7. 进入副本次数过多 → 不进入
        case Map::CANNOT_ENTER_TOO_MANY_INSTANCES:
        {
            LOG_WARN("playerbots", "副本进入修复：机器人 {} 进入副本次数过多，请等待冷却后再试。",
                bot->GetName());
            return false;
        }

        default:
            break;
    }

    // 修复后重新检查
    cannotEnter = sMapMgr->PlayerCannotEnter(mapId, bot, false);
    if (cannotEnter != Map::CAN_ENTER)
    {
        LOG_WARN("playerbots", "副本进入修复：机器人 {} 仍无法进入副本(原因:{})，请检查。",
            bot->GetName(), static_cast<int>(cannotEnter));
        return false;
    }

    return true;
}
// End By leewheel

// ============================================================================
// PlayerScript：在主控玩家传送到副本前，检查并修复所有机器人的进入条件
// ============================================================================

class BotInstanceEntryFixScript : public PlayerScript
{
public:
    BotInstanceEntryFixScript() : PlayerScript("BotInstanceEntryFixScript", {
        PLAYERHOOK_ON_BEFORE_TELEPORT,
        PLAYERHOOK_CAN_JOIN_LFG
    }) {}

    // By leewheel 2026-07-21
    // LFG排队钩子：在排队LFG前，检查并修复副本进入条件
    // 关键：LFG系统在玩家登录时就缓存了锁定副本列表（InitializeLockedDungeons），
    //       即使给机器人添加了钥匙/任务，缓存不会自动更新。必须在此处重新生成缓存。
    // 处理两种情况：
    //   1. 随机机器人独立排队（ForceBotsJoinLfg/策略系统触发）→ 修复自身条件
    //   2. 真实玩家带bot组队排队 → 修复队伍中所有bot的条件
    bool OnPlayerCanJoinLfg(Player* player, uint8 /*roles*/, std::set<uint32>& /*dungeons*/,
                            std::string const& /*comment*/) override
    {
        if (!player)
            return true;

        // 情况1：机器人独立排队LFG（随机bot通过ForceBotsJoinLfg或策略系统加入）
        // 原代码直接return true跳过了bot，导致bot的副本准入条件未被修复
        if (player->GetSession()->IsBot())
        {
            PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
            if (botAI && !IsRealPlayer(player))
            {
                // By leewheel 2026-07-21
                // 移除随机本冷却(71328)和逃兵惩罚(71041)：
                // bot完成随机本后获得冷却光环，或战斗中拒绝提案后获得150秒冷却，
                // 或离开LFG队伍后获得30分钟逃兵debuff。这些会阻止bot重新排队。
                // 坦克bot尤其容易中此循环：刷怪进战斗→提案来了→拒绝→冷却→无法排队
                if (player->HasAura(lfg::LFG_SPELL_DUNGEON_COOLDOWN))
                    player->RemoveAura(lfg::LFG_SPELL_DUNGEON_COOLDOWN);
                if (player->HasAura(lfg::LFG_SPELL_DUNGEON_DESERTER))
                    player->RemoveAura(lfg::LFG_SPELL_DUNGEON_DESERTER);
                // End By leewheel

                // By leewheel 2026-08-01
                // 周期性卡顿修复：全地图遍历修复 + 锁定缓存重建是重操作（900+地图×双难度×全部要求），
                // 同一 bot 30 秒内只执行一次，避免坦克/治疗 bot 每约4秒一次 LFG join 时反复全量遍历。
                if (!IsDungeonFixThrottled(player))
                {
                    // 修复所有副本进入条件（钥匙/任务/成就）
                    FixAllDungeonRequirements(player);
                    // 重新生成LFG锁定副本缓存（关键！否则修复了条件但缓存还是旧的）
                    sLFGMgr->InitializeLockedDungeons(player, player->GetGroup());
                }
                // End By leewheel

                // By leewheel 2026-07-29 诊断：dump 当前 lockmap，找出 bot 无法入队的原因
                lfg::LfgLockMap const& lockMap = sLFGMgr->GetLockedDungeons(player->GetGUID());
                if (!lockMap.empty())
                {
                    std::string lockInfo;
                    for (auto const& [dungeonId, status] : lockMap)
                    {
                        if (!lockInfo.empty()) lockInfo += ",";
                        lockInfo += std::to_string(dungeonId) + "=" + std::to_string((uint32)status);
                    }
                    // By leewheel 2026-07-29
                    // 锁定状态必须使用 LFG.h 中的真实枚举值，避免把任务锁定 1022 误诊为普通编号 9。
                    // By leewheel 2026-08-01
                    // 周期性卡顿修复：诊断日志降级为 DEBUG——每次 LFG join 都输出会与 8 秒撮合周期
                    // 叠加形成日志 I/O 尖峰，属于周期卡顿的来源之一。
                    LOG_DEBUG("playerbots", "[LFG诊断] bot {} 锁定的副本({}): {} (LFG_LOCKSTATUS: 1=版本不足 2=等级过低 3=等级过高 4=装等过低 5=装等过高 6=副本已锁定 1001=协调等级过低 1002=协调等级过高 1022=任务未完成 1025=缺少物品 1031=不在开放季 1034=缺少成就)",
                        player->GetName().c_str(), (uint32)lockMap.size(), lockInfo.c_str());
                    // End By leewheel
                }
                else
                {
                    // By leewheel 2026-08-01
                    // 周期性卡顿修复：诊断日志降级为 DEBUG（同 lockmap dump）
                    LOG_DEBUG("playerbots", "[LFG诊断] bot {} lockmap 为空(全部通过)", player->GetName().c_str());
                    // End By leewheel
                }
                // End By leewheel
            }
            return true;
        }

        // 情况2：真实玩家带bot组队排队
        // 获取队伍
        Group* group = player->GetGroup();
        if (!group)
            return true;

        // 收集所有需要检查的机器人
        std::vector<Player*> bots;
        for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || !member->IsInWorld())
                continue;

            // 只处理机器人（非真实玩家）
            if (member->GetSession() && !member->GetSession()->IsBot())
                continue;

            bots.push_back(member);
        }

        if (bots.empty())
            return true;

        // 对每个机器人：修复条件 + 刷新LFG缓存
        for (Player* bot : bots)
        {
            // By leewheel 2026-07-21: 移除冷却/逃兵光环（同情况1）
            if (bot->HasAura(lfg::LFG_SPELL_DUNGEON_COOLDOWN))
                bot->RemoveAura(lfg::LFG_SPELL_DUNGEON_COOLDOWN);
            if (bot->HasAura(lfg::LFG_SPELL_DUNGEON_DESERTER))
                bot->RemoveAura(lfg::LFG_SPELL_DUNGEON_DESERTER);
            // End By leewheel

            // 1. 检查并修复所有副本的进入条件（钥匙/任务/成就/等级）
            //    遍历所有副本地图，给机器人补全缺少的条件
            FixAllDungeonRequirements(bot, player);

            // 2. 重新生成LFG锁定副本缓存
            //    这一步是关键：LFG系统在登录时缓存了锁定列表，
            //    现在条件已修复，必须重新生成缓存才能反映最新状态
            sLFGMgr->InitializeLockedDungeons(bot, group);
        }

        LOG_INFO("playerbots", "LFG排队修复：玩家 {} 排队LFG，已检查并修复 {} 个机器人的副本进入条件。",
            player->GetName(), static_cast<uint32>(bots.size()));

        return true;
    }
    // End By leewheel

    bool OnPlayerBeforeTeleport(Player* player, uint32 mapid, float /*x*/, float /*y*/, float /*z*/,
                                float /*orientation*/, uint32 /*options*/, Unit* /*target*/) override
    {
        if (!player)
            return true;

        // 只处理真实玩家（主控玩家）的传送
        if (player->GetSession()->IsBot())
            return true;

        // 检查目标地图是否是副本
        MapEntry const* entry = sMapStore.LookupEntry(mapid);
        if (!entry || !entry->IsDungeon())
            return true;

        // 获取主控玩家的所有机器人
        PlayerbotMgr* mgr = GET_PLAYERBOT_MGR(player);
        if (!mgr)
            return true;

        // 收集所有需要检查的机器人GUID
        std::vector<ObjectGuid> botGuids;

        // 1. 快速组队的机器人
        std::vector<ObjectGuid> fastGroupBots = sFastGroupMgr.GetFastGroupBotGuids(player->GetGUID());
        for (auto const& g : fastGroupBots)
            botGuids.push_back(g);

        // 2. playerbotMgr 直接管理的机器人
        for (PlayerBotMap::const_iterator it = mgr->GetPlayerBotsBegin();
             it != mgr->GetPlayerBotsEnd(); ++it)
        {
            if (it->second)
            {
                bool found = false;
                for (ObjectGuid const& g : botGuids)
                {
                    if (g == it->first)
                    {
                        found = true;
                        break;
                    }
                }
                if (!found)
                    botGuids.push_back(it->first);
            }
        }

        // 为每个机器人检查并修复进入条件
        uint32 checkedCount = 0;
        for (ObjectGuid const& botGuid : botGuids)
        {
            Player* bot = ObjectAccessor::FindConnectedPlayer(botGuid);
            if (!bot || !bot->IsInWorld())
                continue;

            EnsureBotCanEnterMap(bot, mapid, player);
            ++checkedCount;
        }

        if (checkedCount > 0)
        {
            LOG_INFO("playerbots", "副本进入修复：玩家 {} 传送进副本(地图:{})，检查了 {} 个机器人的进入条件。",
                player->GetName(), mapid, checkedCount);
        }

        return true;
    }
};

// ============================================================================
// 注册脚本
// ============================================================================

void AddSC_BotInstanceEntryFixScripts()
{
    new BotInstanceEntryFixScript();
}
// End By leewheel