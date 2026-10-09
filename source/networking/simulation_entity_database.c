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

/* 0x0008a400: the thing of a number is there to be told of. Its kind is
told, by its nineteenth function, with what was said of it when it was made;
builds how it is from that, by its twentieth; and all of its bits of what
changes are set (the processor shifts by the low five bits of how many) */
void simulation_entity_database_activate_entity(
	long entity_index,
	struct simulation_entity_database *database)
{
	struct simulation_entity *entity= &database->entities[entity_index&REPLICATION_ENTITY_INDEX_MASK];
	struct simulation_entity_type *type= database->types->types[entity->type];

	type->functions->created(type, entity, entity->creation_size, entity->creation_data);
	type->functions->build_state(type, entity->creation_size, entity->creation_data, entity->state_size,
		entity->state_data);
	entity->exists= TRUE;
	entity->pending= (1u<<(type->functions->state_bit_count(type)&31))-1;
}

/* 0x0008add0: what of the thing of a number is to be told to a console
that knows nothing of it: its kind builds how a thing is from what was said
of this one when it was made, in a buffer of zeros, and is asked what of it
is to be told, from all of its bits; nothing if it says no */
unsigned long __attribute__((stdcall)) simulation_entity_database_get_initial_update(
	struct simulation_entity_database *database,
	long entity_index)
{
	byte state[MAXIMUM_SIMULATION_ENTITY_STATE_SIZE];
	struct simulation_entity *entity= &database->entities[entity_index&REPLICATION_ENTITY_INDEX_MASK];
	struct simulation_entity_type *type= database->types->types[entity->type];
	long state_size= type->functions->state_size(type);
	unsigned long flags;

	memset(state, 0, state_size);
	type->functions->build_state(type, entity->creation_size, entity->creation_data, state_size, state);
	flags= (1u<<(type->functions->state_bit_count(type)&31))-1;
	if (!type->functions->get_update(type, entity, &flags, state_size, state))
	{
		return 0;
	}

	return flags;
}

/* 0x0008af70: memory for what is said of a thing of a kind when it is made,
of the size that the kind says, with zeros; with no size there is none, and
that is well. The size and the memory are given; FALSE if the heap of the
network has none */
boolean __attribute__((stdcall)) simulation_entity_database_allocate_creation_data(
	const struct simulation_entity_database *database,
	long type_index,
	long *size_out,
	void **data_out)
{
	struct simulation_entity_type *type= database->types->types[type_index];
	long size= type->functions->creation_size(type);
	boolean result= TRUE;
	void *data= NULL;

	if (size>0)
	{
		data= network_heap_allocate(size);
		if (data)
		{
			memset(data, 0, size);
		}
		else
		{
			result= FALSE;
		}
	}
	*size_out= size;
	*data_out= data;

	return result;
}

/* 0x0008b010: and for how a thing of a kind is; this is asked for whatever
the size */
boolean __attribute__((stdcall)) simulation_entity_database_allocate_state_data(
	const struct simulation_entity_database *database,
	long type_index,
	long *size_out,
	void **data_out)
{
	struct simulation_entity_type *type= database->types->types[type_index];
	long size= type->functions->state_size(type);
	void *data= network_heap_allocate(size);

	if (data)
	{
		memset(data, 0, size);
	}
	*size_out= size;
	*data_out= data;

	return data!=NULL;
}

/* 0x0008a110: a new thing of a kind: memory for what is said of it when it
is made and for how it is, a place and a number for it
(replication_entity_manager_allocate_index and _create_entity), and it is of
the kind with all that. Its number; NONE, with the memory taken back, if
there is no memory or no place */
long __attribute__((stdcall)) simulation_entity_database_create_entity(
	struct simulation_entity_database *database,
	long type_index)
{
	long creation_size= 0;
	void *creation_data= NULL;
	long state_size;
	void *state_data= NULL;

	if (simulation_entity_database_allocate_creation_data(database, type_index, &creation_size, &creation_data) &&
		simulation_entity_database_allocate_state_data(database, type_index, &state_size, &state_data))
	{
		struct replication_entity_manager *manager= database->manager;
		long index= replication_entity_manager_allocate_index(manager);

		if (index!=NONE)
		{
			long entity_index= replication_entity_manager_create_entity(index, manager);

			if (entity_index!=NONE)
			{
				simulation_entity_database_set_entity(entity_index, database, (short)type_index, creation_size, creation_data,
					state_size, state_data);

				return entity_index;
			}
		}
	}
	if (creation_data)
	{
		network_heap_free(creation_data);
	}
	if (state_data)
	{
		network_heap_free(state_data);
	}

	return NONE;
}

/* 0x0008a580: the kind of the thing of a number does something with it and
three numbers, by its eleventh function; what it says. It is one of the
functions of what the things are of, which the table at 0x00450d60 has, and
Ghidra's list does not have it */
long __attribute__((stdcall)) simulation_entity_database_unknown_0008a580(
	struct simulation_entity_database *database,
	long entity_index,
	long unknown4,
	long unknown8,
	long unknownC)
{
	struct simulation_entity *entity= &database->entities[entity_index&REPLICATION_ENTITY_INDEX_MASK];
	struct simulation_entity_type *type= database->types->types[entity->type];

	return type->functions->unknown28(type, entity, unknown4, unknown8, unknownC);
}

/* 0x0008a5c0: the thing of a number is written in a packet for a console
that does not have it: its kind, in five bits; what was said of it when it
was made, which its kind writes by its thirteenth function; and, if the kind
has bits of what changes, a bit that says whether any of the ones that are
asked for is one of them, and then how the thing is for those, which the kind
writes by its fifteenth. Whether it went well: the kind said so, and wrote
all the bits; the bits that it wrote are given. It is another of the
functions of the table that Ghidra's list does not have; the game has
bitstream_write_integer_checked and bitstream_write_bool written here */
boolean __attribute__((stdcall)) simulation_entity_database_write_creation(
	struct simulation_entity_database *database,
	long entity_index,
	unsigned long flags,
	long unknown,
	struct bitstream *stream,
	long unknown2,
	unsigned long *written)
{
	struct simulation_entity *entity= &database->entities[entity_index&REPLICATION_ENTITY_INDEX_MASK];
	struct simulation_entity_type *type= database->types->types[entity->type];
	unsigned long all;

	*written= 0;
	bitstream_write_integer_checked((unsigned long)(long)entity->type, SIMULATION_ENTITY_TYPE_BITS, stream);
	type->functions->write_creation(type, entity->creation_size, entity->creation_data, unknown, stream);
	all= type->functions->state_flags(type);
	if (all==0)
	{
		return TRUE;
	}
	flags&= all;
	bitstream_write_bool(stream, flags!=0);
	if (flags==0)
	{
		return TRUE;
	}

	return type->functions->encode_state(type, TRUE, flags, written, entity->state_size, entity->state_data, unknown,
		stream, unknown2) && *written==flags;
}

/* 0x0008aa50: how the thing of a number is is read from a packet, for some
bits, by the fifteenth function of its kind; whether it went well: the kind
said so, and read some bit; the bits that it read are given. Another of the
functions of the table that Ghidra's list does not have */
boolean __attribute__((stdcall)) simulation_entity_database_read_update(
	struct simulation_entity_database *database,
	long entity_index,
	unsigned long flags,
	long unknown,
	struct bitstream *stream,
	long unknown2,
	unsigned long *read)
{
	struct simulation_entity *entity= &database->entities[entity_index&REPLICATION_ENTITY_INDEX_MASK];
	struct simulation_entity_type *type= database->types->types[entity->type];

	*read= 0;

	return type->functions->encode_state(type, FALSE, flags, read, entity->state_size, entity->state_data, unknown, stream,
		unknown2) && *read!=0;
}
