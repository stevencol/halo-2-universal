/*
LRUV_CACHE.H

A cache of blocks of pages: when there is no room for a new block, the
blocks that were used the longest ago give up theirs. As Halo 1's, with more
fields; the names of those are guesses.
*/

#ifndef __LRUV_CACHE_H
#define __LRUV_CACHE_H

#include "cseries/cseries.h"
#include "memory/data.h"

/* ---------- constants */

#define LRUV_CACHE_SIGNATURE 'weee'

enum
{
	LRUV_CACHE_NAME_LENGTH= 31,
	LRUV_CACHE_UNKNOWN44_COUNT= 8,
	/* the tick that the count goes back to when it reaches its end, and how
	far back a block is still told from another */
	LRUV_CACHE_WRAPPED_TICK= 0x101,
	LRUV_CACHE_WRAPPED_AGE= 0x100,
	LRUV_CACHE_MAXIMUM_TICK= 0x7FFFFFFF
};

/* ---------- structures */

#define LRUV_CALLBACK __attribute__((stdcall))

/* called when a block is deleted, and to ask whether a block cannot be */
typedef void (LRUV_CALLBACK *lruv_delete_block_proc)(long block_index);
typedef boolean (LRUV_CALLBACK *lruv_locked_block_proc)(long block_index);

struct lruv_cache
{
	char name[LRUV_CACHE_NAME_LENGTH+1];
	lruv_delete_block_proc delete_block_proc;
	lruv_locked_block_proc locked_block_proc;
	void *unknown28;
	/* how lruv_block_new chooses among the places for a block */
	long unknown2C;
	long page_count;
	long page_size_bits;
	long tick;
	/* the blocks are in a list, in the order of their pages */
	long first_block_index;
	long last_block_index;
	long unknown44[LRUV_CACHE_UNKNOWN44_COUNT];
	/* right after the cache */
	struct data_array *blocks;
	unsigned long signature;
	struct allocation *allocation;
};

struct lruv_cache_block
{
	short identifier;
	short flags;
	long page_count;
	long first_page_index;
	long next_block_index;
	long previous_block_index;
	long last_used_tick;
};

/* a run of pages that a new block could take: lruv_block_new looks at
several, and takes the best */
struct lruv_hole
{
	long unknown0;
	unsigned long last_used_tick;
	long unknown8;
	long page_count;
};

#if defined(__i386__)
typedef char verify_lruv_cache_size[
	sizeof(struct lruv_cache) == 0x70 ? 1 : -1];
#endif
typedef char verify_lruv_cache_block_size[
	sizeof(struct lruv_cache_block) == 0x18 ? 1 : -1];

/* ---------- prototypes/LRUV_CACHE.C */

void lruv_initialize(struct lruv_cache *cache, const char *name, long page_count, long page_size_bits,
	long maximum_block_count, lruv_delete_block_proc delete_block_proc, lruv_locked_block_proc locked_block_proc,
	void *unknown28, struct allocation *allocation);
void lruv_wrap_ticks(struct lruv_cache *cache);
void lruv_flush(struct lruv_cache *cache);
boolean lruv_hole_is_better(const struct lruv_cache *cache, const struct lruv_hole *hole,
	const struct lruv_hole *other);
void lruv_block_delete(struct lruv_cache *cache, long block_index);
void lruv_resize(struct lruv_cache *cache, long page_count);
long lruv_compute_pages_in_use(struct lruv_cache *cache, unsigned long age);

/* ---------- globals */

extern struct allocation *lruv_blocks_allocation;

#endif
