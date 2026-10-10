/*
REPLICATION_ENTITY_MANAGER.H

What keeps the things that the consoles of a game have in common: 1024 of
them, each with a number, and up to fifteen views of them, one for each of
the other consoles. The name of the file is a guess, and so are the names of
its functions.
*/

#ifndef __REPLICATION_ENTITY_MANAGER_H
#define __REPLICATION_ENTITY_MANAGER_H

#include "cseries/cseries.h"
#include "networking/bitstream.h"

/* ---------- constants */

enum
{
	MAXIMUM_REPLICATION_ENTITIES= 0x400,
	MAXIMUM_REPLICATION_ENTITY_VIEWS= 15,
	/* how many things can be made at once, one after the other */
	MAXIMUM_REPLICATION_ENTITY_CHAIN= 4,
	/* the number of a thing: its place, in the low bits, and how many times
	the place has been used, in the high four */
	REPLICATION_ENTITY_INDEX_MASK= 0x3FF,
	REPLICATION_ENTITY_SALT_SHIFT= 28,
	REPLICATION_ENTITY_SALT_COUNT= 16,
	/* the bits of a thing: its place is in use; it is being done with; it
	was made; it is the first of the ones made at once, or one of the
	others */
	_replication_entity_used_bit= 0x1,
	_replication_entity_deleting_bit= 0x2,
	_replication_entity_created_bit= 0x4,
	_replication_entity_first_bit= 0x8,
	_replication_entity_other_bit= 0x10,
	/* the size of the last of the three things that go with a thing that
	another console made */
	REPLICATION_ENTITY_CREATION_DATA_SIZE= 0x10,
	/* of a view: which of the views it is; what keeps the things; what it
	has of each thing; and how many things it has something to say of. The
	state of a thing in a view in which that is counted */
	REPLICATION_ENTITY_VIEW_INDEX_OFFSET= 0xC,
	REPLICATION_ENTITY_VIEW_MANAGER_OFFSET= 0x14,
	REPLICATION_ENTITY_VIEW_ENTITIES_OFFSET= 0x24,
	REPLICATION_ENTITY_VIEW_PENDING_COUNT_OFFSET= 0x5034,
	_replication_entity_view_state3= 3
};

/* ---------- structures */

/* one of the things: its bits; how many times its place has been used; a
count of what still has it; and the number of the next of the ones that were
made with it */
struct replication_entity
{
	byte flags;
	byte salt;
	short unknown2;
	long next;
};

/* what a view has of a thing: its number; the bits of what it has to say
of it; the state that it is in there; the place of the next of the things
that are in some state in the view, or NONE; and more numbers */
struct replication_entity_view_entity
{
	long entity_index;
	unsigned long pending;
	short state;
	short next;
	long unknownC;
	short unknown10;
	short unknown12;
};

/* what the things are of: something with functions, of which the first
writes a thing in a packet for a console that does not have it; the third is
told that a thing was made by another console, with three things of it; the
eleventh and the thirteenth, that a thing is being done with, and that it is
done with; the fourteenth, that a thing that another console made is this
one's now, and says whether it can be; the fifteenth, that they all are; and
the sixteenth gives something of a thing for a view that is new, or
nothing */
struct replication_entity_client;

struct replication_entity_client_functions
{
	boolean (__attribute__((thiscall)) *write_creation)(struct replication_entity_client *client, long entity,
		unsigned long flags, long unknown, struct bitstream *stream, long unknown2, unsigned long *written);
	void *unknown4;
	void (__attribute__((thiscall)) *created)(struct replication_entity_client *client, long entity, long unknown4,
		long unknown8, long unknownC, void *unknown10);
	void *unknownC[7];
	void (__attribute__((thiscall)) *deleting)(struct replication_entity_client *client, long entity);
	void *unknown2C;
	void (__attribute__((thiscall)) *deleted)(struct replication_entity_client *client, long entity);
	boolean (__attribute__((thiscall)) *adopt)(struct replication_entity_client *client, long entity);
	void (__attribute__((thiscall)) *adopted)(struct replication_entity_client *client);
	long (__attribute__((thiscall)) *unknown3C)(struct replication_entity_client *client, long entity);
};

struct replication_entity_client
{
	const struct replication_entity_client_functions *functions;
};

struct replication_entity_manager
{
	struct replication_entity_client *client;
	byte *views[MAXIMUM_REPLICATION_ENTITY_VIEWS];
	/* a bit for each view that is told of the things */
	unsigned long view_mask;
	struct replication_entity entities[MAXIMUM_REPLICATION_ENTITIES];
	/* the place that the next thing is looked for from */
	long next_index;
};

typedef char verify_replication_entity_manager_size[sizeof(struct replication_entity_manager) == 0x2048 ||
	sizeof(void *) != 4 ? 1 : -1];

/* ---------- prototypes/REPLICATION_ENTITY_MANAGER.C */

void replication_entity_manager_new(struct replication_entity_manager *manager);
void __attribute__((stdcall)) replication_entity_manager_reset(struct replication_entity_manager *manager);
boolean __attribute__((stdcall)) replication_entity_manager_create_entities(struct replication_entity_manager *manager,
	long count, long *entities);
void __attribute__((stdcall)) replication_entity_manager_delete_entity(struct replication_entity_manager *manager,
	long entity);
void replication_entity_manager_delete_entity_now(struct replication_entity_manager *manager, long entity);
void replication_entity_manager_delete_chain(long entity, struct replication_entity_manager *manager);
void __attribute__((stdcall)) replication_entity_manager_delete_entities_now(struct replication_entity_manager *manager,
	long count, const long *entities);
void __attribute__((stdcall)) replication_entity_manager_add_view(byte *view, struct replication_entity_manager *manager,
	long view_index);
void __attribute__((stdcall)) replication_entity_manager_create_entities_remote(const long *unknown4,
	struct replication_entity_manager *manager, long count, const long *entities, const long *unknown8, byte *unknown10);
void __attribute__((stdcall)) replication_entity_manager_mark_entity(struct replication_entity_manager *manager,
	long entity, unsigned long flags);
void replication_entity_manager_adopt_entities(struct replication_entity_manager *manager);
long replication_entity_manager_allocate_index(struct replication_entity_manager *manager);
long replication_entity_manager_create_entity(long index, struct replication_entity_manager *manager);
long replication_entity_manager_get_chain(long entity, long *entities, const struct replication_entity_manager *manager);
long __attribute__((stdcall)) replication_entity_manager_find_chain(const struct replication_entity_manager *manager,
	long *entities, long entity);

#endif
