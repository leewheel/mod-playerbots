/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "Queue.h"
#include "Log.h"
#include "PlayerbotAIConfig.h"

// By leewheel 2026-08-19
// 性能优化（经 CPU 采样验证后修正）：
// 第一版优化曾将容器改为 multimap 优先队列（红黑树），但实际采样（xperf ETW）证实：
// 队列规模小（通常 10-50 个 action）时，红黑树每次 emplace 的节点堆分配 + erase 释放，
// 开销高于原 list 线性扫描（无堆分配），且 ntdll 堆分配/释放成为前十大 CPU 热点。
// 最终方案：list 保持原线性扫描（Peek/Pop 无堆分配）+ unordered_map 名称索引（Push 去重 O(1)）。
// 对外接口语义与原始实现完全一致。
// End By leewheel

void Queue::Push(ActionBasket* action)
{
    if (!action)
    {
        return;
    }

    // 名称去重：O(1) 哈希查找，替代原 O(N) 线性扫描（字符串比较）
    std::string const actionName = action->getAction()->getName();
    std::unordered_map<std::string, ActionBasket*>::iterator it = nameIndex.find(actionName);
    if (it != nameIndex.end())
    {
        updateExistingBasket(it->second, action);
        return;
    }

    actions.push_back(action);
    nameIndex[actionName] = action;
}

ActionNode* Queue::Pop()
{
    ActionBasket* highestRelevanceBasket = findHighestRelevanceBasket();
    if (!highestRelevanceBasket)
    {
        return nullptr;
    }

    return extractAndDeleteBasket(highestRelevanceBasket);
}

ActionBasket* Queue::Peek()
{
    return findHighestRelevanceBasket();
}

uint32 Queue::Size()
{
    return actions.size();
}

void Queue::RemoveExpired()
{
    if (!sPlayerbotAIConfig.expireActionTime)
    {
        return;
    }

    std::list<ActionBasket*> expiredBaskets;
    collectExpiredBaskets(expiredBaskets);
    removeAndDeleteBaskets(expiredBaskets);
}

// Private helper methods
void Queue::updateExistingBasket(ActionBasket* existing, ActionBasket* newBasket)
{
    if (existing->getRelevance() < newBasket->getRelevance())
    {
        existing->setRelevance(newBasket->getRelevance());
    }

    if (ActionNode* actionNode = newBasket->getAction())
    {
        delete actionNode;
    }

    delete newBasket;
}

ActionBasket* Queue::findHighestRelevanceBasket() const
{
    if (actions.empty())
    {
        return nullptr;
    }

    float maxRelevance = -1.0f;
    ActionBasket* selection = nullptr;

    for (ActionBasket* basket : actions)
    {
        if (!basket)
        {
            continue;
        }

        if (basket->getRelevance() > maxRelevance)
        {
            maxRelevance = basket->getRelevance();
            selection = basket;
        }
    }

    return selection;
}

ActionNode* Queue::extractAndDeleteBasket(ActionBasket* basket)
{
    ActionNode* action = basket->getAction();
    actions.remove(basket);

    // 同步从名称索引移除，保持与容器一致性
    nameIndex.erase(action->getName());

    delete basket;
    return action;
}

void Queue::collectExpiredBaskets(std::list<ActionBasket*>& expiredBaskets)
{
    uint32 expiryTime = sPlayerbotAIConfig.expireActionTime;
    for (ActionBasket* basket : actions)
    {
        if (basket->isExpired(expiryTime))
        {
            expiredBaskets.push_back(basket);
        }
    }
}

void Queue::removeAndDeleteBaskets(std::list<ActionBasket*>& basketsToRemove)
{
    for (ActionBasket* basket : basketsToRemove)
    {
        actions.remove(basket);

        // 同步从名称索引移除，保持与容器一致性
        nameIndex.erase(basket->getAction()->getName());

        if (ActionNode* action = basket->getAction())
        {
            delete action;
        }

        delete basket;
    }
}
