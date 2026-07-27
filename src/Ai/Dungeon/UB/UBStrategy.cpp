/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

//By leewheel 2026-07-27 引入brighton-chi的UB(幽暗沼泽)副本策略
// 幽暗沼泽副本策略实现文件
// Hungarfen（饥饿者）遭遇战：处理恶臭孢子（Foul Spores）和蘑菇孢子云（Spore Cloud）
#include "UBStrategy.h"
#include "Playerbots.h"
#include "UBMultipliers.h"

void TbcDungeonUnderbogStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Boss施放恶臭孢子时，所有人撤退到安全距离
    triggers.push_back(new TriggerNode("ub foul spores", {
        NextAction("ub retreat from foul spores", ACTION_EMERGENCY + 10) }));

    // 蘑菇孢子云危险时，远离危险蘑菇
    triggers.push_back(new TriggerNode("ub spore cloud danger", {
        NextAction("ub vacate spore cloud", ACTION_EMERGENCY + 2) }));
}

void TbcDungeonUnderbogStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // 恶臭孢子期间压制非必要动作
    multipliers.push_back(new HungarfenFoulSporesMultiplier(botAI));
    // DPS不应对蘑菇施放AOE
    multipliers.push_back(new HungarfenMushroomIgnoreMultiplier(botAI));
}

// 目标排除：在与Hungarfen战斗时，将蘑菇从可选目标中排除
void TbcDungeonUnderbogStrategy::AppendTargetExclusions(GuidSet& exclusions, TargetValueExclusionType /*type*/)
{
    AiObjectContext* context = botAI->GetAiObjectContext();
    if (!AI_VALUE2(Unit*, "find target", "hungarfen"))
        return;

    GuidVector const& mushrooms = AI_VALUE_REF(GuidVector, "ub mushrooms");
    exclusions.insert(mushrooms.begin(), mushrooms.end());
}
