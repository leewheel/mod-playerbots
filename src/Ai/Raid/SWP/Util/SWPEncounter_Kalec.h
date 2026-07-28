/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPENCOUNTERKALEC_H
#define PLAYERBOTS_SWPENCOUNTERKALEC_H

#include "ObjectGuid.h"
#include "Position.h"
#include "SWPData.h"
#include <array>
#include <limits>
#include <unordered_map>

class Player;
class PlayerbotAI;
//By leewheel 2026-07-28 - 从brighton-chi来源移植：Kalec简化，添加Group前向声明
class Group;
//End By leewheel

namespace SwpHelpers
{

constexpr uint8 KALECGOS_TANK_COUNT = 3;
constexpr uint8 KALECGOS_GROUP_COUNT = 4;
constexpr uint8 KALECGOS_INVALID_GROUP = std::numeric_limits<uint8>::max();
constexpr float KALECGOS_SPECTRAL_REALM_Z = -74.5f;

struct KalecgosRealmState
{
    uint32 lastEnterMs = 0;
    uint32 lastExitMs = 0;
    bool inSpectralRealm = false;
};

struct KalecgosEncounterState
{
    uint32 encounterStartMs = 0;
    uint32 activeRiftOpenedMs = 0;
    uint8 activeRiftGroup = KALECGOS_INVALID_GROUP;
    ObjectGuid blastedPlayerGuid = ObjectGuid::Empty;
    ObjectGuid firstEntrantGuid = ObjectGuid::Empty;
    ObjectGuid currentTankGuid = ObjectGuid::Empty;
    ObjectGuid activeRiftOutgoingTankGuid = ObjectGuid::Empty;
    std::array<ObjectGuid, KALECGOS_TANK_COUNT> tankAssignmentGuids =
    {
        ObjectGuid::Empty, ObjectGuid::Empty, ObjectGuid::Empty
    };
    std::array<ObjectGuid, KALECGOS_TANK_COUNT> tankPortalRotationGuids =
    {
        ObjectGuid::Empty, ObjectGuid::Empty, ObjectGuid::Empty
    };
    std::unordered_map<ObjectGuid, uint8> playerToGroup;
};

extern Position const KALECGOS_TANK_POSITION;
extern Position const KALECGOS_INITIAL_RANGED_POSITION;

extern std::unordered_map<uint32, KalecgosEncounterState> kalecgosEncounterStates;
extern std::unordered_map<ObjectGuid, KalecgosRealmState> kalecgosRealmStates;

bool IsExhausted(Player* bot);
bool IsInSpectralRealm(Player* bot);
bool IsKalecgosDecurser(Player* bot);
//By leewheel 2026-07-28 - 从brighton-chi来源移植：Kalec简化，函数重命名
//                        EnsureKalecgosGroupAssignments → EnsureKalecgosRaidAssignments
//                        GetKalecgosCurrentTank → GetKalecgosDesignatedTank
//                        GetKalecgosReplacementTank → GetNextSurfaceTankInOrder（新参数签名）
//End By leewheel
void EnsureKalecgosRaidAssignments(Player* bot);
Player* GetKalecgosDesignatedTank(Player* player);
Player* GetNextSurfaceTankInOrder(
    Group* group, std::array<ObjectGuid, KALECGOS_TANK_COUNT> const& orderedGuids,
    ObjectGuid afterGuid, ObjectGuid excludedGuid = ObjectGuid::Empty,
    bool fallbackToFirst = false);
bool ShouldEnterKalecgosSpectralRift(Player* bot);
//By leewheel 2026-07-27 - announcerAI参数由调用方传入，避免函数内部重复查找botAI
void RecordKalecgosSpectralBlastTarget(Player* bot, PlayerbotAI* announcerAI);
void RecordKalecgosSpectralRealmEnter(Player* bot);
void UpdateKalecgosRealmState(Player* bot, bool inSpectralRealm, uint32 timestamp);

}

#endif
