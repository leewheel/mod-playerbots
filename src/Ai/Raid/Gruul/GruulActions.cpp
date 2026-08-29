/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GruulActions.h"
#include "CreatureAI.h"
#include "EncounterHelpers.h"
#include "GruulHelpers.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"
#include <algorithm>
#include <iterator>
#include <list>
#include <vector>

using namespace GruulHelpers;
using namespace EncounterHelpers;

// General

bool GruulsLairResetEncounterStatesAction::Execute(Event /*event*/)
{
    // 合并brighton 2026-08-26: 不再依赖find target守卫, 直接统一重置图标与分散走位初始位置
    //By leewheel 2026年8月26日
    bool reset = false;

    if (!AI_VALUE2(bool, "combat", "self target"))
    {
        reset |= ClearTargetIcon(bot, RtiTargetValue::skullIndex);
        reset |= ClearTargetIcon(bot, RtiTargetValue::crossIndex);
    }

    Action* action = context->GetAction("gruul the dragonkiller spread ranged");
    if (action &&
        static_cast<GruulTheDragonkillerSpreadRangedAction*>(action)->ResetInitialPosition())
    {
        reset = true;
    }

    return reset;
    //End By leewheel
}

// High King Maulgar

bool HighKingMaulgarMeleeTanksPositionBossesAction::Execute(Event /*event*/)
{
    Unit* target = nullptr;
    Position position;
    if (IsMaulgarTank(bot))
    {
        target = AI_VALUE2(Unit*, "find target", "18831");
        position = MAULGAR_TANK_POSITION;
    }
    else if (IsOlmTank(bot))
    {
        target = AI_VALUE2(Unit*, "find target", "18834");
        position = OLM_TANK_POSITION;
    }
    else if (IsBlindeyeTank(bot))
    {
        target = AI_VALUE2(Unit*, "find target", "18836");
        position = BLINDEYE_TANK_POSITION;
    }

    if (!target)
        return false;

    if (AI_VALUE(Unit*, "current target") != target)
        return Attack(target);

    if (target->GetVictim() != bot || !bot->IsWithinMeleeRange(target))
        return false;

    float const distToPosition = bot->GetExactDist2d(position);
    if (distToPosition <= 3.0f)
        return false;

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    float const toPosX = position.GetPositionX() - botX;
    float const toPosY = position.GetPositionY() - botY;

    float const toBossX = target->GetPositionX() - botX;
    float const toBossY = target->GetPositionY() - botY;
    bool const backwards = (toPosX * toBossX + toPosY * toBossY) < 0.0f;

    float const maxMoveDist = backwards ? 2.25f : 3.5f;
    float const moveDist = std::min(maxMoveDist, distToPosition);
    float const moveX = botX + (toPosX / distToPosition) * moveDist;
    float const moveY = botY + (toPosY / distToPosition) * moveDist;

    return MoveTo(
        GRUUL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool HighKingMaulgarMageTankAttackKroshAction::Execute(Event /*event*/)
{
    Unit* krosh = AI_VALUE2(Unit*, "find target", "18832");
    if (!krosh)
        return false;

    if (AttackAndCast(krosh))
        return true;

    if (krosh->GetVictim() != bot)
        return false;

    return MoveToDesiredDistance(krosh);
}

bool HighKingMaulgarMageTankAttackKroshAction::AttackAndCast(Unit* krosh)
{
    if (krosh->HasAura(Id(GruulSpells::SPELL_SPELL_SHIELD)) &&
        botAI->CanCastSpell(Id(GruulSpells::SPELL_SPELLSTEAL), krosh) &&
        botAI->CastSpell(Id(GruulSpells::SPELL_SPELLSTEAL), krosh))
    {
        return true;
    }

    if (AI_VALUE(Unit*, "current target") != krosh)
        return Attack(krosh);

    if (!bot->HasAura(Id(GruulSpells::SPELL_SPELL_SHIELD)) &&
        botAI->CanCastSpell("fire ward", bot))
    {
        return botAI->CastSpell("fire ward", bot);
    }

    return false;
}

// The Mage tank moves to a designated position only if Krosh is 17-30y from the position in order
// to tank Krosh without being in Blast Wave range.
bool HighKingMaulgarMageTankAttackKroshAction::MoveToDesiredDistance(Unit* krosh)
{
    Position const& position = KROSH_TANK_POSITION;
    float const distanceKroshToPosition = krosh->GetExactDist2d(position);
    constexpr float minDistance = 17.0f;
    constexpr float maxDistance = 30.0f;

    if (distanceKroshToPosition > minDistance && distanceKroshToPosition < maxDistance &&
        bot->GetExactDist2d(position) > 1.0f)
    {
        return MoveTo(
            GRUUL_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
            false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    constexpr float safeDistance = 15.0f;
    float const currentDistance = bot->GetDistance(krosh);

    if (currentDistance >= safeDistance)
        return false;

    bot->CastStop();
    return MoveAway(krosh, safeDistance - currentDistance);
}

// The moonkin tank has no tank position, but usually Kiggler remains close to where he starts.
bool HighKingMaulgarMoonkinTankAttackKigglerAction::Execute(Event /*event*/)
{
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "18835");
    if (!kiggler)
        return false;

    if (AI_VALUE(Unit*, "current target") != kiggler)
        return Attack(kiggler);

    if (kiggler->GetVictim() != bot)
        return false;

    constexpr float safeDistance = 28.5f;
    float const currentDistance = bot->GetDistance(kiggler);

    if (currentDistance >= safeDistance)
        return false;

    return MoveAway(kiggler, safeDistance - currentDistance);
}

// Priority: (1) Blindeye, (2) Olm, (3) Krosh (ranged only), (4) Kiggler, and (5) Maulgar
bool HighKingMaulgarAssignDpsPriorityAction::Execute(Event /*event*/)
{
    Unit* target = AI_VALUE2(Unit*, "find target", "18836");
    Unit* krosh = nullptr;
    if (Unit* blindeye = AI_VALUE2(Unit*, "find target", "18831"))
    {
        target = blindeye;
    }
    else if (Unit* olm = AI_VALUE2(Unit*, "find target", "18834"))
    {
        target = olm;
    }
    else if ((krosh = AI_VALUE2(Unit*, "find target", "18832")) &&
        PlayerbotAI::IsRanged(bot))
    {
        target = krosh;
    }
    else if (Unit* kiggler = AI_VALUE2(Unit*, "find target", "18835"))
    {
        target = kiggler;
    }

    if (!target)
        return false;

    if (target == krosh)
    {
        if (MarkTargetWithCross(bot, target))
            return true;
    }
    else if (MarkTargetWithSkull(bot, target))
    {
        return true;
    }

    return AI_VALUE(Unit*, "current target") != target && Attack(target);
}

bool HighKingMaulgarRunAwayFromWhirlwindAction::Execute(Event /*event*/)
{
    // By leewheel 2026-08-29 修复：旋风斩是 High King Maulgar(18836) 的技能，
    // 旧代码误用 18831(Blindeye the Seer) 导致机器人对着错误的boss躲避走位
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "18836");
    // End By leewheel
    if (!maulgar)
        return false;

    float const currentDistance = bot->GetDistance2d(maulgar);
    if (currentDistance >= WHIRLWIND_SAFE_DISTANCE)
        return false;

    bot->CastStop();
    return MoveAway(maulgar, WHIRLWIND_SAFE_DISTANCE - currentDistance);
}

bool HighKingMaulgarFleeFromBlastWaveDangerAction::Execute(Event /*event*/)
{
    Unit* krosh = AI_VALUE2(Unit*, "find target", "18832");
    if (!krosh)
        return false;

    constexpr float safeDistance = 20.0f;
    float const currentDistance = bot->GetDistance2d(krosh);

    if (currentDistance >= safeDistance)
        return false;

    bot->CastStop();
    return FleePosition(krosh->GetPosition(), safeDistance);
}

bool HighKingMaulgarBanishFelStalkerAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Unit*> felStalkers;
    std::list<Creature*> creatureList;
    constexpr float searchRadius = 50.0f;
    bot->GetCreatureListWithEntryInGrid(
        creatureList, Id(GruulNpcs::NPC_WILD_FEL_STALKER), searchRadius);

    for (Creature* creature : creatureList)
    {
        if (creature && creature->IsAlive())
            felStalkers.push_back(creature);
    }

    std::vector<Player*> warlocks;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->GetMapId() == GRUUL_MAP_ID &&
            member->getClass() == CLASS_WARLOCK && GET_PLAYERBOT_AI(member))
        {
            warlocks.push_back(member);
        }
    }

    int warlockIndex = -1;
    for (size_t i = 0; i < warlocks.size(); ++i)
    {
        if (warlocks[i] == bot)
        {
            warlockIndex = static_cast<int>(i);
            break;
        }
    }

    if (warlockIndex < 0 || warlockIndex >= felStalkers.size())
        return false;

    Unit* assignedFelStalker = felStalkers[warlockIndex];
    if (!botAI->HasAura("banish", assignedFelStalker) &&
        botAI->CanCastSpell("banish", assignedFelStalker))
    {
        return botAI->CastSpell("banish", assignedFelStalker);
    }

    return false;
}

// Misdirect order: Blindeye, Olm, Kiggler, Krosh
bool HighKingMaulgarMisdirectOgresToTanksAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_HUNTER &&
            GET_PLAYERBOT_AI(member))
        {
            hunters.push_back(member);
        }

        if (hunters.size() >= 4)
            break;
    }

    int8 hunterIndex = -1;
    for (size_t i = 0; i < hunters.size(); ++i)
    {
        if (hunters[i] == bot)
        {
            hunterIndex = static_cast<int8>(i);
            break;
        }
    }
    if (hunterIndex == -1)
        return false;

    Unit* ogreTarget = nullptr;
    Player* tankTarget = nullptr;
    if (hunterIndex == 0)
    {
        //By leewheel 2026-08-26 合并：保留entry规则查找，采用对侧1参数助手坦克签名，变量名随周边上下文
        ogreTarget = AI_VALUE2(Unit*, "find target", "18836");
        tankTarget = GetGroupAssistTank(bot, 1);
    }
    else if (hunterIndex == 1)
    {
        //By leewheel 2026-08-29 合并：保留entry规则与本项目变量名，采用对侧直取助手坦克的简化写法(去除冗余循环)
        ogreTarget = AI_VALUE2(Unit*, "find target", "18834");
        tankTarget = GetGroupAssistTank(bot, 0);
    }
    else if (hunterIndex == 2)
    {
        ogreTarget = AI_VALUE2(Unit*, "find target", "18835");
        tankTarget = GetKigglerMoonkinTank(bot);
    }
    else if (hunterIndex == 3)
    {
        ogreTarget = AI_VALUE2(Unit*, "find target", "18832");
        tankTarget = GetKroshMageTank(bot);
    }

    if (!ogreTarget || !tankTarget || !tankTarget->IsAlive())
        return false;

    if (botAI->CanCastSpell("misdirection", tankTarget))
        return botAI->CastSpell("misdirection", tankTarget);

    if (bot->HasAura(Id(GruulSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", ogreTarget))
    {
        return botAI->CastSpell("steady shot", ogreTarget);
    }

    return false;
}

bool HighKingMaulgarCastFearWardOnMainTankAction::Execute(Event /*event*/)
{
    constexpr uint32 fearWard = Id(GruulSpells::SPELL_FEAR_WARD);
    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank || mainTank->HasAura(fearWard))
        return false;

    return botAI->CanCastSpell(fearWard, mainTank) && botAI->CastSpell(fearWard, mainTank);
}

// Gruul the Dragonkiller

bool GruulTheDragonkillerTanksPositionBossAction::Execute(Event /*event*/)
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "19044");
    if (!gruul)
        return false;

    if (AI_VALUE(Unit*, "current target") != gruul)
        return Attack(gruul);

    if (gruul->GetVictim() != bot || !bot->IsWithinMeleeRange(gruul))
        return false;

    Position const& position = GRUUL_TANK_POSITION;
    float const distToPosition = bot->GetExactDist2d(position);

    if (distToPosition <= 3.0f)
        return false;

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    float const toPosX = position.GetPositionX() - botX;
    float const toPosY = position.GetPositionY() - botY;

    float const toBossX = gruul->GetPositionX() - botX;
    float const toBossY = gruul->GetPositionY() - botY;
    bool const backwards = (toPosX * toBossX + toPosY * toBossY) < 0.0f;

    float const maxMoveDist = backwards ? 2.25f : 3.5f;
    float const moveDist = std::min(maxMoveDist, distToPosition);
    float const moveX = botX + (toPosX / distToPosition) * moveDist;
    float const moveY = botY + (toPosY / distToPosition) * moveDist;

    return MoveTo(
        GRUUL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool GruulTheDragonkillerSpreadRangedAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Position const& position = GRUUL_TANK_POSITION;

    if (!_hasInitialPosition)
    {
        std::vector<Player*> members;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive())
                continue;

            members.push_back(member);
        }

        auto it = std::find(members.begin(), members.end(), bot);
        uint8 botIndex = (it != members.end()) ? std::distance(members.begin(), it) : 0;
        uint8 count = members.size();

        constexpr float minRadius = 25.0f;
        constexpr float maxRadius = 40.0f;
        float angle = 2 * M_PI * botIndex / count;
        float radius = minRadius + static_cast<float>(rand()) /
            static_cast<float>(RAND_MAX) * (maxRadius - minRadius);
        float targetX = position.GetPositionX() + radius * cos(angle);
        float targetY = position.GetPositionY() + radius * sin(angle);

        _initialPosition = Position(targetX, targetY, position.GetPositionZ());
        _hasInitialPosition = true;
    }

    if (!_hasReachedInitialPosition)
    {
        float const distToTarget = bot->GetExactDist2d(_initialPosition);
        if (distToTarget <= 2.0f)
        {
             _hasReachedInitialPosition = true;
            return false;
        }

        float const moveDist = std::min(3.5f, distToTarget);
        float const botX = bot->GetPositionX();
        float const botY = bot->GetPositionY();
        float const moveX =
            botX + ((_initialPosition.GetPositionX() - botX) / distToTarget) * moveDist;
        float const moveY =
            botY + ((_initialPosition.GetPositionY() - botY) / distToTarget) * moveDist;

        return MoveTo(
            GRUUL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    constexpr float minSpreadDistance = 10.0f;
    Player* nearestPlayer = GetNearestPlayerInRadius(bot, minSpreadDistance);
    return nearestPlayer && FleePosition(nearestPlayer->GetPosition(), minSpreadDistance);
}

bool GruulTheDragonkillerShatterSpreadAction::Execute(Event /*event*/)
{
    constexpr float safeDistance = 20.0f;
    Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance);
    if (!nearestPlayer)
        return false;

    float const distToNearest = bot->GetExactDist2d(nearestPlayer);
    float const moveDist = std::min(3.5f, safeDistance - distToNearest);

    return MoveAway(nearestPlayer, moveDist);
}
