/*
VOICE_ROUTING.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

What goes with the voice between the consoles. The name of the file is a
guess, and so are the names of its functions: the game has them after
voice.c, from 0x00055960 on.
*/

#include "voice_routing.h"

#include "networking/game_voice.h"

#include <string.h>

/* ---------- prototypes of functions not decompiled yet */

/* 0x00055b40: puts what a player said in its queue */
void __attribute__((stdcall)) unknown_00055b40(word *words, byte *queue, void *data, unsigned long size,
	boolean *unknown);

/* 0x00056f60, 0x00057080, 0x00057360, 0x000577d0: what is done each frame
with who hears whom, in one state of the network game; 0x00057960: in the
others */
void __attribute__((stdcall)) unknown_00056f60(byte *channels);
void __attribute__((stdcall)) unknown_00057080(byte *channels);
void __attribute__((stdcall)) unknown_00057360(byte *channels);
void __attribute__((stdcall)) unknown_000577d0(byte *channels);
void __attribute__((stdcall)) unknown_00057960(byte *channels);

/* 0x000589e0: something of a player */
boolean unknown_000589e0(long player);

/* ---------- code */

/* whether there is a voice for the game to do something with: as in
game_voice.c */
static inline __attribute__((always_inline)) boolean voice_routing_active(
	void)
{
	return game_voice_enabled && voice_globals.initialized;
}

/* 0x00055960: the number that says who a player of this console talks to; 0
if it is not on or the player is not of this console */
long voice_targets_get(
	const byte *targets,
	long player_index)
{
	if (targets[VOICE_TARGETS_ON_OFFSET])
	{
		if ((1u<<player_index)&game_voice_get_local_players())
		{
			return *(const long *)(targets+VOICE_TARGETS_PLAYERS_OFFSET+player_index*sizeof(long));
		}
	}

	return 0;
}

/* 0x000560e0: empties the queue of each player */
void voice_queues_reset(
	byte *queues)
{
	if (queues[VOICE_QUEUES_ON_OFFSET])
	{
		long player_index;

		for (player_index= 0; player_index<MAXIMUM_VOICE_ROUTING_PLAYERS; player_index++)
		{
			byte *queue= queues+VOICE_QUEUES_QUEUE_OFFSET+player_index*VOICE_QUEUE_SIZE;

			*(long *)(queue+VOICE_QUEUE_COUNT_OFFSET)= 0;
			*(word *)(queue+VOICE_QUEUE_PEERS_OFFSET)= 0;
			memset(queue+VOICE_QUEUE_ENTRIES_OFFSET, 0, VOICE_QUEUE_ENTRIES_SIZE);
			memset(queue+VOICE_QUEUE_PEER_COUNTS_OFFSET, 0, VOICE_QUEUE_PEER_COUNTS_SIZE);
		}
	}
}

/* 0x00056130: what a player said goes to its queue, if it has the only size
that the queues take */
void voice_queues_add(
	long player_index,
	byte *queues,
	word *words,
	void *data,
	unsigned long size,
	boolean *unknown)
{
	if (queues[VOICE_QUEUES_ON_OFFSET] && size==VOICE_QUEUE_DATA_SIZE)
	{
		unknown_00055b40(words, queues+VOICE_QUEUES_QUEUE_OFFSET+player_index*VOICE_QUEUE_SIZE, data,
			VOICE_QUEUE_DATA_SIZE, unknown);
	}
}

/* 0x00056160: whether some queue in use has something that goes to a
console */
boolean voice_queues_go_to_peer(
	long peer,
	const byte *queues)
{
	if (queues[VOICE_QUEUES_ON_OFFSET])
	{
		long player_index;

		for (player_index= 0; player_index<MAXIMUM_VOICE_ROUTING_PLAYERS; player_index++)
		{
			const byte *queue= queues+VOICE_QUEUES_QUEUE_OFFSET+player_index*VOICE_QUEUE_SIZE;

			if (queue[VOICE_QUEUE_ACTIVE_OFFSET] &&
				((1u<<(peer&0x1F))&*(const word *)(queue+VOICE_QUEUE_PEERS_OFFSET)))
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

/* 0x00056b70: each frame, who hears whom: four things in one state of the
network game, and one in two others */
void voice_channels_update(
	byte *channels)
{
	if (channels[VOICE_CHANNELS_ON_OFFSET] && voice_routing_active())
	{
		if (unknown_004c9888==_game_voice_network1)
		{
			unknown_00056f60(channels);
			unknown_00057080(channels);
			unknown_00057360(channels);
			unknown_000577d0(channels);
		}
		else if (unknown_004c9888==_game_voice_network2 || unknown_004c9888==_game_voice_network3)
		{
			unknown_00057960(channels);
		}
	}
}

/* 0x00056bd0: a bit for each player that a player of the game hears, most
likely: its third number if unknown_000589e0 says not, and its first two
together if it says so. 0 out of the first state of the network game, or if
the player is not one that the voice has. The game reads the numbers of the
one that it has at 0x00527108, whatever it is given */
unsigned long voice_channels_get_player(
	byte *channels,
	long player)
{
	if (channels[VOICE_CHANNELS_ON_OFFSET] && voice_routing_active() && unknown_004c9888==_game_voice_network1 &&
		game_voice_unknown_00054df0(player) && game_voice_get_player_datum(player))
	{
		unsigned long first= 0;
		unsigned long second= 0;

		if (!unknown_000589e0(player))
		{
			return (unsigned long)voice_channels_get_unknown84(unknown_00527108, player);
		}
		if (unknown_00527108[VOICE_CHANNELS_ON_OFFSET])
		{
			first= *(const unsigned long *)(unknown_00527108+VOICE_CHANNELS_UNKNOWN4_OFFSET+player*sizeof(long));
		}
		if (unknown_00527108[VOICE_CHANNELS_ON_OFFSET])
		{
			second= *(const unsigned long *)(unknown_00527108+VOICE_CHANNELS_UNKNOWN44_OFFSET+player*sizeof(long));
		}

		return second|first;
	}

	return 0;
}

/* 0x00056dd0: the third number of a player; 0 if it is not on */
long voice_channels_get_unknown84(
	const byte *channels,
	long player_index)
{
	if (channels[VOICE_CHANNELS_ON_OFFSET])
	{
		return *(const long *)(channels+VOICE_CHANNELS_UNKNOWN84_OFFSET+player_index*sizeof(long));
	}

	return 0;
}
