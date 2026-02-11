/*
 * This is Leewheel Script Project
 */

#include "NaxxStrategy.h"
#include "Playerbots.h"

//By Leewheel 2026-02-11
void NaxxramasStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("in naxxramas", 
        { NextAction("naxx", 90.0f) }));
}
//End By Leewheel

//By Leewheel 2026-02-11
void NaxxAnubrekhanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("anubrekhan locust swarm", 
        { NextAction("spread out", 80.0f) }));
    triggers.push_back(new TriggerNode("crypt guard spawned", 
        { NextAction("attack crypt guard", 70.0f) }));
}
//End By Leewheel

//By Leewheel 2026-02-11
void NaxxFaerlinaStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("faerlina frenzy", 
        { NextAction("use widow's embrace", 85.0f) }));
    triggers.push_back(new TriggerNode("faerlina worshipper", 
        { NextAction("kill worshipper", 75.0f) }));
}
//End By Leewheel

//By Leewheel 2026-02-11
void NaxxMaexxnaStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("web wrap target", 
        { NextAction("free web wrap", 90.0f) }));
    triggers.push_back(new TriggerNode("web spray", 
        { NextAction("dispel web spray", 80.0f) }));
    triggers.push_back(new TriggerNode("spiderlings spawned", 
        { NextAction("kill spiderlings", 70.0f) }));
}
//End By Leewheel

//By Leewheel 2026-02-11
void NaxxPatchwerkStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("patchwerk hateful strike", 
        { NextAction("position for hateful", 85.0f) }));
    triggers.push_back(new TriggerNode("patchwerk berserk", 
        { NextAction("burn cooldowns", 95.0f) }));
}
//End By Leewheel

//By Leewheel 2026-02-11
void NaxxGrobbulusStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("grobbulus injection", 
        { NextAction("move away", 80.0f) }));
    triggers.push_back(new TriggerNode("poison cloud", 
        { NextAction("avoid poison cloud", 75.0f) }));
}
//End By Leewheel

//By Leewheel 2026-02-11
void NaxxGluthStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("gluth decimate", 
        { NextAction("kill zombies", 85.0f) }));
    triggers.push_back(new TriggerNode("mortal wound", 
        { NextAction("tank rotation", 80.0f) }));
}
//End By Leewheel

//By Leewheel 2026-02-11
void NaxxNothStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("noth teleport", 
        { NextAction("position noth", 85.0f) }));
    triggers.push_back(new TriggerNode("skeletons spawned", 
        { NextAction("aoe skeletons", 75.0f) }));
}
//End By Leewheel

//By Leewheel 2026-02-11
void NaxxHeiganStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("heigan dance", 
        { NextAction("dance heigan", 95.0f) }));
    triggers.push_back(new TriggerNode("eruption soon", 
        { NextAction("move to safe zone", 90.0f) }));
}
//End By Leewheel

//By Leewheel 2026-02-11
void NaxxLoathebStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("loatheb necrotic aura", 
        { NextAction("heal rotation", 85.0f) }));
    triggers.push_back(new TriggerNode("spore spawned", 
        { NextAction("kill spore", 70.0f) }));
}
//End By Leewheel

//By Leewheel 2026-02-11
void NaxxRazuviousStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("razuvious shout", 
        { NextAction("disrupt shout", 85.0f) }));
    triggers.push_back(new TriggerNode("death knight understudy", 
        { NextAction("mind control understudy", 80.0f) }));
}
//End By Leewheel

//By Leewheel 2026-02-11
void NaxxGothikStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("gothik wave", 
        { NextAction("position gothik", 80.0f) }));
    triggers.push_back(new TriggerNode("gothik central", 
        { NextAction("central room", 85.0f) }));
}
//End By Leewheel

//By Leewheel 2026-02-11
void NaxxFourHorsemenStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("horseman mark", 
        { NextAction("switch targets", 80.0f) }));
    triggers.push_back(new TriggerNode("meteor", 
        { NextAction("spread out", 85.0f) }));
}
//End By Leewheel

//By Leewheel 2026-02-11
void NaxxSapphironStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("sapphiron flight", 
        { NextAction("position ice block", 90.0f) }));
    triggers.push_back(new TriggerNode("chill", 
        { NextAction("move behind ice block", 85.0f) }));
}
//End By Leewheel

//By Leewheel 2026-02-11
void NaxxKelthuzadStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("kelthuzad frost blast", 
        { NextAction("spread out", 85.0f) }));
    triggers.push_back(new TriggerNode("kelthuzad chains", 
        { NextAction("break chains", 80.0f) }));
    triggers.push_back(new TriggerNode("guardians spawned", 
        { NextAction("kill guardians", 75.0f) }));
}
//End By Leewheel

//By Leewheel 2026-02-11
void NaxxThaddiusStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("thaddius polarity shift", 
        { NextAction("move to polarity", 95.0f) }));
    triggers.push_back(new TriggerNode("tesla coil", 
        { NextAction("attack tesla coil", 70.0f) }));
}
//End By Leewheel
