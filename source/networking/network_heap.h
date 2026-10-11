/*
NETWORK_HEAP.H

The heap of the network: where what the consoles of a game tell each other
of is kept while it waits. The name of the file is a guess.
*/

#ifndef __NETWORK_HEAP_H
#define __NETWORK_HEAP_H

#include "cseries/cseries.h"

/* ---------- structures */

/* what gives and takes memory: something with functions, of which the first
takes some back; the second says how much something is, if it knows; the
sixth gives so much; and the eleventh makes room */
struct network_heap_allocator;

struct network_heap_allocator_functions
{
	void (__attribute__((thiscall)) *free)(struct network_heap_allocator *allocator, void *memory, long unknown);
	boolean (__attribute__((thiscall)) *size)(struct network_heap_allocator *allocator, void *memory, long *size);
	void *unknown8[3];
	void *(__attribute__((thiscall)) *allocate)(struct network_heap_allocator *allocator, long size, long unknown4,
		long unknown8);
	void *unknown18[4];
	void (__attribute__((thiscall)) *compact)(struct network_heap_allocator *allocator, long unknown);
	void *unknown2C[2];
	/* the fourteenth: it is done with, and with a bit its memory is let go of */
	void (__attribute__((thiscall)) *dispose)(struct network_heap_allocator *allocator, long flags);
};

struct network_heap_allocator
{
	const struct network_heap_allocator_functions *functions;
};

/* the heap of the network: what gives its memory, and how many things it
has given that it has not taken back */
struct network_heap
{
	struct network_heap_allocator *allocator;
	long count;
	/* a byte, and a number that the first page of its memory points to */
	boolean unknown8;
	long unknownC;
};

/* ---------- globals */

/* 0x004d87f8 */
extern struct network_heap *unknown_004d87f8;

/* ---------- prototypes/NETWORK_HEAP.C */

void *network_heap_allocate_block(long size);

/* ---------- code */

/* the heap of the network gives so much memory, or NULL; if it has none it
makes room and tries once more. The game has this written wherever it does
it */
static inline __attribute__((always_inline)) void *network_heap_allocate(
	long size)
{
	struct network_heap *heap= unknown_004d87f8;
	void *memory= heap->allocator->functions->allocate(heap->allocator, size, 0, 0);

	if (!memory)
	{
		heap->allocator->functions->compact(heap->allocator, 0);
		memory= heap->allocator->functions->allocate(heap->allocator, size, 0, 0);
	}
	if (memory)
	{
		heap->count++;
	}

	return memory;
}

/* the heap of the network takes back something that it gave. It is asked
how much it is first, and nothing is done with that: the game has this
written wherever it does it */
static inline __attribute__((always_inline)) void network_heap_free(
	void *memory)
{
	struct network_heap *heap= unknown_004d87f8;
	long size;

	heap->allocator->functions->size(heap->allocator, memory, &size);
	heap= unknown_004d87f8;
	heap->allocator->functions->free(heap->allocator, memory, NONE);
	heap->count--;
}

#endif
