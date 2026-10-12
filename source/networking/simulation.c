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
#include "networking/game_voice.h"
#include "networking/network_life_cycle.h"
#include "networking/simulation_view.h"
#include "networking/simulation_world.h"

#include <limits.h>
#include <string.h>

/* ---------- prototypes */

/* 0x00087d00, 0x00087e90: one of
the things that come and wait is written in a stream, and read from one;
whether what was read is good. 0x00083db0: something of what unknown_004cf780
has */
void __attribute__((stdcall)) unknown_00087d00(struct bitstream *stream, const void *update);
boolean __attribute__((stdcall)) unknown_00087e90(struct bitstream *stream, void *update);
long unknown_00083db0(long unknown);

/* 0x00084270: what unknown_004cf780 has is told that its change was taken.
0x00069a00, 0x00069b50: the thing passes on what the players do, and
something more, of one of them. 0x000835c0: whether what unknown_004cf780 has
lets the game go on a tick. 0x000683a0: one of the things that come is given
to the game */
void __attribute__((stdcall)) unknown_00084270(byte *unknown);
void __attribute__((stdcall)) unknown_00069a00(const void *players, byte *world, long unknown8);
void __attribute__((stdcall)) unknown_00069b50(const void *unknown610, byte *world, long unknown5cc);
boolean simulation_players_changed(long unknown);
void __attribute__((stdcall)) unknown_000683a0(long *update);

void simulation_players_reset(byte *players);
/* 0x00082240: fills the two tables of kinds, and tells how many each has.
0x00068c00: sets up the thing that is going and what unknown_004cf780 has.
0x00068e20: ends the thing that is going. 0x00083000: something more of
what unknown_004cf780 has, for a new map. 0x00195a40, 0x00195e90: what
something else of the game does for a new map and for one that goes */
void __attribute__((stdcall)) unknown_00082240(struct simulation_type_table *tables, long *second_count,
	long *first_count);
void __attribute__((stdcall)) simulation_players_initialize_for_new_map(long unknown);
void unknown_00195a40(void);
/* 0x00162060, 0x00162420, 0x000bb7f0, 0x00183f10: what other parts of the
game do when a game of kind 4 or 5 is no more. 0x00083370: what
unknown_004cf780 has after a state was loaded, as the bits say.
0x00083220: whether what unknown_004cf780 has is well. 0x00068f30: the
thing that is going does its work. 0x00068dc0: the thing that is going,
for a game that begins again */
void unknown_00162060(void);
void game_voice_unknown_00162420(void);
void unknown_000bb7f0(void);
void unknown_00183f10(void);
void __attribute__((stdcall)) simulation_players_after_load(long unknown, unsigned long flags);
boolean simulation_players_update_views(long unknown);
void unknown_00195e90(void);

long GetTickCount(void);

/* ---------- globals */

/* 0x00510548, 0x0051054c, of main.c: whether the time is the one that was
kept, and that time. 0x004cf780, of game.c */
extern boolean unknown_00510548;
extern long unknown_0051054c;
extern long unknown_004cf780;

/* 0x004ed39c, of game.c: with it set, the simulation does nothing for a
map. 0x004d87e8, of network_memory.c */
extern boolean unknown_004ed39c;
extern byte *unknown_004d87e8;

/* 0x004cf784: the two tables of kinds. 0x00544b20, 0x00545d40,
0x00546974: the memory of the thing that is going, of what
unknown_004cf780 has and of the two tables */
struct simulation_type_table *unknown_004cf784;
byte unknown_00544b20[SIMULATION_WORLD_STORAGE_SIZE];
byte unknown_00545d40[SIMULATION_UNKNOWN_004CF780_STORAGE_SIZE];
struct simulation_type_table unknown_00546974[2];

/* ---------- code */

/* 0x00068550: the channel of a view is gone: a view that is well goes to
its first state and is as 1 says (simulation_view_fail, written here), and
it lets go of the channel (simulation_view_release_channel) */
void __attribute__((stdcall)) simulation_view_disconnected(
	byte *view)
{
	if (*(const long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)==0)
	{
		simulation_view_set_state(view, _simulation_view_state0, NONE);
		*(long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)= 1;
	}
	simulation_view_release_channel(view);
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
doing now (simulation_world_build_update), all zeros first. And a thing in its third state
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
	simulation_world_build_update(unknown_004cf770.unknownC, update);
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
unknown_004cf780 has letting it (simulation_players_changed), one of the things that
come is made (simulation_build_update), given to the game (unknown_000683a0)
and then to the thing (simulation_apply_update) */
void simulation_update(
	void)
{
	if (unknown_004cf770.unknown0 && main_game_globals && main_game_globals->active && !unknown_004cf770.unknown1[1] &&
		simulation_players_changed(unknown_004cf780))
	{
		long update[SIMULATION_UPDATE_DECODED_SIZE/4];

		simulation_build_update(update);
		unknown_000683a0(update);
		simulation_apply_update(update);
	}
}

/* 0x00068350: a change of what unknown_004cf780 has, if there is one that
was not taken yet: a number and 0x60 bytes of it are given, it is told
(unknown_00084270), and it has none; whether there was one */
void __attribute__((stdcall)) simulation_take_unknown_004cf780_change(
	long *change,
	boolean *changed)
{
	byte *unknown= (byte *)unknown_004cf780;

	*changed= FALSE;
	if (unknown[SIMULATION_UNKNOWN_004CF780_CHANGED_OFFSET])
	{
		change[0]= *(const long *)(unknown+SIMULATION_UNKNOWN_004CF780_UNKNOWN1C_OFFSET);
		memcpy(&change[1], unknown+SIMULATION_UNKNOWN_004CF780_UNKNOWN24_OFFSET, SIMULATION_UNKNOWN_004CF780_UNKNOWN24_SIZE);
		unknown_00084270(unknown);
		*changed= TRUE;
		unknown[SIMULATION_UNKNOWN_004CF780_CHANGED_OFFSET]= FALSE;
	}
}

/* 0x00068580: a view of a kind is made, with the six bytes that it is known
by and a number, and given to the thing that unknown_004cf770 has going; the
view, or NONE if there is no place for it in the tables (simulation_view_allocate).
It is new (simulation_view_new), and takes the first place among the views
of the thing that is free, which then has one more; with none free, it is
of the thing all the same, at a place after the last, and the thing has it
not. The fourth thing that it is given is not looked at */
byte *__attribute__((stdcall)) simulation_view_create(
	long kind,
	const word *unknown14,
	long unknown1c,
	long unused)
{
	byte *view;
	byte *data;
	byte *world;
	long datum_index;
	long index;

	datum_index= simulation_view_allocate(&view, &data);
	if (datum_index==NONE)
	{
		return (byte *)NONE;
	}
	view= simulation_view_new(view, unknown14, datum_index, kind, data, unknown1c);
	world= unknown_004cf770.unknownC;
	for (index= 0; index<NUMBER_OF_SIMULATION_WORLD_VIEWS; index++)
	{
		if (!((byte *const *)(world+SIMULATION_WORLD_VIEWS_OFFSET))[index])
		{
			((byte **)(world+SIMULATION_WORLD_VIEWS_OFFSET))[index]= view;
			(*(long *)(world+SIMULATION_WORLD_VIEW_COUNT_OFFSET))++;
			break;
		}
	}
	*(byte **)(view+SIMULATION_VIEW_WORLD_OFFSET)= world;
	*(long *)(view+SIMULATION_VIEW_INDEX_OFFSET)= index;

	return view;
}

/* 0x00067e30: the simulation is set up, when the game starts: the thing that
is going, what unknown_004cf780 has and the two tables of kinds get their
memory, and the tables are emptied and then filled (unknown_00082240) */
void simulation_initialize(
	void)
{
	long first_count, second_count;
	short index;

	unknown_004cf784= unknown_00546974;
	unknown_004cf770.unknownC= unknown_00544b20;
	unknown_004cf780= (long)unknown_00545d40;
	for (index= 0; index<2; index++)
	{
		unknown_00546974[index].count= NONE;
		memset(unknown_00546974[index].types, 0, sizeof(unknown_00546974[index].types));
	}
	unknown_00082240(unknown_004cf784, &second_count, &first_count);
	unknown_004cf784[0].count= first_count;
	unknown_004cf784[1].count= second_count;
	unknown_004cf770.unknown0= TRUE;
}

/* 0x00067eb0: the simulation that was set up is no more: it has no memory */
void simulation_dispose(
	void)
{
	if (unknown_004cf770.unknown0)
	{
		unknown_004cf770.unknownC= NULL;
		unknown_004cf780= 0;
		unknown_004cf784= NULL;
		unknown_004cf770.unknown0= FALSE;
	}
}

/* 0x00067ee0: the simulation for a new map, unless unknown_004ed39c is set:
it has not failed, the thing that is going and what unknown_004cf780 has are
set up (simulation_world_initialize; with what unknown_004d87e8 is, in a game of kind 4
or 5), and what unknown_004cf780 has gets the thing and is as when it begins
(simulation_players_reset, simulation_players_initialize_for_new_map) */
void simulation_initialize_for_new_map(
	void)
{
	if (!unknown_004ed39c)
	{
		long kind;
		byte *unknown;

		unknown_004cf770.unknown1[0]= FALSE;
		unknown_004cf770.unknown1[1]= FALSE;
		kind= main_game_globals->options.simulation;
		simulation_world_initialize(unknown_004cf780, kind>=4 && kind<=5 ? unknown_004d87e8 : NULL, unknown_004cf784,
			unknown_004cf770.unknownC);
		unknown= (byte *)unknown_004cf780;
		*(byte **)(unknown+SIMULATION_UNKNOWN_004CF780_WORLD_OFFSET)= unknown_004cf770.unknownC;
		*(long *)(unknown+SIMULATION_UNKNOWN_004CF780_UNKNOWNC_OFFSET)= 0;
		*(long *)(unknown+SIMULATION_UNKNOWN_004CF780_UNKNOWN8_OFFSET)= 0;
		simulation_players_reset(unknown);
		simulation_players_initialize_for_new_map(unknown_004cf780);
		unknown_00195a40();
		unknown_004cf770.unknown1[7]= TRUE;
	}
}

/* 0x00067f60: the simulation when a map goes, unless unknown_004ed39c is
set: what unknown_004cf780 has gives up what it had, and has no thing; and
the thing that is going ends (simulation_world_dispose) */
void simulation_dispose_from_old_map(
	void)
{
	if (!unknown_004ed39c)
	{
		byte *unknown= (byte *)unknown_004cf780;
		byte **unknown8= (byte **)(unknown+SIMULATION_UNKNOWN_004CF780_UNKNOWN8_OFFSET);

		if (*unknown8)
		{
			*(long *)(*unknown8+SIMULATION_UNKNOWN_004CF780_UNKNOWN8_UNKNOWN80_OFFSET)= 0;
			*unknown8= NULL;
		}
		*(long *)(unknown+SIMULATION_UNKNOWN_004CF780_UNKNOWNC_OFFSET)= 0;
		*(byte **)(unknown+SIMULATION_UNKNOWN_004CF780_WORLD_OFFSET)= NULL;
		simulation_world_dispose(unknown_004cf770.unknownC);
		unknown_00195e90();
	}
}

/* 0x00067fb0: a state of the game is about to be loaded: it is noted. It is
the last of the things that are done before a state is loaded */
void GAME_STATE_PROC simulation_before_load(
	unsigned long flags)
{
	(void)flags;
	unknown_004cf770.unknownB= TRUE;
}

/* 0x00067fc0: a state of the game was loaded, as bits say. With the third
bit, the game in the network is over (network_life_cycle_end), and the
simulation begins again for a game of kind 1: if it was one already, the
thing that is going gives up (simulation_world_unknown_0006b040) and nothing
has failed; if not, the simulation ends as when a map goes and begins as
with a new one, and a game of kind 4 or 5 is ended first in other parts of
the game. Then the thing that is going has no players
(simulation_world_players_clear), and what unknown_004cf780 has and the
thing are told (simulation_players_after_load, simulation_world_unknown_0006a770) */
void GAME_STATE_PROC simulation_after_load(
	unsigned long flags)
{
	if (flags&4)
	{
		network_life_cycle_end();
		if (main_game_globals->options.simulation==1 &&
			*(const long *)(unknown_004cf770.unknownC+SIMULATION_WORLD_STATE_OFFSET)==1)
		{
			simulation_world_unknown_0006b040(unknown_004cf770.unknownC);
			unknown_004cf770.unknown1[1]= FALSE;
			unknown_004cf770.unknown1[0]= FALSE;
		}
		else
		{
			long kind;

			simulation_dispose_from_old_map();
			if (main_game_globals->options.simulation==4)
			{
				main_game_globals->options.simulation= 5;
				if (unknown_0055e4d0[*(const long *)(unknown_004e9ae8+UNKNOWN_004E9AE8_UNKNOWNC14_OFFSET)])
				{
					unknown_00162060();
				}
			}
			kind= main_game_globals->options.simulation;
			if (kind>=4 && kind<=5)
			{
				game_voice_unknown_00162420();
				unknown_000bb7f0();
				unknown_00183f10();
			}
			main_game_globals->options.simulation= 1;
			simulation_initialize_for_new_map();
		}
	}
	simulation_world_players_clear(unknown_004cf770.unknownC);
	simulation_players_after_load(unknown_004cf780, flags);
	simulation_world_unknown_0006a770(unknown_004cf770.unknownC);
	unknown_004cf770.unknownB= FALSE;
}

/* 0x000680f0: what the simulation looks at outside the ticks, with a game in
the network that has not failed and a game that goes on. A failure that was
asked for is done (simulation_fail), and a beginning again that was asked
for (simulation_unknown_000687b0); what unknown_004cf780 has must be well
(simulation_players_update_views), or the simulation fails; the thing that is going does
its work (simulation_world_update), and fails if it is in its third state or its
fifth with the byte at 0x2C set. After any of these the rest is left if the
simulation has failed. At the end, a game that this console rules, that is
over and that is not one of several consoles goes to the main menu. The
name is a guess */
void simulation_idle(
	void)
{
	const byte *world;
	long state;

	if (!unknown_004cf770.unknown0 || unknown_004cf770.unknown1[1] || !main_game_globals || !main_game_globals->active)
	{
		return;
	}
	if (unknown_004cf770.unknown1[0])
	{
		simulation_fail();
		if (unknown_004cf770.unknown1[1])
		{
			return;
		}
	}
	if (unknown_004cf770.unknown1[8])
	{
		simulation_unknown_000687b0();
		if (unknown_004cf770.unknown1[1])
		{
			return;
		}
	}
	if (!simulation_players_update_views(unknown_004cf780))
	{
		simulation_fail();
	}
	if (unknown_004cf770.unknown1[1])
	{
		return;
	}
	simulation_world_update(unknown_004cf770.unknownC);
	if (unknown_004cf770.unknown1[1])
	{
		return;
	}
	world= unknown_004cf770.unknownC;
	state= *(const long *)(world+SIMULATION_WORLD_STATE_OFFSET);
	if ((state==3 || state==5) && world[SIMULATION_WORLD_UNKNOWN2C_OFFSET])
	{
		simulation_fail();
		if (unknown_004cf770.unknown1[1])
		{
			return;
		}
	}
	if (game_is_authoritative() && (boolean)game_finished_wait_over() && !game_is_distributed())
	{
		main_game_goto_main_menu(6);
	}
}

/* 0x000687b0: the game begins again, as was asked: the thing that is going
is made ready (simulation_world_restart) and the game restarts (main_game_restart),
with a byte set meanwhile; and it is asked for no more */
void simulation_unknown_000687b0(
	void)
{
	unknown_004cf770.unknownA= TRUE;
	simulation_world_restart(unknown_004cf770.unknownC);
	main_game_restart();
	unknown_004cf770.unknown1[8]= FALSE;
	unknown_004cf770.unknownA= FALSE;
}

/* 0x00067e10: whether the thing that is going is not in its third state nor
in its fifth: the two that simulation_world_update has another way for. It
is the first function of this file */
boolean simulation_unknown_00067e10(
	const byte *world)
{
	long state= *(const long *)(world+SIMULATION_WORLD_STATE_OFFSET);

	return state!=3 && state!=5;
}
