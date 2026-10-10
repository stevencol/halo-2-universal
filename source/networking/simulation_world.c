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
#include "main/main_game.h"
#include "networking/simulation_view.h"

#include <limits.h>

/* ---------- prototypes */

/* 0x0006ab90: the thing takes something that came; whether it could.
0x000860b0: a view of the second kind is given something */
boolean __attribute__((stdcall)) unknown_0006ab90(byte *world, const long *update);
void unknown_000860b0(byte *view, long unknown);

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

/* 0x0006aae0: the thing gives back the memory that the texture
cache lent it, and has none */
void simulation_world_unknown_0006aae0(
	byte *world)
{
	texture_cache_return_memory(*(void **)(world+SIMULATION_WORLD_MEMORY_OFFSET));
	*(void **)(world+SIMULATION_WORLD_MEMORY_OFFSET)= NULL;
	*(long *)(world+SIMULATION_WORLD_UNKNOWN1200_OFFSET)= NONE;
}

/* 0x00069300: how many things wait for the thing to take, and
whether they were counted here. None if it is not going. In its second state
it is what unknown_00069790 says; in its third, from the first of the ones
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
(unknown_0006ab90) if a number of the thing is 4 or the game is to be brought
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

	if (!(*(const long *)(world+SIMULATION_WORLD_STATE_OFFSET)==_simulation_world_state3 &&
		*(const long *)(world+SIMULATION_WORLD_UNKNOWN1200_OFFSET)!=NONE))
	{
		if (update[0]<next)
		{
			return FALSE;
		}
		if (update[0]==next &&
			(*(const long *)(world+SIMULATION_WORLD_UNKNOWN18_OFFSET)==_simulation_world_unknown18_4 ||
			world[SIMULATION_WORLD_CATCH_UP_OFFSET]))
		{
			if (!unknown_0006ab90(world, update))
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

/* 0x0006a2a0: a view is given two numbers (unknown_00085a70), as a number of
it says: with 2, nothing; with 1, and another number of it 1 too, 2 and the
count of something else, which goes on by one; with 1 and not that, nothing;
and with any other, 1 and NONE */
void simulation_world_unknown_0006a2a0(
	byte *view,
	byte *other)
{
	long state= *(const long *)(view+SIMULATION_VIEW_UNKNOWN2C_OFFSET);

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
			unknown_00085a70(view, 2, count);
		}

		return;
	}
	unknown_00085a70(view, 1, NONE);
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
