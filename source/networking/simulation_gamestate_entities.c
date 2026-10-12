/*
SIMULATION_GAMESTATE_ENTITIES.C

the objects of the game and the things that the simulation tells of them:
which is which. Worked out from halo2ship.exe; the name of the file is a
guess, by the one that later games have.
*/

#include "simulation_gamestate_entities.h"

#include "main/main_game.h"
#include "networking/replication_entity_manager.h"
#include "networking/simulation.h"
#include "networking/simulation_entity_database.h"
#include "networking/simulation_world.h"
#include "objects/objects.h"

/* ---------- structures */

/* a kind of thing: it has functions, as something made by the game */
typedef boolean (__attribute__((thiscall)) *simulation_entity_type_proc)(void *type, struct simulation_entity *entity);

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
