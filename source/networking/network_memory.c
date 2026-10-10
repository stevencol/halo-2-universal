/*
NETWORK_MEMORY.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The memory that the network of the game takes when a map is loaded and lets
go of when it is closed. The name of the file is a guess, and so are the
names of its functions.
*/

#include "network_memory.h"

#include "memory/data.h"
#include "networking/network_heap.h"
#include "networking/network_observer.h"
#include "networking/simulation_view.h"
#include "networking/simulation_world.h"

#include "math/real_math.h"

/* ---------- prototypes */

/* 0x00321379, of the console's libraries: memory is let go of */
void unknown_00321379(void *memory);

/* ---------- globals */

/* 0x004d87d0, of network_channel.c */
extern long unknown_004d87d0;

/* 0x005291a0: the observer of the network; only where it is is known here.
0x004d87e8: what the game in the network has at 0x4 (the things that the
consoles share, what happens, and what each thing is). 0x004d87fc, 0x004d87e0,
0x004d87c4, 0x004d87c8 and 0x004d87cc: more that the network has while a map
is loaded. 0x00450cb8, 0x00450d60: the functions of two things that
unknown_004d87e8 has */
byte unknown_005291a0[1];
byte *unknown_004d87e8;
void *unknown_004d87fc;
long unknown_004d87e0;
long unknown_004d87c4;
long unknown_004d87c8;
long unknown_004d87cc;
void *unknown_00450cb8[1];
void *unknown_00450d60[1];

/* ---------- code */

/* 0x00081f80: the network lets go of what it took for a map: it is the
first of the pair that main_game has for it (main_game_memory_users). The
observer lets go of the channels of the game
(network_observer_release_game_channels); the two things of unknown_004d87e8
that have functions get them again, as when something made by the game is
unmade; the two tables of the views are not good any more; the heap of the
network has what gives its memory done with, and is let go of; and all that
the network had is nothing */
void network_memory_free(
	void)
{
	struct network_heap *heap;

	network_observer_release_game_channels(unknown_005291a0);
	if (unknown_004d87e8)
	{
		*(void **)(unknown_004d87e8+NETWORK_MEMORY_UNKNOWN_004D87E8_UNKNOWNA0AC_OFFSET)= unknown_00450cb8;
		*(void **)(unknown_004d87e8+NETWORK_MEMORY_UNKNOWN_004D87E8_UNKNOWN2098_OFFSET)= unknown_00450d60;
		unknown_004d87e8= NULL;
	}
	if (unknown_004d87f0)
	{
		unknown_004d87f0->valid= FALSE;
		unknown_004d87f0= NULL;
	}
	if (unknown_004d87ec)
	{
		unknown_004d87ec->valid= FALSE;
		unknown_004d87ec= NULL;
	}
	if (unknown_004d87d4)
	{
		unknown_004d87d4= NULL;
	}
	if (unknown_004d87d8)
	{
		unknown_004d87d8= NULL;
	}
	if (unknown_004d87dc)
	{
		unknown_004d87dc= NULL;
	}
	heap= unknown_004d87f8;
	if (heap)
	{
		if (heap->allocator)
		{
			heap->allocator->functions->dispose(heap->allocator, 1);
			heap->allocator= NULL;
		}
		unknown_00321379(heap);
		unknown_004d87f8= NULL;
	}
	if (unknown_004d87fc)
	{
		unknown_004d87fc= NULL;
	}
	unknown_004d87d0= 0;
	unknown_004d87e0= 0;
	unknown_004d87e4= FALSE;
	unknown_004d87c4= 0;
	unknown_004d87c8= 0;
	unknown_004d87cc= 0;
}

/* 0x00081310: something that the network has, at the start of what
network_memory_unknown_000815e0 makes, is made: a byte, a number, a byte and
six numbers more are nothing, and so are its four groups of two numbers and
a real */
void network_memory_unknown_00081310(
	byte *unknown)
{
	long index;

	for (index= 0; index<NETWORK_MEMORY_UNKNOWN_00081310_GROUP_COUNT; index++)
	{
		byte *group= unknown+NETWORK_MEMORY_UNKNOWN_00081310_GROUPS_OFFSET+index*NETWORK_MEMORY_UNKNOWN_00081310_GROUP_SIZE;

		*(long *)group= 0;
		*(long *)(group+4)= 0;
		*(real *)(group+8)= 0.0f;
	}
	unknown[0]= FALSE;
	*(long *)(unknown+4)= 0;
	unknown[8]= FALSE;
	for (index= 0; index<6; index++)
	{
		*(long *)(unknown+0xC+index*4)= 0;
	}
}
