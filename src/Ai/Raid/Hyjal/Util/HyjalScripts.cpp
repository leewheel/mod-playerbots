/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AllCreatureScript.h"
#include "HyjalHelpers.h"
#include "Player.h"
#include "Playerbots.h"
#include "ScriptMgr.h"
#include "Spell.h"

using namespace HyjalHelpers;

// Both spell listeners are driven by DoCastRandomTarget, which always has an explicit unit target.

namespace
{

Player* GetTargetedPlayer(Spell* spell)
{
    if (!spell)
        return nullptr;

    if (Unit* unitTarget = spell->m_targets.GetUnitTarget())
        return unitTarget->ToPlayer();

    return nullptr;
}

// 引入 brighton-chi/the-lab 的 archimonde 脚本修复（5ff7f49c5）：
// Air Burst 打断判断改用施法者(caster)的当前仇恨目标(activeTank)，
// 取代原先依赖 botAI 查找 archimonde 再取 mainTank 的方式，逻辑更准确。
// --By leewheel 2026-08-21
bool ShouldInterruptForArchimondeAirBurst(Player* bot, Unit* caster, Player* target)
{
    if (!target)
        return false;

    Unit* activeTank = caster->GetVictim();
    if (!activeTank || activeTank == bot)
        return false;

    if (target != activeTank && target != bot)
        return false;

    float const distanceToActiveTank = bot->GetExactDist2d(activeTank);
    return distanceToActiveTank < AIR_BURST_SAFE_DISTANCE;
}

} // namespace

// Interrupts a cast when a Doomfire NPC comes too close. The trail it leaves behind is made of
// SPELL_DOOMFIRE_TRAIL dynamic objects, which bots query directly, so nothing is recorded here.
//
// Keyed on the Doomfire itself and not on the Doomfire Spirit it follows. The Spirit is the invisible
// one, and it does not walk--it NearTeleportTo's up to 8 yards every 1600ms--so reading it would let
// bots react to a position no human can see, and to one the fire has not reached yet. The Doomfire
// walks after it carrying 31945, a 1s periodic that drops the next patch at its own feet, so its
// position is where fire is about to be. That is what makes DOOMFIRE_DANGER_RADIUS the right figure
// here: the same distance the avoidance keeps from patches already on the ground
class ArchimondeDoomfireTrailScript : public AllCreatureScript
{
public:
    ArchimondeDoomfireTrailScript() : AllCreatureScript("ArchimondeDoomfireTrailScript") {}

    void OnAllCreatureUpdate(Creature* creature, uint32 /*diff*/) override
    {
        if (creature->GetEntry() != Id(HyjalNpcs::NPC_DOOMFIRE))
            return;

        Map::PlayerList const& players = creature->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive())
                continue;

            PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
            // Centre to centre, as the avoidance measures. GetDistance would subtract both object
            // sizes, quietly making this wider than the figure it shares
            if (!botAI || !botAI->HasStrategy("hyjal", BOT_STATE_COMBAT) ||
                creature->GetExactDist2d(player) > DOOMFIRE_DANGER_RADIUS)
            {
                continue;
            }

            botAI->RequestSpellInterrupt();
        }
    }
};

// Air Burst is a 2s cast that hits all players within 13y of the target.
class ArchimondeAirBurstSpellListenerScript : public AllSpellScript
{
public:
    ArchimondeAirBurstSpellListenerScript() :
        AllSpellScript("ArchimondeAirBurstSpellListenerScript") {}

    void OnSpellPrepare(Spell* spell, Unit* caster, SpellInfo const* spellInfo) override
    {
        if (spellInfo->Id != Id(HyjalSpells::SPELL_AIR_BURST))
            return;

        Player* target = GetTargetedPlayer(spell);
        if (!target)
            return;

        archimondeAirBurstTargets[caster->GetMap()->GetInstanceId()] =
            AirBurstData{ target->GetGUID(), getMSTime() };

        Map::PlayerList const& players = caster->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive())
                continue;

            PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
            if (!botAI || !botAI->HasStrategy("hyjal", BOT_STATE_COMBAT) ||
                !ShouldInterruptForArchimondeAirBurst(player, caster, target))
            {
                continue;
            }

            botAI->RequestSpellInterrupt();
        }
    }
};

// Inferno summons a Towering Infernal at its target's then-current position after a 3.5s cast.
class AnetheronInfernoSpellListenerScript : public AllSpellScript
{
public:
    AnetheronInfernoSpellListenerScript() :
        AllSpellScript("AnetheronInfernoSpellListenerScript") {}

    void OnSpellPrepare(Spell* spell, Unit* /*caster*/, SpellInfo const* spellInfo) override
    {
        if (spellInfo->Id != Id(HyjalSpells::SPELL_INFERNO))
            return;

        Player* target = GetTargetedPlayer(spell);
        if (!target)
            return;

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(target);
        if (!botAI || !botAI->HasStrategy("hyjal", BOT_STATE_COMBAT))
            return;

        botAI->RequestSpellInterrupt();
    }
};

void AddSC_HyjalSummitBotScripts()
{
    new ArchimondeDoomfireTrailScript();
    new ArchimondeAirBurstSpellListenerScript();
    new AnetheronInfernoSpellListenerScript();
}
