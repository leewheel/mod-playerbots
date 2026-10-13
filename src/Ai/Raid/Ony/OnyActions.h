/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// RaidOnyxiaActions.h
#ifndef PLAYERBOTS_ONYACTIONS_H
#define PLAYERBOTS_ONYACTIONS_H

#include "AiObjectContext.h"
#include "AttackAction.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"

class PlayerbotAI;

// By leewheel 2026-10-13 黑龙 boss 的 entry 在 60 级团本与 80 级副本里不是同一个：
//   60 级用 NPC_ONYXIA_40 = 301000（creature_template.ScriptName = boss_onyxia_40，
//   刷新点 spawnMask = 4 即 10 人英雄难度），80 级用 NPC_ONYXIA = 10184。
//   原代码只按 "10184" 取目标，而打 60 级黑龙时 10184 那条刷新（spawnMask = 3）根本不生成，
//   于是避深呼吸 / 杀雏龙 / 远离龙蛋 / 侧移四套战术全部恒不激活。此处叠加"依次尝试两个 entry"，
//   取数方式与原逻辑一致，只多出 301000 这一路；10184 仍然优先，不影响 80 级副本。
// End By leewheel
inline Unit* GetOnyxiaBossForBot(PlayerbotAI* botAI)
{
    if (!botAI)
        return nullptr;

    AiObjectContext* context = botAI->GetAiObjectContext();
    if (!context)
        return nullptr;

    if (Unit* boss = context->GetValue<Unit*>("find target", "10184")->Get())
        return boss;

    return context->GetValue<Unit*>("find target", "301000")->Get();
}

// By leewheel 2026-10-13 雏龙 entry 同理：60 级是 NPC_ONYXIAN_WHELP_40 = 301001，
//   80 级是 NPC_ONYXIAN_WHELP = 11262。原来只认 11262，打 60 级时永远找不到雏龙可打。
// End By leewheel
inline bool IsOnyxiaWhelpEntry(uint32 entry)
{
    return entry == 11262 || entry == 301001;
}

class RaidOnyxiaMoveToSideAction : public MovementAction
{
public:
    RaidOnyxiaMoveToSideAction(PlayerbotAI* botAI, std::string const name = "ony move to side")
        : MovementAction(botAI, name)
    {
    }
    bool Execute(Event event) override;
};

class RaidOnyxiaSpreadOutAction : public MovementAction
{
public:
    RaidOnyxiaSpreadOutAction(PlayerbotAI* botAI, std::string const name = "ony spread out")
        : MovementAction(botAI, name)
    {
    }
    bool Execute(Event event) override;
};

struct SafeZone
{
    Position pos;
    float radius;
};

class RaidOnyxiaMoveToSafeZoneAction : public MovementAction
{
public:
    RaidOnyxiaMoveToSafeZoneAction(PlayerbotAI* botAI, std::string const name = "ony move to safe zone")
        : MovementAction(botAI, name)
    {
    }
    bool Execute(Event event) override;

private:
    static std::vector<SafeZone> GetSafeZonesForBreath(uint32 spellId)
    {
        // Safe zone coordinates based on the map
        // Assumes Onyxia's lair map coordinates

        switch (spellId)
        {
            case 17086:  // N to S
            case 18351:  // S to N
                return {
                    SafeZone{Position(-10.0f, -180.0f, -87.0f), 5.0f},
                    SafeZone{Position(-20.0f, -250.0f, -88.0f), 5.0f}
                };  // Bottom Safe Zone

            case 18576:  // E to W
            case 18609:  // W to E
                return {
                    SafeZone{Position(20.0f, -210.0f, -85.5f), 5.0f},
                    SafeZone{Position(-75.0f, -210.0f, -83.4f), 5.0f},
                };  // Left Safe Zone

            case 18564:  // SE to NW
            case 18584:  // NW to SE
                return {
                    SafeZone{Position(-60.0f, -195.0f, -85.0f), 5.0f},
                    SafeZone{Position(10.0f, -240.0f, -85.9f), 5.0f},
                };  // NW Safe Zone

            case 18596:  // SW to NE
            case 18617:  // NE to SW
                return {
                    SafeZone{Position(7.0f, -185.0f, -86.2f), 5.0f},
                    SafeZone{Position(-60.0f, -240.0f, -85.2f), 5.0f},
                };  // NE Safe Zone

            default:
                return {
                    SafeZone{Position(-40.0f, -214.0f, -86.6f), 5.0f}
                };  // Fallback center - shouldn't ever happen
        }
    }
};

class RaidOnyxiaKillWhelpsAction : public AttackAction
{
public:
    RaidOnyxiaKillWhelpsAction(PlayerbotAI* botAI, std::string const name = "ony kill whelps")
        : AttackAction(botAI, name)
    {
    }

    bool Execute(Event event) override;
};

class OnyxiaAvoidEggsAction : public MovementAction
{
public:
    OnyxiaAvoidEggsAction(PlayerbotAI* botAI) : MovementAction(botAI, "ony avoid eggs move") {}

    bool Execute(Event event) override;
};

#endif
