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
