/*
GAME_UNKNOWN_0002B4A0.C

a part of the game: it has a table of data ("cached object render states"),
and its first function sets the time of the game in seconds. It comes after
the rasterizer in the table of the parts, so it may be what draws the world.
Worked out from halo2ship.exe; what it is for is not known, and the name of
the file is only the address of its first function.
*/

#include "cseries/cseries.h"
#include "memory/data.h"
#include "saved games/game_state.h"

#include <stdint.h>
#include <string.h>

/* ---------- globals */

/* 0x00509434: its table of data ("cached object render states": 0x100 of
0x100 bytes). 0x004c1bd0: a number that is NONE when the table is made */
struct data_array *cached_object_render_state_data;
long unknown_004c1bd0;

/* 0x004ba040, of game.c: the time of the game, in seconds */
extern double game_seconds;

/* what the part sets when it starts: three numbers that are zero
(0x004ba034, 0x004ba038, 0x004ba03c) and two lists of four that are NONE
(0x004ba054, 0x004ba064); and 0x0050942c: 0x18 bytes of the state of the
game */
long unknown_004ba034;
long unknown_004ba038;
long unknown_004ba03c;
long unknown_004ba054[4];
long unknown_004ba064[4];
byte *unknown_0050942c;

/* what it sets for a new map: a number that is zero (0x004ba134), 0x30 bytes
that are zeros but for the first number, which is NONE (0x004b9970), 0x6A8
bytes of zeros (0x004c5018) and a byte that is zero (0x005093fc) */
long unknown_004ba134;
byte unknown_004b9970[0x30];
byte unknown_004c5018[0x6A8];
byte unknown_005093fc;

void unknown_0003b8a0(void);
void unknown_000167a0(void);

/* 0x004c1a5c, 0x004c1a74, 0x004c1a8c: three memories of the console, 0x18
bytes from one to the next */
void *unknown_004c1a5c;
void *unknown_004c1a74;
void *unknown_004c1a8c;

int VirtualFree(void *address, unsigned long size, unsigned long type);
uint32_t GetLastError(void);

void game_unknown_0002b4a0_unknown_0003b950(void);

/* ---------- code */

/* 0x0002b5c0: the part is closed: game_unknown_0002b4a0_unknown_0003b950, and it has no table */
void game_unknown_0002b4a0_dispose(
	void)
{
	game_unknown_0002b4a0_unknown_0003b950();
	cached_object_render_state_data= NULL;
}

/* 0x0002b5a0: when a map goes, the table cannot be used, if it is there and
could */
void game_unknown_0002b4a0_dispose_from_old_map(
	void)
{
	struct data_array *data= cached_object_render_state_data;

	if (data && data->valid)
	{
		data->valid= FALSE;
	}
}

/* 0x0003b950: each of the three memories of the console that is there is
given back (VirtualFree; if that fails, the error is asked for), and there
is none */
void game_unknown_0002b4a0_unknown_0003b950(
	void)
{
	if (unknown_004c1a5c)
	{
		if (!VirtualFree(unknown_004c1a5c, 0, 0x8000))
		{
			GetLastError();
		}
		unknown_004c1a5c= NULL;
	}
	if (unknown_004c1a74)
	{
		if (!VirtualFree(unknown_004c1a74, 0, 0x8000))
		{
			GetLastError();
		}
		unknown_004c1a74= NULL;
	}
	if (unknown_004c1a8c)
	{
		if (!VirtualFree(unknown_004c1a8c, 0, 0x8000))
		{
			GetLastError();
		}
		unknown_004c1a8c= NULL;
	}
}

/* 0x0003d270: the table is made, in the memory of the state of the game
(data_new, which the game has written here). In the game it is far from the
others of this file, so it may be of another */
void cached_object_render_states_initialize(
	void)
{
	cached_object_render_state_data= data_new("cached object render states", 0x100, 0x100, 0, game_state_allocation);
	unknown_004c1bd0= NONE;
}

/* 0x0002b4a0: the first of the functions of the part (game.c): the time of
the game is zero, its numbers are as they start, unknown_0003b8a0, the table
and unknown_000167a0 start, and the 0x18 bytes take their memory in the state
of the game (game_state_malloc, which the game has written here) */
void game_unknown_0002b4a0_initialize(
	void)
{
	long index;

	unknown_004ba034= 0;
	game_seconds= 0.0;
	unknown_004ba038= 0;
	unknown_004ba03c= 0;
	for (index= 0; index<4; index++)
	{
		unknown_004ba054[index]= NONE;
		unknown_004ba064[index]= NONE;
	}
	unknown_0003b8a0();
	cached_object_render_states_initialize();
	unknown_000167a0();
	unknown_0050942c= game_state_malloc(0x18);
}

/* 0x0002b540: the third: for a new map the time of the game is zero, the
table is good to use, with nothing in it, and the rest is zeros */
void game_unknown_0002b4a0_initialize_for_new_map(
	void)
{
	game_seconds= 0.0;
	unknown_004ba134= 0;
	data_make_valid(cached_object_render_state_data);
	memset(unknown_004b9970, 0, sizeof(unknown_004b9970));
	memset(unknown_004c5018, 0, sizeof(unknown_004c5018));
	*(long *)unknown_004b9970= NONE;
	unknown_005093fc= 0;
}

/* 0x0003ba90: whether something lacks a bit in its number at 8 */
long __attribute__((stdcall)) unknown8_lacks_bit(
	long bit,
	const byte *unknown)
{
	return !((1u<<(bit&31))&*(const unsigned long *)(unknown+8));
}
