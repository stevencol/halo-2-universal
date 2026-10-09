/*
SIMULATION_ENTITY_DATABASE.H

What the things that the consoles of a game have in common are: for each
of the 1024, its kind and what was said of it when it was made and of how
it is now. The name of the file is a guess, and so are the names of its
functions.
*/

#ifndef __SIMULATION_ENTITY_DATABASE_H
#define __SIMULATION_ENTITY_DATABASE_H

#include "cseries/cseries.h"
#include "networking/replication_entity_manager.h"

/* ---------- constants */

enum
{
	MAXIMUM_SIMULATION_ENTITIES= MAXIMUM_REPLICATION_ENTITIES
};

/* ---------- structures */

/* one of the things: its number, or NONE; its kind, or NONE; two bytes, of
which the first says that it is there to be told of; a number; the bits of
what has changed in it; and what was said of it when it was made, and of how
it is now, each of a size and taken from the heap of the network */
struct simulation_entity
{
	long entity_index;
	short type;
	boolean exists;
	boolean unknown7;
	long unknown8;
	unsigned long pending;
	long creation_size;
	void *creation_data;
	long state_size;
	void *state_data;
};

typedef char verify_simulation_entity_size[sizeof(struct simulation_entity) == 0x20 || sizeof(void *) != 4 ? 1 : -1];

/* a kind of thing: something with functions, of which the twenty-first is
asked what of a thing is to be told of, from the bits of what changed */
struct simulation_entity_type;

struct simulation_entity_type_functions
{
	void *unknown0[20];
	boolean (__attribute__((thiscall)) *get_update)(struct simulation_entity_type *type,
		struct simulation_entity *entity, unsigned long *flags, long state_size, void *state_data);
};

struct simulation_entity_type
{
	const struct simulation_entity_type_functions *functions;
};

/* the kinds that there are */
struct simulation_entity_types
{
	long unknown0;
	struct simulation_entity_type *types[1];
};

struct simulation_entity_database
{
	long unknown0;
	/* whether the things have been set up once, and whether they are being
	set up now */
	boolean initialized;
	boolean resetting;
	long unknown8;
	struct replication_entity_manager *manager;
	struct simulation_entity_types *types;
	struct simulation_entity entities[MAXIMUM_SIMULATION_ENTITIES];
};

typedef char verify_simulation_entity_database_size[sizeof(struct simulation_entity_database) == 0x8014 ||
	sizeof(void *) != 4 ? 1 : -1];

/* ---------- prototypes/SIMULATION_ENTITY_DATABASE.C */

void simulation_entity_database_reset(struct simulation_entity_database *database);
void __attribute__((stdcall)) simulation_entity_database_flush_updates(struct simulation_entity_database *database);
void __attribute__((stdcall)) simulation_entity_database_set_entity(long entity_index,
	struct simulation_entity_database *database, short type, long creation_size, void *creation_data, long state_size,
	void *state_data);
void simulation_entity_database_free_entity(long entity_index, struct simulation_entity_database *database);

#endif
