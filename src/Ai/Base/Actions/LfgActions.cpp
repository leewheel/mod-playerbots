/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "LfgActions.h"

#include "AiFactory.h"
#include "ItemVisitors.h"
#include "LFGMgr.h"
#include "Opcodes.h"
#include "Playerbots.h"
#include "World.h"
#include "WorldPacket.h"
#include "RandomPlayerbotMgr.h"

using namespace lfg;

bool LfgJoinAction::Execute(Event /*event*/) { return JoinLFG(); }

uint32 LfgJoinAction::GetRoles()
{
    // By leewheel 2026-07-15
    // 修复：非随机机器人（如FastGroup组队的Rndbot）的角色判断必须基于天赋(bySpec=true)，
    // 而非AI策略(bySpec=false)。
    // 根因：ApplyInstanceStrategies 给所有 Bot 添加了 "auto tank mark" 策略，
    //       该策略的 GetType() 原本返回 STRATEGY_TYPE_TANK，
    //       导致 ContainsStrategy(STRATEGY_TYPE_TANK) 对所有 Bot 返回 true，
    //       包括猎人和盗贼！IsTank(bot, false) 因此对所有 Bot 返回 true，
    //       GetRoles() 对所有 Bot 返回 PLAYER_ROLE_TANK。
    // 修复1（根因）：AutoTankMarkStrategy::GetType() 改为 STRATEGY_TYPE_GENERIC
    // 修复2（防御）：此处使用 bySpec=true 直接检查天赋页，不依赖策略状态
    if (!RandomPlayerbotMgr::instance().IsRandomBot(bot))
    {
        if (botAI->IsTank(bot, true))
            return PLAYER_ROLE_TANK;
        if (botAI->IsHeal(bot, true))
            return PLAYER_ROLE_HEALER;
        else
            return PLAYER_ROLE_DAMAGE;
    }
    // End By leewheel

    // By leewheel 2026-07-29
    // Feral 德鲁伊坦克检测 bug 修复：
    // 原代码要求 HasAura(16931 / Thick Hide) 或熊形态，但此光环仅在熊形态下存在。
    // Feral 德鲁伊在 caster/cat 形态下永远被判为 DPS，坦克 LFG 进组概率为 0。
    // 修复：移除形态/光环检测，仅按天赋页判断。Feral（spec==1）= 坦克专精，
    //       与 IsBotTank() 保持一致。bot 进组后会根据实际形态在副本内自动切熊。
    // End By leewheel
    uint8 spec = AiFactory::GetPlayerSpecTab(bot);
    switch (bot->getClass())
    {
        case CLASS_DRUID:
            if (spec == 2)
                return PLAYER_ROLE_HEALER;
            else if (spec == 1)
                return PLAYER_ROLE_TANK;
            else
                return PLAYER_ROLE_DAMAGE;
            break;
        case CLASS_PALADIN:
            if (spec == 1)
                return PLAYER_ROLE_TANK;
            else if (!spec)
                return PLAYER_ROLE_HEALER;
            else
                return PLAYER_ROLE_DAMAGE;
            break;
        case CLASS_PRIEST:
            if (spec != 2)
                return PLAYER_ROLE_HEALER;
            else
                return PLAYER_ROLE_DAMAGE;
            break;
        case CLASS_SHAMAN:
            if (spec == 2)
                return PLAYER_ROLE_HEALER;
            else
                return PLAYER_ROLE_DAMAGE;
            break;
        case CLASS_WARRIOR:
            if (spec == 2)
                return PLAYER_ROLE_TANK;
            else
                return PLAYER_ROLE_DAMAGE;
            break;
        case CLASS_DEATH_KNIGHT:
            if (spec == 0)
                return PLAYER_ROLE_TANK;
            else
                return PLAYER_ROLE_DAMAGE;
            break;

        default:
            return PLAYER_ROLE_DAMAGE;
            break;
    }

    return PLAYER_ROLE_DAMAGE;
}

bool LfgJoinAction::JoinLFG()
{
    // check if already in lfg
    LfgState state = sLFGMgr->GetState(bot->GetGUID());
    if (state != LFG_STATE_NONE)
        return false;

    /*ItemCountByQuality visitor;
    IterateItems(&visitor, ITERATE_ITEMS_IN_EQUIP);
    bool random = urand(0, 100) < 20;
    bool heroic = urand(0, 100) < 50 &&
                  (visitor.count[ITEM_QUALITY_EPIC] >= 3 || visitor.count[ITEM_QUALITY_RARE] >= 10) &&
                  bot->GetLevel() >= 70;
    bool rbotAId = !heroic && (urand(0, 100) < 50 && visitor.count[ITEM_QUALITY_EPIC] >= 5 &&
                               (bot->GetLevel() == 60 || bot->GetLevel() == 70 || bot->GetLevel() == 80));*/

    LfgDungeonSet list;
    std::vector<uint32> selected;

    std::vector<uint32> dungeons = RandomPlayerbotMgr::instance().LfgDungeons[bot->GetTeamId()];
    if (!dungeons.size())
        return false;

    for (std::vector<uint32>::iterator i = dungeons.begin(); i != dungeons.end(); ++i)
    {
        LFGDungeonEntry const* dungeon = sLFGDungeonStore.LookupEntry(*i);
        if (!dungeon || (dungeon->TypeID != LFG_TYPE_RANDOM && dungeon->TypeID != LFG_TYPE_DUNGEON &&
                         dungeon->TypeID != LFG_TYPE_HEROIC && dungeon->TypeID != LFG_TYPE_RAID))
            continue;

        auto const& botLevel = bot->GetLevel();

        /*LFG_TYPE_RANDOM on classic is 15-58 so bot over level 25 will never queue*/
        if ((dungeon->MinLevel && (botLevel < dungeon->MinLevel || botLevel > dungeon->MaxLevel)) ||
            (botLevel > dungeon->MinLevel + 10 && dungeon->TypeID == LFG_TYPE_DUNGEON))
            continue;

        selected.push_back(dungeon->ID);
        list.insert(dungeon->ID);
    }

    if (!selected.size())
        return false;

    if (list.empty())
        return false;

    bool many = list.size() > 1;
    LFGDungeonEntry const* dungeon = sLFGDungeonStore.LookupEntry(*list.begin());

    // check role for console msg
    std::string _roles = "multiple roles";
    uint32 roleMask = GetRoles();
    if (roleMask & PLAYER_ROLE_TANK)
        _roles = "TANK";

    if (roleMask & PLAYER_ROLE_HEALER)
        _roles = "HEAL";

    if (roleMask & PLAYER_ROLE_DAMAGE)
        _roles = "DPS";

    LOG_INFO("playerbots", "Bot {} {}:{} <{}>: queues LFG, Dungeon as {} ({})", bot->GetGUID().ToString().c_str(),
             bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName().c_str(), _roles,
             many ? "several dungeons" : dungeon->Name[0]);

    // Set RbotAId Browser comment
    // By leewheel 2026-07-29
    // 改回直接调 sLFGMgr->JoinLfg（参考 LiyunfanPlayerbotsBranch 的稳定实现）。
    // QueuePacket(CMSG_LFG_JOIN) 路径下 bot 入队后 state 被瞬间清回 NONE，
    // 导致 isUseful 每 22.5s 又触发一次 join，循环几十次始终进不了 QUEUED 状态。
    std::string const _gs = std::to_string(botAI->GetEquipGearScore(bot/*, false, false*/));
    sLFGMgr->JoinLfg(bot, roleMask, list, _gs);
    // End By leewheel

    return true;
}

bool LfgRoleCheckAction::Execute(Event /*event*/)
{
    if (Group* group = bot->GetGroup())
    {
        uint32 newRoles = GetRoles();
        // if (currentRoles == newRoles)
        //     return false;

        // By leewheel 2026-07-29
        // 改回直接调 sLFGMgr::SetRoles / UpdateRoleCheck（参考 LiyunfanPlayerbotsBranch）。
        sLFGMgr->SetRoles(bot->GetGUID(), newRoles);
        sLFGMgr->UpdateRoleCheck(group->GetGUID(), bot->GetGUID(), newRoles);
        // End By leewheel

        return true;
    }

    return false;
}

bool LfgAcceptAction::Execute(Event event)
{
    uint32 id = AI_VALUE(uint32, "lfg proposal");

    // Try accept if already stored
    if (id)
    {
        if (bot->IsInCombat() || bot->isDead())
        {
            sLFGMgr->UpdateProposal(id, bot->GetGUID(), false);
            return true;
        }

        botAI->GetAiObjectContext()->GetValue<uint32>("lfg proposal")->Set(0);
        bot->ClearUnitState(UNIT_STATE_ALL_STATE);

        // By leewheel 2026-07-29
        // 改回直接调 sLFGMgr::UpdateProposal（参考 LiyunfanPlayerbotsBranch）。
        sLFGMgr->UpdateProposal(id, bot->GetGUID(), true);
        // End By leewheel

        if (RandomPlayerbotMgr::instance().IsRandomBot(bot) && !bot->GetGroup())
        {
            RandomPlayerbotMgr::instance().Refresh(bot);
            botAI->ResetStrategies();
        }

        botAI->Reset();
        return true;
    }

    // If we get the proposal packet, accept immediately
    if (!event.getPacket().empty())
    {
        WorldPacket p(event.getPacket());
        uint32 dungeonId;
        uint8 state;
        p >> dungeonId >> state >> id;

        if (id)
        {
            if (bot->IsInCombat() || bot->isDead())
            {
                sLFGMgr->UpdateProposal(id, bot->GetGUID(), false);
                return true;
            }

            botAI->GetAiObjectContext()->GetValue<uint32>("lfg proposal")->Set(0);
            bot->ClearUnitState(UNIT_STATE_ALL_STATE);

            // By leewheel 2026-07-29
            // 改回直接调 sLFGMgr::UpdateProposal（参考 LiyunfanPlayerbotsBranch）。
            sLFGMgr->UpdateProposal(id, bot->GetGUID(), true);
            // End By leewheel

            if (RandomPlayerbotMgr::instance().IsRandomBot(bot) && !bot->GetGroup())
            {
                RandomPlayerbotMgr::instance().Refresh(bot);
                botAI->ResetStrategies();
            }

            botAI->Reset();
            return true;
        }
    }

    return false;
}

bool LfgLeaveAction::Execute(Event /*event*/)
{
    // Don't leave if lfg strategy enabled
    // if (botAI->HasStrategy("lfg", BOT_STATE_NON_COMBAT))
    //    return false;

    // Don't leave if already invited / in dungeon
    if (sLFGMgr->GetState(bot->GetGUID()) > LFG_STATE_QUEUED)
        return false;

    // By leewheel 2026-07-29
    // 改回直接调 sLFGMgr::LeaveLfg（参考 LiyunfanPlayerbotsBranch）。
    sLFGMgr->LeaveLfg(bot->GetGUID());
    // End By leewheel
    return true;
}

bool LfgLeaveAction::isUseful() { return true; }

bool LfgTeleportAction::Execute(Event event)
{
    bool out = false;

    WorldPacket p(event.getPacket());
    if (!p.empty())
    {
        p.rpos(0);
        p >> out;
    }

    bot->ClearUnitState(UNIT_STATE_ALL_STATE);

    // By leewheel 2026-07-29
    // 改回直接调 sLFGMgr::TeleportPlayer（参考 LiyunfanPlayerbotsBranch）。
    sLFGMgr->TeleportPlayer(bot, out);
    // End By leewheel

    return true;
}

bool LfgJoinAction::isUseful()
{
    if (!sPlayerbotAIConfig.randomBotJoinLfg)
    {
        // botAI->ChangeStrategy("-lfg", BOT_STATE_NON_COMBAT);
        return false;
    }

    if (bot->GetLevel() < 15)
        return false;

    // don't use if active player master
    if (GET_PLAYERBOT_AI(bot)->IsRealPlayer())
        return false;

    if (bot->GetGroup() && bot->GetGroup()->GetLeaderGUID() != bot->GetGUID())
    {
        // botAI->ChangeStrategy("-lfg", BOT_STATE_NON_COMBAT);
        return false;
    }

    if (bot->IsBeingTeleported())
        return false;

    if (bot->InBattleground())
        return false;

    if (bot->InBattlegroundQueue())
        return false;

    if (bot->isDead())
        return false;

    if (!RandomPlayerbotMgr::instance().IsRandomBot(bot))
        return false;

    Map* map = bot->GetMap();
    if (map && map->Instanceable())
        return false;

    LfgState state = sLFGMgr->GetState(bot->GetGUID());
    if (state != LFG_STATE_NONE)
        return false;

    return true;
}
