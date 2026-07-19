/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "InventoryChangeFailureAction.h"

#include "Event.h"
#include "Playerbots.h"

std::map<InventoryResult, std::string> InventoryChangeFailureAction::messages;

bool InventoryChangeFailureAction::Execute(Event event)
{
    if (!botAI->GetMaster())
        return false;

    if (messages.empty())
    {
        messages[EQUIP_ERR_CANT_EQUIP_LEVEL_I] = "我的等级太低";
        messages[EQUIP_ERR_CANT_EQUIP_SKILL] = "我的技能等级太低";
        messages[EQUIP_ERR_ITEM_DOESNT_GO_TO_SLOT] = "无效栏位";
        messages[EQUIP_ERR_BAG_FULL] = "背包已满";
        messages[EQUIP_ERR_NONEMPTY_BAG_OVER_OTHER_BAG] = "此背包不为空";
        messages[EQUIP_ERR_CANT_TRADE_EQUIP_BAGS] = "无法交易已装备的背包";
        messages[EQUIP_ERR_ONLY_AMMO_CAN_GO_HERE] = "无效栏位（此处只能放弹药）";
        messages[EQUIP_ERR_NO_REQUIRED_PROFICIENCY] = "我没有所需技能";
        messages[EQUIP_ERR_NO_EQUIPMENT_SLOT_AVAILABLE] = "没有可用装备栏";
        messages[EQUIP_ERR_YOU_CAN_NEVER_USE_THAT_ITEM] = "我永远无法使用此物品";
        messages[EQUIP_ERR_YOU_CAN_NEVER_USE_THAT_ITEM2] = "我永远无法使用此物品";
        messages[EQUIP_ERR_NO_EQUIPMENT_SLOT_AVAILABLE2] = messages[EQUIP_ERR_NO_EQUIPMENT_SLOT_AVAILABLE];
        messages[EQUIP_ERR_CANT_EQUIP_WITH_TWOHANDED] = "已装备双手武器，无法装备";
        messages[EQUIP_ERR_CANT_DUAL_WIELD] = "我无法双持";
        messages[EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG] = "此物品不能放入该背包";
        messages[EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG2] = "此物品不能放入该背包";
        messages[EQUIP_ERR_CANT_CARRY_MORE_OF_THIS] = "无法再携带更多该物品";
        messages[EQUIP_ERR_NO_EQUIPMENT_SLOT_AVAILABLE3] = messages[EQUIP_ERR_NO_EQUIPMENT_SLOT_AVAILABLE];
        messages[EQUIP_ERR_ITEM_CANT_STACK] = "物品无法堆叠";
        messages[EQUIP_ERR_ITEM_CANT_BE_EQUIPPED] = "物品无法装备";
        messages[EQUIP_ERR_ITEMS_CANT_BE_SWAPPED] = "无法交换这些物品";
        messages[EQUIP_ERR_SLOT_IS_EMPTY] = "没有可装备的物品";
        messages[EQUIP_ERR_ITEM_NOT_FOUND] = "找不到该物品";
        messages[EQUIP_ERR_CANT_DROP_SOULBOUND] = "无法丢弃灵魂绑定物品";
        messages[EQUIP_ERR_OUT_OF_RANGE] = "我距离太远";
        messages[EQUIP_ERR_TRIED_TO_SPLIT_MORE_THAN_COUNT] = "无效拆分数量";
        messages[EQUIP_ERR_COULDNT_SPLIT_ITEMS] = "无法拆分";
        messages[EQUIP_ERR_MISSING_REAGENT] = "缺少材料";
        messages[EQUIP_ERR_NOT_ENOUGH_MONEY] = "金币不足";
        messages[EQUIP_ERR_NOT_A_BAG] = "这不是背包";
        messages[EQUIP_ERR_CAN_ONLY_DO_WITH_EMPTY_BAGS] = "背包不为空";
        messages[EQUIP_ERR_DONT_OWN_THAT_ITEM] = "这不是我的物品";
        messages[EQUIP_ERR_CAN_EQUIP_ONLY1_QUIVER] = "只能装备箭袋";
        messages[EQUIP_ERR_MUST_PURCHASE_THAT_BAG_SLOT] = "必须先购买栏位";
        messages[EQUIP_ERR_TOO_FAR_AWAY_FROM_BANK] = "离银行太远";
        messages[EQUIP_ERR_ITEM_LOCKED] = "物品已锁定";
        messages[EQUIP_ERR_YOU_ARE_STUNNED] = "我被眩晕了";
        messages[EQUIP_ERR_YOU_ARE_DEAD] = "我已死亡";
        messages[EQUIP_ERR_CANT_DO_RIGHT_NOW] = "现在无法操作";
        messages[EQUIP_ERR_INT_BAG_ERROR] = "内部错误";
        messages[EQUIP_ERR_CAN_EQUIP_ONLY1_BOLT] = "只能放 bolts";
        messages[EQUIP_ERR_CAN_EQUIP_ONLY1_AMMOPOUCH] = "只能放弹药袋";
        messages[EQUIP_ERR_STACKABLE_CANT_BE_WRAPPED] = "物品无法包装";
        messages[EQUIP_ERR_EQUIPPED_CANT_BE_WRAPPED] = messages[EQUIP_ERR_STACKABLE_CANT_BE_WRAPPED];
        messages[EQUIP_ERR_WRAPPED_CANT_BE_WRAPPED] = messages[EQUIP_ERR_STACKABLE_CANT_BE_WRAPPED];
        messages[EQUIP_ERR_BOUND_CANT_BE_WRAPPED] = messages[EQUIP_ERR_STACKABLE_CANT_BE_WRAPPED];
        messages[EQUIP_ERR_UNIQUE_CANT_BE_WRAPPED] = messages[EQUIP_ERR_STACKABLE_CANT_BE_WRAPPED];
        messages[EQUIP_ERR_BAGS_CANT_BE_WRAPPED] = messages[EQUIP_ERR_STACKABLE_CANT_BE_WRAPPED];
        messages[EQUIP_ERR_ALREADY_LOOTED] = "已经拾取过";
        messages[EQUIP_ERR_INVENTORY_FULL] = "背包已满";
        messages[EQUIP_ERR_BANK_FULL] = "银行已满";
        messages[EQUIP_ERR_ITEM_IS_CURRENTLY_SOLD_OUT] = "物品已售罄";
        messages[EQUIP_ERR_BAG_FULL3] = messages[EQUIP_ERR_BANK_FULL];
        messages[EQUIP_ERR_ITEM_NOT_FOUND2] = messages[EQUIP_ERR_ITEM_NOT_FOUND];
        messages[EQUIP_ERR_ITEM_CANT_STACK2] = messages[EQUIP_ERR_ITEM_CANT_STACK];
        messages[EQUIP_ERR_BAG_FULL4] = messages[EQUIP_ERR_BAG_FULL];
        messages[EQUIP_ERR_ITEM_SOLD_OUT] = messages[EQUIP_ERR_ITEM_IS_CURRENTLY_SOLD_OUT];
        messages[EQUIP_ERR_OBJECT_IS_BUSY] = "该对象正忙";
        messages[EQUIP_ERR_NOT_IN_COMBAT] = "我正在战斗中";
        messages[EQUIP_ERR_NOT_WHILE_DISARMED] = "缴械状态下无法操作";
        messages[EQUIP_ERR_BAG_FULL6] = messages[EQUIP_ERR_BAG_FULL];
        messages[EQUIP_ERR_CANT_EQUIP_RANK] = "军衔不足";
        messages[EQUIP_ERR_CANT_EQUIP_REPUTATION] = "声望不足";
        messages[EQUIP_ERR_TOO_MANY_SPECIAL_BAGS] = "特殊背包过多";
        messages[EQUIP_ERR_LOOT_CANT_LOOT_THAT_NOW] = "现在无法拾取";
    }

    WorldPacket p(event.getPacket());
    p.rpos(0);
    uint8 err;
    p >> err;
    if (err == EQUIP_ERR_OK)
        return false;

    std::string const msg = messages[(InventoryResult)err];
    if (!msg.empty())
    {
        botAI->TellError(msg);
        return true;
    }

    return false;
}
