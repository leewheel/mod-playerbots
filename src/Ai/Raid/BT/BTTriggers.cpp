/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BTTriggers.h"
#include "BTHelpers.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "SharedDefines.h"
#include "Spell.h"
#include <algorithm>
#include <vector>

using namespace BtHelpers;
using namespace EncounterHelpers;

// Shared

bool BlackTempleNoEncounterInProgressTrigger::IsActive()
{
    return !IsEncounterInProgress(bot, BT_MAP_ID);
}

bool BlackTempleHunterShouldMisdirectTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", _bossName);
    return boss && boss->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT;
}

// Trash

bool SisterOfPleasureShouldBeFocusedTrigger::IsActive()
{
    if (IsMechanicTrackerBot(bot, BT_MAP_ID))
        return !GetSkulledSisterOfPain(bot) && FindLinkedSisterOfPleasure(botAI);

    if (PlayerbotAI::IsTank(bot))
        return false;

    Unit* target = AI_VALUE(Unit*, "current target");
    return target && target->GetEntry() == Id(BtNpcs::NPC_SISTER_OF_PAIN) &&
        GetSkulledSisterOfPain(bot) == target;
}

bool ShadowmoonReaverWandBuildsChargesTrigger::IsActive()
{
    Spell* wand = bot->GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL);
    if (!wand || GetChaoticChargeReach(wand->m_spellInfo) == ChaoticChargeReach::None)
        return false;

    return IsShadowmoonReaverUnsafeForMagic(wand->m_targets.GetUnitTarget());
}

bool ShadowmoonReaverShouldControlCasterPetTrigger::IsActive()
{
    Guardian* pet = bot->GetGuardianPet();
    if (!IsChargeBuildingPet(pet))
        return false;

    // Also true while the pet is passive, so the action can restore a stance it set.
    return pet->HasReactState(REACT_PASSIVE) ||
        !context->GetValue<GuidVector>("shadowmoon reavers")->RefGet().empty();
}

// High Warlord Naj'entus

bool HighWarlordNajentusShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "22887");
}

bool HighWarlordNajentusRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    //By leewheel 2026年10月18日 上游本轮此处为英文名 "high warlord naj'entus"；
    // 本核生物名为中文，英文 qualifier 做子串匹配恒失败（见 src/Ai/Base/Value/TargetValue.cpp 08-21 修复注释）
    // ⇒ 按三方核定映射表回退 entry 22887（实库 creature_template + base + 历史映射表）。
    Unit* najentus = AI_VALUE2(Unit*, "find target", "22887");
    if (!najentus)
        //End By leewheel
        return false;

    return GetNearestPlayerInRadius(bot, NAJENTUS_RANGED_SPREAD_DISTANCE);
}

bool HighWarlordNajentusImpaledPlayerNeedsRemoverTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, BT_MAP_ID))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "22887"))
        return false;

    return FindNajentusUnassignedImpaledPlayer(bot);
}

bool HighWarlordNajentusImpalingSpineOnGroupMemberTrigger::IsActiveInEncounter()
{
    return GetNajentusImpaledPlayerToFree(bot);
}

bool HighWarlordNajentusNeedsSpineThrowerTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, BT_MAP_ID))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "22887"))
        return false;

    return !GetNajentusSpineThrower(bot);
}

bool HighWarlordNajentusHasTidalShieldTrigger::IsActiveInEncounter()
{
    if (!IsNajentusSpineThrower(bot) || IsNajentusSpineInFlight(bot))
        return false;

    Unit* najentus = AI_VALUE2(Unit*, "find target", "22887");
    if (!najentus || !najentus->HasAura(Id(BtSpells::SPELL_TIDAL_SHIELD)))
        return false;

    return bot->HasItemCount(Id(BtItems::ITEM_NAJENTUS_SPINE));
}

// Supremus

bool SupremusRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* supremus = AI_VALUE2(Unit*, "find target", "22898");
    if (!supremus || IsSupremusKitePhase(supremus))
        return false;

    // The spread step can go any way and doesn't see fire, so it waits while one step could carry
    // the bot into a volcano's safe zone.
    float const fleeStep =
        std::min(SUPREMUS_RANGED_SPREAD_DISTANCE + 1.0f, sPlayerbotAIConfig.FleeDistance);
    if (IsInSupremusHazard(
            GetSupremusHazards(botAI), bot->GetPositionX(), bot->GetPositionY(),
            SupremusHazardZone::Safe, fleeStep))
    {
        return false;
    }

    return GetNearestPlayerInRadius(bot, SUPREMUS_RANGED_SPREAD_DISTANCE);
}

bool SupremusFixatedOnBotTrigger::IsActiveInEncounter()
{
    Unit* supremus = AI_VALUE2(Unit*, "find target", "22898");
    if (!supremus || supremus->GetVictim() != bot || !IsSupremusKitePhase(supremus))
        return false;

    // The kite multiplier holds every other mover, fire avoidance included.
    if (CanSupremusCatchStandingBot(bot, supremus))
        return true;

    return IsInSupremusHazard(
        GetSupremusHazards(botAI), bot->GetPositionX(), bot->GetPositionY(),
        SupremusHazardZone::Safe);
}

bool SupremusNearFireTrigger::IsActiveInEncounter()
{
    if (!AI_VALUE2(Unit*, "find target", "22898"))
        return false;

    if (bot->isMoving() && CanWalkThroughSupremusFire(bot))
        return false;

    return IsInSupremusHazard(
        GetSupremusHazards(botAI), bot->GetPositionX(), bot->GetPositionY(),
        SupremusHazardZone::Safe);
}

bool SupremusReachBlockedByFireTrigger::IsActiveInEncounter()
{
    if (!AI_VALUE2(Unit*, "find target", "22898"))
        return false;

    return GetSupremusFireBlockedReachTarget(botAI);
}

// Shade of Akama

bool ShadeOfAkamaShouldAttackChannelersAndSorcerersTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsDps(bot))
        return false;

    // Shadow Priests (by proxy via dps check) and melee dps focus on Channelers (and Sorcerers).
    if (bot->getClass() != CLASS_PRIEST && !PlayerbotAI::IsMelee(bot))
        return false;

    return GetShadeOfAkamaKillTarget(botAI);
}

// Teron Gorefiend

bool TeronGorefiendShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "22871");
}

bool TeronGorefiendRangedShouldPositionOnBalconyTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsRanged(bot) && AI_VALUE2(Unit*, "find target", "22871");
}

bool TeronGorefiendCastsShadowOfDeathTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER && bot->getClass() != CLASS_ROGUE)
        return false;

    Unit* gorefiend = AI_VALUE2(Unit*, "find target", "22871");
    if (!gorefiend)
        return false;

    // Once the bolt is launched, nothing cancels it.
    Spell* shadow = gorefiend->FindCurrentSpellBySpellId(Id(BtSpells::SPELL_SHADOW_OF_DEATH));
    return shadow && shadow->getState() == SPELL_STATE_PREPARING &&
        shadow->m_targets.GetUnitTarget() == bot;
}

bool TeronGorefiendShouldPositionForVengefulSpiritTrigger::IsActiveInEncounter()
{
    Aura* shadow = bot->GetAura(Id(BtSpells::SPELL_SHADOW_OF_DEATH));
    return shadow && shadow->GetDuration() <= GOREFIEND_SHADOW_OF_DEATH_MOVE_MS;
}

bool TeronGorefiendTransformedIntoVengefulSpiritTrigger::IsActiveInEncounter()
{
    return bot->HasAura(Id(BtSpells::SPELL_SPIRITUAL_VENGEANCE));
}

// Gurtogg Bloodboil

bool GurtoggBloodboilShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "22948");
    return gurtogg && !gurtogg->HasAura(Id(BtSpells::SPELL_BOSS_FEL_RAGE));
}

bool GurtoggBloodboilShouldPositionForBloodboilTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    // He doesn't cast Bloodboil during Fel Rage.
    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "22948");
    if (!gurtogg || gurtogg->HasAura(Id(BtSpells::SPELL_BOSS_FEL_RAGE)))
        return false;

    return !bot->HasAura(Id(BtSpells::SPELL_PLAYER_FEL_RAGE));
}

bool GurtoggBloodboilMeleeShouldStayBehindTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot))
        return false;

    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "22948");
    if (!gurtogg)
        return false;

    if (bot->HasAura(Id(BtSpells::SPELL_PLAYER_FEL_RAGE)))
        return false;

    Unit* felRageTarget = GetGurtoggFelRageTarget(gurtogg);
    if (!felRageTarget)
        return false;

    return bot->GetExactDist2d(GetGurtoggFelRageMeleePosition(bot, *gurtogg, *felRageTarget)) >
        GURTOGG_FEL_RAGE_MELEE_TOLERANCE;
}

// Melee go behind him instead.
bool GurtoggBloodboilRangedShouldAvoidFrontalConeTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsMelee(bot) || bot->HasAura(Id(BtSpells::SPELL_PLAYER_FEL_RAGE)))
        return false;

    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "22948");
    if (!gurtogg)
        return false;

    Unit* felRageTarget = GetGurtoggFelRageTarget(gurtogg);
    if (!felRageTarget || bot->GetExactDist2d(felRageTarget) >= GURTOGG_FEL_RAGE_AVOID_DISTANCE)
        return false;

    // Arcing Smash only hits in front of him, but he turns faster than bots can react, so at the
    // start of Fel Rage everyone near the target moves regardless of side.
    Aura* felRage = gurtogg->GetAura(Id(BtSpells::SPELL_BOSS_FEL_RAGE));
    if (felRage && GetAuraElapsedMs(felRage) < GURTOGG_FEL_RAGE_EARLY_AVOID_MS)
        return true;

    return gurtogg->HasInArc(static_cast<float>(M_PI), bot);
}

bool GurtoggBloodboilFelRageOnBotTrigger::IsActiveInEncounter()
{
    return bot->HasAura(Id(BtSpells::SPELL_PLAYER_FEL_RAGE)) &&
        bot->GetExactDist2d(GURTOGG_TANK_POSITION) > GURTOGG_POSITION_TOLERANCE;
}

// Reliquary of Souls

bool ReliquaryOfSoulsHunterShouldMisdirectTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* desire = AI_VALUE2(Unit*, "find target", "23419");
    if (desire && desire->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT)
        return true;

    Unit* anger = AI_VALUE2(Unit*, "find target", "23420");
    return anger && anger->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT;
}

bool ReliquaryOfSoulsShouldPositionForSufferingTrigger::IsActiveInEncounter()
{
    Unit* suffering = AI_VALUE2(Unit*, "find target", "23418");
    return suffering && !suffering->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
        IsOutOfSufferingPosition(bot, suffering);
}

bool ReliquaryOfSoulsHealersShouldAttackSufferingTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsHeal(bot))
        return false;

    // Discipline priests keep shielding.
    if (bot->getClass() == CLASS_PRIEST && botAI->HasStrategy("disc", BOT_STATE_COMBAT))
        return false;

    Unit* suffering = AI_VALUE2(Unit*, "find target", "23418");
    return suffering && !suffering->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE);
}

bool ReliquaryOfSoulsEssenceOfDesireHasRuneShieldTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_MAGE)
        return false;

    Unit* desire = AI_VALUE2(Unit*, "find target", "23419");
    return desire && desire->HasAura(Id(BtSpells::SPELL_RUNE_SHIELD));
}

bool ReliquaryOfSoulsEssenceOfDesireCastsDeadenTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_WARRIOR || !PlayerbotAI::IsTank(bot))
        return false;

    //By leewheel 2026-10-24 合并 brighton the-lab(91c5f466 / 5297c170 "Make timing, cast checks and
    //  spine checks consistent")：qualifier 保留本核 entry 23419（欲望精华）——上游仍是英文名
    //  "essence of desire"，本核实库生物名为中文 ⇒ 英文子串匹配恒 nullptr ⇒ 该触发器永不激活。
    //  同时保留本核的 UNIT_STATE_CASTING 前置判据（没在施法就直接排除，省掉后面的法术查表）；
    //  后续的 FindCurrentSpellBySpellId + SPELL_STATE_PREPARING + 目标判据为两边共有，不动。
    Unit* desire = AI_VALUE2(Unit*, "find target", "23419");
    if (!desire || !desire->HasUnitState(UNIT_STATE_CASTING))
        return false;

    Spell* deaden = desire->FindCurrentSpellBySpellId(Id(BtSpells::SPELL_DEADEN));
    return deaden && deaden->getState() == SPELL_STATE_PREPARING &&
        deaden->m_targets.GetUnitTarget() == bot;
}

// Mother Shahraz

bool MotherShahrazShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "22947"))
        return false;

    return !bot->HasAura(Id(BtSpells::SPELL_FATAL_ATTRACTION));
}

bool MotherShahrazBeingPositionedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot) || !PlayerbotAI::IsDps(bot))
        return false;

    // By leewheel 2026-10-22 合并：本核原写法（血线 90% + victim 距离判定）已被上游提取成
    // BtHelpers::IsShahrazBeingPositioned（判空 + SHAHRAZ_MELEE_ENGAGE_HEALTH_PCT + 同款距离判定），
    // 逻辑等价且可复用，故采纳上游调用；仅将英文名回退为 entry 22947（本核生物名为中文，
    // 英文 qualifier 恒匹配失败）。
    return IsShahrazBeingPositioned(AI_VALUE2(Unit*, "find target", "22947"));
}

bool MotherShahrazRangedShouldPositionUnderStatueTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "22947"))
        return false;

    return !bot->HasAura(Id(BtSpells::SPELL_FATAL_ATTRACTION));
}

bool MotherShahrazFatalAttractionTrigger::IsActiveInEncounter()
{
    return bot->HasAura(Id(BtSpells::SPELL_FATAL_ATTRACTION));
}

// Illidari Council

bool IllidariCouncilGathiosShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "22949"))
        return false;

    return PlayerbotAI::IsMainTank(bot);
}

bool IllidariCouncilGathiosCastsJudgementOfCommandTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_WARRIOR || !PlayerbotAI::IsTank(bot))
        return false;

    //By leewheel 2026-10-24 合并 brighton the-lab(91c5f466 / 5297c170)：qualifier 保留本核
    //  entry 22949（加西奥斯）——上游仍是英文名 "gathios the shatterer"，中文库下恒 nullptr。
    //  判定改为采纳上游的 FindCurrentSpellBySpellId(SPELL_JUDGEMENT) + SPELL_STATE_PREPARING
    //  + 目标确实是本 bot + IsMainTank：本核原用 GetCurrentSpell(CURRENT_GENERIC_SPELL) 只比对
    //  id，既不判施法阶段也不判目标 ⇒ 加西奥斯对**别人**施放审判时本触发器也会误激活。
    //  保留本核的 UNIT_STATE_CASTING 前置判据（快速排除）。
    Unit* gathios = AI_VALUE2(Unit*, "find target", "22949");
    if (!gathios || !gathios->HasUnitState(UNIT_STATE_CASTING) ||
        !gathios->HasAura(Id(BtSpells::SPELL_SEAL_OF_COMMAND)))
        return false;

    Spell* judgement = gathios->FindCurrentSpellBySpellId(Id(BtSpells::SPELL_JUDGEMENT));
    return judgement && judgement->getState() == SPELL_STATE_PREPARING &&
        judgement->m_targets.GetUnitTarget() == bot && PlayerbotAI::IsMainTank(bot);
}

bool IllidariCouncilMalandeCastsCircleOfHealingTrigger::IsActiveInEncounter()
{
    if (bot->getClass() == CLASS_DRUID || bot->getClass() == CLASS_HUNTER ||
        bot->getClass() == CLASS_PALADIN || bot->getClass() == CLASS_PRIEST)
    {
        return false;
    }

    if (!PlayerbotAI::IsDps(bot) || IsZerevorMageTank(botAI))
        return false;

    Unit* malande = AI_VALUE2(Unit*, "find target", "22951");
    if (!malande)
        return false;

    // By leewheel 2026-10-21 entry 铁律：上游此处用英文名 "gathios the shatterer" /
    // "high nethermancer zerevor"，本核 creature_template 生物名为中文，TargetValue.cpp 的
    // 英文 qualifier 子串匹配恒失败 ⇒ find target 恒返回 nullptr ⇒ 打断链静默失效。
    // 已回退 entry（实库三核定：22949 击碎者加西奥斯 / 22950 高阶灵术师塞勒沃尔）。
    // 上游「提取局部变量复用查询、避免重复取值」的结构本身正确，予以保留。
    Unit* gathios = AI_VALUE2(Unit*, "find target", "22949");
    Unit* zerevor = AI_VALUE2(Unit*, "find target", "22950");
    if (!gathios || !zerevor)
        return false;

    if (PlayerbotAI::IsRanged(bot) &&
        !ShouldRangedInterruptCircleOfHealing(malande, gathios, zerevor))
    {
        return false;
    }

    return CanInterruptCircleOfHealing(malande);
}

bool IllidariCouncilMalandeShouldBeTankedTrigger::IsActiveInEncounter()
{
    // By leewheel 2026-10-22 合并：采纳上游写法（多一层 PlayerbotAI::IsTank 前置，非坦直接短路），
    // 仅把英文名回退为 entry 22951（女公爵玛兰德）。
    return PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "22951") &&
        PlayerbotAI::IsAssistTankOfIndex(bot, 0, false);
}

bool IllidariCouncilVerasShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    // By leewheel 2026-10-22 合并：采纳上游写法——本核原版只判了 veras 存在且未消失，
    // 上游补上「须为二助坦」的归属判定（IsAssistTankOfIndex(bot, 1, false)），是逻辑补全，
    // 予以采纳；仅把英文名回退为 entry 22952（薇拉·暗影）。
    Unit* veras = AI_VALUE2(Unit*, "find target", "22952");
    if (!veras || IsVerasVanished(veras))
        return false;

    return PlayerbotAI::IsAssistTankOfIndex(bot, 1, false);
}

bool IllidariCouncilZerevorShouldBeTankedByMageTrigger::IsActiveInEncounter()
{
    return IsZerevorMageTank(botAI) && AI_VALUE2(Unit*, "find target", "22950");
}

bool IllidariCouncilMageTankHealerShouldRepositionTrigger::IsActiveInEncounter()
{
    // By leewheel 2026-10-22 合并：采纳上游写法（多一层 PlayerbotAI::IsHeal 前置，非治疗直接短路），
    // 仅把英文名回退为 entry 22950（高阶灵术师塞勒沃尔）。
    if (!PlayerbotAI::IsHeal(bot))
        return false;

    Unit* zerevor = AI_VALUE2(Unit*, "find target", "22950");
    if (!zerevor)
        return false;

    if (!PlayerbotAI::IsAssistHealOfIndex(bot, 0, true) || !IsZerevorOnMageTank(botAI, zerevor))
        return false;

    Player* mageTank = GetZerevorMageTank(botAI);
    return !IsMageTankHealerPositionSafe(bot->GetPosition(), mageTank, zerevor) ||
        !bot->IsWithinLOSInMap(mageTank);
}

bool IllidariCouncilRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "22950"))
        return false;

    if (IsZerevorMageTank(botAI) || PlayerbotAI::IsAssistHealOfIndex(bot, 0, true))
        return false;

    return !HasDangerousCouncilAura(bot) &&
        GetNearestPlayerInRadius(bot, COUNCIL_RANGED_SPREAD_DISTANCE);
}

bool IllidariCouncilPetShouldSwitchTargetTrigger::IsActiveInEncounter()
{
    Guardian* pet = bot->GetGuardianPet();
    if (!pet || !pet->IsAlive() || pet->HasReactState(REACT_PASSIVE))
        return false;

    Unit* target = GetCouncilPetTarget(botAI, pet);
    return target && pet->GetVictim() != target;
}

bool IllidariCouncilShouldAssignDpsPriorityTrigger::IsActiveInEncounter()
{
    if (!AI_VALUE2(Unit*, "find target", "22949"))
        return false;

    if ((PlayerbotAI::IsTank(bot) && PlayerbotAI::IsMainTank(bot)) ||
        PlayerbotAI::IsAssistTankOfIndex(bot, 0, false) ||
        IsZerevorMageTank(botAI))
    {
        return false;
    }

    if (!PlayerbotAI::IsAssistTankOfIndex(bot, 1, false))
        return true;

    Unit* veras = AI_VALUE2(Unit*, "find target", "22952");
    return !veras || IsVerasVanished(veras);
}

bool IllidariCouncilShouldManageDpsTimerTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, BT_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "22949");
}

// Illidan Stormrage <The Betrayer>

bool IllidanStormrageShouldPositionForPullTrigger::IsActive()
{
    // You are not prepared!
    return AI_VALUE(bool, "illidan stormrage speech in progress");
}

// Hunters misdirect onto the Main Tank at each Phase 1, 3 and 5 start, the Flame tanks in Phase 2
// and the warlock tank in Demon Form.
bool IllidanStormrageHunterShouldMisdirectTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "22917");
    if (!illidan)
        return false;

    IllidanPhase const phase = GetIllidanPhase(illidan);
    if (phase == IllidanPhase::Initial || phase == IllidanPhase::Grounded ||
        phase == IllidanPhase::Maiev)
    {
        return IsIllidanPhaseStartWithin(*illidan, phase, ILLIDAN_PHASE_START_MISDIRECT_MS);
    }

    return phase == IllidanPhase::Flying || phase == IllidanPhase::Demon;
}

bool IllidanStormrageShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "22917");
    if (!illidan)
        return false;

    IllidanPhase const phase = GetIllidanPhase(illidan);
    if (phase == IllidanPhase::Flying || phase == IllidanPhase::Dying)
        return false;

    // In Maiev's arrival, only once the tank is out of Shadow Prison.
    if (phase == IllidanPhase::None && bot->HasAura(Id(BtSpells::SPELL_SHADOW_PRISON)))
        return false;

    return PlayerbotAI::IsMainTank(bot);
}

// An infected bot runs to the infection position, and so does a trap hunter on duty, to lay its
// Frost Trap. The Main Tank never runs, and the warlock tank not in Demon Form.
bool IllidanStormrageParasiticShadowfiendOnGroupMemberTrigger::IsActiveInEncounter()
{
    if (!HasParasiticShadowfiend(bot) && !IsIllidanTrapHunterOnDuty(botAI))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "22917");
    if (!illidan || illidan->GetVictim() == bot)
        return false;

    IllidanPhase const phase = GetIllidanPhase(illidan);
    if (phase == IllidanPhase::None)
        return false;

    if (phase == IllidanPhase::Flying && !IsIllidanInfectionPositionKeptInFlight(botAI))
        return false;

    return !IsExemptFromIllidanInfectionRun(botAI, illidan, bot);
}

// The mechanic tracker records each new infection while there is an infection position, keeps
// looking for a trap hunter while none is ready, and drops an entry once its debuff is gone.
bool IllidanStormrageShouldTrackInfectionsTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, BT_MAP_ID))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "22917");
    if (!illidan)
        return false;

    auto const& infected = context->GetValue<GuidVector>(
        "illidan stormrage players with parasitic shadowfiend")->RefGet();
    IllidanPhase const phase = GetIllidanPhase(illidan);
    bool const hasInfectionPosition = phase != IllidanPhase::None && phase != IllidanPhase::Flying;
    auto const* infections = FindIllidanInfections(bot->GetInstanceId());
    if (infections)
    {
        for (auto const& [player, infection] : *infections)
        {
            if (std::find(infected.begin(), infected.end(), player) == infected.end())
                return true;

            if (!hasInfectionPosition)
                continue;

            if (!infection.isTrapAssigned)
                return true;

            if (infection.hunter.IsEmpty())
                continue;

            // Its bot has stopped running, so the hunter is freed.
            if (IsExemptFromIllidanInfectionRun(
                    botAI, illidan, ObjectAccessor::FindPlayer(player)))
            {
                return true;
            }
        }
    }

    if (!hasInfectionPosition)
        return false;

    for (ObjectGuid const& guid : infected)
    {
        if (!infections || !infections->contains(guid))
            return true;
    }

    return false;
}

bool IllidanStormrageFlamesOfAzzinothShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "22917");
    if (!illidan || GetIllidanPhase(illidan) != IllidanPhase::Flying)
        return false;

    return IsFlameOfAzzinothTank(bot) && !IsIllidanInfectionPositionKeptInFlight(botAI);
}

bool IllidanStormragePetsDieToHazardsTrigger::IsActiveInEncounter()
{
    Pet* pet = bot->GetPet();
    if (!pet || !pet->IsAlive())
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "22917");
    if (!illidan)
        return false;

    return ShouldIllidanPetsBePassive(botAI, illidan) != pet->HasReactState(REACT_PASSIVE);
}

// Hunter and warlock pets take the Shadowfiends, which never attack pets.
// Hunter and warlock pets in Phases 1, 2, 3 and 5 and the death scene. In Demon Form every pet,
// passive then, once no Shadow Demon is left for it.
bool IllidanStormragePetShouldAttackShadowfiendTrigger::IsActiveInEncounter()
{
    Pet* pet = bot->GetPet();
    if (!pet || !pet->IsAlive())
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "22917");
    if (!illidan)
        return false;

    // A pet's hit puts its owner on the Shadowfiend's threat list, the only entry it can act on,
    // so while its owner has just birthed Shadowfiends the pet is called off them.
    Unit* victim = pet->GetVictim();
    uint32 const victimEntry = victim && victim->IsAlive() ? victim->GetEntry() : 0;
    bool const isNearNewShadowfiend = IsNearNewParasiticShadowfiend(botAI);

    if (victimEntry == Id(BtNpcs::NPC_PARASITIC_SHADOWFIEND))
        return isNearNewShadowfiend;

    if (victimEntry == Id(BtNpcs::NPC_SHADOW_DEMON) || isNearNewShadowfiend)
        return false;

    IllidanPhase const phase = GetIllidanPhase(illidan);
    if (phase == IllidanPhase::Demon)
    {
        if (GetShadowDemonForPetToAttack(botAI, *pet))
            return false;
    }
    else if (phase == IllidanPhase::Initial || phase == IllidanPhase::Flying ||
        phase == IllidanPhase::Grounded || phase == IllidanPhase::Maiev ||
        phase == IllidanPhase::Dying)
    {
        // While he flies the pet stays passive, so it takes no Flame, and only the command sends
        // it onto Shadowfiends left from Phase 1.
        if ((bot->getClass() != CLASS_HUNTER && bot->getClass() != CLASS_WARLOCK) ||
            (phase != IllidanPhase::Flying && pet->HasReactState(REACT_PASSIVE)))
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    return GetNearestParasiticShadowfiend(botAI, *pet);
}

// Every pet, passive in Demon Form, is sent onto a Shadow Demon and kept on it until it dies.
bool IllidanStormragePetShouldAttackShadowDemonTrigger::IsActiveInEncounter()
{
    Pet* pet = bot->GetPet();
    if (!pet || !pet->IsAlive())
        return false;

    Unit* victim = pet->GetVictim();
    if (victim && victim->IsAlive() && victim->GetEntry() == Id(BtNpcs::NPC_SHADOW_DEMON))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "22917");
    if (!illidan || GetIllidanPhase(illidan) != IllidanPhase::Demon)
        return false;

    return GetShadowDemonForPetToAttack(botAI, *pet);
}

bool IllidanStormrageShouldPositionAboveGrateTrigger::IsActiveInEncounter()
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "22917");
    if (!illidan || GetIllidanPhase(illidan) != IllidanPhase::Flying)
        return false;

    if (IsIllidanInfectionPositionKeptInFlight(botAI))
        return false;

    if (!PlayerbotAI::IsTank(bot))
        return true;

    return !IsFlameOfAzzinothTank(bot);
}

// Ice Block, Divine Shield and Cloak of Shadows each clear both debuffs on use, Dark Barrage
// after a moment. Not for the Main Tank or a Flame tank: an immunity loses it whatever it holds.
bool IllidanStormrageImmunityCanClearDebuffTrigger::IsActiveInEncounter()
{
    bool const isPaladin = bot->getClass() == CLASS_PALADIN;
    if (!isPaladin && bot->getClass() != CLASS_MAGE && bot->getClass() != CLASS_ROGUE)
        return false;

    Aura const* darkBarrage = bot->GetAura(Id(BtSpells::SPELL_DARK_BARRAGE));
    bool const isDarkBarrageDue =
        darkBarrage && GetAuraElapsedMs(darkBarrage) >= ILLIDAN_DARK_BARRAGE_REACTION_MS;
    if (!isDarkBarrageDue && !bot->HasAura(Id(BtSpells::SPELL_AGONIZING_FLAMES)))
        return false;

    if (isPaladin)
    {
        return !(PlayerbotAI::IsTank(bot) && PlayerbotAI::IsMainTank(bot)) &&
            !IsFlameOfAzzinothTank(bot);
    }

    return true;
}

// Below half health, the immunity may be holding off something else, so it is kept.
bool IllidanStormrageImmunityNoLongerNeededTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_MAGE &&
        (bot->getClass() != CLASS_PALADIN || PlayerbotAI::IsHeal(bot)))
    {
        return false;
    }

    uint32 const spellId = GetSelfImmunitySpell(bot);
    if (!spellId || !bot->HasAura(spellId))
        return false;

    constexpr float keepImmunityHealthPct = 50.0f;
    if (bot->GetHealthPct() <= keepImmunityHealthPct)
        return false;

    return AI_VALUE2(Unit*, "find target", "22917");
}

// Totems set for the last phase would take a Flame in its first 2 s, or be back on his threat
// list before the tank, so they come up at each phase change.
bool IllidanStormrageShamanHasTotemsBeforeTanksHaveAggroTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_SHAMAN)
        return false;

    bool hasTotem = false;
    for (uint8 slot = SUMMON_SLOT_TOTEM_FIRE; slot < MAX_TOTEM_SLOT; ++slot)
        hasTotem = hasTotem || !bot->m_SummonSlot[slot].IsEmpty();

    if (!hasTotem)
        return false;

    //By leewheel 2026-10-25 entry 铁律：上游新写的这一处用英文名 "illidan stormrage"，实库生物名
    //  为中文 ⇒ 子串匹配恒 nullptr，图腾压手触发器静默失效（不崩不报错）。回退为 entry 22917。
    Unit* illidan = AI_VALUE2(Unit*, "find target", "22917");
    //End By leewheel
    return illidan && IsIllidanTankAggroPending(*illidan, GetIllidanPhase(illidan));
}

// An infected bot runs to the infection position instead.
bool IllidanStormrageShouldPositionForLandingTrigger::IsActiveInEncounter()
{
    // By leewheel 2026-10-22 entry 铁律：英文名回退为 entry 22917（伊利丹·怒风）
    Unit* illidan = AI_VALUE2(Unit*, "find target", "22917");
    if (!illidan || GetIllidanPhase(illidan) != IllidanPhase::Landing)
        return false;

    // By leewheel 2026-10-22 合并：采纳上游条件「主坦不参与落地站位」。本核冲突侧在此重复声明了
    // 一次 illidan（与上一行同作用域重定义，本身即编译错），实为上一轮 entry 化时的残留，丢弃。
    if (PlayerbotAI::IsTank(bot) && PlayerbotAI::IsMainTank(bot))
        return false;

    return !HasParasiticShadowfiend(bot);
}

// At the transform, and at 33% or below in Demon Form, melee and tanks still on him end their
// auto-attack: in his melee range they would only need 110% of the warlock tank's threat.
bool IllidanStormrageMeleeShouldStopAttackingTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "22917");
    if (!illidan || (bot->GetVictim() != illidan && AI_VALUE(Unit*, "current target") != illidan))
        return false;

    IllidanDpsHold const window = GetIllidanHoldWindow(illidan);
    return window == IllidanDpsHold::DemonFormStart || window == IllidanDpsHold::DemonFormNoPush;
}

// Auto Shot, wands and channels (Drain Soul, Mind Flay) keep hitting him through a hold, since a
// multiplier can't end a running auto-repeat or channel; in the pickup, so do any
// channel's heals or mana.
bool IllidanStormrageShouldStopShootingTrigger::IsActiveInEncounter()
{
    Spell* shot = bot->GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL);
    Spell* channel = bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
    if (!shot && !channel)
        return false;

    // By leewheel 2026-10-22 合并：采纳上游写法，并把英文名回退为 entry 22917。
    // ★ 上游写法顺带修掉了本核版的一个空指针：函数开头只判 `if (!shot && !channel) return false;`
    //   ⇒ 允许「shot 为空而 channel 非空」。本核版直接 `shot->m_targets...` 解引用，
    //   此时（如正在引导 Drain Soul / Mind Flay 而非自动射击）会空指针崩溃。
    //   上游改为 `(shot && ...) || (channel && ...)`，两者皆可为空，安全。
    Unit* illidan = AI_VALUE2(Unit*, "find target", "22917");
    if (!illidan)
        return false;

    if (channel && IsIllidanPickupChannelHeld(botAI, illidan, *channel))
        return true;

    ObjectGuid const guid = illidan->GetGUID();
    bool const isOnIllidan = (shot && shot->m_targets.GetUnitTargetGUID() == guid) ||
        (channel && channel->m_targets.GetUnitTargetGUID() == guid);
    return isOnIllidan && GetIllidanDpsHold(botAI, illidan) != IllidanDpsHold::None;
}

bool IllidanStormrageRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "22917");
    if (!illidan)
        return false;

    IllidanPhase const phase = GetIllidanPhase(illidan);

    if (phase == IllidanPhase::Demon)
    {
        if (IsIllidanWarlockTank(botAI))
            return false;

        // While a Demon walks to the warlock tank, ranged DPS may leave their slots to reach it.
        if (!PlayerbotAI::IsHeal(bot) && GetShadowDemonMovingToWarlockTank(botAI))
            return false;

        return true;
    }

    return phase == IllidanPhase::Initial || phase == IllidanPhase::Grounded ||
        phase == IllidanPhase::Maiev;
}

// Ranged DPS walk into spell reach of a Shadow Demon walking to the warlock tank, whose death
// wipes the raid. Healers stay to heal.
bool IllidanStormrageRangedShouldReachShadowDemonTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRangedDps(bot))
        return false;

    // By leewheel 2026-10-22 合并：采纳上游写法（把「已寄生腐蚀」与「是术士坦」合并为第二段判定，
    // 语义与本核版一致且更清晰），并把英文名回退为 entry 22917。
    Unit* illidan = AI_VALUE2(Unit*, "find target", "22917");
    if (!illidan || GetIllidanPhase(illidan) != IllidanPhase::Demon)
        return false;

    if (HasParasiticShadowfiend(bot) || IsIllidanWarlockTank(botAI))
        return false;

    Unit* demon = GetShadowDemonMovingToWarlockTank(botAI);
    return demon && !bot->IsWithinCombatRange(demon, botAI->GetRange("spell"));
}

// Melee and tanks take their Demon Form slots; the Main Tank has its own positions.
bool IllidanStormrageMeleeShouldStayAwayFromDemonFormTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot))
        return false;

    // By leewheel 2026-10-22 合并：采纳上游写法——上游多出 HasParasiticShadowfiend(bot) 条件，
    // 即「身上带寄生腐蚀的近战先去跑位、不参与恶魔形态散位」，与本函数注释意图一致，是逻辑补全；
    // 同时把英文名回退为 entry 22917。
    Unit* illidan = AI_VALUE2(Unit*, "find target", "22917");
    if (!illidan || GetIllidanPhase(illidan) != IllidanPhase::Demon || HasParasiticShadowfiend(bot))
        return false;

    if (PlayerbotAI::IsTank(bot) && PlayerbotAI::IsMainTank(bot))
        return false;

    return !IsTargetingShadowDemon(botAI);
}

bool IllidanStormrageWarlockShouldTankDemonFormTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "22917");
    if (!illidan || GetIllidanPhase(illidan) != IllidanPhase::Demon)
        return false;

    return IsIllidanWarlockTank(botAI);
}

// Every warlock, the warlock tank included, casts Seed of Corruption on the Shadow Demons while
// two or more are alive. They all walk toward the raid, so a Seed going off on one usually
// catches others. Not while infected, so the run comes first, nor next to fresh Shadowfiends,
// which the explosion would turn onto the warlock.
bool IllidanStormrageWarlockShouldSeedShadowDemonsTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    // By leewheel 2026-10-22 合并：采纳上游写法——上游多出 HasParasiticShadowfiend(bot) 条件，
    // 与本函数注释「Not while infected, so the run comes first」一致，是逻辑补全；
    // 同时把英文名回退为 entry 22917。
    Unit* illidan = AI_VALUE2(Unit*, "find target", "22917");
    if (!illidan || GetIllidanPhase(illidan) != IllidanPhase::Demon || HasParasiticShadowfiend(bot))
        return false;

    if (IsIllidanPhaseStartWithin(*illidan, IllidanPhase::Demon, ILLIDAN_SHADOW_DEMON_SPAWN_MS))
        return false;

    auto const& shadowDemons =
        context->GetValue<std::vector<ShadowDemon>>("illidan stormrage shadow demons")->RefGet();
    if (shadowDemons.size() < 2)
        return false;

    return !IsNearNewParasiticShadowfiend(botAI);
}

// Tanks other than the Main Tank join the melee in Phases 1, 3, 4 and 5; while he flies the
// assist tanks have the Flames. The Main Tank joins them on Shadow Demons in Demon Form until its
// walk-in. Healers take Shadow Demons as targets in Demon Form.
bool IllidanStormrageShouldAssignTargetPriorityTrigger::IsActiveInEncounter()
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "22917");
    IllidanPhase const phase = GetIllidanPhase(illidan);
    if (phase == IllidanPhase::None || phase == IllidanPhase::Dying)
        return false;

    if (PlayerbotAI::IsHeal(bot))
        return phase == IllidanPhase::Demon;

    if (!PlayerbotAI::IsTank(bot))
        return true;

    if (phase != IllidanPhase::Initial && phase != IllidanPhase::Grounded &&
        phase != IllidanPhase::Demon && phase != IllidanPhase::Maiev)
    {
        return false;
    }

    if (!PlayerbotAI::IsMainTank(bot))
        return true;

    return phase == IllidanPhase::Demon &&
        !IsIllidanDemonFormEnding(*illidan, ILLIDAN_MAIN_TANK_WALK_IN_MS);
}

// By leewheel 2026-10-22 与上游 brighton 合并：IllidanStormrageMaievPlacedCageTrapTrigger 由上游
// illidan pass 2 整类删除（类声明 BTTriggers.h、TriggerContext 注册、依赖的
// BtHelpers::FindNearestCageTrap 与配对的 IllidanStormrageUseCageTrapAction 一并移除）。
// 自动合并已删掉声明/注册/辅助函数，此处为实现残留，保留会因「无声明的定义」与未解析的
// FindNearestCageTrap 而编译失败，故按上游重构一并移除。本核对该段原本唯一的改动只是把
// illidan 英文名回退为 entry 22917，无本核独有逻辑，删除不损失本核功能。
bool IllidanStormrageShouldManageDpsTimersTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, BT_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "22917");
}

// Destroying hazards after phase 2 is not gated behind CheatMask. The strategy simply cannot work
// without doing this.
bool IllidanStormrageShouldClearHazardsBeforeLandingTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, BT_MAP_ID))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "22917");
    return illidan && GetIllidanPhase(illidan) == IllidanPhase::Landing;
}
