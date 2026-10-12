/*
GAME_UNKNOWN_001E6A40.C

a part of the game: it keeps 0x6C8 bytes in the state of the game, four
things of 0x1B0 bytes and eight bytes more.
Worked out from halo2ship.exe; what it is for is not known, and the name of
the file is only the address of its first function.
*/

#include "cseries/cseries.h"
#include "saved games/game_state.h"

#include <string.h>

/* ---------- constants */

enum
{
	GAME_UNKNOWN_001E6A40_SIZE= 0x6C8,
	/* its four things, and the two numbers of each that are NONE for a new
	map; a number and a byte that are nothing then, and a byte that is set */
	GAME_UNKNOWN_001E6A40_COUNT= 4,
	GAME_UNKNOWN_001E6A40_ENTRY_SIZE= 0x1B0,
	GAME_UNKNOWN_001E6A40_ENTRY_UNKNOWN80_OFFSET= 0x80,
	GAME_UNKNOWN_001E6A40_ENTRY_UNKNOWN190_OFFSET= 0x190,
	GAME_UNKNOWN_001E6A40_UNKNOWN6C0_OFFSET= 0x6C0,
	GAME_UNKNOWN_001E6A40_UNKNOWN6C4_OFFSET= 0x6C4,
	GAME_UNKNOWN_001E6A40_UNKNOWN6C5_OFFSET= 0x6C5
};

/* ---------- globals */

/* 0x0051e9c0: what it keeps */
byte *unknown_0051e9c0;

/* 0x001e75d0: what is done with it when a map goes */
void __attribute__((stdcall)) unknown_001e75d0(long unknown);

/* ---------- code */

/* 0x001e6a40: what it keeps takes its memory of the state of the game. The
game has game_state_malloc written here */
void game_unknown_001e6a40_initialize(
	void)
{
	unknown_0051e9c0= game_state_malloc(GAME_UNKNOWN_001E6A40_SIZE);
}

/* 0x001e6a80: for a new map, it is nothing, but for the two numbers of each
of its four things, which are NONE, and a byte that is set */
void game_unknown_001e6a40_initialize_for_new_map(
	void)
{
	byte *unknown= unknown_0051e9c0;
	long index;

	memset(unknown, 0, GAME_UNKNOWN_001E6A40_SIZE);
	unknown[GAME_UNKNOWN_001E6A40_UNKNOWN6C5_OFFSET]= TRUE;
	for (index= 0; index<GAME_UNKNOWN_001E6A40_COUNT; index++)
	{
		byte *entry= unknown+index*GAME_UNKNOWN_001E6A40_ENTRY_SIZE;

		*(long *)(entry+GAME_UNKNOWN_001E6A40_ENTRY_UNKNOWN80_OFFSET)= NONE;
		*(long *)(entry+GAME_UNKNOWN_001E6A40_ENTRY_UNKNOWN190_OFFSET)= NONE;
	}
	*(long *)(unknown+GAME_UNKNOWN_001E6A40_UNKNOWN6C0_OFFSET)= 0;
	unknown[GAME_UNKNOWN_001E6A40_UNKNOWN6C4_OFFSET]= FALSE;
}

/* 0x001e6ae0: when a map goes: unknown_001e75d0, with 1 */
void game_unknown_001e6a40_dispose_from_old_map(
	void)
{
	unknown_001e75d0(1);
}

/* 0x001e6af0: after a state of the game was loaded: unknown_001e75d0, with
nothing. It is one of the things that are done then (game_state.c) */
void GAME_STATE_PROC game_unknown_001e6a40_after_load(
	unsigned long flags)
{
	(void)flags;
	unknown_001e75d0(0);
}
