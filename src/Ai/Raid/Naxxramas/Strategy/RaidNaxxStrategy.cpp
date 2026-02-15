/*
 * 版权所有 (C) 2026 Leewheel
 * 
 * 文件功能：纳克萨玛斯团队副本机器人策略实现
 * 实现纳克萨玛斯副本各Boss的触发器和倍率初始化逻辑
 * 
 * By Leewheel 2026-02-14
 */

#include "RaidNaxxStrategy.h"

void RaidNaxxStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // ==========================================
    // 帕奇维克 (Patchwerk) - 构造区
    // ==========================================
    
    // 副坦克定位 - ACTION_RAID + 3 (位置调整优先级)
    // 需求 17.3: 位置调整优先级
    triggers.push_back(new TriggerNode(
        "naxx patchwerk offtank position",
        { NextAction("naxx patchwerk offtank position", ACTION_RAID + 3) }
    ));
    
    // 狂乱燃烧阶段 - ACTION_RAID + 5 (重要团队机制优先级)
    // 需求 17.2: 重要团队机制优先级
    triggers.push_back(new TriggerNode(
        "naxx patchwerk frenzy",
        { NextAction("naxx patchwerk burn", ACTION_RAID + 5) }
    ));
    
    // 狂暴燃烧阶段 - ACTION_RAID + 5 (重要团队机制优先级)
    // 需求 17.2: 重要团队机制优先级
    triggers.push_back(new TriggerNode(
        "naxx patchwerk berserk",
        { NextAction("naxx patchwerk burn", ACTION_RAID + 5) }
    ));
    
    // ==========================================
    // 格罗布鲁斯 (Grobbulus) - 构造区
    // ==========================================
    
    // 变异注射 - 移动到边缘 - ACTION_EMERGENCY (紧急躲避优先级)
    // 需求 17.1: 紧急躲避优先级
    // 需求 6.2, 6.3: 当机器人获得变异注射时，移动到边缘
    triggers.push_back(new TriggerNode(
        "naxx grobbulus mutating injection",
        { NextAction("naxx grobbulus move to edge", ACTION_EMERGENCY) }
    ));
    
    // 毒云 - 躲避毒云 - ACTION_EMERGENCY (紧急躲避优先级)
    // 需求 17.1: 紧急躲避优先级
    // 需求 6.4, 6.5: 检测并躲避毒云区域
    triggers.push_back(new TriggerNode(
        "naxx grobbulus poison cloud",
        { NextAction("naxx grobbulus avoid poison cloud", ACTION_EMERGENCY) }
    ));
    
    // ==========================================
    // 阿努布雷坎 (Anub'Rekhan) - 蜘蛛区
    // ==========================================
    
    // 蝗虫群 - 分散站位 - ACTION_EMERGENCY (紧急躲避优先级)
    // 需求 17.1: 紧急躲避优先级
    // 需求 8.2, 8.3, 8.4: 检测蝗虫群并分散站位
    triggers.push_back(new TriggerNode(
        "naxx anubrekhan locust swarm",
        { NextAction("naxx anubrekhan spread out", ACTION_EMERGENCY) }
    ));
    
    // 蝗虫群 - 攻击地穴守卫 - ACTION_RAID + 1 (普通机制优先级)
    // 需求 17.4: 普通机制优先级
    // 需求 8.6: 优先攻击地穴守卫小怪
    triggers.push_back(new TriggerNode(
        "naxx anubrekhan locust swarm",
        { NextAction("naxx anubrekhan attack crypt guard", ACTION_RAID + 1) }
    ));
    
    // ==========================================
    // 费尔莉娜 (Faerlina) - 蜘蛛区
    // ==========================================
    
    // 狂乱 - 攻击崇拜者 - ACTION_RAID + 5 (重要团队机制优先级)
    // 需求 17.2: 重要团队机制优先级
    // 需求 9.3, 9.4: 检测狂乱并攻击崇拜者
    triggers.push_back(new TriggerNode(
        "naxx faerlina frenzy",
        { NextAction("naxx faerlina attack worshipper", ACTION_RAID + 5) }
    ));
    
    // ==========================================
    // 迈克斯纳 (Maexxna) - 蜘蛛区
    // ==========================================
    
    // 蛛网缠绕 - 等待救援 - ACTION_EMERGENCY (紧急躲避优先级)
    // 需求 17.1: 紧急躲避优先级
    // 需求 10.3: 被蛛网缠绕时等待救援
    triggers.push_back(new TriggerNode(
        "naxx maexxna web wrap",
        { NextAction("naxx maexxna wait for rescue", ACTION_EMERGENCY) }
    ));
    
    // 毒性冲击 - 停止施法 - ACTION_EMERGENCY (紧急躲避优先级)
    // 需求 17.1: 紧急躲避优先级
    // 需求 10.4: 毒性冲击时停止施法
    triggers.push_back(new TriggerNode(
        "naxx maexxna poison shock",
        { NextAction("naxx maexxna stop casting", ACTION_EMERGENCY) }
    ));
    
    // 蛛网喷射 - 攻击小蜘蛛 - ACTION_RAID + 1 (普通机制优先级)
    // 需求 17.4: 普通机制优先级
    // 需求 10.6: 攻击小蜘蛛
    triggers.push_back(new TriggerNode(
        "naxx maexxna web spray",
        { NextAction("naxx maexxna attack spiderling", ACTION_RAID + 1) }
    ));
    
    // ==========================================
    // 诺斯 (Noth) - 瘟疫区
    // ==========================================
    
    // 诺斯传送 - 切换到小怪 - ACTION_RAID + 3 (位置调整优先级)
    // 需求 17.3: 位置调整优先级
    // 需求 11.2, 11.3: 诺斯传送时切换到小怪
    triggers.push_back(new TriggerNode(
        "naxx noth teleport",
        { NextAction("naxx noth switch to adds", ACTION_RAID + 3) }
    ));
    
    // 诺斯重新出现 - 切换回Boss - ACTION_RAID + 5 (重要团队机制优先级)
    // 需求 17.2: 重要团队机制优先级
    // 需求 11.4: 诺斯重新出现时切换回Boss
    triggers.push_back(new TriggerNode(
        "naxx noth reappear",
        { NextAction("naxx noth switch to boss", ACTION_RAID + 5) }
    ));
    
    // ==========================================
    // 洛欧塞布 (Loatheb) - 瘟疫区
    // ==========================================
    
    // 死灵光环激活 - 停止治疗 - ACTION_RAID + 5 (重要团队机制优先级)
    // 需求 17.2: 重要团队机制优先级
    // 需求 12.2: 死灵光环激活时停止治疗
    triggers.push_back(new TriggerNode(
        "naxx loatheb necrotic aura active",
        { NextAction("naxx loatheb stop healing", ACTION_RAID + 5) }
    ));
    
    // 死灵光环消失 - 爆发治疗 - ACTION_RAID + 5 (重要团队机制优先级)
    // 需求 17.2: 重要团队机制优先级
    // 需求 12.3, 12.5: 死灵光环消失时爆发治疗
    triggers.push_back(new TriggerNode(
        "naxx loatheb necrotic aura inactive",
        { NextAction("naxx loatheb burst healing", ACTION_RAID + 5) }
    ));
    
    // ==========================================
    // 格拉斯 (Gluth) - 构造区
    // ==========================================
    
    // 僵尸食尸鬼附近 - 风筝僵尸 - ACTION_RAID + 3 (位置调整优先级)
    // 需求 17.3: 位置调整优先级
    // 需求 13.3: 风筝僵尸食尸鬼远离Boss
    triggers.push_back(new TriggerNode(
        "naxx gluth zombie chow nearby",
        { NextAction("naxx gluth kite zombies", ACTION_RAID + 3) }
    ));
    
    // 僵尸食尸鬼附近 - 击杀僵尸 - ACTION_RAID + 1 (普通机制优先级)
    // 需求 17.4: 普通机制优先级
    // 需求 13.6: 击杀僵尸食尸鬼
    triggers.push_back(new TriggerNode(
        "naxx gluth zombie chow nearby",
        { NextAction("naxx gluth kill zombies", ACTION_RAID + 1) }
    ));
    
    // ==========================================
    // 海根 (Heigan) - 瘟疫区
    // ==========================================
    
    // 海根跳舞 - ACTION_EMERGENCY + 5 (最高优先级)
    // 需求 17.1: 紧急躲避优先级
    // 需求 3.1, 3.2, 3.3: 检测地板喷发并移动到安全区域
    triggers.push_back(new TriggerNode(
        "naxx heigan dance",
        { NextAction("naxx heigan dance", ACTION_EMERGENCY + 5) }
    ));
    
    // ==========================================
    // 四骑士 (Four Horsemen) - 军事区
    // ==========================================
    
    // 标记层数过高 - 切换目标 - ACTION_RAID + 5 (重要团队机制优先级)
    // 需求 17.2: 重要团队机制优先级
    // 需求 4.2, 4.3, 4.4: 检测标记层数并切换目标
    triggers.push_back(new TriggerNode(
        "naxx four horsemen mark high",
        { NextAction("naxx four horsemen switch", ACTION_RAID + 5) }
    ));
    
    // ==========================================
    // 塔迪乌斯 (Thaddius) - 构造区
    // ==========================================
    
    // 极性转换 - 移动到极性区域 - ACTION_RAID + 5 (重要团队机制优先级)
    // 需求 17.2: 重要团队机制优先级
    // 需求 5.1, 5.2, 5.3, 5.4: 检测极性并移动到对应区域
    triggers.push_back(new TriggerNode(
        "naxx thaddius polarity shift",
        { NextAction("naxx thaddius move to polarity", ACTION_RAID + 5) }
    ));
    
    // ==========================================
    // 戈提克 (Gothik) - 军事区
    // ==========================================
    
    // 第一阶段 - 攻击生者侧 - ACTION_RAID + 1 (普通机制优先级)
    // 需求 17.4: 普通机制优先级
    // 需求 14.2, 14.3: 第一阶段攻击生者侧小怪
    triggers.push_back(new TriggerNode(
        "naxx gothik phase one",
        { NextAction("naxx gothik attack living side", ACTION_RAID + 1) }
    ));
    
    // 第二阶段 - 攻击Boss - ACTION_RAID + 5 (重要团队机制优先级)
    // 需求 17.2: 重要团队机制优先级
    // 需求 14.5: 第二阶段攻击戈提克
    triggers.push_back(new TriggerNode(
        "naxx gothik phase two",
        { NextAction("naxx gothik attack boss", ACTION_RAID + 5) }
    ));
    
    // ==========================================
    // 拉祖维奥斯 (Razuvious) - 军事区
    // ==========================================
    
    // 需要控制 - 精神控制学徒 - ACTION_RAID + 5 (重要团队机制优先级)
    // 需求 17.2: 重要团队机制优先级
    // 需求 15.2, 15.3: 精神控制死亡骑士学徒
    triggers.push_back(new TriggerNode(
        "naxx razuvious need control",
        { NextAction("naxx razuvious mind control", ACTION_RAID + 5) }
    ));
    
    // 控制即将到期 - 切换控制 - ACTION_RAID + 5 (重要团队机制优先级)
    // 需求 17.2: 重要团队机制优先级
    // 需求 15.5: 切换控制到另一个学徒
    triggers.push_back(new TriggerNode(
        "naxx razuvious control expiring",
        { NextAction("naxx razuvious switch control", ACTION_RAID + 5) }
    ));
    
    // ==========================================
    // 萨菲隆 (Sapphiron) - 冰龙巢穴
    // ==========================================
    
    // 空中阶段 - 躲在冰块后面 - ACTION_EMERGENCY + 5 (最高优先级)
    // 需求 17.1: 紧急躲避优先级
    // 需求 7.2, 7.3, 7.4: 检测空中阶段并躲在冰块后面
    triggers.push_back(new TriggerNode(
        "naxx sapphiron air phase",
        { NextAction("naxx sapphiron hide behind ice block", ACTION_EMERGENCY + 5) }
    ));
    
    // ==========================================
    // 克尔苏加德 (Kel'Thuzad) - 冰龙巢穴
    // ==========================================
    
    // 第一阶段 - 攻击小怪 - ACTION_RAID + 1 (普通机制优先级)
    // 需求 17.4: 普通机制优先级
    // 需求 16.2: 第一阶段攻击小怪
    triggers.push_back(new TriggerNode(
        "naxx kelthuzad phase one",
        { NextAction("naxx kelthuzad attack adds", ACTION_RAID + 1) }
    ));
    
    // 第二阶段 - 攻击Boss - ACTION_RAID + 3 (位置调整优先级)
    // 需求 17.3: 位置调整优先级
    // 需求 16.3: 第二阶段攻击Boss
    triggers.push_back(new TriggerNode(
        "naxx kelthuzad phase two",
        { NextAction("naxx kelthuzad attack boss", ACTION_RAID + 3) }
    ));
    
    // 第三阶段 - 攻击守护者 - ACTION_RAID + 5 (重要团队机制优先级)
    // 需求 17.2: 重要团队机制优先级
    // 需求 16.6: 第三阶段攻击守护者
    triggers.push_back(new TriggerNode(
        "naxx kelthuzad phase three",
        { NextAction("naxx kelthuzad attack guardian", ACTION_RAID + 5) }
    ));
    
    // 冰霜冲击 - 等待解冻 - ACTION_EMERGENCY (紧急躲避优先级)
    // 需求 17.1: 紧急躲避优先级
    // 需求 16.5: 被冰霜冲击冻结时等待解冻
    triggers.push_back(new TriggerNode(
        "naxx kelthuzad frost blast",
        { NextAction("naxx kelthuzad wait for unfreeze", ACTION_EMERGENCY) }
    ));
    
    // 暗影裂隙 - 远离裂隙 - ACTION_EMERGENCY + 5 (最高优先级)
    // 需求 17.1: 紧急躲避优先级
    // 需求 16.6: 远离暗影裂隙
    triggers.push_back(new TriggerNode(
        "naxx kelthuzad shadow fissure",
        { NextAction("naxx kelthuzad move fissure", ACTION_EMERGENCY + 5) }
    ));
    
    // 待办：在后续任务中添加其他Boss触发器
}

void RaidNaxxStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // 待办：添加纳克萨玛斯倍率
    // 将在后续任务中填充
}

// By Leewheel 2026-02-14
