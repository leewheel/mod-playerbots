/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 *
 * By leewheel 2026-09-03
 * 战场游走节点图子系统 —— 移植自 NPCBots(Trickerer版) botwanderful 战场部分。
 *   数据来源: playerbot_wander_nodes 表(数据导自 NPCBots 官方 154 个战场节点)。
 *   职责: 加载节点/邻接表, 提供"最短路径下一跳"决策(NPCBots GetShortestPathLinks 等价物),
 *         供 BGTactics 的战场策略(WSG/AV/AB/EY)按 NPCBots 决策内核选用节点。
 *   与 NPCBots 的差异: 我们只服务战场(不做野外漫游), 节点数少(154), BFS 全量算最短路足够快。
 * End By leewheel
 */

#ifndef PLAYERBOTS_BG_WANDER_GRAPH_H
#define PLAYERBOTS_BG_WANDER_GRAPH_H

#include "Define.h"
#include "Position.h"
#include "SharedDefines.h"

#include <unordered_map>
#include <vector>
#include <cstdint>

class Player;
class Unit;

// By leewheel 2026-09-03 节点标志位 —— 与 NPCBots botwanderful.h BotWPFlags 对齐(仅保留战场用到的)
namespace BgWpFlags
{
    constexpr uint32 BG_FLAG_DELIVER_TARGET = 0x00000040;   // 旗手交旗目标点
    constexpr uint32 BG_FLAG_PICKUP_TARGET = 0x00000080;    // 拾旗目标点(双方旗架)
    constexpr uint32 BG_MISC_OBJECTIVE_1 = 0x00000200;      // AV=矿洞, EY=出生点拦截位
    constexpr uint32 BG_MISC_OBJECTIVE_2 = 0x00000400;      // AV=双方BOSS房
    constexpr uint32 BG_OPTIONAL_PICKUP_1 = 0x00000800;     // WS=恢复神水NW / AB=兽栏 / EY=神水NW
    constexpr uint32 BG_OPTIONAL_PICKUP_2 = 0x00001000;     // WS=狂暴神水NE / AB=农场 / EY=神水NE
    constexpr uint32 BG_OPTIONAL_PICKUP_3 = 0x00002000;     // WS=恢复神水SE / AB=磨坊 / EY=神水SW
    constexpr uint32 BG_OPTIONAL_PICKUP_4 = 0x00004000;     // WS=狂暴神水SW / AB=矿洞 / EY=神水SE
    constexpr uint32 BG_OPTIONAL_PICKUP_5 = 0x00008000;     // AB=铁匠铺

    constexpr uint32 ALLIANCE_ONLY = 0x00000002;            // 仅联盟机器人可通行
    constexpr uint32 HORDE_ONLY = 0x00000004;               // 仅部落机器人可通行
    constexpr uint32 SPAWN = 0x00000001;                    // 出生点(阵营出生=旗点)

    // 组合标志(同 NPCBots)
    constexpr uint32 ALLIANCE_SPAWN_POINT = SPAWN | ALLIANCE_ONLY;
    constexpr uint32 HORDE_SPAWN_POINT = SPAWN | HORDE_ONLY;
    constexpr uint32 ALLIANCE_FLAG_DELIVER_TARGET = BG_FLAG_DELIVER_TARGET | ALLIANCE_ONLY;
    constexpr uint32 HORDE_FLAG_DELIVER_TARGET = BG_FLAG_DELIVER_TARGET | HORDE_ONLY;
    constexpr uint32 ALLIANCE_FLAG_PICKUP_TARGET = BG_FLAG_PICKUP_TARGET | ALLIANCE_ONLY;
    constexpr uint32 HORDE_FLAG_PICKUP_TARGET = BG_FLAG_PICKUP_TARGET | HORDE_ONLY;
    constexpr uint32 ALLIANCE_BOSS_ROOM = BG_MISC_OBJECTIVE_2 | ALLIANCE_ONLY;
    constexpr uint32 HORDE_BOSS_ROOM = BG_MISC_OBJECTIVE_2 | HORDE_ONLY;
    constexpr uint32 ALLIANCE_SPAWN_INTERCEPT = BG_MISC_OBJECTIVE_1 | ALLIANCE_ONLY;
    constexpr uint32 HORDE_SPAWN_INTERCEPT = BG_MISC_OBJECTIVE_1 | HORDE_ONLY;
    constexpr uint32 OPTIONAL_PICKUP_MASK = BG_OPTIONAL_PICKUP_1 | BG_OPTIONAL_PICKUP_2 | BG_OPTIONAL_PICKUP_3 |
        BG_OPTIONAL_PICKUP_4 | BG_OPTIONAL_PICKUP_5;
    constexpr uint32 WS_PICKUP_RESTORATION = BG_OPTIONAL_PICKUP_1 | BG_OPTIONAL_PICKUP_3;
    constexpr uint32 WS_PICKUP_BERSERKING = BG_OPTIONAL_PICKUP_2 | BG_OPTIONAL_PICKUP_4;
}
// End By leewheel

// By leewheel 2026-09-03 战场游走节点结构
struct BgWanderNode
{
    uint32 id = 0;
    uint32 mapId = 0;
    uint32 zoneId = 0;
    uint32 areaId = 0;
    uint32 flags = 0;
    Position pos;                                   // 节点坐标(含朝向)
    std::vector<uint32> links;                      // 邻接节点ID表
    std::vector<BgWanderNode const*> linkPtrs;      // 邻接节点指针(图构建完成后填充)
};
// End By leewheel

// By leewheel 2026-09-03 战场游走节点图(单例, 服务器启动时从 playerbot_wander_nodes 加载)
class BgWanderGraph
{
public:
    static BgWanderGraph* instance()
    {
        static BgWanderGraph instance;
        return &instance;
    }

    // 从 WorldDatabase.playerbot_wander_nodes 加载并构建邻接图
    void Load();

    bool IsLoaded() const { return _loaded; }

    // 取地图全部节点(无则返回空表)
    std::vector<BgWanderNode const*> const& GetMapNodes(uint32 mapId) const;

    // 按谓词查找地图内首个匹配节点(对应 NPCBots WanderNode::FindInMapWPs(pred))
    template <typename Pred>
    BgWanderNode const* FindInMapWPs(uint32 mapId, Pred pred) const
    {
        for (BgWanderNode const* node : GetMapNodes(mapId))
            if (pred(node))
                return node;
        return nullptr;
    }

    // 遍历地图内全部节点(对应 NPCBots WanderNode::DoForAllMapWPs)
    template <typename Func>
    void DoForAllMapWPs(uint32 mapId, Func func) const
    {
        for (BgWanderNode const* node : GetMapNodes(mapId))
            func(node);
    }

    // 距坐标最近的可通行节点(阵营过滤; 对应 NPCBots GetClosestWanderNode)
    // maxDist < 0 表示不限制距离
    BgWanderNode const* GetClosestNode(uint32 mapId, float x, float y, float z, TeamId team, float maxDist = -1.f) const;

    // 最短路径下一跳: 从 from 出发走向 target 的相邻节点(NPCBots GetShortestPathLinks 等价物)。
    // 返回 from 的直接邻居中位于最短路径上的那个节点集合(通常1个, 等价跳多时返回唯一下一跳)。
    // from 为 nullptr 时返回空表。
    std::vector<BgWanderNode const*> GetShortestPathNextLinks(BgWanderNode const* from, BgWanderNode const* target) const;

    // 与 NPCBots 语义一致: 返回 from->target 最短路径上的下一跳(单点便捷版, 无路返回 nullptr)
    BgWanderNode const* NextHopToward(BgWanderNode const* from, BgWanderNode const* target) const;

    // 两节点是否直接相邻
    static bool HasLink(BgWanderNode const* node, BgWanderNode const* other);

private:
    BgWanderGraph() = default;
    ~BgWanderGraph() = default;
    BgWanderGraph(BgWanderGraph const&) = delete;
    BgWanderGraph& operator=(BgWanderGraph const&) = delete;

    // 解析 links 字段(格式: "id:weight id:weight ...")
    std::vector<uint32> ParseLinks(std::string const& linksStr) const;

    std::unordered_map<uint32, BgWanderNode> _nodes;                    // id -> node
    std::unordered_map<uint32, std::vector<BgWanderNode const*>> _mapNodes;  // mapId -> nodes(按id排序)
    bool _loaded = false;
};

// End By leewheel
#endif // PLAYERBOTS_BG_WANDER_GRAPH_H
