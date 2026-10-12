/*
GAME_UNKNOWN_001EC420.C

a part of the game that keeps, in the state of the game, eight numbers that are nothing for a new map, and a byte that says that a
map is loaded.
Worked out from halo2ship.exe; what it is for is not known, and the name of
the file is only the address of its first function.
*/

#include "game_unknown_001ec420.h"

#include "saved games/game_state.h"
#include "scenario/scenario.h"

/* ---------- globals */

/* 0x0051e9c8: what it keeps */
struct game_unknown_001ec420_globals *game_unknown_001ec420_globals;

/* ---------- code */

/* 0x001ec420: what it keeps takes its memory of the state of the game, and no
map is loaded. The game has game_state_malloc written here */
void game_unknown_001ec420_initialize(
	void)
{
	struct game_unknown_001ec420_globals *globals= game_state_malloc(sizeof(struct game_unknown_001ec420_globals));

	game_unknown_001ec420_globals= globals;
	globals->map_initialized= FALSE;
}

/* 0x001ec470: it has no memory */
void game_unknown_001ec420_dispose(
	void)
{
	game_unknown_001ec420_globals= NULL;
}

/* 0x001ec480: what it keeps is as it starts, and a map is loaded */
void game_unknown_001ec420_initialize_for_new_map(
	void)
{
	struct game_unknown_001ec420_globals *globals= game_unknown_001ec420_globals;
	long index;

	for (index= 0; index<GAME_UNKNOWN_001EC420_COUNT; index++)
	{
		globals->unknown0[index]= 0;
	}
	globals->map_initialized= TRUE;
}

/* 0x001ec4b0: no map is loaded */
void game_unknown_001ec420_dispose_from_old_map(
	void)
{
	game_unknown_001ec420_globals->map_initialized= FALSE;
}

/* 0x001ec4c0: the eight numbers are 256 bits, and this takes one out: the
one that one of the things of 0x44 bytes that the scenario has at 0x10C names
with its short at 0x40. Nothing for NONE, or if the thing names none */
void game_unknown_001ec420_clear_bit(
	long index)
{
	if (index!=NONE)
	{
		const byte *element= *(const byte *const *)((const byte *)global_scenario+0x10C)+index*0x44;

		if (*(const short *)(element+0x40)!=NONE)
		{
			long bit= *(const short *)(element+0x40);

			game_unknown_001ec420_globals->unknown0[bit>>5]&= ~(1<<(bit&0x1F));
		}
	}
}
