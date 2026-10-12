/*
GAME_UNKNOWN_00134D90.C

a part of the game: 0x204 bytes of the state of the game, that are zeros in a new map.
Worked out from halo2ship.exe; what it is for is not known, and the name of
the file is only the address of its first function.
*/

#include "cseries/cseries.h"

#include <string.h>

#include "saved games/game_state.h"

/* ---------- constants */

enum
{
	GAME_UNKNOWN_00134D90_SIZE= 0x204
};

/* ---------- globals */

/* 0x004e6740: the 0x204 bytes; NULL when the part is closed */
byte *unknown_004e6740;

/* ---------- code */

/* 0x00134d90: the bytes take their memory in the state of the game
(game_state_malloc, which the game has written here) */
void game_unknown_00134d90_initialize(
	void)
{
	unknown_004e6740= game_state_malloc(GAME_UNKNOWN_00134D90_SIZE);
}

/* 0x00134dd0: the part is closed: there are no bytes */
void game_unknown_00134d90_dispose(
	void)
{
	unknown_004e6740= NULL;
}

/* 0x00134de0: for a new map the bytes are zeros, if they are there */
void game_unknown_00134d90_initialize_for_new_map(
	void)
{
	if (unknown_004e6740)
	{
		memset(unknown_004e6740, 0, GAME_UNKNOWN_00134D90_SIZE);
	}
}
