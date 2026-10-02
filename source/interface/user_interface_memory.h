/*
USER_INTERFACE_MEMORY.H

The memory of the menus and of what else the interface shows: a memory pool
that is made when a map is loaded. Worked out from halo2ship.exe; the names
are guesses.
*/

#ifndef __USER_INTERFACE_MEMORY_H
#define __USER_INTERFACE_MEMORY_H

#include "cseries/cseries.h"
#include "memory/memory_pool.h"

/* ---------- constants */

enum
{
	/* the size of the pool: for a campaign in a session of the network game,
	for anything that is not a campaign, and for a campaign */
	USER_INTERFACE_MEMORY_POOL_SESSION_SIZE= 0x60000,
	USER_INTERFACE_MEMORY_POOL_SIZE= 0x100000,
	USER_INTERFACE_MEMORY_POOL_CAMPAIGN_SIZE= 0x18000
};

/* ---------- prototypes/USER_INTERFACE_MEMORY.C */

#define USER_INTERFACE_MEMORY_CALLBACK __attribute__((stdcall))

void USER_INTERFACE_MEMORY_CALLBACK user_interface_memory_pool_new(long configuration);
void user_interface_memory_pool_delete(void);
void *user_interface_memory_allocate(long size, long unknown4, long unknown8);
void user_interface_memory_free(void *memory);

/* ---------- prototypes of functions not decompiled yet */

/* 0x00147ebe: lets go of what the interface can do without, for there to be
room in its pool */
void unknown_00147ebe(void);

/* ---------- globals */

extern struct memory_pool *user_interface_memory_pool;
extern struct allocation user_interface_allocation;

#endif
