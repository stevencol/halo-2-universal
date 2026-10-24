/*
OBJECT_EARLY_MOVERS.C

the objects that move before the others. Worked out from halo2ship.exe; the
name of the file is a guess, by the one that later games have.
*/

#include "object_early_movers.h"

#include "cache/cache_files.h"
#include "objects/objects.h"
#include "saved games/game_state.h"

#include <string.h>

/* ---------- globals */

/* 0x005107f0: what there is of them, in the state of the game */
struct object_early_movers_globals *object_early_movers_globals;

/* ---------- code */

/* 0x00109300: what there is of them takes its memory of the state of the
game, and is nothing. The game has game_state_malloc written here */
void object_early_movers_initialize(
	void)
{
	struct object_early_movers_globals *globals= game_state_malloc(sizeof(struct object_early_movers_globals));

	memset(globals, 0, sizeof(struct object_early_movers_globals));
	object_early_movers_globals= globals;
}

/* 0x00109350: it has no memory */
void object_early_movers_dispose(
	void)
{
	object_early_movers_globals= NULL;
}

/* 0x00109360: for a new map, it is nothing, and a map is loaded */
void object_early_movers_initialize_for_new_map(
	void)
{
	struct object_early_movers_globals *globals= object_early_movers_globals;

	memset(globals, 0, sizeof(struct object_early_movers_globals));
	globals->map_initialized= TRUE;
}

/* 0x00109380: no map is loaded */
void object_early_movers_dispose_from_old_map(
	void)
{
	object_early_movers_globals->map_initialized= FALSE;
}

/* 0x00109390: an object is one of those that move before the others, if
what it is says so and there is room for one more: it is marked, and noted */
void object_early_movers_add(
	long object_index)
{
	byte *object= (byte *)((const struct object_header_datum *)object_header_data->data)[object_index&0xFFFF].datum;
	const byte *definition= (const byte *)global_tag_instances[*(const unsigned long *)object&0xFFFF].address;
	struct object_early_movers_globals *globals;

	if (!(definition[OBJECT_EARLY_MOVER_DEFINITION_FLAGS_OFFSET]&_object_early_mover_definition_bit))
	{
		return;
	}
	globals= object_early_movers_globals;
	if (globals->count>=MAXIMUM_OBJECT_EARLY_MOVERS)
	{
		return;
	}
	object[OBJECT_EARLY_MOVER_OBJECT_FLAGS_OFFSET]|= _object_early_mover_object_bit;
	globals->object_indices[globals->count]= object_index;
	globals->count++;
}

/* 0x00109fd0: the three longs at 0x24 of the entry (0xE8 bytes) of an early
mover; FALSE for NONE or one that is not one */
boolean __attribute__((stdcall)) object_early_movers_unknown_00109fd0(
	long object_index,
	unsigned long *result)
{
	const struct object_early_movers_globals *globals;
	long index;

	if (object_index==NONE)
	{
		return FALSE;
	}
	globals= object_early_movers_globals;
	for (index= 0; index<globals->count; index++)
	{
		if (globals->object_indices[index]==object_index)
		{
			const unsigned long *values= (const unsigned long *)(globals->unknown0+index*0xE8+0x24);

			result[0]= values[0];
			result[1]= values[1];
			result[2]= values[2];
			return TRUE;
		}
	}

	return FALSE;
}
