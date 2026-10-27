#include "CoaBotRacePolicy.h"

#include <cassert>

int main()
{
    std::vector<std::uint32_t> const crashingRaces = {32, 72, 77, 97};
    for (std::uint32_t race : {1, 2, 3, 4, 5, 6, 7, 8, 10, 11})
    {
        assert(IsCoaBotRaceAllowed(race, false, crashingRaces));
        assert(IsCoaBotRaceAllowed(race, true, crashingRaces));
    }
    for (std::uint32_t race : {9, 37, 75, 98, 100, 102, 110, 112, 128})
    {
        assert(!IsCoaBotRaceAllowed(race, false, crashingRaces));
        assert(IsCoaBotRaceAllowed(race, true, crashingRaces));
        assert(!IsCoaBotRaceAllowed(race, false, crashingRaces));
    }
    for (std::uint32_t race : crashingRaces)
        assert(!IsCoaBotRaceAllowed(race, true, crashingRaces));
    assert(!IsCoaBotRaceAllowed(1, true, {1}));
}
