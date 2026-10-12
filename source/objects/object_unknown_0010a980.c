/*
OBJECT_UNKNOWN_0010A980.C

a part of the game that keeps, in the state of the game, thirty-two things of 0x14 bytes, thirty-two numbers that are NONE for a new
map, and a byte that says that a map is loaded.
Worked out from halo2ship.exe; what it is for is not known, and the name of
the file is only the address of its first function.
*/

#include "object_unknown_0010a980.h"

#include "saved games/game_state.h"

/* ---------- globals */

/* 0x005107f4: what it keeps */
struct object_unknown_0010a980_globals *object_unknown_0010a980_globals;

/* ---------- code */

/* 0x0010a980: what it keeps takes its memory of the state of the game, and no
map is loaded. The game has game_state_malloc written here */
void object_unknown_0010a980_initialize(
	void)
{
	struct object_unknown_0010a980_globals *globals= game_state_malloc(sizeof(struct object_unknown_0010a980_globals));

	object_unknown_0010a980_globals= globals;
	globals->map_initialized= FALSE;
}

/* 0x0010a9d0: it has no memory */
void object_unknown_0010a980_dispose(
	void)
{
	object_unknown_0010a980_globals= NULL;
}

/* 0x0010a9e0: what it keeps is as it starts, and a map is loaded */
void object_unknown_0010a980_initialize_for_new_map(
	void)
{
	struct object_unknown_0010a980_globals *globals= object_unknown_0010a980_globals;
	long index;

	for (index= 0; index<OBJECT_UNKNOWN_0010A980_COUNT; index++)
	{
		globals->unknown280[index]= NONE;
	}
	globals->map_initialized= TRUE;
}

/* 0x0010aab0: no map is loaded */
void object_unknown_0010a980_dispose_from_old_map(
	void)
{
	object_unknown_0010a980_globals->map_initialized= FALSE;
}
