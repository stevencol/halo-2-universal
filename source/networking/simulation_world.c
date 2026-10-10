/*
SIMULATION_WORLD.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

What unknown_004cf770 has going of a game in the network, and the views
that it has of it: one for each channel of the game that it talks by. Each
view has the thing that it is of at 0xC. The name of the file is a guess,
and so are the names of its functions.
*/

#include "simulation_world.h"

#include "cache/texture_cache.h"
#include "game/game.h"
#include "main/main.h"
#include "main/main_game.h"
#include "memory/data.h"
#include "saved games/game_state.h"
#include "networking/network_heap.h"
#include "networking/network_observer.h"
#include "networking/network_session.h"
#include "networking/simulation.h"
#include "networking/simulation_view.h"

#include <string.h>

#include <limits.h>

/* ---------- prototypes */

/* 0x00199740: the state of the game that came, of a size, is put where the state of the
game is, and its size is given; whether it was good */
boolean __attribute__((stdcall)) unknown_00199740(const void *data, long size, void *base, long *size_out);

/* 0x00083610: the things that happened to what unknown_004cf780 has are
given, and how many. 0x00069110: what the players do goes into one of the
things that come. 0x000c6ef0: something of one of the other things of the
thing is made ready. 0x000694c0: the thing gets a player of the game */
void __attribute__((stdcall)) unknown_00083610(long unknown, long *count, void *events);
void __attribute__((stdcall)) unknown_00069110(byte *world, long *update);
void unknown_000c6ef0(byte *unknown14);

/* 0x000847d0: what the thing has of a player takes what the player does */
void __attribute__((stdcall)) unknown_000847d0(byte *player, const void *actions);

/* ---------- more globals */

/* 0x004cf780, of game.c */
extern long unknown_004cf780;


/* 0x00083bd0: whether what the thing has first has a player, by its place,
with the three numbers that it is known by */
boolean __attribute__((stdcall)) unknown_00083bd0(long player, const void *unknown, const long *identifier);
void __attribute__((stdcall)) unknown_000694c0(byte *world, long player);


/* 0x000693a0: the views of the thing are done with: what main_game_unload
ends the game in the network with. 0x0006a560: the thing is given a byte */
void __attribute__((stdcall)) unknown_000693a0(void *world);
void __attribute__((stdcall)) unknown_0006a560(byte *world, boolean unknown);

long GetTickCount(void);

/* ---------- globals */

/* 0x00510548, 0x0051054c, of main.c: whether the time is the one that was
kept, and that time */
extern boolean unknown_00510548;
extern long unknown_0051054c;

/* 0x004ced58, 0x004ced5c: how many times the thing can be on its way to
going before it gives up, and for how long each time, in milliseconds */
long unknown_004ced58;

/* 0x004ced4c: how long the thing waits for its first view of the first kind
or of the third, in milliseconds */
long unknown_004ced4c;

/* 0x004ced48: how many views can be sent the state of the game at a time.
0x004ced50: how many times one can be sent it. 0x004ced54: how long the
thing waits between starting to send it to one view and to the next, in
milliseconds */
long unknown_004ced48;
long unknown_004ced50;
long unknown_004ced54;
long unknown_004ced5c;

/* ---------- code */

/* 0x00069470: the next view of the thing that is of one of the kinds that
are wanted; whether there is one. The views are looked at from the place
that the iterator has, which goes on past each one that is looked at; there
is none from a place before the first */
boolean __attribute__((stdcall)) simulation_world_next_view(
	struct simulation_view_iterator *iterator,
	const byte *world,
	byte **view_out)
{
	if (iterator->index<0)
	{
		return FALSE;
	}
	do
	{
		byte *view;

		if (iterator->index>=NUMBER_OF_SIMULATION_WORLD_VIEWS)
		{
			return FALSE;
		}
		view= ((byte *const *)(world+SIMULATION_WORLD_VIEWS_OFFSET))[iterator->index];
		iterator->index++;
		if (view && (iterator->kind_mask&(1u<<(view[SIMULATION_VIEW_KIND_OFFSET]&31))))
		{
			*view_out= view;

			return TRUE;
		}
	} while (iterator->index>=0);

	return FALSE;
}

/* 0x0006adc0: the view that the thing has of a channel of the game, by the
number of the channel in the list; NULL if it has none. Views of every kind
are looked at */
byte *simulation_world_get_channel_view(
	const byte *world,
	long channel_index)
{
	struct simulation_view_iterator iterator= {0xFFFFFFFF, 0};
	byte *view;

	while (simulation_world_next_view(&iterator, world, &view))
	{
		if (*(const long *)(view+SIMULATION_VIEW_CHANNEL_OFFSET)==channel_index)
		{
			return view;
		}
	}

	return NULL;
}

/* 0x000696d0: the byte of what the thing has of a player of the game, by
the low word of its number; FALSE with no such player */
boolean simulation_world_player_unknown25(
	long player,
	const byte *world)
{
	const byte *player_data= world+((unsigned long)player&0xFFFF)*SIMULATION_WORLD_PLAYER_SIZE+SIMULATION_WORLD_PLAYERS_OFFSET;

	if (*(const long *)player_data==NONE)
	{
		return FALSE;
	}

	return player_data[SIMULATION_WORLD_PLAYER_UNKNOWN25_OFFSET];
}

/* 0x00069610: the byte of what the thing has of each player of the game is
clear */
void simulation_world_players_clear_unknown25(
	byte *world)
{
	long player;

	for (player= 0; player<NUMBER_OF_SIMULATION_WORLD_PLAYERS; player++)
	{
		byte *player_data= world+player*SIMULATION_WORLD_PLAYER_SIZE+SIMULATION_WORLD_PLAYERS_OFFSET;

		if (*(const long *)player_data!=NONE && player_data[SIMULATION_WORLD_PLAYER_UNKNOWN25_OFFSET])
		{
			player_data[SIMULATION_WORLD_PLAYER_UNKNOWN25_OFFSET]= FALSE;
		}
	}
}

/* 0x0006acb0: the first view that the thing has of the first kind or of the
third; NULL if it has none */
byte *simulation_world_unknown_0006acb0(
	const byte *world)
{
	struct simulation_view_iterator iterator= {SIMULATION_WORLD_UNKNOWN_0006ACB0_KINDS, 0};
	byte *view= NULL;

	simulation_world_next_view(&iterator, world, &view);

	return view;
}

/* 0x0006aae0: the thing gives back the memory that the texture cache lent
it, and has none */
void simulation_world_unknown_0006aae0(
	byte *world)
{
	texture_cache_return_memory(*(void **)(world+SIMULATION_WORLD_MEMORY_OFFSET));
	*(void **)(world+SIMULATION_WORLD_MEMORY_OFFSET)= NULL;
	*(long *)(world+SIMULATION_WORLD_UNKNOWN1200_OFFSET)= NONE;
}

/* 0x00069300: how many things wait for the thing to take, and whether they
were counted here. None if it is not going. In its second state it is what
simulation_world_unknown_00069790 says; in its third, from the first of the ones
that have come to the last, and those were counted; in the others, the most
that there can be. The game has no more states than five in its table */
long simulation_world_unknown_00069300(
	byte *world,
	boolean *counted)
{
	*counted= FALSE;
	if (!world[SIMULATION_WORLD_GOING_OFFSET])
	{
		return 0;
	}
	switch (*(const long *)(world+SIMULATION_WORLD_STATE_OFFSET))
	{
	case _simulation_world_state2:
		return simulation_world_unknown_00069790(world);
	case _simulation_world_state3:
		*counted= TRUE;

		return (long)(*(const unsigned long *)(world+SIMULATION_WORLD_LAST_UPDATE_OFFSET)-
			*(const unsigned long *)(world+SIMULATION_WORLD_FIRST_UPDATE_OFFSET)+1);
	}

	return LONG_MAX;
}

/* 0x00069350: the thing is told whether the game is to be
brought up to what has come; and if so, and while it is going, the game goes
on a tick for as long as things wait: the game has
simulation_world_unknown_00069300 written here, so in the states that are
not the second or the third it goes on until the thing is not going */
void simulation_world_unknown_00069350(
	boolean catch_up,
	byte *world)
{
	world[SIMULATION_WORLD_CATCH_UP_OFFSET]= catch_up;
	if (!catch_up)
	{
		return;
	}
	while (world[SIMULATION_WORLD_GOING_OFFSET])
	{
		long count;

		switch (*(const long *)(world+SIMULATION_WORLD_STATE_OFFSET))
		{
		case _simulation_world_state2:
			count= simulation_world_unknown_00069790(world);
			break;
		case _simulation_world_state3:
			count= (long)(*(const unsigned long *)(world+SIMULATION_WORLD_LAST_UPDATE_OFFSET)-
				*(const unsigned long *)(world+SIMULATION_WORLD_FIRST_UPDATE_OFFSET)+1);
			break;
		default:
			count= LONG_MAX;
			break;
		}
		if (count<=0)
		{
			return;
		}
		game_tick();
	}
}

/* 0x000698e0: something came for the thing, which starts with
its number; whether it was taken. One from before the one after the last
that came is not, and nothing is done. The one after the last is taken
(simulation_world_add_update) if a number of the thing is 4 or the game is to be brought
up to what has come; and if it could not be, a byte of unknown_004cf770 says
so. Any other, or that one if not, or whatever comes in the third state with
something at 0x1200, is not taken, and a byte of the thing says so.

After one that is taken, if the game is to be brought up to what has come,
it goes on a tick for as long as things wait
(simulation_world_unknown_00069300): while there is a game and it is on,
unknown_004cf770 does not say otherwise with its third byte, and the byte of
the thing is clear */
boolean simulation_world_unknown_000698e0(
	byte *world,
	const long *update)
{
	long next= (long)(*(const unsigned long *)(world+SIMULATION_WORLD_LAST_UPDATE_OFFSET)+1);

	if (!simulation_world_receiving_game_state(world))
	{
		if (update[0]<next)
		{
			return FALSE;
		}
		if (update[0]==next &&
			(*(const long *)(world+SIMULATION_WORLD_UNKNOWN18_OFFSET)==_simulation_world_unknown18_4 ||
			world[SIMULATION_WORLD_CATCH_UP_OFFSET]))
		{
			if (!simulation_world_add_update(world, update))
			{
				unknown_004cf770.unknown1[0]= TRUE;

				return FALSE;
			}
			if (world[SIMULATION_WORLD_CATCH_UP_OFFSET])
			{
				while (main_game_globals && main_game_globals->active &&
					!(unknown_004cf770.unknown0 && unknown_004cf770.unknown1[1]) &&
					!world[SIMULATION_WORLD_UNKNOWN2C_OFFSET])
				{
					boolean counted;

					if (simulation_world_unknown_00069300(world, &counted)<=0)
					{
						break;
					}
					game_tick();
				}
			}

			return TRUE;
		}
	}
	world[SIMULATION_WORLD_UNKNOWN2C_OFFSET]= TRUE;

	return FALSE;
}

/* 0x0006a2a0: a view goes on to its next state (simulation_view_set_state),
as the one that it is in says: in the third, nothing; in the second, and
with another number of it 1, to the third, with the count of something else,
which goes on by one; in the second and not that, nothing; and in any other,
to the second, with NONE */
void simulation_world_unknown_0006a2a0(
	byte *view,
	byte *other)
{
	long state= *(const long *)(view+SIMULATION_VIEW_STATE_OFFSET);

	if (state==2)
	{
		return;
	}
	if (state==1)
	{
		if (*(const long *)(view+SIMULATION_VIEW_UNKNOWN34_OFFSET)==1)
		{
			long count= *(const long *)(other+SIMULATION_WORLD_UNKNOWN_0006A2A0_COUNT_OFFSET);

			*(long *)(other+SIMULATION_WORLD_UNKNOWN_0006A2A0_COUNT_OFFSET)= (long)((unsigned long)count+1);
			simulation_view_set_state(view, 2, count);
		}

		return;
	}
	simulation_view_set_state(view, 1, NONE);
}

/* 0x00069790: how many things wait for the thing to take, in its second
state. Of its views of the second kind that have a byte set, the one with
the least number is looked for: with none, the most that there can be; if
not, how far that number is after the one of the thing less 0x81, and none
if it is not after it. Then each view of the second kind is told whether it
is the one that keeps the thing from going on (simulation_view_set_blocked):
that one, when none wait */
long simulation_world_unknown_00069790(
	byte *world)
{
	struct simulation_view_iterator iterator= {_simulation_view_kind2_bit, 0};
	byte *slowest= NULL;
	byte *blocking= NULL;
	long result= LONG_MAX;
	long least= LONG_MAX;
	byte *view;

	while (simulation_world_next_view(&iterator, world, &view))
	{
		if (view[SIMULATION_VIEW_UNKNOWN78_OFFSET] && *(const long *)(view+SIMULATION_VIEW_UNKNOWN84_OFFSET)<least)
		{
			slowest= view;
			least= *(const long *)(view+SIMULATION_VIEW_UNKNOWN84_OFFSET);
		}
	}
	if (slowest)
	{
		long count= (long)((unsigned long)least-*(const unsigned long *)(world+SIMULATION_WORLD_UNKNOWN28_OFFSET)+
			SIMULATION_WORLD_UNKNOWN_00069790_REACH);

		if (count>0)
		{
			result= count;
		}
		else
		{
			result= 0;
			blocking= slowest;
		}
	}
	iterator.kind_mask= _simulation_view_kind2_bit;
	iterator.index= 0;
	while (simulation_world_next_view(&iterator, world, &view))
	{
		simulation_view_set_blocked(view, view==blocking);
	}

	return result;
}

/* 0x00069880: each view of the second kind that the thing has is given
one of the things that come and wait (simulation_view_send_update) */
void __attribute__((stdcall)) simulation_world_unknown_00069880(
	byte *world,
	const void *update)
{
	struct simulation_view_iterator iterator= {_simulation_view_kind2_bit, 0};
	byte *view;

	while (simulation_world_next_view(&iterator, world, &view))
	{
		simulation_view_send_update(view, update);
	}
}

/* 0x0006ace0: the view of the second kind or of the fourth that the thing
has with a number, or NULL */
byte *simulation_world_get_view_by_unknown1c(
	const byte *world,
	long unknown1c)
{
	struct simulation_view_iterator iterator= {SIMULATION_VIEW_KINDS_2_AND_4, 0};
	byte *view;

	while (simulation_world_next_view(&iterator, world, &view))
	{
		if (*(const long *)(view+SIMULATION_VIEW_UNKNOWN1C_OFFSET)==unknown1c)
		{
			return view;
		}
	}

	return NULL;
}

/* 0x0006ad40: the view of the second kind or of the fourth that the thing
has with the six bytes that it is known by, or NULL */
byte *__attribute__((stdcall)) simulation_world_get_view_by_unknown14(
	const byte *world,
	const word *unknown14)
{
	struct simulation_view_iterator iterator= {SIMULATION_VIEW_KINDS_2_AND_4, 0};
	byte *view;

	while (simulation_world_next_view(&iterator, world, &view))
	{
		if (memcmp(view+SIMULATION_VIEW_UNKNOWN14_OFFSET, unknown14, SIMULATION_VIEW_UNKNOWN14_SIZE)==0)
		{
			return view;
		}
	}

	return NULL;
}

/* 0x0006ac30: the first of the things that have come and wait is taken: a
copy of it is given, the next one is the first (and if it was the last,
there is no last), the heap of the network takes it back, there is one
fewer, and the number of the first goes on by one. The game does not look
whether there is one */
void simulation_world_take_update(
	byte *world,
	void *update)
{
	struct simulation_update_node *node= *(struct simulation_update_node **)(world+SIMULATION_WORLD_FIRST_UPDATE_POINTER_OFFSET);

	memcpy(update, node->update, sizeof(node->update));
	*(struct simulation_update_node **)(world+SIMULATION_WORLD_FIRST_UPDATE_POINTER_OFFSET)= node->next;
	if (*(struct simulation_update_node **)(world+SIMULATION_WORLD_LAST_UPDATE_POINTER_OFFSET)==node)
	{
		*(struct simulation_update_node **)(world+SIMULATION_WORLD_LAST_UPDATE_POINTER_OFFSET)= NULL;
	}
	network_heap_free(node);
	(*(long *)(world+SIMULATION_WORLD_UPDATE_COUNT_OFFSET))--;
	(*(long *)(world+SIMULATION_WORLD_FIRST_UPDATE_OFFSET))++;
}

/* 0x00069eb0: the first view of the first kind or of the third that the
thing has, if it is of a channel and is well, goes to its second state; and
the number that the thing has at 0x18 is 3, with nothing
(simulation_world_unknown_0006b1f0) */
void simulation_world_unknown_00069eb0(
	byte *world)
{
	byte *view= simulation_world_unknown_0006acb0(world);

	if (view && *(const long *)(view+SIMULATION_VIEW_CHANNEL_OFFSET)!=NONE &&
		*(const long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)==0)
	{
		simulation_view_set_state(view, 1, NONE);
		simulation_world_unknown_0006b1f0(world, 0);
	}
}

/* 0x0006b2a0: the thing starts to go, and its count starts over. One that
was going already is only not to bring the game up to what has come. If
not: in its third state, the things that have come and wait are done with
(simulation_world_clear_updates), and the next one that it waits for is the one of its
number; and it is going. And the number that it has at 0x18 is 4; if it was
4 already, the time is noted. The game has the time of the network written
here */
void simulation_world_unknown_0006b2a0(
	byte *world)
{
	*(long *)(world+SIMULATION_WORLD_UNKNOWN30_OFFSET)= 0;
	if (world[SIMULATION_WORLD_GOING_OFFSET])
	{
		world[SIMULATION_WORLD_CATCH_UP_OFFSET]= FALSE;
	}
	else
	{
		long number= *(const long *)(world+SIMULATION_WORLD_UNKNOWN28_OFFSET);

		if (*(const long *)(world+SIMULATION_WORLD_STATE_OFFSET)==_simulation_world_state3)
		{
			simulation_world_clear_updates(world);
			*(long *)(world+SIMULATION_WORLD_LAST_UPDATE_OFFSET)= (long)((unsigned long)number-1);
			*(long *)(world+SIMULATION_WORLD_FIRST_UPDATE_OFFSET)= number;
		}
		world[SIMULATION_WORLD_GOING_OFFSET]= TRUE;
	}
	if (*(const long *)(world+SIMULATION_WORLD_UNKNOWN18_OFFSET)==_simulation_world_unknown18_4)
	{
		*(long *)(world+SIMULATION_WORLD_UNKNOWN34_OFFSET)= unknown_00510548 ? unknown_0051054c : GetTickCount();
	}
	*(long *)(world+SIMULATION_WORLD_UNKNOWN18_OFFSET)= _simulation_world_unknown18_4;
}

/* 0x0006b110: the thing is told something of a view, with a byte: if the
byte is set, the view is the first of the first kind or of the third that
the thing has, and the number that the thing has at 0x18 is 3, something
follows (simulation_world_unknown_00069f10). The game has simulation_world_unknown_0006acb0
written here */
void __attribute__((stdcall)) simulation_world_unknown_0006b110(
	byte *world,
	boolean unknown,
	const byte *view)
{
	if (unknown && view==simulation_world_unknown_0006acb0(world) &&
		*(const long *)(world+SIMULATION_WORLD_UNKNOWN18_OFFSET)==_simulation_world_unknown18_3)
	{
		simulation_world_unknown_00069f10(world);
	}
}

/* 0x0006b160: the number that the thing has at 0x18 is another. Unless that
is 4, the thing is not to bring the game up to what has come, and is not
going; and if it was going, in its third state, the things that have come
and wait are done with (simulation_world_clear_updates). From 3, and not to 4: the memory
that the texture cache lent for the state of the game is given back, if the
thing was being sent it, and its count goes on by one. From 4: the time is
noted. The game has the time of the network written here */
void simulation_world_set_unknown18(
	byte *world,
	long unknown18)
{
	if (unknown18!=_simulation_world_unknown18_4)
	{
		world[SIMULATION_WORLD_CATCH_UP_OFFSET]= FALSE;
		if (world[SIMULATION_WORLD_GOING_OFFSET])
		{
			world[SIMULATION_WORLD_GOING_OFFSET]= FALSE;
			if (*(const long *)(world+SIMULATION_WORLD_STATE_OFFSET)==_simulation_world_state3)
			{
				simulation_world_clear_updates(world);
			}
		}
	}
	switch (*(const long *)(world+SIMULATION_WORLD_UNKNOWN18_OFFSET))
	{
	case _simulation_world_unknown18_3:
		if (unknown18!=_simulation_world_unknown18_4)
		{
			if (simulation_world_receiving_game_state(world))
			{
				simulation_world_unknown_0006aae0(world);
			}
			(*(long *)(world+SIMULATION_WORLD_UNKNOWN30_OFFSET))++;
		}
		break;
	case _simulation_world_unknown18_4:
		*(long *)(world+SIMULATION_WORLD_UNKNOWN34_OFFSET)= unknown_00510548 ? unknown_0051054c : GetTickCount();
		break;
	}
	*(long *)(world+SIMULATION_WORLD_UNKNOWN18_OFFSET)= unknown18;
}

/* 0x0006b1f0: the number that the thing has at 0x18 is 3
(simulation_world_set_unknown18, written here), and the time of that and a
number that is given are kept */
void __attribute__((stdcall)) simulation_world_unknown_0006b1f0(
	byte *world,
	long unknown)
{
	long time;

	simulation_world_set_unknown18(world, _simulation_world_unknown18_3);
	time= unknown_00510548 ? unknown_0051054c : GetTickCount();
	*(long *)(world+SIMULATION_WORLD_UNKNOWN20_OFFSET)= unknown;
	*(long *)(world+SIMULATION_WORLD_UNKNOWN1C_OFFSET)= time;
}

/* 0x0006b040: the thing gives up: its views are done with
(unknown_000693a0), and if the number that it has at 0x18 was from 3 to 6, it
is 2 (simulation_world_set_unknown18), and the time is noted. The game has
the time of the network written here */
void simulation_world_unknown_0006b040(
	byte *world)
{
	long unknown18;

	unknown_000693a0(world);
	unknown18= *(const long *)(world+SIMULATION_WORLD_UNKNOWN18_OFFSET);
	if (unknown18>=_simulation_world_unknown18_3 && unknown18<=_simulation_world_unknown18_6)
	{
		simulation_world_set_unknown18(world, _simulation_world_unknown18_2);
		*(long *)(world+SIMULATION_WORLD_UNKNOWN1C_OFFSET)= unknown_00510548 ? unknown_0051054c : GetTickCount();
	}
}

/* 0x00069f90: the thing has been on its way for too long, or too many
times: if its count is as many as the game has for it, or the time since the
one that it noted is as long as the game has for it, and the number that it
has at 0x18 is not 1, it gives up (simulation_world_unknown_0006b040) and
that number is 1 */
void simulation_world_unknown_00069f90(
	byte *world)
{
	long since= *(const long *)(world+SIMULATION_WORLD_UNKNOWN34_OFFSET);
	long now= unknown_00510548 ? unknown_0051054c : GetTickCount();

	if (*(const long *)(world+SIMULATION_WORLD_UNKNOWN30_OFFSET)<unknown_004ced58 &&
		(long)((unsigned long)now-(unsigned long)since)<unknown_004ced5c)
	{
		return;
	}
	if (*(const long *)(world+SIMULATION_WORLD_UNKNOWN18_OFFSET)!=_simulation_world_unknown18_1)
	{
		simulation_world_unknown_0006b040(world);
		simulation_world_set_unknown18(world, _simulation_world_unknown18_1);
	}
}

/* 0x00069fe0: the thing gives up (simulation_world_unknown_0006b040) if it
has no view of the first kind or of the third; or if the first one is of no
channel, or is not ready, and the number that the thing has at 0x18 is not 3.
The game has simulation_world_unknown_0006acb0 written here */
void simulation_world_unknown_00069fe0(
	byte *world)
{
	const byte *view= simulation_world_unknown_0006acb0(world);

	if (view)
	{
		if (*(const long *)(view+SIMULATION_VIEW_CHANNEL_OFFSET)!=NONE && view[SIMULATION_VIEW_READY_OFFSET])
		{
			return;
		}
		if (*(const long *)(world+SIMULATION_WORLD_UNKNOWN18_OFFSET)==_simulation_world_unknown18_3)
		{
			return;
		}
	}
	simulation_world_unknown_0006b040(world);
}

/* 0x0006b090: the thing is told that a view is ready, or not: if it is, and
it is the first view of the first kind or of the third that the thing has,
the thing is given a set byte (unknown_0006a560). Before that, the ninth
byte of unknown_004cf770 is cleared if it was set; if it was not, the tenth
is set. The game has simulation_world_unknown_0006acb0 written here */
void __attribute__((stdcall)) simulation_world_unknown_0006b090(
	boolean ready,
	byte *world,
	const byte *view)
{
	if (ready && view==simulation_world_unknown_0006acb0(world))
	{
		if (unknown_004cf770.unknown1[7])
		{
			unknown_004cf770.unknown1[7]= FALSE;
		}
		else
		{
			unknown_004cf770.unknown1[8]= TRUE;
		}
		unknown_0006a560(world, TRUE);
	}
}

/* 0x0006ab10: the things that have come and wait are done with: the heap of
the network takes each back, and the thing has none, with NONE for the last
that came */
void __attribute__((stdcall)) simulation_world_clear_updates(
	byte *world)
{
	struct simulation_update_node *node= *(struct simulation_update_node **)(world+SIMULATION_WORLD_FIRST_UPDATE_POINTER_OFFSET);

	while (node)
	{
		struct simulation_update_node *next= node->next;

		network_heap_free(node);
		node= next;
	}
	*(struct simulation_update_node **)(world+SIMULATION_WORLD_FIRST_UPDATE_POINTER_OFFSET)= NULL;
	*(struct simulation_update_node **)(world+SIMULATION_WORLD_LAST_UPDATE_POINTER_OFFSET)= NULL;
	*(long *)(world+SIMULATION_WORLD_UPDATE_COUNT_OFFSET)= 0;
	*(long *)(world+SIMULATION_WORLD_LAST_UPDATE_OFFSET)= NONE;
	*(long *)(world+SIMULATION_WORLD_FIRST_UPDATE_OFFSET)= 0;
}

/* 0x0006ab90: something came for the thing to take, which starts with its
number: a copy of it, in memory of the heap of the network, goes after the
last of the ones that wait, and its number is the one of the last that came.
FALSE, with nothing done, if there is no memory. The game has
network_heap_allocate written here */
boolean __attribute__((stdcall)) simulation_world_add_update(
	byte *world,
	const long *update)
{
	struct simulation_update_node *node= network_heap_allocate(sizeof(struct simulation_update_node));
	struct simulation_update_node *last;

	if (!node)
	{
		return FALSE;
	}
	last= *(struct simulation_update_node **)(world+SIMULATION_WORLD_LAST_UPDATE_POINTER_OFFSET);
	if (last)
	{
		last->next= node;
	}
	else
	{
		*(struct simulation_update_node **)(world+SIMULATION_WORLD_FIRST_UPDATE_POINTER_OFFSET)= node;
	}
	node->next= NULL;
	(*(long *)(world+SIMULATION_WORLD_UPDATE_COUNT_OFFSET))++;
	*(struct simulation_update_node **)(world+SIMULATION_WORLD_LAST_UPDATE_POINTER_OFFSET)= node;
	memcpy(node->update, update, sizeof(node->update));
	*(long *)(world+SIMULATION_WORLD_LAST_UPDATE_OFFSET)= update[0];

	return TRUE;
}

/* 0x0006a990: a piece of the state of the game came, of a size, that goes at
a place; whether it was taken. It is if the thing is being sent the state of
the game, the place is where the bytes that have come end, there is memory
for it, and the piece is of some size and does not go past 0x40000 bytes: it
is copied there, and so many more bytes have come */
boolean __attribute__((stdcall)) simulation_world_receive_game_state(
	byte *world,
	long offset,
	long size,
	const void *data)
{
	boolean result= FALSE;

	if (simulation_world_receiving_game_state(world) &&
		*(const long *)(world+SIMULATION_WORLD_UNKNOWN1200_OFFSET)==offset)
	{
		byte *memory= *(byte **)(world+SIMULATION_WORLD_MEMORY_OFFSET);

		if (memory && offset>=0 && size>0 &&
			(long)((unsigned long)offset+(unsigned long)size)<=MAXIMUM_SIMULATION_WORLD_GAME_STATE_SIZE)
		{
			memcpy(memory+offset, data, size);
			*(long *)(world+SIMULATION_WORLD_UNKNOWN1200_OFFSET)+= size;
			result= TRUE;
		}
	}

	return result;
}

/* 0x00069f10: the thing looks at its first view of the first kind or of the
third. If that one has its byte set, the thing starts to go
(simulation_world_unknown_0006b2a0). If not, it is waited for as long as the
game has for it, from the time that the thing noted; after that, or with no
such view, the thing gives up (simulation_world_unknown_0006b040). The game
has simulation_world_unknown_0006acb0 and network_time_since written here */
void simulation_world_unknown_00069f10(
	byte *world)
{
	const byte *view= simulation_world_unknown_0006acb0(world);

	if (view)
	{
		long since;
		long now;

		if (view[SIMULATION_VIEW_UNKNOWN78_OFFSET])
		{
			simulation_world_unknown_0006b2a0(world);

			return;
		}
		since= *(const long *)(world+SIMULATION_WORLD_UNKNOWN1C_OFFSET);
		now= unknown_00510548 ? unknown_0051054c : GetTickCount();
		if ((long)((unsigned long)now-(unsigned long)since)<unknown_004ced4c)
		{
			return;
		}
	}
	simulation_world_unknown_0006b040(world);
}

/* 0x0006a3b0: what the thing has of a player of a console, by the six bytes
that the console is known by and which of its players it is; NULL if there
is no such console or no such player. The game looks through a copy of the
consoles of players_globals */
byte *__attribute__((stdcall)) simulation_world_find_player(
	byte *world,
	const word *unknown14,
	long local_index)
{
	const byte *globals= (const byte *)players_globals;
	unsigned long machine_mask= *(const unsigned long *)(globals+PLAYERS_GLOBALS_MACHINE_MASK_OFFSET);
	long machine;
	long player;

	for (machine= 0; machine<NUMBER_OF_SIMULATION_MACHINES; machine++)
	{
		if ((machine_mask&(1u<<machine)) && memcmp(unknown14,
			globals+PLAYERS_GLOBALS_MACHINES_OFFSET+machine*SIMULATION_VIEW_UNKNOWN14_SIZE, SIMULATION_VIEW_UNKNOWN14_SIZE)==0)
		{
			break;
		}
	}
	if (machine==NUMBER_OF_SIMULATION_MACHINES)
	{
		return NULL;
	}
	for (player= 0; player<NUMBER_OF_SIMULATION_WORLD_PLAYERS; player++)
	{
		byte *player_data_= world+player*SIMULATION_WORLD_PLAYER_SIZE+SIMULATION_WORLD_PLAYERS_OFFSET;

		if (*(const long *)player_data_!=NONE)
		{
			const byte *datum= (const byte *)player_data->data+
				(*(const unsigned long *)(player_data_+SIMULATION_WORLD_PLAYER_INDEX_OFFSET)&0xFFFF)*SIMULATION_PLAYER_DATUM_SIZE;

			if (*(const short *)(datum+SIMULATION_PLAYER_DATUM_MACHINE_OFFSET)==machine &&
				*(const short *)(datum+SIMULATION_PLAYER_DATUM_LOCAL_INDEX_OFFSET)==local_index)
			{
				return player_data_;
			}
		}
	}

	return NULL;
}

/* 0x0006a7f0: the thing takes what some players of a console do, as four
bits say which: for each of them that the thing has (simulation_world_find_player)
and that is in its third state, what it does is kept, with the time of the
game */
void __attribute__((stdcall)) simulation_world_set_player_actions(
	byte *world,
	const word *unknown14,
	long players,
	const void *actions)
{
	long index;

	for (index= 0; index<MAXIMUM_SIMULATION_CONSOLE_PLAYERS; index++)
	{
		if (players&(1<<index))
		{
			byte *player= simulation_world_find_player(world, unknown14, index);

			if (player && *(const long *)(player+SIMULATION_WORLD_PLAYER_STATE_OFFSET)==_simulation_world_player_state3)
			{
				memcpy(player+SIMULATION_WORLD_PLAYER_ACTIONS_OFFSET, (const byte *)actions+index*SIMULATION_PLAYER_ACTIONS_SIZE,
					SIMULATION_PLAYER_ACTIONS_SIZE);
				*(unsigned long *)(player+SIMULATION_WORLD_PLAYER_ACTIONS_TIME_OFFSET)= game_time_globals->time;
			}
		}
	}
}

/* 0x0006aa20: all of the state of the game has come, so many bytes; whether
it is the state of the game now. Nothing is done unless the thing is being
sent it, that many bytes have come, and there is memory with them. Between
what is done before a state is loaded and after, it is put where the state
of the game is (unknown_00199740); if that is not good, or it is not of the
size of the whole state, the game says that its files are damaged
(main_files_damaged). Then the memory goes back to the texture cache, as
simulation_world_unknown_0006aae0 does (written here) */
boolean __attribute__((stdcall)) simulation_world_finish_game_state(
	byte *world,
	long size)
{
	boolean result= FALSE;

	if (simulation_world_receiving_game_state(world) &&
		*(const long *)(world+SIMULATION_WORLD_UNKNOWN1200_OFFSET)==size &&
		*(void *const *)(world+SIMULATION_WORLD_MEMORY_OFFSET))
	{
		void *base= game_state_globals.base_address;
		long loaded_size;

		game_state_call_before_load_procs(0);
		if (unknown_00199740(*(void *const *)(world+SIMULATION_WORLD_MEMORY_OFFSET), size, base, &loaded_size) &&
			loaded_size==SIMULATION_GAME_STATE_SIZE)
		{
			result= TRUE;
		}
		else
		{
			main_files_damaged();
		}
		game_state_call_after_load_procs(0);
		simulation_world_unknown_0006aae0(world);
	}

	return result;
}

/* 0x0006a600: the thing has none of the players of the game: each one that
it had has NONE for its three numbers, and nothing at 0x20. The game has the
sixteen written four by four */
void simulation_world_players_clear(
	byte *world)
{
	long player;

	for (player= 0; player<NUMBER_OF_SIMULATION_WORLD_PLAYERS; player++)
	{
		byte *player_data_= world+player*SIMULATION_WORLD_PLAYER_SIZE+SIMULATION_WORLD_PLAYERS_OFFSET;

		if (*(const long *)player_data_!=NONE)
		{
			*(long *)player_data_= NONE;
			*(long *)(player_data_+SIMULATION_WORLD_PLAYER_INDEX_OFFSET)= NONE;
			*(long *)(player_data_+SIMULATION_WORLD_PLAYER_STATE_OFFSET)= NONE;
			*(long *)(player_data_+SIMULATION_WORLD_PLAYER_UNKNOWN20_OFFSET)= 0;
		}
	}
}

/* 0x0006a6f0: the thing has none of its sixteen other things: each one that
it had has NONE for its two numbers, and is of nothing */
void simulation_world_others_clear(
	byte *world)
{
	long index;

	for (index= 0; index<NUMBER_OF_SIMULATION_WORLD_OTHERS; index++)
	{
		byte *other= world+index*SIMULATION_WORLD_OTHER_SIZE+SIMULATION_WORLD_OTHERS_OFFSET;

		if (*(const long *)other!=NONE)
		{
			*(long *)other= NONE;
			*(long *)(other+SIMULATION_WORLD_OTHER_UNKNOWN4_OFFSET)= NONE;
			*(void **)(other+SIMULATION_WORLD_OTHER_WORLD_OFFSET)= NULL;
		}
	}
}

/* 0x0006a690: the thing has one more of its other things, with a number:
at a place that is given, or with NONE the first that is free; its place, or
NONE if there is none free. It is of the thing, with NONE at 0x10, and what
it has at 0x14 is made ready (unknown_000c6ef0) */
long __attribute__((stdcall)) simulation_world_other_new(
	long index,
	byte *world,
	long unknown4)
{
	byte *other;

	if (index==NONE)
	{
		for (index= 0; index<NUMBER_OF_SIMULATION_WORLD_OTHERS; index++)
		{
			if (*(const long *)(world+index*SIMULATION_WORLD_OTHER_SIZE+SIMULATION_WORLD_OTHERS_OFFSET)==NONE)
			{
				break;
			}
		}
		if (index==NUMBER_OF_SIMULATION_WORLD_OTHERS)
		{
			return NONE;
		}
	}
	other= world+index*SIMULATION_WORLD_OTHER_SIZE+SIMULATION_WORLD_OTHERS_OFFSET;
	*(byte **)(other+SIMULATION_WORLD_OTHER_WORLD_OFFSET)= world;
	*(long *)other= index;
	*(long *)(other+SIMULATION_WORLD_OTHER_UNKNOWN4_OFFSET)= unknown4;
	*(long *)(other+SIMULATION_WORLD_OTHER_UNKNOWN10_OFFSET)= NONE;
	unknown_000c6ef0(other+SIMULATION_WORLD_OTHER_UNKNOWN14_OFFSET);

	return index;
}

/* 0x000699a0: a number of a player of the game or of one of the other
things of the thing, which are counted after the sixteen players: of a
player that there is, what it has at 0x2C; of another thing that there is,
its number; NONE if there is none */
long simulation_world_unknown_000699a0(
	long index,
	const byte *world)
{
	if (index<NUMBER_OF_SIMULATION_WORLD_PLAYERS)
	{
		if (index!=NONE && index>=0 && index<player_data->first_unallocated)
		{
			const byte *datum= (const byte *)player_data->data+player_data->size*index;

			if (*(const short *)datum!=0)
			{
				return *(const long *)(datum+SIMULATION_PLAYER_DATUM_UNKNOWN2C_OFFSET);
			}
		}
	}
	else if (index<NUMBER_OF_SIMULATION_WORLD_PLAYERS+NUMBER_OF_SIMULATION_WORLD_OTHERS)
	{
		const byte *other= world+(index-NUMBER_OF_SIMULATION_WORLD_PLAYERS)*SIMULATION_WORLD_OTHER_SIZE+SIMULATION_WORLD_OTHERS_OFFSET;

		if (*(const long *)other!=NONE)
		{
			return *(const long *)(other+SIMULATION_WORLD_OTHER_UNKNOWN4_OFFSET);
		}
	}

	return NONE;
}

/* 0x0006a770: the thing has the players of the game again: none
(simulation_world_players_clear), and then each player that there is in the
game, but for the ones with a bit, is one that it gets (unknown_000694c0).
The game has the going through the players written here */
void __attribute__((stdcall)) simulation_world_unknown_0006a770(
	byte *world)
{
	long index;

	simulation_world_players_clear(world);
	for (index= 0; index<player_data->first_unallocated; index++)
	{
		if (player_data->in_use_bit_vector[index>>5]&(1u<<(index&31)))
		{
			const byte *datum= (const byte *)player_data->data+player_data->size*index;

			if (!(datum[SIMULATION_PLAYER_DATUM_FLAGS_OFFSET]&_simulation_player_datum_unknown_bit))
			{
				unknown_000694c0(world, (long)(((unsigned long)(long)*(const short *)datum<<16)|(unsigned long)index));
			}
		}
	}
}

/* 0x00069580: the thing has a player of the game no more: what it had of it
has NONE for its three numbers and nothing at 0x20; and each view of the
second kind or of the fourth that is of a channel and is ready knows of that
player no more */
void simulation_world_player_delete(
	long player,
	byte *world)
{
	struct simulation_view_iterator iterator= {SIMULATION_VIEW_KINDS_2_AND_4, 0};
	unsigned long index= (unsigned long)player&0xFFFF;
	byte *player_data_= world+index*SIMULATION_WORLD_PLAYER_SIZE+SIMULATION_WORLD_PLAYERS_OFFSET;
	byte *view;

	*(long *)(player_data_+SIMULATION_WORLD_PLAYER_UNKNOWN20_OFFSET)= 0;
	*(long *)player_data_= NONE;
	*(long *)(player_data_+SIMULATION_WORLD_PLAYER_INDEX_OFFSET)= NONE;
	*(long *)(player_data_+SIMULATION_WORLD_PLAYER_STATE_OFFSET)= NONE;
	while (simulation_world_next_view(&iterator, world, &view))
	{
		if (*(const long *)(view+SIMULATION_VIEW_CHANNEL_OFFSET)!=NONE && view[SIMULATION_VIEW_READY_OFFSET])
		{
			*(unsigned long *)(view+SIMULATION_VIEW_PLAYER_MASK_OFFSET)&= ~(1u<<(index&31));
		}
	}
}

/* 0x000696f0: a bit for each player that the thing has whose byte is set */
unsigned long simulation_world_players_unknown25_mask(
	const byte *world)
{
	unsigned long mask= 0;
	long player;

	for (player= 0; player<NUMBER_OF_SIMULATION_WORLD_PLAYERS; player++)
	{
		const byte *player_data_= world+player*SIMULATION_WORLD_PLAYER_SIZE+SIMULATION_WORLD_PLAYERS_OFFSET;

		if (*(const long *)player_data_!=NONE && player_data_[SIMULATION_WORLD_PLAYER_UNKNOWN25_OFFSET])
		{
			mask|= 1u<<player;
		}
	}

	return mask;
}

/* 0x0006a480: a bit for each player that the thing has and that every view
of the second kind or of the fourth that is past being made ready knows of */
unsigned long simulation_world_players_known_mask(
	const byte *world)
{
	struct simulation_view_iterator iterator= {SIMULATION_VIEW_KINDS_2_AND_4, 0};
	unsigned long mask= 0;
	long player;
	byte *view;

	for (player= 0; player<NUMBER_OF_SIMULATION_WORLD_PLAYERS; player++)
	{
		if (*(const long *)(world+player*SIMULATION_WORLD_PLAYER_SIZE+SIMULATION_WORLD_PLAYERS_OFFSET)!=NONE)
		{
			mask|= 1u<<player;
		}
	}
	while (simulation_world_next_view(&iterator, world, &view))
	{
		if (simulation_view_established(view))
		{
			mask&= *(const unsigned long *)(view+SIMULATION_VIEW_PLAYER_MASK_OFFSET);
		}
	}

	return mask;
}

/* 0x00069640: whether a player that the thing has, by the low word of its
number, is the one that three numbers say: they are the ones that the thing
has of it, and what the thing has first has it with them (unknown_00083bd0).
FALSE with no such player */
boolean __attribute__((stdcall)) simulation_world_player_matches(
	long player,
	const byte *world,
	const long *identifier)
{
	unsigned long index= (unsigned long)player&0xFFFF;
	const byte *player_data_;

	if (index>=NUMBER_OF_SIMULATION_WORLD_PLAYERS)
	{
		return FALSE;
	}
	player_data_= world+index*SIMULATION_WORLD_PLAYER_SIZE+SIMULATION_WORLD_PLAYERS_OFFSET;
	if (*(const long *)player_data_==NONE ||
		memcmp(identifier, player_data_+SIMULATION_WORLD_PLAYER_IDENTIFIER_OFFSET, SIMULATION_PLAYER_IDENTIFIER_SIZE)!=0)
	{
		return FALSE;
	}

	return unknown_00083bd0((long)index, *(const void *const *)(world+SIMULATION_WORLD_UNKNOWN0_OFFSET), identifier)!=FALSE;
}

/* 0x0006b310: the number that the thing has at 0x18 goes on: from 4 to 5,
and from 3 or 5 to 6 (simulation_world_set_unknown18) */
void simulation_world_unknown_0006b310(
	byte *world)
{
	long unknown18= *(const long *)(world+SIMULATION_WORLD_UNKNOWN18_OFFSET);

	if (unknown18==_simulation_world_unknown18_4)
	{
		simulation_world_set_unknown18(world, _simulation_world_unknown18_5);
	}
	else if (unknown18>=_simulation_world_unknown18_3 && unknown18<_simulation_world_unknown18_6)
	{
		simulation_world_set_unknown18(world, _simulation_world_unknown18_6);
	}
}

/* 0x0006b350: the number that the thing has at 0x18 is 6, if it was 3, 4 or
5 */
void simulation_world_unknown_0006b350(
	byte *world)
{
	long unknown18= *(const long *)(world+SIMULATION_WORLD_UNKNOWN18_OFFSET);

	if (unknown18>=_simulation_world_unknown18_3 && unknown18<_simulation_world_unknown18_6)
	{
		simulation_world_set_unknown18(world, _simulation_world_unknown18_6);
	}
}

/* 0x00069d50: the thing, unless it is in its first state, brings on each
view of the second kind or of the fourth that is well, has not its byte at
0x78 set, and is not in its sixth state: one that is of a channel and ready,
with simulation_world_advance_view; any other goes on to its next state
(simulation_world_unknown_0006a2a0) */
void simulation_world_unknown_00069d50(
	byte *world)
{
	struct simulation_view_iterator iterator= {SIMULATION_VIEW_KINDS_2_AND_4, 0};
	byte *view;

	if (*(const long *)(world+SIMULATION_WORLD_STATE_OFFSET)==_simulation_world_state1)
	{
		return;
	}
	while (simulation_world_next_view(&iterator, world, &view))
	{
		if (*(const long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)==0 && !view[SIMULATION_VIEW_UNKNOWN78_OFFSET] &&
			*(const long *)(view+SIMULATION_VIEW_STATE_OFFSET)!=_simulation_view_state5)
		{
			if (*(const long *)(view+SIMULATION_VIEW_CHANNEL_OFFSET)!=NONE && view[SIMULATION_VIEW_READY_OFFSET])
			{
				simulation_world_advance_view(view, world);
			}
			else
			{
				simulation_world_unknown_0006a2a0(view, world);
			}
		}
	}
}

/* 0x00069c80: the thing waits for the views of the other consoles before
it starts to go. For each console that there is, but for its own: the view
that it has with that number (simulation_world_get_view_by_unknown1c), if
it is well, is brought on as simulation_world_unknown_00069d50 does; and one
that is ready, of a console that the thing waits for (a bit of what it has
at 0x20), is there when simulation_world_advance_view says so. When no console that it
waits for is missing, or the time that the game has for it has gone by since
the one that the thing noted, the thing starts to go
(simulation_world_unknown_0006b2a0). The game has the time of the network
written here */
void simulation_world_unknown_00069c80(
	byte *world)
{
	unsigned long machines= *(const unsigned long *)(*(const byte *const *)(world+SIMULATION_WORLD_UNKNOWN0_OFFSET)+
		SIMULATION_WORLD_UNKNOWN0_MACHINE_MASK_OFFSET);
	long since= *(const long *)(world+SIMULATION_WORLD_UNKNOWN1C_OFFSET);
	long elapsed= (long)((unsigned long)(unknown_00510548 ? unknown_0051054c : GetTickCount())-(unsigned long)since);
	unsigned long others= 0;
	unsigned long there= 0;
	long machine;

	for (machine= 0; machine<NUMBER_OF_SIMULATION_MACHINES; machine++)
	{
		unsigned long bit= 1u<<machine;

		if ((machines&bit) && machine!=*(const long *)(world+SIMULATION_WORLD_MACHINE_INDEX_OFFSET))
		{
			byte *view= simulation_world_get_view_by_unknown1c(world, machine);

			others|= bit;
			if (view && *(const long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)==0)
			{
				if (*(const long *)(view+SIMULATION_VIEW_CHANNEL_OFFSET)!=NONE && view[SIMULATION_VIEW_READY_OFFSET])
				{
					if ((*(const unsigned long *)(world+SIMULATION_WORLD_UNKNOWN20_OFFSET)&bit) && simulation_world_advance_view(view, world))
					{
						there|= bit;
					}
				}
				else
				{
					simulation_world_unknown_0006a2a0(view, world);
				}
			}
		}
	}
	if (!(others&*(const unsigned long *)(world+SIMULATION_WORLD_UNKNOWN20_OFFSET)&~there) || elapsed>=unknown_004ced4c)
	{
		simulation_world_unknown_0006b2a0(world);
	}
}

/* whether a view knows of every player of the thing whose byte is set */
static inline __attribute__((always_inline)) boolean simulation_world_view_knows_players(
	const byte *view,
	const byte *world)
{
	unsigned long needed= simulation_world_players_unknown25_mask(world);

	return (needed&*(const unsigned long *)(view+SIMULATION_VIEW_PLAYER_MASK_OFFSET))==needed;
}

/* 0x0006a040: a view that is ready is brought on by the thing, a state at a
time; 1 when it is where it has to be, 3 when it could not be, and nothing
while it is on its way.

Before its fourth state: it goes to it, if it is of the second kind or knows
of every player of the thing whose byte is set.

In its fourth state: a view that is not of the second kind goes to its
fifth, with a byte set in what it has at 0x8. One of the second kind goes to
its fifth at once if the game has not started; if it has, it is sent the
state of the game first (simulation_view_begin_send_game_state), which the thing starts with one
view at a time: only if fewer views than the game has for it are being sent
it, and so long has gone by since it last started. When the view has it all,
it goes to its fifth state; if not, the time is noted, and a view that has
been sent it too many times could not be brought on.

In its fifth state: if it has nothing more to send (simulation_view_nothing_to_send) and
knows of every player, it is there; if not, nothing is said. And one that is
there, with the number that the thing has at 0x18 being 4, goes to its sixth
state (one of the second kind gives back its memory first, and any other has
simulation_view_entities_pending called).

A view that could not be brought on goes to its first state and is as 3
says: the game has simulation_view_fail written here, and
simulation_view_set_state in it */
long simulation_world_advance_view(
	byte *view,
	byte *world)
{
	long result= _simulation_world_advance_view_waiting;
	boolean second_kind= *(const short *)(view+SIMULATION_VIEW_KIND_OFFSET)==2;

	if (*(const long *)(view+SIMULATION_VIEW_STATE_OFFSET)<3)
	{
		if (second_kind || simulation_world_view_knows_players(view, world))
		{
			simulation_view_set_state(view, 3, *(const long *)(view+SIMULATION_VIEW_STATE_NUMBER_OFFSET));
		}
	}
	if (*(const long *)(view+SIMULATION_VIEW_STATE_OFFSET)==3)
	{
		if (!second_kind)
		{
			(*(byte *const *)(view+SIMULATION_VIEW_UNKNOWN8_OFFSET))[SIMULATION_VIEW_UNKNOWN8_UNKNOWN39_OFFSET]= TRUE;
			simulation_view_set_state(view, 4, *(const long *)(view+SIMULATION_VIEW_STATE_NUMBER_OFFSET));
		}
		else if (game_time_globals->time==0)
		{
			simulation_view_set_state(view, 4, *(const long *)(view+SIMULATION_VIEW_STATE_NUMBER_OFFSET));
		}
		else
		{
			struct simulation_view_iterator iterator= {_simulation_view_kind2_bit, 0};
			boolean allowed= TRUE;
			long count= 0;
			byte *other;
			long last;

			while (simulation_world_next_view(&iterator, world, &other))
			{
				if (*(void *const *)(other+SIMULATION_VIEW_UNKNOWN94_OFFSET))
				{
					count++;
				}
			}
			if (count>=unknown_004ced48)
			{
				allowed= FALSE;
			}
			last= *(const long *)(world+SIMULATION_WORLD_GAME_STATE_SEND_TIME_OFFSET);
			if ((last==NONE || network_time_since(last)>=unknown_004ced54) && allowed)
			{
				if (simulation_view_begin_send_game_state(view))
				{
					simulation_view_set_state(view, 4, *(const long *)(view+SIMULATION_VIEW_STATE_NUMBER_OFFSET));
				}
				else
				{
					long times= *(const long *)(view+SIMULATION_VIEW_GAME_STATE_SEND_COUNT_OFFSET);

					*(long *)(world+SIMULATION_WORLD_GAME_STATE_SEND_TIME_OFFSET)= network_time();
					if (times>=unknown_004ced50)
					{
						result= _simulation_world_advance_view_failed;
					}
				}
			}
		}
	}
	if (*(const long *)(view+SIMULATION_VIEW_STATE_OFFSET)==4)
	{
		if (!(simulation_view_nothing_to_send(view) && simulation_world_view_knows_players(view, world)))
		{
			return _simulation_world_advance_view_waiting;
		}
		result= _simulation_world_advance_view_there;
	}
	if (result==_simulation_world_advance_view_there)
	{
		if (*(const long *)(view+SIMULATION_VIEW_STATE_OFFSET)==4 &&
			*(const long *)(world+SIMULATION_WORLD_UNKNOWN18_OFFSET)==_simulation_world_unknown18_4)
		{
			if (!second_kind)
			{
				simulation_view_entities_pending(view);
			}
			else if (*(void *const *)(view+SIMULATION_VIEW_UNKNOWN94_OFFSET))
			{
				simulation_view_unknown_00086590(view);
			}
			simulation_view_set_state(view, 5, *(const long *)(view+SIMULATION_VIEW_STATE_NUMBER_OFFSET));
		}
	}
	else if (result==_simulation_world_advance_view_failed && *(const long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)==0)
	{
		if (!(*(const long *)(view+SIMULATION_VIEW_STATE_OFFSET)==_simulation_view_state0 &&
			*(const long *)(view+SIMULATION_VIEW_STATE_NUMBER_OFFSET)==NONE))
		{
			struct simulation_view_state_message message;

			*(long *)(view+SIMULATION_VIEW_STATE_OFFSET)= _simulation_view_state0;
			*(long *)(view+SIMULATION_VIEW_STATE_NUMBER_OFFSET)= NONE;
			message.state= _simulation_view_state0;
			message.number= NONE;
			if (*(const long *)(view+SIMULATION_VIEW_OBSERVER_CHANNEL_OFFSET)!=NONE)
			{
				network_observer_channel_send(FALSE, *(const long *)(view+SIMULATION_VIEW_OBSERVER_CHANNEL_OFFSET),
					*(byte *const *)(view+SIMULATION_VIEW_OBSERVER_OFFSET), SIMULATION_VIEW_SESSION_INDEX,
					_network_message_simulation_view_state, sizeof(message), &message);
			}
			simulation_view_update_ready(view);
		}
		*(long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)= 3;
	}

	return result;
}

/* 0x00069dd0: each view of the thing that is well and past its third state
goes back to it, with its number (simulation_view_set_state, written here) */
void simulation_world_unknown_00069dd0(
	byte *world)
{
	struct simulation_view_iterator iterator= {0xFFFFFFFF, 0};
	byte *view;

	while (simulation_world_next_view(&iterator, world, &view))
	{
		if (*(const long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)==0 &&
			*(const long *)(view+SIMULATION_VIEW_STATE_OFFSET)>_simulation_view_state2)
		{
			simulation_view_set_state(view, _simulation_view_state2, *(const long *)(view+SIMULATION_VIEW_STATE_NUMBER_OFFSET));
		}
	}
}

/* 0x0006a8a0: the thing makes ready to be sent the state of the game: it
borrows 0x40000 bytes from the texture cache, waiting a little for them, and
no bytes have come; FALSE if there is no memory. The game has
texture_cache_steal_memory written here */
boolean __attribute__((stdcall)) simulation_world_begin_game_state(
	byte *world)
{
	void *memory= texture_cache_steal_memory(_texture_cache_steal_wait_short, MAXIMUM_SIMULATION_WORLD_GAME_STATE_SIZE, 0,
		NULL, NULL);

	*(void **)(world+SIMULATION_WORLD_MEMORY_OFFSET)= memory;
	if (!memory)
	{
		return FALSE;
	}
	*(long *)(world+SIMULATION_WORLD_UNKNOWN1200_OFFSET)= 0;

	return TRUE;
}

/* 0x0006a2e0: the byte of each player that the thing has is set when the
player is one that all know of: it is not set yet, what the thing has first
has the player with the same three numbers and is to count it, every view
that is past being made ready knows of it (simulation_world_players_known_mask),
and another byte of it is clear */
void __attribute__((stdcall)) simulation_world_update_players_unknown25(
	byte *world)
{
	unsigned long known= simulation_world_players_known_mask(world);
	long player;

	for (player= 0; player<NUMBER_OF_SIMULATION_WORLD_PLAYERS; player++)
	{
		byte *player_data_= world+player*SIMULATION_WORLD_PLAYER_SIZE+SIMULATION_WORLD_PLAYERS_OFFSET;

		if (*(const long *)player_data_!=NONE && !player_data_[SIMULATION_WORLD_PLAYER_UNKNOWN25_OFFSET])
		{
			const byte *unknown0= *(const byte *const *)(world+SIMULATION_WORLD_UNKNOWN0_OFFSET);
			unsigned long bit= 1u<<player;

			if (bit&*(const unsigned long *)(unknown0+SIMULATION_WORLD_UNKNOWN0_PLAYER_MASK_OFFSET))
			{
				const byte *other= unknown0+SIMULATION_WORLD_UNKNOWN0_PLAYERS_OFFSET+player*SIMULATION_WORLD_UNKNOWN0_PLAYER_SIZE;

				if (memcmp(player_data_+SIMULATION_WORLD_PLAYER_IDENTIFIER_OFFSET, other, SIMULATION_PLAYER_IDENTIFIER_SIZE)==0 &&
					!other[SIMULATION_WORLD_UNKNOWN0_PLAYER_UNKNOWNC_OFFSET] && (known&bit) &&
					!player_data_[SIMULATION_WORLD_PLAYER_UNKNOWN24_OFFSET])
				{
					player_data_[SIMULATION_WORLD_PLAYER_UNKNOWN25_OFFSET]= TRUE;
				}
			}
		}
	}
}

/* 0x0006a860: the thing makes ready to send the state of the game: what is
done before a state is saved is done, a byte of the thing says that it is
being sent, and its size is given, which is the one of the whole state. The
game has game_state_call_before_save_procs written here */
void __attribute__((stdcall)) simulation_world_before_send_game_state(
	byte *world,
	long *size_out)
{
	long index;

	for (index= 0; index<NUMBER_OF_GAME_STATE_BEFORE_SAVE_PROCS; index++)
	{
		game_state_before_save_procs[index](0);
	}
	world[SIMULATION_WORLD_SENDING_GAME_STATE_OFFSET]= TRUE;
	*size_out= SIMULATION_GAME_STATE_SIZE;
}

/* 0x00069c50: the thing starts to wait before it goes: the number that it
has at 0x18 is 3 (simulation_world_unknown_0006b1f0), with the consoles that
it waits for: the ones that what it has first has. In its first state it
waits for none, and starts to go at once
(simulation_world_unknown_0006b2a0) */
void simulation_world_unknown_00069c50(
	byte *world)
{
	if (*(const long *)(world+SIMULATION_WORLD_STATE_OFFSET)==_simulation_world_state1)
	{
		simulation_world_unknown_0006b1f0(world, 0);
		simulation_world_unknown_0006b2a0(world);
	}
	else
	{
		simulation_world_unknown_0006b1f0(world, *(const long *)(*(const byte *const *)(world+SIMULATION_WORLD_UNKNOWN0_OFFSET)+
			SIMULATION_WORLD_UNKNOWN0_MACHINE_MASK_OFFSET));
	}
}

/* 0x00069260: the thing gives the one of the things that come that is of
this tick. In its third state it is the first of the ones that came from
another console and wait (simulation_world_take_update). In any other it is
made here: its number is the one of the thing; the time of the game and its
seed; the change of what unknown_004cf780 has, if there is one
(simulation_take_unknown_004cf780_change), and what happened to it
(unknown_00083610); whether the game in the network is being played; and if
it is, what the players do (unknown_00069110), and whether the state of the
game was saved since the last one, which a thing that is not in its fifth
state says once */
void simulation_world_build_update(
	byte *world,
	long *update)
{
	byte *bytes= (byte *)update;
	boolean in_progress;

	if (*(const long *)(world+SIMULATION_WORLD_STATE_OFFSET)==_simulation_world_state3)
	{
		simulation_world_take_update(world, update);

		return;
	}
	update[0]= *(const long *)(world+SIMULATION_WORLD_UNKNOWN28_OFFSET);
	*(unsigned long *)(bytes+SIMULATION_UPDATE_GAME_TIME_OFFSET)= game_time_globals->time;
	*(unsigned long *)(bytes+SIMULATION_UPDATE_RANDOM_SEED_OFFSET)= *global_random_seed_reference;
	simulation_take_unknown_004cf780_change((long *)(bytes+SIMULATION_UPDATE_CHANGE_OFFSET), bytes+SIMULATION_UPDATE_CHANGED_OFFSET);
	unknown_00083610(unknown_004cf780, (long *)(bytes+SIMULATION_UPDATE_EVENT_COUNT_OFFSET), bytes+SIMULATION_UPDATE_EVENTS_OFFSET);
	in_progress= simulation_in_progress();
	bytes[SIMULATION_UPDATE_IN_PROGRESS_OFFSET]= in_progress;
	if (in_progress)
	{
		long state;

		unknown_00069110(world, update);
		state= *(const long *)(world+SIMULATION_WORLD_STATE_OFFSET);
		if (state!=3 && state!=5 && world[SIMULATION_WORLD_SAVED_OFFSET])
		{
			bytes[SIMULATION_UPDATE_SAVED_OFFSET]= TRUE;
			world[SIMULATION_WORLD_SAVED_OFFSET]= FALSE;
		}
	}
}

/* 0x00069040: the thing is given what the players of this console do, as
four bits say which. In its third state, which follows another console, its
first view of the first kind or of the third sends it there
(simulation_view_send_player_actions). In any other, each of those players
that the thing has (simulation_world_find_player, with the six bytes of this
console) takes it (unknown_000847d0). The game has
simulation_world_unknown_0006acb0 written here, and does not look whether
there is such a view */
void __attribute__((stdcall)) simulation_world_set_local_player_actions(
	const void *actions,
	byte *world,
	unsigned long players)
{
	if (*(const long *)(world+SIMULATION_WORLD_STATE_OFFSET)==_simulation_world_state3)
	{
		simulation_view_send_player_actions(simulation_world_unknown_0006acb0(world), players, actions);
	}
	else
	{
		long index;

		for (index= 0; index<MAXIMUM_SIMULATION_CONSOLE_PLAYERS; index++)
		{
			if (players&(1u<<index))
			{
				byte *player= simulation_world_find_player(world, (const word *)(world+SIMULATION_WORLD_OWN_UNKNOWN14_OFFSET), index);

				if (player)
				{
					unknown_000847d0(player, (const byte *)actions+index*SIMULATION_PLAYER_ACTIONS_SIZE);
				}
			}
		}
	}
}

/* 0x000690d0: one of the other things of the thing is given what it has at
0x14, with the time of the game, if there is such a one; not in the third
state of the thing nor in the fifth */
void __attribute__((stdcall)) simulation_world_set_other_unknown14(
	byte *world,
	long index,
	const void *unknown14)
{
	long state= *(const long *)(world+SIMULATION_WORLD_STATE_OFFSET);

	if (state!=3 && state!=5)
	{
		byte *other= world+index*SIMULATION_WORLD_OTHER_SIZE+SIMULATION_WORLD_OTHERS_OFFSET;

		if (*(const long *)other!=NONE)
		{
			*(unsigned long *)(other+SIMULATION_WORLD_OTHER_UNKNOWN10_OFFSET)= game_time_globals->time;
			memcpy(other+SIMULATION_WORLD_OTHER_UNKNOWN14_OFFSET, unknown14, SIMULATION_WORLD_OTHER_UNKNOWN14_SIZE);
		}
	}
}
