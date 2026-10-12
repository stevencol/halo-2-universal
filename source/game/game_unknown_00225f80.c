/*
GAME_UNKNOWN_00225F80.C

a part of the game: four bytes of the state of the game, with a number that is NONE in a
new map.
Worked out from halo2ship.exe; what it is for is not known, and the name of
the file is only the address of its first function.
*/

#include "cseries/cseries.h"

#include <string.h>

#include "saved games/game_state.h"

/* ---------- globals */

/* 0x0051ebf8: the four bytes. 0x004701ec: a number that is zero in a new
map */
long *unknown_0051ebf8;
long unknown_004701ec;

/* ---------- code */

/* 0x00225f80: the four bytes take their memory in the state of the game
(game_state_malloc, which the game has written here) */
void game_unknown_00225f80_initialize(
	void)
{
	unknown_0051ebf8= game_state_malloc(sizeof(long));
}

/* 0x00225fc0: for a new map the number is NONE, and the other is zero */
void game_unknown_00225f80_initialize_for_new_map(
	void)
{
	*unknown_0051ebf8= NONE;
	unknown_004701ec= 0;
}
