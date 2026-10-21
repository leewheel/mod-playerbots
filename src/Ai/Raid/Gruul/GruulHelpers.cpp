/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GruulHelpers.h"
#include "AiFactory.h"
#include "GroupReference.h"
#include "Playerbots.h"
#include "Unit.h"

namespace GruulsLairHelpers
{

const Position MAULGAR_TANK_POSITION  = {  90.686f, 167.047f, -13.234f };
const Position OLM_TANK_POSITION      = { 101.050f, 219.359f,  -9.503f };
const Position BLINDEYE_TANK_POSITION = {  99.681f, 213.989f, -10.345f };
const Position KROSH_TANK_POSITION    = { 116.880f, 166.208f, -14.231f };
const Position MAULGAR_ROOM_CENTER    = {  88.754f, 150.759f, -11.569f };
const Position GRUUL_TANK_POSITION    = { 241.238f, 365.025f,  -4.220f };

Player* GetKroshMageTank(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    // (1) First loop: Return the first assistant Mage (real player or bot)
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->getClass() != CLASS_MAGE)
            continue;

        if (group->IsAssistant(member->GetGUID()))
            return member;

    }

    // (2) Fall back to bot Mage with highest HP
    Player* highestHpMage = nullptr;
    uint32 highestHp = 0;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member) ||
            member->getClass() != CLASS_MAGE)
        {
            continue;
        }

        uint32 hp = member->GetMaxHealth();
        if (!highestHpMage || hp > highestHp)
        {
            highestHpMage = member;
            highestHp = hp;
        }
    }

    if (highestHpMage)
        return highestHpMage;

    // (3) No Mage (a CoA raid has none): a third tank, else the ranged damage dealer with the most health. Krosh and
    // his Greater Fireballs (8.7k) went on the main tank until she died (ladiabale, Gruul 2.5).
    return GetCoaOgreHolder(group, nullptr, true);
}

// A bot that holds a caster ogre in a raid without vanilla classes: a tank that is neither main tank nor one of the
// two assist tanks (if tanks are allowed), else the living ranged non-healer bot with the most health, not the
// holder of the other ogre.
Player* GetCoaOgreHolder(Group* group, Player* exclude, bool tanks)
{
    if (tanks)
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive() && member != exclude && GET_PLAYERBOT_AI(member) &&
                PlayerbotAI::IsTank(member) && !PlayerbotAI::IsMainTank(member) &&
                !PlayerbotAI::IsAssistTankOfIndex(member, 0, false) && !PlayerbotAI::IsAssistTankOfIndex(member, 1, false))
                return member;
        }

    Player* best = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member == exclude || !GET_PLAYERBOT_AI(member) ||
            !PlayerbotAI::IsRanged(member) || PlayerbotAI::IsHeal(member) || PlayerbotAI::IsTank(member))
            continue;
        if (!best || member->GetMaxHealth() > best->GetMaxHealth())
            best = member;
    }
    return best;
}

Player* GetKigglerMoonkinTank(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    // (1) First loop: Return the first assistant Moonkin (real player or bot)
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->getClass() != CLASS_DRUID)
            continue;

        if (group->IsAssistant(member->GetGUID()) && AiFactory::GetPlayerSpecTab(member) == DRUID_TAB_BALANCE)
            return member;
    }

    // (2) Fall back to bot Moonkin with highest HP
    Player* highestHpMoonkin = nullptr;
    uint32 highestHp = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->getClass() != CLASS_DRUID ||
            !GET_PLAYERBOT_AI(member) || AiFactory::GetPlayerSpecTab(member) != DRUID_TAB_BALANCE)
        {
            continue;
        }

        uint32 hp = member->GetMaxHealth();
        if (!highestHpMoonkin || hp > highestHp)
        {
            highestHpMoonkin = member;
            highestHp = hp;
        }
    }

    if (highestHpMoonkin)
        return highestHpMoonkin;

    // No Moonkin: a ranged damage dealer at 28 yd, not the one on Krosh.
    return GetCoaOgreHolder(group, GetKroshMageTank(bot), false);
}

}
