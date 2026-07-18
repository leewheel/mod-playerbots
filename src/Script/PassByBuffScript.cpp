/*
 * This file is part of the mod-playerbots module for AzerothCore.
 * Released under GNU GPL v2 license.
 */

// By leewheel 2026-07-17
// 路过增益系统：机器人在非战斗状态下路过真实玩家时，有随机概率给玩家上增益法术
// 当真实玩家给机器人上增益时，机器人会有多元化的随机反应（感谢/无视/嘲讽等）
// End By leewheel

#include "Playerbots.h"

#include "ObjectAccessor.h"
#include "Player.h"
#include "PlayerScript.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "RandomPlayerbotMgr.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "SpellAuras.h"
#include "SpellMgr.h"
#include "WorldScript.h"
#include "WorldSession.h"

#include <unordered_map>
#include <vector>

// ============================================================================
// 各职业可施放的增益法术名称（英文，与 spell_dbc Name_Lang_enUS 一致）
// playerbot 只认英文技能名称，中文显示在 GenericBuffUtils.cpp 中映射
// ============================================================================

namespace
{
    struct BuffEntry
    {
        const char* name;
    };

    // 每个职业可施放给路人的增益列表
    static std::unordered_map<uint32, std::vector<BuffEntry>> const classBuffs = {
        {CLASS_MAGE, {
            {"arcane intellect"},
        }},
        {CLASS_PRIEST, {
            {"power word: fortitude"},
            {"divine spirit"},
            {"shadow protection"},
        }},
        {CLASS_DRUID, {
            {"mark of the wild"},
        }},
        {CLASS_PALADIN, {
            {"blessing of kings"},
            {"blessing of might"},
            {"blessing of wisdom"},
        }},
    };

    // 判断一个法术是否为增益法术（用于收到buff时检测）
    bool IsBuffSpell(SpellInfo const* spellInfo)
    {
        if (!spellInfo)
            return false;

        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            AuraType aura = spellInfo->Effects[i].ApplyAuraName;
            // 常见增益 aura 类型
            if (aura == SPELL_AURA_MOD_STAT ||
                aura == SPELL_AURA_MOD_INCREASE_SPEED ||
                aura == SPELL_AURA_MOD_BASE_RESISTANCE_PCT ||
                aura == SPELL_AURA_MOD_RESISTANCE ||
                aura == SPELL_AURA_MOD_POWER_REGEN ||
                aura == SPELL_AURA_MOD_INCREASE_ENERGY ||
                aura == SPELL_AURA_MOD_PERCENT_STAT ||
                aura == SPELL_AURA_ADD_FLAT_MODIFIER ||
                aura == SPELL_AURA_MOD_ATTACK_POWER)
            {
                return true;
            }
        }

        return false;
    }

    // 路过增益的冷却记录：每个机器人对每个玩家的冷却时间
    // key = botGUID.GetCounter() * 1000000 + playerGUID.GetCounter()
    // value = 时间戳
    static std::unordered_map<uint64, time_t> buffCooldownMap;

    // 收到buff反应的冷却记录：每个机器人的上次反应时间
    static std::unordered_map<uint64, time_t> thankCooldownMap;

    // 路过增益冷却时间（秒）
    constexpr time_t PASSBY_BUFF_COOLDOWN = 120;

    // 收到buff反应冷却时间（秒）
    constexpr time_t THANK_COOLDOWN = 30;

    // WorldScript 累积计时器
    uint32 passByBuffAccumDiff = 0;
    constexpr uint32 PASSBY_BUFF_TICK_MS = 10000;  // 每10秒检查一次

    // 多元化反应文本
    struct ReactionText
    {
        std::string text;
        uint32 emote;    // EMOTE_ONESHOT_* 常量，0 = 不使用
        int32 chance;    // 权重
    };

    // 收到真实玩家增益时的反应列表
    static std::vector<ReactionText> const buffReactions = {
        // 感谢类（40%）
        {"谢谢你，朋友！",                         EMOTE_ONESHOT_BOW,     20},
        {"多谢多谢！",                             EMOTE_ONESHOT_BOW,     10},
        {"太好了，正需要这个。",                   EMOTE_ONESHOT_EXCLAMATION, 10},
        // 礼貌性致意（20%）
        {"嗯。",                                   EMOTE_ONESHOT_EXCLAMATION, 10},
        {"收到了。",                               EMOTE_ONESHOT_EXCLAMATION, 10},
        // 置之不理（15%）
        {"",                                       0,                     15},
        // 有点不耐烦（10%）
        {"……我本来就想自己加的。",               EMOTE_ONESHOT_QUESTION,  5},
        {"不用了，我自己的还没断。",               0,                     5},
        // 冷嘲热讽（15%）
        {"哟，难得你还有这闲心。",                 EMOTE_ONESHOT_LAUGH,   8},
        {"行行行，你是个好萨满……哦你不是。",       EMOTE_ONESHOT_LAUGH,   4},
        {"哈，这也叫增益？",                       EMOTE_ONESHOT_RUDE,    3},
    };

    int32 GetTotalReactionWeight()
    {
        int32 total = 0;
        for (auto const& r : buffReactions)
            total += r.chance;

        return total > 0 ? total : 1;
    }

    ReactionText const& PickRandomReaction()
    {
        int32 roll = irand(1, GetTotalReactionWeight());
        int32 accum = 0;
        for (auto const& r : buffReactions)
        {
            accum += r.chance;
            if (roll <= accum)
                return r;
        }

        return buffReactions[0];
    }
}

// ============================================================================
// WorldScript：定期检查所有在线机器人，路过时给真实玩家上buff
// ============================================================================

class PassByBuffWorldScript : public WorldScript
{
public:
    PassByBuffWorldScript() : WorldScript("PassByBuffWorldScript", {WORLDHOOK_ON_UPDATE}) {}

    void OnUpdate(uint32 diff) override
    {
        passByBuffAccumDiff += diff;
        if (passByBuffAccumDiff < PASSBY_BUFF_TICK_MS)
            return;

        passByBuffAccumDiff = 0;

        // 获取所有在线随机机器人
        PlayerBotMap const& allBots = sRandomPlayerbotMgr.GetAllBots();

        for (auto const& [guid, bot] : allBots)
        {
            if (!bot || !bot->IsInWorld() || !bot->IsAlive())
                continue;

            // 跳过战斗中、战场、竞技场、飞行、上坐骑的
            if (bot->IsInCombat() || bot->InBattleground() || bot->InArena())
                continue;

            if (bot->IsInFlight() || bot->IsMounted())
                continue;

            // 只有随机机器人才执行
            if (!sRandomPlayerbotMgr.IsRandomBot(bot))
                continue;

            // 随机概率：只有20%的概率尝试给路过玩家上buff
            if (!urand(0, 4))
                continue;

            PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
            if (!botAI || !botAI->AllowActivity())
                continue;

            // 非战斗状态才执行
            if (botAI->GetState() != BOT_STATE_NON_COMBAT)
                continue;

            // 获取该职业可施放的增益列表
            uint32 cls = bot->getClass();
            auto it = classBuffs.find(cls);
            if (it == classBuffs.end())
                continue;

            // 找附近30码内的真实玩家
            Player* nearestPlayer = nullptr;
            float nearestDist = 30.0f;

            auto const& allPlayers = ObjectAccessor::GetPlayers();
            for (auto const& [playerGuid, player] : allPlayers)
            {
                if (!player || !player->IsInWorld() || !player->IsAlive())
                    continue;

                // 跳过机器人本身
                if (player == bot)
                    continue;

                // 只给真实玩家上buff（非机器人会话）
                if (player->GetSession()->IsBot())
                    continue;

                // 必须在同一张地图
                if (player->GetMap() != bot->GetMap())
                    continue;

                // 阵营检查
                if (player->GetTeamId() != bot->GetTeamId())
                    continue;

                float dist = bot->GetDistance(player);
                if (dist < nearestDist)
                {
                    // 检查冷却
                    uint64 cdKey = (uint64)bot->GetGUID().GetCounter() * 1000000ULL +
                                  player->GetGUID().GetCounter();

                    auto cdIt = buffCooldownMap.find(cdKey);
                    if (cdIt != buffCooldownMap.end())
                    {
                        if (time(nullptr) - cdIt->second < PASSBY_BUFF_COOLDOWN)
                            continue;
                    }

                    nearestPlayer = player;
                    nearestDist = dist;
                }
            }

            if (!nearestPlayer)
                continue;

            // 尝试给玩家上buff
            for (auto const& buff : it->second)
            {
                // 检查玩家是否已经有这个buff
                if (botAI->HasAura(buff.name, nearestPlayer))
                    continue;

                // 检查机器人是否会这个法术
                uint32 spellId = botAI->GetAiObjectContext()
                    ->GetValue<uint32>("spell id", buff.name)->Get();

                if (!spellId || !bot->HasSpell(spellId))
                    continue;

                // 检查法力是否足够
                if (!botAI->CanCastSpell(buff.name, nearestPlayer))
                    continue;

                // 施放增益
                if (botAI->CastSpell(buff.name, nearestPlayer))
                {
                    // 设置冷却
                    uint64 cdKey = (uint64)bot->GetGUID().GetCounter() * 1000000ULL +
                                  nearestPlayer->GetGUID().GetCounter();
                    buffCooldownMap[cdKey] = time(nullptr);

                    LOG_DEBUG("playerbots", "路过增益：机器人 {} 给玩家 {} 施放了增益。",
                        bot->GetName(), nearestPlayer->GetName());

                    break;  // 每次只上一个buff
                }
            }
        }
    }
};

// ============================================================================
// PlayerScript：当真实玩家给机器人上增益时，机器人有多元化随机反应
// ============================================================================

class PassByBuffPlayerScript : public PlayerScript
{
public:
    PassByBuffPlayerScript() : PlayerScript("PassByBuffPlayerScript", {
        PLAYERHOOK_ON_SPELL_CAST
    }) {}

    void OnPlayerSpellCast(Player* caster, Spell* spell, bool /*skipCheck*/) override
    {
        if (!caster || !spell)
            return;

        // 只有真实玩家（非机器人会话）施法时才触发
        if (caster->GetSession()->IsBot())
            return;

        // 获取法术目标
        Unit* target = spell->m_targets.GetUnitTarget();
        if (!target || !target->IsPlayer())
            return;

        // 目标必须是机器人
        Player* botPlayer = target->ToPlayer();
        if (!botPlayer || !botPlayer->GetSession()->IsBot())
            return;

        // 检查法术是否为增益
        SpellInfo const* spellInfo = spell->GetSpellInfo();
        if (!spellInfo || !IsBuffSpell(spellInfo))
            return;

        // 获取机器人AI
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(botPlayer);
        if (!botAI || !botAI->AllowActivity())
            return;

        // 跳过战斗、死亡、战场中的机器人
        if (botPlayer->IsInCombat() || botPlayer->InBattleground() || botPlayer->InArena())
            return;

        // 检查冷却
        uint64 cdKey = botPlayer->GetGUID().GetCounter();
        auto cdIt = thankCooldownMap.find(cdKey);
        if (cdIt != thankCooldownMap.end())
        {
            if (time(nullptr) - cdIt->second < THANK_COOLDOWN)
                return;
        }

        // 设置冷却
        thankCooldownMap[cdKey] = time(nullptr);

        // 随机选择反应
        ReactionText const& reaction = PickRandomReaction();

        // 如果是空文本且没有表情，直接返回（置之不理）
        if (reaction.text.empty() && reaction.emote == 0)
            return;

        // 面向施法者
        if (!botPlayer->isMoving())
            botPlayer->SetFacingToObject(caster);

        // 说话
        if (!reaction.text.empty())
            botAI->Say(reaction.text);

        // 表情
        if (reaction.emote != 0)
            botPlayer->HandleEmoteCommand(reaction.emote);

        LOG_DEBUG("playerbots", "路过增益反应：机器人 {} 收到玩家 {} 的增益，反应：{}",
            botPlayer->GetName(), caster->GetName(),
            reaction.text.empty() ? "(沉默)" : reaction.text);
    }
};

// ============================================================================
// 注册脚本
// ============================================================================

void AddSC_PassByBuffScripts()
{
    new PassByBuffWorldScript();
    new PassByBuffPlayerScript();
}
