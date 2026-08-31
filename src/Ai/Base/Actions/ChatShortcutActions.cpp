/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ChatShortcutActions.h"
#include "Event.h"
#include "Formations.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"
#include "PositionValue.h"
#include "PullStrategy.h"

void PositionsResetAction::ResetReturnPosition()
{
    PositionMap& posMap = context->GetValue<PositionMap&>("position")->Get();
    PositionInfo pos = posMap["return"];
    pos.Reset();
    posMap["return"] = pos;
}

void PositionsResetAction::SetReturnPosition(float x, float y, float z)
{
    PositionMap& posMap = context->GetValue<PositionMap&>("position")->Get();
    PositionInfo pos = posMap["return"];
    pos.Set(x, y, z, botAI->GetBot()->GetMapId());
    posMap["return"] = pos;
}

void PositionsResetAction::ResetStayPosition()
{
    PositionMap& posMap = context->GetValue<PositionMap&>("position")->Get();
    PositionInfo pos = posMap["stay"];
    pos.Reset();
    posMap["stay"] = pos;
}

void PositionsResetAction::SetStayPosition(float x, float y, float z)
{
    PositionMap& posMap = context->GetValue<PositionMap&>("position")->Get();
    PositionInfo pos = posMap["stay"];
    pos.Set(x, y, z, botAI->GetBot()->GetMapId());
    posMap["stay"] = pos;
}

bool FollowChatShortcutAction::Execute(Event /*event*/)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    // botAI->Reset();
    botAI->ChangeStrategy("+follow,-passive,-grind,-move from group", BOT_STATE_NON_COMBAT);
    botAI->ChangeStrategy("-stay,-follow,-passive,-grind,-move from group", BOT_STATE_COMBAT);
    botAI->GetAiObjectContext()->GetValue<GuidVector>("prioritized targets")->Reset();

    PositionMap& posMap = context->GetValue<PositionMap&>("position")->Get();
    PositionInfo pos = posMap["return"];
    pos.Reset();
    posMap["return"] = pos;

    pos = posMap["stay"];
    pos.Reset();
    posMap["stay"] = pos;

    if (bot->IsInCombat())
    {
        Formation* formation = AI_VALUE(Formation*, "formation");
        std::string const target = formation->GetTargetName();
        bool moved = false;
        if (!target.empty())
            moved = Follow(AI_VALUE(Unit*, target));
        else
        {
            WorldLocation loc = formation->GetLocation();
            if (Formation::IsNullLocation(loc) || loc.GetMapId() == MAPID_INVALID)
                return false;

            MovementPriority priority = botAI->GetState() == BOT_STATE_COMBAT ? MovementPriority::MOVEMENT_COMBAT : MovementPriority::MOVEMENT_NORMAL;
            moved = MoveTo(loc.GetMapId(), loc.GetPositionX(), loc.GetPositionY(), loc.GetPositionZ(), false, false, false,
                        true, priority);
        }

        if (bot->GetPet())
            botAI->PetFollow();

        if (moved)
        {
            botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "following", "跟随中", {}));
            return true;
        }
    }

    /* Default mechanics takes care of this now.
    if (bot->GetMapId() != master->GetMapId() || (master && bot->GetDistance(master) >
    sPlayerbotAIConfig.sightDistance))
    {
        if (bot->isDead())
        {
            bot->ResurrectPlayer(1.0f, false);
            botAI->TellMasterNoFacing("Back from the grave!");
        }
        else
            botAI->TellMaster("你离我太远了！我很快就到。");

        bot->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_TELEPORTED | AURA_INTERRUPT_FLAG_CHANGE_MAP);
        bot->TeleportTo(master->GetMapId(), master->GetPositionX(), master->GetPositionY(), master->GetPositionZ(),
    master->GetOrientation()); return true;
    }
    */

    botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "following", "跟随中", {}));
    return true;
}

bool StayChatShortcutAction::Execute(Event /*event*/)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    botAI->Reset();
    botAI->ChangeStrategy("+stay,-passive,-move from group", BOT_STATE_NON_COMBAT);
    botAI->ChangeStrategy("+stay,-follow,-passive,-move from group", BOT_STATE_COMBAT);

    SetReturnPosition(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
    SetStayPosition(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());

    botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "staying", "停留中", {}));
    return true;
}

bool MoveFromGroupChatShortcutAction::Execute(Event /*event*/)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    // dont need to remove stay or follow, move from group takes priority over both
    // (see their isUseful() methods)
    botAI->ChangeStrategy("+move from group", BOT_STATE_NON_COMBAT);
    botAI->ChangeStrategy("+move from group", BOT_STATE_COMBAT);

    botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "move_from_group", "正在远离队伍", {}));
    return true;
}

bool FleeChatShortcutAction::Execute(Event /*event*/)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    botAI->Reset();
    botAI->ChangeStrategy("+follow,-stay,+passive", BOT_STATE_NON_COMBAT);
    botAI->ChangeStrategy("+follow,-stay,+passive", BOT_STATE_COMBAT);

    ResetReturnPosition();
    ResetStayPosition();

    if (bot->GetMapId() != master->GetMapId() || bot->GetDistance(master) > sPlayerbotAIConfig.sightDistance)
    {
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "fleeing_far", "距离太远，我不会与你一起逃跑", {}));
        return true;
    }

    botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "fleeing", "逃跑中", {}));
    return true;
}

bool GoawayChatShortcutAction::Execute(Event /*event*/)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    botAI->Reset();
    botAI->ChangeStrategy("+runaway,-stay", BOT_STATE_NON_COMBAT);
    botAI->ChangeStrategy("+runaway,-stay", BOT_STATE_COMBAT);

    ResetReturnPosition();
    ResetStayPosition();

    botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "running_away", "正在逃离", {}));
    return true;
}

bool GrindChatShortcutAction::Execute(Event /*event*/)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    botAI->Reset();
    botAI->ChangeStrategy("+grind,-passive,-stay", BOT_STATE_NON_COMBAT);

    ResetReturnPosition();
    ResetStayPosition();

    botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "grinding", "刷怪中", {}));
    return true;
}

bool TankAttackChatShortcutAction::Execute(Event /*event*/)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    if (!botAI->IsTank(bot))
        return false;

    botAI->Reset();
    botAI->ChangeStrategy("-passive", BOT_STATE_NON_COMBAT);
    botAI->ChangeStrategy("-passive", BOT_STATE_COMBAT);

    ResetReturnPosition();
    ResetStayPosition();

    //By leewheel 2026-08-05 修复：坦克攻击命令不设置攻击目标
    //  原因：原实现只移除passive策略，bot之后自行选目标，未必攻击玩家选择的怪
    //  修复：把玩家当前选择的目标设为优先攻击目标(与"攻击"命令逻辑一致)
    //  注意：必须在 botAI->Reset() 之后设置，否则会被Reset清空
    //  补充1：设置前校验目标存在且为有效攻击目标，避免对友方/无效目标执行拉怪
    //  补充2(2026-08-05 security review)：对齐AttackAction::Attack的死亡/PvP禁区校验
    ObjectGuid targetGuid = master->GetTarget();
    if (!targetGuid)
    {
        //By leewheel 2026-08-05 修复: 未选中目标时明确提示(与"攻击"命令语义一致)
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "attack_target_missing_error", "请先选中要攻击的目标。", {}));
        return false;
        //End By leewheel
    }

    //By leewheel 2026-08-15 简化: 上面已对空目标return, 此处无需再判(原冗余if移除)
    Unit* target = botAI->GetUnit(targetGuid);
    if (!target || !target->IsInWorld())
    {
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "attack_target_not_in_world_error", "目标已不在世界中。", {}));
        return false;
    }
    // 死亡目标不可攻击(与AttackAction一致)
    if (target->isDead())
    {
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "attack_target_dead_error", "目标已死亡。", {}));
        return false;
    }
    // 禁PvP区域不可攻击玩家/宠物(与AttackAction一致, 决斗除外)
    if ((target->IsPlayer() || target->IsPet()) &&
        (!bot->duel || bot->duel->Opponent != target) &&
        (sPlayerbotAIConfig.IsPvpProhibited(bot->GetZoneId(), bot->GetAreaId()) ||
         sPlayerbotAIConfig.IsPvpProhibited(target->GetZoneId(), target->GetAreaId())))
    {
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "attack_pvp_prohibited_error", "在禁止 PvP 的区域无法攻击其他玩家。", {}));
        return false;
    }

    // 目标必须可攻击(敌对/有效), 与AttackAction一致
    if (!bot->IsValidAttackTarget(target))
    {
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "attack_target_friendly_error", "该目标无法攻击(友方或无效目标)。", {}));
        return false;
    }

    // 设置为优先攻击目标
    botAI->GetAiObjectContext()->GetValue<GuidVector>("prioritized targets")->Set({targetGuid});
    botAI->GetAiObjectContext()->GetValue<ObjectGuid>("pull target")->Set(targetGuid);

    //By leewheel 2026-08-31 修复: 坦克开怪只设目标不拉怪, 坦克原地开打且DPS抢仇恨
    //  修复1: 发起完整拉怪流程, 拉怪点设为主人位置, 坦克把怪拉回队伍再打
    //  修复2: 给队伍DPS启用"等待攻击", 等坦克建立仇恨后再输出
    if (target->GetMapId() == bot->GetMapId())
    {
        botAI->ChangeStrategy("+pull,+pull back", BOT_STATE_COMBAT);

        PullStrategy* pullStrategy = PullStrategy::Get(botAI);
        if (pullStrategy && pullStrategy->CanDoPullAction(target))
        {
            PositionMap& posMap = context->GetValue<PositionMap&>("position")->Get();
            PositionInfo pullPosition = posMap["pull"];
            pullPosition.Set(master->GetPositionX(), master->GetPositionY(), master->GetPositionZ(), master->GetMapId());
            posMap["pull"] = pullPosition;

            pullStrategy->RequestPull(target);
            context->GetValue<Unit*>("current target")->Set(target);
            botAI->ChangeEngine(BOT_STATE_COMBAT);
            botAI->SetNextCheckDelay(sPlayerbotAIConfig.reactDelay);
        }

        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (!member || member == bot || member == master)
                    continue;

                PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
                if (!memberAI || !PlayerbotAI::IsDps(member, true) || PlayerbotAI::IsTank(member, true))
                    continue;

                memberAI->ChangeStrategy("+wait for attack", BOT_STATE_COMBAT);
                AiObjectContext* memberContext = memberAI->GetAiObjectContext();
                memberContext->GetValue<uint8>("wait for attack time")->Set(10);
                memberContext->GetValue<time_t>("combat start time")->Set(0);
            }
        }
    }
    //End By leewheel

    if (verbose)
        botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "attacking", "攻击中", {}));
    return true;
    //End By leewheel
}

bool MaxDpsChatShortcutAction::Execute(Event /*event*/)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    if (!botAI->ContainsStrategy(STRATEGY_TYPE_DPS))
        return false;

    botAI->Reset();

    botAI->ChangeStrategy("-threat,-conserve mana,-cast time,+dps debuff,+boost", BOT_STATE_COMBAT);
    botAI->TellMaster("最大 DPS！");

    return true;
}

bool NaxxChatShortcutAction::Execute(Event /*event*/)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    botAI->Reset();
    botAI->ChangeStrategy("+naxx", BOT_STATE_NON_COMBAT);
    botAI->ChangeStrategy("+naxx", BOT_STATE_COMBAT);
    botAI->TellMasterNoFacing("已启用纳克萨玛斯策略！");
    // bot->Say("Add Naxx Strategies!", LANG_UNIVERSAL);
    return true;
}

bool BwlChatShortcutAction::Execute(Event /*event*/)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    botAI->Reset();
    botAI->ChangeStrategy("+bwl", BOT_STATE_NON_COMBAT);
    botAI->ChangeStrategy("+bwl", BOT_STATE_COMBAT);
    botAI->TellMasterNoFacing("已启用黑翼之巢策略！");
    return true;
}
