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
	LRUV_CACHE_AGE_CLASS_COUNT= 8,
	/* how many runs of pages lruv_block_new looks at at a time */
	LRUV_CACHE_HOLE_COUNT= 256,
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
	long hole_algorithm;
	long page_count;
	long page_size_bits;
	long tick;
	/* the blocks are in a list, in the order of their pages */
	long first_block_index;
	long last_block_index;
	/* per the age that a new block was asked with: the least pages that
	there was no room for; a new block of as many is not looked for again.
	LRUV_CACHE_MAXIMUM_TICK when the cache changes */
	long failed_page_counts[LRUV_CACHE_AGE_CLASS_COUNT];
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
	/* the block before the first page; NONE if there is none */
	long previous_block_index;
	/* of the blocks in it: the one used last */
	unsigned long last_used_tick;
	long first_page_index;
	long page_count;
};

/* the values of hole_algorithm */
enum
{
	/* the run used the longest ago; of two the same, the one of less pages */
	_lruv_hole_algorithm_oldest,
	/* lruv_hole_is_better */
	_lruv_hole_algorithm_oldest_with_margin,
	/* the run with the most age for each of its pages */
	_lruv_hole_algorithm_age_per_page
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
long lruv_block_new(struct lruv_cache *cache, long size, long age);
void lruv_block_delete(struct lruv_cache *cache, long block_index);
void lruv_resize(struct lruv_cache *cache, long page_count);
long lruv_compute_pages_in_use(struct lruv_cache *cache, unsigned long age);

/* ---------- globals */

extern struct allocation *lruv_blocks_allocation;

#endif
