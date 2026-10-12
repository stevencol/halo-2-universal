/*
GAME_UNKNOWN_00153870.C

a part of the game: it keeps 0x2B0 bytes in the state of the game.
Worked out from halo2ship.exe; what it is for is not known, and the name of
the file is only the address of its first function.
*/

#include "cseries/cseries.h"
#include "saved games/game_state.h"

/* ---------- globals */

/* 0x00510c5c: what it keeps */
byte *unknown_00510c5c;

/* ---------- code */

/* 0x00153870: what it keeps takes its memory of the state of the game. The
game has game_state_malloc written here */
void game_unknown_00153870_initialize(
	void)
{
	unknown_00510c5c= game_state_malloc(0x2B0);
}
