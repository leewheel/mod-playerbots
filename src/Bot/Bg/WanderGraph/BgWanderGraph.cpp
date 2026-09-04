/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 *
 * By leewheel 2026-09-03
 * 战场游走节点图实现 —— 移植自 NPCBots(Trickerer版) botwanderful.cpp 战场部分。
 * End By leewheel
 */

#include "BgWanderGraph.h"
#include "DatabaseEnv.h"
#include "Field.h"
#include "Log.h"
#include "QueryResult.h"
#include "Unit.h"

#include <algorithm>
#include <queue>
#include <unordered_set>

// By leewheel 2026-09-03 节点表加载
void BgWanderGraph::Load()
{
    uint32 oldMSTime = getMSTime();

    // 全表加载(仅154个战场节点, 一次读入)
    QueryResult result = WorldDatabase.Query("SELECT `id`,`name`,`mapid`,`zoneid`,`areaid`,`flags`,`x`,`y`,`z`,`o`,`links` FROM `playerbot_wander_nodes`");
    if (!result)
    {
        LOG_ERROR("playerbots", ">> 加载战场游走节点失败: 表 `playerbot_wander_nodes` 为空或不存在! 战场机器人将退化为旧点位策略。");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();
        BgWanderNode node;
        node.id = fields[0].Get<uint32>();
        node.mapId = fields[2].Get<uint32>();
        node.zoneId = fields[3].Get<uint32>();
        node.areaId = fields[4].Get<uint32>();
        node.flags = fields[5].Get<uint32>();
        node.pos.Relocate(fields[6].Get<float>(), fields[7].Get<float>(), fields[8].Get<float>(), fields[9].Get<float>());
        node.links = ParseLinks(fields[10].Get<std::string>());

        if (!node.links.empty() || node.flags)  // 孤立无标志节点无意义, 跳过
            _nodes[node.id] = std::move(node);
        ++count;
    } while (result->NextRow());

    // 构建指针邻接表(链向不存在节点的边剔除) + 地图索引
    for (auto& [id, node] : _nodes)
    {
        for (uint32 linkId : node.links)
        {
            auto itr = _nodes.find(linkId);
            if (itr != _nodes.end())
            {
                node.linkPtrs.push_back(&itr->second);
            }
            else
            {
                LOG_WARN("playerbots", ">> 战场游走节点 {}({}) 链接了不存在的节点 {}", id, node.pos.ToString(), linkId);
            }
        }
    }

    for (auto const& [id, node] : _nodes)
        _mapNodes[node.mapId].push_back(&node);

    for (auto& [mapId, nodes] : _mapNodes)
        std::sort(nodes.begin(), nodes.end(), [](BgWanderNode const* a, BgWanderNode const* b) { return a->id < b->id; });

    _loaded = true;
    LOG_INFO("playerbots", ">> 从 `playerbot_wander_nodes` 加载了 {} 个战场游走节点 ({} 个地图), 耗时 {} ms",
        count, _mapNodes.size(), GetMSTimeDiffToNow(oldMSTime));
}
// End By leewheel

// By leewheel 2026-09-03 links 字段解析("id:weight ..." 只取id, 权重战场用不到)
std::vector<uint32> BgWanderGraph::ParseLinks(std::string const& linksStr) const
{
    std::vector<uint32> out;
    std::string const& s = linksStr;
    std::size_t i = 0;
    while (i < s.size())
    {
        // 跳过空白
        while (i < s.size() && (s[i] == ' ' || s[i] == '\t'))
            ++i;
        if (i >= s.size())
            break;
        // 读数字
        std::size_t start = i;
        while (i < s.size() && s[i] >= '0' && s[i] <= '9')
            ++i;
        if (i == start)
        {
            ++i;    // 非数字非空白, 前进防死循环
            continue;
        }
        out.push_back(static_cast<uint32>(std::strtoul(s.substr(start, i - start).c_str(), nullptr, 10)));
        // 跳过 ":weight"
        if (i < s.size() && s[i] == ':')
        {
            ++i;
            while (i < s.size() && s[i] >= '0' && s[i] <= '9')
                ++i;
        }
    }
    return out;
}
// End By leewheel

std::vector<BgWanderNode const*> const& BgWanderGraph::GetMapNodes(uint32 mapId) const
{
    static std::vector<BgWanderNode const*> const empty;
    auto itr = _mapNodes.find(mapId);
    return itr != _mapNodes.end() ? itr->second : empty;
}

// By leewheel 2026-09-03 最近可通行节点(阵营过滤: A_ONLY/H_ONLY)
// z 参数保留以对齐 NPCBots GetClosestWanderNode 签名(战场节点均为地面点, 2D距离足够)
BgWanderNode const* BgWanderGraph::GetClosestNode(uint32 mapId, float x, float y, float /*z*/, TeamId team, float maxDist) const
{
    BgWanderNode const* best = nullptr;
    float bestDistSq = maxDist > 0.f ? maxDist * maxDist : std::numeric_limits<float>::max();
    for (BgWanderNode const* node : GetMapNodes(mapId))
    {
        // 阵营限定节点过滤
        if ((node->flags & BgWpFlags::ALLIANCE_ONLY) && team != TEAM_ALLIANCE)
            continue;
        if ((node->flags & BgWpFlags::HORDE_ONLY) && team != TEAM_HORDE)
            continue;

        float distSq = node->pos.GetExactDist2dSq(x, y);
        if (distSq < bestDistSq)
        {
            bestDistSq = distSq;
            best = node;
        }
    }
    return best;
}
// End By leewheel

bool BgWanderGraph::HasLink(BgWanderNode const* node, BgWanderNode const* other)
{
    if (!node || !other)
        return false;
    return std::find(node->linkPtrs.begin(), node->linkPtrs.end(), other) != node->linkPtrs.end();
}

// By leewheel 2026-09-03 BFS 最短路下一跳(节点数少, BFS 足够; 对应 NPCBots GetShortestPathLinks 语义)
// 返回 from 的邻居集合中, 位于 from->target 最短路径上的所有节点(等价跳返回唯一下一跳)
std::vector<BgWanderNode const*> BgWanderGraph::GetShortestPathNextLinks(BgWanderNode const* from, BgWanderNode const* target) const
{
    static std::vector<BgWanderNode const*> const empty;
    if (!from || !target || from == target)
        return empty;
    if (from->linkPtrs.empty())
        return empty;

    // BFS: 从 target 反向生长, 记录每个节点到 target 的最短距离
    std::unordered_map<BgWanderNode const*, uint32> distToTarget;
    std::queue<BgWanderNode const*> queue;
    distToTarget[target] = 0;
    queue.push(target);
    while (!queue.empty())
    {
        BgWanderNode const* cur = queue.front();
        queue.pop();
        uint32 nextDist = distToTarget[cur] + 1;
        for (BgWanderNode const* link : cur->linkPtrs)
        {
            auto itr = distToTarget.find(link);
            if (itr == distToTarget.end())
            {
                distToTarget[link] = nextDist;
                queue.push(link);
            }
        }
    }

    auto fromDist = distToTarget.find(from);
    if (fromDist == distToTarget.end())
        return empty;   // 不可达

    // from 的邻居中, 到 target 距离 = from距离-1 的即是最短路径下一跳
    uint32 need = fromDist->second - 1;
    std::vector<BgWanderNode const*> nextLinks;
    for (BgWanderNode const* link : from->linkPtrs)
    {
        auto itr = distToTarget.find(link);
        if (itr != distToTarget.end() && itr->second == need)
            nextLinks.push_back(link);
    }
    return nextLinks;
}
// End By leewheel

BgWanderNode const* BgWanderGraph::NextHopToward(BgWanderNode const* from, BgWanderNode const* target) const
{
    std::vector<BgWanderNode const*> const links = GetShortestPathNextLinks(from, target);
    if (links.empty())
        return nullptr;
    if (links.size() == 1u)
        return links.front();
    // 多条等价下一跳: NPCBots 用带权随机, 我们节点少直接随机取一
    return links[urand(0, static_cast<int32>(links.size()) - 1)];
}
