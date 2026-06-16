/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "QuickPartyHelper.h"

#include "AiFactory.h"
#include "Group.h"
#include "Item.h"
#include "ObjectAccessor.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotFactory.h"
#include "PlayerbotOperations.h"
#include "Playerbots.h"
#include "RandomPlayerbotMgr.h"
#include "World.h"

namespace
{
BotRoles GetBotRole(Player* player)
{
    if (PlayerbotAI::IsTank(player, true))
        return BOT_ROLE_TANK;
    if (PlayerbotAI::IsHeal(player, true))
        return BOT_ROLE_HEALER;
    return BOT_ROLE_DPS;
}

void ShuffleBots(std::vector<Player*>& bots)
{
    for (size_t i = bots.size(); i > 1; --i)
    {
        size_t j = urand(0, i - 1);
        std::swap(bots[i - 1], bots[j]);
    }
}
}  // namespace

uint32 QuickPartyHelper::GearQualityForLevel(uint32 level)
{
    return level >= 60 ? ITEM_QUALITY_EPIC : ITEM_QUALITY_RARE;
}

void QuickPartyHelper::GetRoleRequirements(uint32 partySize, std::unordered_map<BotRoles, uint32>& roles)
{
    roles.clear();
    if (partySize <= 5)
    {
        roles[BOT_ROLE_TANK] = 1;
        roles[BOT_ROLE_HEALER] = 1;
        roles[BOT_ROLE_DPS] = 3;
    }
    else if (partySize <= 10)
    {
        roles[BOT_ROLE_TANK] = 2;
        roles[BOT_ROLE_HEALER] = 3;
        roles[BOT_ROLE_DPS] = 5;
    }
    else if (partySize <= 25)
    {
        roles[BOT_ROLE_TANK] = 3;
        roles[BOT_ROLE_HEALER] = 7;
        roles[BOT_ROLE_DPS] = 15;
    }
    else
    {
        roles[BOT_ROLE_TANK] = 4;
        roles[BOT_ROLE_HEALER] = 16;
        roles[BOT_ROLE_DPS] = 20;
    }
}

BotRoles QuickPartyHelper::GetPlayerRole(Player* player)
{
    return GetBotRole(player);
}

void QuickPartyHelper::CountExistingRoles(Player* player, Group* group,
                                          std::unordered_map<BotRoles, uint32>& roles)
{
    if (!group)
    {
        BotRoles role = GetPlayerRole(player);
        if (roles[role] > 0)
            roles[role]--;
        return;
    }

    Group::MemberSlotList const& slots = group->GetMemberSlots();
    for (Group::member_citerator itr = slots.begin(); itr != slots.end(); ++itr)
    {
        Player* member = ObjectAccessor::FindPlayer(itr->guid);
        if (!member)
            continue;

        BotRoles role = GetBotRole(member);
        if (roles[role] > 0)
            roles[role]--;
    }
}

bool QuickPartyHelper::CanClassFillRole(uint8 cls, BotRoles role)
{
    if (role == BOT_ROLE_DPS)
        return true;

    if (role == BOT_ROLE_TANK)
    {
        return cls == CLASS_WARRIOR || cls == CLASS_PALADIN || cls == CLASS_DEATH_KNIGHT || cls == CLASS_DRUID;
    }

    return cls == CLASS_PRIEST || cls == CLASS_PALADIN || cls == CLASS_SHAMAN || cls == CLASS_DRUID;
}

bool QuickPartyHelper::BotMatchesRole(Player* bot, BotRoles role)
{
    if (role == BOT_ROLE_TANK)
        return PlayerbotAI::IsTank(bot, true);
    if (role == BOT_ROLE_HEALER)
        return PlayerbotAI::IsHeal(bot, true);
    return !PlayerbotAI::IsTank(bot, true) && !PlayerbotAI::IsHeal(bot, true);
}

uint32 QuickPartyHelper::GetRoleSpecTab(uint8 cls, BotRoles role)
{
    if (role == BOT_ROLE_TANK)
    {
        switch (cls)
        {
            case CLASS_WARRIOR:
                return WARRIOR_TAB_PROTECTION;
            case CLASS_PALADIN:
                return PALADIN_TAB_PROTECTION;
            case CLASS_DEATH_KNIGHT:
                return DEATH_KNIGHT_TAB_BLOOD;
            case CLASS_DRUID:
                return DRUID_TAB_FERAL;
            default:
                break;
        }
    }
    else if (role == BOT_ROLE_HEALER)
    {
        switch (cls)
        {
            case CLASS_PRIEST:
                return PRIEST_TAB_HOLY;
            case CLASS_PALADIN:
                return PALADIN_TAB_HOLY;
            case CLASS_SHAMAN:
                return SHAMAN_TAB_RESTORATION;
            case CLASS_DRUID:
                return DRUID_TAB_RESTORATION;
            default:
                break;
        }
    }
    else
    {
        switch (cls)
        {
            case CLASS_WARRIOR:
                return urand(0, 1) ? WARRIOR_TAB_ARMS : WARRIOR_TAB_FURY;
            case CLASS_PALADIN:
                return PALADIN_TAB_RETRIBUTION;
            case CLASS_HUNTER:
                return urand(0, 2);
            case CLASS_ROGUE:
                return urand(0, 2);
            case CLASS_PRIEST:
                return PRIEST_TAB_SHADOW;
            case CLASS_DEATH_KNIGHT:
                return urand(0, 2);
            case CLASS_SHAMAN:
                return urand(0, 1) ? SHAMAN_TAB_ELEMENTAL : SHAMAN_TAB_ENHANCEMENT;
            case CLASS_MAGE:
                return urand(0, 2);
            case CLASS_WARLOCK:
                return urand(0, 2);
            case CLASS_DRUID:
                return urand(0, 1) ? DRUID_TAB_BALANCE : DRUID_TAB_FERAL;
            default:
                break;
        }
    }

    return 3;
}

void QuickPartyHelper::ApplyRoleTalents(Player* bot, BotRoles role)
{
    uint32 specTab = GetRoleSpecTab(bot->getClass(), role);
    uint8 cls = bot->getClass();

    if (specTab < 3)
    {
        uint32 specNo = sPlayerbotAIConfig.randomClassSpecIndex[cls][specTab];
        PlayerbotFactory::InitTalentsBySpecNo(bot, specNo, true);
    }
    else
    {
        PlayerbotFactory factory(bot, bot->GetLevel());
        factory.InitTalentsTree(false, true, true);
    }
}

void QuickPartyHelper::StripHeirloomItems(Player* player)
{
    for (uint8 slot = EQUIPMENT_SLOT_START; slot < INVENTORY_SLOT_ITEM_END; ++slot)
    {
        if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
        {
            if (ItemTemplate const* proto = item->GetTemplate())
            {
                if (proto->Quality == ITEM_QUALITY_HEIRLOOM)
                    player->DestroyItem(INVENTORY_SLOT_BAG_0, slot, true);
            }
        }
    }

    for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; ++bag)
    {
        if (Bag* pBag = player->GetBagByPos(bag))
        {
            for (uint32 slot = 0; slot < pBag->GetBagSize(); ++slot)
            {
                if (Item* item = pBag->GetItemByPos(slot))
                {
                    if (ItemTemplate const* proto = item->GetTemplate())
                    {
                        if (proto->Quality == ITEM_QUALITY_HEIRLOOM)
                            player->DestroyItem(bag, slot, true);
                    }
                }
            }
        }
    }
}

void QuickPartyHelper::PrepareBotForGroup(Player* bot, Player* player, BotRoles role)
{
    if (Group* group = bot->GetGroup())
        group->RemoveMember(bot->GetGUID());

    if (bot->isDead())
        bot->ResurrectPlayer(1.0f, false);
    bot->CombatStop(true);

    uint32 targetLevel = player->GetLevel() + urand(0, 2);
    if (targetLevel > DEFAULT_MAX_LEVEL)
        targetLevel = DEFAULT_MAX_LEVEL;

    if (bot->getClass() == CLASS_DEATH_KNIGHT &&
        targetLevel < sWorld->getIntConfig(CONFIG_START_HEROIC_PLAYER_LEVEL))
    {
        targetLevel = sWorld->getIntConfig(CONFIG_START_HEROIC_PLAYER_LEVEL);
    }

    bot->GiveLevel(targetLevel);
    bot->SetUInt32Value(PLAYER_XP, 0);

    ApplyRoleTalents(bot, role);

    uint32 quality = GearQualityForLevel(targetLevel);
    PlayerbotFactory factory(bot, targetLevel, quality);
    factory.InitEquipment(true);
    factory.InitAmmo();
    if (targetLevel >= sPlayerbotAIConfig.minEnchantingBotLevel)
        factory.ApplyEnchantAndGemsNew();
    bot->DurabilityRepairAll(false, 1.0f, false);

    bot->TeleportTo(player->GetMapId(), player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(),
                    player->GetOrientation());

    if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot))
    {
        botAI->SetMaster(player);
        botAI->ResetStrategies();
    }
}

bool QuickPartyHelper::AddBotToGroup(Player* player, Player* bot)
{
    GroupInviteOperation op(player->GetGUID(), bot->GetGUID());
    return op.Execute();
}

void QuickPartyHelper::CollectAvailableBots(Player* player, std::vector<Player*>& out, std::set<ObjectGuid> const& used)
{
    PlayerBotMap bots = sRandomPlayerbotMgr.GetAllBots();
    for (PlayerBotMap::const_iterator itr = bots.begin(); itr != bots.end(); ++itr)
    {
        Player* bot = itr->second;
        if (!bot || !bot->IsInWorld())
            continue;
        if (!sRandomPlayerbotMgr.IsRandomBot(bot))
            continue;
        if (used.count(bot->GetGUID()))
            continue;
        if (bot->GetTeamId(true) != player->GetTeamId(true))
            continue;
        if (bot->GetGroup())
            continue;
        if (bot->IsBeingTeleported())
            continue;
        if (bot->InBattleground() || bot->InBattlegroundQueue())
            continue;

        if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot))
        {
            if (botAI->HasActivePlayerMaster())
                continue;
        }

        out.push_back(bot);
    }

    ShuffleBots(out);
}

Player* QuickPartyHelper::PickBotForRole(std::vector<Player*>& candidates, BotRoles role,
                                           std::set<ObjectGuid>& used)
{
    for (Player* bot : candidates)
    {
        if (used.count(bot->GetGUID()))
            continue;
        if (!BotMatchesRole(bot, role))
            continue;
        if (!CanClassFillRole(bot->getClass(), role))
            continue;
        used.insert(bot->GetGUID());
        return bot;
    }

    for (Player* bot : candidates)
    {
        if (used.count(bot->GetGUID()))
            continue;
        if (!CanClassFillRole(bot->getClass(), role))
            continue;
        used.insert(bot->GetGUID());
        return bot;
    }

    for (Player* bot : candidates)
    {
        if (used.count(bot->GetGUID()))
            continue;
        if (role != BOT_ROLE_DPS)
            continue;
        used.insert(bot->GetGUID());
        return bot;
    }

    return nullptr;
}

std::vector<std::string> QuickPartyHelper::FormParty(Player* player, uint32 size)
{
    std::vector<std::string> messages;

    if (!player)
    {
        messages.push_back("无效玩家。");
        return messages;
    }

    if (player->InBattleground() || player->InBattlegroundQueue())
    {
        messages.push_back("你在战场中，无法快速组队。");
        return messages;
    }

    Group* group = player->GetGroup();
    if (group && group->GetLeaderGUID() != player->GetGUID())
    {
        messages.push_back("你不是队长，无法快速组队。");
        return messages;
    }

    uint32 currentCount = group ? group->GetMembersCount() : 1;
    if (currentCount >= size)
    {
        messages.push_back("队伍人数已满。");
        return messages;
    }

    std::unordered_map<BotRoles, uint32> roles;
    GetRoleRequirements(size, roles);
    CountExistingRoles(player, group, roles);

    uint32 botsNeeded = size - currentCount;
    uint32 rolesNeeded = roles[BOT_ROLE_TANK] + roles[BOT_ROLE_HEALER] + roles[BOT_ROLE_DPS];
    if (rolesNeeded > botsNeeded)
    {
        while (rolesNeeded > botsNeeded && roles[BOT_ROLE_DPS] > 0)
        {
            roles[BOT_ROLE_DPS]--;
            rolesNeeded--;
        }
    }

    std::vector<Player*> candidates;
    std::set<ObjectGuid> used;
    CollectAvailableBots(player, candidates, used);

    if (candidates.empty())
    {
        messages.push_back("没有可用的在线随机机器人。");
        return messages;
    }

    struct RoleSlot
    {
        BotRoles role;
    };
    std::vector<RoleSlot> slots;
    for (uint32 i = 0; i < roles[BOT_ROLE_TANK]; ++i)
        slots.push_back({BOT_ROLE_TANK});
    for (uint32 i = 0; i < roles[BOT_ROLE_HEALER]; ++i)
        slots.push_back({BOT_ROLE_HEALER});
    for (uint32 i = 0; i < roles[BOT_ROLE_DPS]; ++i)
        slots.push_back({BOT_ROLE_DPS});

    while (slots.size() > botsNeeded)
        slots.pop_back();
    while (slots.size() < botsNeeded)
        slots.push_back({BOT_ROLE_DPS});

    for (size_t i = slots.size(); i > 1; --i)
    {
        size_t j = urand(0, i - 1);
        std::swap(slots[i - 1], slots[j]);
    }

    if (size > 5 && player->GetGroup() && !player->GetGroup()->isRaidGroup())
    {
        GroupConvertToRaidOperation convertOp(player->GetGUID());
        convertOp.Execute();
    }

    uint32 added = 0;
    std::ostringstream summary;
    summary << "已添加机器人：";

    for (RoleSlot const& slot : slots)
    {
        Player* bot = PickBotForRole(candidates, slot.role, used);
        if (!bot)
            break;

        PrepareBotForGroup(bot, player, slot.role);

        if (AddBotToGroup(player, bot))
        {
            if (added)
                summary << "、";
            summary << bot->GetName();
            ++added;
        }
    }

    if (added == 0)
    {
        messages.push_back("未能找到符合职责的机器人。");
        return messages;
    }

    summary << "（共 " << added << " 人）";
    messages.push_back(summary.str());
    return messages;
}

std::vector<std::string> QuickPartyHelper::DisbandParty(Player* player)
{
    std::vector<std::string> messages;

    Group* group = player->GetGroup();
    if (!group)
    {
        messages.push_back("你不在队伍中。");
        return messages;
    }

    if (group->GetLeaderGUID() != player->GetGUID())
    {
        messages.push_back("你不是队长，无法解散队伍。");
        return messages;
    }

    group->Disband(true);
    messages.push_back("队伍已解散。");
    return messages;
}

std::vector<std::string> QuickPartyHelper::RandomGear(Player* invoker, Player* target)
{
    std::vector<std::string> messages;

    if (!target)
        target = invoker;

    if (target != invoker && !invoker->CanBeGameMaster() && !GET_PLAYERBOT_AI(target))
    {
        messages.push_back("只能对自己或机器人使用随机装备。");
        return messages;
    }

    if (target->isDead())
        target->ResurrectPlayer(1.0f, false);

    StripHeirloomItems(target);

    uint32 level = target->GetLevel();
    uint32 quality = GearQualityForLevel(level);

    PlayerbotFactory factory(target, level, quality);
    factory.SetExcludeHeirloom(true);
    factory.InitTalentsTree(true, true, true);
    factory.InitEquipment(true);
    factory.InitBags(true);
    factory.InitAmmo();
    if (level >= sPlayerbotAIConfig.minEnchantingBotLevel)
        factory.ApplyEnchantAndGemsNew();
    target->DurabilityRepairAll(false, 1.0f, false);

    messages.push_back("已为 " + target->GetName() + " 随机装备（含36格背包，不含传家宝）。");
    return messages;
}
