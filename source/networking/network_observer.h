/*
NETWORK_OBSERVER.H

What a session of the network game is given when it is set up: it has
something of each session, and a channel for each console that this one
talks with. The name of the file is a guess, and so are the names of its
functions.
*/

#ifndef __NETWORK_OBSERVER_H
#define __NETWORK_OBSERVER_H

#include "cseries/cseries.h"
#include "math/real_math.h"

/* ---------- constants */

enum
{
	/* three things that it is given when it is set up */
	NETWORK_OBSERVER_UNKNOWN4_OFFSET= 0x4,
	NETWORK_OBSERVER_UNKNOWNC_OFFSET= 0xC,
	NETWORK_OBSERVER_UNKNOWN10_OFFSET= 0x10,

	/* what it has of each session: the session, two numbers (the first is
	NONE for a session that is known by nothing), what the session is known
	by in the network and its key */
	NETWORK_OBSERVER_SESSIONS_OFFSET= 0x14,
	NETWORK_OBSERVER_SESSION_SIZE= 0x24,
	MAXIMUM_NETWORK_OBSERVER_SESSIONS= 4,
	NETWORK_OBSERVER_SESSION_SESSION_OFFSET= 0,
	NETWORK_OBSERVER_SESSION_UNKNOWN4_OFFSET= 0x4,
	NETWORK_OBSERVER_SESSION_UNKNOWN8_OFFSET= 0x8,
	NETWORK_OBSERVER_SESSION_IDENTIFIER_OFFSET= 0xC,
	NETWORK_OBSERVER_SESSION_IDENTIFIER_SIZE= 0x8,
	NETWORK_OBSERVER_SESSION_KEY_OFFSET= 0x14,
	NETWORK_OBSERVER_SESSION_KEY_SIZE= 0x10,

	/* its channels: of each, a state that is zero for one that is not in
	use, a bit for each session that uses it, which of the game's channels
	it is (NONE for none), and six bytes that it can be looked for by */
	NETWORK_OBSERVER_CHANNELS_OFFSET= 0xA8,
	NETWORK_OBSERVER_CHANNEL_SIZE= 0x528,
	MAXIMUM_NETWORK_OBSERVER_CHANNELS= 15,
	NETWORK_OBSERVER_CHANNEL_STATE_OFFSET= 0,
	NETWORK_OBSERVER_CHANNEL_SESSIONS_OFFSET= 0x9,
	NETWORK_OBSERVER_CHANNEL_CHANNEL_OFFSET= 0xC,
	NETWORK_OBSERVER_CHANNEL_UNKNOWN1E_OFFSET= 0x1E,
	NETWORK_OBSERVER_CHANNEL_UNKNOWN1E_SIZE= 6,

	/* numbers that it keeps after the channels: two that are NONE when
	they say nothing, and two counts */
	NETWORK_OBSERVER_UNKNOWN4E08_OFFSET= 0x4E08,
	NETWORK_OBSERVER_UNKNOWN4E0C_OFFSET= 0x4E0C,
	NETWORK_OBSERVER_UNKNOWN4E18_OFFSET= 0x4E18,
	NETWORK_OBSERVER_UNKNOWN4E1C_OFFSET= 0x4E1C,

	/* the channels of the game (0x004d87d4): the size of one, and its
	state, which is more than 2 for one that can be told something */
	NETWORK_CHANNEL_SIZE= 0xF8,
	NETWORK_CHANNEL_STATE_OFFSET= 0x54,
	_network_channel_state2= 2,
	/* more of a channel: its address of the game, and a bit for each kind
	of thing that it has to send, in 64 bits; and how much its channel of
	the game can have waiting for it to take more: with none of the kind to
	send yet, and with it */
	NETWORK_OBSERVER_CHANNEL_ADDRESS_OFFSET= 0x5C,
	NETWORK_OBSERVER_CHANNEL_PENDING_OFFSET= 0x520,
	NETWORK_OBSERVER_CHANNEL_WAITING_LIMIT= 0x4000,
	NETWORK_OBSERVER_CHANNEL_PENDING_LIMIT= 0xC000,
	/* more of a channel of the game: the state in which things can be
	asked of it, and two times. And two numbers of what the observer has at
	0x10: how long ago each of those times can be */
	_network_channel_state5= 5,
	NETWORK_CHANNEL_UNKNOWNA8_OFFSET= 0xA8,
	NETWORK_CHANNEL_UNKNOWND8_OFFSET= 0xD8,
	NETWORK_OBSERVER_PARAMETERS_UNKNOWN78_OFFSET= 0x78,
	NETWORK_OBSERVER_PARAMETERS_UNKNOWN7C_OFFSET= 0x7C,
	/* the sessions of a table of them, and what counts of each: its state,
	whether it is known by something in the network, and what by */
	MAXIMUM_NETWORK_SESSIONS= 3,
	NETWORK_SESSIONS_SESSION_STATE_OFFSET= 0x741C,
	NETWORK_SESSIONS_SESSION_HAS_IDENTIFIER_OFFSET= 0x24,
	NETWORK_SESSIONS_SESSION_IDENTIFIER_OFFSET= 0x1C,
	/* one more thing that the observer is given, and more of a channel:
	something that unknown_000783d0 looks for an address with. Of a channel
	of the game: which of the things of 0x2850 bytes of unknown_004d87dc is
	its own, and its address of the game. And of what network_unknown_00075910 is asked
	about */
	NETWORK_OBSERVER_UNKNOWN8_OFFSET= 0x8,
	NETWORK_OBSERVER_CHANNEL_UNKNOWN14_OFFSET= 0x14,
	NETWORK_CHANNEL_UNKNOWN14_OFFSET= 0x14,
	NETWORK_CHANNEL_UNKNOWN14_SIZE= 0x2850,
	NETWORK_CHANNEL_ADDRESS_OFFSET= 0x70,
	NETWORK_UNKNOWN_00075910_UNKNOWN3C_OFFSET= 0x3C,
	NETWORK_UNKNOWN_00075910_UNKNOWN30_OFFSET= 0x30,
	/* more of a channel: when it went into its state; a byte whose first
	bit is set when it is asked to connect; how many times it has tried, in
	16 bits; and a number of something of the network, NONE for none. Its
	states, and how its address of the game is. And what unknown_000784a0 is told when it
	loses its console, and when it is no more */
	NETWORK_OBSERVER_CHANNEL_STATE_TIME_OFFSET= 0x4,
	NETWORK_OBSERVER_CHANNEL_UNKNOWN8_OFFSET= 0x8,
	NETWORK_OBSERVER_CHANNEL_TRIES_OFFSET= 0xA,
	NETWORK_OBSERVER_CHANNEL_UNKNOWN70_OFFSET= 0x70,
	_network_observer_channel_state1= 1,
	_network_observer_channel_state2= 2,
	_network_observer_channel_state3= 3,
	_network_observer_channel_state4= 4,
	_network_observer_connect_status1= 1,
	_network_observer_connect_status2= 2,
	NETWORK_OBSERVER_CHANNEL_LOST_REASON= 0xD,
	NETWORK_OBSERVER_CHANNEL_DELETED_REASON= 0xE,
	/* more of a channel: two sets of the last numbers of something, and a
	byte that says whether more is done with what the channel is told */
	NETWORK_OBSERVER_CHANNEL_WINDOW250_OFFSET= 0x250,
	NETWORK_OBSERVER_CHANNEL_WINDOW360_OFFSET= 0x360,
	NETWORK_OBSERVER_CHANNEL_UNKNOWN48C_OFFSET= 0x48C,
	MAXIMUM_NETWORK_WINDOW_SAMPLES= 32,
	/* more of a channel: a time that is set while its channel of the game
	is not as network_observer_channel_unknown_000773a0 wants it; when it was first asked
	whether it is to be let go of; and when it was first asked that with
	something left to send. Two times of a channel of the game. Three more
	numbers of what the observer has at 0x10. And one more thing that
	unknown_00088650 is told */
	NETWORK_OBSERVER_CHANNEL_UNKNOWN94_OFFSET= 0x94,
	NETWORK_OBSERVER_CHANNEL_UNUSED_TIME_OFFSET= 0x98,
	NETWORK_OBSERVER_CHANNEL_WAITING_TIME_OFFSET= 0x9C,
	NETWORK_CHANNEL_UNKNOWN98_OFFSET= 0x98,
	NETWORK_CHANNEL_UNKNOWNC8_OFFSET= 0xC8,
	NETWORK_OBSERVER_PARAMETERS_UNKNOWN74_OFFSET= 0x74,
	NETWORK_OBSERVER_PARAMETERS_UNKNOWN80_OFFSET= 0x80,
	NETWORK_OBSERVER_PARAMETERS_UNKNOWN84_OFFSET= 0x84,
	_unknown_00088650_unknown10= 0x10,
	/* more of a channel: two bits of its byte of 0x8 (it has asked for
	something of the network about its console, and it has what was
	found); where what was found is kept; and two numbers that start over
	with it */
	_network_observer_channel_probed_bit= 0x8,
	_network_observer_channel_has_probe_bit= 0x10,
	NETWORK_OBSERVER_CHANNEL_UNKNOWN74_OFFSET= 0x74,
	NETWORK_OBSERVER_CHANNEL_UNKNOWN8C_OFFSET= 0x8C,
	NETWORK_OBSERVER_CHANNEL_UNKNOWN90_OFFSET= 0x90,
	/* the rest of what the observer keeps after its channels, which
	network_observer_reset_unknown4e08 starts over; and two numbers of what it has at
	0x10 that it starts from */
	NETWORK_OBSERVER_UNKNOWN4E04_OFFSET= 0x4E04,
	NETWORK_OBSERVER_UNKNOWN4E10_OFFSET= 0x4E10,
	NETWORK_OBSERVER_UNKNOWN4E14_OFFSET= 0x4E14,
	NETWORK_OBSERVER_UNKNOWN4E20_OFFSET= 0x4E20,
	NETWORK_OBSERVER_UNKNOWN4E24_OFFSET= 0x4E24,
	NETWORK_OBSERVER_UNKNOWN4E28_OFFSET= 0x4E28,
	NETWORK_OBSERVER_UNKNOWN4E30_OFFSET= 0x4E30,
	NETWORK_OBSERVER_UNKNOWN4E58_OFFSET= 0x4E58,
	NETWORK_OBSERVER_UNKNOWN4E5C_OFFSET= 0x4E5C,
	NETWORK_OBSERVER_UNKNOWN4F08_OFFSET= 0x4F08,
	NETWORK_OBSERVER_UNKNOWN4F0C_OFFSET= 0x4F0C,
	NETWORK_OBSERVER_UNKNOWN4F10_OFFSET= 0x4F10,
	NETWORK_OBSERVER_UNKNOWN4F14_OFFSET= 0x4F14,
	NETWORK_OBSERVER_UNKNOWN4F18_OFFSET= 0x4F18,
	NETWORK_OBSERVER_UNKNOWN4F1C_OFFSET= 0x4F1C,
	NETWORK_OBSERVER_UNKNOWN4F20_OFFSET= 0x4F20,
	NETWORK_OBSERVER_UNKNOWN4F24_OFFSET= 0x4F24,
	NETWORK_OBSERVER_UNKNOWN4F28_OFFSET= 0x4F28,
	NETWORK_OBSERVER_UNKNOWN4F2C_OFFSET= 0x4F2C,
	NETWORK_OBSERVER_UNKNOWN4F30_OFFSET= 0x4F30,
	NETWORK_OBSERVER_UNKNOWN4F34_OFFSET= 0x4F34,
	NETWORK_OBSERVER_UNKNOWN4F38_OFFSET= 0x4F38,
	NETWORK_OBSERVER_UNKNOWN4F3C_OFFSET= 0x4F3C,
	NETWORK_OBSERVER_UNKNOWN4E30_SIZE= 0x1C,
	NETWORK_OBSERVER_UNKNOWN4E5C_SIZE= 0xA8,
	NETWORK_OBSERVER_UNKNOWN4E24_DEFAULT= 0x400,
	NETWORK_OBSERVER_UNKNOWN4E04_UNIT= 1024,
	NETWORK_OBSERVER_PARAMETERS_UNKNOWN134_OFFSET= 0x134,
	NETWORK_OBSERVER_PARAMETERS_UNKNOWN138_OFFSET= 0x138,
	/* more of a channel: the count of what comes or goes by it. And of a
	channel of the game: a byte with bits, and which of the things of 0x97C
	bytes of unknown_004d87d8 is its own, which has a number at 0x96C */
	NETWORK_OBSERVER_CHANNEL_RATE_OFFSET= 0x178,
	MAXIMUM_NETWORK_RATE_SPANS= 20,
	NETWORK_RATE_MILLISECONDS_PER_SECOND= 1000,
	NETWORK_CHANNEL_UNKNOWN48_OFFSET= 0x48,
	_network_channel_unknown48_bit8= 0x8,
	NETWORK_CHANNEL_UNKNOWN10_OFFSET= 0x10,
	NETWORK_CHANNEL_UNKNOWN10_SIZE= 0x97C,
	NETWORK_CHANNEL_UNKNOWN10_UNKNOWN96C_OFFSET= 0x96C,
	/* more of a channel: of the last things that it was told of, a bit for
	each that was not so, and how many of those bits are set. And of what
	the observer has at 0x10: how many of the last things count, and what
	part of them (a real) is too many */
	NETWORK_OBSERVER_CHANNEL_UNKNOWN4B8_OFFSET= 0x4B8,
	NETWORK_OBSERVER_CHANNEL_UNKNOWN4BC_OFFSET= 0x4BC,
	NETWORK_OBSERVER_PARAMETERS_UNKNOWN174_OFFSET= 0x174,
	NETWORK_OBSERVER_PARAMETERS_UNKNOWN178_OFFSET= 0x178,
	/* what network_observer_channel_unknown_00075e40 gives unknown_00088650 */
	_unknown_00088650_unknown11= 0x11
};

/* the last numbers that are kept of something: how many are kept, where the
next one goes, each with the time that it was added at, the sum of all, and
how long ago the oldest was when the last one was added */
struct network_window_sample
{
	long time;
	long value;
};

struct network_window
{
	long count;
	long index;
	struct network_window_sample samples[MAXIMUM_NETWORK_WINDOW_SAMPLES];
	unsigned long sum;
	long span;
};

typedef char verify_network_window_size[sizeof(struct network_window) == 0x110 ? 1 : -1];

/* what a channel asks with for something of the network about its console:
what a session is known by, its key, and what the channel has at 0x14 */
struct network_observer_probe
{
	byte identifier[NETWORK_OBSERVER_SESSION_IDENTIFIER_SIZE];
	byte key[NETWORK_OBSERVER_SESSION_KEY_SIZE];
	byte unknown14[0x24];
};

typedef char verify_network_observer_probe_size[sizeof(struct network_observer_probe) == 0x3C ? 1 : -1];

/* the count of something by spans of time: how many there have been and of
what size in all; when the span of now started, and how many and of what
size there are in it; how long the twenty spans are, and one of them, in
milliseconds, and what turns a sum of the twenty into so much a second; and
the last twenty spans, with the sums of them */
struct network_rate_span
{
	unsigned long count;
	unsigned long size;
};

struct network_rate
{
	unsigned long long total_count;
	long long total_size;
	unsigned long start;
	unsigned long count;
	unsigned long size;
	long period;
	unsigned long span;
	real per_second;
	long index;
	struct network_rate_span spans[MAXIMUM_NETWORK_RATE_SPANS];
	unsigned long spans_count;
	unsigned long spans_size;
};

typedef char verify_network_rate_size[sizeof(struct network_rate) == 0xD4 || sizeof(struct network_rate) == 0xD8 ? 1 : -1];
typedef char verify_network_rate_spans_count_offset[offsetof(struct network_rate, spans_count) == 0xCC ? 1 : -1];

/* a function that takes its arguments off the stack */
#define NETWORK_OBSERVER_CALLBACK __attribute__((stdcall))

/* ---------- prototypes/NETWORK_OBSERVER.C */

long NETWORK_OBSERVER_CALLBACK network_time_since(long time);
void network_observer_dispose(byte *observer);
void NETWORK_OBSERVER_CALLBACK network_observer_set_session(long session_index, byte *observer, const byte *identifier,
	const byte *key, long unknown4, long unknown8);
boolean NETWORK_OBSERVER_CALLBACK network_observer_get_unknown4e08(const byte *observer, long *unknown4e08,
	real *fraction, long *unknown4e0c);
long network_observer_find_channel(const byte *observer, long session_index, long channel);
long NETWORK_OBSERVER_CALLBACK network_observer_find_channel_by_unknown1e(const byte *observer, const byte *unknown1e,
	long session_index);
void network_observer_channel_unknown_00075e40(long channel_index, const byte *observer);
long NETWORK_OBSERVER_CALLBACK network_sessions_find(byte *sessions, const byte *identifier);
boolean NETWORK_OBSERVER_CALLBACK network_observer_channel_unknown_000768b0(long channel_index, byte *observer,
	long type);
void NETWORK_OBSERVER_CALLBACK network_observer_channel_set_pending(long channel_index, byte *observer, long type);
boolean NETWORK_OBSERVER_CALLBACK network_observer_channel_alive(long channel_index, const byte *observer);
long network_observer_channel_connect_status(const byte *observer, long channel_index);
long network_unknown_00075910(const byte *unknown);
void NETWORK_OBSERVER_CALLBACK network_observer_channel_unknown_000797b0(byte *observer, long channel_index,
	long unknown);
void network_rate_update(struct network_rate *rate);
void network_rate_new(struct network_rate *rate, long period);
void network_window_reset(struct network_window *window);
void NETWORK_OBSERVER_CALLBACK network_observer_unknown_00076720(byte *observer, long game_channel_index, long size,
	long unknown);
void network_observer_reset_unknown4e08(byte *observer);
void network_observer_channel_update_probe(long channel_index, byte *observer);
boolean network_observer_channel_unused_expired(long channel_index, byte *observer, long unknown);
void NETWORK_OBSERVER_CALLBACK network_observer_channel_unknown_000773a0(long channel_index, byte *observer);
void NETWORK_OBSERVER_CALLBACK network_observer_update(byte *observer);
void network_window_add(struct network_window *window, long value);
void NETWORK_OBSERVER_CALLBACK network_observer_unknown_00076810(byte *observer, long game_channel_index, long unknown4,
	byte unknown8);
void NETWORK_OBSERVER_CALLBACK network_observer_channel_set_state(long state, byte *observer, long channel_index);
void network_observer_channel_delete(long channel_index, byte *observer);
void network_observer_channel_update_connection(long channel_index, byte *observer);
void network_observer_channel_connect(long channel_index, byte *observer);
struct transport_address;
boolean network_channel_get_address(const byte *game_channel, struct transport_address *address);
void NETWORK_OBSERVER_CALLBACK network_observer_channel_send(boolean out_of_band, long channel_index, byte *observer,
	long session_index, long type, long size, const void *data);

/* ---------- globals */

extern byte *unknown_004d87d4;
extern byte *unknown_004d87dc;
extern byte *unknown_004d87d8;

#endif
