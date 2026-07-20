//By leewheel 2026-07-20
/*
 * BotBeggingInteraction.cpp - 新手玩家向高级机器人求助互动
 *
 * 功能说明：
 *   当等级<=10的玩家向等级>=60的同阵营机器人私信要金币或背包时，
 *   该机器人有45%的几率传送到玩家身边，主动给予金币或背包；
 *   55%的几率拒绝并回复嘲讽/不耐烦的话语。
 *
 * 要金币：给予玩家请求的金币数（最高88金），未指定数量则给88金。
 * 要背包：给予玩家4个22格1级可用背包 + 18金币。
 *
 * 作者: leewheel
 */

#include "Playerbots.h"

#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "WorldSession.h"
#include "Map.h"

#include <string>
#include <vector>
#include <cstdlib>

// 金币上限：88金 = 880000铜
static constexpr uint32 MAX_BEGGING_GOLD_COPPER = 88 * 10000;
// 要包时附赠金币：18金 = 180000铜
static constexpr uint32 BAG_BONUS_GOLD_COPPER = 18 * 10000;
// 给予背包数量
static constexpr uint32 BAG_COUNT = 4;
// 给予概率（百分比）
static constexpr uint32 GIVE_CHANCE_PERCENT = 45;

// 22格通用背包候选列表（bagFamily=0, RequiredLevel=0）
// 优先使用霜纹包(41599)，备选烈日背包(35516)、20格包包(1977)
static constexpr uint32 BAG_ITEM_CANDIDATES[] = { 41599, 35516, 1977 };
// 备选：不小于18格的通用背包（霜纹包本身就是22格，此处作为兜底）
static constexpr uint32 BAG_ITEM_FALLBACK[] = { 41599, 1977 };

// ============================================================
// 要钱的关键词列表（玩家可能的各种说法）
// ============================================================
static const char* s_goldKeywords[] = {
    "金币", "给我钱", "给点钱", "要钱", "借点钱", "借钱",
    "资助", "救济", "施舍", "给点金", "来点钱", "没钱了",
    "穷", "给钱", "要点钱", "给些钱", "给一些钱",
    "给我金", "来点金", "缺钱", "没钱", "赏点",
    "给几个金", "给点金币", "要金币", "来点金币",
    nullptr
};

// ============================================================
// 要包的关键词列表（玩家可能的各种说法）
// ============================================================
static const char* s_bagKeywords[] = {
    "背包", "包包", "给我包", "给个包", "要包", "来个包",
    "给几个包", "要几个包", "缺包", "没包了", "包满了",
    "给点包", "要个背包", "给个背包", "来个背包",
    "口袋", "给个袋", "要袋子", "包裹",
    nullptr
};

// ============================================================
// 给予时的友好话语（机器人传送过来后说的）
// ============================================================
static const char* s_giveGoldMessages[] = {
    "拿着吧，新手路上不容易。",
    "这点钱你收着，别跟我客气。",
    "看你可怜的，给你点路费。",
    "行了行了，给你点钱，省得你到处要。",
    "拿去花吧，当年我也是这么过来的。",
    "小意思，别嫌少。",
    "给你，省着点用。",
    "拿好了，别弄丢了。",
};

static const char* s_giveBagMessages[] = {
    "这几个包你拿着，够你装一阵子了。",
    "看你那小包袱，给你换几个大的。",
    "包给你，别再到处喊了。",
    "拿着，装东西用，别客气。",
    "给你几个包，新手就该有新手的样子。",
    "行了，包给你，钱也给你点，够意思了吧。",
};

// ============================================================
// 拒绝时的嘲讽/不耐烦话语
// ============================================================
static const char* s_refuseMessages[] = {
    "去去去，自己挣去，别整天伸手要。",
    "我凭什么给你？你又不是我徒弟。",
    "哼，又一个伸手党。",
    "想要钱？自己去打怪赚，别做梦了。",
    "你当我开银行的？滚。",
    "切，有手有脚的，自己不会赚？",
    "我看起来像冤大头吗？走开。",
    "你这是乞讨吗？你咋不要我的装备呢？",
    "笑死，现在的新人就知道要，自己不会努力？",
    "别烦我，忙着呢。",
    "你找错人了，我比你还穷。",
    "呵，想白嫖？门都没有。",
    "我辛辛苦苦攒的钱，凭什么给你？",
    "年轻人，要学会自力更生。",
    "你这是在乞讨吗？真丢人。",
    "不给，有本事你来打我啊。",
    "我当年可没人给我钱，自己练去。",
    "啧啧，又一个想不劳而获的。",
    "你的脸呢？",
    "做梦比较快，睡吧。",
};

// ============================================================
// 工具函数：判断字符串是否包含子串
// ============================================================
static bool MsgContains(const std::string& msg, const char* keyword)
{
    return msg.find(keyword) != std::string::npos;
}

// ============================================================
// 判断玩家是否在要钱
// ============================================================
static bool IsAskingForGold(const std::string& msg)
{
    for (int i = 0; s_goldKeywords[i] != nullptr; ++i)
    {
        if (MsgContains(msg, s_goldKeywords[i]))
            return true;
    }

    // 额外模式：消息中同时包含数字和"金"字，大概率是要钱
    // 例如 "给我10金" "来5个金币"
    if (msg.find("金") != std::string::npos)
    {
        for (char c : msg)
        {
            if (c >= '0' && c <= '9')
                return true;
        }
    }

    return false;
}

// ============================================================
// 判断玩家是否在要包
// 注意排除"面包"等食物相关词汇
// ============================================================
static bool IsAskingForBag(const std::string& msg)
{
    // 先排除明显的食物语境
    if (MsgContains(msg, "面包") && !MsgContains(msg, "背包") && !MsgContains(msg, "包包"))
    {
        // 如果只提到"面包"而没有其他包的关键词，不算要包
        // 但如果同时有"背包""包包"等，仍然算
        bool hasOtherBagKeyword = false;
        for (int i = 0; s_bagKeywords[i] != nullptr; ++i)
        {
            // 跳过单独的"包"字检查，用其他更明确的关键词
            if (std::string(s_bagKeywords[i]) == "包")
                continue;
            if (MsgContains(msg, s_bagKeywords[i]))
            {
                hasOtherBagKeyword = true;
                break;
            }
        }
        if (!hasOtherBagKeyword)
            return false;
    }

    for (int i = 0; s_bagKeywords[i] != nullptr; ++i)
    {
        if (MsgContains(msg, s_bagKeywords[i]))
            return true;
    }

    return false;
}

// ============================================================
// 从消息中解析玩家想要的金币数量（返回铜币数）
// 如果无法解析出具体数字，返回0（表示使用默认值）
// ============================================================
static uint32 ParseGoldAmount(const std::string& msg)
{
    // 尝试从消息中提取数字
    // 支持的模式："10个金币" "5金" "88金币" "给我10金" "来20个金"
    uint32 amount = 0;
    bool foundDigit = false;

    for (size_t i = 0; i < msg.size(); ++i)
    {
        if (msg[i] >= '0' && msg[i] <= '9')
        {
            // 提取连续数字
            uint32 num = 0;
            size_t j = i;
            while (j < msg.size() && msg[j] >= '0' && msg[j] <= '9')
            {
                num = num * 10 + (msg[j] - '0');
                ++j;
            }

            // 检查数字后面是否跟着"金"或"金币"
            // 也检查数字前面是否有"给""来""要"等动词（表示是请求的数量）
            std::string afterNum = msg.substr(j, std::min((size_t)6, msg.size() - j));
            if (afterNum.find("金") != std::string::npos ||
                afterNum.find("个金") != std::string::npos)
            {
                amount = num;
                foundDigit = true;
                break;
            }

            // 如果数字前面有"金"字（如"给我金10个"这种不太常见的说法）
            if (i > 0)
            {
                std::string beforeNum = msg.substr(0, i);
                if (beforeNum.find("金") != std::string::npos &&
                    (afterNum.find("个") != std::string::npos || afterNum.empty()))
                {
                    amount = num;
                    foundDigit = true;
                    break;
                }
            }

            i = j - 1; // 跳过已处理的数字
        }
    }

    if (!foundDigit || amount == 0)
        return 0; // 未找到有效数字，使用默认值

    // 转换为铜币并限制上限
    uint32 copper = amount * 10000;
    if (copper > MAX_BEGGING_GOLD_COPPER)
        copper = MAX_BEGGING_GOLD_COPPER;

    return copper;
}

// ============================================================
// 尝试给玩家添加背包，返回实际使用的物品ID
// 优先22格，失败则尝试备选
// ============================================================
static uint32 TryGiveBags(Player* player)
{
    // 优先尝试22格背包
    for (uint32 candidate : BAG_ITEM_CANDIDATES)
    {
        // 检查物品模板是否存在
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(candidate);
        if (!proto)
            continue;

        // 确认是通用背包（bagFamily=0）且格数>=22
        if (proto->BagFamily != 0 || proto->ContainerSlots < 22)
            continue;

        // 尝试添加
        if (player->AddItem(candidate, BAG_COUNT))
            return candidate;
    }

    // 兜底：尝试不小于18格的背包
    for (uint32 candidate : BAG_ITEM_FALLBACK)
    {
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(candidate);
        if (!proto)
            continue;

        if (proto->BagFamily != 0 || proto->ContainerSlots < 18)
            continue;

        if (player->AddItem(candidate, BAG_COUNT))
            return candidate;
    }

    return 0; // 全部失败
}

// ============================================================
// 主处理函数：处理新手玩家向高级机器人的求助私信
//
// 参数：
//   sender     - 发送私信的玩家
//   receiver   - 接收私信的机器人
//   msg        - 私信内容
//
// 返回值：
//   true  - 消息已被处理（是求助消息），调用方不应继续处理
//   false - 不是求助消息，调用方继续正常流程
// ============================================================
bool HandleBotBeggingInteraction(Player* sender, Player* receiver, const std::string& msg)
{
    // ---- 基本资格检查 ----

    // 发送者必须是真实玩家（非机器人）
    if (!sender || !sender->GetSession() || sender->GetSession()->IsBot())
        return false;

    // 接收者必须是机器人
    if (!receiver || !receiver->GetSession() || !receiver->GetSession()->IsBot())
        return false;

    // 玩家等级 <= 10
    if (sender->GetLevel() > 10)
        return false;

    // 机器人等级 >= 60
    if (receiver->GetLevel() < 60)
        return false;

    // 必须同阵营
    if (sender->GetTeamId() != receiver->GetTeamId())
        return false;

    // ---- 关键词检测 ----
    bool wantGold = IsAskingForGold(msg);
    bool wantBag = IsAskingForBag(msg);

    // 既不要钱也不要包，不处理
    if (!wantGold && !wantBag)
        return false;

    // ---- 机器人状态检查（用于决定是否能传送） ----
    bool canTeleport = true;

    // 机器人必须存活
    if (!receiver->IsAlive())
        canTeleport = false;

    // 机器人不能在战斗中
    if (receiver->IsInCombat())
        canTeleport = false;

    // 机器人不能在战场/竞技场中
    if (receiver->InBattleground() || receiver->InArena())
        canTeleport = false;

    // 玩家不能在副本中（机器人无法随意进入）
    if (sender->GetMap() && sender->GetMap()->IsDungeon())
        canTeleport = false;

    // 机器人不能在副本中
    if (receiver->GetMap() && receiver->GetMap()->IsDungeon())
        canTeleport = false;

    // ---- 45%概率判定 ----
    bool willGive = (urand(1, 100) <= GIVE_CHANCE_PERCENT) && canTeleport;

    // 选择语言（联盟用通用语，部落用兽人语）
    Language lang = (receiver->GetTeamId() == TEAM_ALLIANCE) ? LANG_COMMON : LANG_ORCISH;

    if (willGive)
    {
        // ==== 给予分支：传送到玩家身边并给予物品/金币 ====

        // 传送到玩家身边
        receiver->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_TELEPORTED | AURA_INTERRUPT_FLAG_CHANGE_MAP);
        receiver->TeleportTo(sender->GetMapId(), sender->GetPositionX(), sender->GetPositionY(),
                             sender->GetPositionZ(), sender->GetOrientation());

        if (wantBag)
        {
            // ---- 要包：给4个22格背包 + 18金 ----
            uint32 bagItemId = TryGiveBags(sender);

            // 附赠18金
            sender->ModifyMoney(BAG_BONUS_GOLD_COPPER);
            // 从机器人身上扣除对应金币（保持经济平衡）
            if (receiver->GetMoney() >= BAG_BONUS_GOLD_COPPER)
                receiver->ModifyMoney(-static_cast<int32>(BAG_BONUS_GOLD_COPPER));

            // 发送友好话语
            int msgIdx = urand(0, (sizeof(s_giveBagMessages) / sizeof(s_giveBagMessages[0])) - 1);
            receiver->Whisper(s_giveBagMessages[msgIdx], lang, sender);

            LOG_INFO("playerbots", "机器人 {} 向新手玩家 {} 赠送了{}个背包和18金币",
                     receiver->GetName(), sender->GetName(),
                     bagItemId ? BAG_COUNT : 0);
        }
        else
        {
            // ---- 要钱：给予请求的金币数（最高88金） ----
            uint32 goldCopper = ParseGoldAmount(msg);

            // 未指定数量或解析失败，默认给88金
            if (goldCopper == 0)
                goldCopper = MAX_BEGGING_GOLD_COPPER;

            // 确保不超过上限
            if (goldCopper > MAX_BEGGING_GOLD_COPPER)
                goldCopper = MAX_BEGGING_GOLD_COPPER;

            // 确保机器人有足够的钱（不够则补足）
            if (receiver->GetMoney() < goldCopper)
                receiver->SetMoney(goldCopper);

            // 给玩家金币
            sender->ModifyMoney(goldCopper);
            // 从机器人身上扣除
            receiver->ModifyMoney(-static_cast<int32>(goldCopper));

            // 发送友好话语
            int msgIdx = urand(0, (sizeof(s_giveGoldMessages) / sizeof(s_giveGoldMessages[0])) - 1);
            receiver->Whisper(s_giveGoldMessages[msgIdx], lang, sender);

            LOG_INFO("playerbots", "机器人 {} 向新手玩家 {} 赠送了{}金{}银{}铜",
                     receiver->GetName(), sender->GetName(),
                     goldCopper / 10000, (goldCopper % 10000) / 100, (goldCopper % 100) / 1);
        }
    }
    else
    {
        // ==== 拒绝分支：嘲讽/不耐烦回复 ====
        int msgIdx = urand(0, (sizeof(s_refuseMessages) / sizeof(s_refuseMessages[0])) - 1);
        receiver->Whisper(s_refuseMessages[msgIdx], lang, sender);

        LOG_DEBUG("playerbots", "机器人 {} 拒绝了新手玩家 {} 的求助",
                  receiver->GetName(), sender->GetName());
    }

    // 消息已处理，不再走正常命令/聊天流程
    return true;
}
//End By leewheel
