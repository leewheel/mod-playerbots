/*
 * This is Leewheel Script Project
 */

#ifndef _PLAYERBOT_NAXXRAMASSTRATEGYCONTEXT_H
#define _PLAYERBOT_NAXXRAMASSTRATEGYCONTEXT_H

#include "Strategy.h"
#include "NamedObjectContext.h"
#include "NaxxStrategy.h"

//By Leewheel 2026-02-11
class RaidNaxxramasStrategyContext : public NamedObjectContext<Strategy>
{
public:
    RaidNaxxramasStrategyContext()
    {
        creators["naxx"] = &RaidNaxxramasStrategyContext::naxx;
        creators["naxx anubrekhan"] = &RaidNaxxramasStrategyContext::naxx_anubrekhan;
        creators["naxx faerlina"] = &RaidNaxxramasStrategyContext::naxx_faerlina;
        creators["naxx maexxna"] = &RaidNaxxramasStrategyContext::naxx_maexxna;
        creators["naxx patchwerk"] = &RaidNaxxramasStrategyContext::naxx_patchwerk;
        creators["naxx grobbulus"] = &RaidNaxxramasStrategyContext::naxx_grobbulus;
        creators["naxx gluth"] = &RaidNaxxramasStrategyContext::naxx_gluth;
        creators["naxx noth"] = &RaidNaxxramasStrategyContext::naxx_noth;
        creators["naxx heigan"] = &RaidNaxxramasStrategyContext::naxx_heigan;
        creators["naxx loatheb"] = &RaidNaxxramasStrategyContext::naxx_loatheb;
        creators["naxx razuvious"] = &RaidNaxxramasStrategyContext::naxx_razuvious;
        creators["naxx gothik"] = &RaidNaxxramasStrategyContext::naxx_gothik;
        creators["naxx four horsemen"] = &RaidNaxxramasStrategyContext::naxx_four_horsemen;
        creators["naxx sapphiron"] = &RaidNaxxramasStrategyContext::naxx_sapphiron;
        creators["naxx kelthuzad"] = &RaidNaxxramasStrategyContext::naxx_kelthuzad;
        creators["naxx thaddius"] = &RaidNaxxramasStrategyContext::naxx_thaddius;
    }

private:
    static Strategy* naxx(PlayerbotAI* botAI) { return new NaxxramasStrategy(botAI); }
    static Strategy* naxx_anubrekhan(PlayerbotAI* botAI) { return new NaxxAnubrekhanStrategy(botAI); }
    static Strategy* naxx_faerlina(PlayerbotAI* botAI) { return new NaxxFaerlinaStrategy(botAI); }
    static Strategy* naxx_maexxna(PlayerbotAI* botAI) { return new NaxxMaexxnaStrategy(botAI); }
    static Strategy* naxx_patchwerk(PlayerbotAI* botAI) { return new NaxxPatchwerkStrategy(botAI); }
    static Strategy* naxx_grobbulus(PlayerbotAI* botAI) { return new NaxxGrobbulusStrategy(botAI); }
    static Strategy* naxx_gluth(PlayerbotAI* botAI) { return new NaxxGluthStrategy(botAI); }
    static Strategy* naxx_noth(PlayerbotAI* botAI) { return new NaxxNothStrategy(botAI); }
    static Strategy* naxx_heigan(PlayerbotAI* botAI) { return new NaxxHeiganStrategy(botAI); }
    static Strategy* naxx_loatheb(PlayerbotAI* botAI) { return new NaxxLoathebStrategy(botAI); }
    static Strategy* naxx_razuvious(PlayerbotAI* botAI) { return new NaxxRazuviousStrategy(botAI); }
    static Strategy* naxx_gothik(PlayerbotAI* botAI) { return new NaxxGothikStrategy(botAI); }
    static Strategy* naxx_four_horsemen(PlayerbotAI* botAI) { return new NaxxFourHorsemenStrategy(botAI); }
    static Strategy* naxx_sapphiron(PlayerbotAI* botAI) { return new NaxxSapphironStrategy(botAI); }
    static Strategy* naxx_kelthuzad(PlayerbotAI* botAI) { return new NaxxKelthuzadStrategy(botAI); }
    static Strategy* naxx_thaddius(PlayerbotAI* botAI) { return new NaxxThaddiusStrategy(botAI); }
};
//End By Leewheel

#endif
