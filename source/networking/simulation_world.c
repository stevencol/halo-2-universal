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
#include "networking/simulation_view.h"

#include <string.h>

#include <limits.h>

/* ---------- prototypes */

/* 0x000860b0: a view of the second kind is given something. 0x00199740:
the state of the game that came, of a size, is put where the state of the
game is, and its size is given; whether it was good */
void unknown_000860b0(byte *view, long unknown);
boolean __attribute__((stdcall)) unknown_00199740(const void *data, long size, void *base, long *size_out);


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
something (unknown_000860b0) */
void __attribute__((stdcall)) simulation_world_unknown_00069880(
	byte *world,
	long unknown)
{
	struct simulation_view_iterator iterator= {_simulation_view_kind2_bit, 0};
	byte *view;

	while (simulation_world_next_view(&iterator, world, &view))
	{
		unknown_000860b0(view, unknown);
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
