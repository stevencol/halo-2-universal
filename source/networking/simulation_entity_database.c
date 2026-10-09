/*
SIMULATION_ENTITY_DATABASE.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

What the things that the consoles of a game have in common are: for each
of the 1024, its kind and what was said of it when it was made and of how
it is now. The name of the file is a guess, and so are the names of its
functions.
*/

#include "simulation_entity_database.h"

#include "networking/network_heap.h"

#include <string.h>

/* ---------- code */

/* 0x0008a030: there are no things. The first time they are all set to
nothing; after that, each one that there is is let go of
(simulation_entity_database_free_entity). A byte says that this is being
done, while it is */
void simulation_entity_database_reset(
	struct simulation_entity_database *database)
{
	long index;

	database->resetting= TRUE;
	if (!database->initialized)
	{
		memset(database->entities, 0, sizeof(database->entities));
	}
	for (index= 0; index<MAXIMUM_SIMULATION_ENTITIES; index++)
	{
		struct simulation_entity *entity= &database->entities[index];

		if (database->initialized)
		{
			if (entity->entity_index!=NONE)
			{
				simulation_entity_database_free_entity(entity->entity_index, database);
			}
		}
		else
		{
			entity->entity_index= NONE;
			entity->type= NONE;
		}
	}
	database->resetting= FALSE;
}

/* 0x0008a090: what has changed in the things is told of. For each thing
that there is with bits of what changed: if it is there to be told of, its
kind says, by its twenty-first function, what of it is to be told, from the
bits and how it is now; and if it says so, and there is something, every
view has that to say of it (replication_entity_manager_mark_entity). The
thing has no bits */
void __attribute__((stdcall)) simulation_entity_database_flush_updates(
	struct simulation_entity_database *database)
{
	long index;

	for (index= 0; index<MAXIMUM_SIMULATION_ENTITIES; index++)
	{
		struct simulation_entity *entity= &database->entities[index];

		if (entity->entity_index!=NONE && entity->pending!=0)
		{
			if (entity->exists)
			{
				struct simulation_entity_type *type= database->types->types[entity->type];
				unsigned long flags= entity->pending;

				if (type->functions->get_update(type, entity, &flags, entity->state_size, entity->state_data) && flags!=0)
				{
					replication_entity_manager_mark_entity(database->manager, entity->entity_index, flags);
				}
			}
			entity->pending= 0;
		}
	}
}

/* 0x0008ae80: the thing of a number is of a kind, with what was said of it
when it was made and of how it is now; it is not there to be told of yet,
and nothing has changed in it */
void __attribute__((stdcall)) simulation_entity_database_set_entity(
	long entity_index,
	struct simulation_entity_database *database,
	short type,
	long creation_size,
	void *creation_data,
	long state_size,
	void *state_data)
{
	struct simulation_entity *entity= &database->entities[entity_index&REPLICATION_ENTITY_INDEX_MASK];

	entity->entity_index= entity_index;
	entity->type= type;
	entity->exists= FALSE;
	entity->pending= 0;
	entity->unknown7= FALSE;
	entity->creation_size= creation_size;
	entity->creation_data= creation_data;
	entity->unknown8= NONE;
	entity->state_size= state_size;
	entity->state_data= state_data;
}

/* 0x0008aed0: the thing of a number is no more: the heap of the network
takes back what was said of it when it was made and of how it is now, if
there is any, and it has no number and no kind */
void simulation_entity_database_free_entity(
	long entity_index,
	struct simulation_entity_database *database)
{
	struct simulation_entity *entity= &database->entities[entity_index&REPLICATION_ENTITY_INDEX_MASK];

	if (entity->creation_data)
	{
		network_heap_free(entity->creation_data);
		entity->creation_data= NULL;
		entity->creation_size= 0;
	}
	if (entity->state_data)
	{
		network_heap_free(entity->state_data);
		entity->state_data= NULL;
		entity->state_size= 0;
	}
	entity->entity_index= NONE;
	entity->type= NONE;
}
