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

/* 0x0008a210: so many new things at once, four at the most, each of a kind:
memory for each, places and numbers for them all
(replication_entity_manager_create_entities), and each is of its kind with
its memory. Their numbers are given; FALSE, with the memory taken back, if
there is no memory or no places. The game has
simulation_entity_database_set_entity written here */
boolean __attribute__((stdcall)) simulation_entity_database_create_entities(
	long *entities,
	struct simulation_entity_database *database,
	long count,
	const long *types)
{
	void *creation_data[MAXIMUM_REPLICATION_ENTITY_CHAIN]= {NULL};
	void *state_data[MAXIMUM_REPLICATION_ENTITY_CHAIN]= {NULL};
	long creation_size[MAXIMUM_REPLICATION_ENTITY_CHAIN]= {0};
	long state_size[MAXIMUM_REPLICATION_ENTITY_CHAIN]= {0};
	boolean result= TRUE;
	long index;

	for (index= 0; index<count; index++)
	{
		if (!simulation_entity_database_allocate_creation_data(database, types[index], &creation_size[index],
			&creation_data[index]) ||
			!simulation_entity_database_allocate_state_data(database, types[index], &state_size[index], &state_data[index]))
		{
			result= FALSE;
			break;
		}
	}
	if (result && !replication_entity_manager_create_entities(database->manager, count, entities))
	{
		result= FALSE;
	}
	if (!result)
	{
		for (index= 0; index<count; index++)
		{
			if (creation_data[index])
			{
				network_heap_free(creation_data[index]);
			}
			if (state_data[index])
			{
				network_heap_free(state_data[index]);
			}
		}

		return FALSE;
	}
	for (index= 0; index<count; index++)
	{
		simulation_entity_database_set_entity(entities[index], database, (short)types[index], creation_size[index],
			creation_data[index], state_size[index], state_data[index]);
	}

	return TRUE;
}

/* 0x0008a920: a thing that another console made is here, with a number
and of a kind: it has what came with it, which is not what came's any more
(so many pairs of numbers of it are zeros: one, and the count that is
given). Its kind is told, by its twenty-third function, and says whether the
thing is there to be told of. It is the third function of what the things
are of, which the table at 0x00450d60 has, as the ones below are, and
Ghidra's list has none of them; the game has
simulation_entity_database_set_entity written here */
void __attribute__((stdcall)) simulation_entity_database_created_remote(
	struct simulation_entity_database *database,
	long entity_index,
	long type_index,
	long unknown,
	long count,
	struct simulation_entity_creation *creation)
{
	struct simulation_entity *entity= &database->entities[entity_index&REPLICATION_ENTITY_INDEX_MASK];
	long creation_size= creation->creation_size;
	void *creation_data= creation->creation_data;
	long state_size= creation->state_size;
	void *state_data= creation->state_data;
	struct simulation_entity_type *type;

	memset(creation, 0, (unsigned long)count*2*sizeof(long));
	type= database->types->types[type_index];
	simulation_entity_database_set_entity(entity_index, database, (short)type_index, creation_size, creation_data,
		state_size, state_data);
	entity->exists= type->functions->created_remote(type, entity, creation_size, creation_data, unknown, entity->state_size,
		state_data)!=0;
}

/* 0x0008ac40: how the thing of a number is is what came: a copy of it; and
its kind is told, by its twenty-fourth function, with a number. What the
kind says. The eighth function of the table */
long __attribute__((stdcall)) simulation_entity_database_set_state(
	struct simulation_entity_database *database,
	long entity_index,
	long unknown4,
	long unknown8,
	const struct simulation_entity_update *update)
{
	struct simulation_entity *entity= &database->entities[entity_index&REPLICATION_ENTITY_INDEX_MASK];
	struct simulation_entity_type *type= database->types->types[entity->type];

	memcpy(entity->state_data, update->state_data, entity->state_size);

	return type->functions->state_set(type, entity, unknown4, entity->state_size, entity->state_data);
}

/* 0x0008aca0: a real of the thing of a number: nothing if its second byte
is more than nothing, and 0.85 if not. The tenth function of the table */
void __attribute__((stdcall)) simulation_entity_database_unknown_0008aca0(
	struct simulation_entity_database *database,
	long entity_index,
	long unknown,
	real *result)
{
	const struct simulation_entity *entity= &database->entities[entity_index&REPLICATION_ENTITY_INDEX_MASK];

	*result= (signed char)entity->unknown7>0 ? 0.0f : 0.85f;
}

/* 0x0008ace0: the thing of a number is being done with: if it is there to
be told of, its kind is told, by its twenty-fifth function, and it is not
there any more, with nothing changed. The eleventh function of the table */
void __attribute__((stdcall)) simulation_entity_database_deleting(
	struct simulation_entity_database *database,
	long entity_index)
{
	struct simulation_entity *entity= &database->entities[entity_index&REPLICATION_ENTITY_INDEX_MASK];

	if (entity->exists)
	{
		struct simulation_entity_type *type= database->types->types[entity->type];

		type->functions->deleting(type, entity);
		entity->exists= FALSE;
		entity->pending= 0;
	}
}

/* 0x0008ad20: the thing of a number is done with
(simulation_entity_database_free_entity). The thirteenth function of the
table */
void __attribute__((stdcall)) simulation_entity_database_deleted(
	struct simulation_entity_database *database,
	long entity_index)
{
	simulation_entity_database_free_entity(entity_index, database);
}

/* 0x0008ad30: whether the thing of a number, which another console made,
can be this one's: what its kind says, by its twenty-sixth function. The
fourteenth function of the table */
boolean __attribute__((stdcall)) simulation_entity_database_adopt(
	struct simulation_entity_database *database,
	long entity_index)
{
	struct simulation_entity *entity= &database->entities[entity_index&REPLICATION_ENTITY_INDEX_MASK];
	struct simulation_entity_type *type= database->types->types[entity->type];

	return type->functions->adopt(type, entity)!=0;
}

/* 0x0008ad70: the things that another console made are this one's: the
number of each thing that there is says that its place has been used once
more, as replication_entity_manager_adopt_entities does with its own, and
its kind is told, by its twenty-second function. The fifteenth function of
the table */
void simulation_entity_database_adopted(
	struct simulation_entity_database *database)
{
	long index;

	for (index= 0; index<MAXIMUM_SIMULATION_ENTITIES; index++)
	{
		struct simulation_entity *entity= &database->entities[index];

		if (entity->entity_index!=NONE)
		{
			struct simulation_entity_type *type= database->types->types[entity->type];
			unsigned long entity_index= (unsigned long)entity->entity_index;

			entity->entity_index= (long)((((entity_index>>REPLICATION_ENTITY_SALT_SHIFT)+1)%REPLICATION_ENTITY_SALT_COUNT)<<
				REPLICATION_ENTITY_SALT_SHIFT|(entity_index&REPLICATION_ENTITY_INDEX_MASK));
			type->functions->adopted(type, entity);
		}
	}
}

/* 0x0008a6f0: a thing that another console made is read from a packet: its
kind, in five bits; memory for what was said of it when it was made, which
the kind reads by its fourteenth function, and for how it is, which the kind
builds from that; and, if the kind has bits of what changes and a bit of the
packet says so, how it is, which the kind reads by its sixteenth. The kind
and the bits that were read are given, and the two pieces of memory are the
next two of a list, which has two more. How it went: well; there is no
memory (2); or what was read is not good (3), which is also what is said of
a kind that there is not. With that the memory is taken back. It is the
second function of what the things are of, which the table at 0x00450d60
has, as the one below is, and Ghidra's list has it */
long __attribute__((stdcall)) simulation_entity_database_decode_creation(
	struct simulation_entity_database *database,
	long unknown4,
	long *type_index_out,
	unsigned long *flags_out,
	long unknown10,
	long *block_count,
	struct simulation_entity_block *blocks,
	struct bitstream *stream)
{
	long type_index= (long)bitstream_read_integer(stream, SIMULATION_ENTITY_TYPE_BITS);
	struct simulation_entity_type *type= database->types->types[type_index];
	long result= _simulation_entity_read_bad;
	void *creation_data= NULL;
	void *state_data;
	long creation_size;
	long state_size;
	unsigned long all;
	unsigned long flags;

	if (!type)
	{
		return _simulation_entity_read_bad;
	}
	creation_size= type->functions->creation_size(type);
	state_size= type->functions->state_size(type);
	if (creation_size>0)
	{
		creation_data= network_heap_allocate_block(creation_size);
		if (!creation_data)
		{
			result= _simulation_entity_read_no_memory;
		}
	}
	state_data= network_heap_allocate(state_size);
	if (!state_data)
	{
		result= _simulation_entity_read_no_memory;
	}
	if (creation_size!=0 && !creation_data)
	{
		goto free_state;
	}
	if (!state_data)
	{
		goto free_creation;
	}
	if (creation_size>0)
	{
		memset(creation_data, 0, creation_size);
	}
	if (!type->functions->read_creation(type, creation_size, creation_data, stream) ||
		!type->functions->build_state(type, creation_size, creation_data, state_size, state_data))
	{
		result= _simulation_entity_read_bad;
		goto free_creation;
	}
	all= type->functions->state_flags(type);
	flags= 0;
	if (all!=0 && bitstream_read_bool(stream))
	{
		if (!type->functions->decode_state(type, TRUE, &flags, state_size, state_data, stream) || (flags&~all))
		{
			result= _simulation_entity_read_bad;
			goto free_creation;
		}
	}
	*type_index_out= type_index;
	*flags_out= flags;
	blocks[*block_count].type= _simulation_entity_block_creation;
	blocks[*block_count].size= (short)creation_size;
	blocks[*block_count].data= creation_data;
	blocks[*block_count+1].type= _simulation_entity_block_state;
	blocks[*block_count+1].size= (short)state_size;
	blocks[*block_count+1].data= state_data;
	*block_count+= 2;

	return _simulation_entity_read_good;

free_creation:
	if (creation_data)
	{
		network_heap_free(creation_data);
	}
free_state:
	if (state_data)
	{
		network_heap_free(state_data);
	}

	return result;
}

/* 0x0008aac0: how the thing of a number is now is read from a packet, into
memory of the heap of the network: a copy of how it is here, or, in a stream
that is in its fourth state, what its kind builds from what was said of it
when it was made; and then what the kind reads, by its sixteenth function.
The bits that were read are given, and the memory is the next of a list,
which has one more. How it went: well, or not (3): there is no such thing,
or no memory, or the kind says no; and then the memory is taken back. The
seventh function of the table */
long __attribute__((stdcall)) simulation_entity_database_decode_update(
	struct simulation_entity_database *database,
	long entity_index,
	unsigned long *flags_out,
	long unknownC,
	long *block_count,
	struct simulation_entity_block *blocks,
	struct bitstream *stream)
{
	struct simulation_entity *entity= &database->entities[entity_index&REPLICATION_ENTITY_INDEX_MASK];
	struct simulation_entity_type *type;
	unsigned long flags= 0;
	void *state_data;

	if (entity_index==NONE || entity->entity_index!=entity_index)
	{
		return _simulation_entity_read_bad;
	}
	type= database->types->types[entity->type];
	state_data= network_heap_allocate(entity->state_size);
	if (!state_data)
	{
		return _simulation_entity_read_bad;
	}
	if (stream->unknownC==_simulation_entity_stream_from_creation)
	{
		memset(state_data, 0, entity->state_size);
		if (!type->functions->build_state(type, entity->creation_size, entity->creation_data, entity->state_size,
			state_data))
		{
			network_heap_free(state_data);

			return _simulation_entity_read_bad;
		}
	}
	else
	{
		memcpy(state_data, entity->state_data, entity->state_size);
	}
	if (!type->functions->decode_state(type, FALSE, &flags, entity->state_size, state_data, stream))
	{
		network_heap_free(state_data);

		return _simulation_entity_read_bad;
	}
	*flags_out= flags;
	blocks[*block_count].type= _simulation_entity_block_state;
	blocks[*block_count].size= (short)entity->state_size;
	blocks[*block_count].data= state_data;
	(*block_count)++;

	return _simulation_entity_read_good;
}
