/*
SIMULATION_GAMESTATE_ENTITIES.C

the objects of the game and the things that the simulation tells of them:
which is which. Worked out from halo2ship.exe; the name of the file is a
guess, by the one that later games have.
*/

#include "simulation_gamestate_entities.h"

#include "main/main.h"
#include "main/main_game.h"
#include "memory/data.h"
#include "networking/bitstream.h"
#include "networking/replication_entity_manager.h"
#include "networking/simulation.h"
#include "networking/simulation_entity_database.h"
#include "networking/simulation_world.h"
#include "objects/objects.h"

#include <string.h>

/* ---------- structures */

/* a kind of thing: it has functions, as something made by the game */
typedef boolean (__attribute__((thiscall)) *simulation_entity_type_proc)(void *type, struct simulation_entity *entity);

typedef boolean (__attribute__((thiscall)) *simulation_entity_type_object_proc)(void *type, long object_index);

/* ---------- globals */

/* 0x004cf784, of simulation.c: the two tables of kinds */
extern struct simulation_type_table *unknown_004cf784;

/* ---------- private code */

/* what the things are, in the memory of the thing that is going */
static inline __attribute__((always_inline)) struct simulation_entity_database *simulation_gamestate_entity_database(
	void)
{
	return (struct simulation_entity_database *)
		(*(byte *const *)(unknown_004cf770.unknownC+SIMULATION_WORLD_UNKNOWN4_OFFSET)+SIMULATION_WORLD_MEMORY_ENTITY_DATABASE_OFFSET);
}

/* the thing of an object, and the bits of it in what tells of the things
that are */
static inline __attribute__((always_inline)) long simulation_object_get_entity(
	long object_index,
	byte *flags)
{
	const struct object_header_datum *header=
		(const struct object_header_datum *)object_header_data->data+(object_index&0xFFFF);
	long entity_index= *(const long *)((const byte *)header->datum+SIMULATION_OBJECT_ENTITY_OFFSET);

	if (entity_index!=NONE)
	{
		*flags= simulation_gamestate_entity_database()->manager->entities[entity_index&(MAXIMUM_REPLICATION_ENTITIES-1)].flags;
	}

	return entity_index;
}

/* a player, by its place among the players of the game: the place with
what tells it from others that had it; FALSE with a place that is none or
has no player */
static inline __attribute__((always_inline)) boolean simulation_game_event_get_player(
	long absolute_index,
	long *player_index)
{
	short identifier;

	if (absolute_index<0 || absolute_index>=player_data->first_unallocated)
	{
		return FALSE;
	}
	identifier= *(const short *)((const byte *)player_data->data+player_data->size*absolute_index);
	if (identifier==0)
	{
		return FALSE;
	}
	*player_index= ((long)identifier<<16)|absolute_index;

	return TRUE;
}

/* ---------- code */

/* 0x000a58d0: the object of a thing that came in a message, by its number;
NONE with no number, with one that is not the one of the thing at its
place, with a thing whose kind says that it is of no object (the eighth of
its functions), or with one that has none */
long simulation_gamestate_entity_get_object(
	long entity_index)
{
	struct simulation_entity *entity;
	void *type;

	if (entity_index==NONE)
	{
		return NONE;
	}
	entity= &simulation_gamestate_entity_database()->entities[entity_index&(MAXIMUM_REPLICATION_ENTITIES-1)];
	if (entity->entity_index!=entity_index)
	{
		return NONE;
	}
	type= unknown_004cf784->types[entity->type];
	if (!(*(simulation_entity_type_proc **)type)[SIMULATION_ENTITY_TYPE_HAS_OBJECT_PROC](type, entity))
	{
		return NONE;
	}

	return entity->unknown8;
}

/* 0x000a5930: the thing of an object, for a message from the console that
rules the game: NONE with no object, with one that has none, or with one
whose thing was not made yet for the others */
long simulation_object_get_created_entity(
	long object_index)
{
	byte flags;
	long entity_index;

	if (object_index==NONE)
	{
		return NONE;
	}
	entity_index= simulation_object_get_entity(object_index, &flags);
	if (entity_index==NONE || !(flags&_replication_entity_created_bit))
	{
		return NONE;
	}

	return entity_index;
}

/* 0x000a5980: the same from a console that does not rule it: NONE with one
whose thing is marked as made here */
long simulation_object_get_remote_entity(
	long object_index)
{
	byte flags;
	long entity_index;

	if (object_index==NONE)
	{
		return NONE;
	}
	entity_index= simulation_object_get_entity(object_index, &flags);
	if (entity_index==NONE || (flags&_replication_entity_created_bit))
	{
		return NONE;
	}

	return entity_index;
}

/* 0x0019eb30: something that happened in the game is done */
void unknown_0019eb30(const long *event);

/* 0x000a5570: something that happened in the game is read from a message:
its kind (4 bits), a number (6), five numbers of 4 bits each of which a bit
can say is NONE, one more number (nothing, 1, or 32 bits, by two bits), and
for the ninth kind 4 bits, less one. It is one of the functions of a kind
of thing; always TRUE */
boolean __attribute__((stdcall)) simulation_game_event_decode(
	long unused,
	long *event,
	struct bitstream *stream)
{
	long index;

	(void)unused;
	event[0]= bitstream_read_integer(stream, 4);
	event[1]= bitstream_read_integer(stream, 6);
	for (index= 0; index<SIMULATION_GAME_EVENT_NUMBERS; index++)
	{
		event[2+index]= NONE;
		if (!bitstream_read_bool(stream))
		{
			event[2+index]= bitstream_read_integer(stream, 4);
		}
	}
	event[7]= 0;
	if (!bitstream_read_bool(stream))
	{
		event[7]= 1;
		if (!bitstream_read_bool(stream))
		{
			event[7]= bitstream_read_integer(stream, 32);
		}
	}
	if (event[0]==_simulation_game_event_kind9)
	{
		*(short *)&event[8]= (short)(bitstream_read_integer(stream, 4)-1);
	}
	else
	{
		*(short *)&event[8]= 0;
	}

	return TRUE;
}

/* 0x000a5650: something that happened in the game, as it came in a message,
is done (unknown_0019eb30): with its three players as the game knows them
and not by their places. FALSE, with nothing done, if one of them is at a
place that has no player. It is one of the functions of a kind of thing */
boolean __attribute__((stdcall)) simulation_game_event_apply(
	long unused0,
	long unused1,
	long unused2,
	const long *event)
{
	long copy[SIMULATION_GAME_EVENT_SIZE/sizeof(long)];

	(void)unused0;
	(void)unused1;
	(void)unused2;
	memcpy(copy, event, sizeof(copy));
	if (event[2]!=NONE && !simulation_game_event_get_player(event[2], &copy[2]))
	{
		return FALSE;
	}
	if (event[3]!=NONE && !simulation_game_event_get_player(event[3], &copy[3]))
	{
		return FALSE;
	}
	if (event[5]!=NONE && !simulation_game_event_get_player(event[5], &copy[5]))
	{
		return FALSE;
	}
	unknown_0019eb30(copy);

	return TRUE;
}

/* 0x000bb7b0, 0x000b8540: an object is made ready to go, and goes */
void unknown_000bb7b0(long object_index);
void __attribute__((stdcall)) unknown_000b8540(long object_index);

/* 0x000a5b40: a thing of the simulation is no more, and neither is its
object, unless its kind says that the object stays (the thirty-first of its
functions); the thing has no object then. FALSE if it had none. It is one
of the functions of a kind of thing */
boolean __attribute__((thiscall)) simulation_object_entity_delete_object(
	void *type,
	struct simulation_entity *entity)
{
	if (entity->unknown8==NONE)
	{
		return FALSE;
	}
	if (!(*(simulation_entity_type_object_proc **)type)[SIMULATION_ENTITY_TYPE_KEEP_OBJECT_PROC](type, entity->unknown8))
	{
		unknown_000bb7b0(entity->unknown8);
		unknown_000b8540(entity->unknown8);
	}
	entity->unknown8= NONE;

	return TRUE;
}
