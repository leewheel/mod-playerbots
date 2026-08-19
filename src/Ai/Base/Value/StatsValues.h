/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_STATSVALUES_H
#define PLAYERBOTS_STATSVALUES_H

#include "NamedObjectContext.h"
#include "Value.h"

class PlayerbotAI;
class Unit;

class HealthValue : public Uint8CalculatedValue, public Qualified
{
public:
    HealthValue(PlayerbotAI* botAI, std::string const name = "health") : Uint8CalculatedValue(botAI, name) {}

    Unit* GetTarget();
    uint8 Calculate() override;
};

class IsDeadValue : public BoolCalculatedValue, public Qualified
{
public:
    IsDeadValue(PlayerbotAI* botAI, std::string const name = "dead") : BoolCalculatedValue(botAI, name) {}

    Unit* GetTarget();
    bool Calculate() override;
};

class PetIsDeadValue : public BoolCalculatedValue
{
public:
    // By leewheel 2026-08-19
    // 性能优化：checkInterval 由默认 1（每 tick 重算）改为 1000ms。
    // Calculate() 在 bot 无宠物时会执行同步数据库查询（character_pet 表），
    // 每 tick 每 bot 查询会导致大量 DB 阻塞；宠物状态 1 秒刷新完全满足业务需求。
    PetIsDeadValue(PlayerbotAI* botAI, std::string const name = "pet dead")
        : BoolCalculatedValue(botAI, name, 1000) {}
    // End By leewheel

    bool Calculate() override;
};

class PetIsHappyValue : public BoolCalculatedValue
{
public:
    PetIsHappyValue(PlayerbotAI* botAI, std::string const name = "pet happy") : BoolCalculatedValue(botAI, name) {}

    bool Calculate() override;
};

class RageValue : public Uint8CalculatedValue, public Qualified
{
public:
    RageValue(PlayerbotAI* botAI, std::string const name = "rage") : Uint8CalculatedValue(botAI, name) {}

    Unit* GetTarget();
    uint8 Calculate() override;
};

class EnergyValue : public Uint8CalculatedValue, public Qualified
{
public:
    EnergyValue(PlayerbotAI* botAI, std::string const name = "energy") : Uint8CalculatedValue(botAI, name) {}

    Unit* GetTarget();
    uint8 Calculate() override;
};

class ManaValue : public Uint8CalculatedValue, public Qualified
{
public:
    ManaValue(PlayerbotAI* botAI, std::string const name = "mana") : Uint8CalculatedValue(botAI, name) {}

    Unit* GetTarget();
    uint8 Calculate() override;
};

class HasManaValue : public BoolCalculatedValue, public Qualified
{
public:
    HasManaValue(PlayerbotAI* botAI, std::string const name = "has mana") : BoolCalculatedValue(botAI, name, 2 * 1000)
    {
    }

    Unit* GetTarget();
    bool Calculate() override;
};

class ComboPointsValue : public Uint8CalculatedValue, public Qualified
{
public:
    ComboPointsValue(PlayerbotAI* botAI, std::string const name = "combo points") : Uint8CalculatedValue(botAI, name) {}

    Unit* GetTarget();
    uint8 Calculate() override;
};

class IsMountedValue : public BoolCalculatedValue, public Qualified
{
public:
    IsMountedValue(PlayerbotAI* botAI, std::string const name = "mounted") : BoolCalculatedValue(botAI, name) {}

    Unit* GetTarget();
    bool Calculate() override;
};

class IsInCombatValue : public MemoryCalculatedValue<bool>, public Qualified
{
public:
    IsInCombatValue(PlayerbotAI* botAI, std::string const name = "combat") : MemoryCalculatedValue(botAI, name) {}

    Unit* GetTarget();
    bool Calculate() override;
    bool EqualToLast(bool value) override;
};

class BagSpaceValue : public Uint8CalculatedValue
{
public:
    BagSpaceValue(PlayerbotAI* botAI, std::string const name = "bag space") : Uint8CalculatedValue(botAI, name) {}

    uint8 Calculate() override;
};

class DurabilityValue : public Uint8CalculatedValue
{
public:
    DurabilityValue(PlayerbotAI* botAI, std::string const name = "durability") : Uint8CalculatedValue(botAI, name) {}

    uint8 Calculate() override;
};

class SpeedValue : public Uint8CalculatedValue, public Qualified
{
public:
    SpeedValue(PlayerbotAI* botAI, std::string const name = "speed") : Uint8CalculatedValue(botAI, name) {}

    Unit* GetTarget();
    uint8 Calculate() override;
};

class IsInGroupValue : public BoolCalculatedValue
{
public:
    IsInGroupValue(PlayerbotAI* botAI, std::string const name = "in group") : BoolCalculatedValue(botAI, name) {}

    bool Calculate() override;
};

class DeathCountValue : public ManualSetValue<uint32>
{
public:
    DeathCountValue(PlayerbotAI* botAI, std::string const name = "death count") : ManualSetValue<uint32>(botAI, 0, name)
    {
    }
};

class ExperienceValue : public MemoryCalculatedValue<uint32>
{
public:
    ExperienceValue(PlayerbotAI* botAI, std::string const name = "experience", uint32 checkInterval = 60)
        : MemoryCalculatedValue<uint32>(botAI, name, checkInterval)
    {
    }

    bool EqualToLast(uint32 value) override;
    uint32 Calculate() override;
};

#endif
