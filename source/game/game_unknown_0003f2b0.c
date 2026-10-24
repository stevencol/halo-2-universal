/*
GAME_UNKNOWN_0003F2B0.C

a part of the game: a number that is zero when a structure bsp comes.
Worked out from halo2ship.exe; what it is for is not known, and the name of
the file is only the address of its function.
*/

#include "cseries/cseries.h"
#include "scenario/scenario.h"
#include <stdint.h>

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

/* 0x00040de0: 0 if there is nothing, 1 if the first byte is set, 2 if only
the second, and NONE if none. It is before unknown_00040e10 in the game */
long __attribute__((stdcall)) unknown_00040de0(
	const void *something,
	boolean first,
	boolean second)
{
	if (something==NULL)
	{
		return 0;
	}
	if (first)
	{
		return 1;
	}

	return second ? 2 : NONE;
}

/* 0x004b9ee9 and 0x004b9eec: a byte that says yes, and an index of the list
of shorts at 0xB0 of the structure bsp */
byte unknown_004b9ee9;
long unknown_004b9eec;

/* 0x0003eb20: whether the short of the structure bsp that unknown_004b9eec
names is this number; FALSE if any of them is not there. It is before
game_unknown_0003f2b0_initialize_for_new_structure_bsp in the game; whether it
is of this file is not known */
boolean unknown_004b9eec_matches(
	long value)
{
	const byte *structure_bsp= global_structure_bsp;

	if (value!=NONE && unknown_004b9ee9 && unknown_004b9eec!=NONE && global_structure_bsp_index!=NONE &&
		*(const long *)(structure_bsp+0xAC)>0 &&
		((const short *)(uintptr_t)*(const unsigned long *)(structure_bsp+0xB0))[unknown_004b9eec]==value)
	{
		return TRUE;
	}

	return FALSE;
}
