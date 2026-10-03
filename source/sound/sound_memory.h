/*
SOUND_MEMORY.H

The memory that the console's libraries of the sound ask for. The name of
the file is a guess, and so are the names of its functions.
*/

#ifndef __SOUND_MEMORY_H
#define __SOUND_MEMORY_H

#include "cseries/cseries.h"
#include "memory/memory_pool.h"

/* ---------- constants */

enum
{
	/* what is given to an allocation of no size */
	SOUND_MEMORY_DEFAULT_SIZE= 0x200
};

/* ---------- prototypes/SOUND_MEMORY.C */

void *sound_memory_allocate(unsigned long size, unsigned long attributes);
void __attribute__((stdcall)) sound_memory_free(void *address, unsigned long attributes);

/* ---------- globals */

extern struct memory_pool *sound_memory_pool;
extern struct memory_pool *sound_memory_other_pool;
extern boolean sound_memory_use_other_pool;

#endif
