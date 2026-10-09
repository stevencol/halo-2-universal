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
void network_observer_channel_delete(long channel_index, byte *observer);

/* 0x00088650: something that one of the game's channels is told, with a
number */
void unknown_00088650(byte *channel, long unknown);

/* ---------- prototypes of functions of the console's libraries */

/* 0x003314b0 */
long GetTickCount(void);

/* ---------- globals */

/* 0x00510548, 0x0051054c, of main.c: whether the time is the one that was
kept, and that time */
extern boolean unknown_00510548;
extern long unknown_0051054c;

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
			network_observer_channel_delete(channel_index, observer);
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

/* 0x00075890: how many milliseconds ago a time was: by the time that main.c
kept, if it says so, and by GetTickCount if not. The game has it before the
functions of the observer */
long NETWORK_OBSERVER_CALLBACK network_time_since(
	long time)
{
	return (long)((unsigned long)(unknown_00510548 ? unknown_0051054c : GetTickCount())-(unsigned long)time);
}

#include "networking/transport_security.h"

/* 0x00089070, of the channels of the game: a number of one of them, which
is how much it has waiting to be sent, in units of four */
long unknown_00089070(const byte *game_channel);

/* the channel of the game that a channel of the observer is, if it is one
and it is in its fifth state; NULL if not. The game has this in each function
that asks something of it */
static inline __attribute__((always_inline)) byte *network_observer_channel_connected_game_channel(
	const byte *observer,
	long channel_index)
{
	long channel= *(const long *)(network_observer_channel(observer, channel_index)+NETWORK_OBSERVER_CHANNEL_CHANNEL_OFFSET);

	if (channel!=NONE)
	{
		byte *game_channel= unknown_004d87d4+channel*NETWORK_CHANNEL_SIZE;

		if (*(const long *)(game_channel+NETWORK_CHANNEL_STATE_OFFSET)==_network_channel_state5)
		{
			return game_channel;
		}
	}

	return NULL;
}

/* 0x00075800: the session, of the three of a table, that has a state and is
known by eight bytes in the network; 0 if none. The game has it before the
functions of the observer */
long NETWORK_OBSERVER_CALLBACK network_sessions_find(
	byte *sessions,
	const byte *identifier)
{
	byte **table= (byte **)sessions;
	long index;

	for (index= 0; index<MAXIMUM_NETWORK_SESSIONS; index++)
	{
		const byte *session= table[index];

		if (session && *(const long *)(session+NETWORK_SESSIONS_SESSION_STATE_OFFSET)!=0 &&
			session[NETWORK_SESSIONS_SESSION_HAS_IDENTIFIER_OFFSET] &&
			memcmp(session+NETWORK_SESSIONS_SESSION_IDENTIFIER_OFFSET, identifier, NETWORK_OBSERVER_SESSION_IDENTIFIER_SIZE)==0)
		{
			return (long)session;
		}
	}

	return 0;
}

/* 0x000768b0: whether a channel of the observer can take something more of
a kind to send: its channel of the game has to be in its fifth state, and
what that one has waiting (unknown_00089070, times four as the processor
multiplies, so that a big number comes out small) has to be less than 0x4000, or than 0xC000 if the channel has something of that kind to send
already (a bit for each kind, in 64 bits) */
boolean NETWORK_OBSERVER_CALLBACK network_observer_channel_unknown_000768b0(
	long channel_index,
	byte *observer,
	long type)
{
	const byte *channel= network_observer_channel(observer, channel_index);
	const byte *game_channel= network_observer_channel_connected_game_channel(observer, channel_index);
	unsigned long low= (unsigned long)type<32 ? 1u<<type : 0;
	unsigned long high= (unsigned long)type>=32 && (unsigned long)type<64 ? 1u<<(type-32) : 0;

	if (!game_channel)
	{
		return FALSE;
	}
	if ((*(const unsigned long *)(channel+NETWORK_OBSERVER_CHANNEL_PENDING_OFFSET)&low) ||
		(*(const unsigned long *)(channel+NETWORK_OBSERVER_CHANNEL_PENDING_OFFSET+sizeof(long))&high))
	{
		return (long)((unsigned long)unknown_00089070(game_channel)*4)<NETWORK_OBSERVER_CHANNEL_PENDING_LIMIT;
	}

	return (long)((unsigned long)unknown_00089070(game_channel)*4)<NETWORK_OBSERVER_CHANNEL_WAITING_LIMIT;
}

/* 0x00076930: a channel of the observer has something of a kind to send, if
its channel of the game is in its fifth state: the kind gets its bit */
void NETWORK_OBSERVER_CALLBACK network_observer_channel_set_pending(
	long channel_index,
	byte *observer,
	long type)
{
	byte *channel= (byte *)network_observer_channel(observer, channel_index);
	unsigned long low= (unsigned long)type<32 ? 1u<<type : 0;
	unsigned long high= (unsigned long)type>=32 && (unsigned long)type<64 ? 1u<<(type-32) : 0;

	if (network_observer_channel_connected_game_channel(observer, channel_index))
	{
		*(unsigned long *)(channel+NETWORK_OBSERVER_CHANNEL_PENDING_OFFSET)|= low;
		*(unsigned long *)(channel+NETWORK_OBSERVER_CHANNEL_PENDING_OFFSET+sizeof(long))|= high;
	}
}

/* 0x000769a0: whether the console of a channel of the observer is there
still: FALSE if its channel of the game is not in its fifth state; and FALSE
too if the time of 0xA8 of that one was less ago than the number at 0x78 of
what the observer has at 0x10 says, and the time of 0xD8 was as long ago as
the number at 0x7C says, or longer */
boolean NETWORK_OBSERVER_CALLBACK network_observer_channel_alive(
	long channel_index,
	const byte *observer)
{
	const byte *game_channel= network_observer_channel_connected_game_channel(observer, channel_index);
	const byte *parameters;
	long since_unknownA8;
	long unknownD8= 0;
	long now;

	if (!game_channel)
	{
		return FALSE;
	}
	since_unknownA8= network_time_since(*(const long *)(game_channel+NETWORK_CHANNEL_UNKNOWNA8_OFFSET));
	if (*(const long *)(game_channel+NETWORK_CHANNEL_STATE_OFFSET)>_network_channel_state2)
	{
		unknownD8= *(const long *)(game_channel+NETWORK_CHANNEL_UNKNOWND8_OFFSET);
	}
	now= network_time_since(0);
	parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);

	return since_unknownA8>=*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN78_OFFSET) ||
		(long)((unsigned long)now-(unsigned long)unknownD8)<*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN7C_OFFSET);
}

/* 0x00078580: how the address of the game of a channel of the observer is,
as transport_address_get_connect_status says; 0 if it has none */
long network_observer_channel_connect_status(
	const byte *observer,
	long channel_index)
{
	const struct transport_address *address=
		(const struct transport_address *)(network_observer_channel(observer, channel_index)+NETWORK_OBSERVER_CHANNEL_ADDRESS_OFFSET);

	if (!transport_address_valid(address))
	{
		return 0;
	}

	return transport_address_get_connect_status(address);
}

/* 0x0007b140: sends something of a kind and a size to an address of the
game. 0x000783d0: the address of the game of a channel that has none of its
own, for a session; whether it could. 0x00095580: something of a kind and a
size is put in what a channel of the game has to send */
void __attribute__((stdcall)) unknown_0007b140(long unknown4, const struct transport_address *address, long type,
	long size, const void *data);
boolean __attribute__((stdcall)) unknown_000783d0(byte *observer, const byte *channel_unknown14,
	struct transport_address *address, long session_index, byte **unknown0, long *unknown4, long *unknown8);
void __attribute__((stdcall)) unknown_00095580(byte *unknown, long type, long size, const void *data);

/* 0x004d87dc: things of 0x2850 bytes, one for each of what the channels of
the game say at 0x14 */
byte *unknown_004d87dc;

/* 0x00075910: whether what something has at 0x3C is there and has its byte
of 0x30 set. The game has it before the functions of the observer */
long network_unknown_00075910(
	const byte *unknown)
{
	const byte *unknown3c= *(const byte **)(unknown+NETWORK_UNKNOWN_00075910_UNKNOWN3C_OFFSET);

	return unknown3c && unknown3c[NETWORK_UNKNOWN_00075910_UNKNOWN30_OFFSET] ? 1 : 0;
}

/* 0x00075930: the address of the game of a channel of the game, which it
has in any state after the first two; whether it has it. The game has it
before the functions of the observer */
boolean network_channel_get_address(
	const byte *game_channel,
	struct transport_address *address)
{
	long state= *(const long *)(game_channel+NETWORK_CHANNEL_STATE_OFFSET);

	if (state==0 || state==1)
	{
		return FALSE;
	}
	memcpy(address, game_channel+NETWORK_CHANNEL_ADDRESS_OFFSET, sizeof(*address));

	return TRUE;
}

/* 0x00075e80: something of a kind and a size is sent to the console of a
channel of the observer, for a session; nothing is done with a channel that
is none of the game. In one way, it is put in what the channel of the game
has to send, if that one is past its second state, and the kind is no more
one that the channel has to send. In the other, it is sent to the address of
the game of the channel: its own, or with none the one that unknown_000783d0
gives, if it can */
void NETWORK_OBSERVER_CALLBACK network_observer_channel_send(
	boolean out_of_band,
	long channel_index,
	byte *observer,
	long session_index,
	long type,
	long size,
	const void *data)
{
	byte *channel= (byte *)network_observer_channel(observer, channel_index);
	long game_channel_index= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_CHANNEL_OFFSET);
	byte *game_channel;

	if (game_channel_index==NONE)
	{
		return;
	}
	game_channel= unknown_004d87d4+game_channel_index*NETWORK_CHANNEL_SIZE;
	if (out_of_band)
	{
		const struct transport_address *own= (const struct transport_address *)(channel+NETWORK_OBSERVER_CHANNEL_ADDRESS_OFFSET);
		struct transport_address address;

		if (transport_address_valid(own))
		{
			address= *own;
		}
		else
		{
			byte *unknown0= game_channel;
			long unknown4;
			long unknown8;

			if (!unknown_000783d0(observer, channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN14_OFFSET, &address, session_index,
				&unknown0, &unknown4, &unknown8))
			{
				return;
			}
		}
		unknown_0007b140(*(const long *)(observer+NETWORK_OBSERVER_UNKNOWN8_OFFSET), &address, type, size, data);
	}
	else if (*(const long *)(game_channel+NETWORK_CHANNEL_STATE_OFFSET)>_network_channel_state2)
	{
		unsigned long low= (unsigned long)type<32 ? 1u<<type : 0;
		unsigned long high= (unsigned long)type>=32 && (unsigned long)type<64 ? 1u<<(type-32) : 0;

		*(unsigned long *)(channel+NETWORK_OBSERVER_CHANNEL_PENDING_OFFSET)&= ~low;
		*(unsigned long *)(channel+NETWORK_OBSERVER_CHANNEL_PENDING_OFFSET+sizeof(long))&= ~high;
		unknown_00095580(unknown_004d87dc+*(const long *)(game_channel+NETWORK_CHANNEL_UNKNOWN14_OFFSET)*
			NETWORK_CHANNEL_UNKNOWN14_SIZE, type, size, data);
	}
}

/* 0x000784a0: something that is noted of a channel of the observer, with a
byte and a number. 0x000886e0: a channel of the game is let go of.
0x0007b650: something of the network that a number says is let go of.
0x00076ff0, 0x000776a0: two more things that are done with a channel of the
observer each time */
void __attribute__((stdcall)) unknown_000784a0(long channel_index, byte *observer, boolean unknown0, long unknown4);
void __attribute__((stdcall)) unknown_000886e0(byte *game_channel);
void unknown_0007b650(long unknown);
void __attribute__((stdcall)) unknown_00076ff0(long channel_index, byte *observer);
void __attribute__((stdcall)) unknown_000776a0(byte *observer, long channel_index);

/* what the observer has of a session starts with something that is told of
its channels: by its fifth function, that one of them is in its first state
again */
struct network_observer_session_owner;

struct network_observer_session_owner_functions
{
	void *unknown0[4];
	void (__attribute__((thiscall)) *channel_reset)(struct network_observer_session_owner *owner, long channel_index);
};

struct network_observer_session_owner
{
	const struct network_observer_session_owner_functions *functions;
};

/* 0x00077330: a channel of the observer goes into a state, since now, if it
is not in it already. Into the first one, what each session that uses the
channel has in the observer is told */
void NETWORK_OBSERVER_CALLBACK network_observer_channel_set_state(
	long state,
	byte *observer,
	long channel_index)
{
	byte *channel= (byte *)network_observer_channel(observer, channel_index);
	long session_index;

	if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)==state)
	{
		return;
	}
	*(long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)= state;
	*(long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_TIME_OFFSET)= network_time_since(0);
	if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)!=_network_observer_channel_state1)
	{
		return;
	}
	for (session_index= 0; session_index<MAXIMUM_NETWORK_OBSERVER_SESSIONS; session_index++)
	{
		if (channel[NETWORK_OBSERVER_CHANNEL_SESSIONS_OFFSET]&(1u<<session_index))
		{
			struct network_observer_session_owner *owner= *(struct network_observer_session_owner **)(observer+
				NETWORK_OBSERVER_SESSIONS_OFFSET+session_index*NETWORK_OBSERVER_SESSION_SIZE+
				NETWORK_OBSERVER_SESSION_SESSION_OFFSET);

			owner->functions->channel_reset(owner, channel_index);
		}
	}
}

/* 0x00076ef0: a channel of the observer is no more: unknown_000784a0 notes it with 0xE,
its channel of the game is let go of, and so is what its number of 0x70
says; and all of it is empty, with NONE in those two */
void network_observer_channel_delete(
	long channel_index,
	byte *observer)
{
	byte *channel= (byte *)network_observer_channel(observer, channel_index);
	long game_channel_index;
	long unknown70;

	unknown_000784a0(channel_index, observer, FALSE, NETWORK_OBSERVER_CHANNEL_DELETED_REASON);
	game_channel_index= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_CHANNEL_OFFSET);
	if (game_channel_index!=NONE)
	{
		unknown_000886e0(unknown_004d87d4+game_channel_index*NETWORK_CHANNEL_SIZE);
		*(long *)(channel+NETWORK_OBSERVER_CHANNEL_CHANNEL_OFFSET)= NONE;
	}
	unknown70= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN70_OFFSET);
	if (unknown70!=NONE)
	{
		unknown_0007b650(unknown70);
		*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN70_OFFSET)= NONE;
	}
	memset(channel, 0, NETWORK_OBSERVER_CHANNEL_SIZE);
	*(long *)(channel+NETWORK_OBSERVER_CHANNEL_CHANNEL_OFFSET)= NONE;
	*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN70_OFFSET)= NONE;
}

/* 0x00076f50: a channel of the observer follows how its address of the game
is (network_observer_channel_connect_status). In its third state: with 2, it
goes into the fourth; with 1, it waits; with anything else it goes back to
the second, noted with 0, and counts one more try. In a state after the
third, or before none: unless it is 2, it goes back to the second, noted
with 0xD, with no tries. In the others nothing is done */
void network_observer_channel_update_connection(
	long channel_index,
	byte *observer)
{
	byte *channel= (byte *)network_observer_channel(observer, channel_index);
	long state= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET);
	long status;

	if (state>=0 && state<=_network_observer_channel_state2)
	{
		return;
	}
	status= network_observer_channel_connect_status(observer, channel_index);
	if (state==_network_observer_channel_state3)
	{
		if (status==_network_observer_connect_status2)
		{
			network_observer_channel_set_state(_network_observer_channel_state4, observer, channel_index);
		}
		else if (status!=_network_observer_connect_status1)
		{
			unknown_000784a0(channel_index, observer, TRUE, 0);
			network_observer_channel_set_state(_network_observer_channel_state2, observer, channel_index);
			(*(short *)(channel+NETWORK_OBSERVER_CHANNEL_TRIES_OFFSET))++;
		}
	}
	else if (status!=_network_observer_connect_status2)
	{
		unknown_000784a0(channel_index, observer, TRUE, NETWORK_OBSERVER_CHANNEL_LOST_REASON);
		network_observer_channel_set_state(_network_observer_channel_state2, observer, channel_index);
		*(short *)(channel+NETWORK_OBSERVER_CHANNEL_TRIES_OFFSET)= 0;
	}
}

/* 0x00076a40: a channel of the observer that has a channel of the game is
asked to connect: from its first state it goes into the second, with no
tries; the first bit of its byte of 0x8 is set; and what is done with a
channel each time is done now: network_observer_channel_update_connection, unknown_00076ff0 and
unknown_000776a0 */
void network_observer_channel_connect(
	long channel_index,
	byte *observer)
{
	byte *channel= (byte *)network_observer_channel(observer, channel_index);

	if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_CHANNEL_OFFSET)==NONE)
	{
		return;
	}
	if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)==_network_observer_channel_state1)
	{
		network_observer_channel_set_state(_network_observer_channel_state2, observer, channel_index);
		*(short *)(channel+NETWORK_OBSERVER_CHANNEL_TRIES_OFFSET)= 0;
	}
	channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN8_OFFSET]|= 1;
	network_observer_channel_update_connection(channel_index, observer);
	unknown_00076ff0(channel_index, observer);
	unknown_000776a0(observer, channel_index);
}

/* 0x000797b0: something more that is done with a channel of the observer
whose byte of 0x48C is set, with a byte */
void __attribute__((stdcall)) unknown_000797b0(byte *observer, long channel_index, long unknown);

/* 0x00092a30: one more number in the last ones that are kept of something,
with the time: it takes the place of the oldest, the sum of all has it and
not that one, and how long ago the oldest was is kept. The game has it far
from the functions of the observer */
void network_window_add(
	struct network_window *window,
	long value)
{
	long now= network_time_since(0);
	struct network_window_sample *oldest= window->samples+window->index;

	window->span= (long)((unsigned long)now-(unsigned long)oldest->time);
	window->sum-= (unsigned long)oldest->value;
	oldest->time= now;
	window->samples[window->index].value= value;
	window->sum+= (unsigned long)value;
	window->index= (window->index+1)%window->count;
}

/* which channel of the observer is in use and is one of the channels of the
game; NONE if none. The game has this in each function that is told of a
channel of the game */
static inline __attribute__((always_inline)) long network_observer_find_game_channel(
	const byte *observer,
	long game_channel_index)
{
	long channel_index;

	for (channel_index= 0; channel_index<MAXIMUM_NETWORK_OBSERVER_CHANNELS; channel_index++)
	{
		const byte *channel= network_observer_channel(observer, channel_index);

		if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)!=0 &&
			*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_CHANNEL_OFFSET)==game_channel_index)
		{
			return channel_index;
		}
	}

	return NONE;
}

/* 0x00076810: the observer is told two things of a channel of the game: the
channel of the observer that it is keeps whether the first is not so in the
numbers of 0x250, and the second in those of 0x360; and with its byte of
0x48C, unknown_000797b0 is told the first */
void NETWORK_OBSERVER_CALLBACK network_observer_unknown_00076810(
	byte *observer,
	long game_channel_index,
	long unknown4,
	byte unknown8)
{
	long channel_index= network_observer_find_game_channel(observer, game_channel_index);
	byte *channel= (byte *)network_observer_channel(observer, channel_index);

	network_window_add((struct network_window *)(channel+NETWORK_OBSERVER_CHANNEL_WINDOW250_OFFSET), (byte)unknown4==0);
	network_window_add((struct network_window *)(channel+NETWORK_OBSERVER_CHANNEL_WINDOW360_OFFSET), unknown8);
	if (channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48C_OFFSET])
	{
		unknown_000797b0(observer, channel_index, unknown4);
	}
}

/* 0x00077580: whether a channel of the observer that no session uses is to
be let go of, with a number. 0x000773a0, 0x00077480: two more things that are
done with a channel each time. 0x000779f0, 0x00078ac0, 0x0007a4a0: three
things that are done with the observer each time, after its channels */
boolean network_observer_channel_unused_expired(long channel_index, byte *observer, long unknown);
void __attribute__((stdcall)) network_observer_channel_unknown_000773a0(long channel_index, byte *observer);
void network_observer_channel_update_probe(long channel_index, byte *observer);
void unknown_000779f0(byte *observer);
void __attribute__((stdcall)) unknown_00078ac0(byte *observer);
void unknown_0007a4a0(byte *observer);

/* 0x00075da0: what the observer does each time. Each channel that is in
use: if no session uses it and network_observer_channel_unused_expired says so, it is no more
(network_observer_channel_delete); and if it still is in use, it follows its
connection (network_observer_channel_update_connection) and unknown_00076ff0,
unknown_000776a0, network_observer_channel_unknown_000773a0 and network_observer_channel_update_probe are done with it. Then
unknown_000779f0, unknown_00078ac0 and unknown_0007a4a0, with the observer */
void NETWORK_OBSERVER_CALLBACK network_observer_update(
	byte *observer)
{
	long channel_index;

	for (channel_index= 0; channel_index<MAXIMUM_NETWORK_OBSERVER_CHANNELS; channel_index++)
	{
		const byte *channel= network_observer_channel(observer, channel_index);

		if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)==0)
		{
			continue;
		}
		if (!channel[NETWORK_OBSERVER_CHANNEL_SESSIONS_OFFSET] &&
			network_observer_channel_unused_expired(channel_index, observer, NETWORK_OBSERVER_CHANNEL_DELETED_REASON))
		{
			network_observer_channel_delete(channel_index, observer);
		}
		if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)!=0)
		{
			network_observer_channel_update_connection(channel_index, observer);
			unknown_00076ff0(channel_index, observer);
			unknown_000776a0(observer, channel_index);
			network_observer_channel_unknown_000773a0(channel_index, observer);
			network_observer_channel_update_probe(channel_index, observer);
		}
	}
	unknown_000779f0(observer);
	unknown_00078ac0(observer);
	unknown_0007a4a0(observer);
}

/* 0x004cf56c: how long a channel of the observer that no session uses is
kept at most, in milliseconds. 0x004cf568: how long it is kept while what it
had to send has not gone */
long unknown_004cf56c;
long unknown_004cf568;

/* 0x00077580: whether a channel of the observer that no session uses is to
be let go of. If its channel of the game is in its fifth state: with less
than 0x4000 waiting to be sent (unknown_00089070) it has nothing left to
send; with more, that channel is told the number that is given
(unknown_00088650). The channel keeps when it was first asked this, and when
it was first asked with something left to send. Yes, once the first was
more than unknown_004cf56c milliseconds ago; yes too with something left to
send for more than unknown_004cf568 milliseconds; and if not, only if the
channel has no address of the game */
boolean network_observer_channel_unused_expired(
	long channel_index,
	byte *observer,
	long unknown)
{
	byte *channel= (byte *)network_observer_channel(observer, channel_index);
	byte *game_channel= network_observer_channel_connected_game_channel(observer, channel_index);
	boolean waiting= TRUE;
	boolean expired;

	if (game_channel)
	{
		if (unknown_00089070(game_channel)<NETWORK_OBSERVER_CHANNEL_WAITING_LIMIT)
		{
			waiting= FALSE;
		}
		else
		{
			unknown_00088650(game_channel, unknown);
		}
	}
	if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNUSED_TIME_OFFSET)==0)
	{
		*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNUSED_TIME_OFFSET)= network_time_since(0);
	}
	expired= network_time_since(*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNUSED_TIME_OFFSET))>unknown_004cf56c;
	if (waiting)
	{
		if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_WAITING_TIME_OFFSET)==0)
		{
			*(long *)(channel+NETWORK_OBSERVER_CHANNEL_WAITING_TIME_OFFSET)= network_time_since(0);
		}
		if (network_time_since(*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_WAITING_TIME_OFFSET))>unknown_004cf568 ||
			expired)
		{
			return TRUE;
		}
	}
	else
	{
		*(long *)(channel+NETWORK_OBSERVER_CHANNEL_WAITING_TIME_OFFSET)= 0;
		if (expired)
		{
			return TRUE;
		}
	}

	return !transport_address_valid(
		(const struct transport_address *)(channel+NETWORK_OBSERVER_CHANNEL_ADDRESS_OFFSET));
}

/* 0x000773a0: a channel of the observer that is in use and whose channel of
the game is in its fifth state and had its time of 0x98 less ago than the
number at 0x74 of what the observer has at 0x10 says: that channel is told
0x10 (unknown_00088650) if the time that the channel of the observer keeps
at 0x94 was as long ago as the number at 0x80 says and the time of 0xC8 of
the channel of the game as long ago as the one at 0x84 says. In any other
case the time of 0x94 is now */
void NETWORK_OBSERVER_CALLBACK network_observer_channel_unknown_000773a0(
	long channel_index,
	byte *observer)
{
	byte *channel= (byte *)network_observer_channel(observer, channel_index);
	byte *game_channel;
	const byte *parameters;

	if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)==0)
	{
		return;
	}
	game_channel= network_observer_channel_connected_game_channel(observer, channel_index);
	parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
	if (game_channel && network_time_since(*(const long *)(game_channel+NETWORK_CHANNEL_UNKNOWN98_OFFSET))<
		*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN74_OFFSET))
	{
		long since_unknown94= network_time_since(*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN94_OFFSET));
		long since_unknownC8= network_time_since(
			*(const long *)(game_channel+NETWORK_CHANNEL_STATE_OFFSET)>_network_channel_state2 ?
			*(const long *)(game_channel+NETWORK_CHANNEL_UNKNOWNC8_OFFSET) : 0);

		parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
		if (since_unknown94>=*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN80_OFFSET) &&
			since_unknownC8>=*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN84_OFFSET))
		{
			unknown_00088650(game_channel, _unknown_00088650_unknown10);
		}
	}
	else
	{
		*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN94_OFFSET)= network_time_since(0);
	}
}

/* 0x0007b4c0: something of the network is asked for, for a session and a
console; its number, NONE if it cannot be. 0x0007b6c0: whether it is done.
0x0007b7b0: what it found goes where it is told; whether there is any */
long __attribute__((stdcall)) unknown_0007b4c0(long unknown0, long unknown4, long unknown8,
	const struct network_observer_probe *probe);
boolean unknown_0007b6c0(long unknown);
boolean __attribute__((stdcall)) unknown_0007b7b0(long unknown, byte *result, long unknown4);

/* 0x00077480: a channel of the observer asks, once, for something of the
network about its console (unknown_0007b4c0), for the first session that
uses it and is known by something: with what the session is known by, its
key and what the channel has at 0x14. The number of what was asked for is
kept at 0x70. When it is done (unknown_0007b6c0): what it found is kept at
0x74 and, if there is any, the bit 0x10 of the byte of 0x8 is set and the
numbers of 0x8C and 0x90 are zero; it is let go of (unknown_0007b650); and
the bit 8 of that byte says that the channel has asked already */
void network_observer_channel_update_probe(
	long channel_index,
	byte *observer)
{
	byte *channel= (byte *)network_observer_channel(observer, channel_index);
	long unknown70;

	if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)!=0 &&
		*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN70_OFFSET)==NONE &&
		!(channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN8_OFFSET]&_network_observer_channel_probed_bit))
	{
		long session_index;

		for (session_index= 0; session_index<MAXIMUM_NETWORK_OBSERVER_SESSIONS; session_index++)
		{
			const byte *session= observer+NETWORK_OBSERVER_SESSIONS_OFFSET+session_index*NETWORK_OBSERVER_SESSION_SIZE;

			if ((channel[NETWORK_OBSERVER_CHANNEL_SESSIONS_OFFSET]&(1u<<session_index)) &&
				*(const long *)(session+NETWORK_OBSERVER_SESSION_UNKNOWN4_OFFSET)!=NONE)
			{
				struct network_observer_probe probe;

				memcpy(probe.identifier, session+NETWORK_OBSERVER_SESSION_IDENTIFIER_OFFSET, sizeof(probe.identifier));
				memcpy(probe.key, session+NETWORK_OBSERVER_SESSION_KEY_OFFSET, sizeof(probe.key));
				memcpy(probe.unknown14, channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN14_OFFSET, sizeof(probe.unknown14));
				*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN70_OFFSET)= unknown_0007b4c0(NONE, 1, 0, &probe);
				break;
			}
		}
	}
	unknown70= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN70_OFFSET);
	if (unknown70!=NONE && unknown_0007b6c0(unknown70))
	{
		if (unknown_0007b7b0(unknown70, channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN74_OFFSET, 0))
		{
			channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN8_OFFSET]|= _network_observer_channel_has_probe_bit;
			*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN90_OFFSET)= 0;
			*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN8C_OFFSET)= 0;
		}
		unknown_0007b650(*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN70_OFFSET));
		*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN70_OFFSET)= NONE;
		channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN8_OFFSET]|= _network_observer_channel_probed_bit;
	}
}

/* 0x00075af0: what the observer keeps after its channels starts over: a
number that is the one at 0x138 of what it has at 0x10; 0x400; the bigger of
that first number and the number of 0x4E04, in units of 1024, less the one at
0x134; three numbers that are NONE and three more further on; three times
that are now; a byte that is 1; and the rest zero. And no channel that is in
use has its byte of 0x48C */
void network_observer_reset_unknown4e08(
	byte *observer)
{
	const byte *parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
	long unknown4e04;
	long unknown4e28;
	long channel_index;

	*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4E20_OFFSET)=
		*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN138_OFFSET);
	*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4E24_OFFSET)= NETWORK_OBSERVER_UNKNOWN4E24_DEFAULT;
	observer[NETWORK_OBSERVER_UNKNOWN4E14_OFFSET]= FALSE;
	*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4E08_OFFSET)= NONE;
	*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4E0C_OFFSET)= NONE;
	*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4E10_OFFSET)= NONE;
	*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4E18_OFFSET)= 0;
	*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4E1C_OFFSET)= 0;
	memset(observer+NETWORK_OBSERVER_UNKNOWN4E30_OFFSET, 0, NETWORK_OBSERVER_UNKNOWN4E30_SIZE);
	*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4E58_OFFSET)= 0;
	memset(observer+NETWORK_OBSERVER_UNKNOWN4E5C_OFFSET, 0, NETWORK_OBSERVER_UNKNOWN4E5C_SIZE);
	unknown4e04= *(const long *)(observer+NETWORK_OBSERVER_UNKNOWN4E04_OFFSET);
	parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
	unknown4e28= unknown4e04/NETWORK_OBSERVER_UNKNOWN4E04_UNIT-
		*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN134_OFFSET);
	if (unknown4e28<=*(const long *)(observer+NETWORK_OBSERVER_UNKNOWN4E20_OFFSET))
	{
		unknown4e28= *(const long *)(observer+NETWORK_OBSERVER_UNKNOWN4E20_OFFSET);
	}
	*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4E28_OFFSET)= unknown4e28;
	*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4F08_OFFSET)= network_time_since(0);
	*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4F0C_OFFSET)= network_time_since(0);
	*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4F10_OFFSET)= 0;
	*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4F14_OFFSET)= 0;
	observer[NETWORK_OBSERVER_UNKNOWN4F18_OFFSET]= FALSE;
	*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4F1C_OFFSET)= 0;
	*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4F20_OFFSET)= 0;
	*(real *)(observer+NETWORK_OBSERVER_UNKNOWN4F24_OFFSET)= 0.0f;
	*(real *)(observer+NETWORK_OBSERVER_UNKNOWN4F28_OFFSET)= 0.0f;
	*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4F30_OFFSET)= NONE;
	*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4F34_OFFSET)= NONE;
	*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4F38_OFFSET)= NONE;
	observer[NETWORK_OBSERVER_UNKNOWN4F3C_OFFSET]= TRUE;
	*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4F2C_OFFSET)= network_time_since(0);
	for (channel_index= 0; channel_index<MAXIMUM_NETWORK_OBSERVER_CHANNELS; channel_index++)
	{
		byte *channel= (byte *)network_observer_channel(observer, channel_index);

		if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)!=0)
		{
			channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48C_OFFSET]= FALSE;
		}
	}
}

/* 0x00079660: something more that is done with a channel of the observer
whose byte of 0x48C is set, when something of a size is told of it */
void __attribute__((stdcall)) unknown_00079660(long unknown, long size, byte *observer, long channel_index,
	long unknown96c);

/* 0x004d87d8: things of 0x97C bytes, one for each of what the channels of
the game say at 0x10 */
byte *unknown_004d87d8;

/* 0x000928e0: how many of something, and of what size in all, there were in
each of the last twenty spans of time: when a span is over, what was counted
in it takes the place of the oldest, the sums of the twenty have it and not
that one, and the next span starts. After more than twenty spans with
nothing asked, all of it starts over. The game has it far from the functions
of the observer */
void network_rate_update(
	struct network_rate *rate)
{
	unsigned long now= (unsigned long)network_time_since(0);

	if (rate->start==0)
	{
		rate->start= now;
	}
	if (now<rate->span+rate->start)
	{
		return;
	}
	if ((long)((now-rate->start)/rate->span)>MAXIMUM_NETWORK_RATE_SPANS)
	{
		rate->spans_count= 0;
		rate->spans_size= 0;
		memset(rate->spans, 0, sizeof(rate->spans));
		rate->start= now;

		return;
	}
	do
	{
		struct network_rate_span *oldest= rate->spans+rate->index;

		rate->spans_count= rate->spans_count-oldest->count+rate->count;
		rate->spans_size= rate->spans_size-oldest->size+rate->size;
		oldest->count= rate->count;
		oldest->size= rate->size;
		rate->index= (rate->index+1)%MAXIMUM_NETWORK_RATE_SPANS;
		rate->count= 0;
		rate->size= 0;
		rate->start+= rate->span;
	} while (now>=rate->span+rate->start);
}

/* 0x00076720: the observer is told that something of a size came or went by
a channel of the game: the channel of the observer that it is counts it, in
all and in the span of now (network_rate_update); and with its byte of
0x48C, unknown_00079660 is told, with a number that the channel of the game
has for it if it is in its fifth state with the bit 8 of its byte of 0x48 */
void NETWORK_OBSERVER_CALLBACK network_observer_unknown_00076720(
	byte *observer,
	long game_channel_index,
	long size,
	long unknown)
{
	long channel_index= network_observer_find_game_channel(observer, game_channel_index);
	byte *channel= (byte *)network_observer_channel(observer, channel_index);
	const byte *game_channel= unknown_004d87d4+
		*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_CHANNEL_OFFSET)*NETWORK_CHANNEL_SIZE;
	struct network_rate *rate= (struct network_rate *)(channel+NETWORK_OBSERVER_CHANNEL_RATE_OFFSET);

	network_rate_update(rate);
	rate->total_count++;
	rate->total_size+= size;
	rate->count++;
	rate->size+= (unsigned long)size;
	if (channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48C_OFFSET])
	{
		long unknown96c= 0;

		if (*(const long *)(game_channel+NETWORK_CHANNEL_STATE_OFFSET)==_network_channel_state5 &&
			(game_channel[NETWORK_CHANNEL_UNKNOWN48_OFFSET]&_network_channel_unknown48_bit8))
		{
			unknown96c= *(const long *)(unknown_004d87d8+*(const long *)(game_channel+NETWORK_CHANNEL_UNKNOWN10_OFFSET)*
				NETWORK_CHANNEL_UNKNOWN10_SIZE+NETWORK_CHANNEL_UNKNOWN10_UNKNOWN96C_OFFSET);
		}
		unknown_00079660(unknown, size, observer, channel_index, unknown96c);
	}
}
