/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "MagStrategy.h"
#include "MagMultipliers.h"

void RaidMagtheridonStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("magtheridon no encounter in progress", {
        NextAction("magtheridon reset encounter states", ACTION_EMERGENCY + 12) }));

    triggers.push_back(new TriggerNode("magtheridon first three channelers engaged by main tank", {
        NextAction("magtheridon main tank attack first three channelers", ACTION_RAID + 1) }));

    // By leewheel 2026-08-18 - 修正 TriggerNode 名与 MagTriggerContext/MagTriggers 实际注册名不一致：strategy 原引用"magtheridon nw channeler engaged by first assist tank"，但该名字不存在于 trigger context 中，导致副坦处理西北/东北两个 channeler 的策略永远无法触发（功能失效）；改为实际注册名"magtheridon last two channelers engaged by assist tanks"
    triggers.push_back(new TriggerNode("magtheridon last two channelers engaged by assist tanks", {
        NextAction("magtheridon assist tanks attack last two channelers", ACTION_RAID + 1) }));
    // End By leewheel

    triggers.push_back(new TriggerNode("magtheridon pulling west and east channelers", {
        NextAction("magtheridon misdirect hellfire channelers to main tank", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("magtheridon determining kill order", {
        NextAction("magtheridon assign dps priority", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("magtheridon burning abyssal spawned", {
        NextAction("magtheridon warlock cc burning abyssal", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("magtheridon boss engaged by main tank", {
        NextAction("magtheridon main tank position boss", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("magtheridon boss engaged by ranged", {
        NextAction("magtheridon spread ranged", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("magtheridon standing in debris", {
        NextAction("magtheridon move out of debris", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("magtheridon incoming blast nova", {
        NextAction("magtheridon use manticron cube", ACTION_EMERGENCY + 9) }));

    triggers.push_back(new TriggerNode("magtheridon need to manage timers and assignments", {
        NextAction("magtheridon manage timers and assignments", ACTION_EMERGENCY + 11) }));
}

void RaidMagtheridonStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new MagtheridonUseManticronCubeMultiplier(botAI));
    multipliers.push_back(new MagtheridonWaitToAttackMultiplier(botAI));
    multipliers.push_back(new MagtheridonControlTankActionsMultiplier(botAI));
    multipliers.push_back(new MagtheridonDebrisDangerMultiplier(botAI));
}
