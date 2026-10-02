/*
LRUV_CACHE.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The names are those of Halo 1's lruv_cache.c where it has them. Not here
yet: lruv_block_new (0x0013d370).
*/

#include "lruv_cache.h"

#include <string.h>

/* ---------- constants */

enum
{
	/* in lruv_hole_is_better: no block counts as older than this, and two
	ages this close count as the same */
	LRUV_HOLE_MAXIMUM_AGE= 300,
	LRUV_HOLE_AGE_MARGIN= 150
};

/* ---------- globals */

/* 0x0046875c: what the data array of the blocks is given as its allocation.
The name is a guess */
struct allocation *lruv_blocks_allocation;

/* ---------- code */

/* the blocks of a cache, one after another: the absolute index of the next
one after an index, or NONE. The game has the loop in each function */
static inline __attribute__((always_inline)) long lruv_next_block(
	const struct data_array *blocks,
	long absolute_index)
{
	for (absolute_index++; absolute_index>=0 && absolute_index<blocks->first_unallocated; absolute_index++)
	{
		if (BIT_VECTOR_TEST_FLAG(blocks->in_use_bit_vector, absolute_index))
		{
			return absolute_index;
		}
	}

	return NONE;
}

static inline __attribute__((always_inline)) struct lruv_cache_block *lruv_block(
	const struct data_array *blocks,
	long absolute_index)
{
	return (struct lruv_cache_block *)((byte *)blocks->data+blocks->size*absolute_index);
}

static inline __attribute__((always_inline)) long lruv_block_index(
	const struct lruv_cache_block *block,
	long absolute_index)
{
	return (long)((unsigned long)(unsigned short)block->identifier<<16)|absolute_index;
}

/* 0x0013d170 */
void lruv_initialize(
	struct lruv_cache *cache,
	const char *name,
	long page_count,
	long page_size_bits,
	long maximum_block_count,
	lruv_delete_block_proc delete_block_proc,
	lruv_locked_block_proc locked_block_proc,
	void *unknown28,
	struct allocation *allocation)
{
	struct data_array *blocks= (struct data_array *)(cache+1);
	long index;

	data_initialize(blocks, name, maximum_block_count, sizeof(struct lruv_cache_block), 0, lruv_blocks_allocation);
	data_make_valid(blocks);

	memset(cache, 0, sizeof(*cache));
	strncpy(cache->name, name, sizeof(cache->name));
	cache->delete_block_proc= delete_block_proc;
	cache->unknown28= unknown28;
	cache->page_size_bits= page_size_bits;
	cache->locked_block_proc= locked_block_proc;
	cache->blocks= blocks;
	cache->tick= 1;
	cache->page_count= page_count;
	cache->signature= LRUV_CACHE_SIGNATURE;
	cache->name[LRUV_CACHE_NAME_LENGTH]= 0;
	cache->unknown2C= 0;
	cache->allocation= allocation;
	cache->first_block_index= NONE;
	cache->last_block_index= NONE;
	for (index= 0; index<LRUV_CACHE_UNKNOWN44_COUNT; index++)
	{
		cache->unknown44[index]= LRUV_CACHE_MAXIMUM_TICK;
	}
}

/* 0x0013d230: the count of ticks reached its end: starts it again, and gives
the blocks the ticks that keep their order, as far back as it matters. The
name is a guess */
void lruv_wrap_ticks(
	struct lruv_cache *cache)
{
	struct data_array *blocks= cache->blocks;
	long absolute_index;

	cache->tick= LRUV_CACHE_WRAPPED_TICK;
	for (absolute_index= lruv_next_block(blocks, NONE); absolute_index!=NONE;
		absolute_index= lruv_next_block(blocks, absolute_index))
	{
		struct lruv_cache_block *block= lruv_block(blocks, absolute_index);
		unsigned long age= (unsigned long)(LRUV_CACHE_MAXIMUM_TICK-block->last_used_tick);

		if (age>LRUV_CACHE_WRAPPED_AGE)
		{
			block->last_used_tick= 0;
		}
		else
		{
			block->last_used_tick= (long)(LRUV_CACHE_WRAPPED_AGE-age);
		}
	}
}

/* 0x0013d2b0: deletes every block */
void lruv_flush(
	struct lruv_cache *cache)
{
	struct data_array *blocks= cache->blocks;
	long absolute_index;

	for (absolute_index= lruv_next_block(blocks, NONE); absolute_index!=NONE;
		absolute_index= lruv_next_block(blocks, absolute_index))
	{
		lruv_block_delete(cache, lruv_block_index(lruv_block(blocks, absolute_index), absolute_index));
	}
}

/* 0x0013d320: whether a run of pages is better to take than another: the
one that was used longer ago, or among two used at about the same time, the
one of less pages. The name is a guess */
boolean lruv_hole_is_better(
	const struct lruv_cache *cache,
	const struct lruv_hole *hole,
	const struct lruv_hole *other)
{
	unsigned long age= (unsigned long)cache->tick-hole->last_used_tick;
	unsigned long other_age= (unsigned long)cache->tick-other->last_used_tick;

	if (age>LRUV_HOLE_MAXIMUM_AGE)
	{
		age= LRUV_HOLE_MAXIMUM_AGE;
	}
	if (other_age>LRUV_HOLE_MAXIMUM_AGE)
	{
		other_age= LRUV_HOLE_MAXIMUM_AGE;
	}

	if ((long)age+LRUV_HOLE_AGE_MARGIN<(long)other_age)
	{
		return FALSE;
	}
	if ((long)age>(long)other_age+LRUV_HOLE_AGE_MARGIN)
	{
		return TRUE;
	}

	return hole->page_count<other->page_count;
}

/* 0x0013d830 */
void lruv_block_delete(
	struct lruv_cache *cache,
	long block_index)
{
	struct lruv_cache_block *block= lruv_block(cache->blocks, block_index&0xFFFF);

	if (cache->delete_block_proc)
	{
		cache->delete_block_proc(block_index);
	}

	if (block->previous_block_index!=NONE)
	{
		lruv_block(cache->blocks, block->previous_block_index&0xFFFF)->next_block_index= block->next_block_index;
	}
	else
	{
		cache->first_block_index= block->next_block_index;
	}

	if (block->next_block_index!=NONE)
	{
		lruv_block(cache->blocks, block->next_block_index&0xFFFF)->previous_block_index=
			block->previous_block_index;
	}
	else
	{
		cache->last_block_index= block->previous_block_index;
	}

	datum_delete(cache->blocks, block_index);
}

/* 0x0013d8b0: gives the cache another count of pages: the blocks that go
past it are deleted */
void lruv_resize(
	struct lruv_cache *cache,
	long page_count)
{
	struct data_array *blocks= cache->blocks;
	long absolute_index;

	for (absolute_index= lruv_next_block(blocks, NONE); absolute_index!=NONE;
		absolute_index= lruv_next_block(blocks, absolute_index))
	{
		struct lruv_cache_block *block= lruv_block(blocks, absolute_index);

		if (block->first_page_index+block->page_count>page_count)
		{
			lruv_block_delete(cache, lruv_block_index(block, absolute_index));
		}
	}
	cache->page_count= page_count;
}

/* 0x0013d950: how many pages are of blocks that were used in the last so
many ticks, or that cannot be deleted. The name is a guess */
long lruv_compute_pages_in_use(
	struct lruv_cache *cache,
	unsigned long age)
{
	struct data_array *blocks= cache->blocks;
	long page_count= 0;
	long absolute_index;

	for (absolute_index= lruv_next_block(blocks, NONE); absolute_index!=NONE;
		absolute_index= lruv_next_block(blocks, absolute_index))
	{
		struct lruv_cache_block *block= lruv_block(blocks, absolute_index);

		if ((unsigned long)block->last_used_tick+age>=(unsigned long)cache->tick ||
			(cache->locked_block_proc && cache->locked_block_proc(lruv_block_index(block, absolute_index))))
		{
			page_count+= block->page_count;
		}
	}

	return page_count;
}
