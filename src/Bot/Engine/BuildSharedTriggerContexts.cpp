/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AiObjectContext.h"
#include "TriggerContext.h"
#include "ChatTriggerContext.h"
#include "WorldPacketTriggerContext.h"
#include "Aq20TriggerContext.h"
#include "MCTriggerContext.h"
#include "BWLTriggerContext.h"
#include "KaraTriggerContext.h"
#include "GruulTriggerContext.h"
#include "MagTriggerContext.h"
#include "NaxxTriggerContext.h"
#include "SSCTriggerContext.h"
#include "TKTriggerContext.h"
#include "HyjalTriggerContext.h"
#include "BTTriggerContext.h"
#include "ZATriggerContext.h"
#include "OSTriggerContext.h"
#include "EoETriggerContext.h"
#include "VoATriggerContext.h"
#include "UldTriggerContext.h"
#include "OnyTriggerContext.h"
#include "ICCTriggerContext.h"
#include "RSTriggerContext.h"
//By leewheel 2026-07-08
#include "SWPTriggerContext.h"
//End By leewheel
//By leewheel 2026-07-09
#include "VimgolTriggerContext.h"
//End By leewheel
//By leewheel 2026-07-11
#include "VanillaNaxxTriggerContext.h"
//End By leewheel
#include "Ai/Dungeon/TbcDungeonTriggerContext.h"
#include "Ai/Dungeon/WotlkDungeonTriggerContext.h"

void AiObjectContext::BuildSharedTriggerContexts(SharedNamedObjectContextList<Trigger>& triggerContexts)
{
    triggerContexts.Add(new TriggerContext());
    triggerContexts.Add(new ChatTriggerContext());
    triggerContexts.Add(new WorldPacketTriggerContext());
    triggerContexts.Add(new RaidAq20TriggerContext());
    triggerContexts.Add(new RaidMcTriggerContext());
    triggerContexts.Add(new RaidBwlTriggerContext());
    triggerContexts.Add(new RaidKarazhanTriggerContext());
    triggerContexts.Add(new RaidGruulsLairTriggerContext());
    triggerContexts.Add(new RaidMagtheridonTriggerContext());
    triggerContexts.Add(new RaidNaxxTriggerContext());
    triggerContexts.Add(new RaidSSCTriggerContext());
    triggerContexts.Add(new RaidTempestKeepTriggerContext());
    triggerContexts.Add(new RaidHyjalSummitTriggerContext());
    triggerContexts.Add(new RaidBlackTempleTriggerContext());
    triggerContexts.Add(new RaidZulAmanTriggerContext());
    triggerContexts.Add(new RaidOsTriggerContext());
    triggerContexts.Add(new RaidEoETriggerContext());
    triggerContexts.Add(new RaidVoATriggerContext());
    triggerContexts.Add(new RaidUlduarTriggerContext());
    triggerContexts.Add(new RaidOnyxiaTriggerContext());
    triggerContexts.Add(new RaidIccTriggerContext());
    triggerContexts.Add(new RaidRsTriggerContext());
    //By leewheel 2026-07-08
    //By leewheel 2026-07-26 替换为brighton-chi版本SWP实现(类名RaidSunwellTriggerContext)
    triggerContexts.Add(new RaidSunwellTriggerContext());
    //End By leewheel
    //By leewheel 2026-07-09
    triggerContexts.Add(new VimgolTriggerContext());
    //End By leewheel
    //By leewheel 2026-07-11
    triggerContexts.Add(new RaidVanillaNaxxTriggerContext());
    //End By leewheel
    triggerContexts.Add(new TbcDungeonHellfireRampartsTriggerContext());
    //By leewheel 2026-07-27 引入brighton-chi的UB(幽暗沼泽)副本策略
    triggerContexts.Add(new TbcDungeonUnderbogTriggerContext());
    //End By leewheel
    triggerContexts.Add(new TbcDungeonAuchenaiCryptsTriggerContext());
    triggerContexts.Add(new TbcDungeonSethekkHallsTriggerContext());
    triggerContexts.Add(new TbcDungeonMechanarTriggerContext());
    triggerContexts.Add(new WotlkDungeonUKTriggerContext());
    triggerContexts.Add(new WotlkDungeonNexTriggerContext());
    triggerContexts.Add(new WotlkDungeonANTriggerContext());
    triggerContexts.Add(new WotlkDungeonOKTriggerContext());
    triggerContexts.Add(new WotlkDungeonDTKTriggerContext());
    triggerContexts.Add(new WotlkDungeonVHTriggerContext());
    triggerContexts.Add(new WotlkDungeonGDTriggerContext());
    triggerContexts.Add(new WotlkDungeonHoSTriggerContext());
    triggerContexts.Add(new WotlkDungeonHoLTriggerContext());
    triggerContexts.Add(new WotlkDungeonOccTriggerContext());
    triggerContexts.Add(new WotlkDungeonUPTriggerContext());
    triggerContexts.Add(new WotlkDungeonCoSTriggerContext());
    triggerContexts.Add(new WotlkDungeonFoSTriggerContext());
    triggerContexts.Add(new WotlkDungeonPoSTriggerContext());
    triggerContexts.Add(new WotlkDungeonToCTriggerContext());
}
