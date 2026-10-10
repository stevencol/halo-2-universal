/*
SIMULATION_WORLD.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

What unknown_004cf770 has going of a game in the network, and the views
that it has of it: one for each channel of the game that it talks by. The
name of the file is a guess, and so are the names of its functions.
*/

#include "simulation_world.h"

#include "cache/texture_cache.h"
#include "game/game.h"
#include "main/main_game.h"

#include <limits.h>

/* ---------- prototypes */

/* 0x00069790: how many things wait, for what a view has at 0xC in its second
state. 0x0006ab90: it takes something that came; whether it could.
0x00085a70: a view is given two numbers */
long unknown_00069790(const byte *unknown);
boolean __attribute__((stdcall)) unknown_0006ab90(byte *unknown, const long *update);
void __attribute__((stdcall)) unknown_00085a70(byte *view, long unknown, long unknown2);

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
	const byte *unknown= world+((unsigned long)player&0xFFFF)*SIMULATION_WORLD_PLAYER_SIZE+SIMULATION_WORLD_PLAYERS_OFFSET;

	if (*(const long *)unknown==NONE)
	{
		return FALSE;
	}

	return unknown[SIMULATION_WORLD_PLAYER_UNKNOWN25_OFFSET];
}

/* 0x00069610: the byte of what the thing has of each player of the game is
clear */
void simulation_world_players_clear_unknown25(
	byte *world)
{
	long player;

	for (player= 0; player<NUMBER_OF_SIMULATION_WORLD_PLAYERS; player++)
	{
		byte *unknown= world+player*SIMULATION_WORLD_PLAYER_SIZE+SIMULATION_WORLD_PLAYERS_OFFSET;

		if (*(const long *)unknown!=NONE && unknown[SIMULATION_WORLD_PLAYER_UNKNOWN25_OFFSET])
		{
			unknown[SIMULATION_WORLD_PLAYER_UNKNOWN25_OFFSET]= FALSE;
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

/* 0x0006aae0: what a view has at 0xC gives back the memory that the texture
cache lent it, and has none */
void simulation_world_unknown_0006aae0(
	byte *unknown)
{
	texture_cache_return_memory(*(void **)(unknown+SIMULATION_VIEW_UNKNOWNC_MEMORY_OFFSET));
	*(void **)(unknown+SIMULATION_VIEW_UNKNOWNC_MEMORY_OFFSET)= NULL;
	*(long *)(unknown+SIMULATION_VIEW_UNKNOWNC_UNKNOWN1200_OFFSET)= NONE;
}

/* 0x00069300: how many things wait for what a view has at 0xC to take, and
whether they were counted here. None if it is not going. In its second state
it is what unknown_00069790 says; in its third, from the first of the ones
that have come to the last, and those were counted; in the others, the most
that there can be. The game has no more states than five in its table */
long simulation_world_unknown_00069300(
	const byte *unknown,
	boolean *counted)
{
	*counted= FALSE;
	if (!unknown[SIMULATION_VIEW_UNKNOWNC_GOING_OFFSET])
	{
		return 0;
	}
	switch (*(const long *)(unknown+SIMULATION_VIEW_UNKNOWNC_STATE_OFFSET))
	{
	case _simulation_view_unknownc_state2:
		return unknown_00069790(unknown);
	case _simulation_view_unknownc_state3:
		*counted= TRUE;

		return (long)(*(const unsigned long *)(unknown+SIMULATION_VIEW_UNKNOWNC_LAST_UPDATE_OFFSET)-
			*(const unsigned long *)(unknown+SIMULATION_VIEW_UNKNOWNC_FIRST_UPDATE_OFFSET)+1);
	}

	return LONG_MAX;
}

/* 0x00069350: what a view has at 0xC is told whether the game is to be
brought up to what has come; and if so, and while it is going, the game goes
on a tick for as long as things wait: the game has
simulation_world_unknown_00069300 written here, so in the states that are
not the second or the third it goes on until the thing is not going */
void simulation_world_unknown_00069350(
	boolean catch_up,
	byte *unknown)
{
	unknown[SIMULATION_VIEW_UNKNOWNC_CATCH_UP_OFFSET]= catch_up;
	if (!catch_up)
	{
		return;
	}
	while (unknown[SIMULATION_VIEW_UNKNOWNC_GOING_OFFSET])
	{
		long count;

		switch (*(const long *)(unknown+SIMULATION_VIEW_UNKNOWNC_STATE_OFFSET))
		{
		case _simulation_view_unknownc_state2:
			count= unknown_00069790(unknown);
			break;
		case _simulation_view_unknownc_state3:
			count= (long)(*(const unsigned long *)(unknown+SIMULATION_VIEW_UNKNOWNC_LAST_UPDATE_OFFSET)-
				*(const unsigned long *)(unknown+SIMULATION_VIEW_UNKNOWNC_FIRST_UPDATE_OFFSET)+1);
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

/* 0x000698e0: something came for what a view has at 0xC, which starts with
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
	byte *unknown,
	const long *update)
{
	long next= (long)(*(const unsigned long *)(unknown+SIMULATION_VIEW_UNKNOWNC_LAST_UPDATE_OFFSET)+1);

	if (!(*(const long *)(unknown+SIMULATION_VIEW_UNKNOWNC_STATE_OFFSET)==_simulation_view_unknownc_state3 &&
		*(const long *)(unknown+SIMULATION_VIEW_UNKNOWNC_UNKNOWN1200_OFFSET)!=NONE))
	{
		if (update[0]<next)
		{
			return FALSE;
		}
		if (update[0]==next &&
			(*(const long *)(unknown+SIMULATION_VIEW_UNKNOWNC_UNKNOWN18_OFFSET)==_simulation_view_unknownc_unknown18_4 ||
			unknown[SIMULATION_VIEW_UNKNOWNC_CATCH_UP_OFFSET]))
		{
			if (!unknown_0006ab90(unknown, update))
			{
				unknown_004cf770.unknown1[0]= TRUE;

				return FALSE;
			}
			if (unknown[SIMULATION_VIEW_UNKNOWNC_CATCH_UP_OFFSET])
			{
				while (main_game_globals && main_game_globals->active &&
					!(unknown_004cf770.unknown0 && unknown_004cf770.unknown1[1]) &&
					!unknown[SIMULATION_VIEW_UNKNOWNC_UNKNOWN2C_OFFSET])
				{
					boolean counted;

					if (simulation_world_unknown_00069300(unknown, &counted)<=0)
					{
						break;
					}
					game_tick();
				}
			}

			return TRUE;
		}
	}
	unknown[SIMULATION_VIEW_UNKNOWNC_UNKNOWN2C_OFFSET]= TRUE;

	return FALSE;
}

/* 0x0006a2a0: a view is given two numbers (unknown_00085a70), as a number of
it says: with 2, nothing; with 1, and another number of it 1 too, 2 and the
count of something else, which goes on by one; with 1 and not that, nothing;
and with any other, 1 and NONE */
void simulation_world_unknown_0006a2a0(
	byte *view,
	byte *unknown)
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
			long count= *(const long *)(unknown+SIMULATION_WORLD_UNKNOWN_0006A2A0_COUNT_OFFSET);

			*(long *)(unknown+SIMULATION_WORLD_UNKNOWN_0006A2A0_COUNT_OFFSET)= (long)((unsigned long)count+1);
			unknown_00085a70(view, 2, count);
		}

		return;
	}
	unknown_00085a70(view, 1, NONE);
}
