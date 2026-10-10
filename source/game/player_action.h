/*
PLAYER_ACTION.H

What a player does in a tick of the game: 0x5C bytes that go from the
gamepad to the game, and from console to console in a game in the network.
The name of the file is a guess, and so are the names of its functions.
*/

#ifndef __PLAYER_ACTION_H
#define __PLAYER_ACTION_H

#include "cseries/cseries.h"

/* ---------- constants */

enum
{
	PLAYER_ACTION_SIZE= 0x5C,
	/* of it: numbers that are NONE when there is nothing, of several sizes;
	and three more that are nothing */
	PLAYER_ACTION_UNKNOWN1E_OFFSET= 0x1E,
	PLAYER_ACTION_UNKNOWN20_OFFSET= 0x20,
	PLAYER_ACTION_UNKNOWN21_OFFSET= 0x21,
	PLAYER_ACTION_UNKNOWN22_OFFSET= 0x22,
	PLAYER_ACTION_UNKNOWN24_OFFSET= 0x24,
	PLAYER_ACTION_UNKNOWN28_OFFSET= 0x28,
	PLAYER_ACTION_UNKNOWN2C_OFFSET= 0x2C,
	PLAYER_ACTION_UNKNOWN30_OFFSET= 0x30,
	PLAYER_ACTION_UNKNOWN34_OFFSET= 0x34,
	PLAYER_ACTION_UNKNOWN38_OFFSET= 0x38,
	PLAYER_ACTION_UNKNOWN3C_OFFSET= 0x3C,
	PLAYER_ACTION_UNKNOWN4C_OFFSET= 0x4C,
	PLAYER_ACTION_UNKNOWN50_OFFSET= 0x50,
	PLAYER_ACTION_UNKNOWN54_OFFSET= 0x54
};

/* ---------- prototypes/PLAYER_ACTION.C */

void player_action_clear(byte *action);

#endif
