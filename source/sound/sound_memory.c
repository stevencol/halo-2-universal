/*
SOUND_MEMORY.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The memory that the console's libraries of the sound ask for, which comes
from one of two pools of the game. The name of the file is a guess, and so
are the names of its functions.
*/

#include "sound_memory.h"

/* ---------- globals */

/* 0x004c995c, 0x004c9960 and 0x004c9964: the pools, and whether the second
one is used */
struct memory_pool *sound_memory_pool;
struct memory_pool *sound_memory_other_pool;
boolean sound_memory_use_other_pool;

/* ---------- code */

/* the pool that the memory comes from now */
static inline __attribute__((always_inline)) struct memory_pool *sound_memory_get_pool(
	void)
{
	return sound_memory_use_other_pool ? sound_memory_other_pool : sound_memory_pool;
}

/* 0x00053290: a block of the pool; NULL if there is no room. An allocation
of no size gets 0x200 bytes. What is asked for of it is not looked at */
void *sound_memory_allocate(
	unsigned long size,
	unsigned long attributes)
{
	void *block= NULL;

	(void)attributes;

	if (!size)
	{
		size= SOUND_MEMORY_DEFAULT_SIZE;
	}
	memory_pool_block_allocate(sound_memory_get_pool(), &block, (long)size, 0, 0);

	return block;
}

/* 0x000532e0: gives back a block that sound_memory_allocate gave */
void __attribute__((stdcall)) sound_memory_free(
	void *address,
	unsigned long attributes)
{
	(void)attributes;

	if (address)
	{
		memory_pool_block_free(sound_memory_get_pool(), &address);
	}
}
