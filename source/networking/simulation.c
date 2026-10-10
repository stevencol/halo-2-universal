/*
SIMULATION.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The game in the network, as the rest of the game sees it: what
unknown_004cf770 has going and its views. The name of the file is a guess,
and so are the names of its functions.
*/

#include "simulation.h"

#include "main/main_game.h"
#include "networking/bitstream.h"
#include "networking/simulation_view.h"
#include "networking/simulation_world.h"

#include <string.h>

/* ---------- prototypes */

/* 0x00085880: a view lets go of its channel. 0x00087d00, 0x00087e90: one of
the things that come and wait is written in a stream, and read from one;
whether what was read is good. 0x00083db0: something of what unknown_004cf780
has */
void unknown_00085880(byte *view);
void __attribute__((stdcall)) unknown_00087d00(struct bitstream *stream, const void *update);
boolean __attribute__((stdcall)) unknown_00087e90(struct bitstream *stream, void *update);
long unknown_00083db0(long unknown);

long GetTickCount(void);

/* ---------- globals */

/* 0x00510548, 0x0051054c, of main.c: whether the time is the one that was
kept, and that time. 0x004cf780, of game.c */
extern boolean unknown_00510548;
extern long unknown_0051054c;
extern long unknown_004cf780;

/* ---------- code */

/* 0x00068550: the channel of a view is gone: a view that is well goes to
its first state and is as 1 says (simulation_view_fail, written here), and
it lets go of the channel (unknown_00085880) */
void __attribute__((stdcall)) simulation_view_disconnected(
	byte *view)
{
	if (*(const long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)==0)
	{
		simulation_view_set_state(view, _simulation_view_state0, NONE);
		*(long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)= 1;
	}
	unknown_00085880(view);
}

/* 0x000685f0: one of the things that come and wait is put in bytes, in
memory of a size, which is all zeros first; how many bytes it is. It is
written in a stream (unknown_00087d00). It says TRUE always. The game has a
stream started for writing written here */
boolean __attribute__((stdcall)) simulation_update_encode(
	void *buffer,
	long capacity,
	const void *update,
	long *size_out)
{
	struct bitstream stream;

	memset(buffer, 0, capacity);
	stream.data= buffer;
	stream.size= capacity;
	stream.position= 0;
	stream.position_count= 0;
	stream.error= FALSE;
	stream.unknown2C= 0;
	stream.unknown30= 0;
	stream.unknown8= 1;
	stream.unknownC= _bitstream_writing;
	unknown_00087d00(&stream, update);
	*size_out= (stream.position+7)/8;

	return TRUE;
}

/* 0x00068670: one of the things that come and wait is made from the bytes
that were kept of it; whether it is good. It is all zeros first, and is read
from a stream (unknown_00087e90). The game has bitstream_begin_reading
written here */
boolean simulation_update_decode(
	const void *data,
	long size,
	void *update)
{
	struct bitstream stream;

	memset(update, 0, SIMULATION_UPDATE_DECODED_SIZE);
	stream.data= (unsigned long *)data;
	stream.size= size;
	stream.unknown8= 1;
	bitstream_begin_reading(&stream);

	return unknown_00087e90(&stream, update)!=FALSE;
}

/* 0x000686e0: whether the console that six bytes say is in the game for
good: with no game in the network, it is; with one that is not in use, it is
not; this console is; and any other, if the thing has a view of it
(simulation_world_get_view_by_unknown14) whose byte at 0x78 is set */
boolean simulation_machine_established(
	const word *unknown14)
{
	byte *world;
	const byte *view;

	if (!unknown_004cf770.unknown0)
	{
		return TRUE;
	}
	world= unknown_004cf770.unknownC;
	if (!*(const long *)(world+SIMULATION_WORLD_STATE_OFFSET))
	{
		return FALSE;
	}
	if (memcmp(unknown14, world+SIMULATION_WORLD_OWN_UNKNOWN14_OFFSET, SIMULATION_VIEW_UNKNOWN14_SIZE)==0)
	{
		return TRUE;
	}
	view= simulation_world_get_view_by_unknown14(world, unknown14);
	if (!view)
	{
		return FALSE;
	}

	return view[SIMULATION_VIEW_UNKNOWN78_OFFSET];
}

/* 0x00068750: the game in the network cannot go on. If the thing is in use
and that was not said yet: it is said, with the time; and unless the number
that the thing has at 0x18 is 1 already, the thing gives up
(simulation_world_unknown_0006b040) and that number is 1. The game has the
time of the network written here */
void simulation_fail(
	void)
{
	byte *world= unknown_004cf770.unknownC;

	if (!*(const long *)(world+SIMULATION_WORLD_STATE_OFFSET) || unknown_004cf770.unknown1[1])
	{
		return;
	}
	unknown_004cf770.unknown1[1]= TRUE;
	*(long *)&unknown_004cf770.unknown1[3]= unknown_00510548 ? unknown_0051054c : GetTickCount();
	world= unknown_004cf770.unknownC;
	if (*(const long *)(world+SIMULATION_WORLD_UNKNOWN18_OFFSET)!=_simulation_world_unknown18_1)
	{
		simulation_world_unknown_0006b040(world);
		simulation_world_set_unknown18(world, _simulation_world_unknown18_1);
	}
}

/* 0x000687e0: something of what unknown_004cf780 has (unknown_00083db0), or
nothing with no game in the network */
long simulation_unknown_000687e0(
	void)
{
	if (unknown_004cf770.unknown0)
	{
		return unknown_00083db0(unknown_004cf780);
	}

	return 0;
}
