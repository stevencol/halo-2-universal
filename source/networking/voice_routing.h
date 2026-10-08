/*
VOICE_ROUTING.H

What goes with the voice between the consoles: who each player talks to, the
queues of what the players said, and who hears whom. The name of the file is
a guess, and so are the names of its functions.
*/

#ifndef __VOICE_ROUTING_H
#define __VOICE_ROUTING_H

#include "cseries/cseries.h"
#include "math/real_math.h"

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
	/* and which player it is of */
	VOICE_QUEUE_PLAYER_OFFSET= 0xC,
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
	/* what goes to a console starts with a byte whose first bit says that
	the console is the one chosen for the voice, and how many queues follow.
	It is not written with room for less than this */
	VOICE_PACKET_FLAGS_OFFSET= 0,
	VOICE_PACKET_QUEUE_COUNT_OFFSET= 1,
	VOICE_PACKET_MINIMUM_SIZE= 15,
	/* each queue in it starts with its player and how many things */
	VOICE_PACKET_QUEUE_PLAYER_OFFSET= 0,
	VOICE_PACKET_QUEUE_ENTRY_COUNT_OFFSET= 1,
	/* each thing in it: its flag, which is 0 or 1; for the console chosen
	for the voice, a bit for each console that it goes to; and what the
	player said */
	VOICE_PACKET_ENTRY_FLAG_OFFSET= 0,
	VOICE_PACKET_ENTRY_PEERS_OFFSET= 1,
	MAXIMUM_VOICE_PACKET_ENTRY_FLAG= 1,
	/* a queue that is read can say that it has one thing more than a queue
	has room for */
	MAXIMUM_VOICE_PACKET_QUEUE_ENTRIES= 16,
	VOICE_PACKET_QUEUE_SIZE= 2,
	VOICE_PACKET_ENTRY_SIZE= 11,
	VOICE_PACKET_CHOSEN_PEER_ENTRY_EXTRA_SIZE= 2,

	/* who hears whom (0x00527108): whether it is on, and three numbers for
	each player */
	VOICE_CHANNELS_ON_OFFSET= 0,
	VOICE_CHANNELS_UNKNOWN4_OFFSET= 0x4,
	VOICE_CHANNELS_UNKNOWN44_OFFSET= 0x44,
	VOICE_CHANNELS_UNKNOWN84_OFFSET= 0x84,
	/* a bit for each player of another console that some player of this one
	hears now */
	VOICE_CHANNELS_HEARD_OFFSET= 0xC4,
	/* and a time of the game for each player, in ticks; 0 for none */
	VOICE_CHANNELS_TIMES_OFFSET= 0x194,
	/* another time for each player: when what it said started to come */
	VOICE_CHANNELS_STARTED_OFFSET= 0x148,
	/* the sound of the voice that is played when a player stops being
	heard */
	_voice_sound_unknown2= 2,

	/* where the sounds of the voice are: the globals of the game have a tag
	at 0x16c, which points at 0xc to what has the sounds, how many and
	where: eight bytes each, with the tag of the sound in the second half */
	GAME_GLOBALS_UNKNOWN16C_OFFSET= 0x16C,
	UNKNOWN_TAG_BLOCK_ADDRESS_OFFSET= 0xC,
	VOICE_SOUNDS_COUNT_OFFSET= 0x90,
	VOICE_SOUNDS_ADDRESS_OFFSET= 0x94,
	VOICE_SOUND_SIZE= 0x8,
	VOICE_SOUND_TAG_INDEX_OFFSET= 0x4,

	/* where a sound is played that only the player of a gamepad hears: one
	of these for each gamepad */
	VOICE_SOUND_GAMEPAD_OUTPUT= 0x14000140,

	/* in a player of the game: a number that is NONE or not, and an object;
	and in the object, a time of the game, in ticks */
	VOICE_PLAYER_DATUM_UNKNOWN2C_OFFSET= 0x2C,
	VOICE_PLAYER_DATUM_OBJECT_OFFSET= 0x30,
	VOICE_OBJECT_UNKNOWN2E0_OFFSET= 0x2E0
};

/* how long after that time voice_player_unknown_000589e0 still says so, in
seconds */
#define VOICE_PLAYER_UNKNOWN_000589E0_SECONDS 1.0f

/* how soon after it started a player's voice has to end for the sound to be
played, in seconds */
#define VOICE_CHANNELS_ENDED_SECONDS 0.3f

/* ---------- structures */

/* what a sound is played with, when it is not played for everybody */
struct voice_sound_request
{
	long output;
	long sound_index;
	real scale;
	long unknownC;
};

typedef char verify_voice_sound_request_size[
	sizeof(struct voice_sound_request) == 0x10 ? 1 : -1];

/* how a player of this console hears a player: which player it is, how it
hears it (0 if it does not) and something more that is not set here */
struct voice_channel_listener
{
	long player_index;
	long how;
	long unknown8;
};

typedef char verify_voice_channel_listener_size[
	sizeof(struct voice_channel_listener) == 0xC ? 1 : -1];

/* a function that takes its arguments off the stack */
#define VOICE_ROUTING_CALLBACK __attribute__((stdcall))

/* ---------- globals */

extern real unknown_004cf738;

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
unsigned long voice_targets_to_peers(unsigned long targets);
boolean voice_player_unknown_000589e0(long player);
boolean voice_channels_time_is_recent(const byte *channels, long player_index);
unsigned long VOICE_ROUTING_CALLBACK voice_channels_get_player_by_team(const byte *channels, long player_index);
void VOICE_ROUTING_CALLBACK voice_channels_update_ended(byte *channels);
boolean voice_players_can_hear(long player_index, long local_player_index);
void VOICE_ROUTING_CALLBACK voice_play_sound_by_number(long sound, long player_index, byte *unused);
void VOICE_ROUTING_CALLBACK voice_play_sound(long player_index, long sound_index);
void VOICE_ROUTING_CALLBACK voice_channels_update_by_team(byte *channels);
long voice_players_same_team(long player_index, long local_player_index);
void VOICE_ROUTING_CALLBACK voice_route_peers(const byte *unknown, unsigned long peers, word *words);
void VOICE_ROUTING_CALLBACK voice_route_assign(long *budget, unsigned long *peers, unsigned long allowed,
	unsigned long *direct, boolean through_chosen, long chosen_peer, boolean *more);
void VOICE_ROUTING_CALLBACK voice_queue_keep(byte *queue, unsigned long entries);
long VOICE_ROUTING_CALLBACK voice_queue_write(byte *queue, long peer, byte *out, byte *header, long size,
	boolean is_chosen);
long VOICE_ROUTING_CALLBACK voice_queue_read(byte *queue, const byte *in, long size, boolean for_chosen);
long VOICE_ROUTING_CALLBACK voice_queues_build_packet(byte *packet, byte *queues, long peer, long size);
void VOICE_ROUTING_CALLBACK voice_queues_read_packet(long size, byte *queues, const byte *packet);
void voice_unknown_00527104_get_words(unsigned long targets, word *words, byte *unknown);

#endif
