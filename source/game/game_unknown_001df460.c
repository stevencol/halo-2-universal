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
