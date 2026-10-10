/*
SIMULATION.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The game in the network, as the rest of the game sees it: what
unknown_004cf770 has going and its views. The name of the file is a guess,
and so are the names of its functions.
*/

#include "simulation.h"

#include "game/game.h"
#include "main/main_game.h"
#include "saved games/game_state.h"
#include "networking/bitstream.h"
#include "networking/simulation_view.h"
#include "networking/simulation_world.h"

#include <limits.h>
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

/* 0x00069260: one of the things that come is made from what the game is
doing. 0x00069a00, 0x00069b50: the thing passes on what the players do, and
something more, of one of them. 0x000835c0: whether what unknown_004cf780 has
lets the game go on a tick. 0x000683a0: one of the things that come is given
to the game */
void unknown_00069260(byte *world, long *update);
void __attribute__((stdcall)) unknown_00069a00(const void *players, byte *world, long unknown8);
void __attribute__((stdcall)) unknown_00069b50(const void *unknown610, byte *world, long unknown5cc);
boolean unknown_000835c0(long unknown);
void __attribute__((stdcall)) unknown_000683a0(long *update);

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

/* 0x00068090: the state of the game is about to be saved, as bits say: with
a game in the network, and without the first bit, a thing that is not in its
third state or its fifth notes it. It is the second of the things that are
done before a state is saved (game_state_before_save_procs) */
void GAME_STATE_PROC simulation_before_save(
	unsigned long flags)
{
	if (unknown_004cf770.unknown0 && !(flags&1))
	{
		byte *world= unknown_004cf770.unknownC;
		long state= *(const long *)(world+SIMULATION_WORLD_STATE_OFFSET);

		if (state!=3 && state!=5)
		{
			world[SIMULATION_WORLD_SAVED_OFFSET]= TRUE;
		}
	}
}

/* 0x000680c0: how many of the things that come wait for the game to take,
and whether they were counted (simulation_world_unknown_00069300); the most
that there can be, with no game in the network or a thing that is not in
use */
long simulation_updates_waiting(
	boolean *counted)
{
	*counted= FALSE;
	if (!unknown_004cf770.unknown0 || !*(const long *)(unknown_004cf770.unknownC+SIMULATION_WORLD_STATE_OFFSET))
	{
		return LONG_MAX;
	}

	return simulation_world_unknown_00069300(unknown_004cf770.unknownC, counted);
}

/* 0x00068250: whether a game in the network is being played: there is one,
there is a game and it is on, with something at 0x2 of it that is not NONE,
it has not been said that it cannot go on, and the number that the thing has
at 0x18 is 4 */
boolean simulation_in_progress(
	void)
{
	return unknown_004cf770.unknown0 && main_game_globals && main_game_globals->active &&
		*(const short *)((const byte *)main_game_globals+2)!=NONE && !unknown_004cf770.unknown1[1] &&
		*(const long *)(unknown_004cf770.unknownC+SIMULATION_WORLD_UNKNOWN18_OFFSET)==_simulation_world_unknown18_4;
}

/* 0x00068290: whether a game in the network is on its way to being played:
there is one, it has not been said that it cannot go on, the thing is in
use, and the number that it has at 0x18 is not 4 */
boolean simulation_starting(
	void)
{
	if (unknown_004cf770.unknown0 && !unknown_004cf770.unknown1[1])
	{
		const byte *world= unknown_004cf770.unknownC;

		if (*(const long *)(world+SIMULATION_WORLD_STATE_OFFSET))
		{
			return *(const long *)(world+SIMULATION_WORLD_UNKNOWN18_OFFSET)!=_simulation_world_unknown18_4;
		}
	}

	return FALSE;
}

/* 0x000682c0: 1 if there is a game in the network and it has been said
that it cannot go on (simulation_fail); 0 if not */
long simulation_failed(
	void)
{
	return unknown_004cf770.unknown0 && unknown_004cf770.unknown1[1] ? 1 : 0;
}

/* 0x000682e0: one of the things that come is made from what the game is
doing now (unknown_00069260), all zeros first. And a thing in its third state
or its fifth, which takes them from another console, sees that the game is
the same here: if the number of the one that was made is not the one of the
thing, or the time of the game or its seed are others, a byte of the thing
says that something is wrong; once it is set it is not looked at again */
void simulation_build_update(
	long *update)
{
	byte *world;
	long state;

	memset(update, 0, SIMULATION_UPDATE_DECODED_SIZE);
	unknown_00069260(unknown_004cf770.unknownC, update);
	world= unknown_004cf770.unknownC;
	state= *(const long *)(world+SIMULATION_WORLD_STATE_OFFSET);
	if ((state==3 || state==5) && !world[SIMULATION_WORLD_UNKNOWN2C_OFFSET])
	{
		if (update[0]!=*(const long *)(world+SIMULATION_WORLD_UNKNOWN28_OFFSET) ||
			*(const unsigned long *)((const byte *)update+SIMULATION_UPDATE_GAME_TIME_OFFSET)!=game_time_globals->time ||
			*(const unsigned long *)((const byte *)update+SIMULATION_UPDATE_RANDOM_SEED_OFFSET)!=*global_random_seed_reference)
		{
			world[SIMULATION_WORLD_UNKNOWN2C_OFFSET]= TRUE;
		}
	}
}

/* 0x000684e0: the thing is done with one of the things that come: in its
second state, each view of the second kind is given it
(simulation_world_unknown_00069880); in its fourth, what the players do and
something more of it are passed on (unknown_00069a00, unknown_00069b50); in
its third and its fifth, nothing. And the number of the thing is the one
after it */
void simulation_apply_update(
	const long *update)
{
	byte *world= unknown_004cf770.unknownC;
	long state= *(const long *)(world+SIMULATION_WORLD_STATE_OFFSET);

	if (state!=3 && state!=5)
	{
		if (state==2)
		{
			simulation_world_unknown_00069880(world, update);
			world= unknown_004cf770.unknownC;
		}
		else if (state==4)
		{
			unknown_00069a00((const byte *)update+SIMULATION_UPDATE_PLAYERS_OFFSET, world,
				*(const long *)((const byte *)update+SIMULATION_UPDATE_UNKNOWN8_OFFSET));
			unknown_00069b50((const byte *)update+SIMULATION_UPDATE_UNKNOWN610_OFFSET, world,
				*(const long *)((const byte *)update+SIMULATION_UPDATE_UNKNOWN5CC_OFFSET));
			world= unknown_004cf770.unknownC;
		}
	}
	*(long *)(world+SIMULATION_WORLD_UNKNOWN28_OFFSET)= (long)((unsigned long)update[0]+1);
}

/* 0x000681e0: what the game in the network does each tick of the game: with
one, a game that is on, nothing said of it not going on, and what
unknown_004cf780 has letting it (unknown_000835c0), one of the things that
come is made (simulation_build_update), given to the game (unknown_000683a0)
and then to the thing (simulation_apply_update) */
void simulation_update(
	void)
{
	if (unknown_004cf770.unknown0 && main_game_globals && main_game_globals->active && !unknown_004cf770.unknown1[1] &&
		unknown_000835c0(unknown_004cf780))
	{
		long update[SIMULATION_UPDATE_DECODED_SIZE/4];

		simulation_build_update(update);
		unknown_000683a0(update);
		simulation_apply_update(update);
	}
}
