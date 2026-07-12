#include "MCStrategy.h"

#include "MCMultipliers.h"
#include "Strategy.h"
#include "MCHelpers.h"

void RaidMcStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Lucifron
    triggers.push_back(
        new TriggerNode("mc lucifron shadow resistance",
                        { NextAction("mc lucifron shadow resistance", ACTION_RAID) }));

    // Magmadar
    // TODO: Fear ward / tremor totem, or general anti-fear strat development. Same as King Dred (Drak'Tharon) and faction commander (Nexus).
    triggers.push_back(
        new TriggerNode("mc magmadar fire resistance",
                        { NextAction("mc magmadar fire resistance", ACTION_RAID) }));

    // Gehennas
    triggers.push_back(
        new TriggerNode("mc gehennas shadow resistance",
                        { NextAction("mc gehennas shadow resistance", ACTION_RAID) }));

    // Garr
    triggers.push_back(
        new TriggerNode("mc garr fire resistance",
                        { NextAction("mc garr fire resistance", ACTION_RAID) }));

    // Baron Geddon
    triggers.push_back(
        new TriggerNode("mc baron geddon fire resistance",
                        { NextAction("mc baron geddon fire resistance", ACTION_RAID) }));
    triggers.push_back(
        new TriggerNode("mc living bomb debuff",
                        { NextAction("mc move from group", ACTION_RAID) }));
    triggers.push_back(
        new TriggerNode("mc baron geddon inferno",
                        { NextAction("mc move from baron geddon", ACTION_RAID) }));

    // Shazzrah
    triggers.push_back(
        new TriggerNode("mc shazzrah ranged",
                        { NextAction("mc shazzrah move away", ACTION_RAID) }));

    // Sulfuron Harbinger
    // Alternatively, shadow resistance is also possible.
    triggers.push_back(
        new TriggerNode("mc sulfuron harbinger fire resistance",
                        { NextAction("mc sulfuron harbinger fire resistance", ACTION_RAID) }));

    // Golemagg the Incinerator
    triggers.push_back(
        new TriggerNode("mc golemagg fire resistance",
                        { NextAction("mc golemagg fire resistance", ACTION_RAID) }));
    triggers.push_back(
        new TriggerNode("mc golemagg mark boss",
                        { NextAction("mc golemagg mark boss", ACTION_RAID) }));
    triggers.push_back(
        new TriggerNode("mc golemagg is main tank",
                        { NextAction("mc golemagg main tank attack golemagg", ACTION_RAID) }));
    triggers.push_back(
        new TriggerNode("mc golemagg is assist tank",
                        { NextAction("mc golemagg assist tank attack core rager", ACTION_RAID) }));

    // Majordomo Executus
    triggers.push_back(
        new TriggerNode("mc majordomo shadow resistance",
                        { NextAction("mc majordomo shadow resistance", ACTION_RAID) }));

    // Ragnaros
    triggers.push_back(
        new TriggerNode("mc ragnaros fire resistance",
                        { NextAction("mc ragnaros fire resistance", ACTION_RAID) }));

    // Trash
    triggers.push_back(
        new TriggerNode("mc core hound mark",
                        { NextAction("mc core hound mark", ACTION_RAID) }));

    //By leewheel 2026年7月12日
    // 自定义Boss: Smolder (NPC 83001)
    // 火焰抗性 — Boss使用多种火焰法术
    triggers.push_back(
        new TriggerNode("mc smolder fire resistance",
                        { NextAction("mc smolder fire resistance", ACTION_RAID) }));
    // 火焰海啸规避 — 火焰海啸NPC在附近时机器人需要远离
    triggers.push_back(
        new TriggerNode("mc smolder flame tsunami",
                        { NextAction("mc smolder avoid flame tsunami", ACTION_RAID + 5) }));
    // 反恐结界 — Boss使用AOE恐惧，牧师需要在坦克身上保持反恐结界
    triggers.push_back(
        new TriggerNode("mc smolder fear ward",
                        { NextAction("mc smolder fear ward", ACTION_RAID) }));

    // 自定义Boss: Hazzrash (NPC 83000)
    // Evocation阶段 — Boss引导唤醒术暂停攻击，机器人可利用此时间输出
    triggers.push_back(
        new TriggerNode("mc hazzrash evocation",
                        { NextAction("mc hazzrash evocation", ACTION_RAID) }));
    // 远程散开 — Boss使用连锁燃烧，远程需要保持距离
    triggers.push_back(
        new TriggerNode("mc hazzrash ranged spread",
                        { NextAction("mc hazzrash ranged spread", ACTION_RAID) }));
    //End By leewheel
}

void RaidMcStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new GarrDisableDpsAoeMultiplier(botAI));
    multipliers.push_back(new BaronGeddonAbilityMultiplier(botAI));
    multipliers.push_back(new GolemaggMultiplier(botAI));
}
