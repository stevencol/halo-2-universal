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
#include "math/real_math.h"
#include "scenario/scenario.h"

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

/* 0x0013cb40: the byte at 5 of what unknown_00510c50 points to; FALSE if it
points to nothing */
byte unknown_00510c50_get_unknown5(
	void)
{
	const byte *unknown= (const byte *)unknown_00510c50;

	if (unknown)
	{
		return unknown[5];
	}

	return FALSE;
}

/* 0x004e69d0: four of something of 0x270 bytes */
byte unknown_004e69d0[4*0x270];

/* 0x0013ac42: one of those four is zeros, with its first seven longs NONE */
void game_unknown_0013ac42(
	long index)
{
	long *entry= (long *)(unknown_004e69d0+index*0x270);
	long slot;

	memset(entry, 0, 0x270);
	for (slot= 0; slot<7; slot++)
	{
		entry[slot]= NONE;
	}
}

/* 0x0013ac30: all four are made so */
void game_unknown_0013ac30(
	void)
{
	long index;

	for (index= 0; index<4; index++)
	{
		game_unknown_0013ac42(index);
	}
}

/* 0x00510c4c: 0x1E4 bytes of the state of the game. 0x004e6950: 0x80
bytes */
byte *unknown_00510c4c;
byte unknown_004e6950[0x80];

/* 0x00139130: the 0x1E4 bytes take their memory, the 0x80 are zeros, and
the four of game_unknown_0013ac30 are made empty */
void game_unknown_00139130(
	void)
{
	unknown_00510c4c= game_state_malloc(0x1E4);
	memset(unknown_004e6950, 0, sizeof(unknown_004e6950));
	game_unknown_0013ac30();
}

/* 0x0013c680: the timer (0x18 and 0x1C) of unknown_00510c50 runs down; it
fades in or out (a real at 0x00 by the byte at 0x04); and its four titles
(an index and ticks, from 0x08) are done after the up and fade-in times of
their definition (0x24 bytes at 0x1F4 of the scenario, as many as at 0x1F0) */
void unknown_0013c680(
	void)
{
	byte *data= (byte *)unknown_00510c50;
	const struct game_time_globals *time= game_time_globals;
	const byte *scenario= (const byte *)global_scenario;
	short *title;
	long count;

	if (*(const long *)(data+0x18))
	{
		*(real *)(data+0x1C)-= time->seconds_per_tick;
		if (!(0.0f<*(const real *)(data+0x1C)))
		{
			*(long *)(data+0x18)= 0;
			*(long *)(data+0x1C)= 0;
		}
	}
	if (!data)
	{
		return;
	}
	if (data[4] || *(const real *)data>0.0f)
	{
		real fade;

		if (data[4])
		{
			fade= time->seconds_per_tick+*(const real *)data;
			*(real *)data= fade;
			if (fade>1.0f)
			{
				fade= 1.0f;
			}
		}
		else
		{
			fade= *(const real *)data-time->seconds_per_tick;
			*(real *)data= fade;
			if (!(fade>0.0f))
			{
				fade= 0.0f;
			}
		}
		*(real *)data= fade;
	}
	title= (short *)(data+0x08);
	for (count= 0; count<4; count++, title+= 2)
	{
		long last= *(const long *)(scenario+0x1F0)-1;
		long pinned= title[0]<0 ? 0 : title[0]>last ? last : title[0];

		if (pinned==title[0])
		{
			const real *definition= (const real *)(*(const byte *const *)(scenario+0x1F4)+title[0]*0x24);

			title[1]++;
			if ((real)title[1]*time->seconds_per_tick>=definition[8]+definition[7])
			{
				title[0]= NONE;
				title[1]= NONE;
			}
		}
	}
}
