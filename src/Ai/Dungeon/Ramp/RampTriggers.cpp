/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RampTriggers.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "RampBossHelper.h"
#include "RampShared.h"

using namespace EncounterHelpers;
using namespace RampShared;

// By leewheel 2026-09-19 合并上游 the-lab（2b15a2fb..799274b4）：上游把本模块的 HFR/（地狱火城墙，
//   我方已在提交 31446315 完成 entry 化）整体删除并重写为同目录 Ramp/；重写时 Boss 名称又被写回
//   英文字符串，我方 entry 化成果随之丢失。按项目规则第 97 条（机器人策略里的 Boss 名称一律用
//   entry，不得依赖客户端本地化名称，否则中文客户端下 find target 永远命中不到），此处把本文件
//   全部 "find target" 参数补回 entry。映射取自被我方 entry 化过的旧文件
//   modules/mod-playerbots/src/Ai/Dungeon/HFR/HFR*.cpp（提交 31446315）：
//     "hellfire watcher"   => 17309   （原 HFR/HFRActions.cpp:21）
//     "fiendish hound"     => 17280   （原 HFR/HFRActions.cpp:60）
//     "omor the unscarred" => 17308   （原 HFR/HFRMultipliers.cpp:21）
//     "vazruden"           => 17537   （原 HFR/HFRActions.cpp:96）
//     "nazan"              => 17536   （原 HFR/HFRMultipliers.cpp:67）
//   注意：本文件里的 "tremor totem" / "fire resistance totem" / "fire resistance aura" / "has totem"
//   是【法术名与 AI 关键字】，不是 Boss 名，必须保持英文，不得替换。
// End By leewheel

// Watchkeeper Gargolmar

bool GargolmarHellfireWatchersAreActiveTrigger::IsActive()
{
    return  IsMechanicTrackerBot(bot, RAMP_MAP_ID) &&
            AI_VALUE2(Unit*, "find target", "17309");
}

// Omor the Unscarred

bool OmorTreacheryAuraTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "17308"))
        return false;

    if (!helper.HasTreacheryAura(bot))
        return false;

    return !PlayerbotAI::IsMainTank(bot);
}

bool OmorTankHasTreacheryAuraTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "17308"))
        return false;

    Player* tank = GetGroupMainTank(bot);

    if (!tank || tank == bot)
        return false;

    return helper.HasTreacheryAura(tank);
}

bool OmorRangedSpreadTrigger::IsActive()
{
    return  PlayerbotAI::IsRanged(bot) &&
            GetNearestPlayerInRadius(bot, OMOR_TREACHERY_AURA_SAFE_DISTANCE) &&
            AI_VALUE2(Unit*, "find target", "17308");
}

bool OmorFiendishHoundIsActiveTrigger::IsActive()
{
    return  IsMechanicTrackerBot(bot, RAMP_MAP_ID) &&
            AI_VALUE2(Unit*, "find target", "17280");
}

// Vazruden & Nazan

bool VazrudenTankPositionBossTrigger::IsActive()
{
    Unit* nazan = AI_VALUE2(Unit*, "find target", "17536");

    return
        PlayerbotAI::IsMainTank(bot) &&
        AI_VALUE2(Unit*, "find target", "17537") &&
        (!nazan || nazan->IsFlying());
}

bool VazrudenBossIsActiveTrigger::IsActive()
{
    return  IsMechanicTrackerBot(bot, RAMP_MAP_ID) &&
            AI_VALUE2(Unit*, "find target", "17537");
}

bool NazanBossTremorTotemTrigger::IsActive()
{
    if (bot->getClass() != CLASS_SHAMAN)
        return false;

    Unit* nazan = AI_VALUE2(Unit*, "find target", "17536");

    if (!nazan || nazan->IsFlying())
        return false;

    Map* map = nazan->GetMap();
    if (!map || !map->IsHeroic())
        return false;

    return !AI_VALUE2(bool, "has totem", "tremor totem");
}

bool NazanBossFireResistanceTotemTrigger::IsActive()
{
    if (bot->getClass() != CLASS_SHAMAN)
        return false;

    Unit* nazan = AI_VALUE2(Unit*, "find target", "17536");

    if (!nazan || nazan->IsFlying())
        return false;

    return !AI_VALUE2(bool, "has totem", "fire resistance totem");
}

bool NazanBossFireResistanceAuraTrigger::IsActive()
{
    if (bot->getClass() != CLASS_PALADIN)
        return false;

    Unit* nazan = AI_VALUE2(Unit*, "find target", "17536");

    if (!nazan || nazan->IsFlying())
        return false;

    return !botAI->HasAura("fire resistance aura", bot);
}
