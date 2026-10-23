/*
USER_INTERFACE_MEMORY.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The names are guesses.
*/

#include "user_interface_memory.h"

#include "main/main_game.h"

/* ---------- globals */

/* 0x0051e998 */
struct memory_pool *user_interface_memory_pool;

/* 0x0047d924: what gives the pool its memory */
struct allocation user_interface_allocation;

/* ---------- code */

/* 0x001a474c: makes the pool, when a map is loaded: its size is by how the
memory is shared out. A block of it is not moved, and a new one goes where
there is room among the others. With 0x001a479a, in the table of those that
take memory then (0x00453c00) */
void USER_INTERFACE_MEMORY_CALLBACK user_interface_memory_pool_new(
	long configuration)
{
	struct memory_pool *pool;
	long size;

	if (main_game_memory_configuration_is_session(configuration))
	{
		size= USER_INTERFACE_MEMORY_POOL_SESSION_SIZE;
	}
	else
	{
		size= configuration<=_main_game_memory_default ? USER_INTERFACE_MEMORY_POOL_SIZE :
			USER_INTERFACE_MEMORY_POOL_CAMPAIGN_SIZE;
	}

	pool= memory_pool_new(&user_interface_allocation, size, "ui memory pool");
	user_interface_memory_pool= pool;
	if (pool)
	{
		pool->no_references= TRUE;
		pool->fill_gaps= TRUE;
	}
}

/* 0x001a479a */
void user_interface_memory_pool_delete(
	void)
{
	if (user_interface_memory_pool)
	{
		memory_pool_delete(user_interface_memory_pool);
		user_interface_memory_pool= NULL;
	}
}

/* 0x001a47b1: memory of the pool; NULL if there is none. If there is no
room, the interface lets go of what it can and it is tried again */
void *user_interface_memory_allocate(
	long size,
	long unknown4,
	long unknown8)
{
	void *memory;

	if (!memory_pool_block_allocate(user_interface_memory_pool, &memory, size, unknown4, unknown8))
	{
		unknown_00147ebe();
		if (!memory_pool_block_allocate(user_interface_memory_pool, &memory, size, unknown4, unknown8))
		{
			memory= NULL;
		}
	}

	return memory;
}

/* 0x001a4826 */
void user_interface_memory_free(
	void *memory)
{
	memory_pool_block_free(user_interface_memory_pool, &memory);
}

/* 0x001a47fd: memory of the pool (user_interface_memory_allocate), with 0
for the other two */
void *__attribute__((stdcall)) user_interface_memory_allocate_simple(
	long size)
{
	return user_interface_memory_allocate(size, 0, 0);
}
