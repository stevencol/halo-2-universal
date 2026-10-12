/*
GAME_UNKNOWN_001C7FE0.C

something that stops with the players when a map goes: two things of 0x1C
bytes. Worked out from halo2ship.exe; what it is for is not known, and the
name of the file is only the address of its function.
*/

#include "cseries/cseries.h"

/* ---------- globals */

/* 0x004f55cc: the two things */
byte *unknown_004f55cc;

/* ---------- code */

/* 0x001c7fe0: each of the two things has NONE for its first two numbers and
nothing for the short at 0xA */
void game_unknown_001c7fe0(
	void)
{
	byte *unknown= unknown_004f55cc;
	long index;

	for (index= 0; index<2; index++)
	{
		*(long *)unknown= NONE;
		*(long *)(unknown+4)= NONE;
		*(short *)(unknown+0xA)= 0;
		unknown+= 0x1C;
	}
}
