/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ReachTargetActions.h"
#include "Event.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "ServerFacade.h"

static constexpr float GROUP_SIGHT_SPOT_DISTANCE = 3.0f;

bool ReachTargetAction::Execute(Event /*event*/) { return ReachCombatTo(AI_VALUE(Unit*, GetTargetName()), distance); }

bool ReachTargetAction::isUseful()
{
    // do not move while staying
    if (botAI->HasStrategy("stay", botAI->GetState()))
    {
        return false;
    }

    // do not move while casting
    if (bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL) != nullptr)
    {
        return false;
    }
    Unit* target = GetTarget();
    // float dis = distance + CONTACT_DISTANCE;
    return target &&
           !bot->IsWithinCombatRange(target, distance);  // ServerFacade::instance().IsDistanceGreaterThan(AI_VALUE2(float,
                                                         // "distance", GetTargetName()), distance);
}

std::string const ReachTargetAction::GetTargetName() { return "current target"; }

bool CastReachTargetSpellAction::isUseful()
{
    // do not move while staying
    if (botAI->HasStrategy("stay", botAI->GetState()))
    {
        return false;
    }

    return ServerFacade::instance().IsDistanceGreaterThan(AI_VALUE2(float, "distance", "current target"),
                                                (distance + sPlayerbotAIConfig.contactDistance));
}

ReachSpellAction::ReachSpellAction(PlayerbotAI* botAI)
    : ReachTargetAction(botAI, "reach spell", botAI->GetRange("spell"))
{
}

bool ReachLineOfSightAction::Execute(Event /*event*/)
{
    Unit* target = GetTarget();
    if (!target || !IsMovingAllowed(target))
        return false;

    if (!PlayerbotAI::IsRanged(bot) || !bot->GetGroup())
        return MoveToLOS(target, PlayerbotAI::IsRanged(bot));

    // Walking towards the enemy can pull idle packs, a spot held by the group cannot
    Player* member = FindGroupMemberInSightOf(target);
    if (!member)
        return false;

    return MoveNear(member, GROUP_SIGHT_SPOT_DISTANCE, MovementPriority::MOVEMENT_COMBAT);
}

Player* ReachLineOfSightAction::FindGroupMemberInSightOf(Unit* target)
{
    Player* nearest = nullptr;
    float nearestDistance = sPlayerbotAIConfig.sightDistance;
    float range = botAI->GetRange("spell");

    for (GroupReference* ref = bot->GetGroup()->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive() || member->IsCharmed() ||
            member->GetMapId() != bot->GetMapId())
            continue;

        float distance = bot->GetExactDist(member);
        if (distance >= nearestDistance || distance <= GROUP_SIGHT_SPOT_DISTANCE)
            continue;

        if (!member->IsWithinCombatRange(target, range) || !member->IsWithinLOSInMap(target))
            continue;

        nearest = member;
        nearestDistance = distance;
    }

    return nearest;
}

bool ReachLineOfSightAction::isUseful()
{
    if (botAI->HasStrategy("stay", botAI->GetState()))
        return false;

    if (bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL) != nullptr)
        return false;

    Unit* target = GetTarget();
    return target && !bot->IsWithinLOSInMap(target);
}

std::string const ReachLineOfSightAction::GetTargetName() { return "current target"; }

ReachPartyMemberToHealAction::ReachPartyMemberToHealAction(PlayerbotAI* botAI)
    : ReachTargetAction(botAI, "reach party member to heal", botAI->GetRange("heal"))
{
}

bool ReachPartyMemberToHealAction::Execute(Event event)
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    if (sPlayerbotAIConfig.regainLineOfSight && bot->IsWithinCombatRange(target, distance) &&
        !bot->IsWithinLOSInMap(target))
        return IsMovingAllowed(target) && MoveToLOS(target, true);

    return ReachTargetAction::Execute(event);
}

bool ReachPartyMemberToHealAction::isUseful()
{
    if (ReachTargetAction::isUseful())
        return true;

    if (!sPlayerbotAIConfig.regainLineOfSight)
        return false;

    if (botAI->HasStrategy("stay", botAI->GetState()))
        return false;

    if (bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL) != nullptr)
        return false;

    Unit* target = GetTarget();
    return target && !bot->IsWithinLOSInMap(target);
}

std::string const ReachPartyMemberToHealAction::GetTargetName() { return "party member to heal"; }

ReachPartyMemberToResurrectAction::ReachPartyMemberToResurrectAction(PlayerbotAI* botAI)
    : ReachTargetAction(botAI, "reach party member to resurrect", botAI->GetRange("spell"))
{
}

std::string const ReachPartyMemberToResurrectAction::GetTargetName() { return "party member to resurrect"; }
