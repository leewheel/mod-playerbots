/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "EnemyPlayerValue.h"
#include "CombatManager.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "Vehicle.h"

// By leewheel 2026-09-01
// PVP 目标评分（自研超越点：NPCBots 完全没有职业/护甲/状态优先级，只有距离序）。
// 真人 PVP 集火铁律：旗手 > 正在读条的治疗 > 脆皮 > 残血，距离做衰减项。
// 全部 entry 已经 chs_dbc.db_spell_12340_eng 验证：
//   23333/23335=WS 战歌/银翼旗手, 14267/14268=AV 部落/联盟旗帜, 29062=EY 风暴之眼旗手。
// End By leewheel
namespace
{
bool IsFlagCarrier(Unit* unit)
{
    static constexpr uint32 const FLAG_AURAS[] = { 23333, 23335, 14267, 14268, 29062 };
    for (uint32 auraId : FLAG_AURAS)
    {
        if (unit->HasAura(auraId))
            return true;
    }

    return false;
}

bool IsCastingHeal(Unit* unit)
{
    Spell const* casting = unit->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (!casting)
        casting = unit->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
    if (!casting)
        return false;

    SpellInfo const* info = casting->GetSpellInfo();
    if (!info)
        return false;

    for (uint8 i = EFFECT_0; i <= EFFECT_2; ++i)
    {
        switch (info->Effects[i].Effect)
        {
            case SPELL_EFFECT_HEAL:
            case SPELL_EFFECT_HEAL_MAX_HEALTH:
            case SPELL_EFFECT_HEAL_MECHANICAL:
                return true;
            default:
                break;
        }
    }

    return false;
}

float ScorePvpTarget(PlayerbotAI* botAI, Unit* target)
{
    float score = 0.f;

    if (IsFlagCarrier(target))
        score += 1000.f;  // 击杀旗手掉旗，战场第一优先级

    if (IsCastingHeal(target))
        score += 500.f;   // 压治疗：读条中的治疗职业必须优先处理

    if (Player* p = target->ToPlayer())
    {
        switch (p->getClass())
        {
            case CLASS_PRIEST:
            case CLASS_MAGE:
            case CLASS_WARLOCK:
                score += 100.f;  // 布甲脆皮
                break;
            case CLASS_ROGUE:
            case CLASS_DRUID:
                score += 50.f;   // 皮甲
                break;
            default:
                break;           // 锁甲/板甲不加分
        }
    }

    float hpPct = target->GetHealthPct();
    if (hpPct < 25.f)
        score += 300.f;  // 残血补刀
    else if (hpPct < 50.f)
        score += 150.f;

    score -= botAI->GetBot()->GetDistance(target) * 2.f;  // 同分打近的

    return score;
}
}  // namespace

bool NearestEnemyPlayersValue::AcceptUnit(Unit* unit)
{
    // Apply parent's filtering first (includes level difference checks)
    if (!PossibleTargetsValue::AcceptUnit(unit))
        return false;

    bool inCannon = botAI->IsInVehicle(false, true);
    Player* enemy = dynamic_cast<Player*>(unit);
    if (enemy && botAI->IsOpposing(enemy) && enemy->IsPvP() &&
        !sPlayerbotAIConfig.IsPvpProhibited(enemy->GetZoneId(), enemy->GetAreaId()) &&
        !enemy->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NON_ATTACKABLE_2) &&
        ((inCannon || !enemy->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE))) &&
        /*!enemy->HasStealthAura() && !enemy->HasInvisibilityAura()*/ enemy->CanSeeOrDetect(bot) &&
        !(enemy->HasSpiritOfRedemptionAura()))
    {
        // If with master, only attack if master is PvP flagged
        Player* master = botAI->GetMaster();
        if (master && !master->IsPvP() && !master->IsFFAPvP())
            return false;

        return true;
    }

    return false;
}

Unit* EnemyPlayerValue::Calculate()
{
    bool controllingCannon = false;
    bool controllingVehicle = false;
    if (Vehicle* vehicle = bot->GetVehicle())
    {
        VehicleSeatEntry const* seat = vehicle->GetSeatForPassenger(bot);
        if (!seat || !seat->CanControl())  // not in control of vehicle so cant attack anyone
            return nullptr;
        VehicleEntry const* vi = vehicle->GetVehicleInfo();
        if (vi && vi->m_flags & VEHICLE_FLAG_FIXED_POSITION)
            controllingCannon = true;
        else
            controllingVehicle = true;
    }

    // 1. Check units we are currently in PvP combat with.
    std::vector<Unit*> targets;
    Unit* pVictim = bot->GetVictim();
    for (auto const& [guid, combatRef] : bot->GetCombatManager().GetPvPCombatRefs())
    {
        Unit* pTarget = combatRef->GetOther(bot);
        if (!pTarget || pTarget == pVictim || !pTarget->IsPlayer() || !pTarget->CanSeeOrDetect(bot) ||
            !bot->IsWithinDist(pTarget, VISIBILITY_DISTANCE_NORMAL))
            continue;

        if ((bot->GetTeamId() == TEAM_HORDE && pTarget->HasAura(23333)) ||
            (bot->GetTeamId() == TEAM_ALLIANCE && pTarget->HasAura(23335)))
            return pTarget;

        targets.push_back(pTarget);
    }

    if (!targets.empty())
    {
        // By leewheel 2026-09-01 交战目标改按 PVP 评分择优（原纯距离序——真人会转火治疗/脆皮/旗手）
        std::sort(targets.begin(), targets.end(),
                  [&](Unit const* pUnit1, Unit const* pUnit2)
                  { return ScorePvpTarget(botAI, const_cast<Unit*>(pUnit1)) > ScorePvpTarget(botAI, const_cast<Unit*>(pUnit2)); });

        return *targets.begin();
        // End By leewheel
    }

    // 2. Find enemy player in range.

    GuidVector players = AI_VALUE(GuidVector, "nearest enemy players");
    float const maxAggroDistance = GetMaxAttackDistance();

    // By leewheel 2026-09-01
    // 候选收集制（原为遍历到第一个可打就返回）：全部可打候选按 PVP 评分择优，
    // 治疗/旗手/脆皮/残血优先，距离仅做衰减项。夺回己方旗帜的敌人（持我方旗光环）
    // 仍保持最高优先直接返回（原 23333/23335 逻辑并入旗手评分体系）。
    // End By leewheel
    Unit* bestTarget = nullptr;
    float bestScore = -100000.f;
    for (auto const& gTarget : players)
    {
        Unit* pUnit = botAI->GetUnit(gTarget);
        if (!pUnit)
            continue;

        Player* pTarget = dynamic_cast<Player*>(pUnit);
        if (!pTarget)
            continue;

        if (pTarget == pVictim)
            continue;

        if (bot->GetTeamId() == TEAM_HORDE)
        {
            if (pTarget->HasAura(23333))
                return pTarget;
        }
        else
        {
            if (pTarget->HasAura(23335))
                return pTarget;
        }

        // Aggro weak enemies from further away.
        // If controlling mobile vehicle only agro close enemies (otherwise will never reach objective)
        uint32 const aggroDistance = controllingVehicle                                               ? 5.0f
                                     : (controllingCannon || bot->GetHealth() > pTarget->GetHealth()) ? maxAggroDistance
                                                                                                      : 20.0f;
        if (!bot->IsWithinDist(pTarget, aggroDistance))
            continue;

        if (!bot->IsWithinLOSInMap(pTarget) ||
            (!controllingCannon && fabs(bot->GetPositionZ() - pTarget->GetPositionZ()) >= 30.0f))
            continue;

        float score = ScorePvpTarget(botAI, pTarget);
        if (score > bestScore)
        {
            bestScore = score;
            bestTarget = pTarget;
        }
    }

    if (bestTarget)
        return bestTarget;

    // 3. Check party attackers.

    if (Group* pGroup = bot->GetGroup())
    {
        for (GroupReference* itr = pGroup->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            if (Unit* pMember = itr->GetSource())
            {
                if (pMember == bot)
                    continue;

                if (ServerFacade::instance().GetDistance2d(bot, pMember) > 30.0f)
                    continue;

                if (Unit* pAttacker = pMember->getAttackerForHelper())
                    if (pAttacker->IsPlayer() && bot->IsWithinDist(pAttacker, maxAggroDistance * 2.0f) &&
                        bot->IsWithinLOSInMap(pAttacker) && pAttacker != pVictim && pAttacker->CanSeeOrDetect(bot))
                        return pAttacker;
            }
        }
    }

    return nullptr;
}

float EnemyPlayerValue::GetMaxAttackDistance()
{
    if (!bot->GetBattleground())
        return 60.0f;

    Battleground* bg = bot->GetBattleground();
    if (!bg)
        return 40.0f;

    BattlegroundTypeId bgType = bg->GetBgTypeID();
    if (bgType == BATTLEGROUND_RB)
        bgType = bg->GetBgTypeID(true);

    if (bgType == BATTLEGROUND_IC)
    {
        if (botAI->IsInVehicle(false, true))
            return 120.0f;
    }

    return 40.0f;
}
