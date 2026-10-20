/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_COAWCRACTION_H
#define PLAYERBOTS_COAWCRACTION_H

#include "Action.h"

class PlayerbotAI;

// "wcr": a Warcraft Reborn bot whispers the tree it plays and its three trees; "wcr spec <0-2>" moves it to that
// tree and its Character Advancement build.
class CoaWcrAction : public Action
{
public:
    CoaWcrAction(PlayerbotAI* botAI) : Action(botAI, "wcr") {}

    bool Execute(Event event) override;
};

#endif
