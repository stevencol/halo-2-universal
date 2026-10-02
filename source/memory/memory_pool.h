/*
MEMORY_POOL.H

A block of memory that blocks of any size are taken from, one after another,
and that can be packed again when blocks are given back: each block knows
the pointer that its owner has to it, to change it when the block moves. As
Halo 1's, without the signatures of the blocks and with three options more;
the names of what Halo 1 does not have are guesses.
*/

#ifndef __MEMORY_POOL_H
#define __MEMORY_POOL_H

#include <stdint.h>

#include "cseries/cseries.h"
#include "memory/data.h"

/* ---------- constants */

#define MEMORY_POOL_SIGNATURE 'pool'
#define MEMORY_POOL_BLOCK_DEBUG_SIGNATURE 'head'

enum
{
	MEMORY_POOL_NAME_LENGTH= 31,
	/* the blocks start at addresses that are multiples of this, and their
	sizes are multiples of it */
	MEMORY_POOL_ALIGNMENT= 16
};

/* ---------- structures */

/* before the memory of each block */
struct memory_pool_block
{
	/* with what is before the memory */
	long size;
	/* the owner's pointer to the memory; NULL in a pool that keeps none */
	void **reference;
	struct memory_pool_block *next_block;
	struct memory_pool_block *previous_block;
};

/* before each block, in a pool that keeps track of who takes them */
struct memory_pool_block_debug
{
	unsigned long signature;
	long unknown4;
	long unknown8;
	/* GetTickCount: when it was taken */
	unsigned long time;
};

struct memory_pool
{
	unsigned long signature;
	char name[MEMORY_POOL_NAME_LENGTH+1];
	struct allocation *allocation;
	byte *base_address;
	long size;
	long free_size;
	/* in the order of their addresses */
	struct memory_pool_block *first_block;
	struct memory_pool_block *last_block;
	/* the blocks do not know their owners' pointers: they cannot be moved */
	boolean no_references;
	/* a new block goes where there is room between two, or before the
	first, and only then after the last */
	boolean fill_gaps;
	byte unknown3E;
	/* each block has a memory_pool_block_debug before it */
	boolean debug;
};

#if defined(__i386__)
typedef char verify_memory_pool_block_size[
	sizeof(struct memory_pool_block) == 0x10 ? 1 : -1];
typedef char verify_memory_pool_size[
	sizeof(struct memory_pool) == 0x40 ? 1 : -1];
#endif
typedef char verify_memory_pool_block_debug_size[
	sizeof(struct memory_pool_block_debug) == 0x10 ? 1 : -1];

/* ---------- prototypes/MEMORY_POOL.C */

struct memory_pool *memory_pool_new(struct allocation *allocation, long size, const char *name);
void memory_pool_delete(struct memory_pool *pool);
void memory_pool_initialize(struct memory_pool *pool, long size, const char *name, struct allocation *allocation);
boolean memory_pool_block_allocate(struct memory_pool *pool, void **reference, long size, long unknown4,
	long unknown8);
boolean memory_pool_block_reallocate(struct memory_pool *pool, void **reference, long new_size, long unknown4,
	long unknown8);
void memory_pool_block_free(struct memory_pool *pool, void **reference);
void memory_pool_compact(struct memory_pool *pool);

struct memory_pool_block *memory_pool_block_link(struct memory_pool *pool, void *address, long size,
	struct memory_pool_block *previous_block, long unknown4, struct memory_pool_block *next_block, long unknown8);
struct memory_pool_block *memory_pool_block_new_at_end(struct memory_pool *pool, long size, long unknown4,
	long unknown8);
struct memory_pool_block *memory_pool_block_new_at_start(struct memory_pool *pool, long size, long unknown4,
	long unknown8);
struct memory_pool_block *memory_pool_block_new_in_gap(struct memory_pool *pool, long size, long unknown4,
	long unknown8);

#endif
