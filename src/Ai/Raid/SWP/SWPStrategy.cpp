//By leewheel 2026-07-08
/*
 * 太阳之井高地 (Sunwell Plateau) 策略实现
 * 作者: leewheel
 * 注册所有触发器和乘数器
 */
//End By leewheel

#include "SWPStrategy.h"

#include "SWPMultipliers.h"

void RaidSunwellPlateauStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // ===== 通用 =====
    triggers.push_back(new TriggerNode("sunwell bot is not in combat", {
        NextAction("sunwell erase timers and trackers", ACTION_EMERGENCY + 11) }));

    // ===== 卡雷苟斯 (Kalecgos) =====
    triggers.push_back(new TriggerNode("kalecgos pulling boss", {
        NextAction("kalecgos misdirect boss to main tank", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("kalecgos boss engaged by tanks", {
        NextAction("kalecgos tanks position boss", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("kalecgos boss engaged by ranged", {
        NextAction("kalecgos ranged disperse", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("kalecgos need enter spectral realm", {
        NextAction("kalecgos enter spectral realm", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("kalecgos in spectral realm", {
        NextAction("kalecgos attack sathrovarr", ACTION_RAID + 3) }));

    // 内外场血量同步：当血量差异>10%时控制DPS节奏
    triggers.push_back(new TriggerNode("kalecgos health not synced", {
        NextAction("kalecgos health sync", ACTION_RAID + 4) }));

    // 奥术冲击层数过高：进入幽灵领域刷新debuff
    triggers.push_back(new TriggerNode("kalecgos need arcane buffet reset", {
        NextAction("kalecgos manage arcane buffet", ACTION_EMERGENCY + 6) }));

    // 无尽痛苦诅咒驱散：附近队友中诅咒时优先驱散
    triggers.push_back(new TriggerNode("kalecgos curse of boundless agony", {
        NextAction("kalecgos dispelling curse", ACTION_EMERGENCY + 5) }));

    // 冰霜吐息驱散：主坦中冰霜吐息时立即驱散防倒T
    triggers.push_back(new TriggerNode("kalecgos frost breath on tank", {
        NextAction("kalecgos dispelling frost breath", ACTION_EMERGENCY + 5) }));

    // ===== 布鲁塔卢斯 (Brutallus) =====
    triggers.push_back(new TriggerNode("brutallus pulling boss", {
        NextAction("brutallus misdirect boss to main tank", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("brutallus boss engaged by tanks", {
        NextAction("brutallus tanks position boss", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("brutallus casting meteor slash", {
        NextAction("brutallus soak meteor slash", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("brutallus bot has burn", {
        NextAction("brutallus burn move away", ACTION_EMERGENCY + 8) }));

    // ===== 菲米丝 (Felmyst) =====
    triggers.push_back(new TriggerNode("felmyst pulling boss", {
        NextAction("felmyst misdirect boss to main tank", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("felmyst boss engaged by tanks", {
        NextAction("felmyst tanks position boss", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("felmyst casting gas nova", {
        NextAction("felmyst disperse from gas nova", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("felmyst casting encapsulate", {
        NextAction("felmyst avoid encapsulate", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("felmyst in flight phase", {
        NextAction("felmyst flight phase spread", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("felmyst need to manage phase timer", {
        NextAction("felmyst manage phase timer", ACTION_EMERGENCY + 10) }));

    // ===== 艾瑞达双子 (Eredar Twins) =====
    triggers.push_back(new TriggerNode("eredar twins pulling bosses", {
        NextAction("eredar twins misdirect boss to tanks", ACTION_RAID + 4) }));

    triggers.push_back(new TriggerNode("eredar twins determining kill order", {
        NextAction("eredar twins assign kill order", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("eredar twins bot has dark touched", {
        NextAction("eredar twins move to flame source", ACTION_EMERGENCY + 5) }));

    triggers.push_back(new TriggerNode("eredar twins bot has flame touched", {
        NextAction("eredar twins move to shadow source", ACTION_EMERGENCY + 5) }));

    triggers.push_back(new TriggerNode("eredar twins bot has conflagration", {
        NextAction("eredar twins avoid conflagration", ACTION_EMERGENCY + 7) }));

    // ===== 穆鲁 (Muru) =====
    triggers.push_back(new TriggerNode("muru entropius spawned", {
        NextAction("muru misdirect boss to main tank", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("muru adds spawned", {
        NextAction("muru handle adds", ACTION_EMERGENCY + 8) }));

    triggers.push_back(new TriggerNode("muru void sentinel spawned", {
        NextAction("muru handle void sentinel", ACTION_EMERGENCY + 9) }));

    triggers.push_back(new TriggerNode("muru casting darkness", {
        NextAction("muru avoid darkness", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("muru entropius phase", {
        NextAction("muru entropius phase", ACTION_RAID + 2) }));

    // ===== 基尔加丹 (Kil'jaeden) =====
    triggers.push_back(new TriggerNode("kil'jaeden pulling boss", {
        NextAction("kil'jaeden misdirect to tank", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("kil'jaeden casting darkness of souls", {
        NextAction("kil'jaeden avoid darkness", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("kil'jaeden casting armageddon", {
        NextAction("kil'jaeden avoid armageddon", ACTION_EMERGENCY + 8) }));

    triggers.push_back(new TriggerNode("kil'jaeden spawned sinister reflection", {
        NextAction("kil'jaeden handle sinister reflection", ACTION_EMERGENCY + 9) }));

    triggers.push_back(new TriggerNode("kil'jaeden shield orb spawned", {
        NextAction("kil'jaeden handle shield orb", ACTION_RAID + 4) }));

    triggers.push_back(new TriggerNode("kil'jaeden need to manage phase", {
        NextAction("kil'jaeden manage phase", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("kil'jaeden boss engaged by ranged", {
        NextAction("kil'jaeden ranged disperse", ACTION_RAID + 1) }));
}

void RaidSunwellPlateauStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // 卡雷苟斯
    multipliers.push_back(new KalecgosDelayDpsCooldownsMultiplier(botAI));
    multipliers.push_back(new KalecgosControlMovementMultiplier(botAI));

    // 布鲁塔卢斯
    multipliers.push_back(new BrutallusDelayDpsCooldownsMultiplier(botAI));
    multipliers.push_back(new BrutallusControlMovementMultiplier(botAI));

    // 菲米丝
    multipliers.push_back(new FelmystDelayDpsCooldownsMultiplier(botAI));
    multipliers.push_back(new FelmystControlMovementMultiplier(botAI));

    // 艾瑞达双子
    multipliers.push_back(new EredarTwinsDelayDpsCooldownsMultiplier(botAI));
    multipliers.push_back(new EredarTwinsControlMovementMultiplier(botAI));

    // 穆鲁
    multipliers.push_back(new MuruDelayDpsCooldownsMultiplier(botAI));
    multipliers.push_back(new MuruControlMovementMultiplier(botAI));

    // 基尔加丹
    multipliers.push_back(new KiljaedenDelayDpsCooldownsMultiplier(botAI));
    multipliers.push_back(new KiljaedenControlMovementMultiplier(botAI));
}
