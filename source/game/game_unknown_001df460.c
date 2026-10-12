/*
GAME_UNKNOWN_001DF460.C

a part of the game: it keeps 0xE4 bytes in the state of the game, of which the last
0x20 are sixteen rows of sixteen bits.
Worked out from halo2ship.exe; what it is for is not known, and the name of
the file is only the address of its first function.
*/

#include "cseries/cseries.h"
#include "saved games/game_state.h"

#include <string.h>

/* ---------- constants */

enum
{
	GAME_UNKNOWN_001DF460_SIZE= 0xE4,
	/* sixteen rows of sixteen bits, and where they are */
	GAME_UNKNOWN_001DF460_ROWS= 16,
	GAME_UNKNOWN_001DF460_BITS_OFFSET= 0xC4
};

/* ---------- globals */

/* 0x004f55ec: what it keeps */
byte *unknown_004f55ec;

/* ---------- code */

/* 0x001df460: what it keeps takes its memory of the state of the game, and
is nothing. The game has game_state_malloc written here */
void game_unknown_001df460_initialize(
	void)
{
	byte *unknown= game_state_malloc(GAME_UNKNOWN_001DF460_SIZE);

	memset(unknown, 0, GAME_UNKNOWN_001DF460_SIZE);
	unknown_004f55ec= unknown;
}

/* 0x001df4b0: for a new map, it is nothing, but for the bit of each row
that has the number of the row: each of sixteen things with itself */
void game_unknown_001df460_initialize_for_new_map(
	void)
{
	byte *unknown= unknown_004f55ec;
	unsigned long *bits= (unsigned long *)(unknown+GAME_UNKNOWN_001DF460_BITS_OFFSET);
	long index;

	memset(unknown, 0, GAME_UNKNOWN_001DF460_SIZE);
	for (index= 0; index<GAME_UNKNOWN_001DF460_ROWS; index++)
	{
		long bit= index*(GAME_UNKNOWN_001DF460_ROWS+1);

		bits[bit>>5]|= 1u<<(bit&31);
	}
}

/* 0x001df9c0: what is done with one of the things that are counted down,
when its time is over for the last time */
void __attribute__((stdcall)) unknown_001df9c0(byte *element, long unknown2, long unknown3);

/* 0x001df500: each tick of the game (game.c). What the part keeps starts
with how many things it counts down (a short) and has them from 4, of 0x14
bytes each: a short at 0xE that goes down each tick while it is above zero;
when it gets to zero, the short at 0xC goes down, and if that is zero too,
unknown_001df9c0 is done with the thing; if not, the first short starts
again from the one at 6. By the sixteen rows of sixteen bits it may be who is
whose friend, and these the changes of it that last for a time; that is a
guess */
void game_unknown_001df460_update(
	void)
{
	byte *unknown= unknown_004f55ec;
	byte *element= unknown+4;
	short index;

	for (index= 0; index<*(short *)unknown; index++)
	{
		short ticks= *(short *)(element+0xE);

		if (ticks>0)
		{
			ticks--;
			*(short *)(element+0xE)= ticks;
			if (ticks==0)
			{
				if (--*(short *)(element+0xC)==0)
				{
					unknown_001df9c0(element, 0, 0);
					unknown= unknown_004f55ec;
				}
				else
				{
					*(short *)(element+0xE)= *(short *)(element+6);
				}
			}
		}
		element+= 0x14;
	}
}
