/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UseItemAction.h"

#include "ChatHelper.h"
#include "Event.h"
#include "ItemCountValue.h"
#include "ItemPackets.h"
#include "ItemUsageValue.h"
#include "LootObjectStack.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"
#include "ServerFacade.h"

static constexpr uint32 SPELL_LEARNING_1 = 483;
static constexpr uint32 SPELL_LEARNING_2 = 55884;

bool UseItemAction::Execute(Event event)
{
    std::string name = event.getParam();
    if (name.empty())
        name = getName();

    std::vector<Item*> items = AI_VALUE2(std::vector<Item*>, "inventory items", name);
    // By leewheel 2026-09-04 防悬空崩溃: "inventory items" 缓存1000ms, 窗口内物品可能
    //   被用掉/堆叠合并/交易而销毁, 缓存中的裸指针悬空(玩家崩溃日志:
    //   CanUseItem→Item::GetTemplate→Object::GetUInt32Value C0000005)。
    //   取用前按 bot 当前背包物品指针集合过滤, 悬空指针不解引用直接丢弃。
    // End By leewheel
    items = InventoryItemValueBase::FilterLive(bot, items);
    GuidVector gos = chat->parseGameobjects(name);

    if (gos.empty())
    {
        if (!items.empty())
        {
            return UseItemAuto(*items.begin());
        }
    }
    else
    {
        if (items.empty())
            return UseGameObject(*gos.begin());
        else
            return UseItemOnGameObject(*items.begin(), *gos.begin());
    }

//By leewheel 2026-10-06 合并 brighton the-lab: 上游把默认文本回退成英文，
    //   保留上游的 key 名与调用形式，仅把默认文本换回本核中文（汉化在 GetBotTextOrDefault 的第二参数）
    botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault("use_item_none_available",
                                                                      "没有可用的物品（或游戏对象）", {}));
    return false;
}

bool UseItemAction::UseGameObject(ObjectGuid guid)
{
    auto fail = [this](char const* name, char const* defaultText)
    {
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(name, defaultText, {}));
        return false;
    };

    GameObject* go = botAI->GetGameObject(guid);
    if (!go || !go->isSpawned())
        return fail("gameobject_unavailable_error", "Game object is no longer available");

    if (sPlayerbotAIConfig.DisallowedGameObjects.contains(go->GetEntry()))
        return fail("gameobject_disallowed_error", "Game object is disallowed by configuration");

    if (sPlayerbotAIConfig.LootDistance && bot->GetDistance(go) > sPlayerbotAIConfig.LootDistance)
        return fail("gameobject_outside_loot_distance_error", "Game object is outside the configured loot distance");

    if (go->HasFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE) ||
        (go->HasFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND) && !go->ActivateToQuest(bot)))
        return fail("gameobject_not_eligible_error", "Game object is not currently eligible for interaction");

    if (!bot->IsAlive() || bot->IsInFlight() || bot->m_mover != bot || bot->IsNonMeleeSpellCast(false) ||
        bot->GetLootGUID())
        return fail("gameobject_cannot_interact_error",
                    "Cannot interact while dead, flying, remotely controlled, casting, or looting");

    if (go->GetGoType() == GAMEOBJECT_TYPE_CHEST || go->GetGOInfo()->GetLootId())
    {
        LootObject loot(bot, guid);
        if (!loot.IsLootPossible(bot))
            return fail("gameobject_cannot_loot_error",
                        "Cannot loot this object: check quest, skill, tools, key, and object state");

        bool inRange = bot->GetDistance(go) <= INTERACTION_DISTANCE - 2.0f;
        if (botAI->HasStrategy("stay", BOT_STATE_NON_COMBAT) && bot->GetDistance(go) > CONTACT_DISTANCE)
            return fail("gameobject_stay_out_of_range_error", "Game object is out of reach while staying");

        bool canContinue =
            botAI->HasStrategy("loot", BOT_STATE_NON_COMBAT) || botAI->HasStrategy("gather", BOT_STATE_NON_COMBAT);
        if (!inRange && !canContinue)
            return fail("gameobject_approach_unavailable_error",
                        "Move closer or enable the loot or gather strategy to approach this object");

        LootObject previous = AI_VALUE(LootObject, "loot target");
        LootObjectStack* availableLoot = AI_VALUE(LootObjectStack*, "available loot");
        bool added = availableLoot->Add(guid);
        context->GetValue<LootObject>("loot target")->Set(loot);

        bool retryGuaranteed = inRange && (bot->isMoving() || bot->IsMounted());
        std::string objectName = chat->FormatGameobject(go);
        bool requested = botAI->DoSpecificAction(inRange ? "open loot" : "move to loot", Event(), true);
        if (!requested && !retryGuaranteed)
        {
            if (added && availableLoot->CanAttemptLoot(guid))
                availableLoot->Remove(guid);
            if (previous.guid != guid || availableLoot->CanAttemptLoot(previous.guid))
                context->GetValue<LootObject>("loot target")->Set(previous);
            else
                context->GetValue<LootObject>("loot target")->Set(LootObject());
            return fail("gameobject_open_failed_error", "Could not approach or open the game object");
        }

        botAI->TellMasterNoFacing(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            inRange && requested ? "gameobject_open_requested" : "gameobject_loot_queued",
            inRange && requested ? "Opening requested: %gameobject" : "Queued for looting: %gameobject",
            {{"%gameobject", objectName}}));
        return true;
    }

    if (go->GetGOInfo()->GetLockId() && go->HasFlag(GAMEOBJECT_FLAGS, GO_FLAG_LOCKED))
        return fail("gameobject_nonloot_locked_error", "This non-loot object requires an opening spell or key");

    if (!go->IsWithinDistInMap(bot, go->GetInteractionDistance()))
        return fail("gameobject_interact_out_of_range_error", "Move closer to interact with this game object");

    if (bot->isMoving())
        bot->StopMoving();
    ServerFacade::instance().SetFacingTo(bot, go);

    WorldPacket use(CMSG_GAMEOBJ_USE, 8);
    use << guid;
    bot->GetSession()->HandleGameObjectUseOpcode(use);

    go = botAI->GetGameObject(guid);
    if (go && go->isSpawned() && go->IsWithinDistInMap(bot, INTERACTION_DISTANCE))
    {
        WorldPacket report(CMSG_GAMEOBJ_REPORT_USE, 8);
        report << guid;
        bot->GetSession()->HandleGameobjectReportUse(report);
    }

    botAI->TellMasterNoFacing(PlayerbotTextMgr::instance().GetBotTextOrDefault(
//By leewheel 2026-10-06 合并 brighton the-lab: 上游把 key 改名为 gameobject_interaction_requested
        //   但同时丢掉了 %gameobject 占位符（英文文本是硬编码 "Game object interaction requested"），
        //   玩家看不到是哪个物体。处置：沿用上游新 key 名，保留本核带 %gameobject 占位符的中文文本，
        //   这样汉化表能按新 key 覆盖英文，且信息不丢。
        "gameobject_interaction_requested",
        "正在使用 %gameobject",
        {{"%gameobject", chat->FormatGameobject(go)}}));
    return true;
}

bool UseItemAction::UseItemAuto(Item* item) { return UseItem(item, ObjectGuid::Empty, nullptr); }

bool UseItemAction::UseItemOnGameObject(Item* item, ObjectGuid go) { return UseItem(item, go, nullptr); }

bool UseItemAction::UseItemOnItem(Item* item, Item* itemTarget) { return UseItem(item, ObjectGuid::Empty, itemTarget); }

bool UseItemAction::UseItem(Item* item, ObjectGuid goGuid, Item* itemTarget, Unit* unitTarget)
{
    // By leewheel 2026-09-04 防悬空崩溃(纵深防御): 上游调用方(Execute/CheckMountState/OCActions等)
    //   传入的 Item* 可能来自1000ms缓存已失效, 进入 deref 前统一过滤一次。
    // End By leewheel
    if (item)
    {
        std::vector<Item*> check = InventoryItemValueBase::FilterLive(bot, {item});
        if (check.empty())
            return false;
    }
    else
        return false;

    if (bot->CanUseItem(item) != EQUIP_ERR_OK)
        return false;

    if (bot->IsNonMeleeSpellCast(false))
        return false;

    uint8 bagIndex = item->GetBagSlot();
    uint8 slot = item->GetSlot();
    uint8 cast_count = 1;
    ObjectGuid item_guid = item->GetGUID();
    uint32 glyphIndex = 0;
    uint8 castFlags = 0;
    uint32 targetFlag = TARGET_FLAG_NONE;
    GameObject* goTarget = goGuid ? botAI->GetGameObject(goGuid) : nullptr;
    if (goGuid && (!goTarget || !goTarget->isSpawned()))
        return false;

    uint32 spellId = 0;
    ItemTemplate const* itemProto = item->GetTemplate();
    bool const isGenericLearnItem =
        itemProto->Spells[0].SpellId == SPELL_LEARNING_1 || itemProto->Spells[0].SpellId == SPELL_LEARNING_2;

    if (isGenericLearnItem)
    {
        if (bot->HasSpell(itemProto->Spells[1].SpellId))
            return false;
    }
    else if (itemProto->Spells[0].SpellId)
    {
        // Older/direct layout: Spells[0] itself teaches the spell(s), via one or
        // more SPELL_EFFECT_LEARN_SPELL effects (not necessarily in effect slot 0).
        if (SpellInfo const* learnSpellInfo = sSpellMgr->GetSpellInfo(itemProto->Spells[0].SpellId))
        {
            bool foundLearnEffect = false;
            bool allKnown = true;
            for (auto const& effect : learnSpellInfo->Effects)
            {
                if (effect.Effect != SPELL_EFFECT_LEARN_SPELL || !effect.TriggerSpell)
                    continue;

                foundLearnEffect = true;
                if (!bot->HasSpell(effect.TriggerSpell))
                {
                    allKnown = false;
                    break;
                }
            }

            if (foundLearnEffect && allKnown)
                return false;
        }
    }

    // Only check index 0 for generic-learn items; slot 1 is the taught spell id
    uint8 const spellSlotLimit = isGenericLearnItem ? 1 : MAX_ITEM_PROTO_SPELLS;

    for (uint8 i = 0; i < spellSlotLimit; ++i)
    {
        if (itemProto->Spells[i].SpellId > 0)
        {
            spellId = itemProto->Spells[i].SpellId;
            bool canCast = goTarget ? botAI->CanCastSpell(spellId, goTarget, false, item)
                                    : botAI->CanCastSpell(spellId, bot, false, itemTarget, item);
            if (!canCast)
                return false;
        }
    }

    WorldPacket packet(CMSG_USE_ITEM);
    packet << bagIndex << slot << cast_count << spellId << item_guid << glyphIndex << castFlags;

    bool targetSelected = false;

    std::string itemText = chat->FormatItem(item->GetTemplate());
    std::string targetText;

    if (item->GetTemplate()->Stackable > 1)
    {
        uint32 count = item->GetCount();
        if (count > 1)
            itemText += " (" + std::to_string(count) + " available)";
        else
            itemText += " (the last one!)";
    }

    if (goTarget)
    {
        targetFlag = TARGET_FLAG_GAMEOBJECT;

        packet << targetFlag;
        packet << goGuid.WriteAsPacked();
        targetText = chat->FormatGameobject(goTarget);
        targetSelected = true;
    }

    if (itemTarget)
    {
        if (item->GetTemplate()->Class == ITEM_CLASS_GEM)
        {
            bool fit = SocketItem(itemTarget, item) || SocketItem(itemTarget, item, true);
            if (!fit)
//By leewheel 2026-10-06 合并 brighton the-lab: key 名两边一致，仅默认文本本核为中文，保留中文
                botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                    "socket_does_not_fit", "插槽不匹配", {}));

            return fit;
        }
        else
        {
            targetFlag = TARGET_FLAG_ITEM;
            packet << targetFlag;
            packet << itemTarget->GetGUID().WriteAsPacked();
            targetText = chat->FormatItem(itemTarget->GetTemplate());
            targetSelected = true;
        }
    }

    Player* master = GetMaster();
    if (!targetSelected && item->GetTemplate()->Class != ITEM_CLASS_CONSUMABLE && master &&
        IsRealPlayer(botAI->GetMaster()) && !selfOnly)
    {
        if (ObjectGuid masterSelection = master->GetTarget())
        {
            Unit* unit = botAI->GetUnit(masterSelection);
            if (unit)
            {
                targetFlag = TARGET_FLAG_UNIT;
                packet << targetFlag << masterSelection.WriteAsPacked();
                targetText = unit->GetName();
                targetSelected = true;
            }
        }
    }

    if (!targetSelected && item->GetTemplate()->Class != ITEM_CLASS_CONSUMABLE && unitTarget)
    {
        targetFlag = TARGET_FLAG_UNIT;
        packet << targetFlag << unitTarget->GetGUID().WriteAsPacked();
        targetText = unitTarget->GetName();
        targetSelected = true;
    }

    if (uint32 questid = item->GetTemplate()->StartQuest)
    {
        if (Quest const* qInfo = sObjectMgr->GetQuestTemplate(questid))
        {
            WorldPacket packet(CMSG_QUESTGIVER_ACCEPT_QUEST, 8 + 4 + 4);
            packet << item_guid;
            packet << questid;
            packet << uint32(0);
            bot->GetSession()->HandleQuestgiverAcceptQuestOpcode(packet);

            botAI->TellMasterNoFacing("获得任务 " + chat->FormatQuest(qInfo));
            return true;
        }
    }

    bot->ClearUnitState(UNIT_STATE_CHASE);
    bot->ClearUnitState(UNIT_STATE_FOLLOW);

    if (bot->isMoving())
    {
        bot->StopMoving();
        botAI->SetNextCheckDelay(sPlayerbotAIConfig.GlobalCoolDown);
        return false;
    }

    for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; i++)
    {
        uint32 spellId = item->GetTemplate()->Spells[i].SpellId;
        if (!spellId)
            continue;

        if (!botAI->CanCastSpell(spellId, bot, false))
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (spellInfo->Targets & TARGET_FLAG_ITEM)
        {
            Item* itemForSpell = AI_VALUE2(Item*, "item for spell", spellId);
            if (!itemForSpell)
                continue;

            if (itemForSpell->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
                continue;

            if (bot->GetTrader())
            {
                if (selfOnly)
                    return false;

                targetFlag = TARGET_FLAG_TRADE_ITEM;
                packet << targetFlag << (uint8)1 << ObjectGuid((uint64)TRADE_SLOT_NONTRADED).WriteAsPacked();
                targetSelected = true;
                targetText = "交易物品";
            }
            else
            {
                targetFlag = TARGET_FLAG_ITEM;
                packet << targetFlag;
                packet << itemForSpell->GetGUID().WriteAsPacked();
                targetSelected = true;
                targetText = chat->FormatItem(itemForSpell->GetTemplate());
            }
            uint32 castTime = spellInfo->CalcCastTime();
            botAI->SetNextCheckDelay(castTime + sPlayerbotAIConfig.ReactDelay);
        }

        break;
    }

    if (!targetSelected)
    {
        targetFlag = TARGET_FLAG_NONE;
        packet << targetFlag;

        // Use the actual target if provided
        if (unitTarget)
        {
            packet << unitTarget->GetGUID();
            targetSelected = true;

            if (unitTarget == bot || !unitTarget->IsInWorld() || unitTarget->IsDuringRemoveFromWorld())
                targetText = "self";
            else if (unitTarget->IsHostileTo(bot))
                targetText = "self";
            else
                targetText = unitTarget->GetName();
        }
        else
        {
            packet << bot->GetPackGUID();
            targetSelected = true;
            targetText = "self";
        }
    }

    ItemTemplate const* proto = item->GetTemplate();
    bool isDrink = proto->Spells[0].SpellCategory == 59;
    bool isFood = proto->Spells[0].SpellCategory == 11;
    if (proto->Class == ITEM_CLASS_CONSUMABLE &&
        (proto->SubClass == ITEM_SUBCLASS_FOOD || proto->SubClass == ITEM_SUBCLASS_CONSUMABLE) && (isFood || isDrink))
    {
        if (bot->IsInCombat())
            return false;

        // bot->SetStandState(UNIT_STAND_STATE_SIT);
        bot->CastStop();
        float hp = bot->GetHealthPct();
        float mp = bot->GetPower(POWER_MANA) * 100.0f / bot->GetMaxPower(POWER_MANA);
        float p = 0.f;
        if (isDrink && isFood)
        {
            p = std::min(hp, mp);
            TellConsumableUse(item, "盛宴", p);
        }
        else if (isDrink)
        {
            p = mp;
            TellConsumableUse(item, "饮水", p);
        }
        else if (isFood)
        {
            p = std::min(hp, mp);
            TellConsumableUse(item, "进食", p);
        }

        if (!bot->IsInCombat() && !bot->InBattleground())
            SetDuration((uint32)std::max(10.0f * IN_MILLISECONDS, 27.0f * IN_MILLISECONDS * (100 - p) / 100.0f));

        if (!bot->IsInCombat() && bot->InBattleground())
            SetDuration((uint32)std::max(10.0f * IN_MILLISECONDS, 20.0f * IN_MILLISECONDS * (100 - p) / 100.0f));

        // botAI->SetNextCheckDelay(27000.0f * (100 - p) / 100.0f);
        //  botAI->SetNextCheckDelay(20000);
        bot->GetSession()->HandleUseItemOpcode(packet);

        return true;
    }

    if (!spellId)
        return false;

// botAI->SetNextCheckDelay(sPlayerbotAIConfig.GlobalCoolDown);
    // By leewheel 2026-10-07 合并 #2854：配置成员随上游改为 PascalCase（globalCoolDown -> GlobalCoolDown）；
    //   下列使用提示文案保留本 fork 的中文默认值。
    std::string useText =
        targetSelected
            ? PlayerbotTextMgr::instance().GetBotTextOrDefault("use_item_on_target", "正在对 %target 使用 %item",
                                                               {{"%item", itemText}, {"%target", targetText}})
            : PlayerbotTextMgr::instance().GetBotTextOrDefault("use_item", "正在使用 %item", {{"%item", itemText}});
    botAI->TellMasterNoFacing(useText);
    bot->GetSession()->HandleUseItemOpcode(packet);
    return true;
}

void UseItemAction::TellConsumableUse(Item* item, std::string const action, float percent)
{
    if (!sPlayerbotAIConfig.AnnounceConsumableUse)
        return;

    std::ostringstream out;
    out << action << " " << chat->FormatItem(item->GetTemplate());

    if (item->GetTemplate()->Stackable > 1)
        out << "/x" << item->GetCount();

    out << " (" << round(percent) << "%)";
    botAI->TellMasterNoFacing(out.str());
}

bool UseItemAction::SocketItem(Item* item, Item* gem, bool replace)
{
    WorldPacket packet(CMSG_SOCKET_GEMS);
    packet << item->GetGUID();

    // A buckle's gem goes into the first colourless template socket - see WorldSession::HandleSocketOpcode.
    uint8 firstPrismatic = 0;
    while (firstPrismatic < MAX_GEM_SOCKETS && item->GetTemplate()->Socket[firstPrismatic].Color)
        ++firstPrismatic;

    bool const hasPrismaticSocket = item->GetEnchantmentId(PRISMATIC_ENCHANTMENT_SLOT) != 0;

    bool fits = false;
    for (uint32 enchant_slot = SOCK_ENCHANTMENT_SLOT; enchant_slot < SOCK_ENCHANTMENT_SLOT + MAX_GEM_SOCKETS;
         ++enchant_slot)
    {
        uint32 socketIndex = enchant_slot - SOCK_ENCHANTMENT_SLOT;
        uint8 socketColor = item->GetTemplate()->Socket[socketIndex].Color;
        GemPropertiesEntry const* gemProperty = sGemPropertiesStore.LookupEntry(gem->GetTemplate()->GemProperties);

        // A socket added by a buckle carries no colour of its own and takes any gem except a meta one.
        bool const isPrismatic = !socketColor && hasPrismaticSocket && socketIndex == firstPrismatic;
        bool const gemFitsSocket = gemProperty && (isPrismatic ? gemProperty->color != SOCKET_COLOR_META
                                                               : (gemProperty->color & socketColor) != 0);
        if (gemFitsSocket)
        {
            if (fits)
            {
                packet << ObjectGuid::Empty;
                continue;
            }

            uint32 enchant_id = item->GetEnchantmentId(EnchantmentSlot(enchant_slot));
            if (!enchant_id)
            {
                packet << gem->GetGUID();
                fits = true;
                continue;
            }

            SpellItemEnchantmentEntry const* enchantEntry = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
            if (!enchantEntry || !enchantEntry->GemID)
            {
                packet << gem->GetGUID();
                fits = true;
                continue;
            }

            if (replace && enchantEntry->GemID != gem->GetTemplate()->ItemId)
            {
                packet << gem->GetGUID();
                fits = true;
                continue;
            }
        }

        packet << ObjectGuid::Empty;
    }

    if (fits)
    {
        botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
//By leewheel 2026-10-06 合并 brighton the-lab: key 名与占位符一致，保留本核中文文本
            "socketing_item_with_gem",
            "正在用 %gem 镶嵌 %item",
            {{"%item", chat->FormatItem(item->GetTemplate())}, {"%gem", chat->FormatItem(gem->GetTemplate())}}));

        WorldPackets::Item::SocketGems nicePacket(std::move(packet));
        nicePacket.Read();
        bot->GetSession()->HandleSocketOpcode(nicePacket);
    }

    return fits;
}

bool UseItemAction::isPossible() { return getName() == "use" || AI_VALUE2(uint32, "item count", getName()) > 0; }

bool UseSpellItemAction::isUseful() { return AI_VALUE2(bool, "spell cast useful", getName()); }

bool UseHealingPotion::isUseful() { return AI_VALUE2(bool, "combat", "self target"); }

bool UseManaPotion::isUseful() { return AI_VALUE2(bool, "combat", "self target"); }

bool UseHearthStone::Execute(Event event)
{
    if (bot->isMoving())
    {
        MotionMaster& mm = *bot->GetMotionMaster();
        bot->StopMoving();
        mm.Clear();
    }

    bool used = UseItemAction::Execute(event);
    if (used)
    {
        RESET_AI_VALUE(bool, "combat::self target");
        RESET_AI_VALUE(WorldPosition, "current position");
        botAI->SetNextCheckDelay(10 * IN_MILLISECONDS);
    }

    return used;
}

bool UseHearthStone::isUseful() { return !bot->InBattleground(); }

bool UseRandomRecipe::Execute(Event /*event*/)
{
    std::vector<Item*> recipes = AI_VALUE2(std::vector<Item*>, "inventory items", "recipe");
    // By leewheel 2026-09-04 防悬空崩溃: 过滤缓存列表中已失效的物品指针
    // End By leewheel
    recipes = InventoryItemValueBase::FilterLive(bot, recipes);

    std::string recipeName = "";

    for (auto& recipe : recipes)
    {
        recipeName = recipe->GetTemplate()->Name1;
    }

    if (recipeName.empty())
        return false;

    bool used = UseItemAction::Execute(Event(name, recipeName));

    if (used)
        botAI->SetNextCheckDelay(3.0 * IN_MILLISECONDS);

    return used;
}

bool UseRandomRecipe::isUseful()
{
    return !bot->IsInCombat() && !IsRealPlayer(botAI->GetMaster()) && !bot->InBattleground();
}

bool UseRandomRecipe::isPossible() { return AI_VALUE2(uint32, "item count", "recipe") > 0; }

bool UseRandomQuestItem::Execute(Event /*event*/)
{
    Unit* unitTarget = nullptr;
    ObjectGuid goTarget;

    std::vector<Item*> questItems = AI_VALUE2(std::vector<Item*>, "inventory items", "quest");
    // By leewheel 2026-09-04 防悬空崩溃: 过滤缓存列表中已失效的物品指针
    // End By leewheel
    questItems = InventoryItemValueBase::FilterLive(bot, questItems);
    if (questItems.empty())
        return false;

    Item* item = nullptr;
    for (uint8 i = 0; i < 5; i++)
    {
        auto itr = questItems.begin();
        std::advance(itr, urand(0, questItems.size() - 1));
        Item* questItem = *itr;

        ItemTemplate const* proto = questItem->GetTemplate();
        if (proto->StartQuest)
        {
            Quest const* qInfo = sObjectMgr->GetQuestTemplate(proto->StartQuest);
            if (bot->CanTakeQuest(qInfo, false))
            {
                item = questItem;
                break;
            }
        }
    }

    if (!item)
        return false;

    bool used = UseItem(item, goTarget, nullptr, unitTarget);
    if (used)
        botAI->SetNextCheckDelay(sPlayerbotAIConfig.GlobalCoolDown);

    return used;
}

bool UseRandomQuestItem::isUseful()
{
    return !IsRealPlayer(botAI->GetMaster()) && !bot->InBattleground() && !bot->HasUnitState(UNIT_STATE_IN_FLIGHT);
}

bool UseRandomQuestItem::isPossible() { return AI_VALUE2(uint32, "item count", "quest") > 0; }
