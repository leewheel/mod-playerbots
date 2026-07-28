#ifndef PLAYERBOTS_RAIDBOSSHELPERS_H
#define PLAYERBOTS_RAIDBOSSHELPERS_H

#include "AiObject.h"
#include "Unit.h"

//By leewheel 2026-07-28 - 同步brighton-chi/mod-playerbots：所有Mark*函数返回bool(标记是否发生变化),
//                        供TK等模块在 && 短路中判断是否成功设置RTI图标
//                        GetNearestPlayerInRadius 返回类型从 Unit* 改为 Player*，
//                        避免调用处再做 ToPlayer 转换（brighton-chi 直接返回 Player*）
//End By leewheel
bool MarkTargetWithIcon(Player* bot, Unit* target, uint8 iconId);
bool MarkTargetWithSkull(Player* bot, Unit* target);
bool MarkTargetWithSquare(Player* bot, Unit* target);
bool MarkTargetWithStar(Player* bot, Unit* target);
bool MarkTargetWithCircle(Player* bot, Unit* target);
bool MarkTargetWithDiamond(Player* bot, Unit* target);
bool MarkTargetWithTriangle(Player* bot, Unit* target);
bool MarkTargetWithCross(Player* bot, Unit* target);
bool MarkTargetWithMoon(Player* bot, Unit* target);
bool ClearTargetIcon(Player* bot, uint8 iconId);
void SetRtiTarget(PlayerbotAI* botAI, const std::string& rtiName, Unit* target);
bool IsMechanicTrackerBot(PlayerbotAI* botAI, Player* bot, uint32 mapId, Player* exclude = nullptr);
Player* GetGroupMainTank(PlayerbotAI* botAI, Player* bot);
Player* GetGroupAssistTank(PlayerbotAI* botAI, Player* bot, uint8 index);
Unit* GetFirstAliveUnitByEntry(
    PlayerbotAI* botAI, uint32 entry);
Player* GetNearestPlayerInRadius(Player* bot, float radius);

#endif
