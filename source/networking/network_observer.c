/*
NETWORK_OBSERVER.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

What a session of the network game is given when it is set up: it has
something of each session, and a channel for each console that this one
talks with. The name of the file is a guess, and so are the names of its
functions: the game has them from 0x00075970 on.
*/

#include "network_observer.h"

#include <string.h>

/* ---------- prototypes of functions not decompiled yet */

/* 0x00076ef0: a channel is not in use any more */
void unknown_00076ef0(long channel_index, byte *observer);

/* 0x00088650: something that one of the game's channels is told, with a
number */
void unknown_00088650(byte *channel, long unknown);

/* ---------- globals */

/* 0x004d87d4: the channels of the game */
byte *unknown_004d87d4;

/* ---------- code */

/* a channel of the observer */
static inline __attribute__((always_inline)) const byte *network_observer_channel(
	const byte *observer,
	long channel_index)
{
	return observer+NETWORK_OBSERVER_CHANNELS_OFFSET+channel_index*NETWORK_OBSERVER_CHANNEL_SIZE;
}

/* 0x00075a40: the channels that are in use are not any more, and the
observer has nothing of any session and none of the three things that it was
given */
void network_observer_dispose(
	byte *observer)
{
	long channel_index;

	for (channel_index= 0; channel_index<MAXIMUM_NETWORK_OBSERVER_CHANNELS; channel_index++)
	{
		if (*(const long *)(network_observer_channel(observer, channel_index)+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)!=0)
		{
			unknown_00076ef0(channel_index, observer);
		}
	}
	memset(observer+NETWORK_OBSERVER_SESSIONS_OFFSET, 0, MAXIMUM_NETWORK_OBSERVER_SESSIONS*NETWORK_OBSERVER_SESSION_SIZE);
	*(long *)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET)= 0;
	*(long *)(observer+NETWORK_OBSERVER_UNKNOWNC_OFFSET)= 0;
	*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4_OFFSET)= 0;
}

/* 0x00075a90: what the observer has of a session: two numbers, what the
session is known by in the network and its key. With NONE for the first
number, only that number is kept: the session is known by nothing */
void NETWORK_OBSERVER_CALLBACK network_observer_set_session(
	long session_index,
	byte *observer,
	const byte *identifier,
	const byte *key,
	long unknown4,
	long unknown8)
{
	byte *session= observer+NETWORK_OBSERVER_SESSIONS_OFFSET+session_index*NETWORK_OBSERVER_SESSION_SIZE;

	if (unknown4!=NONE)
	{
		*(long *)(session+NETWORK_OBSERVER_SESSION_UNKNOWN4_OFFSET)= unknown4;
		*(long *)(session+NETWORK_OBSERVER_SESSION_UNKNOWN8_OFFSET)= unknown8;
		memcpy(session+NETWORK_OBSERVER_SESSION_IDENTIFIER_OFFSET, identifier, NETWORK_OBSERVER_SESSION_IDENTIFIER_SIZE);
		memcpy(session+NETWORK_OBSERVER_SESSION_KEY_OFFSET, key, NETWORK_OBSERVER_SESSION_KEY_SIZE);
	}
	else
	{
		*(long *)(session+NETWORK_OBSERVER_SESSION_UNKNOWN4_OFFSET)= NONE;
	}
}

/* 0x00075c80: two numbers of the observer, and how much the second of its
two counts is of both; FALSE, and nothing, if either number is NONE or the
counts add up to nothing */
boolean NETWORK_OBSERVER_CALLBACK network_observer_get_unknown4e08(
	const byte *observer,
	long *unknown4e08,
	real *fraction,
	long *unknown4e0c)
{
	long first= *(const long *)(observer+NETWORK_OBSERVER_UNKNOWN4E18_OFFSET);
	long second= *(const long *)(observer+NETWORK_OBSERVER_UNKNOWN4E1C_OFFSET);
	/* the game adds them as the processor does: two that are too big add up
	to less than nothing */
	long both= (long)((unsigned long)second+(unsigned long)first);

	if (*(const long *)(observer+NETWORK_OBSERVER_UNKNOWN4E08_OFFSET)==NONE ||
		*(const long *)(observer+NETWORK_OBSERVER_UNKNOWN4E0C_OFFSET)==NONE || both<=0)
	{
		return FALSE;
	}
	*unknown4e08= *(const long *)(observer+NETWORK_OBSERVER_UNKNOWN4E08_OFFSET);
	*fraction= (real)second/(real)both;
	*unknown4e0c= *(const long *)(observer+NETWORK_OBSERVER_UNKNOWN4E0C_OFFSET);

	return TRUE;
}

/* 0x00075ce0: which channel of the observer is in use, is one of the game's
channels and is used by a session; NONE if none */
long network_observer_find_channel(
	const byte *observer,
	long session_index,
	long channel)
{
	long channel_index;

	for (channel_index= 0; channel_index<MAXIMUM_NETWORK_OBSERVER_CHANNELS; channel_index++)
	{
		const byte *observer_channel= network_observer_channel(observer, channel_index);

		if (*(const long *)(observer_channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)!=0 &&
			*(const long *)(observer_channel+NETWORK_OBSERVER_CHANNEL_CHANNEL_OFFSET)==channel &&
			(observer_channel[NETWORK_OBSERVER_CHANNEL_SESSIONS_OFFSET]&(byte)(1u<<session_index)))
		{
			return channel_index;
		}
	}

	return NONE;
}

/* 0x00075d30: which channel of the observer is in use, is used by a session
and has six bytes; NONE if none */
long NETWORK_OBSERVER_CALLBACK network_observer_find_channel_by_unknown1e(
	const byte *observer,
	const byte *unknown1e,
	long session_index)
{
	long channel_index;

	for (channel_index= 0; channel_index<MAXIMUM_NETWORK_OBSERVER_CHANNELS; channel_index++)
	{
		const byte *observer_channel= network_observer_channel(observer, channel_index);

		if (*(const long *)(observer_channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)!=0 &&
			(observer_channel[NETWORK_OBSERVER_CHANNEL_SESSIONS_OFFSET]&(byte)(1u<<session_index)) &&
			memcmp(unknown1e, observer_channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN1E_OFFSET,
				NETWORK_OBSERVER_CHANNEL_UNKNOWN1E_SIZE)==0)
		{
			return channel_index;
		}
	}

	return NONE;
}

/* 0x00075e40: the channel of the game that a channel of the observer is, if
it is one and its state is more than the second, is told 0x11 by
unknown_00088650 */
void network_observer_channel_unknown_00075e40(
	long channel_index,
	const byte *observer)
{
	long channel= *(const long *)(network_observer_channel(observer, channel_index)+NETWORK_OBSERVER_CHANNEL_CHANNEL_OFFSET);

	if (channel!=NONE)
	{
		byte *game_channel= unknown_004d87d4+channel*NETWORK_CHANNEL_SIZE;

		if (*(const long *)(game_channel+NETWORK_CHANNEL_STATE_OFFSET)>_network_channel_state2)
		{
			unknown_00088650(game_channel, _unknown_00088650_unknown11);
		}
	}
}
