/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

// CoA bots in PvP. The generic arena tactics place a bot by its class, and every CoA bot is class 27: they all went
// to the melee spot. Here the role decides. Each behaviour has its own AiPlayerbot.CoaArena* / CoaPvp* / CoaDuel*
// switch, read on every use so that ".reload config" takes effect at once.
//
// Strategy "coa pvp", held by every CoA bot in both engines. Its triggers only open the way (the right place, the
// switch on, the role); the actions then look at the fight and decline when there is nothing to do, so that
// outside an arena, a duel or a battleground nothing changes.

#include "CoaPvpTactics.h"

#include "AscensionSpecialization.h"
#include "Battleground.h"
#include "CellImpl.h"
#include "Config.h"
#include "CoaSpecialization.h"
#include "GameObject.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Map.h"
#include "MovementActions.h"
#include "Multiplier.h"
#include "NamedObjectContext.h"
#include "PathGenerator.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Strategy.h"
#include "Timer.h"
#include "Trigger.h"

#include <algorithm>
#include <cmath>
#include <list>
#include <vector>

namespace
{
    bool On(char const* key) { return sConfigMgr->GetOption<bool>(key, true); }

    bool IsCoaBot(Player* bot) { return bot && IsAscensionCustomClassId(bot->getClass()); }

    bool Healer(Player* player) { return PlayerbotAI::IsHeal(player, true); }
    bool Tank(Player* player) { return PlayerbotAI::IsTank(player, true); }
    // Healers and ranged damage dealers: the ones that should not stand in melee.
    bool Squishy(Player* player) { return Healer(player) || (PlayerbotAI::IsRanged(player, true) && !Tank(player)); }

    Battleground* ArenaOf(Player* bot)
    {
        if (!bot->InArena())
            return nullptr;
        Battleground* bg = bot->GetBattleground();
        return bg && bg->isArena() ? bg : nullptr;
    }

    bool ArenaWaiting(Player* bot)
    {
        Battleground* bg = ArenaOf(bot);
        return bg && bg->GetStatus() == STATUS_WAIT_JOIN;
    }

    // Each arena, as the bots need it: the floor height at its centre (the melee spot of ArenaTactics::moveToCenter,
    // the water and floor of the Dalaran and Ring of Valor scripts), how far from the centre a bot may go in a fight,
    // and how deep into its starting room it may spread before the gates open (none on the Ring of Valor lifts).
    struct ArenaGround
    {
        uint32 map;
        float floorZ;
        float reach;
        float roomDepth;
    };

    constexpr ArenaGround Grounds[] = {
        { 559, 13.5f, 38.0f, 7.0f },  // Nagrand
        { 562, 0.9f, 42.0f, 7.0f },   // Blade's Edge, the floor under the bridge
        { 572, 34.0f, 38.0f, 7.0f },  // Ruins of Lordaeron
        { 617, 7.1f, 34.0f, 3.0f },   // Dalaran Sewers, narrow pipes
        { 618, 28.3f, 34.0f, 0.0f },  // Ring of Valor
    };

    ArenaGround const* GroundOf(uint32 map)
    {
        for (ArenaGround const& ground : Grounds)
            if (ground.map == map)
                return &ground;
        return nullptr;
    }

    // Halfway between the two starting points, which every arena puts on either side of its centre.
    bool ArenaCentre(Battleground* bg, Position& centre)
    {
        ArenaGround const* ground = GroundOf(bg->GetMapId());
        Position const* a = bg->GetTeamStartPosition(TEAM_ALLIANCE);
        Position const* b = bg->GetTeamStartPosition(TEAM_HORDE);
        if (!ground || !a || !b)
            return false;
        centre.Relocate((a->GetPositionX() + b->GetPositionX()) / 2.0f, (a->GetPositionY() + b->GetPositionY()) / 2.0f,
                        ground->floorZ);
        return true;
    }

    // The floor under x, y close to `z`: no jump onto a bridge or down a ledge.
    bool GroundAt(Player* bot, float x, float y, float& z)
    {
        float const found = bot->GetMap()->GetHeight(bot->GetPhaseMask(), x, y, z + 3.0f, true, 8.0f);
        if (found <= INVALID_HEIGHT || std::fabs(found - z) > 4.0f)
            return false;
        z = found;
        return true;
    }

    // Whether the bot walks there by a path not much longer than `maxLength`.
    bool Walkable(Player* bot, float x, float y, float z, float maxLength)
    {
        PathGenerator path(bot);
        path.CalculatePath(x, y, z, false);
        PathType const type = path.GetPathType();
        if (type & PATHFIND_NOT_USING_PATH)
            return true;
        if (!(type & PATHFIND_NORMAL) || (type & (PATHFIND_NOPATH | PATHFIND_INCOMPLETE | PATHFIND_SHORTCUT | PATHFIND_SHORT)))
            return false;
        Movement::PointsArray const& points = path.GetPath();
        if (points.empty())
            return false;
        float length = 0.0f;
        for (size_t i = 1; i < points.size(); ++i)
            length += (points[i] - points[i - 1]).length();
        float const dx = points.back().x - x;
        float const dy = points.back().y - y;
        return length <= maxLength && dx * dx + dy * dy < 4.0f;
    }

    // Base of the moving actions: a combat move, and the time it should take, during which the action stands back.
    class CoaPvpMoveAction : public MovementAction
    {
    public:
        CoaPvpMoveAction(PlayerbotAI* botAI, std::string const name) : MovementAction(botAI, name) {}

    protected:
        bool Go(float x, float y, float z, MovementPriority priority = MovementPriority::MOVEMENT_COMBAT)
        {
            float const distance = bot->GetExactDist2d(x, y);
            if (!MoveTo(bot->GetMapId(), x, y, z, false, false, false, false, priority))
                return false;
            float const speed = std::max(bot->GetSpeed(MOVE_RUN), 2.0f);
            busyUntil = getMSTime() + uint32(distance / speed * IN_MILLISECONDS * 0.8f);
            return true;
        }

        bool Busy() const { return getMSTime() < busyUntil && bot->isMoving(); }

        uint32 busyUntil = 0;
    };

    /*
     * Before the gates open. The bots stood on their start point for the whole preparation. Now each one takes a
     * place of its role between the start point and the gate (healers at the back, then ranged, melee at the front),
     * side by side with the others of its row, and faces the gate. The room's depth is the distance to the nearest
     * door of the arena, capped per arena; none on the Ring of Valor lifts. Buffs and pets come from "coa buff",
     * which runs once the bot stands still, and CoaArenaReady still marks the bot ready.
     */
    class CoaArenaPreparationAction : public CoaPvpMoveAction
    {
    public:
        CoaArenaPreparationAction(PlayerbotAI* botAI) : CoaPvpMoveAction(botAI, "coa arena preparation") {}

        bool isUseful() override
        {
            if (!On("AiPlayerbot.CoaArenaPreparation") || !IsCoaBot(bot) || !ArenaWaiting(bot) || !bot->IsAlive() ||
                bot->IsBeingTeleported() || bot->IsNonMeleeSpellCast(false) || Busy())
                return false;
            Plan();
            return planned && (bot->GetExactDist2d(spot.GetPositionX(), spot.GetPositionY()) > 1.2f || !faced);
        }

        bool Execute(Event /*event*/) override
        {
            if (!planned)
                return false;
            if (bot->GetExactDist2d(spot.GetPositionX(), spot.GetPositionY()) > 1.2f)
            {
                faced = false;
                return Go(spot.GetPositionX(), spot.GetPositionY(), spot.GetPositionZ(), MovementPriority::MOVEMENT_NORMAL);
            }
            bot->SetFacingTo(facing);
            faced = true;
            return true;
        }

    private:
        // Planned again every few seconds: the rows change as the team arrives.
        void Plan()
        {
            uint32 const now = getMSTime();
            Battleground* bg = ArenaOf(bot);
            if (!bg || (planned && bg->GetInstanceID() == plannedFor && now - plannedAt < 4000))
                return;
            plannedAt = now;
            plannedFor = bg->GetInstanceID();
            planned = false;

            ArenaGround const* ground = GroundOf(bg->GetMapId());
            Position const* start = bg->GetTeamStartPosition(bot->GetBgTeamId());
            Position centre;
            if (!ground || !start || !ArenaCentre(bg, centre))
                return;

            // Towards the gate when the room has one near, else towards the centre.
            float angle = start->GetAbsoluteAngle(&centre);
            float depth = 0.0f;
            if (GameObject* door = NearestDoor(*start))
            {
                angle = start->GetAbsoluteAngle(door);
                depth = std::min(start->GetExactDist2d(door) - 2.5f, ground->roomDepth);
            }
            else
                depth = std::min(3.0f, ground->roomDepth);
            depth = std::max(depth, 0.0f);
            facing = angle;

            // The row of its role, and its place in the row among the team members of the same row.
            int const row = Row(bot);
            float const forward = depth * (row == 0 ? 0.2f : row == 1 ? 0.55f : 0.9f);
            std::vector<ObjectGuid> mates;
            for (auto const& [guid, player] : bg->GetPlayers())
                if (player && player->GetBgTeamId() == bot->GetBgTeamId() && Row(player) == row)
                    mates.push_back(guid);
            std::sort(mates.begin(), mates.end());
            int const index = int(std::find(mates.begin(), mates.end(), bot->GetGUID()) - mates.begin());
            float side = float((index + 1) / 2) * 2.5f * (index % 2 ? 1.0f : -1.0f);
            side = std::clamp(side, -5.0f, 5.0f);
            if (depth < 1.0f)
                side = 0.0f;

            for (float lateral : { side, 0.0f })
            {
                float const x = start->GetPositionX() + forward * std::cos(angle) - lateral * std::sin(angle);
                float const y = start->GetPositionY() + forward * std::sin(angle) + lateral * std::cos(angle);
                float z = start->GetPositionZ();
                if (!GroundAt(bot, x, y, z))
                    continue;
                // In the room: seen from the start point, and walked to straight.
                if (!bot->GetMap()->isInLineOfSight(start->GetPositionX(), start->GetPositionY(), start->GetPositionZ() + 1.6f,
                                                    x, y, z + 1.6f, bot->GetPhaseMask(), LINEOFSIGHT_ALL_CHECKS,
                                                    VMAP::ModelIgnoreFlags::Nothing))
                    continue;
                if (bot->GetExactDist2d(x, y) > 1.2f && !Walkable(bot, x, y, z, bot->GetExactDist2d(x, y) * 1.5f + 3.0f))
                    continue;
                spot.Relocate(x, y, z);
                planned = true;
                return;
            }
            // Nowhere better: the start point itself, facing the gate.
            spot.Relocate(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
            planned = true;
        }

        static int Row(Player* player)
        {
            if (Healer(player))
                return 0;
            return Squishy(player) ? 1 : 2;
        }

        GameObject* NearestDoor(Position const& start)
        {
            std::list<GameObject*> found;
            Acore::GameObjectInRangeCheck check(start.GetPositionX(), start.GetPositionY(), start.GetPositionZ(), 30.0f);
            Acore::GameObjectListSearcher<Acore::GameObjectInRangeCheck> searcher(bot, found, check);
            Cell::VisitObjects(start.GetPositionX(), start.GetPositionY(), bot->GetMap(), searcher, 30.0f);
            GameObject* nearest = nullptr;
            for (GameObject* go : found)
                if (go->GetGoType() == GAMEOBJECT_TYPE_DOOR &&
                    (!nearest || start.GetExactDist2d(go) < start.GetExactDist2d(nearest)))
                    nearest = go;
            return nearest;
        }

        Position spot;
        float facing = 0.0f;
        uint32 plannedAt = 0;
        uint32 plannedFor = 0;
        bool planned = false;
        bool faced = false;
    };

    // The triggers only check the place, the switch and the role, each a cheap test; the action decides the rest.
    using Gate = bool (*)(Player* bot);

    class CoaPvpTrigger : public Trigger
    {
    public:
        CoaPvpTrigger(PlayerbotAI* botAI, std::string const name, int32 interval, Gate gate)
            : Trigger(botAI, name, interval), gate(gate)
        {
        }

        bool IsActive() override { return gate(bot); }

    private:
        Gate gate;
    };

    bool PreparationGate(Player* bot)
    {
        return ArenaWaiting(bot) && IsCoaBot(bot) && On("AiPlayerbot.CoaArenaPreparation");
    }

    class CoaPvpStrategy : public Strategy
    {
    public:
        CoaPvpStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

        std::string const getName() override { return "coa pvp"; }

        void InitTriggers(std::vector<TriggerNode*>& triggers) override
        {
            // Just above "coa buff": into place first, buffs once standing.
            triggers.push_back(new TriggerNode("coa arena preparation", { NextAction("coa arena preparation", ACTION_NORMAL + 6) }));
        }
    };

    class CoaPvpStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        CoaPvpStrategyContext() : NamedObjectContext<Strategy>(false, false) { creators["coa pvp"] = &pvp; }

    private:
        static Strategy* pvp(PlayerbotAI* botAI) { return new CoaPvpStrategy(botAI); }
    };

    class CoaPvpActionContext : public NamedObjectContext<Action>
    {
    public:
        CoaPvpActionContext() : NamedObjectContext<Action>(false, false)
        {
            creators["coa arena preparation"] = &preparation;
        }

    private:
        static Action* preparation(PlayerbotAI* botAI) { return new CoaArenaPreparationAction(botAI); }
    };

    class CoaPvpTriggerContext : public NamedObjectContext<Trigger>
    {
    public:
        CoaPvpTriggerContext() : NamedObjectContext<Trigger>(false, false)
        {
            creators["coa arena preparation"] = &preparation;
        }

    private:
        static Trigger* preparation(PlayerbotAI* botAI)
        {
            return new CoaPvpTrigger(botAI, "coa arena preparation", 1000, &PreparationGate);
        }
    };
}

NamedObjectContext<Strategy>* NewCoaPvpStrategyContext() { return new CoaPvpStrategyContext(); }
NamedObjectContext<Action>* NewCoaPvpActionContext() { return new CoaPvpActionContext(); }
NamedObjectContext<Trigger>* NewCoaPvpTriggerContext() { return new CoaPvpTriggerContext(); }

uint32 CoaPvp::ArenaSpot(Player* bot)
{
    if (!IsCoaBot(bot) || !On("AiPlayerbot.CoaArenaRoleSpot"))
        return 0;
    if (Healer(bot))
        return 3;
    if (Squishy(bot))
        return 9;
    return 6;
}
