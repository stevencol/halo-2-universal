/*
GAME_UNKNOWN_0003F2B0.C

a part of the game: a number that is zero when a structure bsp comes.
Worked out from halo2ship.exe; what it is for is not known, and the name of
the file is only the address of its function.
*/

#include "cseries/cseries.h"

/* ---------- globals */

/* 0x00509438 */
long unknown_00509438;

/* ---------- code */

/* 0x0003f2b0: the fifth of the functions of the part (game.c): for a new
structure bsp the number is zero */
void game_unknown_0003f2b0_initialize_for_new_structure_bsp(
	void)
{
	unknown_00509438= 0;
}

/* 0x00040e10: a number from two bytes: 0 if both are set, 1 if only the
first, 2 if only the second and 3 if none. It is after this file in the
game */
long unknown_00040e10(
	boolean first,
	boolean second)
{
	if (first)
	{
		return !second;
	}

	return second ? 2 : 3;
}
