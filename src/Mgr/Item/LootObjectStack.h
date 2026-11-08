/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_LOOTOBJECTSTACK_H
#define PLAYERBOTS_LOOTOBJECTSTACK_H

#include <array>
#include <chrono>
//By leewheel 2026-11-08 (bug_107): 放弃表用 unordered_map
#include <unordered_map>
//End By leewheel

#include "ObjectGuid.h"

class AiObjectContext;
class Player;
class WorldObject;

struct ItemTemplate;

class LootStrategy
{
public:
    LootStrategy() {}
    virtual ~LootStrategy() {};
    virtual bool CanLoot(ItemTemplate const* proto, AiObjectContext* context) = 0;
    virtual std::string const GetName() = 0;
};

constexpr uint8 MAX_LOOT_LOCK_REQUIREMENTS = 8;

struct LootLockRequirement
{
    uint32 SkillId = 0;
    uint32 ReqSkillValue = 0;
    uint32 ReqItem = 0;
    uint32 LockType = 0;
};

class LootObject
{
public:
    LootObject() : skillId(0), reqSkillValue(0), reqItem(0) {}
    LootObject(Player* bot, ObjectGuid guid);
    LootObject(LootObject const& other) = default;
    LootObject& operator=(LootObject const& other) = default;

    bool IsEmpty() { return !guid; }
    bool IsLootPossible(Player* bot);
    // True when the bot is within the server's interaction range for this target: the core
    // creature range for corpses, or the gameobject's own type/display/spell based range.
    bool IsAtInteractDistance(Player* bot);
    void Refresh(Player* bot, ObjectGuid guid);
    WorldObject* GetWorldObject(Player* bot);
    uint32 GetLockType() const { return _lockType; }
    ObjectGuid guid;

    uint32 skillId;
    uint32 reqSkillValue;
    uint32 reqItem;

private:
    void AddLockRequirement(LootLockRequirement const& requirement);

    std::array<LootLockRequirement, MAX_LOOT_LOCK_REQUIREMENTS> _lockRequirements;
    uint8 _lockRequirementCount = 0;
    uint32 _lockType = 0;
    bool _hasUnsupportedLockRequirement = false;
    static bool IsNeededForQuest(Player* bot, uint32 itemId);
};

class LootTarget
{
public:
    LootTarget(ObjectGuid guid);
    LootTarget(ObjectGuid guid, time_t asOfTime);
    LootTarget(LootTarget const& other) = default;

public:
    LootTarget& operator=(LootTarget const& other) = default;
    bool operator<(LootTarget const& other) const;
    bool IsReady() const;
    void Defer();
    //By leewheel 2026-11-08 (bug_107): 供放弃机制判断已重试次数
    uint8 GetRetryCount() const { return _retryCount; }
    //End By leewheel

public:
    ObjectGuid guid;
    time_t asOfTime;

private:
    std::chrono::steady_clock::time_point _retryUntil;
    uint8 _retryCount = 0;
};

class LootTargetList : public std::set<LootTarget>
{
public:
    void shrink(time_t fromTime);
};

class LootObjectStack
{
public:
    LootObjectStack(Player* bot) : bot(bot) {}

    bool Add(ObjectGuid guid);
    void Remove(ObjectGuid guid);
    void Clear();
    LootObject GetLoot(float maxDistance = 0);

    bool IsLootPending();
    void BeginLoot(ObjectGuid guid);
    void LootOpened(ObjectGuid guid);
    void CancelLoot(ObjectGuid guid);
    void RetryLoot(ObjectGuid guid);
    void DeferLoot(ObjectGuid guid);
    bool CanAttemptLoot(ObjectGuid guid) const;

    //By leewheel 2026-11-08 (bug_107): 放弃机制
    //  上游 Defer() 的重试上限只有 16s（5 次退避后恒 16s），对「这个目标怎么试都拿不到」的情形
    //  等于每 16s 无限重试一次 —— 机器人卡在采不了的草药/矿石点上反复上下马就是这个循环。
    //  这里给单次放弃加时间窗：窗口内不进 available loot 队列、也不被 GetNearest 选中。
    uint8 GetRetryCount(ObjectGuid guid) const;
    void GiveUp(ObjectGuid guid, uint32 seconds);
    bool IsGivenUp(ObjectGuid guid);
    bool IsRetryExhausted(ObjectGuid guid) const;
    //End By leewheel

private:
    LootObject GetNearest(float maxDistance = 0);

    Player* bot;
    LootTargetList availableLoot;
    ObjectGuid _pendingLoot;
    std::chrono::steady_clock::time_point _pendingUntil;
    bool _awaitingRelease = false;
    //By leewheel 2026-11-08 (bug_107): guid -> 放弃截止时刻（到期自动恢复，节点重生/条件变化后能重新尝试）
    std::unordered_map<ObjectGuid, std::chrono::steady_clock::time_point> _givenUpUntil;
    //End By leewheel
};

#endif
