/*
 * 版权所有 (C) 2026 Leewheel
 * 
 * 文件功能：纳克萨玛斯团队副本触发器实现
 * 实现纳克萨玛斯副本各Boss的触发器逻辑
 * 
 * By Leewheel 2026-02-14
 */

#include "RaidNaxxTriggers.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

// ==========================================
// 帕奇维克触发器实现
// ==========================================

// 验证：需求 2.1
bool NaxxPatchwerkCombatTrigger::IsActive()
{
    // 查找帕奇维克Boss
    Unit* boss = AI_VALUE2(Unit*, "find target", "patchwerk");
    if (!boss || boss->GetEntry() != NPC_PATCHWERK)
        return false;
    
    // 检查Boss是否在战斗中且存活
    return boss->IsInCombat() && boss->IsAlive();
}

// 验证：需求 2.3
bool NaxxPatchwerkFrenzyTrigger::IsActive()
{
    // 查找帕奇维克Boss
    Unit* boss = AI_VALUE2(Unit*, "find target", "patchwerk");
    if (!boss || boss->GetEntry() != NPC_PATCHWERK)
        return false;
    
    // 检查Boss是否存活且在战斗中
    if (!boss->IsAlive() || !boss->IsInCombat())
        return false;
    
    // 检查狂乱光环（在≤5%生命值时施放）
    return boss->HasAura(SPELL_FRENZY);
}

// 验证：需求 2.5
bool NaxxPatchwerkBerserkTrigger::IsActive()
{
    // 查找帕奇维克Boss
    Unit* boss = AI_VALUE2(Unit*, "find target", "patchwerk");
    if (!boss || boss->GetEntry() != NPC_PATCHWERK)
        return false;
    
    // 检查Boss是否存活且在战斗中
    if (!boss->IsAlive() || !boss->IsInCombat())
        return false;
    
    // 检查狂暴光环（6分钟后施放）
    return boss->HasAura(SPELL_BERSERK);
}

// 验证：需求 2.2
bool NaxxPatchwerkOffTankPositionTrigger::IsActive()
{
    // 只有副坦克需要定位
    if (!botAI->IsTank(bot) || botAI->IsMainTank(bot))
        return false;
    
    // 查找帕奇维克Boss
    Unit* boss = AI_VALUE2(Unit*, "find target", "patchwerk");
    if (!boss || boss->GetEntry() != NPC_PATCHWERK)
        return false;
    
    // 检查Boss是否存活且在战斗中
    if (!boss->IsAlive() || !boss->IsInCombat())
        return false;
    
    // 检查机器人是否不在近战范围内或不在正确位置（Boss侧面）
    // 副坦克应该在近战范围内（5码）在Boss侧面
    if (!bot->IsWithinMeleeRange(boss))
        return true;
    
    // 检查机器人是否在侧面（不在前面或后面）
    // 这是一个简化的检查 - 实际上，动作将处理精确定位
    float angle = boss->GetAngle(bot);
    float bossOrientation = boss->GetOrientation();
    float relativeAngle = fabs(angle - bossOrientation);
    
    // 将角度标准化到0-2π
    while (relativeAngle > M_PI * 2)
        relativeAngle -= M_PI * 2;
    while (relativeAngle < 0)
        relativeAngle += M_PI * 2;
    
    // 检查机器人是否在侧面（大约90°或270°，±30°容差）
    bool atSide = (relativeAngle > M_PI_4 && relativeAngle < 3 * M_PI_4) ||  // 右侧
                  (relativeAngle > 5 * M_PI_4 && relativeAngle < 7 * M_PI_4); // 左侧
    
    return !atSide;
}

// ==========================================
// 格罗布鲁斯触发器实现
// ==========================================

// 验证：需求 6.2
bool NaxxGrobbulusMutatingInjectionTrigger::IsActive()
{
    // 查找格罗布鲁斯Boss
    Unit* boss = AI_VALUE2(Unit*, "find target", "grobbulus");
    if (!boss || boss->GetEntry() != NPC_GROBBULUS)
        return false;
    
    // 检查Boss是否存活且在战斗中
    if (!boss->IsAlive() || !boss->IsInCombat())
        return false;
    
    // 检查机器人是否有变异注射减益效果
    return bot->HasAura(SPELL_MUTATING_INJECTION);
}

// 验证：需求 6.4
bool NaxxGrobbulusPoisonCloudTrigger::IsActive()
{
    // 查找格罗布鲁斯Boss
    Unit* boss = AI_VALUE2(Unit*, "find target", "grobbulus");
    if (!boss || boss->GetEntry() != NPC_GROBBULUS)
        return false;
    
    // 检查Boss是否存活且在战斗中
    if (!boss->IsAlive() || !boss->IsInCombat())
        return false;
    
    // 搜索附近的毒云NPC或检查地面上的毒云光环
    // 毒云通常作为生物或游戏对象生成
    // 我们需要检查是否有任何毒云效果在危险范围内
    
    // 获取所有附近的单位
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (auto& guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;
        
        // 检查这是否是毒云（它可能有毒云法术效果）
        // 或者它是施放毒云的触发NPC
        if (unit->HasAura(SPELL_POISON_CLOUD))
        {
            // 检查到毒云的距离
            float distance = bot->GetExactDist2d(unit);
            if (distance < 10.0f)  // 在毒云10码范围内
                return true;
        }
    }
    
    // 同时检查机器人是否站在毒云中（有减益效果）
    if (bot->HasAura(SPELL_POISON_CLOUD))
        return true;
    
    return false;
}

// ==========================================
// 阿努布雷坎触发器实现
// ==========================================

// 验证：需求 8.2
bool NaxxAnubRekhanLocustSwarmTrigger::IsActive()
{
    // 查找阿努布雷坎Boss
    Unit* boss = AI_VALUE2(Unit*, "find target", "anub'rekhan");
    if (!boss || boss->GetEntry() != NPC_ANUBREKHAN)
        return false;
    
    // 检查Boss是否存活且在战斗中
    if (!boss->IsAlive() || !boss->IsInCombat())
        return false;
    
    // 检查Boss是否施放蝗虫群技能
    // 蝗虫群是一个持续性光环，Boss会在战斗中定期施放
    return boss->HasAura(SPELL_LOCUST_SWARM);
}

// ==========================================
// 费尔莉娜触发器实现
// ==========================================

// 验证：需求 9.3
bool NaxxFaerlinaFrenzyTrigger::IsActive()
{
    // 查找费尔莉娜Boss
    Unit* boss = AI_VALUE2(Unit*, "find target", "grand widow faerlina");
    if (!boss || boss->GetEntry() != NPC_FAERLINA)
        return false;
    
    // 检查Boss是否存活且在战斗中
    if (!boss->IsAlive() || !boss->IsInCombat())
        return false;
    
    // 检查Boss是否有狂乱光环
    return boss->HasAura(SPELL_FAERLINA_FRENZY);
}

// ==========================================
// 迈克斯纳触发器实现
// ==========================================

// 验证：需求 10.2
bool NaxxMaexxnaWebSprayTrigger::IsActive()
{
    // 查找迈克斯纳Boss
    Unit* boss = AI_VALUE2(Unit*, "find target", "maexxna");
    if (!boss || boss->GetEntry() != NPC_MAEXXNA)
        return false;
    
    // 检查Boss是否存活且在战斗中
    if (!boss->IsAlive() || !boss->IsInCombat())
        return false;
    
    // 检查Boss是否正在施放蛛网喷射
    return boss->HasAura(SPELL_WEB_SPRAY) || bot->HasAura(SPELL_WEB_SPRAY);
}

// 验证：需求 10.3
bool NaxxMaexxnaWebWrapTrigger::IsActive()
{
    // 查找迈克斯纳Boss
    Unit* boss = AI_VALUE2(Unit*, "find target", "maexxna");
    if (!boss || boss->GetEntry() != NPC_MAEXXNA)
        return false;
    
    // 检查Boss是否存活且在战斗中
    if (!boss->IsAlive() || !boss->IsInCombat())
        return false;
    
    // 检查机器人是否被蛛网缠绕
    return bot->HasAura(SPELL_WEB_WRAP);
}

// 验证：需求 10.4
bool NaxxMaexxnaPoisonShockTrigger::IsActive()
{
    // 查找迈克斯纳Boss
    Unit* boss = AI_VALUE2(Unit*, "find target", "maexxna");
    if (!boss || boss->GetEntry() != NPC_MAEXXNA)
        return false;
    
    // 检查Boss是否存活且在战斗中
    if (!boss->IsAlive() || !boss->IsInCombat())
        return false;
    
    // 检查Boss是否正在施放毒性冲击
    return boss->HasUnitState(UNIT_STATE_CASTING);
}

// ==========================================
// 诺斯触发器实现
// ==========================================

// 验证：需求 11.2
bool NaxxNothTeleportTrigger::IsActive()
{
    // 查找诺斯Boss
    Unit* boss = AI_VALUE2(Unit*, "find target", "noth the plaguebringer");
    if (!boss || boss->GetEntry() != NPC_NOTH)
        return false;
    
    // 检查Boss是否在战斗中
    if (!boss->IsInCombat())
        return false;
    
    // 检查Boss是否不可见（传送到阳台）
    return !boss->IsVisible() || boss->HasAura(SPELL_NOTH_TELEPORT);
}

// 验证：需求 11.3
bool NaxxNothReappearTrigger::IsActive()
{
    // 查找诺斯Boss
    Unit* boss = AI_VALUE2(Unit*, "find target", "noth the plaguebringer");
    if (!boss || boss->GetEntry() != NPC_NOTH)
        return false;
    
    // 检查Boss是否在战斗中
    if (!boss->IsInCombat())
        return false;
    
    // 检查Boss是否可见（从阳台返回）
    return boss->IsVisible() && !boss->HasAura(SPELL_NOTH_TELEPORT);
}

// ==========================================
// 洛欧塞布触发器实现
// ==========================================

// 验证：需求 12.2
bool NaxxLoathebNecroticAuraActiveTrigger::IsActive()
{
    // 查找洛欧塞布Boss
    Unit* boss = AI_VALUE2(Unit*, "find target", "loatheb");
    if (!boss || boss->GetEntry() != NPC_LOATHEB)
        return false;
    
    // 检查Boss是否存活且在战斗中
    if (!boss->IsAlive() || !boss->IsInCombat())
        return false;
    
    // 检查机器人是否有死灵光环减益效果（阻止治疗）
    return bot->HasAura(SPELL_NECROTIC_AURA);
}

// 验证：需求 12.3
bool NaxxLoathebNecroticAuraInactiveTrigger::IsActive()
{
    // 查找洛欧塞布Boss
    Unit* boss = AI_VALUE2(Unit*, "find target", "loatheb");
    if (!boss || boss->GetEntry() != NPC_LOATHEB)
        return false;
    
    // 检查Boss是否存活且在战斗中
    if (!boss->IsAlive() || !boss->IsInCombat())
        return false;
    
    // 检查机器人是否没有死灵光环（治疗窗口）
    return !bot->HasAura(SPELL_NECROTIC_AURA);
}

// ==========================================
// 格拉斯触发器实现
// ==========================================

// 验证：需求 13.2
bool NaxxGluthDecimateTrigger::IsActive()
{
    // 查找格拉斯Boss
    Unit* boss = AI_VALUE2(Unit*, "find target", "gluth");
    if (!boss || boss->GetEntry() != NPC_GLUTH)
        return false;
    
    // 检查Boss是否存活且在战斗中
    if (!boss->IsAlive() || !boss->IsInCombat())
        return false;
    
    // 检查Boss是否施放毁灭技能
    return boss->HasAura(SPELL_DECIMATE);
}

// 验证：需求 13.3
bool NaxxGluthZombieChowNearbyTrigger::IsActive()
{
    // 查找格拉斯Boss
    Unit* boss = AI_VALUE2(Unit*, "find target", "gluth");
    if (!boss || boss->GetEntry() != NPC_GLUTH)
        return false;
    
    // 检查Boss是否存活且在战斗中
    if (!boss->IsAlive() || !boss->IsInCombat())
        return false;
    
    // 搜索附近的僵尸食尸鬼
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (auto& guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;
        
        // 检查是否是僵尸食尸鬼
        if (unit->GetEntry() == NPC_ZOMBIE_CHOW && unit->IsAlive())
        {
            // 检查距离
            float distance = bot->GetExactDist2d(unit);
            if (distance < 30.0f)
                return true;
        }
    }
    
    return false;
}

// ==========================================
// 海根触发器实现
// ==========================================

// 验证：需求 3.1, 3.2
bool NaxxHeiganDanceTrigger::IsActive()
{
    // 查找海根Boss
    Unit* boss = AI_VALUE2(Unit*, "find target", "heigan the unclean");
    if (!boss || boss->GetEntry() != NPC_HEIGAN)
        return false;
    
    // 检查Boss是否存活且在战斗中
    if (!boss->IsAlive() || !boss->IsInCombat())
        return false;
    
    // 检查是否有瘟疫云（地板喷发）
    return bot->HasAura(SPELL_PLAGUE_CLOUD) || boss->HasAura(SPELL_SPELL_DISRUPTION);
}

// ==========================================
// 四骑士触发器实现
// ==========================================

// 验证：需求 4.2, 4.3
bool NaxxFourHorsemenMarkHighTrigger::IsActive()
{
    // 检查四个骑士的标记层数
    uint32 zeliekStacks = 0;
    uint32 blaumeuxStacks = 0;
    uint32 rivendareStacks = 0;
    uint32 korthazzStacks = 0;
    
    // 获取机器人身上的标记层数
    if (Aura* aura = bot->GetAura(SPELL_MARK_OF_ZELIEK))
        zeliekStacks = aura->GetStackAmount();
    if (Aura* aura = bot->GetAura(SPELL_MARK_OF_BLAUMEUX))
        blaumeuxStacks = aura->GetStackAmount();
    if (Aura* aura = bot->GetAura(SPELL_MARK_OF_RIVENDARE))
        rivendareStacks = aura->GetStackAmount();
    if (Aura* aura = bot->GetAura(SPELL_MARK_OF_KORTHAZZ))
        korthazzStacks = aura->GetStackAmount();
    
    // 如果任何标记层数 >= 4，需要切换目标
    uint32 maxStacks = std::max({zeliekStacks, blaumeuxStacks, rivendareStacks, korthazzStacks});
    return maxStacks >= 4;
}

// ==========================================
// 塔迪乌斯触发器实现
// ==========================================

// 验证：需求 5.1, 5.2
bool NaxxThaddiusPolarityShiftTrigger::IsActive()
{
    // 查找塔迪乌斯Boss
    Unit* boss = AI_VALUE2(Unit*, "find target", "thaddius");
    if (!boss || boss->GetEntry() != NPC_THADDIUS)
        return false;
    
    // 检查Boss是否存活且在战斗中
    if (!boss->IsAlive() || !boss->IsInCombat())
        return false;
    
    // 检查机器人是否有极性充能（正极或负极）
    return bot->HasAura(SPELL_POSITIVE_CHARGE) || bot->HasAura(SPELL_NEGATIVE_CHARGE);
}

// ==========================================
// 戈提克触发器实现
// ==========================================

// 验证：需求 14.2
bool NaxxGothikPhaseOneTrigger::IsActive()
{
    // 查找戈提克Boss
    Unit* boss = AI_VALUE2(Unit*, "find target", "gothik the harvester");
    if (!boss || boss->GetEntry() != NPC_GOTHIK)
        return false;
    
    // 检查Boss是否在战斗中
    if (!boss->IsInCombat())
        return false;
    
    // 第一阶段：Boss不可攻击（在阳台上）
    return !boss->IsVisible() || !boss->CanHaveThreatList();
}

// 验证：需求 14.3
bool NaxxGothikPhaseTwoTrigger::IsActive()
{
    // 查找戈提克Boss
    Unit* boss = AI_VALUE2(Unit*, "find target", "gothik the harvester");
    if (!boss || boss->GetEntry() != NPC_GOTHIK)
        return false;
    
    // 检查Boss是否在战斗中
    if (!boss->IsInCombat())
        return false;
    
    // 第二阶段：Boss可攻击（下到地面）
    return boss->IsVisible() && boss->CanHaveThreatList();
}

// ==========================================
// 拉祖维奥斯触发器实现
// ==========================================

// 验证：需求 15.2
bool NaxxRazuviousNeedControlTrigger::IsActive()
{
    // 查找拉祖维奥斯Boss
    Unit* boss = AI_VALUE2(Unit*, "find target", "instructor razuvious");
    if (!boss || boss->GetEntry() != NPC_RAZUVIOUS)
        return false;
    
    // 检查Boss是否存活且在战斗中
    if (!boss->IsAlive() || !boss->IsInCombat())
        return false;
    
    // 只有牧师或术士可以控制
    if (bot->getClass() != CLASS_PRIEST && bot->getClass() != CLASS_WARLOCK)
        return false;
    
    // 检查是否有死亡骑士学徒未被控制
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (auto& guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;
        
        // 检查是否是死亡骑士学徒
        if (unit->GetEntry() == NPC_DEATHKNIGHT_UNDERSTUDY && unit->IsAlive())
        {
            // 检查是否未被控制
            if (!unit->HasAuraType(SPELL_AURA_MOD_CHARM))
                return true;
        }
    }
    
    return false;
}

// 验证：需求 15.5
bool NaxxRazuviousControlExpiringTrigger::IsActive()
{
    // 查找拉祖维奥斯Boss
    Unit* boss = AI_VALUE2(Unit*, "find target", "instructor razuvious");
    if (!boss || boss->GetEntry() != NPC_RAZUVIOUS)
        return false;
    
    // 检查Boss是否存活且在战斗中
    if (!boss->IsAlive() || !boss->IsInCombat())
        return false;
    
    // 只有牧师或术士可以控制
    if (bot->getClass() != CLASS_PRIEST && bot->getClass() != CLASS_WARLOCK)
        return false;
    
    // 检查当前控制的学徒的控制时间是否即将到期
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (auto& guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;
        
        // 检查是否是被控制的死亡骑士学徒
        if (unit->GetEntry() == NPC_DEATHKNIGHT_UNDERSTUDY && unit->IsAlive())
        {
            if (unit->HasAuraType(SPELL_AURA_MOD_CHARM))
            {
                // 检查控制剩余时间
                Unit::AuraApplicationMap const& auras = unit->GetAppliedAuras();
                for (auto& pair : auras)
                {
                    Aura* aura = pair.second->GetBase();
                    if (aura && aura->GetSpellInfo()->HasAura(SPELL_AURA_MOD_CHARM))
                    {
                        int32 duration = aura->GetDuration();
                        if (duration > 0 && duration < 5000)  // 少于5秒
                            return true;
                    }
                }
            }
        }
    }
    
    return false;
}

// ==========================================
// 萨菲隆触发器实现
// ==========================================

// 验证：需求 7.2
bool NaxxSapphironAirPhaseTrigger::IsActive()
{
    // 查找萨菲隆Boss
    Unit* boss = AI_VALUE2(Unit*, "find target", "sapphiron");
    if (!boss || boss->GetEntry() != NPC_SAPPHIRON)
        return false;
    
    // 检查Boss是否存活且在战斗中
    if (!boss->IsAlive() || !boss->IsInCombat())
        return false;
    
    // 检查Boss是否在飞行（空中阶段）
    return boss->HasUnitMovementFlag(MOVEMENTFLAG_FLYING) || !boss->CanHaveThreatList();
}

// ==========================================
// 克尔苏加德触发器实现
// ==========================================

// 验证：需求 16.2
bool NaxxKelThuzadPhaseOneTrigger::IsActive()
{
    // 查找克尔苏加德Boss
    Unit* boss = AI_VALUE2(Unit*, "find target", "kel'thuzad");
    if (!boss || boss->GetEntry() != NPC_KELTHUZAD)
        return false;
    
    // 检查Boss是否在战斗中
    if (!boss->IsInCombat())
        return false;
    
    // 第一阶段：Boss不可攻击，只有小怪
    return !boss->CanHaveThreatList();
}

// 验证：需求 16.3
bool NaxxKelThuzadPhaseTwoTrigger::IsActive()
{
    // 查找克尔苏加德Boss
    Unit* boss = AI_VALUE2(Unit*, "find target", "kel'thuzad");
    if (!boss || boss->GetEntry() != NPC_KELTHUZAD)
        return false;
    
    // 检查Boss是否存活且在战斗中
    if (!boss->IsAlive() || !boss->IsInCombat())
        return false;
    
    // 第二阶段：Boss可攻击，生命值 > 40%
    return boss->CanHaveThreatList() && boss->GetHealthPct() > 40.0f;
}

// 验证：需求 16.4
bool NaxxKelThuzadPhaseThreeTrigger::IsActive()
{
    // 查找克尔苏加德Boss
    Unit* boss = AI_VALUE2(Unit*, "find target", "kel'thuzad");
    if (!boss || boss->GetEntry() != NPC_KELTHUZAD)
        return false;
    
    // 检查Boss是否存活且在战斗中
    if (!boss->IsAlive() || !boss->IsInCombat())
        return false;
    
    // 第三阶段：Boss生命值 <= 40%，守护者出现
    return boss->GetHealthPct() <= 40.0f;
}

// 验证：需求 16.5
bool NaxxKelThuzadFrostBlastTrigger::IsActive()
{
    // 查找克尔苏加德Boss
    Unit* boss = AI_VALUE2(Unit*, "find target", "kel'thuzad");
    if (!boss || boss->GetEntry() != NPC_KELTHUZAD)
        return false;
    
    // 检查Boss是否存活且在战斗中
    if (!boss->IsAlive() || !boss->IsInCombat())
        return false;
    
    // 检查机器人是否被冰霜冲击冻结
    return bot->HasAura(SPELL_FROST_BLAST);
}

// 验证：需求 16.6
bool NaxxKelThuzadShadowFissureTrigger::IsActive()
{
    // 查找克尔苏加德Boss
    Unit* boss = AI_VALUE2(Unit*, "find target", "kel'thuzad");
    if (!boss || boss->GetEntry() != NPC_KELTHUZAD)
        return false;
    
    // 检查Boss是否存活且在战斗中
    if (!boss->IsAlive() || !boss->IsInCombat())
        return false;
    
    // 检查附近是否有暗影裂隙（通过检测地面效果或视觉效果）
    // 这是一个简化的检查 - 实际上可能需要检查游戏对象或特定的地面效果
    return bot->HasAura(SPELL_SHADOW_FISSURE);
}

// By Leewheel 2026-02-14
