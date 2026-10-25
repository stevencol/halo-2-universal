/*
GAME_UNKNOWN_001552E0.C

a part of the game: it keeps 0x50C bytes of its own, and a byte and 0x78 bytes in
the state of the game.
Worked out from halo2ship.exe; what it is for is not known, and the name of
the file is only the address of its first function.
*/

#include "cseries/cseries.h"
#include "saved games/game_state.h"

#include <string.h>

/* ---------- globals */

/* 0x004e8c38: what it keeps of its own. 0x004e8c34: the byte. 0x00510c6c,
of game.c: the 0x78 bytes */
byte unknown_004e8c38[0x50C];
byte *unknown_004e8c34;
struct unknown_00510c6c;
extern struct unknown_00510c6c *unknown_00510c6c;

/* ---------- code */

/* 0x001552e0: what it keeps of its own is nothing; the byte takes four
bytes of the state of the game and is clear, and the 0x78 bytes take theirs.
The game has game_state_malloc written here, twice */
void game_unknown_001552e0_initialize(
	void)
{
	byte *unknown;

	memset(unknown_004e8c38, 0, sizeof(unknown_004e8c38));
	unknown= game_state_malloc(4);
	*unknown= 0;
	unknown_004e8c34= unknown;
	unknown_00510c6c= game_state_malloc(0x78);
}

/* 0x00155490: when a map goes, what it keeps of its own is nothing, and the
byte is clear */
void game_unknown_001552e0_dispose_from_old_map(
	void)
{
	memset(unknown_004e8c38, 0, sizeof(unknown_004e8c38));
	*unknown_004e8c34= 0;
}

/* 0x00155380: the part for a new map. 0x00155a30: something that is done
with the byte */
void unknown_00155380(void);
void __attribute__((stdcall)) unknown_00155a30(long unknown);

/* 0x00155360: after a state of the game was loaded: the part is as for a new
map (unknown_00155380), and unknown_00155a30 is done with its byte. It is
one of the things that are done then (game_state.c) */
void GAME_STATE_PROC game_unknown_001552e0_after_load(
	unsigned long flags)
{
	(void)flags;
	unknown_00155380();
	unknown_00155a30(*unknown_004e8c34);
}

/* 0x0023c110: what each of the four is given to do (of three arguments) */
void __attribute__((stdcall)) unknown_0023c110(long, long, long);

/* 0x00155710: when the long at 0x04 is zero, one of the four (0x140 bytes
each, from 0x0C) starts over: the short at 0x58 and the reals at 0x10 and
0x14 to zero, unknown_0023c110 at 0x0C, 1 at 0x10C and the byte at 0x108 to
zero */
void unknown_00155710(
	long index)
{
	byte *entry= unknown_004e8c38+0x0C+index*0x140;

	if (*(const long *)(unknown_004e8c38+0x04)!=0)
	{
		return;
	}
	*(short *)(entry+0x58)= 0;
	*(float *)(entry+0x10)= 0.0f;
	*(float *)(entry+0x14)= 0.0f;
	*(void (__attribute__((stdcall)) **)(long, long, long))(entry+0x0C)= unknown_0023c110;
	*(float *)(entry+0x10C)= 1.0f;
	entry[0x108]= 0;
}
