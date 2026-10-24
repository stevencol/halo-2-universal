/*
OBJECT_UNKNOWN_0010A980.C

a part of the game that keeps, in the state of the game, thirty-two things of 0x14 bytes, thirty-two numbers that are NONE for a new
map, and a byte that says that a map is loaded.
Worked out from halo2ship.exe; what it is for is not known, and the name of
the file is only the address of its first function.
*/

#include "object_unknown_0010a980.h"

#include "saved games/game_state.h"
#include "objects/objects.h"
#include "game/game.h"

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

/* 0x0010ace0: an object with the bit 9 of its flags at 0xC0 is taken out of
the list at 0x280, and the bit is cleared */
void object_unknown_0010a980_remove(
	long object_index)
{
	byte *object;
	long index;

	if (object_index==NONE)
	{
		return;
	}
	object= (byte *)((const struct object_header_datum *)object_header_data->data)[object_index&0xFFFF].datum;
	if (!((*(const unsigned short *)(object+0xC0)>>9)&1))
	{
		return;
	}
	for (index= 0; index<OBJECT_UNKNOWN_0010A980_COUNT; index++)
	{
		if (object_unknown_0010a980_globals->unknown280[index]==object_index)
		{
			object_unknown_0010a980_globals->unknown280[index]= NONE;
		}
	}
	object[0xC1]&= ~0x02;
}

/* 0x005107f4 (object_unknown_0010a980_globals) has, from 0x00, 32 entries of 0x14 bytes: a value, two reals
(from and to), a time and a length of time */

/* 0x0010aac0: the real of the entry of an object (in the list at 0x280) with
a value: between its two reals by the time gone (of game_time_globals), or
the last after its length; FALSE and NONE when there is none */
boolean __attribute__((stdcall)) object_unknown_0010a980_get_real(
	long object_index,
	long value,
	real *result,
	long *index)
{
	const struct object_unknown_0010a980_globals *globals;
	long which;

	*index= NONE;
	if (object_index==NONE)
	{
		return FALSE;
	}
	globals= object_unknown_0010a980_globals;
	for (which= 0; which<OBJECT_UNKNOWN_0010A980_COUNT; which++)
	{
		const long *entry= (const long *)(globals->unknown0+which*0x14);

		if (globals->unknown280[which]==object_index && entry[0]==value)
		{
			long length= entry[4];
			long gone= game_time_globals->time-entry[3];
			real from= ((const real *)entry)[1];
			real to= ((const real *)entry)[2];

			if (gone<length && length)
			{
				*result= (real)gone/(real)length*(to-from)+from;
			}
			else
			{
				*result= to;
			}
			*index= which;
			return TRUE;
		}
	}

	return FALSE;
}
