/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SMStrategy.h"
#include "SMMultipliers.h"

void VanillaDungeonSMStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Arcanist Doan
    triggers.push_back(new TriggerNode("arcanist doan boss casting detonation", {
        NextAction("arcanist doan move out of detonation", ACTION_EMERGENCY + 10) }));
    // The rest of the fight, casters and healers stand outside his Silence and Arcanist's Explosion.
    triggers.push_back(new TriggerNode("arcanist doan ranged", {
        NextAction("arcanist doan ranged keep away", ACTION_RAID) }));

    // Herod: the damage dealers and healers step out of Whirlwind; the tank holds him.
    triggers.push_back(new TriggerNode("herod whirlwind", {
        NextAction("herod move from whirlwind", ACTION_RAID + 1) }));

    // Mograine and Whitemane: once she has raised him, Whitemane dies first.
    triggers.push_back(new TriggerNode("whitemane focus", {
        NextAction("attack whitemane", ACTION_RAID) }));
}

void VanillaDungeonSMStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new ArcanistDoanDetonationMultiplier(botAI));
    multipliers.push_back(new HerodWhirlwindMultiplier(botAI));
    multipliers.push_back(new WhitemaneFocusMultiplier(botAI));
}
