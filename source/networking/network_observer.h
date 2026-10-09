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
	/* what network_observer_channel_unknown_00075e40 gives unknown_00088650 */
	_unknown_00088650_unknown11= 0x11
};

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

/* ---------- globals */

extern byte *unknown_004d87d4;

#endif
