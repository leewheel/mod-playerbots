/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CheckMountStateAction.h"
#include "AreaDefines.h"
#include "BattleGroundTactics.h"
#include "BattlegroundEY.h"
#include "BattlegroundWS.h"
#include "DBCStores.h"
#include "Event.h"
#include "ItemCountValue.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "SpellAuraEffects.h"

static constexpr uint32 SPELL_COLD_WEATHER_FLYING = 54197;
static constexpr float PARACHUTE_LAND_THRESHOLD = 15.0f;

// By leewheel 2026-07-18
// 硬编码坐骑法术数组：机器人上坐骑时如果没有坐骑法术，从数组中随机取一个补学
// 地面坐骑：EffectAura_2=32(SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED)
// 飞行坐骑：EffectAura_2=207(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED)
static const uint32 groundMountSpells[] = {
    1700012, 1700013, 1700014, 1700015, 1700016, 1700024, 1700025, 1700026,
    1700027, 1700028, 1700029, 1700033, 1700038, 1700039, 1700040, 1700041,
    1700042, 1700043, 1700044, 1700045, 1700046, 1700047, 1700049, 1700054,
    1700056, 1700057, 1700058, 1700060, 1700069, 1700070, 1700071, 1700073,
    1700074, 1700075, 1700076, 1700077, 1700078, 1700079, 1700080, 1700081,
    1700082, 1700083, 1700084, 1700085, 1700086, 1700087, 1700088, 1700089,
    1700095, 1700096, 1700097, 1700098, 1700099, 1700100, 1700101, 1700103,
    1700104, 1700105, 1700106, 1700107, 1700109, 1700110, 1700111, 1700113,
    1700114, 1700115, 1700116, 1700128, 1700129, 1700130, 1700135, 1700136,
    1700137, 1700138, 1700140, 1700141, 1700142, 1700147, 1700148, 1700149,
    1700150, 1700151, 1700152, 1700153, 1700154, 1700155, 1700156, 1700163,
    1700165, 1700166, 1700167, 1700168, 1700169, 1700170, 1700171, 1700172,
    1700173, 1700174, 1700175, 1700179, 1700184, 1700197, 1700198, 1700199,
    1700200, 1700201, 1700202, 1700203, 1700204, 1700205, 1700206, 1700207,
    1700208, 1700217, 1700218, 1700219, 1700220, 1700226, 1700227, 1700228,
    1700229, 1700230, 1700231, 1700232, 1700235, 1700236, 1700237, 1700238,
    1700239, 1700240, 1700241, 1700242, 1700243, 1700244, 1700245, 1700246,
    1700247, 1700248, 1700249, 1700250, 1700251, 1700252, 1700253, 1700254,
    1700255, 1700256, 1700257, 1700258, 1700260, 1700261, 1700262, 1700263,
    1700264, 1700265, 1700266, 1700267, 1700268, 1700269, 1700270, 1700271,
    1700272, 1700273, 1700274, 1700275, 1700276, 1700280, 1700281, 1700282,
    1700287, 1700288, 1700289, 1700290, 1700291, 1700292, 1700293, 1700294,
    1700307, 1700308, 1700318, 1700319, 1700320, 1700321, 1700323, 1700326,
    1700328, 1700329, 1700331, 1700332, 1700333, 1700337, 1700338, 1700339,
    1700340, 1700341, 1700342, 1700344, 1700345, 1700348, 1700349, 1700359,
    1700360, 1700361, 1700362, 1700363, 1700364, 1700365, 1700366, 1700367,
    1700368, 1700369, 1700370, 1700371, 1700372, 1700373, 1700374, 1700375,
    1700376, 1700377, 1700378, 1700379, 1700380, 1700381, 1700387, 1700388,
    1700389, 1700390, 1700391, 1700392, 1700393, 1700394, 1700395, 1700396,
    1700397, 1700398, 1700399, 1700400, 1700401
};
static const uint32 flightMountSpells[] = {
    1700001, 1700002, 1700003, 1700004, 1700005, 1700006, 1700007, 1700008,
    1700009, 1700010, 1700011, 1700017, 1700018, 1700019, 1700020, 1700021,
    1700022, 1700023, 1700030, 1700031, 1700032, 1700034, 1700035, 1700036,
    1700037, 1700048, 1700050, 1700051, 1700052, 1700053, 1700055, 1700059,
    1700061, 1700062, 1700063, 1700064, 1700065, 1700066, 1700067, 1700068,
    1700072, 1700090, 1700091, 1700092, 1700093, 1700094, 1700102, 1700108,
    1700112, 1700117, 1700118, 1700119, 1700120, 1700121, 1700122, 1700123,
    1700124, 1700125, 1700126, 1700127, 1700131, 1700132, 1700133, 1700134,
    1700139, 1700143, 1700144, 1700145, 1700146, 1700157, 1700158, 1700159,
    1700160, 1700161, 1700162, 1700164, 1700176, 1700177, 1700178, 1700180,
    1700181, 1700182, 1700183, 1700185, 1700186, 1700187, 1700188, 1700189,
    1700190, 1700192, 1700193, 1700194, 1700195, 1700196, 1700209, 1700210,
    1700211, 1700212, 1700213, 1700214, 1700215, 1700216, 1700221, 1700222,
    1700223, 1700224, 1700225, 1700233, 1700234, 1700259, 1700277, 1700278,
    1700279, 1700283, 1700284, 1700285, 1700286, 1700295, 1700296, 1700297,
    1700298, 1700299, 1700300, 1700301, 1700302, 1700303, 1700304, 1700305,
    1700306, 1700309, 1700310, 1700311, 1700312, 1700313, 1700314, 1700315,
    1700316, 1700317, 1700322, 1700324, 1700325, 1700327, 1700330, 1700334,
    1700335, 1700336, 1700343, 1700346, 1700347, 1700350, 1700351, 1700352,
    1700353, 1700354, 1700355, 1700356, 1700357, 1700358, 1700382, 1700383,
    1700384, 1700385, 1700386, 1700402, 1700403, 1700404, 1700405, 1700406,
    1700407, 1700408, 1700409, 1700410, 1700411, 1700412
};
// End By leewheel

// Define the static map / init bool for caching bot preferred mount data globally
std::unordered_map<uint32, PreferredMountCache> CheckMountStateAction::mountCache;
bool CheckMountStateAction::preferredMountTableChecked = false;

MountData CollectMountData(Player const* bot)
{
    MountData data;
    for (auto& entry : bot->GetSpellMap())
    {
        uint32 spellId = entry.first;
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo || spellInfo->Effects[0].ApplyAuraName != SPELL_AURA_MOUNTED)
            continue;

        if (entry.second->State == PLAYERSPELL_REMOVED || !entry.second->Active || spellInfo->IsPassive())
            continue;

        int32 effect1 = spellInfo->Effects[1].BasePoints;
        int32 effect2 = spellInfo->Effects[2].BasePoints;

        int32 speed = std::max(effect1, effect2);

        // Update max speed if appropriate.
        if (speed > data.maxSpeed)
            data.maxSpeed = speed;  // In BG, clamp max speed to 99 later; here we just store the maximum found.

        // Determine index: flight if either effect has flight aura or specific mount ID.
        uint32 index = (spellInfo->Effects[1].ApplyAuraName == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED ||
                        spellInfo->Effects[2].ApplyAuraName == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED ||
                        // Winged Steed of the Ebon Blade
                        // This mount is meant to autoscale from a 150% flyer
                        // up to a 280% as you train your flying skill up.
                        // This incorrectly gets categorised as a ground mount, force this to flyer only.
                        // TODO: Add other scaling mounts here if they have the same issue, or adjust above
                        // checks so that they are all correctly detected.
                        spellInfo->Id == 54729) ? 1 : 0;
        data.allSpells[index][speed].push_back(spellId);
    }
    return data;
}

bool CheckMountStateAction::Execute(Event /*event*/)
{
    // Forced flight dismount:
    // Bots get stale flight movement flags after a forced dismount (e.g: Dalaran) because the post landing dismount cleanup
    // needs MSG_MOVE_FALL_LAND (a client opcode) and client movement packets. The stale flags cause the bot to be stuck with
    // the parachute, or even keep the bot hovering indefinitely and block MMAP routing.
    // Note: Without MSG_MOVE_FALL_LAND, HandleFall doesn't trigger, meaning bots don't get fall damage in forced dismounts anyway,
    // so the parachute usage here is more of an immersion feature.
    if (bot->HasFeatherFallAura())
    {
        float floorZ = bot->GetMapHeight(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
        if (floorZ != INVALID_HEIGHT && floorZ != VMAP_INVALID_HEIGHT_VALUE &&
            bot->GetPositionZ() - floorZ <= PARACHUTE_LAND_THRESHOLD)
            bot->RemoveAurasByType(SPELL_AURA_FEATHER_FALL);
    }
    ClearStaleFlightFlags();

    // Determine if there are no attackers
    bool noAttackers = !AI_VALUE2(bool, "combat", "self target") || !AI_VALUE(uint8, "attacker count");
    bool enemy = AI_VALUE(Unit*, "enemy player target");
    bool dps = AI_VALUE(Unit*, "dps target");
    bool shouldDismount = false;
    bool shouldMount = false;

    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    if (currentTarget)
    {
        float dismountDistance = CalculateDismountDistance();
        float mountDistance = CalculateMountDistance();
        float combatReach = bot->GetCombatReach() + currentTarget->GetCombatReach();
        float distanceToTarget = bot->GetExactDist(currentTarget);

        shouldDismount = (distanceToTarget <= dismountDistance + combatReach);
        shouldMount = (distanceToTarget > mountDistance + combatReach);
    }
    else
    {
        shouldMount = true;
    }

    // If should dismount, or master (if any) is no longer in travel form, yet bot still is, remove the shapeshifts
    if (shouldDismount ||
        (masterInShapeshiftForm != FORM_TRAVEL && botInShapeshiftForm == FORM_TRAVEL) ||
        (masterInShapeshiftForm != FORM_FLIGHT && botInShapeshiftForm == FORM_FLIGHT && master && !master->IsMounted()) ||
        (masterInShapeshiftForm != FORM_FLIGHT_EPIC && botInShapeshiftForm == FORM_FLIGHT_EPIC && master && !master->IsMounted()))
        botAI->RemoveShapeshift();

    if (shouldDismount && bot->IsMounted())
    {
        Dismount();
        return true;
    }

    bool inBattleground = bot->InBattleground();
    bool const noRealMaster = (!master || master == bot);

    // If there is a master and bot not in BG, follow master's mount state regardless of group leader
    if (!noRealMaster && !inBattleground)
    {
        if (ShouldFollowMasterMountState(master, noAttackers, shouldMount))
            return Mount();

        else if (ShouldDismountForMaster(master) && bot->IsMounted())
        {
            Dismount();
            return true;
        }

        return false;
    }

    // No real master (random bot or self-bot) OR bot in BG
    if ((noRealMaster || inBattleground) && !bot->IsMounted() &&
        noAttackers && shouldMount && !bot->IsInCombat())
        return Mount();

    if (!bot->IsFlying() && shouldDismount && bot->IsMounted() &&
        (enemy || dps || (!noAttackers && bot->IsInCombat())))
    {
        Dismount();
        return true;
    }

    return false;
}

bool CheckMountStateAction::isUseful()
{
    // Not useful when:
    if (botAI->IsInVehicle() || bot->isDead() || bot->HasUnitState(UNIT_STATE_IN_FLIGHT) ||
        !bot->IsOutdoors() || bot->InArena())
        return false;

    // Selfbots don't auto-mount.
    // If they are already mounted (manually), allow dismount logic to proceed.
    if (IsSelfBot(bot) && !bot->IsMounted())
        return false;

    master = GetMaster();

    // Get shapeshift states, only applicable when there's a master
    if (master)
    {
        botInShapeshiftForm = bot->GetShapeshiftForm();
        masterInShapeshiftForm = master->GetShapeshiftForm();
    }

    // Not useful when in combat and not currently mounted / travel formed
    if ((bot->IsInCombat() || botAI->GetState() == BOT_STATE_COMBAT) &&
        !bot->IsMounted() && botInShapeshiftForm != FORM_TRAVEL && botInShapeshiftForm != FORM_FLIGHT && botInShapeshiftForm != FORM_FLIGHT_EPIC)
        return false;

    // In addition to checking IsOutdoors, also check whether bot is clipping below floor slightly because that will
    // cause bot to falsly indicate they are outdoors. This fixes bug where bot tries to mount indoors (which seems
    // to mostly be an issue in tunnels of WSG and AV)
    float posZ = bot->GetPositionZ();
    float groundLevel = bot->GetMapWaterOrGroundLevel(bot->GetPositionX(), bot->GetPositionY(), posZ);
    if (!bot->IsMounted() && !bot->HasWaterWalkAura() && posZ < groundLevel)
        return false;

    // Not useful when bot does not have mount strat and is not currently mounted
    if (!GET_PLAYERBOT_AI(bot)->HasStrategy("mount", BOT_STATE_NON_COMBAT) && !bot->IsMounted())
        return false;

    // Not useful when level lower than minimum required
    if (bot->GetLevel() < sPlayerbotAIConfig.useGroundMountAtMinLevel)
        return false;

    // Allow mounting while transformed only if the form allows it
    if (bot->HasAuraType(SPELL_AURA_TRANSFORM) && bot->IsInDisallowedMountForm())
        return false;

    // BG Logic
    if (bot->InBattleground())
    {
        // Do not use when carrying BG Flags
        if (bot->HasAura(BG_WS_SPELL_WARSONG_FLAG) || bot->HasAura(BG_WS_SPELL_SILVERWING_FLAG) || bot->HasAura(BG_EY_NETHERSTORM_FLAG_SPELL))
            return false;

        // Only mount if BG starts in less than 30 sec
        if (Battleground* bg = bot->GetBattleground())
            if (bg->GetStatus() == STATUS_WAIT_JOIN && bg->GetStartDelayTime() > BG_START_DELAY_30S)
                return false;
    }

    return true;
}

bool CheckMountStateAction::Mount()
{
    // Remove current Shapeshift if need be
    if (botInShapeshiftForm != FORM_TRAVEL &&
        botInShapeshiftForm != FORM_FLIGHT &&
        botInShapeshiftForm != FORM_FLIGHT_EPIC)
    {
        botAI->RemoveShapeshift();
        botAI->RemoveAura("tree of life");
    }

    if (TryPreferredMount(master))
        return true;

    // Get bot mount data
    MountData mountData = CollectMountData(bot);
    int32 masterMountType = GetMountType(master);
    int32 masterSpeed = CalculateMasterMountSpeed(master);

    // Try shapeshift
    if (TryForms(master, masterMountType, masterSpeed))
        return true;

    // By leewheel 2026-07-18
    // 补学：如果机器人没有对应类型的坐骑法术，直接从硬编码数组中随机取一个学习
    // 学习失败就重新再学，直到学会
    {
        auto checkIt = mountData.allSpells.find(masterMountType);
        bool hasMount = (checkIt != mountData.allSpells.end() && !checkIt->second.empty());
        if (!hasMount)
        {
            const uint32* mountArray = (masterMountType == 0) ? groundMountSpells : flightMountSpells;
            uint32 mountArraySize = (masterMountType == 0) ?
                (sizeof(groundMountSpells) / sizeof(groundMountSpells[0])) :
                (sizeof(flightMountSpells) / sizeof(flightMountSpells[0]));
            for (uint32 attempt = 0; attempt < 10 && mountArraySize > 0; ++attempt)
            {
                uint32 index = urand(0, mountArraySize - 1);
                uint32 spell = mountArray[index];
                if (bot->HasSpell(spell))
                    break;
                bot->learnSpell(spell);
                if (bot->HasSpell(spell))
                {
                    LOG_INFO("playerbots", "机器人 {} 上坐骑时补学坐骑法术 {}。", bot->GetName(), spell);
                    break;
                }
            }
            // 补学后重新收集坐骑数据
            mountData = CollectMountData(bot);
        }
    }
    // End By leewheel

    // Try random mount
    auto spellsIt = mountData.allSpells.find(masterMountType);
    if (spellsIt != mountData.allSpells.end())
    {
        auto& spells = spellsIt->second;
        if (TryRandomMountFiltered(spells, masterSpeed))
            return true;
    }

    std::vector<Item*> items = AI_VALUE2(std::vector<Item*>, "inventory items", "mount");
    // By leewheel 2026-09-04 防悬空崩溃: 缓存物品列表先过滤失效指针再取用
    // End By leewheel
    items = InventoryItemValueBase::FilterLive(bot, items);
    if (!items.empty())
        return UseItemAuto(*items.begin());

    return false;
}

void CheckMountStateAction::Dismount()
{
    if (bot->isMoving())
        bot->StopMoving();

    WorldPacket emptyPacket;
    bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);

    ClearStaleFlightFlags();
}

void CheckMountStateAction::ClearStaleFlightFlags()
{
    if (bot->HasIncreaseMountedFlightSpeedAura() || bot->HasFlyAura())
        return;

    if (bot->HasUnitMovementFlag(MOVEMENTFLAG_FLYING | MOVEMENTFLAG_DISABLE_GRAVITY))
    {
        bot->RemoveUnitMovementFlag(MOVEMENTFLAG_FLYING | MOVEMENTFLAG_DISABLE_GRAVITY | MOVEMENTFLAG_CAN_FLY);
        if (!bot->IsRooted())
            bot->SendMovementFlagUpdate();
    }
}

void CheckMountStateAction::CompleteDismount(Player* bot)
{
    if (!bot || !bot->IsInWorld())
        return;

    float const x = bot->GetPositionX();
    float const y = bot->GetPositionY();
    float const startZ = bot->GetPositionZ();

    float groundZ = startZ;
    bot->UpdateAllowedPositionZ(x, y, groundZ);

    bot->GetMotionMaster()->MoveFall();
    MovementInfo fallInfo = bot->m_movementInfo;
    // Need to set the start of the fall, otherwise the fall may start from too high of a Z and kill the bot.
    bot->SetFallInformation(0, startZ);
    fallInfo.pos.Relocate(x, y, groundZ);
    bot->HandleFall(fallInfo);
    bot->RemoveUnitMovementFlag(MOVEMENTFLAG_FALLING | MOVEMENTFLAG_FALLING_FAR);
}

bool CheckMountStateAction::TryForms(Player* master, int32 masterMountType, int32 masterSpeed) const
{
    if (!master)
        return false;

    // If both master and bot are in matching forms or master is mounted with corresponding speed, nothing to do
    else if
        ((masterInShapeshiftForm == FORM_TRAVEL && botInShapeshiftForm == FORM_TRAVEL) ||
        ((masterInShapeshiftForm == FORM_FLIGHT || (masterMountType == 1 && masterSpeed == 149)) && botInShapeshiftForm == FORM_FLIGHT) ||
        ((masterInShapeshiftForm == FORM_FLIGHT_EPIC || (masterMountType == 1 && masterSpeed >= 279)) && botInShapeshiftForm == FORM_FLIGHT_EPIC))
        return true;

    // Check if master is in Travel Form and bot can do the same
    if (botAI->CanCastSpell(SPELL_TRAVEL_FORM, bot, true) &&
        masterInShapeshiftForm == FORM_TRAVEL && botInShapeshiftForm != FORM_TRAVEL)
    {
        botAI->CastSpell(SPELL_TRAVEL_FORM, bot);
        return true;
    }

    // Check if master is in Flight Form or has a flying mount and bot can flight form
    if (botAI->CanCastSpell(SPELL_FLIGHT_FORM, bot, true) &&
        ((masterInShapeshiftForm == FORM_FLIGHT && botInShapeshiftForm != FORM_FLIGHT) ||
        (masterMountType == 1 && masterSpeed == 149)))
    {
        botAI->CastSpell(SPELL_FLIGHT_FORM, bot);

        // Compensate speedbuff
        bot->SetSpeed(MOVE_RUN, 2.5, true);
        return true;
    }

    // Check if master is in Swift Flight Form or has an epic flying mount and bot can swift flight form
    if (botAI->CanCastSpell(SPELL_SWIFT_FLIGHT_FORM, bot, true) &&
        ((masterInShapeshiftForm == FORM_FLIGHT_EPIC && botInShapeshiftForm != FORM_FLIGHT_EPIC) ||
        (masterMountType == 1 && masterSpeed >= 279)))
    {
        botAI->CastSpell(SPELL_SWIFT_FLIGHT_FORM, bot);

        // Compensate speedbuff
        bot->SetSpeed(MOVE_RUN, 3.8, true);
        return true;
    }

    return false;
}

bool CheckMountStateAction::TryPreferredMount(Player* master) const
{
    uint32 botGUID = bot->GetGUID().GetRawValue();

    // Build cache (only once)
    if (!preferredMountTableChecked)
    {
        // Verify preferred mounts table existance in the database
        QueryResult checkTable = PlayerbotsDatabase.Query(
            "SELECT EXISTS(SELECT * FROM information_schema.tables WHERE table_schema = 'acore_playerbots' AND table_name = 'playerbots_preferred_mounts')");

        if (checkTable && checkTable->Fetch()[0].Get<uint32>() == 1)
        {
            preferredMountTableChecked = true;

            // Cache all mounts of both types globally, for all entries
            QueryResult result = PlayerbotsDatabase.Query("SELECT guid, spellid, type FROM playerbots_preferred_mounts");

            if (result)
            {
                uint32 totalResults = 0;
                while (auto row = result->Fetch())
                {
                    uint32 guid = row[0].Get<uint32>();
                    uint32 spellId = row[1].Get<uint32>();
                    uint32 mountType = row[2].Get<uint32>();

                    if (mountType == 0)
                        mountCache[guid].groundMounts.push_back(spellId);

                    else if (mountType == 1)
                        mountCache[guid].flightMounts.push_back(spellId);

                    totalResults++;

                    result->NextRow();
                }
                LOG_INFO("playerbots", "Preferred mounts initialized | Total records: {}", totalResults);
            }
        }
        else // If the SQL table is missing, log an error and return false
        {
            preferredMountTableChecked = true;

            LOG_DEBUG("playerbots", "Preferred mounts SQL table playerbots_preferred_mounts does not exist!");

            return false;
        }
    }

    // Pick a random preferred mount from the selection, if available
    uint32 chosenMountId = 0;

    if (GetMountType(master) == 0 && !mountCache[botGUID].groundMounts.empty())
    {
        uint32 index = urand(0, mountCache[botGUID].groundMounts.size() - 1);
        chosenMountId = mountCache[botGUID].groundMounts[index];
    }

    else if (GetMountType(master) == 1 && !mountCache[botGUID].flightMounts.empty())
    {
        uint32 index = urand(0, mountCache[botGUID].flightMounts.size() - 1);
        chosenMountId = mountCache[botGUID].flightMounts[index];
    }

    // No suitable preferred mount found
    if (chosenMountId == 0)
        return false;

    // Check if spell exists
    if (!sSpellMgr->GetSpellInfo(chosenMountId))
    {
        LOG_ERROR("playerbots", "Preferred mount failed: Invalid spell {} | Bot Guid: {}", chosenMountId, botGUID);
        return false;
    }

    // Required here as otherwise bots won't mount in BG's due to them constant moving
    if (bot->isMoving())
        bot->StopMoving();

    // Check if spell can be cast - for now allow all, even if the bot does not have the actual mount
    //if (botAI->CanCastSpell(mountId, botAI->GetBot()))
    //{
    botAI->CastSpell(chosenMountId, botAI->GetBot());
    return true;
    //}

    LOG_DEBUG("playerbots", "Preferred mount failed! | Bot Guid: {}", botGUID);
    return false;
}

// By leewheel 2026-07-18
// 修复：机器人跟随主控移动时无法上坐骑的问题
// 原因：StopMoving() 只是发送停止移动包，服务端需要下一个tick才更新 isMoving() 状态，
//       所以紧接着的 CanCastSpell() 检查时 bot->isMoving() 仍为 true，导致 CanCastSpell 返回 false。
//       结果：地面坐骑时间歇性失败（有时1个上、有时3个上、有时全不上）。
//       飞行坐骑不受影响，因为飞行区域机器人通常已经停下。
// 修复：StopMoving 后清除移动状态标志，使 CanCastSpell 通过移动检查。
// 2026-09-04 合并brighton-chi/the-lab：参数声明随上游改为east-const风格，与头文件声明保持一致，逻辑不变。
// End By leewheel
bool CheckMountStateAction::TryRandomMountFiltered(std::map<int32, std::vector<uint32>> const& spells, int32 masterSpeed) const
{
    for (auto it = spells.rbegin(); it != spells.rend(); ++it)
    {
        int32 currentSpeed = it->first;

        if ((masterSpeed > 59 && currentSpeed < 99) || (masterSpeed > 149 && currentSpeed < 279))
            continue;

        // Pick a random mount from the candidate group.
        auto const& ids = it->second;
        if (!ids.empty())
        {
            // Required here as otherwise bots won't mount in BGs due to them constant moving
            if (bot->isMoving())
            {
                bot->StopMoving();
                // By leewheel 2026-07-18
                // StopMoving 后立即清除移动状态标志，否则 CanCastSpell 仍认为机器人在移动
                bot->ClearUnitState(UNIT_STATE_MOVING);
                // End By leewheel
            }

            uint32 index = urand(0, ids.size() - 1);

            if (botAI->CanCastSpell(ids[index], bot))
            {
                botAI->CastSpell(ids[index], bot);
                return true;
            }
            // By leewheel 2026-07-18
            // 如果 CanCastSpell 仍因移动检查失败，直接尝试施法（坐骑法术不会因移动而真正失败）
            // 坐骑法术施放时会自动让机器人停下来
            else if (bot->HasSpell(ids[index]) && !bot->HasSpellCooldown(ids[index]))
            {
                botAI->CastSpell(ids[index], bot);
                return true;
            }
            // End By leewheel
        }
    }
    return false;
}

float CheckMountStateAction::CalculateDismountDistance() const
{
    // Warrior bots should dismount far enough to charge (because it's important for generating some initial rage),
    // a real player would be riding toward enemy mashing the charge key but the bots won't cast charge while mounted.
    bool isMelee = PlayerbotAI::IsMelee(bot);
    float dismountDistance = isMelee ? sPlayerbotAIConfig.meleeDistance + 2.0f : sPlayerbotAIConfig.spellDistance + 2.0f;
    return bot->getClass() == CLASS_WARRIOR ? std::max(18.0f, dismountDistance) : dismountDistance;
}

float CheckMountStateAction::CalculateMountDistance() const
{
    // Mount distance should be >= 21 regardless of class, because when travelling a distance < 21 it takes longer
    // to cast mount-spell than the time saved from the speed increase. At a distance of 21 both approaches take 3
    // seconds:
    // 21 / 7  =  21 / 14 + 1.5  =  3   (7 = dismounted speed  14 = epic-mount speed  1.5 = mount-spell cast time)
    bool isMelee = PlayerbotAI::IsMelee(bot);
    float baseDistance = isMelee ? sPlayerbotAIConfig.meleeDistance + 10.0f : sPlayerbotAIConfig.spellDistance + 10.0f;
    return std::max(21.0f, baseDistance);
}

bool CheckMountStateAction::ShouldFollowMasterMountState(Player* master, bool noAttackers, bool shouldMount) const
{
    bool isMasterMounted = master->IsMounted() || (masterInShapeshiftForm == FORM_FLIGHT ||
                                                    masterInShapeshiftForm == FORM_FLIGHT_EPIC ||
                                                    masterInShapeshiftForm == FORM_TRAVEL);
    bool isBotMountedOrForm = bot->IsMounted() || botInShapeshiftForm == FORM_FLIGHT ||
                              botInShapeshiftForm == FORM_FLIGHT_EPIC || botInShapeshiftForm == FORM_TRAVEL;
    return isMasterMounted && !isBotMountedOrForm && noAttackers &&
        shouldMount && !bot->IsInCombat() && botAI->GetState() != BOT_STATE_COMBAT;
}

bool CheckMountStateAction::ShouldDismountForMaster(Player* master) const
{
    bool isMasterMounted = master->IsMounted() || (masterInShapeshiftForm == FORM_FLIGHT ||
                                                   masterInShapeshiftForm == FORM_FLIGHT_EPIC ||
                                                   masterInShapeshiftForm == FORM_TRAVEL);
    return !isMasterMounted && bot->IsMounted();
}

static bool BotCanUseFlyingMount(Player const* bot)
{
    if (bot->GetPureSkillValue(SKILL_RIDING) < 225)
        return false;

    AreaTableEntry const* area = sAreaTableStore.LookupEntry(bot->GetAreaId());
    if (!area || !area->IsFlyable())
        return false;
    if (area->flags & AREA_FLAG_NO_FLY_ZONE)
        return false;

    uint32 const vmap = GetVirtualMapForMapAndZone(bot->GetMapId(), bot->GetZoneId());
    if (vmap == MAP_NORTHREND && !bot->HasSpell(SPELL_COLD_WEATHER_FLYING))
        return false;

    return true;
}

int32 CheckMountStateAction::CalculateMasterMountSpeed(Player* master) const
{
    // Check riding skill and level requirements
    int32 ridingSkill = bot->GetPureSkillValue(SKILL_RIDING);
    int32 botLevel = bot->GetLevel();

    if (ridingSkill <= 75 && botLevel < static_cast<int32>(sPlayerbotAIConfig.useFastGroundMountAtMinLevel))
        return 59;

    // check if bot has master and if master is self
    bool const noRealMaster = (!master || master == bot);

    if (!noRealMaster && !bot->InBattleground())
    {
        auto auraEffects = master->GetAuraEffectsByType(SPELL_AURA_MOUNTED);
        if (!auraEffects.empty())
        {
            SpellInfo const* masterSpell = auraEffects.front()->GetSpellInfo();
            int32 effect1 = masterSpell->Effects[1].BasePoints;
            int32 effect2 = masterSpell->Effects[2].BasePoints;
            return std::max(effect1, effect2);
        }
        else if (masterInShapeshiftForm == FORM_FLIGHT_EPIC)
            return 279;
        else if (masterInShapeshiftForm == FORM_FLIGHT)
            return 149;
        return 59;  // walk pace
    }

    // No real master OR battleground: pick speed by skill tier.
    if (!bot->InBattleground() && BotCanUseFlyingMount(bot))
        return (ridingSkill >= 300) ? 279 : 149;

    int32 maxGround = (ridingSkill >= 150) ? 99 : 59;
    if (bot->InBattleground() && maxGround > 99)
        maxGround = 99;
    return maxGround;
}

uint32 CheckMountStateAction::GetMountType(Player* master) const
{
    bool const noRealMaster = (!master || master == bot);

    if (noRealMaster)
        return (!bot->InBattleground() && BotCanUseFlyingMount(bot)) ? 1 : 0;

    auto auraEffects = master->GetAuraEffectsByType(SPELL_AURA_MOUNTED);
    if (!auraEffects.empty())
    {
        SpellInfo const* masterSpell = auraEffects.front()->GetSpellInfo();
        return (masterSpell->Effects[1].ApplyAuraName == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED ||
                masterSpell->Effects[2].ApplyAuraName == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED) ? 1 : 0;
    }
    else if (masterInShapeshiftForm == FORM_FLIGHT || masterInShapeshiftForm == FORM_FLIGHT_EPIC)
        return 1;

    return 0;
}
