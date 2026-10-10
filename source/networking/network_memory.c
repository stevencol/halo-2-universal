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

#include <string.h>

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

/* 0x00450e10, 0x00450e28, 0x00450df8, 0x00450da0: the functions of four
kinds of thing that the network has */
void *unknown_00450e10[1];
void *unknown_00450e28[1];
void *unknown_00450df8[1];
void *unknown_00450da0[1];

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

/* 0x00081440: something more that the network has is made, and given back:
it gets its functions, and its fifteen things, its group, its two numbers
and its 0x24 numbers are nothing */
byte *network_memory_unknown_00081440(
	byte *unknown)
{
	long index;

	*(void **)unknown= unknown_00450e10;
	for (index= 0; index<NETWORK_MEMORY_UNKNOWN_00081440_THING_COUNT; index++)
	{
		byte *thing= unknown+NETWORK_MEMORY_UNKNOWN_00081440_THINGS_OFFSET+index*NETWORK_MEMORY_UNKNOWN_00081440_THING_SIZE;
		byte *group= thing+NETWORK_MEMORY_UNKNOWN_00081440_THING_GROUP1_OFFSET;

		*(long *)thing= 0;
		*(long *)(thing+4)= 0;
		*(real *)(thing+8)= 0.0f;
		*(long *)group= 0;
		*(long *)(group+4)= 0;
		*(real *)(group+8)= 0.0f;
		*(long *)(thing+NETWORK_MEMORY_UNKNOWN_00081440_THING_UNKNOWN194_OFFSET)= 0;
		*(long *)(thing+NETWORK_MEMORY_UNKNOWN_00081440_THING_UNKNOWN2A4_OFFSET)= 0;
	}
	*(long *)(unknown+NETWORK_MEMORY_UNKNOWN_00081440_GROUP_OFFSET)= 0;
	*(long *)(unknown+NETWORK_MEMORY_UNKNOWN_00081440_GROUP_OFFSET+4)= 0;
	*(real *)(unknown+NETWORK_MEMORY_UNKNOWN_00081440_GROUP_OFFSET+8)= 0.0f;
	*(long *)(unknown+NETWORK_MEMORY_UNKNOWN_00081440_UNKNOWN4_OFFSET)= 0;
	*(long *)(unknown+NETWORK_MEMORY_UNKNOWN_00081440_UNKNOWNC_OFFSET)= 0;
	memset(unknown+NETWORK_MEMORY_UNKNOWN_00081440_NUMBERS_OFFSET, 0,
		NETWORK_MEMORY_UNKNOWN_00081440_NUMBER_COUNT*sizeof(long));

	return unknown;
}

/* 0x000815e0: all that the network has is made, and given back: what
network_memory_unknown_00081310 makes, first; the thing with 0x600 bytes of
its own, with none in use; what network_memory_unknown_00081440 makes; the
three things of 0x78B8 bytes, of nothing; the game in the network
(simulation_world_construct); and the two lists, each of nothing */
byte *network_memory_construct(
	byte *memory)
{
	byte *unknownb28= memory+NETWORK_MEMORY_UNKNOWNB28_OFFSET;
	long index;

	network_memory_unknown_00081310(memory);
	*(void **)unknownb28= unknown_00450e28;
	*(long *)(unknownb28+NETWORK_MEMORY_UNKNOWNB28_UNKNOWN634_OFFSET)= 1;
	*(long *)(unknownb28+NETWORK_MEMORY_UNKNOWNB28_BYTES_SIZE_OFFSET)= NETWORK_MEMORY_UNKNOWNB28_BYTES_SIZE;
	*(long *)(unknownb28+NETWORK_MEMORY_UNKNOWNB28_UNKNOWN638_OFFSET)= 0;
	*(long *)(unknownb28+NETWORK_MEMORY_UNKNOWNB28_UNKNOWN63C_OFFSET)= 0;
	*(long *)(unknownb28+NETWORK_MEMORY_UNKNOWNB28_UNKNOWN644_OFFSET)= 0;
	unknownb28[NETWORK_MEMORY_UNKNOWNB28_UNKNOWN640_OFFSET]= FALSE;
	*(byte **)(unknownb28+NETWORK_MEMORY_UNKNOWNB28_BYTES_POINTER_OFFSET)= unknownb28+NETWORK_MEMORY_UNKNOWNB28_BYTES_OFFSET;
	*(long *)(unknownb28+NETWORK_MEMORY_UNKNOWNB28_UNKNOWN8_OFFSET)= 0;
	*(long *)(unknownb28+NETWORK_MEMORY_UNKNOWNB28_UNKNOWNC_OFFSET)= 0;
	*(long *)(unknownb28+NETWORK_MEMORY_UNKNOWNB28_UNKNOWN10_OFFSET)= 0;
	unknownb28[NETWORK_MEMORY_UNKNOWNB28_UNKNOWN4_OFFSET]= FALSE;
	memory[NETWORK_MEMORY_UNKNOWN1188_OFFSET]= FALSE;
	network_memory_unknown_00081440(memory+NETWORK_MEMORY_UNKNOWN11A0_OFFSET);
	for (index= 0; index<NETWORK_MEMORY_UNKNOWN60F8_COUNT; index++)
	{
		byte *unknown60f8= memory+NETWORK_MEMORY_UNKNOWN60F8_OFFSET+index*NETWORK_MEMORY_UNKNOWN60F8_SIZE;

		*(void **)unknown60f8= unknown_00450df8;
		*(long *)(unknown60f8+NETWORK_MEMORY_UNKNOWN60F8_UNKNOWN14_OFFSET)= 0;
		*(long *)(unknown60f8+NETWORK_MEMORY_UNKNOWN60F8_UNKNOWN4_OFFSET)= 0;
		*(long *)(unknown60f8+NETWORK_MEMORY_UNKNOWN60F8_UNKNOWN18_OFFSET)= NONE;
		*(long *)(unknown60f8+NETWORK_MEMORY_UNKNOWN60F8_UNKNOWN741C_OFFSET)= NONE;
		*(long *)(unknown60f8+NETWORK_MEMORY_UNKNOWN60F8_UNKNOWN1C_OFFSET)= 0;
		*(long *)(unknown60f8+NETWORK_MEMORY_UNKNOWN60F8_UNKNOWN20_OFFSET)= 0;
		unknown60f8[NETWORK_MEMORY_UNKNOWN60F8_UNKNOWN24_OFFSET]= FALSE;
	}
	simulation_world_construct(memory+NETWORK_MEMORY_WORLD_OFFSET);
	*(long *)(memory+NETWORK_MEMORY_UNKNOWN1DD40_OFFSET+4)= 0;
	*(long *)(memory+NETWORK_MEMORY_UNKNOWN1DD40_OFFSET+0xC)= 0;
	*(long *)(memory+NETWORK_MEMORY_UNKNOWN1DD40_OFFSET+8)= 0;
	*(void **)(memory+NETWORK_MEMORY_UNKNOWN1DD40_OFFSET)= unknown_00450da0;
	*(long *)(memory+NETWORK_MEMORY_UNKNOWN1E974_OFFSET)= NONE;
	memset(memory+NETWORK_MEMORY_UNKNOWN1E974_OFFSET+4, 0, NETWORK_MEMORY_LIST_COUNT*sizeof(long));
	*(long *)(memory+NETWORK_MEMORY_UNKNOWN1E9F8_OFFSET)= NONE;
	memset(memory+NETWORK_MEMORY_UNKNOWN1E9F8_OFFSET+4, 0, NETWORK_MEMORY_LIST_COUNT*sizeof(long));

	return memory;
}
