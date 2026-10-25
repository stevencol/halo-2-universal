/*
GAME_UNKNOWN_0003F2B0.C

a part of the game: a number that is zero when a structure bsp comes.
Worked out from halo2ship.exe; what it is for is not known, and the name of
the file is only the address of its function.
*/

#include "cseries/cseries.h"
#include "scenario/scenario.h"
#include "math/real_math.h"
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

/* 0x00547f88: the first of the two things of game_unknown_00131ff0.c; its
first long points to what 0x0003f3f0 reads */
extern byte unknown_00547f88[];

/* 0x004c56c0: 512 bits, cleared by 0x0002b5d0 */
unsigned long unknown_004c56c0[16];

/* 0x0003f3f0: for each of the entries (0x1A bytes) at 0xA6E of what the
first thing points to (as many as its short at 0xA6C), the bit named by the
entry's first short is set in unknown_004c56c0 */
void unknown_0003f3f0(
	void)
{
	const byte *data= *(const byte *const *)unknown_00547f88;
	long index;

	for (index= 0; index<*(const short *)(data+0xA6C); index++)
	{
		short bit= *(const short *)(data+index*0x1A+0xA6E);

		unknown_004c56c0[bit>>5]|= 1UL<<(bit&0x1F);
	}
}

/* 0x0003fd20: whether a sphere is in front of any of six planes by at least
its radius times the square root of three (the half diagonal of a cube) */
boolean unknown_0003fd20(
	const real_plane3d *planes,
	const real_point3d *center,
	real radius)
{
	real distance= radius*1.7320508f;
	long index;

	for (index= 0; index<6; index++)
	{
		const real_plane3d *plane= planes+index;

		if (plane->n.i*center->x + plane->n.k*center->z + plane->n.j*center->y - plane->d>=distance)
		{
			return TRUE;
		}
	}

	return FALSE;
}

/* 0x004c6700: eight of something of 0x408 bytes (a long at 0x400 and a
short at 0x404). 0x004c8740: sixteen numbers. 0x0050943c and 0x00509440:
two more */
byte unknown_004c6700[8*0x408];
long unknown_004c8740[16];
long unknown_0050943c;
long unknown_00509440;

/* 0x0003fd70: the sixteen are NONE, and the eight have NONE and 0 */
void unknown_0003fd70(
	void)
{
	long index;

	for (index= 0; index<16; index++)
	{
		unknown_004c8740[index]= NONE;
	}
	unknown_0050943c= 0;
	for (index= 0; index<8; index++)
	{
		*(long *)(unknown_004c6700+index*0x408+0x400)= NONE;
		*(short *)(unknown_004c6700+index*0x408+0x404)= 0;
	}
	unknown_00509440= 0;
}
