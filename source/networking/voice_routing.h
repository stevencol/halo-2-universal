/*
VOICE_ROUTING.H

What goes with the voice between the consoles: who each player talks to, the
queues of what the players said, and who hears whom. The name of the file is
a guess, and so are the names of its functions.
*/

#ifndef __VOICE_ROUTING_H
#define __VOICE_ROUTING_H

#include "cseries/cseries.h"

/* ---------- constants */

enum
{
	MAXIMUM_VOICE_ROUTING_PLAYERS= 16,

	/* who each player talks to (0x005259b8): whether it is on, and a number
	for each player */
	VOICE_TARGETS_ON_OFFSET= 0,
	VOICE_TARGETS_PLAYERS_OFFSET= 0x4,
	VOICE_TARGETS_PLAYERS_SIZE= 0x40,

	/* the queues of what the players said (0x00525a00): whether they are on,
	and a queue for each player. Of a queue: whether it is in use, how many
	things it has, a bit for each console that they go to, the things, and
	how many go to each console */
	VOICE_QUEUES_ON_OFFSET= 0,
	VOICE_QUEUES_QUEUE_OFFSET= 0x4,
	VOICE_QUEUE_SIZE= 0x170,
	VOICE_QUEUE_ACTIVE_OFFSET= 0,
	VOICE_QUEUE_COUNT_OFFSET= 0x4,
	VOICE_QUEUE_PEERS_OFFSET= 0x8,
	VOICE_QUEUE_ENTRIES_OFFSET= 0x10,
	VOICE_QUEUE_ENTRIES_SIZE= 0x120,
	VOICE_QUEUE_PEER_COUNTS_OFFSET= 0x130,
	VOICE_QUEUE_PEER_COUNTS_SIZE= 0x40,
	/* the only size of what a player said that a queue takes */
	VOICE_QUEUE_DATA_SIZE= 10,
	/* a queue has room for one thing less than this: when it is full it is
	emptied */
	MAXIMUM_VOICE_QUEUE_ENTRIES= 16,
	/* a thing of a queue: the two numbers of 16 bits that say where it goes,
	a flag, the first number again and what the player said */
	VOICE_QUEUE_ENTRY_SIZE= 0x12,
	VOICE_QUEUE_ENTRY_WORDS_OFFSET= 0,
	VOICE_QUEUE_ENTRY_FLAG_OFFSET= 0x4,
	VOICE_QUEUE_ENTRY_PEERS_OFFSET= 0x6,
	VOICE_QUEUE_ENTRY_DATA_OFFSET= 0x8,
	/* what goes to a console: this much first, this much for each queue,
	and this much for each thing, or a little more if the console is the
	one chosen for the voice */
	VOICE_PACKET_HEADER_SIZE= 2,
	VOICE_PACKET_QUEUE_SIZE= 2,
	VOICE_PACKET_ENTRY_SIZE= 11,
	VOICE_PACKET_CHOSEN_PEER_ENTRY_EXTRA_SIZE= 2,

	/* who hears whom (0x00527108): whether it is on, and three numbers for
	each player */
	VOICE_CHANNELS_ON_OFFSET= 0,
	VOICE_CHANNELS_UNKNOWN4_OFFSET= 0x4,
	VOICE_CHANNELS_UNKNOWN44_OFFSET= 0x44,
	VOICE_CHANNELS_UNKNOWN84_OFFSET= 0x84
};

/* a function that takes its arguments off the stack */
#define VOICE_ROUTING_CALLBACK __attribute__((stdcall))

/* ---------- prototypes/VOICE_ROUTING.C */

long voice_targets_get(const byte *targets, long player_index);
void voice_queues_reset(byte *queues);
void voice_queues_add(long player_index, byte *queues, word *words, void *data, unsigned long size,
	boolean *unknown);
boolean voice_queues_go_to_peer(long peer, const byte *queues);
void voice_channels_update(byte *channels);
unsigned long voice_channels_get_player(byte *channels, long player);
long voice_channels_get_unknown84(const byte *channels, long player_index);
void voice_targets_update(byte *targets);
void VOICE_ROUTING_CALLBACK voice_queue_add(const word *words, byte *queue, const void *data, unsigned long size,
	const boolean *unknown);
long voice_queues_size_for_peer(const byte *queues, long peer);
void voice_unknown_00527104_get_words(unsigned long targets, word *words, byte *unknown);

#endif
