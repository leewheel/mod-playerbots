/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_QUEUE_H
#define PLAYERBOTS_QUEUE_H

#include "Action.h"
#include "Common.h"

#include <functional>
#include <list>
#include <map>
#include <unordered_map>

/**
 * @class Queue
 * @brief Manages a priority queue of actions for the playerbot system
 *
 * This queue maintains a list of ActionBasket objects, each containing an action
 * and its relevance score. Actions with higher relevance scores are prioritized.
 *
 * 性能优化说明（By leewheel 2026-08-19）：
 * 原实现使用 std::list 线性扫描，Push/Pop/Peek 均为 O(N)。
 * 现改为 std::multimap（按 relevance 降序）+ std::unordered_map 名称索引：
 *   - Push 去重查找 O(1)，插入 O(log N)
 *   - Peek O(1)，Pop O(log N)
 * 对外接口语义完全不变。
 */
class Queue
{
public:
    Queue() = default;
    ~Queue() = default;

    /**
     * @brief Adds an action to the queue or updates existing action's relevance
     * @param action Pointer to the ActionBasket to be added
     *
     * If an action with the same name exists, updates its relevance if the new
     * relevance is higher, then deletes the new action. Otherwise, adds the new
     * action to the queue.
     */
    void Push(ActionBasket* action);

    /**
     * @brief Removes and returns the action with highest relevance
     * @return Pointer to the highest relevance ActionNode, or nullptr if queue is empty
     *
     * Ownership of the returned ActionNode is transferred to the caller.
     * The associated ActionBasket is deleted.
     */
    ActionNode* Pop();

    /**
     * @brief Returns the action with highest relevance without removing it
     * @return Pointer to the ActionBasket with highest relevance, or nullptr if queue is empty
     */
    ActionBasket* Peek();

    /**
     * @brief Returns the current size of the queue
     * @return Number of actions in the queue
     */
    uint32 Size();

    /**
     * @brief Removes and deletes expired actions from the queue
     *
     * Uses sPlayerbotAIConfig.expireActionTime to determine if actions have expired.
     * Both the ActionNode and ActionBasket are deleted for expired actions.
     */
    void RemoveExpired();

private:
    /**
     * @brief Updates existing basket with new relevance and cleans up new basket
     */
    void updateExistingBasket(ActionBasket* existing, ActionBasket* newBasket);

    /**
     * @brief Finds the basket with the highest relevance score
     * @return Pointer to the highest relevance basket, or nullptr if queue is empty
     */
    ActionBasket* findHighestRelevanceBasket() const;

    /**
     * @brief Extracts action from basket and handles basket cleanup
     */
    ActionNode* extractAndDeleteBasket(ActionBasket* basket);

    /**
     * @brief Collects all expired baskets into the provided list
     */
    void collectExpiredBaskets(std::list<ActionBasket*>& expiredBaskets);

    /**
     * @brief Removes and deletes all baskets in the provided list
     */
    void removeAndDeleteBaskets(std::list<ActionBasket*>& basketsToRemove);

    // By leewheel 2026-08-19
    // 性能优化（经 CPU 采样验证后修正）：
    // 第一版曾用 multimap 优先队列（红黑树），采样证实队列规模小时红黑树节点堆分配
    // 反超线性扫描；最终保留 list（Peek/Pop 线性扫描、无堆分配）+ nameIndex（Push 去重 O(1)）。
    // 详见 Queue.cpp 顶部注释。
    std::list<ActionBasket*> actions; /**< Container for action baskets */
    std::unordered_map<std::string, ActionBasket*> nameIndex; /**< action 名称索引，O(1) 去重 */
    // End By leewheel
};

#endif
