/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_COAWILDCARDACTION_H
#define PLAYERBOTS_COAWILDCARDACTION_H

#include "Action.h"

class PlayerbotAI;

// "wildcard": a Wildcard Hero bot whispers its role, stat path, abilities and talents; "wildcard role
// <tank|heal|dps>" makes it play that role; "wildcard redraft" has a random bot roll its whole build again.
class CoaWildcardAction : public Action
{
public:
    CoaWildcardAction(PlayerbotAI* botAI) : Action(botAI, "wildcard") {}

    bool Execute(Event event) override;
};

#endif
