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

/* 0x00056c60: as voice_channels_get_player, in the other states of the
network game */
unsigned long __attribute__((stdcall)) unknown_00056c60(byte *channels, long player_index);

/* 0x00056990: something made of who a player talks to; 0x000565c0: the two
numbers of 16 bits that unknown_00527104 gives for it */
long unknown_00056990(unsigned long targets);
void __attribute__((stdcall)) unknown_000565c0(byte *unknown, long unknown4, word *words);

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
		voice_queue_add(words, queues+VOICE_QUEUES_QUEUE_OFFSET+player_index*VOICE_QUEUE_SIZE, data,
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

/* 0x00055990: each frame, says who each player of this console talks to, if
it has a voice: the players of other consoles that the session has, less
those that it wants not to hear, and of them only those that hear it by who
hears whom; and less each one that wants not to hear it, wants no voice or
is not allowed one. The game has game_voice_get_players,
game_voice_get_talkers, game_voice_player_has_muted,
game_voice_player_voice_off and game_voice_player_not_allowed in the
function itself */
void voice_targets_update(
	byte *targets)
{
	if (targets[VOICE_TARGETS_ON_OFFSET])
	{
		unsigned long session_players= 0;
		unsigned long local_players;
		unsigned long remote_players;
		unsigned long talkers= 0;
		long player_index;

		if (voice_routing_active())
		{
			const byte *membership= game_voice_get_membership();

			if (membership)
			{
				session_players= *(const unsigned long *)(membership+GAME_VOICE_MEMBERSHIP_PLAYERS_OFFSET);
			}
		}
		local_players= game_voice_get_local_players();
		remote_players= ~local_players&session_players;
		if (voice_routing_active())
		{
			talkers= game_voice_talkers;
		}
		memset(targets+VOICE_TARGETS_PLAYERS_OFFSET, 0, VOICE_TARGETS_PLAYERS_SIZE);
		for (player_index= 0; player_index<MAXIMUM_VOICE_ROUTING_PLAYERS; player_index++)
		{
			unsigned long bit= 1u<<player_index;

			if ((local_players&bit) && (talkers&bit))
			{
				unsigned long mutes= 0;
				unsigned long player_targets;
				long other_index;

				if (voice_routing_active())
				{
					mutes= game_voice_player_mutes[player_index];
				}
				player_targets= ~mutes&remote_players;
				if (voice_routing_active())
				{
					if (unknown_004c9888==_game_voice_network1)
					{
						player_targets&= voice_channels_get_player(unknown_00527108, player_index);
					}
					else if (unknown_004c9888==_game_voice_network2 || unknown_004c9888==_game_voice_network3)
					{
						player_targets&= unknown_00056c60(unknown_00527108, player_index);
					}
				}
				for (other_index= 0; other_index<MAXIMUM_VOICE_ROUTING_PLAYERS; other_index++)
				{
					unsigned long other_bit= 1u<<other_index;

					if ((player_targets&other_bit) && game_voice_enabled)
					{
						if (voice_globals.initialized &&
							((bit&game_voice_player_mutes[other_index]) ||
							((game_voice_player_flags(other_index)>>_game_voice_player_voice_off_bit)&1)))
						{
							player_targets&= ~other_bit;
						}
						else if (voice_routing_active() &&
							((game_voice_player_flags(other_index)>>_game_voice_player_not_allowed_bit)&1))
						{
							player_targets&= ~other_bit;
						}
					}
				}
				*(unsigned long *)(targets+VOICE_TARGETS_PLAYERS_OFFSET+player_index*sizeof(long))= player_targets;
			}
		}
	}
}

/* 0x00056590: the two numbers of 16 bits that unknown_00527104 gives for who
a player talks to: 0 if it is not on or the player talks to nobody */
void voice_unknown_00527104_get_words(
	unsigned long targets,
	word *words,
	byte *unknown)
{
	words[1]= 0;
	words[0]= 0;
	if (unknown[0] && targets)
	{
		long unknown4= unknown_00056990(targets);

		if (unknown4)
		{
			unknown_000565c0(unknown, unknown4, words);
		}
	}
}

/* 0x00055b40: puts what a player said at the end of a queue that is in use,
with where it goes; the queue is emptied first if it is full. The game asks
which console this one is and does nothing with the answer */
void VOICE_ROUTING_CALLBACK voice_queue_add(
	const word *words,
	byte *queue,
	const void *data,
	unsigned long size,
	const boolean *unknown)
{
	if (queue[VOICE_QUEUE_ACTIVE_OFFSET])
	{
		long *count= (long *)(queue+VOICE_QUEUE_COUNT_OFFSET);
		word *peers= (word *)(queue+VOICE_QUEUE_PEERS_OFFSET);
		long *peer_counts= (long *)(queue+VOICE_QUEUE_PEER_COUNTS_OFFSET);
		byte *entry;
		long peer;

		game_voice_get_local_peer();
		if (*count+1>=MAXIMUM_VOICE_QUEUE_ENTRIES)
		{
			*count= 0;
			*peers= 0;
			memset(queue+VOICE_QUEUE_ENTRIES_OFFSET, 0, VOICE_QUEUE_ENTRIES_SIZE);
			memset(peer_counts, 0, VOICE_QUEUE_PEER_COUNTS_SIZE);
		}
		entry= queue+VOICE_QUEUE_ENTRIES_OFFSET+*count*VOICE_QUEUE_ENTRY_SIZE;
		*(word *)(entry+VOICE_QUEUE_ENTRY_PEERS_OFFSET)= words[0];
		for (peer= 0; peer<MAXIMUM_VOICE_ROUTING_PLAYERS; peer++)
		{
			if (words[0]&(1u<<peer))
			{
				peer_counts[peer]++;
			}
		}
		memcpy(entry+VOICE_QUEUE_ENTRY_WORDS_OFFSET, words, 2*sizeof(word));
		entry[VOICE_QUEUE_ENTRY_FLAG_OFFSET]= *unknown;
		memcpy(entry+VOICE_QUEUE_ENTRY_DATA_OFFSET, data, size);
		*peers|= words[0];
		*count= *count+1;
	}
}

/* 0x00056220: how much there is to send to a console of what the queues
have; 0 if nothing goes to it */
long voice_queues_size_for_peer(
	const byte *queues,
	long peer)
{
	long size= 0;

	if (queues[VOICE_QUEUES_ON_OFFSET] && voice_queues_go_to_peer(peer, queues))
	{
		long chosen_peer= voice_routing_active() ? unknown_004c9878 : 0;
		boolean is_chosen= chosen_peer==peer;
		long player_index;

		size= VOICE_PACKET_HEADER_SIZE;
		for (player_index= 0; player_index<MAXIMUM_VOICE_ROUTING_PLAYERS; player_index++)
		{
			const byte *queue= queues+VOICE_QUEUES_QUEUE_OFFSET+player_index*VOICE_QUEUE_SIZE;

			if (queue[VOICE_QUEUE_ACTIVE_OFFSET] &&
				((1u<<(peer&0x1F))&*(const word *)(queue+VOICE_QUEUE_PEERS_OFFSET)))
			{
				size+= (is_chosen*VOICE_PACKET_CHOSEN_PEER_ENTRY_EXTRA_SIZE+VOICE_PACKET_ENTRY_SIZE)*
					*(const long *)(queue+VOICE_QUEUE_PEER_COUNTS_OFFSET+peer*sizeof(long))+VOICE_PACKET_QUEUE_SIZE;
			}
		}
	}

	return size;
}
