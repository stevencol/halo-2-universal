/*
MEMORY_POOL.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The names are those of Halo 1's memory_pool.c where it has them.
*/

#include "memory_pool.h"

#include <string.h>

/* ---------- prototypes of functions of the console's libraries */

/* 0x003314b0 */
long GetTickCount(void);

/* ---------- code */

/* what a block takes of the pool, for a size: with what is before the
memory, in whole multiples of the alignment. In Halo 1,
memory_pool_block_compute_actual_size */
static inline __attribute__((always_inline)) unsigned long memory_pool_block_compute_actual_size(
	const struct memory_pool *pool,
	long size)
{
	unsigned long actual_size= (unsigned long)size+sizeof(struct memory_pool_block)+
		(pool->debug ? sizeof(struct memory_pool_block_debug) : 0);

	if (actual_size&(MEMORY_POOL_ALIGNMENT-1))
	{
		actual_size= (actual_size|(MEMORY_POOL_ALIGNMENT-1))+1;
	}

	return actual_size;
}

/* where what a block takes of the pool starts */
static inline __attribute__((always_inline)) byte *memory_pool_block_start(
	const struct memory_pool *pool,
	const struct memory_pool_block *block)
{
	return (byte *)block-(pool->debug ? sizeof(struct memory_pool_block_debug) : 0);
}

/* the block that starts at an address; in a pool that keeps track, with who
takes it written before it */
static inline __attribute__((always_inline)) struct memory_pool_block *memory_pool_block_at(
	const struct memory_pool *pool,
	byte *address,
	long unknown4,
	long unknown8)
{
	if (pool->debug)
	{
		struct memory_pool_block_debug *debug= (struct memory_pool_block_debug *)address;

		debug->signature= MEMORY_POOL_BLOCK_DEBUG_SIGNATURE;
		debug->unknown4= unknown4;
		debug->unknown8= unknown8;
		debug->time= (unsigned long)GetTickCount();
		address+= sizeof(struct memory_pool_block_debug);
	}

	return (struct memory_pool_block *)address;
}

/* 0x0018e1f0: a pool of a size, in memory of an allocation: the heap's if
none is given */
struct memory_pool *memory_pool_new(
	struct allocation *allocation,
	long size,
	const char *name)
{
	struct memory_pool *pool;

	if (!allocation)
	{
		allocation= heap_allocation;
	}
	pool= allocation->functions->allocate(allocation, (unsigned long)size+sizeof(struct memory_pool)+MEMORY_POOL_ALIGNMENT);
	if (pool)
	{
		memory_pool_initialize(pool, size, name, allocation);
	}

	return pool;
}

/* 0x0018e230 */
void memory_pool_delete(
	struct memory_pool *pool)
{
	struct allocation *allocation= pool->allocation;

	memset(pool, 0, sizeof(*pool));
	allocation->functions->deallocate(allocation, pool);
}

/* 0x0018e250: the memory of the pool is what is after it */
void memory_pool_initialize(
	struct memory_pool *pool,
	long size,
	const char *name,
	struct allocation *allocation)
{
	memset(pool, 0, sizeof(*pool));
	pool->signature= MEMORY_POOL_SIGNATURE;
	strncpy(pool->name, name, MEMORY_POOL_NAME_LENGTH+1);
	pool->name[MEMORY_POOL_NAME_LENGTH]= 0;
	pool->allocation= allocation;
	pool->base_address= (byte *)(((uintptr_t)pool+sizeof(*pool)+MEMORY_POOL_ALIGNMENT-1)&
		~(uintptr_t)(MEMORY_POOL_ALIGNMENT-1));
	pool->size= size;
	pool->free_size= size;
	pool->first_block= NULL;
	pool->last_block= NULL;
	pool->no_references= FALSE;
	pool->fill_gaps= FALSE;
	pool->debug= FALSE;
}

/* 0x0018e2b0: takes a block of the pool, and gives its owner's pointer the
address of its memory; whether there was room. The pool is not packed to
make room */
boolean memory_pool_block_allocate(
	struct memory_pool *pool,
	void **reference,
	long size,
	long unknown4,
	long unknown8)
{
	unsigned long actual_size= memory_pool_block_compute_actual_size(pool, size);
	struct memory_pool_block *block;

	if (pool->fill_gaps)
	{
		block= memory_pool_block_new_in_gap(pool, (long)actual_size, unknown4, unknown8);
		if (!block)
		{
			block= memory_pool_block_new_at_start(pool, (long)actual_size, unknown4, unknown8);
			if (!block)
			{
				block= memory_pool_block_new_at_end(pool, (long)actual_size, unknown4, unknown8);
			}
		}
	}
	else
	{
		block= memory_pool_block_new_at_end(pool, (long)actual_size, unknown4, unknown8);
	}
	if (!block)
	{
		return FALSE;
	}

	block->reference= pool->no_references ? NULL : reference;
	pool->free_size-= block->size;
	*reference= block+1;

	return TRUE;
}

/* 0x0018e340: changes the size of a block. If it does not have the room
after it, another block is taken, what the old one had goes there, and the
owner's pointer is changed; whether it could be done */
boolean memory_pool_block_reallocate(
	struct memory_pool *pool,
	void **reference,
	long new_size,
	long unknown4,
	long unknown8)
{
	struct memory_pool_block *block= (struct memory_pool_block *)*reference-1;
	unsigned long actual_size= memory_pool_block_compute_actual_size(pool, new_size);
	byte *end= block->next_block ? (byte *)block->next_block : pool->base_address+pool->size;
	void *new_memory;

	if ((byte *)block+actual_size<=end)
	{
		pool->free_size+= block->size-(long)actual_size;
		block->size= (long)actual_size;
		if (pool->debug)
		{
			struct memory_pool_block_debug *debug= (struct memory_pool_block_debug *)block-1;

			debug->unknown4= unknown4;
			debug->unknown8= unknown8;
			debug->time= (unsigned long)GetTickCount();
		}

		return TRUE;
	}

	if (!memory_pool_block_allocate(pool, &new_memory, new_size, unknown4, unknown8))
	{
		return FALSE;
	}
	memcpy(new_memory, *reference, (unsigned long)block->size-sizeof(struct memory_pool_block)-
		(pool->debug ? sizeof(struct memory_pool_block_debug) : 0));
	memory_pool_block_free(pool, reference);
	((struct memory_pool_block *)new_memory-1)->reference= pool->no_references ? NULL : reference;
	*reference= new_memory;

	return TRUE;
}

/* 0x0018e430: gives a block back. The owner's pointer is left as it is */
void memory_pool_block_free(
	struct memory_pool *pool,
	void **reference)
{
	struct memory_pool_block *block= (struct memory_pool_block *)*reference-1;

	pool->free_size+= block->size;
	if (block->previous_block)
	{
		block->previous_block->next_block= block->next_block;
	}
	else
	{
		pool->first_block= block->next_block;
	}
	if (block->next_block)
	{
		block->next_block->previous_block= block->previous_block;
	}
	else
	{
		pool->last_block= block->previous_block;
	}
}

/* 0x0018e470: packs the pool: each block goes right after the one before
it, and its owner's pointer is changed. A pool that does not know the
pointers is left as it is */
void memory_pool_compact(
	struct memory_pool *pool)
{
	if (!pool->no_references && pool->first_block)
	{
		struct memory_pool_block *block= pool->first_block;
		struct memory_pool_block *previous_block= NULL;
		byte *address= pool->base_address;

		do
		{
			byte *start= memory_pool_block_start(pool, block);

			if (start>address)
			{
				memmove(address, start, (unsigned long)block->size);
				start= address;
				block= (struct memory_pool_block *)(address+(pool->debug ? sizeof(struct memory_pool_block_debug) : 0));
				if (pool->no_references)
				{
					block->reference= NULL;
				}
				else
				{
					*block->reference= block+1;
				}
			}
			block->previous_block= previous_block;
			if (previous_block)
			{
				previous_block->next_block= block;
			}
			else
			{
				pool->first_block= block;
			}
			address= start+block->size;
			previous_block= block;
			block= block->next_block;
		}
		while (block);
		previous_block->next_block= NULL;
		pool->last_block= previous_block;
	}
}

/* 0x0018e4f0: makes a block at an address, between two blocks of the list
(either can be none); the block */
struct memory_pool_block *memory_pool_block_link(
	struct memory_pool *pool,
	void *address,
	long size,
	struct memory_pool_block *previous_block,
	long unknown4,
	struct memory_pool_block *next_block,
	long unknown8)
{
	struct memory_pool_block *block= memory_pool_block_at(pool, address, unknown4, unknown8);

	block->size= size;
	block->next_block= next_block;
	block->previous_block= previous_block;
	if (previous_block)
	{
		previous_block->next_block= block;
	}
	else
	{
		pool->first_block= block;
	}
	if (next_block)
	{
		next_block->previous_block= block;
	}
	else
	{
		pool->last_block= block;
	}

	return block;
}

/* 0x0018e560: a block after the last one, if there is room; NULL if not. In
Halo 1, memory_pool_block_new */
struct memory_pool_block *memory_pool_block_new_at_end(
	struct memory_pool *pool,
	long size,
	long unknown4,
	long unknown8)
{
	struct memory_pool_block *last_block= pool->last_block;
	struct memory_pool_block *block;
	byte *address;

	if (last_block)
	{
		address= memory_pool_block_start(pool, last_block)+last_block->size;
	}
	else
	{
		address= pool->base_address;
	}
	if (address+size>pool->base_address+pool->size)
	{
		return NULL;
	}

	block= memory_pool_block_at(pool, address, unknown4, unknown8);
	block->size= size;
	block->next_block= NULL;
	block->previous_block= last_block;
	if (last_block)
	{
		last_block->next_block= block;
	}
	else
	{
		pool->first_block= block;
	}
	pool->last_block= block;

	return block;
}

/* 0x0018e600: a block before the first one, where the pool's memory
starts, if there is room; NULL if not */
struct memory_pool_block *memory_pool_block_new_at_start(
	struct memory_pool *pool,
	long size,
	long unknown4,
	long unknown8)
{
	struct memory_pool_block *first_block= pool->first_block;
	struct memory_pool_block *block;
	byte *end;

	if (first_block)
	{
		end= memory_pool_block_start(pool, first_block);
	}
	else
	{
		end= pool->base_address+pool->size;
	}
	if (pool->base_address+size>end)
	{
		return NULL;
	}

	block= memory_pool_block_at(pool, pool->base_address, unknown4, unknown8);
	block->size= size;
	block->next_block= first_block;
	block->previous_block= NULL;
	pool->first_block= block;
	if (first_block)
	{
		first_block->previous_block= block;
	}
	else
	{
		pool->last_block= block;
	}

	return block;
}

/* 0x0018e690: a block in the first place between two blocks that has the
room; NULL if there is none */
struct memory_pool_block *memory_pool_block_new_in_gap(
	struct memory_pool *pool,
	long size,
	long unknown4,
	long unknown8)
{
	struct memory_pool_block *block;

	for (block= pool->first_block; block; block= block->next_block)
	{
		struct memory_pool_block *next_block= block->next_block;

		if (next_block)
		{
			byte *end= memory_pool_block_start(pool, block)+block->size;

			if (end+size<=memory_pool_block_start(pool, next_block))
			{
				return memory_pool_block_link(pool, end, size, block, unknown4, next_block, unknown8);
			}
		}
	}

	return NULL;
}
