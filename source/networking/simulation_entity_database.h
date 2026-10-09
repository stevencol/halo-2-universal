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
#include "math/real_math.h"
#include "networking/bitstream.h"
#include "networking/replication_entity_manager.h"

/* ---------- constants */

enum
{
	MAXIMUM_SIMULATION_ENTITIES= MAXIMUM_REPLICATION_ENTITIES,
	/* the most that how a thing is can be, in bytes */
	MAXIMUM_SIMULATION_ENTITY_STATE_SIZE= 0x400,
	/* the bits that the kind of a thing is written in */
	SIMULATION_ENTITY_TYPE_BITS= 5,
	/* what reading a thing from a packet can say: it went well, there is no
	memory, or what was read is not good */
	_simulation_entity_read_good= 0,
	_simulation_entity_read_no_memory= 2,
	_simulation_entity_read_bad= 3,
	/* the kinds of memory that come with a thing: what was said of it when
	it was made, and how it is */
	_simulation_entity_block_creation= 0xC,
	_simulation_entity_block_state= 0xD,
	/* the state of a stream in which how a thing is is read from nothing,
	and not from how the thing is here */
	_simulation_entity_stream_from_creation= 4
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

/* a kind of thing: something with functions. The third and the fourth say
the sizes of how a thing of the kind is and of what is said of it when it is
made, and the fifth how many bits of what changes it has; the sixth, those
bits, all of them. The eleventh does something with a thing and three
numbers. The thirteenth writes in a packet what is said of a thing when it is
made, and the fourteenth reads it; the fifteenth writes or reads how it is,
for some bits, and says which it did, and the sixteenth reads it into memory
that it is given. The nineteenth is
told that a thing is there, with what was said of it when it was made; the
twentieth builds how it is from that; and the twenty-first is asked what of a
thing is to be told of, from the bits of what changed. The twenty-second is
told that a thing is this console's now; the twenty-third, that a thing that
another console made is here, and says whether it is there to be told of;
the twenty-fourth, that how a thing is was said again; the twenty-fifth, that
a thing is being done with; and the twenty-sixth says whether a thing that
another console made can be this one's */
struct simulation_entity_type;

struct simulation_entity_type_functions
{
	void *unknown0[2];
	long (__attribute__((thiscall)) *state_size)(struct simulation_entity_type *type);
	long (__attribute__((thiscall)) *creation_size)(struct simulation_entity_type *type);
	long (__attribute__((thiscall)) *state_bit_count)(struct simulation_entity_type *type);
	unsigned long (__attribute__((thiscall)) *state_flags)(struct simulation_entity_type *type);
	void *unknown18[4];
	long (__attribute__((thiscall)) *unknown28)(struct simulation_entity_type *type, struct simulation_entity *entity,
		long unknown4, long unknown8, long unknownC);
	void *unknown2C;
	void (__attribute__((thiscall)) *write_creation)(struct simulation_entity_type *type, long creation_size,
		void *creation_data, long unknown, struct bitstream *stream);
	boolean (__attribute__((thiscall)) *read_creation)(struct simulation_entity_type *type, long creation_size,
		void *creation_data, struct bitstream *stream);
	boolean (__attribute__((thiscall)) *encode_state)(struct simulation_entity_type *type, boolean write,
		unsigned long flags, unsigned long *done, long state_size, void *state_data, long unknown,
		struct bitstream *stream, long unknown2);
	boolean (__attribute__((thiscall)) *decode_state)(struct simulation_entity_type *type, boolean creation,
		unsigned long *flags, long state_size, void *state_data, struct bitstream *stream);
	void *unknown40[2];
	void (__attribute__((thiscall)) *created)(struct simulation_entity_type *type, struct simulation_entity *entity,
		long creation_size, void *creation_data);
	boolean (__attribute__((thiscall)) *build_state)(struct simulation_entity_type *type, long creation_size,
		void *creation_data, long state_size, void *state_data);
	boolean (__attribute__((thiscall)) *get_update)(struct simulation_entity_type *type,
		struct simulation_entity *entity, unsigned long *flags, long state_size, void *state_data);
	void (__attribute__((thiscall)) *adopted)(struct simulation_entity_type *type, struct simulation_entity *entity);
	boolean (__attribute__((thiscall)) *created_remote)(struct simulation_entity_type *type,
		struct simulation_entity *entity, long creation_size, void *creation_data, long unknown, long state_size,
		void *state_data);
	long (__attribute__((thiscall)) *state_set)(struct simulation_entity_type *type, struct simulation_entity *entity,
		long unknown, long state_size, void *state_data);
	void (__attribute__((thiscall)) *deleting)(struct simulation_entity_type *type, struct simulation_entity *entity);
	boolean (__attribute__((thiscall)) *adopt)(struct simulation_entity_type *type, struct simulation_entity *entity);
};

struct simulation_entity_type
{
	const struct simulation_entity_type_functions *functions;
};

/* what comes with a thing that another console made: what was said of it
when it was made and how it is, each of a size and in memory of the heap of
the network, which the thing keeps */
struct simulation_entity_creation
{
	short creation_size;
	void *creation_data;
	short state_size;
	void *state_data;
};

/* memory of the heap of the network that comes with a thing: its size,
its kind, and where it is */
struct simulation_entity_block
{
	short size;
	short type;
	void *data;
};

/* how a thing is, as it came: something, and where it is */
struct simulation_entity_update
{
	long unknown0;
	const void *state_data;
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
long __attribute__((stdcall)) simulation_entity_database_create_entity(struct simulation_entity_database *database,
	long type_index);
long __attribute__((stdcall)) simulation_entity_database_unknown_0008a580(struct simulation_entity_database *database,
	long entity_index, long unknown4, long unknown8, long unknownC);
boolean __attribute__((stdcall)) simulation_entity_database_write_creation(struct simulation_entity_database *database,
	long entity_index, unsigned long flags, long unknown, struct bitstream *stream, long unknown2, unsigned long *written);
boolean __attribute__((stdcall)) simulation_entity_database_read_update(struct simulation_entity_database *database,
	long entity_index, unsigned long flags, long unknown, struct bitstream *stream, long unknown2, unsigned long *read);
boolean __attribute__((stdcall)) simulation_entity_database_create_entities(long *entities,
	struct simulation_entity_database *database, long count, const long *types);
void __attribute__((stdcall)) simulation_entity_database_created_remote(struct simulation_entity_database *database,
	long entity_index, long type_index, long unknown, long count, struct simulation_entity_creation *creation);
long __attribute__((stdcall)) simulation_entity_database_set_state(struct simulation_entity_database *database,
	long entity_index, long unknown4, long unknown8, const struct simulation_entity_update *update);
void __attribute__((stdcall)) simulation_entity_database_unknown_0008aca0(struct simulation_entity_database *database,
	long entity_index, long unknown, real *result);
void __attribute__((stdcall)) simulation_entity_database_deleting(struct simulation_entity_database *database,
	long entity_index);
void __attribute__((stdcall)) simulation_entity_database_deleted(struct simulation_entity_database *database,
	long entity_index);
boolean __attribute__((stdcall)) simulation_entity_database_adopt(struct simulation_entity_database *database,
	long entity_index);
void simulation_entity_database_adopted(struct simulation_entity_database *database);
long __attribute__((stdcall)) simulation_entity_database_decode_creation(struct simulation_entity_database *database,
	long unknown4, long *type_index, unsigned long *flags, long unknown10, long *block_count,
	struct simulation_entity_block *blocks, struct bitstream *stream);
long __attribute__((stdcall)) simulation_entity_database_decode_update(struct simulation_entity_database *database,
	long entity_index, unsigned long *flags, long unknownC, long *block_count, struct simulation_entity_block *blocks,
	struct bitstream *stream);
void simulation_entity_database_activate_entity(long entity_index, struct simulation_entity_database *database);
unsigned long __attribute__((stdcall)) simulation_entity_database_get_initial_update(
	struct simulation_entity_database *database, long entity_index);
boolean __attribute__((stdcall)) simulation_entity_database_allocate_creation_data(
	const struct simulation_entity_database *database, long type_index, long *size, void **data);
boolean __attribute__((stdcall)) simulation_entity_database_allocate_state_data(
	const struct simulation_entity_database *database, long type_index, long *size, void **data);
void simulation_entity_database_free_entity(long entity_index, struct simulation_entity_database *database);

#endif
