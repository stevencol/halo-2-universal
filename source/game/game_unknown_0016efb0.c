/*
GAME_UNKNOWN_0016EFB0.C

a part of the game: it keeps five numbers in the state of the game.
Worked out from halo2ship.exe; what it is for is not known, and the name of
the file is only the address of its first function.
*/

#include "cseries/cseries.h"
#include "saved games/game_state.h"

/* ---------- globals */

/* 0x00510c70: what it keeps */
long *unknown_00510c70;

/* ---------- code */

/* 0x0016efb0: what it keeps takes its memory of the state of the game. The
game has game_state_malloc written here */
void game_unknown_0016efb0_initialize(
	void)
{
	unknown_00510c70= game_state_malloc(5*sizeof(long));
}
