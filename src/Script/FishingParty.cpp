/*
 * Copyright 2026 Leewheel
 *
 * 组队钓鱼策略:
 *   - 人数 ≤5: 机器人跟随玩家一起钓鱼（分散站位）
 *   - 人数 ≥6 且不在团本: 机器人骂街后逐个离队(3分钟内全部走完)
 *   - 团本内(raid instance): 不激活（部分团本需钓鱼触发boss）
 *   - 玩家停钓: 机器人不回心转意，继续离队
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Chat.h"
#include "Group.h"
#include "Map.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "WorldSession.h"
#include "World.h"
#include "Timer.h"
#include "ObjectAccessor.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Event.h"

#include <vector>
#include <unordered_map>
#include <string>
#include <random>
#include <cmath>

// ==================== 钓鱼法术ID ====================
static constexpr uint32 FISHING_SPELL_IDS[] = { 7620, 7731, 7732, 18248, 33095, 51294 };

static bool IsFishingSpell(uint32 spellId)
{
    for (uint32 id : FISHING_SPELL_IDS)
        if (id == spellId)
            return true;
    return false;
}

// ==================== 骂街语录 (150+ 条) ====================
static const char* COMPLAINTS[] = {
    // ---- 经典抱怨 ----
    "钓鱼？这么多人还钓鱼？老娘不奉陪了！",
    "我是来打本的，不是来看你钓鱼的，再见！",
    "钓鱼佬能不能自己退队？别浪费大家时间！",
    "组了这么多人你就给我看这个？溜了溜了！",
    "鱼比我们重要是吧？行，我走！",
    "你慢慢钓，我先告辞了！",
    "时间就是金钱朋友，而你却在钓鱼？",
    "这队伍没救了，全在看一个人钓鱼！",
    "我加入的是冒险队不是钓鱼队！",
    "再见了您嘞，我去找个正经队伍！",
    "钓鱼模拟器2026？我不玩了！",
    "团长带头摸鱼（物理意义上），散了散了！",
    "你是不是觉得鱼比我们更能打？",
    "钓鱼能出装备吗？不能？那我走了！",
    "我是来砍人的不是来看你钓鱼的！",
    "服了，组团看钓鱼，门票多少钱一张？",
    "请问这里是钓鱼俱乐部吗？我退会！",
    "钓到巫妖王了吗？没钓到？再见！",
    "你是不是以为我们在达拉然喷泉？",
    "我宁愿去单刷也不在这看你钓鱼！",
    // ---- 愤怒骂街 ----
    "你!#$%的钓什么鱼！老子退了！",
    "我真服了！组满人钓鱼？你脑子进水了？",
    "退队！不跟钓鱼佬浪费时间！",
    "我忍你很久了！天天钓鱼！走了！",
    "你钓鱼我受罪，凭什么？退队！",
    "你是来玩游戏的还是来模拟退休生活的？",
    "这是魔兽世界不是钓鱼世界！",
    "我怒气值满了！拜拜！",
    "你钓一条我就骂一句，你钓一百条我骂一百句！",
    "钓鱼佬真可怕，我先跑了！",
    "你是不是钓到橙竿了？这么执着？",
    "组队界面写着「来DPS」，进来发现是「来DPS...钓鱼」？",
    "我要去举报你！钓鱼误国！",
    "你信不信我把你的鱼竿抢过来折了？",
    "钓鱼也就算了，你组满人钓鱼是几个意思？",
    "你是不是觉得人多鱼就会自己跳上来？",
    "我真是倒了八辈子霉才进了你的队！",
    "你这是组团忽悠鱼呢？",
    "你有没有考虑过鱼的感受？也没有考虑过我们的？",
    "我装备都红了就为了看你钓鱼？",
    // ---- 幽默吐槽 ----
    "你钓的是鱼还是我们的耐心？",
    "恭喜你获得成就：气走所有队友！",
    "我帮你数着，你钓了几条我就走几个人！",
    "你是暴雪派来的卧底吧？专门气走玩家的？",
    "鱼说：求求你别钓了，你队友要走了！",
    "你猜猜下一个走的是谁？是我！",
    "我以为你是带我们打本，没想到是带我们看海！",
    "你钓的是鱼，我丢的是时间！",
    "鱼肯定在想：这人队友都走光了还在钓？",
    "你是不是在等一条叫「队友的耐心」的鱼？那已经跑光了！",
    "我去找纳特·帕格评评理！",
    "纳特帕格都没你这么能钓！",
    "你是不是把钓鱼技能绑在鼠标左键上了？",
    "你钓上来的鱼够开一个海鲜市场了！",
    "钓鱼冠军就是你，气走队友冠军也是你！",
    "鱼漂动了，我的心也凉了！",
    "你是部落派来的间谍吧？专业拆队？",
    "我猜你下一杆会钓上来一个大大的「退队」！",
    "艾泽拉斯的鱼都要被你钓灭绝了！",
    "请问你钓到过队友吗？今天你钓到了，就是我！",
    // ---- 角色扮演 ----
    "圣光在上！我不能容忍这种浪费时间的行为！",
    "先祖之灵告诉我，该离开这个队伍了...",
    "月神指引我离开这片水域！",
    "大地母亲在忽悠着你...也在忽悠我离开！",
    "为了部落！...算了不为部落了，我走了！",
    "愿圣光与你同在...我去找别的圣光了！",
    "元素之灵已经愤怒了！它们不喜欢被无视！",
    "我感受到了虚空的召唤...比这队伍有意思多了！",
    "巫妖王都没你这么冷酷无情！",
    "连死亡之翼都比你更尊重队员的时间！",
    "萨格拉斯看了都摇头！",
    "我是死亡骑士不是钓鱼骑士！",
    "恶魔猎手不钓鱼，恶魔猎手只猎恶魔！",
    "我牺牲了视力换来了魔能，不是为了看人钓鱼的！",
    "自然之力在哭泣...因为没人打本！",
    "奥术智慧告诉我：立刻退队！",
    "暗影牧师感受到了你的钓鱼欲望...太可怕了！",
    "战士的怒火已经被你的钓鱼行为点燃了！",
    "德鲁伊变熊了，不是因为战斗，是因为太无聊了！",
    "萨满的图腾都倒了，因为没有战斗可打！",
    // ---- 方言/外语梗 ----
    "Fishing again? I'm out!",
    "C'est fini! Je ne peux plus!",
    "你慢慢fishing，我快快leaving！",
    "我回奥格瑞玛了，拜拜了您嘞！",
    "这队伍待不下去了，我回暴风城种地去！",
    "我去藏宝海湾当海盗也比在这看你钓鱼强！",
    "荆棘谷的猩猩都比你有团队精神！",
    "我去诺森德冻死也不在这无聊死！",
    "外域都比这队伍有温度！",
    "我选择回出生地重新开始人生！",
    "炉石！启动！再见！",
    "我的炉石CD刚好，这就是命运！",
    "这队伍的气氛比冰冠冰川还冷！",
    "你的钓鱼技能450/450，你的领导力0/450！",
    "我去找个钓鱼团吧，起码那里面钓鱼是正事！",
    // ---- 追加 ----
    "你是不是把「集合石」看成「钓鱼台」了？",
    "我数到三，不停钓我就退！一...二...三！再见！",
    "你钓的鱼能分我一条吗？不能？那我走了！",
    "我是来刷声望的，不是来看你刷钓鱼技能的！",
    "这比等稀有刷新还无聊！",
    "你钓我也钓，大家一起钓——不对，我退队了！",
    "再见，我要去一个有追求的团队！",
    "你猜你的鱼竿能不能钓到一个新队友？",
    "我去主城喊一嗓子，看谁愿意来看你钓鱼！",
    "你要是能钓上来一个BOSS我就留下来！",
    "我不是针对你，我是针对所有在团队里钓鱼的人！",
    "你确定你不是在用钓鱼bot？",
    "钓鱼是好事，但组满人钓鱼就是你的不对了！",
    "我来翻译一下你的行为：你们都是空气，鱼才是真爱！",
    "你的钓鱼宏能不能顺便加一个「解散队伍」？",
    "鱼比你更懂团队合作，至少它们会成群！",
    "我猜你的钓鱼技能比你的社交技能高很多！",
    "你在水边钓的是鱼，你在队里钓的是队友的底线！",
    "钓鱼能出凤凰吗？不能？那我为什么在这？",
    "我要去找卡德加，问问他能不能把我传送走！",
    "你的鱼漂就是我离队的倒计时！",
    "这队伍最大的BOSS是无聊，我已经被击败了！",
    "我去刷无敌了，起码那还有1%的希望！",
    "退队保平安，钓鱼毁一生！",
    "你是不是在测试新鱼竿？拿我们当小白鼠？",
    "老子连加尔鲁什都不怕，但怕了你的钓鱼！",
    "钓鱼五分钟，看表两小时！",
    "这是团队不是你的私人鱼塘！",
    "我去找吉安娜了，起码她知道什么时候该战斗！",
    "萨尔要是看到你在这钓鱼，估计也会退队！",
    "希尔瓦娜斯都不会这么折磨被遗忘者！",
    "你再钓下去我就要变成敌对阵营了！",
    "退队！我要去找个打架的队伍，打架！懂吗！",
    "钓鱼不犯法，但组满人钓鱼犯众怒！",
    "你是艾泽拉斯最持久的男人——在钓鱼这件事上！",
    "我真想把你的鱼竿插到伊利丹的角上！",
    "你到底在钓什么？钓我们的命吗？",
    "我要是鱼我早就咬钩了——然后诅咒你！",
    "你钓上来的鱼够建一个娜迦军团了！",
    "再钓我就要开始在水里放食人鱼了！",
    "你是不是在做法？召唤水元素帮你钓鱼？",
    "这队伍的名字应该改成「一个人的钓鱼与一群人的等待」！",
    "你的钓鱼日志比你的团队日志还长吧？",
    "我去找钓鱼训练师告你，说你破坏钓鱼声誉！",
    "我甚至开始羡慕鱼了，起码它们不用组队！",
    "你难道就不觉得水面倒映着队友们失望的脸吗？",
    "别钓了！鱼都要学会走路上来打你了！",
    "我去单挑巫妖王都比这刺激！",
    "你知道现在几点了吗？钓鱼时间到，退队时间也到了！",
    "你的鱼竿是不是被诅咒了？诅咒内容是：队友全跑光！",
    "我宁愿被死亡之翼踩一脚也不想再看一秒钓鱼！",
    "我要去找青铜龙回到组队前的那一刻！",
    "此时此刻，鱼和你一样快乐，只有我不快乐！",
    "你是用钓鱼来筛选有耐心的队友吗？我出局了！",
    "艾泽拉斯第一钓鱼天团，成员：你一个人！",
};

static constexpr size_t COMPLAINT_COUNT = sizeof(COMPLAINTS) / sizeof(COMPLAINTS[0]);

static const char* PickRandomComplaint()
{
    return COMPLAINTS[urand(0, COMPLAINT_COUNT - 1)];
}

// ==================== 状态管理 ====================
struct FishingPartyState
{
    uint32 lastFishingCastMs = 0;
    std::vector<ObjectGuid> leaveQueue;
    uint32 nextLeaveMs = 0;
    uint32 totalLeaveDurationMs = 0;
    size_t leaveIndex = 0;
    bool active = false;
};

static std::unordered_map<ObjectGuid, FishingPartyState> _states;
static std::mt19937 _rng(std::random_device{}());

// ==================== 帮助函数 ====================
static bool IsRaidInstance(Player* player)
{
    return player->GetMap() && player->GetMap()->IsRaid();
}

static std::vector<Player*> GetBotsInGroup(Player* player)
{
    std::vector<Player*> bots;
    Group* group = player->GetGroup();
    if (!group)
        return bots;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == player)
            continue;

        if (member->GetSession() && member->GetSession()->IsBot())
            bots.push_back(member);
    }
    return bots;
}

// ==================== PlayerScript ====================
class FishingPartyPlayerScript : public PlayerScript
{
public:
    FishingPartyPlayerScript() : PlayerScript("FishingPartyPlayerScript") { }

    void OnPlayerSpellCast(Player* player, Spell* spell, bool /*skipCheck*/) override
    {
        if (!player || !spell)
            return;

        SpellInfo const* spellInfo = spell->GetSpellInfo();
        if (!spellInfo || !IsFishingSpell(spellInfo->Id))
            return;

        Group* group = player->GetGroup();
        if (!group)
            return;

        uint32 memberCount = group->GetMembersCount();
        if (memberCount < 2)
            return;

        // 团本内不触发
        if (IsRaidInstance(player))
            return;

        auto& state = _states[player->GetGUID()];
        state.lastFishingCastMs = getMSTime();

        if (memberCount <= 5)
        {
            HandleFishTogether(player);
        }
        else if (memberCount >= 6)
        {
            if (!state.active)
                StartLeaveSequence(player, state);
        }
    }

private:
    // ≤5人：机器人分散站位钓鱼
    void HandleFishTogether(Player* player)
    {
        std::vector<Player*> bots = GetBotsInGroup(player);
        if (bots.empty())
            return;

        float playerX = player->GetPositionX();
        float playerY = player->GetPositionY();
        float playerZ = player->GetPositionZ();
        uint32 mapId = player->GetMapId();
        size_t botCount = bots.size();

        for (size_t i = 0; i < botCount; i++)
        {
            Player* bot = bots[i];
            PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
            if (!botAI)
                continue;

            // 每个bot在玩家周围不同角度和距离散开（5~14码）
            float angle = (2.0f * M_PI * i) / botCount;
            float dist = 5.0f + (i % 4) * 3.0f;
            float offsetX = playerX + cos(angle) * dist;
            float offsetY = playerY + sin(angle) * dist;

            // 预设不同的钓鱼搜索起点
            WorldPosition spot(mapId, offsetX, offsetY, playerZ);
            botAI->GetAiObjectContext()->GetValue<WorldPosition>("fishing spot")->Set(spot);

            botAI->DoSpecificAction("go fishing", Event(), true);
        }
    }

    // ≥6人：启动离队流程
    void StartLeaveSequence(Player* player, FishingPartyState& state)
    {
        std::vector<Player*> bots = GetBotsInGroup(player);
        if (bots.empty())
            return;

        std::shuffle(bots.begin(), bots.end(), _rng);

        state.leaveQueue.clear();
        for (Player* bot : bots)
            state.leaveQueue.push_back(bot->GetGUID());

        uint32 leaveCount = static_cast<uint32>(bots.size());
        state.totalLeaveDurationMs = 180000;
        uint32 interval = state.totalLeaveDurationMs / leaveCount;

        state.leaveIndex = 0;
        state.nextLeaveMs = getMSTime() + interval;
        state.active = true;
    }
};

// ==================== WorldScript ====================
class FishingPartyWorldScript : public WorldScript
{
public:
    FishingPartyWorldScript() : WorldScript("FishingPartyWorldScript") { }

    void OnUpdate(uint32 /*diff*/) override
    {
        uint32 now = getMSTime();
        std::vector<ObjectGuid> toRemove;

        for (auto& [playerGuid, state] : _states)
        {
            if (!state.active)
                continue;

            if (now >= state.nextLeaveMs && state.leaveIndex < state.leaveQueue.size())
                ProcessNextLeave(state, now);

            if (state.leaveIndex >= state.leaveQueue.size() && state.leaveQueue.size() > 0)
                toRemove.push_back(playerGuid);
        }

        for (auto& guid : toRemove)
            _states.erase(guid);
    }

private:
    void ProcessNextLeave(FishingPartyState& state, uint32 now)
    {
        Player* bot = ObjectAccessor::FindPlayer(state.leaveQueue[state.leaveIndex]);
        if (bot && bot->GetGroup())
        {
            PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
            if (botAI)
            {
                botAI->TellMaster(PickRandomComplaint());
                bot->GetGroup()->RemoveMember(bot->GetGUID());
            }
        }

        state.leaveIndex++;

        if (state.leaveIndex < state.leaveQueue.size())
        {
            uint32 leaveCount = static_cast<uint32>(state.leaveQueue.size());
            uint32 interval = state.totalLeaveDurationMs / leaveCount;
            state.nextLeaveMs = now + interval;
        }
    }
};

// ==================== 注册 ====================
void AddSC_FishingParty()
{
    new FishingPartyPlayerScript();
    new FishingPartyWorldScript();
}
