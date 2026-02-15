/*
 * 版权所有 (C) 2026 Leewheel
 * 
 * 文件功能：纳克萨玛斯团队副本动作实现
 * 实现纳克萨玛斯副本各Boss战斗的动作逻辑
 * 
 * By Leewheel 2026-02-14
 */

#include "RaidNaxxActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "../Trigger/RaidNaxxTriggers.h"

// ==========================================
// 帕奇维克动作实现
// ==========================================

//  副坦克定位在Boss侧面的近战范围内
bool NaxxPatchwerkOffTankPositionAction::Execute(Event event)
{
    // 查找帕奇维克
    Unit* boss = AI_VALUE2(Unit*, "find target", "patchwerk");
    if (!boss || boss->GetEntry() != NPC_PATCHWERK)
        return false;
    
    // 仅对副坦克执行
    if (!PlayerbotAI::IsTank(bot) || PlayerbotAI::IsMainTank(bot))
        return false;
    
    // 计算Boss侧面的位置（从Boss朝向偏移90度）
    float angle = boss->GetOrientation() + M_PI_2;  // 向右偏移90度
    float distance = 5.0f;  // 近战范围
    float x = boss->GetPositionX() + distance * cos(angle);
    float y = boss->GetPositionY() + distance * sin(angle);
    float z = boss->GetPositionZ();
    
    // 移动到计算出的位置
    return MoveTo(boss->GetMapId(), x, y, z,
                 false, false, false, false,
                 MovementPriority::MOVEMENT_COMBAT);
}

// 需求 2.4, 2.6: 在狂乱或狂暴期间触发燃烧阶段
bool NaxxPatchwerkBurnPhaseAction::Execute(Event event)
{
    // 查找帕奇维克
    Unit* boss = AI_VALUE2(Unit*, "find target", "patchwerk");
    if (!boss || boss->GetEntry() != NPC_PATCHWERK)
        return false;
    
    // 验证狂乱或狂暴是否激活
    if (!boss->HasAura(SPELL_FRENZY) && !boss->HasAura(SPELL_BERSERK))
        return false;
    
    // 根据职业使用所有可用的DPS冷却技能
    bool executed = false;
    
    // 通用冷却技能
    if (botAI->CastSpell("bloodlust", bot))
        executed = true;
    if (botAI->CastSpell("heroism", bot))
        executed = true;
    if (botAI->CastSpell("berserking", bot))
        executed = true;
    
    // 职业特定冷却技能
    switch (bot->getClass())
    {
        case CLASS_WARRIOR:
            if (botAI->CastSpell("recklessness", bot))
                executed = true;
            if (botAI->CastSpell("death wish", bot))
                executed = true;
            break;
        case CLASS_ROGUE:
            if (botAI->CastSpell("adrenaline rush", bot))
                executed = true;
            if (botAI->CastSpell("blade flurry", bot))
                executed = true;
            break;
        case CLASS_HUNTER:
            if (botAI->CastSpell("rapid fire", bot))
                executed = true;
            if (botAI->CastSpell("bestial wrath", bot))
                executed = true;
            break;
        case CLASS_MAGE:
            if (botAI->CastSpell("arcane power", bot))
                executed = true;
            if (botAI->CastSpell("icy veins", bot))
                executed = true;
            break;
        case CLASS_WARLOCK:
            if (botAI->CastSpell("metamorphosis", bot))
                executed = true;
            break;
        case CLASS_DRUID:
            if (botAI->CastSpell("berserk", bot))
                executed = true;
            break;
        case CLASS_PALADIN:
            if (botAI->CastSpell("avenging wrath", bot))
                executed = true;
            break;
        case CLASS_SHAMAN:
            if (botAI->CastSpell("elemental mastery", bot))
                executed = true;
            break;
        case CLASS_PRIEST:
            if (botAI->CastSpell("power infusion", bot))
                executed = true;
            break;
        case CLASS_DEATH_KNIGHT:
            if (botAI->CastSpell("summon gargoyle", bot))
                executed = true;
            if (botAI->CastSpell("empower rune weapon", bot))
                executed = true;
            break;
    }
    
    // 继续攻击Boss
    return Attack(boss) || executed;
}

// ==========================================
// 格罗布鲁斯动作实现
// ==========================================

// 需求 6.3: 当变异注射即将爆炸时移动到边缘
bool NaxxGrobbulusMoveToEdgeAction::Execute(Event event)
{
    // 查找格罗布鲁斯
    Unit* boss = AI_VALUE2(Unit*, "find target", "grobbulus");
    if (!boss || boss->GetEntry() != NPC_GROBBULUS)
        return false;
    
    // 验证机器人有变异注射减益效果
    if (!bot->HasAura(SPELL_MUTATING_INJECTION))
        return false;
    
    // 获取Boss位置（房间中心）
    float bossX = boss->GetPositionX();
    float bossY = boss->GetPositionY();
    float bossZ = boss->GetPositionZ();
    
    // 计算机器人相对于Boss的当前位置
    float botX = bot->GetPositionX();
    float botY = bot->GetPositionY();
    
    // 计算从Boss到机器人的方向向量
    float dx = botX - bossX;
    float dy = botY - bossY;
    float distance = sqrt(dx * dx + dy * dy);
    
    // 标准化方向向量
    if (distance < 0.1f)
    {
        // 机器人离Boss太近，选择一个随机方向
        float angle = frand(0, M_PI * 2);
        dx = cos(angle);
        dy = sin(angle);
    }
    else
    {
        dx /= distance;
        dy /= distance;
    }
    
    // 向远离Boss 30码的方向移动（朝向边缘）
    float edgeDistance = 30.0f;
    float targetX = bossX + dx * edgeDistance;
    float targetY = bossY + dy * edgeDistance;
    float targetZ = bossZ;
    
    // 移动到边缘位置
    return MoveTo(boss->GetMapId(), targetX, targetY, targetZ,
                 false, false, false, false,
                 MovementPriority::MOVEMENT_COMBAT);
}

// 需求 6.5: 远离毒云区域
bool NaxxGrobbulusAvoidPoisonCloudAction::Execute(Event event)
{
    // 查找格罗布鲁斯
    Unit* boss = AI_VALUE2(Unit*, "find target", "grobbulus");
    if (!boss || boss->GetEntry() != NPC_GROBBULUS)
        return false;
    
    // 查找所有毒云位置
    struct CloudPosition {
        float x, y, z;
    };
    std::vector<CloudPosition> poisonClouds;
    
    // 获取所有附近的单位以查找毒云
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (auto& guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;
        
        // 检查此单位是否有毒云光环（表示它是毒云）
        if (unit->HasAura(SPELL_POISON_CLOUD))
        {
            CloudPosition pos;
            pos.x = unit->GetPositionX();
            pos.y = unit->GetPositionY();
            pos.z = unit->GetPositionZ();
            poisonClouds.push_back(pos);
        }
    }
    
    // 如果没有找到毒云，无需躲避
    if (poisonClouds.empty())
        return false;
    
    // 计算远离所有毒云的安全位置
    float botX = bot->GetPositionX();
    float botY = bot->GetPositionY();
    float botZ = bot->GetPositionZ();
    
    // 计算远离最近毒云的方向
    float nearestDistance = 1000.0f;
    float nearestCloudX = 0.0f;
    float nearestCloudY = 0.0f;
    
    for (const auto& cloud : poisonClouds)
    {
        float dx = botX - cloud.x;
        float dy = botY - cloud.y;
        float dist = sqrt(dx * dx + dy * dy);
        
        if (dist < nearestDistance)
        {
            nearestDistance = dist;
            nearestCloudX = cloud.x;
            nearestCloudY = cloud.y;
        }
    }
    
    // 计算远离最近毒云的方向
    float dx = botX - nearestCloudX;
    float dy = botY - nearestCloudY;
    float distance = sqrt(dx * dx + dy * dy);
    
    // 标准化方向
    if (distance < 0.1f)
    {
        // 机器人在毒云上方，选择一个随机方向
        float angle = frand(0, M_PI * 2);
        dx = cos(angle);
        dy = sin(angle);
    }
    else
    {
        dx /= distance;
        dy /= distance;
    }
    
    // 向远离最近毒云 15码的方向移动
    float safeDistance = 15.0f;
    float targetX = botX + dx * safeDistance;
    float targetY = botY + dy * safeDistance;
    float targetZ = botZ;
    
    // 验证目标位置不靠近其他毒云
    bool isSafe = true;
    for (const auto& cloud : poisonClouds)
    {
        float dx2 = targetX - cloud.x;
        float dy2 = targetY - cloud.y;
        float dist = sqrt(dx2 * dx2 + dy2 * dy2);
        
        if (dist < 10.0f)  // 离另一个毒云太近
        {
            isSafe = false;
            break;
        }
    }
    
    // 如果目标位置不安全，尝试向Boss方向移动
    if (!isSafe && boss)
    {
        targetX = boss->GetPositionX();
        targetY = boss->GetPositionY();
        targetZ = boss->GetPositionZ();
    }
    
    // 移动到安全位置
    return MoveTo(boss->GetMapId(), targetX, targetY, targetZ,
                 false, false, false, false,
                 MovementPriority::MOVEMENT_COMBAT);
}

// ==========================================
// 阿努布雷坎动作实现
// ==========================================

// 需求 8.3, 8.4: 在蝗虫群期间分散站位
bool NaxxAnubRekhanSpreadOutAction::Execute(Event event)
{
    // 查找阿努布雷坎
    Unit* boss = AI_VALUE2(Unit*, "find target", "anub'rekhan");
    if (!boss || boss->GetEntry() != NPC_ANUBREKHAN)
        return false;
    
    // 验证Boss正在施放蝗虫群
    if (!boss->HasAura(SPELL_LOCUST_SWARM))
        return false;
    
    // 获取所有团队成员的位置
    Group* group = bot->GetGroup();
    if (!group)
        return false;
    
    // 计算远离所有团队成员的位置
    float botX = bot->GetPositionX();
    float botY = bot->GetPositionY();
    float botZ = bot->GetPositionZ();
    
    // 收集所有团队成员的位置
    std::vector<Unit*> memberPositions;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive())
            continue;
        
        // 检查成员是否在附近
        if (bot->GetExactDist2d(member) < 50.0f)
        {
            memberPositions.push_back(member);
        }
    }
    
    // 如果没有附近的成员，保持当前位置
    if (memberPositions.empty())
        return false;
    
    // 计算远离所有成员的方向
    float totalDx = 0.0f;
    float totalDy = 0.0f;
    
    for (const auto& member : memberPositions)
    {
        float dx = botX - member->GetPositionX();
        float dy = botY - member->GetPositionY();
        float dist = sqrt(dx * dx + dy * dy);
        
        // 如果太近，增加权重
        if (dist < 10.0f && dist > 0.1f)
        {
            // 标准化并加权（距离越近，权重越大）
            float weight = (10.0f - dist) / dist;
            totalDx += dx * weight;
            totalDy += dy * weight;
        }
    }
    
    // 如果已经足够分散，不需要移动
    float magnitude = sqrt(totalDx * totalDx + totalDy * totalDy);
    if (magnitude < 0.1f)
        return false;
    
    // 标准化方向
    totalDx /= magnitude;
    totalDy /= magnitude;
    
    // 向远离人群的方向移动 12码（确保至少 10码距离）
    float spreadDistance = 12.0f;
    float targetX = botX + totalDx * spreadDistance;
    float targetY = botY + totalDy * spreadDistance;
    float targetZ = botZ;
    
    // 移动到分散位置
    return MoveTo(boss->GetMapId(), targetX, targetY, targetZ,
                 false, false, false, false,
                 MovementPriority::MOVEMENT_COMBAT);
}

// 需求 8.6: 优先攻击地穴守卫小怪
bool NaxxAnubRekhanAttackCryptGuardAction::Execute(Event event)
{
    // 查找阿努布雷坎
    Unit* boss = AI_VALUE2(Unit*, "find target", "anub'rekhan");
    if (!boss || boss->GetEntry() != NPC_ANUBREKHAN)
        return false;
    
    // 验证Boss在战斗中
    if (!boss->IsAlive() || !boss->IsInCombat())
        return false;
    
    // 搜索地穴守卫小怪
    Unit* cryptGuard = nullptr;
    float nearestDistance = 100.0f;
    
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (auto& guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;
        
        // 检查这是否是地穴守卫
        if (unit->GetEntry() == NPC_CRYPT_GUARD && unit->IsAlive())
        {
            float distance = bot->GetExactDist2d(unit);
            if (distance < nearestDistance)
            {
                nearestDistance = distance;
                cryptGuard = unit;
            }
        }
    }
    
    // 如果找到地穴守卫，攻击它
    if (cryptGuard)
    {
        return Attack(cryptGuard);
    }
    
    // 如果没有地穴守卫，攻击Boss
    return Attack(boss);
}

// By Leewheel 2026-02-14

// ==========================================
// 费尔莉娜动作实现
// ==========================================

// 需求 9.4: 优先攻击崇拜者以移除狂乱
bool NaxxFaerlinaAttackWorshipperAction::Execute(Event event)
{
    // 查找费尔莉娜
    Unit* boss = AI_VALUE2(Unit*, "find target", "faerlina");
    if (!boss || boss->GetEntry() != NPC_FAERLINA)
        return false;
    
    // 验证Boss在战斗中且有狂乱
    if (!boss->IsAlive() || !boss->IsInCombat() || !boss->HasAura(SPELL_FAERLINA_FRENZY))
        return false;
    
    // 搜索纳克萨玛斯崇拜者
    Unit* worshipper = nullptr;
    float nearestDistance = 100.0f;
    
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (auto& guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;
        
        // 检查是否是崇拜者
        if (unit->GetEntry() == NPC_NAXXRAMAS_WORSHIPPER && unit->IsAlive())
        {
            float distance = bot->GetExactDist2d(unit);
            if (distance < nearestDistance)
            {
                nearestDistance = distance;
                worshipper = unit;
            }
        }
    }
    
    // 如果找到崇拜者，攻击它
    if (worshipper)
    {
        return Attack(worshipper);
    }
    
    // 如果没有崇拜者，攻击Boss
    return Attack(boss);
}

// ==========================================
// 迈克斯纳动作实现
// ==========================================

// 需求 10.3: 被蛛网缠绕时等待救援
bool NaxxMaexxnaWaitForRescueAction::Execute(Event event)
{
    // 验证机器人被蛛网缠绕
    if (!bot->HasAura(SPELL_WEB_WRAP))
        return false;
    
    // 停止所有动作，等待救援
    // 机器人无法移动或施法，只能等待
    return true;
}

// 需求 10.4: 毒性冲击时停止施法
bool NaxxMaexxnaStopCastingAction::Execute(Event event)
{
    // 查找迈克斯纳
    Unit* boss = AI_VALUE2(Unit*, "find target", "maexxna");
    if (!boss || boss->GetEntry() != NPC_MAEXXNA)
        return false;
    
    // 验证Boss正在施放毒性冲击
    if (!boss->HasUnitState(UNIT_STATE_CASTING))
        return false;
    
    // 如果机器人正在施法，中断施法
    if (bot->HasUnitState(UNIT_STATE_CASTING))
    {
        bot->InterruptNonMeleeSpells(false);
        return true;
    }
    
    return false;
}

// 需求 10.6: 攻击小蜘蛛
bool NaxxMaexxnaAttackSpiderlingAction::Execute(Event event)
{
    // 查找迈克斯纳
    Unit* boss = AI_VALUE2(Unit*, "find target", "maexxna");
    if (!boss || boss->GetEntry() != NPC_MAEXXNA)
        return false;
    
    // 搜索小蜘蛛
    Unit* spiderling = nullptr;
    float nearestDistance = 100.0f;
    
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (auto& guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;
        
        // 检查是否是小蜘蛛
        if (unit->GetEntry() == NPC_SPIDERLING && unit->IsAlive())
        {
            float distance = bot->GetExactDist2d(unit);
            if (distance < nearestDistance)
            {
                nearestDistance = distance;
                spiderling = unit;
            }
        }
    }
    
    // 如果找到小蜘蛛，攻击它
    if (spiderling)
    {
        return Attack(spiderling);
    }
    
    // 如果没有小蜘蛛，攻击Boss
    return Attack(boss);
}

// ==========================================
// 诺斯动作实现
// ==========================================

// 需求 11.3: 诺斯传送时切换到小怪
bool NaxxNothSwitchToAddsAction::Execute(Event event)
{
    // 查找诺斯
    Unit* boss = AI_VALUE2(Unit*, "find target", "noth");
    if (!boss || boss->GetEntry() != NPC_NOTH)
        return false;
    
    // 验证Boss不可见（在阳台上）
    if (boss->IsVisible())
        return false;
    
    // 搜索瘟疫战士或瘟疫勇士
    Unit* add = nullptr;
    float nearestDistance = 100.0f;
    
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (auto& guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;
        
        // 检查是否是瘟疫战士或瘟疫勇士
        if ((unit->GetEntry() == NPC_PLAGUED_WARRIOR || unit->GetEntry() == NPC_PLAGUED_CHAMPION) && unit->IsAlive())
        {
            float distance = bot->GetExactDist2d(unit);
            if (distance < nearestDistance)
            {
                nearestDistance = distance;
                add = unit;
            }
        }
    }
    
    // 如果找到小怪，攻击它
    if (add)
    {
        return Attack(add);
    }
    
    return false;
}

// 需求 11.4: 诺斯重新出现时切换回Boss
bool NaxxNothSwitchToBossAction::Execute(Event event)
{
    // 查找诺斯
    Unit* boss = AI_VALUE2(Unit*, "find target", "noth");
    if (!boss || boss->GetEntry() != NPC_NOTH)
        return false;
    
    // 验证Boss可见（从阳台返回）
    if (!boss->IsVisible())
        return false;
    
    // 攻击Boss
    return Attack(boss);
}

// ==========================================
// 洛欧塞布动作实现
// ==========================================

// 需求 12.2: 死灵光环激活时停止治疗
bool NaxxLoathebStopHealingAction::Execute(Event event)
{
    // 验证机器人是治疗者
    if (!botAI->IsHeal(bot))
        return false;
    
    // 验证有死灵光环
    if (!bot->HasAura(SPELL_NECROTIC_AURA))
        return false;
    
    // 停止治疗动作（通过不执行治疗法术）
    // 这个动作主要是一个标记，告诉AI不要尝试治疗
    return true;
}

// 需求 12.3, 12.5: 死灵光环消失时爆发治疗
bool NaxxLoathebBurstHealingAction::Execute(Event event)
{
    // 验证机器人是治疗者
    if (!botAI->IsHeal(bot))
        return false;
    
    // 验证没有死灵光环（治疗窗口）
    if (bot->HasAura(SPELL_NECROTIC_AURA))
        return false;
    
    // 查找洛欧塞布
    Unit* boss = AI_VALUE2(Unit*, "find target", "loatheb");
    if (!boss || boss->GetEntry() != NPC_LOATHEB)
        return false;
    
    // 验证Boss在战斗中
    if (!boss->IsAlive() || !boss->IsInCombat())
        return false;
    
    // 使用所有可用的治疗冷却技能
    bool executed = false;
    
    // 根据职业使用治疗冷却技能
    switch (bot->getClass())
    {
        case CLASS_PRIEST:
            if (botAI->CastSpell("guardian spirit", bot))
                executed = true;
            if (botAI->CastSpell("power infusion", bot))
                executed = true;
            break;
        case CLASS_PALADIN:
            if (botAI->CastSpell("avenging wrath", bot))
                executed = true;
            if (botAI->CastSpell("divine favor", bot))
                executed = true;
            break;
        case CLASS_SHAMAN:
            if (botAI->CastSpell("nature's swiftness", bot))
                executed = true;
            break;
        case CLASS_DRUID:
            if (botAI->CastSpell("nature's swiftness", bot))
                executed = true;
            if (botAI->CastSpell("swiftmend", bot))
                executed = true;
            break;
    }
    
    // 施放强力治疗法术
    Group* group = bot->GetGroup();
    if (group)
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive())
                continue;
            
            // 治疗生命值低的成员
            if (member->GetHealthPct() < 80.0f)
            {
                if (botAI->CastSpell("flash heal", member))
                    executed = true;
                if (botAI->CastSpell("holy light", member))
                    executed = true;
                if (botAI->CastSpell("healing wave", member))
                    executed = true;
                if (botAI->CastSpell("healing touch", member))
                    executed = true;
                
                if (executed)
                    break;
            }
        }
    }
    
    return executed;
}

// ==========================================
// 格拉斯动作实现
// ==========================================

// 需求 13.3: 风筝僵尸食尸鬼远离Boss
bool NaxxGluthKiteZombiesAction::Execute(Event event)
{
    // 只有副坦克执行风筝
    if (!botAI->IsTank(bot) || botAI->IsMainTank(bot))
        return false;
    
    // 查找格拉斯
    Unit* boss = AI_VALUE2(Unit*, "find target", "gluth");
    if (!boss || boss->GetEntry() != NPC_GLUTH)
        return false;
    
    // 验证Boss在战斗中
    if (!boss->IsAlive() || !boss->IsInCombat())
        return false;
    
    // 获取Boss位置
    float bossX = boss->GetPositionX();
    float bossY = boss->GetPositionY();
    float bossZ = boss->GetPositionZ();
    
    // 计算远离Boss的位置（25码）
    float botX = bot->GetPositionX();
    float botY = bot->GetPositionY();
    
    float dx = botX - bossX;
    float dy = botY - bossY;
    float distance = sqrt(dx * dx + dy * dy);
    
    // 如果已经足够远，保持位置
    if (distance >= 25.0f)
        return false;
    
    // 标准化方向
    if (distance < 0.1f)
    {
        float angle = frand(0, M_PI * 2);
        dx = cos(angle);
        dy = sin(angle);
    }
    else
    {
        dx /= distance;
        dy /= distance;
    }
    
    // 向远离Boss 30码的方向移动
    float targetDistance = 30.0f;
    float targetX = bossX + dx * targetDistance;
    float targetY = bossY + dy * targetDistance;
    float targetZ = bossZ;
    
    // 移动到风筝位置
    return MoveTo(boss->GetMapId(), targetX, targetY, targetZ,
                 false, false, false, false,
                 MovementPriority::MOVEMENT_COMBAT);
}

// 需求 13.6: 击杀僵尸食尸鬼
bool NaxxGluthKillZombiesAction::Execute(Event event)
{
    // 查找格拉斯
    Unit* boss = AI_VALUE2(Unit*, "find target", "gluth");
    if (!boss || boss->GetEntry() != NPC_GLUTH)
        return false;
    
    // 搜索僵尸食尸鬼
    Unit* zombie = nullptr;
    float nearestDistance = 100.0f;
    
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (auto& guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;
        
        // 检查是否是僵尸食尸鬼
        if (unit->GetEntry() == NPC_ZOMBIE_CHOW && unit->IsAlive())
        {
            float distance = bot->GetExactDist2d(unit);
            if (distance < nearestDistance)
            {
                nearestDistance = distance;
                zombie = unit;
            }
        }
    }
    
    // 如果找到僵尸，攻击它
    if (zombie)
    {
        return Attack(zombie);
    }
    
    // 如果没有僵尸，攻击Boss
    return Attack(boss);
}

// ==========================================
// 海根动作实现
// ==========================================

// 需求 3.3, 3.6: 执行海根跳舞机制
bool NaxxHeiganDanceAction::Execute(Event event)
{
    // 查找海根
    Unit* boss = AI_VALUE2(Unit*, "find target", "heigan");
    if (!boss || boss->GetEntry() != NPC_HEIGAN)
        return false;
    
    // 验证Boss在战斗中
    if (!boss->IsAlive() || !boss->IsInCombat())
        return false;
    
    // 海根跳舞的安全区域坐标（简化版本）
    // 实际游戏中需要根据喷发区域动态计算
    struct SafeZone {
        float x, y, z;
    };
    
    // 预定义的安全区域（房间的四个角落）
    SafeZone safeZones[] = {
        {2771.0f, -3298.0f, 267.7f},  // 区域 0
        {2771.0f, -3312.0f, 267.7f},  // 区域 1
        {2771.0f, -3326.0f, 267.7f},  // 区域 2
        {2771.0f, -3340.0f, 267.7f}   // 区域 3
    };
    
    // 找到最近的安全区域
    float botX = bot->GetPositionX();
    float botY = bot->GetPositionY();
    float nearestDistance = 1000.0f;
    SafeZone* nearestZone = nullptr;
    
    for (int i = 0; i < 4; ++i)
    {
        float dx = botX - safeZones[i].x;
        float dy = botY - safeZones[i].y;
        float dist = sqrt(dx * dx + dy * dy);
        
        if (dist < nearestDistance)
        {
            nearestDistance = dist;
            nearestZone = &safeZones[i];
        }
    }
    
    // 移动到最近的安全区域
    if (nearestZone)
    {
        return MoveTo(boss->GetMapId(), nearestZone->x, nearestZone->y, nearestZone->z,
                     false, false, false, false,
                     MovementPriority::MOVEMENT_COMBAT);
    }
    
    return false;
}

// ==========================================
// 四骑士动作实现
// ==========================================

// 需求 4.4, 4.5: 切换到标记层数最少的骑士
bool NaxxFourHorsemenSwitchAction::Execute(Event event)
{
    // 获取机器人身上的标记层数
    uint32 zeliekStacks = 0;
    uint32 blaumeuxStacks = 0;
    uint32 rivendareStacks = 0;
    uint32 korthazzStacks = 0;
    
    if (Aura* aura = bot->GetAura(SPELL_MARK_OF_ZELIEK))
        zeliekStacks = aura->GetStackAmount();
    if (Aura* aura = bot->GetAura(SPELL_MARK_OF_BLAUMEUX))
        blaumeuxStacks = aura->GetStackAmount();
    if (Aura* aura = bot->GetAura(SPELL_MARK_OF_RIVENDARE))
        rivendareStacks = aura->GetStackAmount();
    if (Aura* aura = bot->GetAura(SPELL_MARK_OF_KORTHAZZ))
        korthazzStacks = aura->GetStackAmount();
    
    // 找到标记层数最少的骑士
    struct HorsemanInfo {
        uint32 npcEntry;
        uint32 stacks;
        const char* name;
    };
    
    HorsemanInfo horsemen[] = {
        {NPC_ZELIEK, zeliekStacks, "zeliek"},
        {NPC_BLAUMEUX, blaumeuxStacks, "blaumeux"},
        {NPC_RIVENDARE, rivendareStacks, "rivendare"},
        {NPC_KORTHAZZ, korthazzStacks, "korthazz"}
    };
    
    // 找到标记最少的骑士
    HorsemanInfo* bestTarget = &horsemen[0];
    for (int i = 1; i < 4; ++i)
    {
        if (horsemen[i].stacks < bestTarget->stacks)
        {
            bestTarget = &horsemen[i];
        }
    }
    
    // 查找目标骑士
    Unit* target = AI_VALUE2(Unit*, "find target", bestTarget->name);
    if (!target || target->GetEntry() != bestTarget->npcEntry)
        return false;
    
    // 攻击目标骑士
    return Attack(target);
}

// ==========================================
// 塔迪乌斯动作实现
// ==========================================

// 需求 5.3, 5.4: 移动到对应的极性区域
bool NaxxThaddiusMoveToPolarityAction::Execute(Event event)
{
    // 查找塔迪乌斯
    Unit* boss = AI_VALUE2(Unit*, "find target", "thaddius");
    if (!boss || boss->GetEntry() != NPC_THADDIUS)
        return false;
    
    // 验证Boss在战斗中
    if (!boss->IsAlive() || !boss->IsInCombat())
        return false;
    
    // 检查机器人的极性
    bool hasPositive = bot->HasAura(SPELL_POSITIVE_CHARGE);
    bool hasNegative = bot->HasAura(SPELL_NEGATIVE_CHARGE);
    
    if (!hasPositive && !hasNegative)
        return false;
    
    // 极性区域坐标
    struct PolarityZone {
        float x, y, z;
    };
    
    PolarityZone positiveZone = {3510.0f, -2950.0f, 312.0f};
    PolarityZone negativeZone = {3450.0f, -2950.0f, 312.0f};
    
    // 根据极性选择目标区域
    PolarityZone* targetZone = hasPositive ? &positiveZone : &negativeZone;
    
    // 移动到极性区域
    return MoveTo(boss->GetMapId(), targetZone->x, targetZone->y, targetZone->z,
                 false, false, false, false,
                 MovementPriority::MOVEMENT_COMBAT);
}

// ==========================================
// 戈提克动作实现
// ==========================================

// 需求 14.3: 第一阶段攻击生者侧小怪
bool NaxxGothikAttackLivingSideAction::Execute(Event event)
{
    // 查找戈提克
    Unit* boss = AI_VALUE2(Unit*, "find target", "gothik");
    if (!boss || boss->GetEntry() != NPC_GOTHIK)
        return false;
    
    // 验证是第一阶段
    if (boss->IsVisible() && boss->CanHaveThreatList())
        return false;
    
    // 搜索生者侧的小怪（简化：攻击最近的敌对单位）
    Unit* add = nullptr;
    float nearestDistance = 100.0f;
    
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (auto& guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;
        
        // 攻击任何敌对的小怪
        if (unit->IsAlive() && unit->GetEntry() != NPC_GOTHIK)
        {
            float distance = bot->GetExactDist2d(unit);
            if (distance < nearestDistance)
            {
                nearestDistance = distance;
                add = unit;
            }
        }
    }
    
    // 如果找到小怪，攻击它
    if (add)
    {
        return Attack(add);
    }
    
    return false;
}

// 需求 14.4: 第一阶段攻击亡者侧小怪
bool NaxxGothikAttackDeadSideAction::Execute(Event event)
{
    // 查找戈提克
    Unit* boss = AI_VALUE2(Unit*, "find target", "gothik");
    if (!boss || boss->GetEntry() != NPC_GOTHIK)
        return false;
    
    // 验证是第一阶段
    if (!boss->IsVisible() || boss->CanHaveThreatList())
        return false;
    
    // 搜索亡者侧的小怪
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (auto& guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;
        
        // 检查是否是亡者侧的小怪（简化：检查位置）
        // 亡者侧在房间的另一侧
        if (unit->GetPositionX() < 2700.0f)  // 示例坐标
        {
            return Attack(unit);
        }
    }
    
    return false;
}

// 需求 14.5: 第二阶段攻击戈提克
bool NaxxGothikAttackBossAction::Execute(Event event)
{
    // 查找戈提克
    Unit* boss = AI_VALUE2(Unit*, "find target", "gothik");
    if (!boss || boss->GetEntry() != NPC_GOTHIK)
        return false;
    
    // 验证是第二阶段（Boss可攻击）
    if (!boss->IsVisible() || !boss->CanHaveThreatList())
        return false;
    
    // 攻击Boss
    return Attack(boss);
}

// ==========================================
// 拉祖维奥斯动作实现
// ==========================================

// 需求 15.3: 精神控制死亡骑士学徒
bool NaxxRazuviousMindControlAction::Execute(Event event)
{
    // 只有牧师或术士可以控制
    if (bot->getClass() != CLASS_PRIEST && bot->getClass() != CLASS_WARLOCK)
        return false;
    
    // 查找拉祖维奥斯
    Unit* boss = AI_VALUE2(Unit*, "find target", "razuvious");
    if (!boss || boss->GetEntry() != NPC_RAZUVIOUS)
        return false;
    
    // 搜索未被控制的死亡骑士学徒
    Unit* understudy = nullptr;
    
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
            {
                understudy = unit;
                break;
            }
        }
    }
    
    // 如果找到学徒，施放精神控制
    if (understudy)
    {
        return botAI->CastSpell("mind control", understudy);
    }
    
    return false;
}

// 需求 15.4: 使用学徒嘲讽Boss
bool NaxxRazuviousTauntAction::Execute(Event event)
{
    // 这个动作需要控制学徒并使用其技能
    // 简化实现：假设学徒会自动嘲讽
    return true;
}

// 需求 15.5: 切换控制到另一个学徒
bool NaxxRazuviousSwitchControlAction::Execute(Event event)
{
    // 只有牧师或术士可以控制
    if (bot->getClass() != CLASS_PRIEST && bot->getClass() != CLASS_WARLOCK)
        return false;
    
    // 查找拉祖维奥斯
    Unit* boss = AI_VALUE2(Unit*, "find target", "razuvious");
    if (!boss || boss->GetEntry() != NPC_RAZUVIOUS)
        return false;
    
    // 搜索未被控制的死亡骑士学徒
    Unit* understudy = nullptr;
    
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
            {
                understudy = unit;
                break;
            }
        }
    }
    
    // 如果找到学徒，施放精神控制
    if (understudy)
    {
        return botAI->CastSpell("mind control", understudy);
    }
    
    return false;
}

// ==========================================
// 萨菲隆动作实现
// ==========================================

// 需求 7.3, 7.4: 躲在冰块后面
bool NaxxSapphironHideBehindIceBlockAction::Execute(Event event)
{
    // 查找萨菲隆
    Unit* boss = AI_VALUE2(Unit*, "find target", "sapphiron");
    if (!boss || boss->GetEntry() != NPC_SAPPHIRON)
        return false;
    
    // 验证Boss在空中阶段
    if (!boss->HasUnitMovementFlag(MOVEMENTFLAG_FLYING))
        return false;
    
    // 搜索冰块（被冰霜冲击冻结的玩家）
    Unit* iceBlock = nullptr;
    float nearestDistance = 100.0f;
    
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (auto& guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;
        
        // 检查是否有冰霜冲击效果（冰块）
        if (unit->HasAura(SPELL_ICEBOLT))
        {
            float distance = bot->GetExactDist2d(unit);
            if (distance < nearestDistance)
            {
                nearestDistance = distance;
                iceBlock = unit;
            }
        }
    }
    
    // 如果找到冰块，移动到其后面
    if (iceBlock && boss)
    {
        // 计算冰块与Boss之间的连线
        float bossX = boss->GetPositionX();
        float bossY = boss->GetPositionY();
        float iceX = iceBlock->GetPositionX();
        float iceY = iceBlock->GetPositionY();
        
        // 计算方向向量（从Boss到冰块）
        float dx = iceX - bossX;
        float dy = iceY - bossY;
        float distance = sqrt(dx * dx + dy * dy);
        
        if (distance > 0.1f)
        {
            // 标准化
            dx /= distance;
            dy /= distance;
            
            // 在冰块后面5码的位置
            float targetX = iceX + dx * 5.0f;
            float targetY = iceY + dy * 5.0f;
            float targetZ = iceBlock->GetPositionZ();
            
            // 移动到冰块后面
            return MoveTo(boss->GetMapId(), targetX, targetY, targetZ,
                         false, false, false, false,
                         MovementPriority::MOVEMENT_COMBAT);
        }
    }
    
    return false;
}

// ==========================================
// 克尔苏加德动作实现
// ==========================================

// 需求 16.2: 第一阶段攻击小怪
bool NaxxKelThuzadAttackAddsAction::Execute(Event event)
{
    // 查找克尔苏加德
    Unit* boss = AI_VALUE2(Unit*, "find target", "kel'thuzad");
    if (!boss || boss->GetEntry() != NPC_KELTHUZAD)
        return false;
    
    // 验证是第一阶段
    if (boss->CanHaveThreatList())
        return false;
    
    // 搜索小怪
    Unit* add = nullptr;
    float nearestDistance = 100.0f;
    
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (auto& guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;
        
        // 攻击任何敌对的小怪
        if (unit->IsAlive() && unit->GetEntry() != NPC_KELTHUZAD)
        {
            float distance = bot->GetExactDist2d(unit);
            if (distance < nearestDistance)
            {
                nearestDistance = distance;
                add = unit;
            }
        }
    }
    
    // 如果找到小怪，攻击它
    if (add)
    {
        return Attack(add);
    }
    
    return false;
}

// 需求 16.3: 第二阶段攻击Boss
bool NaxxKelThuzadAttackBossAction::Execute(Event event)
{
    // 查找克尔苏加德
    Unit* boss = AI_VALUE2(Unit*, "find target", "kel'thuzad");
    if (!boss || boss->GetEntry() != NPC_KELTHUZAD)
        return false;
    
    // 验证是第二阶段
    if (!boss->CanHaveThreatList() || boss->GetHealthPct() <= 40.0f)
        return false;
    
    // 攻击Boss
    return Attack(boss);
}

// 需求 16.6: 第三阶段攻击守护者
bool NaxxKelThuzadAttackGuardianAction::Execute(Event event)
{
    // 查找克尔苏加德
    Unit* boss = AI_VALUE2(Unit*, "find target", "kel'thuzad");
    if (!boss || boss->GetEntry() != NPC_KELTHUZAD)
        return false;
    
    // 验证是第三阶段
    if (boss->GetHealthPct() > 40.0f)
        return false;
    
    // 搜索冰冠守护者
    Unit* guardian = nullptr;
    float nearestDistance = 100.0f;
    
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (auto& guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;
        
        // 检查是否是冰冠守护者
        if (unit->GetEntry() == NPC_GUARDIAN_OF_ICECROWN && unit->IsAlive())
        {
            float distance = bot->GetExactDist2d(unit);
            if (distance < nearestDistance)
            {
                nearestDistance = distance;
                guardian = unit;
            }
        }
    }
    
    // 如果找到守护者，攻击它
    if (guardian)
    {
        return Attack(guardian);
    }
    
    // 如果没有守护者，攻击Boss
    return Attack(boss);
}

// 需求 16.5: 被冰霜冲击冻结时等待解冻
bool NaxxKelThuzadWaitForUnfreezeAction::Execute(Event event)
{
    // 验证机器人被冰霜冲击冻结
    if (!bot->HasAura(SPELL_FROST_BLAST))
        return false;
    
    // 停止所有动作，等待解冻
    return true;
}

// 需求 16.6: 远离暗影裂隙
bool NaxxKelThuzadMoveFissureAction::Execute(Event event)
{
    // 验证机器人在暗影裂隙中
    if (!bot->HasAura(SPELL_SHADOW_FISSURE))
        return false;
    
    // 查找克尔苏加德
    Unit* boss = AI_VALUE2(Unit*, "find target", "kel'thuzad");
    if (!boss || boss->GetEntry() != NPC_KELTHUZAD)
        return false;
    
    // 快速移动到随机方向
    float botX = bot->GetPositionX();
    float botY = bot->GetPositionY();
    float botZ = bot->GetPositionZ();
    
    // 选择一个随机方向
    float angle = frand(0, M_PI * 2);
    float distance = 10.0f;
    
    float targetX = botX + distance * cos(angle);
    float targetY = botY + distance * sin(angle);
    float targetZ = botZ;
    
    // 快速移动
    return MoveTo(boss->GetMapId(), targetX, targetY, targetZ,
                 false, false, false, false,
                 MovementPriority::MOVEMENT_COMBAT);
}

// By Leewheel 2026-02-14
