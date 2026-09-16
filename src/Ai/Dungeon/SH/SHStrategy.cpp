/*
* This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
* information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License, or (at your option) any later version.
*/

#include "SHStrategy.h"
#include "SHMultipliers.h"

// By leewheel 2026-09-16 破碎大厅（map 540）机器人策略
void TbcDungeonShatteredHallsStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    // 高阶术士奈瑟库斯（16807）
    // 25% 血的暗影旋风是原地 AoE + 击退，非坦克必须离开近战圈。
    triggers.push_back(new TriggerNode("nethekurse dark spin", {
        NextAction("nethekurse avoid dark spin", ACTION_EMERGENCY + 1) }));

    // 血卫士波伦（20923 英雄）/ 血卫士（17461 普通）
    // 弓手火焰箭在地面留下烈焰，踩上去就持续掉血，立刻挪窝即可。
    triggers.push_back(new TriggerNode("porung blaze on ground", {
        NextAction("porung move out of blaze", ACTION_EMERGENCY + 2) }));

    // 战争使者沃姆罗格（16809）
    // 燃烧之锤阶段伴随恐惧与反复的火焰冲击波，非坦克远离近战圈最省事。
    triggers.push_back(new TriggerNode("omrogg burning maul", {
        NextAction("omrogg avoid burning maul", ACTION_EMERGENCY + 1) }));

    // 酋长卡加斯·刃拳（16808）
    // 刃舞：连续 8 次冲入 5~16 码范围，非坦克站到 20 码外。
    triggers.push_back(new TriggerNode("kargath blade dance", {
        NextAction("kargath avoid blade dance", ACTION_EMERGENCY + 1) }));

    // 破碎刺客会死后 20 秒复活，集中火力秒掉更划算（机制标记位挂骷髅）。
    triggers.push_back(new TriggerNode("kargath assassins are active", {
        NextAction("kargath mark assassins", ACTION_RAID + 1) }));

    // 下水道：玩家跳下去之后机器人跟着跳下去
    // （参考 SWP 艾瑞达双子的"阳台跳下"：MoveTo 到边缘 + JumpTo 落点）
    triggers.push_back(new TriggerNode("shattered halls master in sewer", {
        NextAction("shattered halls follow master into sewer", ACTION_EMERGENCY + 3) }));
}

void TbcDungeonShatteredHallsStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    multipliers.push_back(new KargathBladeDanceMultiplier(botAI));

    multipliers.push_back(new ShatteredHallsSewerJumpMultiplier(botAI));
}
