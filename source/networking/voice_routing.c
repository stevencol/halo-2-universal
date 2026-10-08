/*
VOICE_ROUTING.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

What goes with the voice between the consoles. The name of the file is a
guess, and so are the names of its functions: the game has them after
voice.c, from 0x00055960 on.
*/

#include "voice_routing.h"

#include "game/game.h"
#include "networking/game_voice.h"
#include "objects/objects.h"

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

/* 0x00056c60: as voice_channels_get_player, in the other states of the
network game */
unsigned long __attribute__((stdcall)) unknown_00056c60(byte *channels, long player_index);

/* 0x00056df0: something that who hears whom is told of a player whose
voice comes from another console, with the flag of what it said */
void __attribute__((stdcall)) unknown_00056df0(byte *channels, long player_index, long flag);


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
likely: its third number if voice_player_unknown_000589e0 says not, and its first two
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

		if (!voice_player_unknown_000589e0(player))
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
		long unknown4= voice_targets_to_peers(targets);

		if (unknown4)
		{
			voice_route_peers(unknown, unknown4, words);
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

/* 0x00056000: leaves in a queue only those of its things that have their
bit in a number, in the order that they had */
void VOICE_ROUTING_CALLBACK voice_queue_keep(
	byte *queue,
	unsigned long entries)
{
	long *count= (long *)(queue+VOICE_QUEUE_COUNT_OFFSET);
	long kept= 0;
	long index;

	for (index= 0; index<*count; index++)
	{
		if ((entries&0xFFFF)&(1u<<(index&0x1F)))
		{
			if (index!=kept)
			{
				memcpy(queue+VOICE_QUEUE_ENTRIES_OFFSET+kept*VOICE_QUEUE_ENTRY_SIZE,
					queue+VOICE_QUEUE_ENTRIES_OFFSET+index*VOICE_QUEUE_ENTRY_SIZE, VOICE_QUEUE_ENTRY_SIZE);
			}
			kept++;
		}
	}
	*count= kept;
}

/* 0x00056380: writes what there is to send to a console of what the queues
have, in room of a size; how much was written, and 0 if nothing: with
nothing for the console, with too little room, or if no queue could write.
The first bit of the first byte says whether the console is the one chosen
for the voice: the rest of the byte is left as it was */
long VOICE_ROUTING_CALLBACK voice_queues_build_packet(
	byte *packet,
	byte *queues,
	long peer,
	long size)
{
	if (queues[VOICE_QUEUES_ON_OFFSET] && voice_queues_go_to_peer(peer, queues) && size>=VOICE_PACKET_MINIMUM_SIZE)
	{
		long queue_count= 0;
		boolean is_chosen= (voice_routing_active() ? unknown_004c9878 : 0)==peer;
		long remaining= size-VOICE_PACKET_HEADER_SIZE;
		byte *out= packet+VOICE_PACKET_HEADER_SIZE;
		long player_index;

		if (is_chosen)
		{
			packet[VOICE_PACKET_FLAGS_OFFSET]|= 1;
		}
		else
		{
			packet[VOICE_PACKET_FLAGS_OFFSET]&= 0xFE;
		}
		for (player_index= 0; player_index<MAXIMUM_VOICE_ROUTING_PLAYERS; player_index++)
		{
			const byte *queue= queues+VOICE_QUEUES_QUEUE_OFFSET+player_index*VOICE_QUEUE_SIZE;

			if (queue[VOICE_QUEUE_ACTIVE_OFFSET] &&
				((1u<<(peer&0x1F))&*(const word *)(queue+VOICE_QUEUE_PEERS_OFFSET)) && remaining>0)
			{
				long written= voice_queue_write((byte *)queue, peer, out+VOICE_PACKET_QUEUE_SIZE, out, remaining, is_chosen);

				if (written<=0)
				{
					break;
				}
				queue_count++;
				out+= written;
				remaining-= written;
			}
		}
		if (queue_count>0)
		{
			packet[VOICE_PACKET_QUEUE_COUNT_OFFSET]= (byte)queue_count;

			return size-remaining;
		}
		packet[VOICE_PACKET_QUEUE_COUNT_OFFSET]= 0;
	}

	return 0;
}

/* 0x000564c0: takes what another console sent of what its players said:
each queue goes to that of its player. One that says that it is for the
console chosen for the voice is only taken by that console. It is left as
soon as something is not right: a count of queues or a player out of range,
or a queue that is not taken. A queue with a count of things out of range is
passed over, without going on to the next */
void VOICE_ROUTING_CALLBACK voice_queues_read_packet(
	long size,
	byte *queues,
	const byte *packet)
{
	if (queues[VOICE_QUEUES_ON_OFFSET])
	{
		boolean for_chosen= packet[VOICE_PACKET_FLAGS_OFFSET]&1;
		long remaining= size-VOICE_PACKET_HEADER_SIZE;
		long queue_count;
		const byte *in;
		long index;

		if (for_chosen && !game_voice_unknown_000549d0())
		{
			return;
		}
		queue_count= packet[VOICE_PACKET_QUEUE_COUNT_OFFSET];
		if (queue_count<1 || queue_count>MAXIMUM_VOICE_QUEUE_ENTRIES-1)
		{
			return;
		}
		in= packet+VOICE_PACKET_HEADER_SIZE;
		for (index= 0; index<packet[VOICE_PACKET_QUEUE_COUNT_OFFSET]; index++)
		{
			long player_index= in[VOICE_PACKET_QUEUE_PLAYER_OFFSET];
			long entry_count= in[VOICE_PACKET_QUEUE_ENTRY_COUNT_OFFSET];

			if (player_index>MAXIMUM_VOICE_ROUTING_PLAYERS-1)
			{
				return;
			}
			if (entry_count>=1 && entry_count<=MAXIMUM_VOICE_QUEUE_ENTRIES-1)
			{
				long taken= voice_queue_read(queues+VOICE_QUEUES_QUEUE_OFFSET+player_index*VOICE_QUEUE_SIZE, in,
					remaining, for_chosen);

				if (taken<=0)
				{
					return;
				}
				remaining-= taken;
				in+= taken;
			}
		}
	}
}

/* 0x00055c40: writes the things that a queue has for a console, as many as
there is room for, after two bytes with the player of the queue and how
many they are; how much was written. Each thing is no longer for that
console: one that is for no console is taken out of the queue. For the
console chosen for the voice each thing has the consoles that it goes to: if
it has none, the console itself. 0 with a queue that is not in use or with
no room for one thing, and -1 if the queue has nothing for the console */
long VOICE_ROUTING_CALLBACK voice_queue_write(
	byte *queue,
	long peer,
	byte *out,
	byte *header,
	long size,
	boolean is_chosen)
{
	long *count= (long *)(queue+VOICE_QUEUE_COUNT_OFFSET);
	word *peers= (word *)(queue+VOICE_QUEUE_PEERS_OFFSET);
	unsigned long bit= 1u<<(peer&0x1F);
	long written= 0;
	unsigned long kept;
	long entry_size;
	long index;

	if (!queue[VOICE_QUEUE_ACTIVE_OFFSET])
	{
		return 0;
	}
	if (!(bit&*peers) || peer<0 || peer>MAXIMUM_VOICE_ROUTING_PLAYERS-1)
	{
		return NONE;
	}
	kept= (1u<<(*count&0x1F))-1;
	size-= VOICE_PACKET_QUEUE_SIZE;
	entry_size= (is_chosen ? VOICE_PACKET_CHOSEN_PEER_ENTRY_EXTRA_SIZE : 0)+VOICE_PACKET_ENTRY_SIZE;
	if (size<entry_size)
	{
		return 0;
	}
	for (index= 0; index<*count; index++)
	{
		byte *entry= queue+VOICE_QUEUE_ENTRIES_OFFSET+index*VOICE_QUEUE_ENTRY_SIZE;
		word *entry_peers= (word *)(entry+VOICE_QUEUE_ENTRY_PEERS_OFFSET);

		if (bit&*entry_peers)
		{
			if (size<entry_size)
			{
				break;
			}
			out[VOICE_PACKET_ENTRY_FLAG_OFFSET]= entry[VOICE_QUEUE_ENTRY_FLAG_OFFSET];
			if (is_chosen)
			{
				word targets= *(const word *)(entry+VOICE_QUEUE_ENTRY_WORDS_OFFSET+sizeof(word));

				if (!targets)
				{
					targets= (word)(1u<<peer);
				}
				memcpy(out+VOICE_PACKET_ENTRY_PEERS_OFFSET, &targets, sizeof(targets));
				out+= VOICE_PACKET_ENTRY_PEERS_OFFSET+sizeof(targets);
			}
			else
			{
				out+= VOICE_PACKET_ENTRY_PEERS_OFFSET;
			}
			memcpy(out, entry+VOICE_QUEUE_ENTRY_DATA_OFFSET, VOICE_QUEUE_DATA_SIZE);
			out+= VOICE_QUEUE_DATA_SIZE;
			*entry_peers&= (word)~(1u<<peer);
			(*(long *)(queue+VOICE_QUEUE_PEER_COUNTS_OFFSET+peer*sizeof(long)))--;
			if (!*entry_peers)
			{
				kept&= ~(1u<<(index&0x1F));
			}
			written++;
			size-= entry_size;
		}
	}
	header[VOICE_PACKET_QUEUE_ENTRY_COUNT_OFFSET]= (byte)written;
	header[VOICE_PACKET_QUEUE_PLAYER_OFFSET]= queue[VOICE_QUEUE_PLAYER_OFFSET];
	if ((kept&0xFFFF)!=(1u<<(*count&0x1F))-1)
	{
		voice_queue_keep(queue, kept);
	}
	*peers= 0;
	for (index= 0; index<*count; index++)
	{
		*peers|= *(const word *)(queue+VOICE_QUEUE_ENTRIES_OFFSET+index*VOICE_QUEUE_ENTRY_SIZE+
			VOICE_QUEUE_ENTRY_PEERS_OFFSET);
	}

	return written*entry_size+VOICE_PACKET_QUEUE_SIZE;
}

/* 0x00055e10: takes the things of a queue that another console sent, which
start after the two bytes of the queue; how much was taken, and -1 if
something is not right. What a player said goes to the library, if the
player has a voice, and who hears whom is told. A console chosen for the
voice gets each thing with the consoles that it goes to: its own bit is for
this console, and with the others the thing goes to the queue of the player,
to be sent on. The game has game_voice_has_remote_talker in the function
itself, where the thing is not for the chosen console */
long VOICE_ROUTING_CALLBACK voice_queue_read(
	byte *queue,
	const byte *in,
	long size,
	boolean for_chosen)
{
	const byte *header= in;
	long count= header[VOICE_PACKET_QUEUE_ENTRY_COUNT_OFFSET];
	long entry_size= (for_chosen ? VOICE_PACKET_CHOSEN_PEER_ENTRY_EXTRA_SIZE : 0)+VOICE_PACKET_ENTRY_SIZE;
	long taken= VOICE_PACKET_QUEUE_SIZE;
	long index= 0;

	if (count<1 || count>MAXIMUM_VOICE_PACKET_QUEUE_ENTRIES)
	{
		return NONE;
	}
	size-= VOICE_PACKET_QUEUE_SIZE;
	in+= VOICE_PACKET_QUEUE_SIZE;
	do
	{
		if (size<entry_size)
		{
			return NONE;
		}
		if (for_chosen)
		{
			long local_peer= game_voice_get_local_peer();
			word peers;

			if (local_peer==NONE || in[VOICE_PACKET_ENTRY_FLAG_OFFSET]>MAXIMUM_VOICE_PACKET_ENTRY_FLAG)
			{
				return NONE;
			}
			memcpy(&peers, in+VOICE_PACKET_ENTRY_PEERS_OFFSET, sizeof(peers));
			if (!peers)
			{
				return NONE;
			}
			if (peers&(1u<<(local_peer&0x1F)))
			{
				if (game_voice_has_remote_talker(*(const long *)(queue+VOICE_QUEUE_PLAYER_OFFSET)))
				{
					unknown_00056df0(unknown_00527108, *(const long *)(queue+VOICE_QUEUE_PLAYER_OFFSET),
						in[VOICE_PACKET_ENTRY_FLAG_OFFSET]);
					game_voice_remote_talker_data(*(const long *)(queue+VOICE_QUEUE_PLAYER_OFFSET),
						in+VOICE_PACKET_ENTRY_PEERS_OFFSET+sizeof(peers), VOICE_QUEUE_DATA_SIZE);
				}
				peers&= (word)~(1u<<(local_peer&0x1F));
			}
			if (peers)
			{
				word words[2];

				words[0]= 0;
				words[1]= 0;
				voice_route_peers(unknown_00527104, peers, words);
				voice_queue_add(words, queue, in+VOICE_PACKET_ENTRY_PEERS_OFFSET+sizeof(peers), VOICE_QUEUE_DATA_SIZE,
					in+VOICE_PACKET_ENTRY_FLAG_OFFSET);
			}
		}
		else
		{
			if (in[VOICE_PACKET_ENTRY_FLAG_OFFSET]>MAXIMUM_VOICE_PACKET_ENTRY_FLAG)
			{
				return NONE;
			}
			if (voice_routing_active() &&
				voice_has_remote_talker(&voice_globals, *(const long *)(queue+VOICE_QUEUE_PLAYER_OFFSET)))
			{
				unknown_00056df0(unknown_00527108, *(const long *)(queue+VOICE_QUEUE_PLAYER_OFFSET),
					in[VOICE_PACKET_ENTRY_FLAG_OFFSET]);
				game_voice_remote_talker_data(*(const long *)(queue+VOICE_QUEUE_PLAYER_OFFSET),
					in+VOICE_PACKET_ENTRY_PEERS_OFFSET, VOICE_QUEUE_DATA_SIZE);
			}
		}
		size-= entry_size;
		taken+= entry_size;
		in+= entry_size;
		index++;
	} while (index<header[VOICE_PACKET_QUEUE_ENTRY_COUNT_OFFSET]);

	return taken;
}

/* 0x00056990: the consoles of the players that a player talks to: a bit for
each, but for this one. 0 with no session. The game has
game_voice_get_player_array and game_voice_get_players in the function
itself */
unsigned long voice_targets_to_peers(
	unsigned long targets)
{
	const byte *players= NULL;
	unsigned long session_players= 0;
	unsigned long peers= 0;
	long local_peer;

	if (voice_routing_active())
	{
		const byte *membership= game_voice_get_membership();

		if (membership)
		{
			players= membership+GAME_VOICE_MEMBERSHIP_PLAYER_ARRAY_OFFSET;
		}
	}
	if (voice_routing_active())
	{
		const byte *membership= game_voice_get_membership();

		if (membership)
		{
			session_players= *(const unsigned long *)(membership+GAME_VOICE_MEMBERSHIP_PLAYERS_OFFSET);
		}
	}
	local_peer= game_voice_get_local_peer();
	if (players && session_players && local_peer!=NONE)
	{
		long player_index;

		for (player_index= 0; player_index<MAXIMUM_VOICE_ROUTING_PLAYERS; player_index++)
		{
			unsigned long bit= 1u<<player_index;

			if ((targets&bit) && (session_players&bit))
			{
				long peer= *(const long *)(players+player_index*GAME_VOICE_MEMBERSHIP_PLAYER_SIZE+
					GAME_VOICE_MEMBERSHIP_PLAYER_PEER_OFFSET);

				if (peer!=local_peer && peer!=NONE)
				{
					peers|= 1u<<(peer&0x1F);
				}
			}
		}
	}

	return peers;
}

/* 0x000565c0: says where something that a player said goes, of the consoles
that it is for: the first number, a bit for each console that this one sends
it to; the second, a bit for each console that the one chosen for the voice
has to send it on to. Both 0 if unknown_00527104 is not on, there are no
consoles or it can go to none.

It goes straight to the consoles that this one is joined to, as many as
game_voice_unknown_00054cc0 says, first to those that the two sessions share
when the session comes from the second place. What is left over, and the
consoles that this one is not joined to, goes by the chosen one, if there is
one that is not this console and this one is joined to it. With
unknown_004c987c at 1 or 2 everything goes by the chosen one */
void VOICE_ROUTING_CALLBACK voice_route_peers(
	const byte *unknown,
	unsigned long peers,
	word *words)
{
	long budget;
	boolean through_chosen;
	long chosen_peer;
	long mode;
	unsigned long joined;
	unsigned long chosen_bit;
	unsigned long direct= 0;
	unsigned long relayed= 0;
	boolean more= FALSE;

	words[1]= 0;
	words[0]= 0;
	if (!unknown[0] || !peers)
	{
		return;
	}
	budget= game_voice_unknown_00054cc0();
	if (budget<=0)
	{
		return;
	}
	through_chosen= game_voice_unknown_00054c70();
	chosen_peer= voice_routing_active() ? unknown_004c9878 : 0;
	mode= game_voice_enabled ? unknown_004c987c : 0;
	joined= (unsigned long)game_voice_local_peer_unknownA8();
	chosen_bit= 1u<<(chosen_peer&0x1F);
	if ((joined&chosen_bit) && through_chosen && (mode==1 || mode==2))
	{
		more= TRUE;
	}
	else
	{
		if (!(joined&chosen_bit))
		{
			through_chosen= FALSE;
		}
		if (!through_chosen && mode==2)
		{
			budget= 1;
		}
		long source= game_voice_get_session_source();

		relayed= ~joined&peers;
		peers&= joined;
		if (relayed&0xFFFF)
		{
			more= TRUE;
		}
		if (source==_game_voice_session_source2)
		{
			voice_route_assign(&budget, &peers, game_voice_unknown_00053c70(), &direct, through_chosen, chosen_peer,
				&more);
		}
		if (!(budget==1 && through_chosen && more))
		{
			voice_route_assign(&budget, &peers, 0xFFFFFFFF, &direct, through_chosen, chosen_peer, &more);
		}
	}
	if (!through_chosen)
	{
		relayed= 0;
	}
	else if (more)
	{
		direct|= 1u<<(chosen_peer&0x1F);
		relayed|= peers;
	}
	else if (peers&chosen_bit)
	{
		direct|= chosen_bit;
		relayed|= chosen_bit;
	}
	words[0]= (word)direct;
	words[1]= (word)relayed;
}

/* 0x00056790: of the consoles that are left and that are allowed, gives a
bit to as many as the budget says, from the first on, and takes them from
those that are left. The chosen console is passed over when things can go
by it; and then the last of the budget is not given if more than that one
console is left, or more was left before: it says so and stops, for the
last to be the chosen one */
void VOICE_ROUTING_CALLBACK voice_route_assign(
	long *budget,
	unsigned long *peers,
	unsigned long allowed,
	unsigned long *direct,
	boolean through_chosen,
	long chosen_peer,
	boolean *more)
{
	long peer;

	for (peer= 0; peer<MAXIMUM_VOICE_ROUTING_PLAYERS; peer++)
	{
		unsigned long bit= 1u<<peer;

		if ((*peers&bit) && (allowed&bit) && *budget>0)
		{
			if (through_chosen && chosen_peer==peer)
			{
				continue;
			}
			if (*budget==1 && through_chosen)
			{
				*more= *more || bit!=*peers;
				if (*more)
				{
					return;
				}
			}
			*(word *)direct|= (word)bit;
			*peers&= ~bit;
			(*budget)--;
		}
	}
}

/* 0x000589e0: of a player that the voice has: whether a number of it is not
NONE, or it has no object, or it is no more than a second since a time of
its object. FALSE for a player that the voice does not have */
boolean voice_player_unknown_000589e0(
	long player)
{
	if (game_voice_unknown_00054df0(player))
	{
		const byte *datum= game_voice_get_player_datum(player);

		if (datum)
		{
			long object_index;
			const struct object_header_datum *header;
			long ticks;

			if (*(const long *)(datum+VOICE_PLAYER_DATUM_UNKNOWN2C_OFFSET)!=NONE)
			{
				return TRUE;
			}
			object_index= *(const long *)(datum+VOICE_PLAYER_DATUM_OBJECT_OFFSET);
			if (object_index==NONE)
			{
				return TRUE;
			}
			header= (const struct object_header_datum *)object_header_data->data+(object_index&0xFFFF);
			ticks= (long)(game_time_globals->time-
				*(const unsigned long *)((const byte *)header->datum+VOICE_OBJECT_UNKNOWN2E0_OFFSET));
			if (VOICE_PLAYER_UNKNOWN_000589E0_SECONDS>=(real)ticks*game_time_globals->seconds_per_tick)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}
