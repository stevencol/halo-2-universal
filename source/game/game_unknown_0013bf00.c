/*
GAME_UNKNOWN_0013BF00.C

a part of the game: it keeps 0x24 bytes in the state of the game, which main.c
looks at (unknown_00510c50).
Worked out from halo2ship.exe; what it is for is not known, and the name of
the file is only the address of its first function.
*/

#include "cseries/cseries.h"
#include "saved games/game_state.h"
#include "game/game.h"

#include <string.h>

/* ---------- constants */

enum
{
	GAME_UNKNOWN_0013BF00_SIZE= 0x24,
	/* a real that is 1 in a game of the first mode, a byte that is set
	then, another byte, four numbers that are NONE for a new map, and one
	more byte */
	GAME_UNKNOWN_0013BF00_UNKNOWN4_OFFSET= 0x4,
	GAME_UNKNOWN_0013BF00_UNKNOWN5_OFFSET= 0x5,
	GAME_UNKNOWN_0013BF00_UNKNOWN8_OFFSET= 0x8,
	GAME_UNKNOWN_0013BF00_UNKNOWN22_OFFSET= 0x22
};

/* ---------- globals */

/* 0x00510c50, of main.c: what it keeps */
struct unknown_00510c50;
extern struct unknown_00510c50 *unknown_00510c50;

/* ---------- code */

/* 0x0013bf00: what it keeps takes its memory of the state of the game, and
is nothing. The game has game_state_malloc written here */
void game_unknown_0013bf00_initialize(
	void)
{
	byte *unknown= game_state_malloc(GAME_UNKNOWN_0013BF00_SIZE);

	memset(unknown, 0, GAME_UNKNOWN_0013BF00_SIZE);
	unknown_00510c50= (struct unknown_00510c50 *)unknown;
}

/* 0x0013bf60: for a new map, it is nothing, with NONE for its four numbers;
and in a game of the first mode its real is 1 and its first byte set */
void game_unknown_0013bf00_initialize_for_new_map(
	void)
{
	byte *unknown= (byte *)unknown_00510c50;
	long index;

	memset(unknown, 0, GAME_UNKNOWN_0013BF00_SIZE);
	for (index= 0; index<4; index++)
	{
		*(long *)(unknown+GAME_UNKNOWN_0013BF00_UNKNOWN8_OFFSET+index*4)= NONE;
	}
	if (main_game_globals->options.game_mode==1)
	{
		unknown[GAME_UNKNOWN_0013BF00_UNKNOWN4_OFFSET]= TRUE;
		*(float *)unknown= 1.0f;
	}
}

/* 0x0013bfc0: when a map goes, its three bytes are clear; the first is left
in a game of the first mode */
void game_unknown_0013bf00_dispose_from_old_map(
	void)
{
	byte *unknown= (byte *)unknown_00510c50;

	if (main_game_globals->options.game_mode!=1)
	{
		unknown[GAME_UNKNOWN_0013BF00_UNKNOWN4_OFFSET]= FALSE;
	}
	unknown[GAME_UNKNOWN_0013BF00_UNKNOWN5_OFFSET]= FALSE;
	unknown[GAME_UNKNOWN_0013BF00_UNKNOWN22_OFFSET]= FALSE;
}
