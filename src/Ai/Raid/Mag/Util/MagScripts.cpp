/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AllSpellScript.h"
#include "DynamicObject.h"
#include "DynamicObjectScript.h"
#include "MagHelpers.h"
#include "Playerbots.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "Timer.h"

using namespace MagHelpers;

// By leewheel 2026-09-13 合并brighton 8c96a663:
//   本文件整体采用上游版本。我方原先只有 MagtheridonBotSpellScript(即此处的
//   MagtheridonQuakeSpellListenerScript)并带注释 "NOTE: Need to add DynObj Script also for
//   spell interrupt in Debris" —— 上游 c8f273d2 已按该 TODO 补上 MagtheridonDebrisDynamicObjectScript，
//   本核心 ScriptMgr/DynamicObjectScript.h 与 DynamicObject.cpp:176 的 OnDynamicObjectUpdate 派发链路齐全，
//   故恢复该脚本（机器人在 Magtheridon 落石(30630)上会主动打断自身施法）。
//   计时器查询采用上游 Object::GetInstanceId()（等价于原 GetMap()->GetInstanceId()）。
// End By leewheel 2026-09-13
class MagtheridonQuakeSpellListenerScript : public AllSpellScript
{
public:
    MagtheridonQuakeSpellListenerScript() : AllSpellScript("MagtheridonQuakeSpellListenerScript") {}

    void OnSpellCast(
        Spell* /*spell*/, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        if (spellInfo->Id != static_cast<uint32>(MagSpells::SPELL_QUAKE))
            return;

        // To account for Blast Nova delay caused by Quake's DelayAll(6999ms).
        auto it = blastNovaTimer.find(caster->GetInstanceId());
        if (it != blastNovaTimer.end())
            it->second += 7 * IN_MILLISECONDS;
    }
};

// Not to be confused with the 30% ceiling collapse, which is also called "Debris." This is for the
// small patches that fall from time-to-time after the ceiling collapses, which deal 87,500 to
// 112,500 damage on hit (!!!). This is potentially higher damage than Archimonde's instant-wipe
// ability if he reaches the Well of Eternity, Hand of Death, which hits for 99,999.
class MagtheridonDebrisDynamicObjectScript : public DynamicObjectScript
{
public:
    MagtheridonDebrisDynamicObjectScript() :
        DynamicObjectScript("MagtheridonDebrisDynamicObjectScript") {}

    void OnUpdate(DynamicObject* debris, uint32 /*diff*/) override
    {
        if (debris->GetSpellId() != Id(MagSpells::SPELL_DEBRIS_SPAWN))
            return;

        Map::PlayerList const& players = debris->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive())
                continue;

            PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
            if (!botAI || !botAI->HasStrategy("magtheridon", BOT_STATE_COMBAT) ||
                debris->GetExactDist2d(player) > DEBRIS_HAZARD_RADIUS)
            {
                continue;
            }

            botAI->RequestSpellInterrupt();
        }
    }
};

void AddSC_MagtheridonBotScripts()
{
    new MagtheridonQuakeSpellListenerScript();
    new MagtheridonDebrisDynamicObjectScript();
}
