/*
PLAYER_ACTION.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

What a player does in a tick of the game. The name of the file is a guess,
and so are the names of its functions.
*/

#include "player_action.h"

#include "math/real_math.h"

#include <string.h>

/* ---------- code */

/* 0x0014d040: what a player does is nothing: all zeros, but for the numbers
that are NONE when there is nothing */
void player_action_clear(
	byte *action)
{
	memset(action, 0, PLAYER_ACTION_SIZE);
	*(short *)(action+PLAYER_ACTION_UNKNOWN1E_OFFSET)= NONE;
	action[PLAYER_ACTION_UNKNOWN20_OFFSET]= (byte)NONE;
	action[PLAYER_ACTION_UNKNOWN21_OFFSET]= (byte)NONE;
	*(short *)(action+PLAYER_ACTION_UNKNOWN22_OFFSET)= NONE;
	*(short *)(action+PLAYER_ACTION_UNKNOWN24_OFFSET)= NONE;
	*(short *)(action+PLAYER_ACTION_UNKNOWN4C_OFFSET)= 0;
	*(real *)(action+PLAYER_ACTION_UNKNOWN50_OFFSET)= 0.0f;
	*(real *)(action+PLAYER_ACTION_UNKNOWN54_OFFSET)= 0.0f;
	*(long *)(action+PLAYER_ACTION_UNKNOWN34_OFFSET)= NONE;
	*(long *)(action+PLAYER_ACTION_UNKNOWN38_OFFSET)= NONE;
	*(long *)(action+PLAYER_ACTION_UNKNOWN3C_OFFSET)= NONE;
	*(long *)(action+PLAYER_ACTION_UNKNOWN28_OFFSET)= 0;
	*(long *)(action+PLAYER_ACTION_UNKNOWN2C_OFFSET)= NONE;
	*(long *)(action+PLAYER_ACTION_UNKNOWN30_OFFSET)= NONE;
}
