/*
NETWORK_HEAP.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The heap of the network: where what the consoles of a game tell each other
of is kept while it waits. The name of the file is a guess.
*/

#include "network_heap.h"

/* ---------- code */

/* 0x00096e90: the heap of the network gives so much memory, or NULL: what
network_heap_allocate is, where the game does not have it written in
place */
void *network_heap_allocate_block(
	long size)
{
	return network_heap_allocate(size);
}
