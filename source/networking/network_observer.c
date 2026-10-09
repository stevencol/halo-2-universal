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
the game of the channel: its own, or with none the one that network_observer_session_get_address
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
			long unknown4;
			byte identifier[NETWORK_OBSERVER_SESSION_IDENTIFIER_SIZE];
			byte key[NETWORK_OBSERVER_SESSION_KEY_SIZE];

			if (!network_observer_session_get_address(observer, channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN14_OFFSET, &address, session_index,
				&unknown4, identifier, key))
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
0x0007b650: something of the network that a number says is let go of */
void __attribute__((stdcall)) network_observer_channel_forget_address(long channel_index, byte *observer, boolean unknown0, long unknown4);
void __attribute__((stdcall)) unknown_000886e0(byte *game_channel);
void transport_qos_delete(long index);

/* what the observer has of a session starts with something that is told of
its channels: by its fifth function, that one of them is in its first state
again; by its sixth, which of its consoles one of them is for; by its fourth, that one of them has what its channel of the game is
of, or has it no more; and asked things of one of them: something by its
first, and by its third whether it is still wanted, with whether it has had
what its channel of the game is of */
struct network_observer_session_owner;

struct network_observer_session_owner_functions
{
	boolean (__attribute__((thiscall)) *channel_unknown0)(struct network_observer_session_owner *owner, long channel_index);
	boolean (__attribute__((thiscall)) *channel_unknown4)(struct network_observer_session_owner *owner, long channel_index);
	boolean (__attribute__((thiscall)) *channel_wanted)(struct network_observer_session_owner *owner, long channel_index,
		boolean was_established);
	void (__attribute__((thiscall)) *channel_established)(struct network_observer_session_owner *owner, long channel_index,
		long unknown10, boolean established);
	void (__attribute__((thiscall)) *channel_reset)(struct network_observer_session_owner *owner, long channel_index);
	long (__attribute__((thiscall)) *channel_peer)(struct network_observer_session_owner *owner, long channel_index);
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

/* 0x00076ef0: a channel of the observer is no more: network_observer_channel_forget_address notes it with 0xE,
its channel of the game is let go of, and so is what its number of 0x70
says; and all of it is empty, with NONE in those two */
void network_observer_channel_delete(
	long channel_index,
	byte *observer)
{
	byte *channel= (byte *)network_observer_channel(observer, channel_index);
	long game_channel_index;
	long unknown70;

	network_observer_channel_forget_address(channel_index, observer, FALSE, NETWORK_OBSERVER_CHANNEL_DELETED_REASON);
	game_channel_index= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_CHANNEL_OFFSET);
	if (game_channel_index!=NONE)
	{
		unknown_000886e0(unknown_004d87d4+game_channel_index*NETWORK_CHANNEL_SIZE);
		*(long *)(channel+NETWORK_OBSERVER_CHANNEL_CHANNEL_OFFSET)= NONE;
	}
	unknown70= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN70_OFFSET);
	if (unknown70!=NONE)
	{
		transport_qos_delete(unknown70);
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
			network_observer_channel_forget_address(channel_index, observer, TRUE, 0);
			network_observer_channel_set_state(_network_observer_channel_state2, observer, channel_index);
			(*(short *)(channel+NETWORK_OBSERVER_CHANNEL_TRIES_OFFSET))++;
		}
	}
	else if (status!=_network_observer_connect_status2)
	{
		network_observer_channel_forget_address(channel_index, observer, TRUE, NETWORK_OBSERVER_CHANNEL_LOST_REASON);
		network_observer_channel_set_state(_network_observer_channel_state2, observer, channel_index);
		*(short *)(channel+NETWORK_OBSERVER_CHANNEL_TRIES_OFFSET)= 0;
	}
}

/* 0x00076a40: a channel of the observer that has a channel of the game is
asked to connect: from its first state it goes into the second, with no
tries; the first bit of its byte of 0x8 is set; and what is done with a
channel each time is done now: network_observer_channel_update_connection, network_observer_channel_update_game_channel and
network_observer_channel_update_tries */
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
	network_observer_channel_update_game_channel(channel_index, observer);
	network_observer_channel_update_tries(observer, channel_index);
}

/* 0x000797b0: something more that is done with a channel of the observer
whose byte of 0x48C is set, with a byte */
void __attribute__((stdcall)) network_observer_channel_unknown_000797b0(byte *observer, long channel_index, long unknown);

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
0x48C, network_observer_channel_unknown_000797b0 is told the first */
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
		network_observer_channel_unknown_000797b0(observer, channel_index, unknown4);
	}
}

/* 0x00077580: whether a channel of the observer that no session uses is to
be let go of, with a number. 0x000773a0, 0x00077480: two more things that are
done with a channel each time. 0x000779f0, 0x00078ac0, 0x0007a4a0: three
things that are done with the observer each time, after its channels */
boolean network_observer_channel_unused_expired(long channel_index, byte *observer, long unknown);
void __attribute__((stdcall)) network_observer_channel_unknown_000773a0(long channel_index, byte *observer);
void network_observer_channel_update_probe(long channel_index, byte *observer);

/* 0x00075da0: what the observer does each time. Each channel that is in
use: if no session uses it and network_observer_channel_unused_expired says so, it is no more
(network_observer_channel_delete); and if it still is in use, it follows its
connection (network_observer_channel_update_connection) and network_observer_channel_update_game_channel,
network_observer_channel_update_tries, network_observer_channel_unknown_000773a0 and network_observer_channel_update_probe are done with it. Then
network_observer_update_bandwidth, network_observer_update_flow and network_observer_update_voice_mode, with the observer */
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
			network_observer_channel_update_game_channel(channel_index, observer);
			network_observer_channel_update_tries(observer, channel_index);
			network_observer_channel_unknown_000773a0(channel_index, observer);
			network_observer_channel_update_probe(channel_index, observer);
		}
	}
	network_observer_update_bandwidth(observer);
	network_observer_update_flow(observer);
	network_observer_update_voice_mode(observer);
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

#include "networking/transport_qos.h"

/* 0x00077480: a channel of the observer makes, once, an attempt at finding
out how the connection with its console is (transport_qos_new), for the
first session that uses it and is known by something: with what the session
is known by, its key and what the channel has at 0x14. The index of the
attempt is kept at 0x70. When it is done (transport_qos_is_complete): what
it found is kept at 0x74 and, if there is any, the bit 0x10 of the byte of
0x8 is set and the numbers of 0x8C and 0x90 are zero; it is no more
(transport_qos_delete); and the bit 8 of that byte says that the channel has
asked already */
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
				*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN70_OFFSET)= transport_qos_new(NONE, 1, 0,
					(const struct transport_qos_target *)&probe);
				break;
			}
		}
	}
	unknown70= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN70_OFFSET);
	if (unknown70!=NONE && transport_qos_is_complete(unknown70))
	{
		if (transport_qos_get_result(unknown70,
			(struct transport_qos_result *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN74_OFFSET), 0))
		{
			channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN8_OFFSET]|= _network_observer_channel_has_probe_bit;
			*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN90_OFFSET)= 0;
			*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN8C_OFFSET)= 0;
		}
		transport_qos_delete(*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN70_OFFSET));
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
void __attribute__((stdcall)) network_observer_channel_unknown_00079660(long unknown, long size, byte *observer, long channel_index,
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
0x48C, network_observer_channel_unknown_00079660 is told, with a number that the channel of the game
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
		network_observer_channel_unknown_00079660(unknown, size, observer, channel_index, unknown96c);
	}
}

/* 0x00092870: a count by spans of time starts, for a time in milliseconds
that the twenty spans are: all of it is zero but that time, the twentieth of
it that a span is, and 1000 over it, which turns a sum of the twenty spans
into so much a second. The game has it far from the functions of the
observer */
void network_rate_new(
	struct network_rate *rate,
	long period)
{
	rate->period= period;
	rate->span= period/MAXIMUM_NETWORK_RATE_SPANS;
	rate->total_count= 0;
	rate->total_size= 0;
	rate->start= 0;
	rate->per_second= NETWORK_RATE_MILLISECONDS_PER_SECOND/(real)period;
	rate->count= 0;
	rate->size= 0;
	rate->index= 0;
	memset(rate->spans, 0, sizeof(rate->spans));
	rate->spans_count= 0;
	rate->spans_size= 0;
}

/* 0x000929d0: the last numbers that are kept of something start over: each
is zero and of now, and so are their sum and how long ago the oldest was */
void network_window_reset(
	struct network_window *window)
{
	long now= network_time_since(0);
	long index;

	for (index= 0; index<window->count; index++)
	{
		window->samples[index].time= now;
		window->samples[index].value= 0;
	}
	window->index= 0;
	window->sum= 0;
	window->span= 0;
}

/* 0x00079850: something that is done with a channel of the observer when
too many of the last things that it was told of were not so */
void __attribute__((stdcall)) network_observer_channel_drop_flow(byte *observer, long channel_index);

/* 0x000797b0: a channel of the observer is told one more thing, which is so
or not (a byte): it keeps a bit for each of the last ones that were not, as
many as the number at 0x174 of what the observer has at 0x10 says, and how
many of those bits are set. When they are as many as that number times the
real at 0x178, or more, network_observer_channel_drop_flow is done and the count starts
over */
void NETWORK_OBSERVER_CALLBACK network_observer_channel_unknown_000797b0(
	byte *observer,
	long channel_index,
	long unknown)
{
	byte *channel= (byte *)network_observer_channel(observer, channel_index);
	const byte *parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
	unsigned long oldest= 1u<<((*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN174_OFFSET)-1)&31);
	unsigned long bits= *(const unsigned long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4BC_OFFSET);

	if (bits&oldest)
	{
		(*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4B8_OFFSET))--;
	}
	bits= (bits&~oldest)<<1;
	*(unsigned long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4BC_OFFSET)= bits;
	if (!(byte)unknown)
	{
		long count;

		*(unsigned long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4BC_OFFSET)= bits|1;
		count= ++*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4B8_OFFSET);
		parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
		if ((real)count>=(real)*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN174_OFFSET)*
			*(const real *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN178_OFFSET))
		{
			network_observer_channel_drop_flow(observer, channel_index);
			*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4B8_OFFSET)= 0;
			*(unsigned long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4BC_OFFSET)= 0;
		}
	}
}

/* how many bits a second a number of bytes of a channel is, counted since
the time of 0x4F38 of the observer; 0 if that time is not before now. The
game multiplies as the processor does */
static inline __attribute__((always_inline)) long network_observer_channel_bits_per_second(
	const byte *observer,
	long channel_index,
	long offset)
{
	const byte *channel= network_observer_channel(observer, channel_index);
	long elapsed= network_time_since(*(const long *)(observer+NETWORK_OBSERVER_UNKNOWN4F38_OFFSET));

	if (elapsed<=0)
	{
		return 0;
	}

	return (long)(*(const unsigned long *)(channel+offset)*NETWORK_OBSERVER_BITS_PER_BYTE_MILLISECOND)/elapsed;
}

/* 0x00079560: how many bits a second the number of 0x4A8 of a channel of the
observer is */
long network_observer_channel_unknown4a8_bits_per_second(
	long channel_index,
	const byte *observer)
{
	return network_observer_channel_bits_per_second(observer, channel_index, NETWORK_OBSERVER_CHANNEL_UNKNOWN4A8_OFFSET);
}

/* 0x000795b0: how many bits a second the number of 0x4A4 of a channel of the
observer is */
long network_observer_channel_unknown4a4_bits_per_second(
	long channel_index,
	const byte *observer)
{
	return network_observer_channel_bits_per_second(observer, channel_index, NETWORK_OBSERVER_CHANNEL_UNKNOWN4A4_OFFSET);
}

/* 0x00079c00: something that is done with a channel of the observer, with a
number, when its flow is to change */
void __attribute__((stdcall)) network_observer_channel_lower_flow(byte *observer, long channel_index, boolean unknown);

/* 0x00079d90: the number of 0x4DC of a channel of the observer is none of
them, since now, if it was one */
void network_observer_channel_reset_unknown4dc(
	long channel_index,
	byte *observer)
{
	byte *channel= (byte *)network_observer_channel(observer, channel_index);

	if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4DC_OFFSET)!=0)
	{
		*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4DC_OFFSET)= 0;
		*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4E0_OFFSET)= network_time_since(0);
	}
}

/* 0x0007a160: something has gone well once more for a channel of the
observer. Once the times are as many as the number at 0x1E4 of what the
observer has at 0x10, the number of 0x4F0 of the channel goes up by the one
at 0x1E8; before that, every so many times as the one at 0x1E0 says,
network_observer_channel_lower_flow is done with nothing */
void network_observer_channel_unknown_0007a160(
	long channel_index,
	byte *observer)
{
	byte *channel= (byte *)network_observer_channel(observer, channel_index);
	const byte *parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
	long count= ++*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4EC_OFFSET);

	if (count>=*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN1E4_OFFSET))
	{
		*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4F0_OFFSET)=
			(long)(*(const unsigned long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4F0_OFFSET)+
			*(const unsigned long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN1E8_OFFSET));
	}
	else if (count%*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN1E0_OFFSET)==0)
	{
		network_observer_channel_lower_flow(observer, channel_index, 0);
	}
}

/* 0x00078880: each channel of the observer that is in use lets go of its
channel of the game, and has none: one that is past its second state is told
3 first (unknown_00088650), and network_observer_channel_update_game_channel is done with the channel */
void NETWORK_OBSERVER_CALLBACK network_observer_release_game_channels(
	byte *observer)
{
	long channel_index;

	for (channel_index= 0; channel_index<MAXIMUM_NETWORK_OBSERVER_CHANNELS; channel_index++)
	{
		byte *channel= (byte *)network_observer_channel(observer, channel_index);

		if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)!=0)
		{
			byte *game_channel= unknown_004d87d4+
				*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_CHANNEL_OFFSET)*NETWORK_CHANNEL_SIZE;

			if (*(const long *)(game_channel+NETWORK_CHANNEL_STATE_OFFSET)>_network_channel_state2)
			{
				unknown_00088650(game_channel, _unknown_00088650_unknown3);
				network_observer_channel_update_game_channel(channel_index, observer);
			}
			unknown_000886e0(unknown_004d87d4+
				*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_CHANNEL_OFFSET)*NETWORK_CHANNEL_SIZE);
			*(long *)(channel+NETWORK_OBSERVER_CHANNEL_CHANNEL_OFFSET)= NONE;
		}
	}
}

/* 0x00077940: whether what any session that uses a channel of the observer
has in it says yes of the channel, by its first function. Only if the
channel of the game is in its fourth state or after, and its time of 0xA8
was as long ago as the number at 0x70 of what the observer has at 0x10
says */
boolean network_observer_channel_unknown_00077940(
	byte *observer,
	long channel_index)
{
	const byte *channel= network_observer_channel(observer, channel_index);
	long game_channel_index= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_CHANNEL_OFFSET);
	const byte *game_channel;
	const byte *parameters;
	long session_index;

	if (game_channel_index==NONE)
	{
		return FALSE;
	}
	game_channel= unknown_004d87d4+game_channel_index*NETWORK_CHANNEL_SIZE;
	if (*(const long *)(game_channel+NETWORK_CHANNEL_STATE_OFFSET)<_network_channel_state4)
	{
		return FALSE;
	}
	parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
	if (network_time_since(*(const long *)(game_channel+NETWORK_CHANNEL_UNKNOWNA8_OFFSET))<
		*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN70_OFFSET))
	{
		return FALSE;
	}
	for (session_index= 0; session_index<MAXIMUM_NETWORK_OBSERVER_SESSIONS; session_index++)
	{
		if (channel[NETWORK_OBSERVER_CHANNEL_SESSIONS_OFFSET]&(1u<<session_index))
		{
			struct network_observer_session_owner *owner= *(struct network_observer_session_owner **)(observer+
				NETWORK_OBSERVER_SESSIONS_OFFSET+session_index*NETWORK_OBSERVER_SESSION_SIZE+
				NETWORK_OBSERVER_SESSION_SESSION_OFFSET);

			if (owner->functions->channel_unknown0(owner, channel_index))
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

/* 0x000783d0: the address of the game of a console that is known by
something, for a session of the observer: the one that the console's
libraries give for the number of 0x4 of the session, the one of 0x8 and the
port 1000, if it can be connected to (transport_address_connect). With it go
that number of 0x4, what the session is known by and its key. FALSE, and an
empty address, if the observer has no such session, the session is known by
nothing, or there is no such address */
boolean NETWORK_OBSERVER_CALLBACK network_observer_session_get_address(
	const byte *observer,
	const byte *peer_address,
	struct transport_address *address,
	long session_index,
	long *unknown4,
	byte *identifier,
	byte *key)
{
	const byte *session= observer+NETWORK_OBSERVER_SESSIONS_OFFSET+session_index*NETWORK_OBSERVER_SESSION_SIZE;
	struct transport_address found;

	if (*(const long *)(session+NETWORK_OBSERVER_SESSION_SESSION_OFFSET)!=0 &&
		*(const long *)(session+NETWORK_OBSERVER_SESSION_UNKNOWN4_OFFSET)!=NONE &&
		transport_security_address_from_peer_address(*(const long *)(session+NETWORK_OBSERVER_SESSION_UNKNOWN4_OFFSET),
			&found, peer_address, *(const long *)(session+NETWORK_OBSERVER_SESSION_UNKNOWN8_OFFSET), NETWORK_OBSERVER_PORT) &&
		transport_address_connect(&found))
	{
		*address= found;
		*unknown4= *(const long *)(session+NETWORK_OBSERVER_SESSION_UNKNOWN4_OFFSET);
		memcpy(identifier, session+NETWORK_OBSERVER_SESSION_IDENTIFIER_OFFSET, NETWORK_OBSERVER_SESSION_IDENTIFIER_SIZE);
		memcpy(key, session+NETWORK_OBSERVER_SESSION_KEY_OFFSET, NETWORK_OBSERVER_SESSION_KEY_SIZE);

		return TRUE;
	}
	memset(address, 0, sizeof(*address));

	return FALSE;
}

/* 0x00078330: a channel of the observer gets an address of the game for its
console, unless the one that it has is being connected to or is connected
(network_observer_channel_connect_status 1 or 2). If not, network_observer_channel_forget_address
notes it with nothing, and the first session that uses the channel, has not
failed before and gives one (network_observer_session_get_address) is the
session of the channel from now on. Whether the channel has an address */
boolean NETWORK_OBSERVER_CALLBACK network_observer_channel_find_address(
	long channel_index,
	byte *observer)
{
	byte *channel= (byte *)network_observer_channel(observer, channel_index);
	long status= network_observer_channel_connect_status(observer, channel_index);
	long session_index;

	if (status>0 && status<=_network_observer_connect_status2)
	{
		return TRUE;
	}
	network_observer_channel_forget_address(channel_index, observer, FALSE, 0);
	for (session_index= 0; session_index<MAXIMUM_NETWORK_OBSERVER_SESSIONS; session_index++)
	{
		unsigned long bit= 1u<<session_index;

		if ((channel[NETWORK_OBSERVER_CHANNEL_SESSIONS_OFFSET]&bit) &&
			!(*(const unsigned long *)(channel+NETWORK_OBSERVER_CHANNEL_FAILED_SESSIONS_OFFSET)&bit) &&
			network_observer_session_get_address(observer, channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN14_OFFSET,
				(struct transport_address *)(channel+NETWORK_OBSERVER_CHANNEL_ADDRESS_OFFSET), session_index,
				(long *)(channel+NETWORK_OBSERVER_CHANNEL_SESSION_UNKNOWN4_OFFSET),
				channel+NETWORK_OBSERVER_CHANNEL_SESSION_IDENTIFIER_OFFSET,
				channel+NETWORK_OBSERVER_CHANNEL_SESSION_KEY_OFFSET))
		{
			*(long *)(channel+NETWORK_OBSERVER_CHANNEL_SESSION_OFFSET)= session_index;

			return TRUE;
		}
	}

	return FALSE;
}

/* 0x0007b390: something of the network that the observer was given is told
that an address of the game is no more. 0x003cd344, of the console's
libraries */
void network_message_gateway_flush_address(const struct transport_address *address, byte *gateway);
long __attribute__((stdcall)) XNetUnregisterInAddr(unsigned long in_address);

/* 0x000784a0: a channel of the observer has no address of the game any
more. Its channel of the game, if it is past its second state, is told a
number (unknown_00088650). And if it had an address: network_message_gateway_flush_address is
told; with a byte, the session that the address was of is one that failed;
the console's libraries let go of the address; and it is empty */
void NETWORK_OBSERVER_CALLBACK network_observer_channel_forget_address(
	long channel_index,
	byte *observer,
	boolean failed,
	long unknown)
{
	byte *channel= (byte *)network_observer_channel(observer, channel_index);
	long game_channel_index= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_CHANNEL_OFFSET);
	struct transport_address *address= (struct transport_address *)(channel+NETWORK_OBSERVER_CHANNEL_ADDRESS_OFFSET);
	unsigned long in_address;

	if (game_channel_index!=NONE)
	{
		byte *game_channel= unknown_004d87d4+game_channel_index*NETWORK_CHANNEL_SIZE;

		if (*(const long *)(game_channel+NETWORK_CHANNEL_STATE_OFFSET)>_network_channel_state2)
		{
			unknown_00088650(game_channel, unknown);
		}
	}
	if (!transport_address_valid(address))
	{
		return;
	}
	network_message_gateway_flush_address(address, *(byte **)(observer+NETWORK_OBSERVER_UNKNOWN8_OFFSET));
	if (failed)
	{
		long session_index= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_SESSION_OFFSET);

		if (session_index>=0 && session_index<MAXIMUM_NETWORK_OBSERVER_SESSIONS)
		{
			*(unsigned long *)(channel+NETWORK_OBSERVER_CHANNEL_FAILED_SESSIONS_OFFSET)|= 1u<<session_index;
		}
	}
	if (transport_address_get_secure_in_address(address, &in_address))
	{
		XNetUnregisterInAddr(in_address);
	}
	memset(address, 0, sizeof(*address));
}

/* 0x00078a10: four numbers of a channel of the observer that is in use and
has its byte of 0x48C: the ones of 0x4B0, 0x49C and 0x514, and how much, out
of a hundred, the sum of its numbers of 0x250 is of how many are kept. FALSE,
and nothing, if there is no such channel */
boolean NETWORK_OBSERVER_CALLBACK network_observer_channel_get_unknown4b0(
	long channel_index,
	const byte *observer,
	long *unknown4b0,
	long *unknown49c,
	long *unknown514,
	long *percent)
{
	const byte *channel;
	const struct network_window *window;

	if (channel_index<0 || channel_index>=MAXIMUM_NETWORK_OBSERVER_CHANNELS)
	{
		return FALSE;
	}
	channel= network_observer_channel(observer, channel_index);
	if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)==0 ||
		!channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48C_OFFSET])
	{
		return FALSE;
	}
	window= (const struct network_window *)(channel+NETWORK_OBSERVER_CHANNEL_WINDOW250_OFFSET);
	*unknown4b0= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4B0_OFFSET);
	*unknown49c= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN49C_OFFSET);
	*unknown514= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN514_OFFSET);
	*percent= (long)((real)(long)window->sum/(real)window->count*100.0f);

	return TRUE;
}

/* 0x00079480: the interval that the bytes of each channel of the observer
are counted in is over, and another starts now. Of each channel that is in
use and has its byte of 0x48C: if any bytes of 0x4A4 were counted, its three
bytes of 0x50C keep what the three of 0x4A0 noted; an interval with none of
the three is one more in a row of those (0x51C), and one with any is one more
in a row of the others (0x518); its number of 0x4F0 is no more than the one
of 0x4B0, and when it goes down the times that something went well start
over; and the counts and two of the three bytes of 0x4A0 start over */
void NETWORK_OBSERVER_CALLBACK network_observer_new_interval(
	byte *observer)
{
	long now= network_time_since(0);
	long channel_index;

	/* the game asks for the time once more, for nothing */
	network_time_since(0);
	for (channel_index= 0; channel_index<MAXIMUM_NETWORK_OBSERVER_CHANNELS; channel_index++)
	{
		byte *channel= (byte *)network_observer_channel(observer, channel_index);

		if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)!=0 &&
			channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48C_OFFSET])
		{
			long unknown4a4= network_observer_channel_unknown4a4_bits_per_second(channel_index, observer);
			long unknown4b0;

			/* and this one, for nothing too */
			network_observer_channel_unknown4a8_bits_per_second(channel_index, observer);
			unknown4b0= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4B0_OFFSET);
			if (unknown4a4>0)
			{
				channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN50C_OFFSET]|= channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN4A1_OFFSET];
				channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN50D_OFFSET]|= channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN4A0_OFFSET];
				channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN50E_OFFSET]|= channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN4A2_OFFSET];
			}
			if (!channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN50C_OFFSET] &&
				!channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN50D_OFFSET] &&
				!channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN50E_OFFSET])
			{
				++*(unsigned long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN51C_OFFSET);
				*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN518_OFFSET)= 0;
			}
			else
			{
				++*(unsigned long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN518_OFFSET);
				*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN51C_OFFSET)= 0;
			}
			if (unknown4b0<*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4F0_OFFSET))
			{
				*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4F0_OFFSET)= unknown4b0;
				*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4EC_OFFSET)= 0;
			}
			*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4A8_OFFSET)= 0;
			*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4A4_OFFSET)= 0;
			channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN4A1_OFFSET]= 0;
			channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN4A2_OFFSET]= 0;
		}
	}
	*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4F38_OFFSET)= now;
}

/* 0x0007a1c0: how the flow of a channel of the observer is, once the
interval has lasted as long as the number at 0x1A0 of what the observer has
at 0x10 says (1 before that). With a margin that is one of two numbers of
that, as the byte of 0x48E of the channel says: 2 if its number of 0x4B0 is
over the one of 0x504 and the margin; 0 if its bits a second of 0x4A8 are
over the number of 0x508 and the number of 0x4B0, less the one of 0x4B4, is
not over the one of 0x4F0 and the margin; and 1 otherwise, when, if it is
over, something has gone well once more (network_observer_channel_unknown_0007a160) */
long NETWORK_OBSERVER_CALLBACK network_observer_channel_unknown_0007a1c0(
	byte *observer,
	long channel_index)
{
	const byte *channel= network_observer_channel(observer, channel_index);
	const byte *parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
	long bits_per_second;
	unsigned long margin;
	unsigned long unknown4b0;
	boolean within;

	if (network_time_since(*(const long *)(observer+NETWORK_OBSERVER_UNKNOWN4F38_OFFSET))<
		*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN1A0_OFFSET))
	{
		return _network_observer_channel_flow1;
	}
	bits_per_second= network_observer_channel_unknown4a8_bits_per_second(channel_index, observer);
	parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
	margin= *(const unsigned long *)(parameters+(channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48E_OFFSET] ?
		NETWORK_OBSERVER_PARAMETERS_UNKNOWN1D8_OFFSET : NETWORK_OBSERVER_PARAMETERS_UNKNOWN1D4_OFFSET));
	unknown4b0= *(const unsigned long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4B0_OFFSET);
	/* the game adds and takes away as the processor does */
	within= (long)(unknown4b0-*(const unsigned long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4B4_OFFSET))<=
		(long)(*(const unsigned long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4F0_OFFSET)+margin);
	if ((long)unknown4b0>(long)(*(const unsigned long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN504_OFFSET)+margin))
	{
		return _network_observer_channel_flow2;
	}
	if (bits_per_second>*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN508_OFFSET) && within)
	{
		return _network_observer_channel_flow0;
	}
	if (!within)
	{
		network_observer_channel_unknown_0007a160(channel_index, observer);
	}

	return _network_observer_channel_flow1;
}

/* 0x00077f90: the observer is told three numbers. If the first is more than
nothing: whether the third is more than it times the real at 0x1AC of what
the observer has at 0x10 is which of its two counts goes up (0x4E14), from
now on if it was the other; and each time that it has been the same one for
as long as the number at 0x1B0 says, that count goes up. The second number:
the number of 0x4E08 is the most that it has been; and the one of 0x4E0C, if
it is one, goes up towards it by the part that the number at 0x1B4 says */
void NETWORK_OBSERVER_CALLBACK network_observer_unknown_00077f90(
	long unknown_eax,
	long unknown_ecx,
	byte *observer,
	long unknown4)
{
	const byte *parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
	long unknown4e0c;

	if (unknown_eax>0)
	{
		boolean second= (real)unknown4>(real)unknown_eax**(const real *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN1AC_OFFSET);

		if (*(const long *)(observer+NETWORK_OBSERVER_UNKNOWN4E10_OFFSET)==NONE ||
			observer[NETWORK_OBSERVER_UNKNOWN4E14_OFFSET]!=second)
		{
			observer[NETWORK_OBSERVER_UNKNOWN4E14_OFFSET]= second;
			*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4E10_OFFSET)= network_time_since(0);
		}
		if (network_time_since(*(const long *)(observer+NETWORK_OBSERVER_UNKNOWN4E10_OFFSET))>=
			*(const long *)(*(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET)+
			NETWORK_OBSERVER_PARAMETERS_UNKNOWN1B0_OFFSET))
		{
			++*(unsigned long *)(observer+NETWORK_OBSERVER_UNKNOWN4E18_OFFSET+
				observer[NETWORK_OBSERVER_UNKNOWN4E14_OFFSET]*sizeof(long));
			*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4E10_OFFSET)= network_time_since(0);
		}
	}
	if (unknown_ecx>*(const long *)(observer+NETWORK_OBSERVER_UNKNOWN4E08_OFFSET))
	{
		*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4E08_OFFSET)= unknown_ecx;
	}
	unknown4e0c= *(const long *)(observer+NETWORK_OBSERVER_UNKNOWN4E0C_OFFSET);
	if (unknown4e0c!=NONE && unknown_ecx>unknown4e0c)
	{
		parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
		/* the game adds and takes away as the processor does */
		*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4E0C_OFFSET)= (long)((unsigned long)
			((long)((unsigned long)unknown_ecx-(unsigned long)unknown4e0c)>>
			(*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN1B4_OFFSET)&31))+(unsigned long)unknown4e0c);
	}
}

#include "networking/game_voice.h"

/* 0x004ce12c, 0x004ce174: the least that the number of 0x4E04 of the
observer has to be for something that a player said to go from this console
to each number of others, for unknown_004c9884 and for unknown_004c9880 */
long unknown_004ce12c[MAXIMUM_NETWORK_OBSERVER_VOICE_CONSOLES+1];
long unknown_004ce174[MAXIMUM_NETWORK_OBSERVER_VOICE_CONSOLES+1];

/* 0x0007a740: to how many consoles something that a player said can go from
this one (unknown_004c9880, unknown_004c9884), if the game has to do with
the voice: for each, the most of them that the number of 0x4E04 of the
observer is enough for, by its table; none if it is not enough for one */
void NETWORK_OBSERVER_CALLBACK network_observer_update_voice_console_counts(
	const byte *observer)
{
	long unknown4e04= *(const long *)(observer+NETWORK_OBSERVER_UNKNOWN4E04_OFFSET);
	long first= 0;
	long second= 0;
	long count;

	for (count= MAXIMUM_NETWORK_OBSERVER_VOICE_CONSOLES; count>0; count--)
	{
		if (unknown4e04>=unknown_004ce174[count])
		{
			first= count;
			break;
		}
	}
	for (count= MAXIMUM_NETWORK_OBSERVER_VOICE_CONSOLES; count>0; count--)
	{
		if (unknown4e04>=unknown_004ce12c[count])
		{
			second= count;
			break;
		}
	}
	if (game_voice_enabled)
	{
		unknown_004c9880= first;
		unknown_004c9884= second;
	}
}

#include "shell/shell.h"

/* 0x004e6398: a number of 64 bits of another part of the game, which seems
to go up with each picture that is shown */
long long unknown_004e6398;

/* the count of something by spans of time starts over, for the time that it
was for */
static inline __attribute__((always_inline)) void network_rate_reset(
	struct network_rate *rate)
{
	rate->total_count= 0;
	rate->total_size= 0;
	rate->start= 0;
	rate->count= 0;
	rate->size= 0;
	rate->index= 0;
	memset(rate->spans, 0, sizeof(rate->spans));
	rate->spans_count= 0;
	rate->spans_size= 0;
}

/* what each session that uses a channel of the observer has in it is told
that the channel has what its channel of the game is of, or has it no more */
static inline __attribute__((always_inline)) void network_observer_channel_tell_established(
	byte *observer,
	long channel_index,
	boolean established)
{
	const byte *channel= network_observer_channel(observer, channel_index);
	long session_index;

	for (session_index= 0; session_index<MAXIMUM_NETWORK_OBSERVER_SESSIONS; session_index++)
	{
		if (channel[NETWORK_OBSERVER_CHANNEL_SESSIONS_OFFSET]&(1u<<session_index))
		{
			struct network_observer_session_owner *owner= *(struct network_observer_session_owner **)(observer+
				NETWORK_OBSERVER_SESSIONS_OFFSET+session_index*NETWORK_OBSERVER_SESSION_SIZE+
				NETWORK_OBSERVER_SESSION_SESSION_OFFSET);

			owner->functions->channel_established(owner, channel_index,
				*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN10_OFFSET), established);
		}
	}
}

/* 0x00076ff0: a channel of the observer that is in use follows the state of
its channel of the game. With none, it has no address (noted as failed, with
0) and goes back to its first state. In its sixth or eighth state: with the
channel of the game in its fifth, it goes into the seventh; with it in its
second or before, what the channel of the game has at 0x58 says: 1, 2 or 5,
no address (0xF) and the first state; 9, no address (0) and the second
state, with no tries, and then as with the rest: one more try, in the fifth
state from the sixth, and in the ninth if not. In its seventh state: with the
channel of the game past its second and not in its fifth, the eighth; with
it in its second or before, the ninth, with no tries.

Then, if the sessions were told that the channel had what its channel of the
game is of (the second bit of its byte of 0x8), and it is not so any more,
they are told, and it has none. And a channel in its seventh state that they
were not told of starts over all that it counts, keeps what its channel of
the game is of, and they are told */
void NETWORK_OBSERVER_CALLBACK network_observer_channel_update_game_channel(
	long channel_index,
	byte *observer)
{
	byte *channel= (byte *)network_observer_channel(observer, channel_index);
	long state= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET);
	long game_channel_index= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_CHANNEL_OFFSET);
	const byte *game_channel= NULL;
	long refresh_rate;

	if (state==0)
	{
		return;
	}
	if (game_channel_index==NONE)
	{
		network_observer_channel_forget_address(channel_index, observer, TRUE, 0);
		network_observer_channel_set_state(_network_observer_channel_state1, observer, channel_index);
	}
	else
	{
		game_channel= unknown_004d87d4+game_channel_index*NETWORK_CHANNEL_SIZE;
		if (state==_network_observer_channel_state6 || state==_network_observer_channel_state8)
		{
			long game_state= *(const long *)(game_channel+NETWORK_CHANNEL_STATE_OFFSET);

			if (game_state==_network_channel_state5)
			{
				network_observer_channel_set_state(_network_observer_channel_state7, observer, channel_index);
			}
			else if (game_state<=_network_channel_state2)
			{
				switch (*(const long *)(game_channel+NETWORK_CHANNEL_UNKNOWN58_OFFSET))
				{
				case 1:
				case 2:
				case 5:
					network_observer_channel_forget_address(channel_index, observer, TRUE, NETWORK_OBSERVER_CHANNEL_REFUSED_REASON);
					network_observer_channel_set_state(_network_observer_channel_state1, observer, channel_index);
					break;
				case 9:
					network_observer_channel_forget_address(channel_index, observer, TRUE, 0);
					network_observer_channel_set_state(_network_observer_channel_state2, observer, channel_index);
					*(short *)(channel+NETWORK_OBSERVER_CHANNEL_TRIES_OFFSET)= 0;
					/* and then as with the rest */
				default:
					network_observer_channel_set_state(
						*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)==_network_observer_channel_state6 ?
						_network_observer_channel_state5 : _network_observer_channel_state9, observer, channel_index);
					(*(short *)(channel+NETWORK_OBSERVER_CHANNEL_TRIES_OFFSET))++;
					break;
				}
			}
		}
		else if (state==_network_observer_channel_state7)
		{
			long game_state= *(const long *)(game_channel+NETWORK_CHANNEL_STATE_OFFSET);

			if (game_state!=_network_channel_state5)
			{
				if (game_state>_network_channel_state2)
				{
					network_observer_channel_set_state(_network_observer_channel_state8, observer, channel_index);
				}
				else
				{
					network_observer_channel_set_state(_network_observer_channel_state9, observer, channel_index);
					*(short *)(channel+NETWORK_OBSERVER_CHANNEL_TRIES_OFFSET)= 0;
				}
			}
		}
	}
	if ((channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN8_OFFSET]&_network_observer_channel_established_bit) &&
		!(game_channel && *(const long *)(game_channel+NETWORK_CHANNEL_STATE_OFFSET)==_network_channel_state5 &&
		*(const long *)(game_channel+NETWORK_CHANNEL_UNKNOWN4C_OFFSET)==
		*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN10_OFFSET)))
	{
		network_observer_channel_tell_established(observer, channel_index, FALSE);
		channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN8_OFFSET]&= ~_network_observer_channel_established_bit;
		*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN10_OFFSET)= NONE;
	}
	if ((channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN8_OFFSET]&_network_observer_channel_established_bit) ||
		*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)!=_network_observer_channel_state7)
	{
		return;
	}
	*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN94_OFFSET)= network_time_since(0);
	network_rate_reset((struct network_rate *)(channel+NETWORK_OBSERVER_CHANNEL_RATEA0_OFFSET));
	network_rate_reset((struct network_rate *)(channel+NETWORK_OBSERVER_CHANNEL_RATE_OFFSET));
	network_window_reset((struct network_window *)(channel+NETWORK_OBSERVER_CHANNEL_WINDOW250_OFFSET));
	network_window_reset((struct network_window *)(channel+NETWORK_OBSERVER_CHANNEL_WINDOW360_OFFSET));
	*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN480_OFFSET)= NONE;
	*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN488_OFFSET)= NONE;
	*(real *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN484_OFFSET)= -1.0f;
	*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN470_OFFSET)= network_time_since(NETWORK_RATE_MILLISECONDS_PER_SECOND);
	refresh_rate= xbox_video_settings.refresh_rate;
	if (refresh_rate<=0)
	{
		refresh_rate= NETWORK_OBSERVER_DEFAULT_REFRESH_RATE;
	}
	*(long long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN478_OFFSET)= (long long)((unsigned long long)unknown_004e6398-
		(unsigned long long)refresh_rate);
	channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN8_OFFSET]|= _network_observer_channel_established_bit|_network_observer_channel_unknown8_bit4;
	*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN10_OFFSET)= *(const long *)(game_channel+NETWORK_CHANNEL_UNKNOWN4C_OFFSET);
	network_observer_channel_tell_established(observer, channel_index, TRUE);
}

/* 0x00088220, of the channels of the game: one of them starts to connect to
an address of the game, with a byte */
void __attribute__((stdcall)) unknown_00088220(const struct transport_address *address, byte *game_channel, boolean unknown);

/* 0x000776a0: a channel of the observer that has a channel of the game
tries again when it is time to. In its first state for as long as the number
at 0x6C of what the observer has at 0x10 says, it goes into the second, with
no tries, no address and no sessions that failed, and all that is done with
a channel each time is done once more. In its fourth state, if it was asked
to connect, it goes into the fifth, with no tries.

Then, in its second, fifth or ninth state, it has so many tries, each so
long after it went into the state: the ones to get an address in the second,
the ones to connect in the others, and others again if it has had what its
channel of the game is of. After the first try, or if it has had that, a
session that uses it has to want it still (the third function of what each
has in the observer): if none does, once the channel has been unused for long
enough (network_observer_channel_unused_expired) it has no address and goes
back to its first state. With no tries left, the same. When it is time for
the try: in the second state it looks for an address
(network_observer_channel_find_address) and goes into the third state with
one, and into the first with none; in the others its channel of the game
starts to connect to its address, and it goes into the sixth state from the
fifth, and into the eighth from the ninth */
void NETWORK_OBSERVER_CALLBACK network_observer_channel_update_tries(
	byte *observer,
	long channel_index)
{
	byte *channel= (byte *)network_observer_channel(observer, channel_index);
	long game_channel_index= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_CHANNEL_OFFSET);
	const byte *parameters;
	const struct network_observer_tries *tries;
	byte *game_channel;
	long state;
	long try;

	if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)==0 || game_channel_index==NONE)
	{
		return;
	}
	game_channel= unknown_004d87d4+game_channel_index*NETWORK_CHANNEL_SIZE;
	parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
	if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)==_network_observer_channel_state1 &&
		network_time_since(*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_TIME_OFFSET))>=
		*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN6C_OFFSET))
	{
		network_observer_channel_set_state(_network_observer_channel_state2, observer, channel_index);
		*(short *)(channel+NETWORK_OBSERVER_CHANNEL_TRIES_OFFSET)= 0;
		*(long *)(channel+NETWORK_OBSERVER_CHANNEL_FAILED_SESSIONS_OFFSET)= 0;
		*(long *)(channel+NETWORK_OBSERVER_CHANNEL_SESSION_OFFSET)= NONE;
		network_observer_channel_forget_address(channel_index, observer, FALSE, 0);
		network_observer_channel_update_connection(channel_index, observer);
		network_observer_channel_update_game_channel(channel_index, observer);
		network_observer_channel_update_tries(observer, channel_index);
	}
	if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)==_network_observer_channel_state4 &&
		(channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN8_OFFSET]&_network_observer_channel_connect_bit))
	{
		network_observer_channel_set_state(_network_observer_channel_state5, observer, channel_index);
		*(short *)(channel+NETWORK_OBSERVER_CHANNEL_TRIES_OFFSET)= 0;
	}
	state= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET);
	if (state!=_network_observer_channel_state2 && state!=_network_observer_channel_state5 &&
		state!=_network_observer_channel_state9)
	{
		return;
	}
	parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
	if (state==_network_observer_channel_state2)
	{
		tries= (const struct network_observer_tries *)(parameters+NETWORK_OBSERVER_PARAMETERS_ADDRESS_TRIES_OFFSET);
	}
	else if (!(channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN8_OFFSET]&_network_observer_channel_unknown8_bit4))
	{
		tries= (const struct network_observer_tries *)(parameters+NETWORK_OBSERVER_PARAMETERS_CONNECT_TRIES_OFFSET);
	}
	else
	{
		tries= (const struct network_observer_tries *)(parameters+NETWORK_OBSERVER_PARAMETERS_RECONNECT_TRIES_OFFSET);
	}
	if (*(const short *)(channel+NETWORK_OBSERVER_CHANNEL_TRIES_OFFSET)!=0 ||
		(channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN8_OFFSET]&_network_observer_channel_unknown8_bit4))
	{
		boolean wanted= FALSE;
		long session_index;

		for (session_index= 0; session_index<MAXIMUM_NETWORK_OBSERVER_SESSIONS; session_index++)
		{
			if (channel[NETWORK_OBSERVER_CHANNEL_SESSIONS_OFFSET]&(1u<<session_index))
			{
				struct network_observer_session_owner *owner= *(struct network_observer_session_owner **)(observer+
					NETWORK_OBSERVER_SESSIONS_OFFSET+session_index*NETWORK_OBSERVER_SESSION_SIZE+
					NETWORK_OBSERVER_SESSION_SESSION_OFFSET);

				if (owner->functions->channel_wanted(owner, channel_index,
					(channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN8_OFFSET]&_network_observer_channel_unknown8_bit4)!=0))
				{
					wanted= TRUE;
				}
			}
		}
		if (!wanted)
		{
			if (network_observer_channel_unused_expired(channel_index, observer, 0))
			{
				network_observer_channel_forget_address(channel_index, observer, FALSE, 0);
				network_observer_channel_set_state(_network_observer_channel_state1, observer, channel_index);
			}

			return;
		}
	}
	*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNUSED_TIME_OFFSET)= 0;
	*(long *)(channel+NETWORK_OBSERVER_CHANNEL_WAITING_TIME_OFFSET)= 0;
	try= *(const short *)(channel+NETWORK_OBSERVER_CHANNEL_TRIES_OFFSET);
	if (try<0 || try>=tries->count)
	{
		network_observer_channel_forget_address(channel_index, observer, FALSE, 0);
		network_observer_channel_set_state(_network_observer_channel_state1, observer, channel_index);

		return;
	}
	if (network_time_since(*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_TIME_OFFSET))<tries->delays[try])
	{
		return;
	}
	if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)==_network_observer_channel_state2)
	{
		network_observer_channel_set_state(network_observer_channel_find_address(channel_index, observer) ?
			_network_observer_channel_state3 : _network_observer_channel_state1, observer, channel_index);

		return;
	}
	unknown_00088220((const struct transport_address *)(channel+NETWORK_OBSERVER_CHANNEL_ADDRESS_OFFSET), game_channel, TRUE);
	network_observer_channel_set_state(
		*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)==_network_observer_channel_state5 ?
		_network_observer_channel_state6 : _network_observer_channel_state8, observer, channel_index);
}

/* how many times a second the game goes: 25 with the picture at 50 a second,
and 30 if not */
static inline __attribute__((always_inline)) real network_observer_tick_rate(
	void)
{
	return xbox_video_settings.refresh_rate==NETWORK_OBSERVER_PAL_REFRESH_RATE ?
		NETWORK_OBSERVER_PAL_TICK_RATE : NETWORK_OBSERVER_TICK_RATE;
}

/* 0x00078190: whether so many packets a second are few: with a byte, fewer
than the part of the times a second that the game goes that the real at
0x110 of what the observer has at 0x10 says; and with another, fewer than
those times, or than half of them with a third byte */
boolean NETWORK_OBSERVER_CALLBACK network_observer_packet_rate_low(
	boolean unknown,
	real packet_rate,
	const byte *observer,
	boolean unknown4,
	boolean half)
{
	real tick_rate= network_observer_tick_rate();
	real rate= packet_rate+NETWORK_OBSERVER_PACKET_RATE_EPSILON;
	boolean low= FALSE;

	if (unknown && *(const real *)(*(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET)+
		NETWORK_OBSERVER_PARAMETERS_UNKNOWN110_OFFSET)*tick_rate>rate)
	{
		low= TRUE;
	}
	if (unknown4)
	{
		if (half)
		{
			tick_rate*= 0.5f;
		}
		if (tick_rate>rate)
		{
			low= TRUE;
		}
	}

	return low;
}

/* 0x00078090: how many packets a second so many bits a second are, with
packets of one of the two sizes of what the observer has at 0x10 (in bytes,
and 0x168 bits more each): the first of the rates of that, which are parts
of the times a second that the game goes, that they are enough for, and the
last one if they are enough for none or it is none; and with a byte, no more
than half of those times */
real NETWORK_OBSERVER_CALLBACK network_observer_packet_rate(
	const byte *observer,
	long bits_per_second,
	boolean unknown4,
	boolean half)
{
	const byte *parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
	real tick_rate= network_observer_tick_rate();
	/* the game adds and multiplies as the processor does */
	long packet_bits= (long)(*(const unsigned long *)(parameters+(unknown4 ?
		NETWORK_OBSERVER_PARAMETERS_UNKNOWN10C_OFFSET : NETWORK_OBSERVER_PARAMETERS_UNKNOWN108_OFFSET))*8+
		NETWORK_OBSERVER_PACKET_HEADER_BITS);
	real packets= (real)bits_per_second/(real)packet_bits;
	long count= *(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_PACKET_RATE_COUNT_OFFSET);
	const real *rates= (const real *)(parameters+NETWORK_OBSERVER_PARAMETERS_PACKET_RATES_OFFSET);
	real rate= 0.0f;
	boolean found= FALSE;
	long index;

	for (index= 0; index<count-1; index++)
	{
		rate= rates[index]*tick_rate;
		if (packets>=rate)
		{
			found= rate!=0.0f;
			break;
		}
	}
	if (!found)
	{
		rate= rates[count-1]*tick_rate;
	}
	if (half)
	{
		real half_rate= tick_rate*0.5f;

		if (rate>half_rate)
		{
			rate= half_rate;
		}
	}

	return rate;
}

/* 0x00079600: the flow of a channel of the observer is so many packets a
second, so many bits a second and one more number; and its byte of 0x4A0 is
whether those packets are few (network_observer_packet_rate_low, with its
bytes of 0x48D, 0x48E and 0x490) */
void NETWORK_OBSERVER_CALLBACK network_observer_channel_set_flow(
	long channel_index,
	byte *observer,
	real packet_rate,
	long bits_per_second,
	long unknown498)
{
	byte *channel= (byte *)network_observer_channel(observer, channel_index);

	channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN4A0_OFFSET]= network_observer_packet_rate_low(
		channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48D_OFFSET], packet_rate, observer,
		channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48E_OFFSET], channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN490_OFFSET]);
	*(long *)(channel+NETWORK_OBSERVER_CHANNEL_BITS_PER_SECOND_OFFSET)= bits_per_second;
	*(real *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN49C_OFFSET)= packet_rate;
	*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN498_OFFSET)= unknown498;
}

/* 0x0007a110: a channel of the observer that has a flow kept (its byte of
0x4F4) has that flow again, and none kept; and its number of 0x4DC is 1 */
void network_observer_channel_restore_flow(
	long channel_index,
	byte *observer)
{
	byte *channel= (byte *)network_observer_channel(observer, channel_index);

	if (channel[NETWORK_OBSERVER_CHANNEL_KEPT_FLOW_OFFSET])
	{
		network_observer_channel_set_flow(channel_index, observer,
			*(const real *)(channel+NETWORK_OBSERVER_CHANNEL_KEPT_PACKET_RATE_OFFSET),
			*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_KEPT_BITS_PER_SECOND_OFFSET),
			*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_KEPT_UNKNOWN498_OFFSET));
		channel[NETWORK_OBSERVER_CHANNEL_KEPT_FLOW_OFFSET]= FALSE;
		*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4DC_OFFSET)= 1;
	}
}

/* 0x0007a2a0: a real of a channel of the observer whose number of 0x4DC is
1, and nothing of any other: how long ago its time of 0x4E8 was, times the
number at 0x1CC of what the observer has at 0x10, and no more than the real
at 0x1C8, which it is with no such time; less its bits a second times the
real at 0x1D0 */
real network_observer_channel_unknown_0007a2a0(
	long channel_index,
	const byte *observer)
{
	const byte *channel= network_observer_channel(observer, channel_index);
	const byte *parameters;
	long time;
	real result;

	if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4DC_OFFSET)!=1)
	{
		return 0.0f;
	}
	time= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4E8_OFFSET);
	if (time!=NONE)
	{
		unsigned long elapsed= (unsigned long)network_time_since(time);

		parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
		/* the game multiplies as the processor does */
		result= (real)(long)(elapsed**(const unsigned long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN1CC_OFFSET));
		if (result>*(const real *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN1C8_OFFSET))
		{
			result= *(const real *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN1C8_OFFSET);
		}
	}
	else
	{
		parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
		result= *(const real *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN1C8_OFFSET);
	}

	return result-(real)*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_BITS_PER_SECOND_OFFSET)*
		*(const real *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN1D0_OFFSET);
}

/* a number goes towards another by the part of the way that a number of
what the observer has at 0x10 says. The game adds and takes away as the
processor does */
static inline __attribute__((always_inline)) long network_observer_approach(
	long from,
	long to,
	long shift)
{
	return (long)((unsigned long)((long)((unsigned long)to-(unsigned long)from)>>(shift&31))+(unsigned long)from);
}

/* 0x00079660: a channel of the observer whose byte of 0x48C is set is told
of something of a size, with two numbers. Its bytes of 0x4A8 have the size
more; its number of 0x4AC is the second number; its number of 0x4B0 goes
towards the first, and the one of 0x4B4 towards how long ago this was last
done (0x4E4), which is now. Then, with a flow held (its byte of 0x4C0), it
has that flow again once the flow has been held for the number at 0x17C of
what the observer has at 0x10 times the number of 0x4B0, or the one that it
was when the flow was held if that is more; and with none held, its number
of 0x4D8 goes down by one, if it is more than nothing */
void NETWORK_OBSERVER_CALLBACK network_observer_channel_unknown_00079660(
	long unknown,
	long size,
	byte *observer,
	long channel_index,
	long unknown96c)
{
	byte *channel= (byte *)network_observer_channel(observer, channel_index);
	long last= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4E4_OFFSET);
	const byte *parameters;
	long elapsed= 0;
	long unknown4b0;

	*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4A8_OFFSET)=
		(long)(*(const unsigned long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4A8_OFFSET)+(unsigned long)size);
	*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4AC_OFFSET)= unknown96c;
	if (last!=NONE)
	{
		elapsed= network_time_since(last);
	}
	*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4E4_OFFSET)= network_time_since(0);
	parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
	unknown4b0= network_observer_approach(*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4B0_OFFSET), unknown,
		*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN170_OFFSET));
	*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4B0_OFFSET)= unknown4b0;
	*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4B4_OFFSET)= network_observer_approach(
		*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4B4_OFFSET), elapsed,
		*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN170_OFFSET));
	if (channel[NETWORK_OBSERVER_CHANNEL_HELD_FLOW_OFFSET])
	{
		long held_unknown4b0= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_HELD_UNKNOWN4B0_OFFSET);

		if (unknown4b0<=held_unknown4b0)
		{
			unknown4b0= held_unknown4b0;
		}
		/* the game multiplies as the processor does */
		if (network_time_since(*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_HELD_TIME_OFFSET))>=
			(long)(*(const unsigned long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN17C_OFFSET)*(unsigned long)unknown4b0))
		{
			network_observer_channel_set_flow(channel_index, observer,
				*(const real *)(channel+NETWORK_OBSERVER_CHANNEL_HELD_PACKET_RATE_OFFSET),
				*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_HELD_BITS_PER_SECOND_OFFSET),
				*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_HELD_UNKNOWN498_OFFSET));
			channel[NETWORK_OBSERVER_CHANNEL_HELD_FLOW_OFFSET]= FALSE;
		}
	}
	else if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4D8_OFFSET)>0)
	{
		(*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4D8_OFFSET))--;
	}
}

/* 0x00079de0: something that is tried with the flow of a channel of the
observer: whether it was, and a byte */
boolean __attribute__((stdcall)) network_observer_channel_raise_flow(byte *observer, long channel_index, boolean *unknown);

/* 0x0007a330: what is done with the flow of a channel of the observer each
time, by its number of 0x4DC. With 0: once it has been so for the number at
0x1F0 of what the observer has at 0x10, and the intervals in a row with
something noted (0x518) are as many as the number at 0x1EC, it is 1, and the
byte of 0x4F3C of the observer is set. With any other, after an interval
with nothing noted, it is 0 again (network_observer_channel_reset_unknown4dc).
With 2, as network_observer_channel_unknown_0007a1c0 says: with 0, it is 3;
with 1, the flow that was kept is back (network_observer_channel_restore_flow);
with 2, network_observer_channel_lower_flow with 1. With the others, if no flow is held and the
number of 0x4D8 is nothing: when the number of 0x4B0, less the one of 0x4B4,
is over the one of 0x4F0 and the margin, something has gone well once more
(network_observer_channel_unknown_0007a160); if not, network_observer_channel_raise_flow is
tried, and when it was not and gives a byte, it is 0 again */
void NETWORK_OBSERVER_CALLBACK network_observer_channel_update_flow(
	byte *observer,
	long channel_index)
{
	byte *channel= (byte *)network_observer_channel(observer, channel_index);
	const byte *parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
	long phase= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4DC_OFFSET);
	unsigned long margin;
	boolean unknown;

	if (phase==0)
	{
		long since= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4E0_OFFSET);

		if (since!=NONE &&
			network_time_since(since)<*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN1F0_OFFSET))
		{
			return;
		}
		if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN518_OFFSET)<
			*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN1EC_OFFSET))
		{
			return;
		}
		*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4DC_OFFSET)= 1;
		observer[NETWORK_OBSERVER_UNKNOWN4F3C_OFFSET]= TRUE;

		return;
	}
	if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN51C_OFFSET)>0)
	{
		network_observer_channel_reset_unknown4dc(channel_index, observer);

		return;
	}
	if (phase==2)
	{
		switch (network_observer_channel_unknown_0007a1c0(observer, channel_index))
		{
		case _network_observer_channel_flow0:
			*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4DC_OFFSET)= 3;
			break;
		case _network_observer_channel_flow1:
			network_observer_channel_restore_flow(channel_index, observer);
			break;
		default:
			network_observer_channel_lower_flow(observer, channel_index, TRUE);
			break;
		}

		return;
	}
	margin= *(const unsigned long *)(parameters+(channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48E_OFFSET] ?
		NETWORK_OBSERVER_PARAMETERS_UNKNOWN1D8_OFFSET : NETWORK_OBSERVER_PARAMETERS_UNKNOWN1D4_OFFSET));
	unknown= FALSE;
	if (channel[NETWORK_OBSERVER_CHANNEL_HELD_FLOW_OFFSET] ||
		*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4D8_OFFSET)>0)
	{
		return;
	}
	/* the game adds and takes away as the processor does */
	if ((long)(*(const unsigned long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4B0_OFFSET)-
		*(const unsigned long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4B4_OFFSET))>
		(long)(*(const unsigned long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4F0_OFFSET)+margin))
	{
		network_observer_channel_unknown_0007a160(channel_index, observer);

		return;
	}
	if (!network_observer_channel_raise_flow(observer, channel_index, &unknown) && unknown)
	{
		network_observer_channel_reset_unknown4dc(channel_index, observer);
	}
}

/* 0x00079a10: the channel of the observer that has the most of something
has its flow changed (network_observer_channel_lower_flow, with nothing), since now, if it was
not done in the time that the number at 0x1BC of what the observer has at
0x10 says. What counts of a channel is the more of its numbers of 0x510 and
0x514; the one that is given counts with the number at 0x1B8 more, and only
a channel that is in use and has its byte of 0x48C, and more than it, is
looked at. Nothing is done if that channel has its byte of 0x48E and the one
that is given does not */
void NETWORK_OBSERVER_CALLBACK network_observer_channel_unknown_00079a10(
	long channel_index,
	byte *observer)
{
	const byte *channel= network_observer_channel(observer, channel_index);
	long since= *(const long *)(observer+NETWORK_OBSERVER_UNKNOWN4F34_OFFSET);
	const byte *parameters;
	long most;
	long most_index= NONE;
	byte most_unknown48e;
	long other_index;

	if (since!=NONE)
	{
		long elapsed= network_time_since(since);

		if (elapsed!=NONE &&
			elapsed<*(const long *)(*(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET)+
			NETWORK_OBSERVER_PARAMETERS_UNKNOWN1BC_OFFSET))
		{
			return;
		}
	}
	parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
	most= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN514_OFFSET);
	if (most<=*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN510_OFFSET))
	{
		most= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN510_OFFSET);
	}
	/* the game adds as the processor does */
	most= (long)((unsigned long)most+*(const unsigned long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN1B8_OFFSET));
	most_unknown48e= channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48E_OFFSET];
	for (other_index= 0; other_index<MAXIMUM_NETWORK_OBSERVER_CHANNELS; other_index++)
	{
		const byte *other= network_observer_channel(observer, other_index);

		if (*(const long *)(other+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)!=0 &&
			other[NETWORK_OBSERVER_CHANNEL_UNKNOWN48C_OFFSET])
		{
			long count= *(const long *)(other+NETWORK_OBSERVER_CHANNEL_UNKNOWN514_OFFSET);

			if (count<=*(const long *)(other+NETWORK_OBSERVER_CHANNEL_UNKNOWN510_OFFSET))
			{
				count= *(const long *)(other+NETWORK_OBSERVER_CHANNEL_UNKNOWN510_OFFSET);
			}
			if (count>most)
			{
				most_unknown48e= other[NETWORK_OBSERVER_CHANNEL_UNKNOWN48E_OFFSET];
				most_index= other_index;
				most= count;
			}
		}
	}
	if (most_index==NONE || (most_unknown48e && !channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48E_OFFSET]))
	{
		return;
	}
	network_observer_channel_lower_flow(observer, most_index, FALSE);
	*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4F34_OFFSET)= network_time_since(0);
}

/* a real as the whole number that it is nearest to: the game rounds with
the old processor's instructions, in place */
long lrintf(float value);

/* 0x00078150: how many bits a second so many packets a second are, with
packets of one of the two sizes of what the observer has at 0x10 (in bytes,
and 0x168 bits more each) */
long NETWORK_OBSERVER_CALLBACK network_observer_packet_rate_bits_per_second(
	const byte *observer,
	boolean unknown,
	real packet_rate)
{
	const byte *parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
	/* the game adds and multiplies as the processor does */
	long packet_bits= (long)(*(const unsigned long *)(parameters+(unknown ?
		NETWORK_OBSERVER_PARAMETERS_UNKNOWN10C_OFFSET : NETWORK_OBSERVER_PARAMETERS_UNKNOWN108_OFFSET))*8+
		NETWORK_OBSERVER_PACKET_HEADER_BITS);

	return lrintf((real)packet_bits*packet_rate);
}

/* 0x00079c00: the flow of a channel of the observer goes down for good.
With a byte, the channel that has the most of something has its own changed
first (network_observer_channel_unknown_00079a10); a flow that was kept is
back (network_observer_channel_restore_flow); and nothing has gone well any
time. Its bits a second, or the ones of the flow that is held if one is, go
down by the part that the real at 0x198 of what the observer has at 0x10
says, and by no more than the number at 0x194, and are no less than the
number at 0x14C; its number of 0x498 goes down as much, and is no less than
the number at 0xE0; and its packets a second are what those bits a second
give (network_observer_packet_rate). With a flow held, that is the flow that
is held from now on, and the one that the channel has is no more than it in
any of the three; with none, it is the flow of the channel. Three bytes of
the observer are set, the third only with the byte; and the number of 0x4DC
of the channel is none of them (network_observer_channel_reset_unknown4dc) */
void NETWORK_OBSERVER_CALLBACK network_observer_channel_lower_flow(
	byte *observer,
	long channel_index,
	boolean unknown)
{
	byte *channel= (byte *)network_observer_channel(observer, channel_index);
	const byte *parameters;
	long bits_per_second;
	long unknown498;
	long less;
	long lower_bits_per_second;
	long lower_unknown498;
	real packet_rate;

	if (unknown)
	{
		network_observer_channel_unknown_00079a10(channel_index, observer);
	}
	if (channel[NETWORK_OBSERVER_CHANNEL_KEPT_FLOW_OFFSET])
	{
		network_observer_channel_restore_flow(channel_index, observer);
	}
	*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4EC_OFFSET)= 0;
	if (channel[NETWORK_OBSERVER_CHANNEL_HELD_FLOW_OFFSET])
	{
		bits_per_second= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_HELD_BITS_PER_SECOND_OFFSET);
		unknown498= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_HELD_UNKNOWN498_OFFSET);
	}
	else
	{
		bits_per_second= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_BITS_PER_SECOND_OFFSET);
		unknown498= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN498_OFFSET);
	}
	parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
	less= lrintf((real)bits_per_second**(const real *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN198_OFFSET));
	if (less>*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN194_OFFSET))
	{
		less= *(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN194_OFFSET);
	}
	/* the game takes away and multiplies as the processor does */
	lower_bits_per_second= (long)((unsigned long)bits_per_second-(unsigned long)less);
	if (lower_bits_per_second<=*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN14C_OFFSET))
	{
		lower_bits_per_second= *(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN14C_OFFSET);
	}
	lower_unknown498= (long)((unsigned long)lower_bits_per_second*(unsigned long)unknown498)/bits_per_second;
	if (lower_unknown498<=*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWNE0_OFFSET))
	{
		lower_unknown498= *(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWNE0_OFFSET);
	}
	packet_rate= network_observer_packet_rate(observer, lower_bits_per_second,
		channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48E_OFFSET], channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN490_OFFSET]);
	if (channel[NETWORK_OBSERVER_CHANNEL_HELD_FLOW_OFFSET])
	{
		long now_bits_per_second= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_BITS_PER_SECOND_OFFSET);
		long now_unknown498= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN498_OFFSET);
		real now_packet_rate= *(const real *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN49C_OFFSET);

		if (now_bits_per_second>lower_bits_per_second)
		{
			now_bits_per_second= lower_bits_per_second;
		}
		if (now_unknown498>lower_unknown498)
		{
			now_unknown498= lower_unknown498;
		}
		if (now_packet_rate>packet_rate)
		{
			now_packet_rate= packet_rate;
		}
		/* the game gives the packets a second without what they have after
		the point */
		network_observer_channel_set_flow(channel_index, observer, (real)(long)now_packet_rate, now_bits_per_second,
			now_unknown498);
		*(long *)(channel+NETWORK_OBSERVER_CHANNEL_HELD_BITS_PER_SECOND_OFFSET)= lower_bits_per_second;
		*(long *)(channel+NETWORK_OBSERVER_CHANNEL_HELD_UNKNOWN498_OFFSET)= lower_unknown498;
		*(real *)(channel+NETWORK_OBSERVER_CHANNEL_HELD_PACKET_RATE_OFFSET)= packet_rate;
	}
	else
	{
		network_observer_channel_set_flow(channel_index, observer, packet_rate, lower_bits_per_second, lower_unknown498);
	}
	observer[NETWORK_OBSERVER_UNKNOWN4F3C_OFFSET]= TRUE;
	observer[NETWORK_OBSERVER_UNKNOWN4F3D_OFFSET]= TRUE;
	if (unknown)
	{
		observer[NETWORK_OBSERVER_UNKNOWN4F3E_OFFSET]= TRUE;
	}
	network_observer_channel_reset_unknown4dc(channel_index, observer);
}

/* 0x00079850: the flow of a channel of the observer drops for a while, when
too many of the last things that it was told of were not so. The flow that
it has is held, since now, with what its number of 0x4B0 is; in the third or
fourth phase of its flow (0x4DC), the flow that was kept is back first
(network_observer_channel_restore_flow). Its bits a second and its number of
0x498 are the part of what they were that the real at 0x180 of what the
observer has at 0x10 says, and no less than the numbers at 0x14C and 0xE0;
its packets a second are what those bits a second give
(network_observer_packet_rate); and the number of 0x498 is no more than what
goes in so many packets as there are in the milliseconds of its number of
0x4F0, and two more. Its number of 0x4D8 goes up by the number at 0x184, and
when it is the one at 0x188 or more, the flow goes down for good
(network_observer_channel_lower_flow, with nothing) and it is nothing */
void NETWORK_OBSERVER_CALLBACK network_observer_channel_drop_flow(
	byte *observer,
	long channel_index)
{
	byte *channel= (byte *)network_observer_channel(observer, channel_index);
	long bits_per_second= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_BITS_PER_SECOND_OFFSET);
	long unknown498= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN498_OFFSET);
	const byte *parameters;
	long now;
	long phase;
	long unknown4f0;
	long packets;
	long most;
	long unknown4d8;
	real packet_rate;

	*(real *)(channel+NETWORK_OBSERVER_CHANNEL_HELD_PACKET_RATE_OFFSET)=
		*(const real *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN49C_OFFSET);
	*(long *)(channel+NETWORK_OBSERVER_CHANNEL_HELD_BITS_PER_SECOND_OFFSET)= bits_per_second;
	channel[NETWORK_OBSERVER_CHANNEL_HELD_FLOW_OFFSET]= TRUE;
	*(long *)(channel+NETWORK_OBSERVER_CHANNEL_HELD_UNKNOWN498_OFFSET)= unknown498;
	*(long *)(channel+NETWORK_OBSERVER_CHANNEL_HELD_UNKNOWN4B0_OFFSET)=
		*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4B0_OFFSET);
	now= network_time_since(0);
	*(long *)(channel+NETWORK_OBSERVER_CHANNEL_HELD_TIME_OFFSET)= now;
	*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4E0_OFFSET)= now;
	phase= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4DC_OFFSET);
	if (phase==2 || phase==3)
	{
		network_observer_channel_restore_flow(channel_index, observer);
	}
	parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
	bits_per_second= lrintf((real)*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_BITS_PER_SECOND_OFFSET)*
		*(const real *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN180_OFFSET));
	unknown498= lrintf((real)*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN498_OFFSET)*
		*(const real *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN180_OFFSET));
	if (bits_per_second<=*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN14C_OFFSET))
	{
		bits_per_second= *(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN14C_OFFSET);
	}
	if (unknown498<=*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWNE0_OFFSET))
	{
		unknown498= *(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWNE0_OFFSET);
	}
	packet_rate= network_observer_packet_rate(observer, bits_per_second,
		channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48E_OFFSET], channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN490_OFFSET]);
	unknown4f0= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4F0_OFFSET);
	packets= (long)((unsigned long)(long)((real)unknown4f0*packet_rate*NETWORK_OBSERVER_SECONDS_PER_MILLISECOND)+1);
	/* the game adds and multiplies as the processor does */
	most= (long)((unsigned long)(packets+1)*(unsigned long)((long)((unsigned long)unknown4f0*(unsigned long)bits_per_second)/
		(long)((unsigned long)packets*NETWORK_OBSERVER_BITS_PER_BYTE_MILLISECOND)));
	parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
	if (most<=*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWNE0_OFFSET))
	{
		most= *(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWNE0_OFFSET);
	}
	if (unknown498>most)
	{
		unknown498= most;
	}
	network_observer_channel_set_flow(channel_index, observer, packet_rate, bits_per_second, unknown498);
	parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
	unknown4d8= (long)(*(const unsigned long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4D8_OFFSET)+
		*(const unsigned long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN184_OFFSET));
	*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4D8_OFFSET)= unknown4d8;
	if (unknown4d8>=*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN188_OFFSET))
	{
		network_observer_channel_lower_flow(observer, channel_index, FALSE);
		*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4D8_OFFSET)= 0;
	}
}

/* 0x00078210: the next rate of packets over one: the least of the rates of
what the observer has at 0x10, which are parts of the times a second that
the game goes, that is more than it, and the same one if none is; and with a
byte, no more than half of those times */
real NETWORK_OBSERVER_CALLBACK network_observer_next_packet_rate(
	real packet_rate,
	const byte *observer,
	boolean half)
{
	const byte *parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
	real tick_rate= network_observer_tick_rate();
	const real *rates= (const real *)(parameters+NETWORK_OBSERVER_PARAMETERS_PACKET_RATES_OFFSET);
	real next= NETWORK_OBSERVER_NO_PACKET_RATE;
	long index;

	for (index= *(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_PACKET_RATE_COUNT_OFFSET)-1; index>=0; index--)
	{
		real rate= rates[index]*tick_rate;

		if (rate>packet_rate && next>rate)
		{
			next= rate;
		}
	}
	if (next==NETWORK_OBSERVER_NO_PACKET_RATE)
	{
		next= packet_rate;
	}
	if (half)
	{
		real half_rate= tick_rate*0.5f;

		if (next>half_rate)
		{
			next= half_rate;
		}
	}

	return next;
}

/* 0x00079de0: a channel of the observer tries more flow; whether it does.
The three things that could be more: its bits a second, by the part that
the real at 0x190 of what the observer has at 0x10 says, by no more than the
number at 0x18C, and up to the number at 0x150; its number of 0x498, by the
bytes of one packet; and its packets a second, the next rate
(network_observer_next_packet_rate). With none that can be, the byte is set
and that is all. If not, once the interval has lasted the number at 0x1A0,
and if fewer channels are trying than the number of 0x4F44 of the observer
says, one of them is more, the first that was noted in the interval as
wanted (its bytes of 0x4A2, 0x4A1 and 0x4A0): the bits a second are then no
fewer than the packets need, and the number of 0x498 grows as they do. The
flow that it had is kept, with its number of 0x4B0 and the bits a second
that it was doing, and nothing is noted of it; the new one is its flow; its
number of 0x4DC is 2; and one more channel is trying. The time of 0x4E8 is
now either way */
boolean NETWORK_OBSERVER_CALLBACK network_observer_channel_raise_flow(
	byte *observer,
	long channel_index,
	boolean *unknown)
{
	byte *channel= (byte *)network_observer_channel(observer, channel_index);
	const byte *parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
	long bits_per_second= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_BITS_PER_SECOND_OFFSET);
	long unknown498= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN498_OFFSET);
	real packet_rate= *(const real *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN49C_OFFSET);
	long more= lrintf((real)bits_per_second**(const real *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN190_OFFSET));
	long more_bits_per_second;
	long more_unknown498;
	real more_packet_rate;
	real packet_bytes= 0.0f;
	boolean can_bits_per_second;
	boolean can_unknown498;
	boolean can_packet_rate;
	boolean raised= FALSE;
	long elapsed;

	if (more>*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN18C_OFFSET))
	{
		more= *(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN18C_OFFSET);
	}
	/* the game adds as the processor does */
	more_bits_per_second= (long)((unsigned long)more+(unsigned long)bits_per_second);
	if (more_bits_per_second>*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN150_OFFSET))
	{
		more_bits_per_second= *(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN150_OFFSET);
	}
	can_bits_per_second= more_bits_per_second>bits_per_second;
	if (packet_rate>0.0f)
	{
		packet_bytes= (real)bits_per_second/(packet_rate*NETWORK_OBSERVER_BITS_PER_BYTE);
	}
	more_unknown498= (long)((unsigned long)(long)packet_bytes+(unsigned long)unknown498);
	can_unknown498= more_unknown498>unknown498;
	more_packet_rate= network_observer_next_packet_rate(packet_rate, observer,
		channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN490_OFFSET]);
	can_packet_rate= more_packet_rate-packet_rate>NETWORK_OBSERVER_PACKET_RATE_EPSILON;
	elapsed= network_time_since(*(const long *)(observer+NETWORK_OBSERVER_UNKNOWN4F38_OFFSET));
	if (!can_bits_per_second && !can_unknown498 && !can_packet_rate)
	{
		*unknown= TRUE;

		return FALSE;
	}
	*unknown= FALSE;
	parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
	if (elapsed<*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN1A0_OFFSET))
	{
		return FALSE;
	}
	if (*(const long *)(observer+NETWORK_OBSERVER_UNKNOWN4F40_OFFSET)>=
		*(const long *)(observer+NETWORK_OBSERVER_UNKNOWN4F44_OFFSET))
	{
		if (parameters[NETWORK_OBSERVER_PARAMETERS_UNKNOWN144_OFFSET])
		{
			/* the game works these two out, for nothing */
			network_observer_channel_unknown4a4_bits_per_second(channel_index, observer);
			network_observer_channel_unknown4a8_bits_per_second(channel_index, observer);
		}

		return FALSE;
	}
	if (can_unknown498 && channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN4A2_OFFSET])
	{
		unknown498= more_unknown498;
		raised= TRUE;
	}
	else if (can_bits_per_second && channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN4A1_OFFSET])
	{
		bits_per_second= more_bits_per_second;
		raised= TRUE;
	}
	else if (can_packet_rate && channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN4A0_OFFSET])
	{
		packet_rate= more_packet_rate;
		raised= TRUE;
	}
	if (packet_rate>*(const real *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN49C_OFFSET))
	{
		real needed= (real)network_observer_packet_rate_bits_per_second(observer,
			channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48E_OFFSET], packet_rate);
		real have= (real)bits_per_second;

		if (have>needed)
		{
			needed= have;
		}
		bits_per_second= (long)needed;
	}
	if (bits_per_second>*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_BITS_PER_SECOND_OFFSET))
	{
		/* the game multiplies as the processor does */
		long had= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_BITS_PER_SECOND_OFFSET);
		long grown= (long)(*(const unsigned long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN498_OFFSET)*
			(unsigned long)bits_per_second)/had;

		if (unknown498<=grown)
		{
			unknown498= grown;
		}
	}
	if (raised)
	{
		long doing;

		/* the game works this one out for nothing */
		network_observer_channel_unknown4a4_bits_per_second(channel_index, observer);
		doing= network_observer_channel_unknown4a8_bits_per_second(channel_index, observer);
		*(long *)(channel+NETWORK_OBSERVER_CHANNEL_KEPT_UNKNOWN498_OFFSET)=
			*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN498_OFFSET);
		*(long *)(channel+NETWORK_OBSERVER_CHANNEL_KEPT_BITS_PER_SECOND_OFFSET)=
			*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_BITS_PER_SECOND_OFFSET);
		*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN504_OFFSET)=
			*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4B0_OFFSET);
		*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN508_OFFSET)= doing;
		channel[NETWORK_OBSERVER_CHANNEL_KEPT_FLOW_OFFSET]= TRUE;
		*(real *)(channel+NETWORK_OBSERVER_CHANNEL_KEPT_PACKET_RATE_OFFSET)=
			*(const real *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN49C_OFFSET);
		channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN50C_OFFSET]= 0;
		channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN50D_OFFSET]= 0;
		channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN50E_OFFSET]= 0;
		network_observer_channel_set_flow(channel_index, observer, packet_rate, bits_per_second, unknown498);
		*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4DC_OFFSET)= 2;
		++*(unsigned long *)(observer+NETWORK_OBSERVER_UNKNOWN4F40_OFFSET);
		observer[NETWORK_OBSERVER_UNKNOWN4F3C_OFFSET]= TRUE;
	}
	*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4E8_OFFSET)= network_time_since(0);

	return raised;
}

/* 0x004cf73c: something of the network that makes the voice go by the
console chosen for it when the flow of a channel is held, or something was
noted of one */
boolean unknown_004cf73c;

/* 0x0007a4a0: how the voice goes between the consoles (unknown_004c987c),
if the game has to do with the voice, by the channels of the observer that
are in use and have their byte of 0x48C. With one that has its bytes of
0x48E and 0x48F: 2 if the flow of any is held, and 1 if not. With none, and
unknown_004cf73c: 1 if the flow of any is held, or one that has its byte of
0x48E had its first or third byte noted in an interval. And 0 if not */
void network_observer_update_voice_mode(
	const byte *observer)
{
	boolean any_held= FALSE;
	boolean any_unknown48f= FALSE;
	boolean any_noted= FALSE;
	long mode= 0;
	long channel_index;

	for (channel_index= 0; channel_index<MAXIMUM_NETWORK_OBSERVER_CHANNELS; channel_index++)
	{
		const byte *channel= network_observer_channel(observer, channel_index);

		if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)!=0 &&
			channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48C_OFFSET])
		{
			if (channel[NETWORK_OBSERVER_CHANNEL_HELD_FLOW_OFFSET])
			{
				any_held= TRUE;
			}
			if (channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48E_OFFSET])
			{
				if (channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48F_OFFSET])
				{
					any_unknown48f= TRUE;
				}
				if (channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN50C_OFFSET] ||
					channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN50E_OFFSET])
				{
					any_noted= TRUE;
				}
			}
		}
	}
	if (any_unknown48f)
	{
		mode= any_held ? 2 : 1;
	}
	else if (unknown_004cf73c && (any_held || any_noted))
	{
		mode= 1;
	}
	if (game_voice_enabled)
	{
		unknown_004c987c= mode;
	}
}

/* 0x00082060, of the channels of the game: one that is not in use is taken,
with a name and a number; which, or NONE */
long __attribute__((stdcall)) unknown_00082060(const char *name, long unknown);

/* the four bytes of the address of the internet that what a channel of the
observer has at 0x14 starts with, from the first on, as the number that the
game writes them from */
static inline __attribute__((always_inline)) unsigned long network_observer_channel_internet_address(
	const byte *unknown14)
{
	unsigned long address= *(const unsigned long *)unknown14;

	return (address>>24) | ((address>>8)&0xFF00) | ((address<<8)&0xFF0000) | (address<<24);
}

/* 0x00078900: each channel of the observer that is in use takes a channel
of the game, named after the address of the internet of its console, which
has the observer as its own; with none to take, it has none. And what is
done with a channel each time is done with it now
(network_observer_channel_update_connection,
network_observer_channel_update_game_channel and
network_observer_channel_update_tries). It is the other way of
network_observer_release_game_channels */
void NETWORK_OBSERVER_CALLBACK network_observer_take_game_channels(
	byte *observer)
{
	long channel_index;

	for (channel_index= 0; channel_index<MAXIMUM_NETWORK_OBSERVER_CHANNELS; channel_index++)
	{
		byte *channel= (byte *)network_observer_channel(observer, channel_index);

		if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)!=0)
		{
			unsigned long address= network_observer_channel_internet_address(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN14_OFFSET);
			char name[NETWORK_CHANNEL_NAME_SIZE];
			long game_channel_index;

			csnzprintf(name, sizeof(name), "%hd.%hd.%hd.%hd", address>>24, (address>>16)&0xFF, (address>>8)&0xFF,
				address&0xFF);
			game_channel_index= unknown_00082060(name, NETWORK_OBSERVER_GAME_CHANNEL_UNKNOWN);
			*(long *)(channel+NETWORK_OBSERVER_CHANNEL_CHANNEL_OFFSET)= game_channel_index;
			if (game_channel_index!=NONE)
			{
				*(byte **)(unknown_004d87d4+game_channel_index*NETWORK_CHANNEL_SIZE+NETWORK_CHANNEL_OBSERVER_OFFSET)= observer;
			}
			network_observer_channel_update_connection(channel_index, observer);
			network_observer_channel_update_game_channel(channel_index, observer);
			network_observer_channel_update_tries(observer, channel_index);
		}
	}
}

/* 0x00076aa0: the channel of the observer for a console, which a session
uses from now on; NONE if there is none to give. It is the one that is in
use with what the console is known by; if none is, the first that is not in
use, and with all in use, the first that no session uses, which is no more
(network_observer_channel_delete). A new one takes a channel of the game,
named after the console's address of the internet and its address of the
network card, which has the observer as its own (with none to take, there is
no channel); it is in its second state, with no tries, no address and no
sessions that failed; it has what the console is known by; and its two
counts by spans of time and its two sets of last numbers start, as long as
two numbers of what the observer has at 0x10 say. If the sessions were told
that the channel has what its channel of the game is of, this one is too */
long NETWORK_OBSERVER_CALLBACK network_observer_acquire_channel(
	byte *observer,
	long session_index,
	const byte *unknown14)
{
	long channel_index= NONE;
	long index;
	byte *channel;

	for (index= 0; index<MAXIMUM_NETWORK_OBSERVER_CHANNELS; index++)
	{
		channel= (byte *)network_observer_channel(observer, index);
		if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)!=0 &&
			memcmp(unknown14, channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN14_OFFSET, NETWORK_OBSERVER_CHANNEL_UNKNOWN14_SIZE)==0)
		{
			channel_index= index;
			break;
		}
	}
	if (channel_index==NONE)
	{
		const byte *parameters;
		unsigned long address;
		char name[NETWORK_CHANNEL_NAME_SIZE];
		long game_channel_index;
		long new_index= NONE;

		for (index= 0; index<MAXIMUM_NETWORK_OBSERVER_CHANNELS; index++)
		{
			if (*(const long *)(network_observer_channel(observer, index)+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)==0)
			{
				new_index= index;
				break;
			}
		}
		if (new_index==NONE)
		{
			for (index= 0; index<MAXIMUM_NETWORK_OBSERVER_CHANNELS; index++)
			{
				channel= (byte *)network_observer_channel(observer, index);
				if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)!=0 &&
					!channel[NETWORK_OBSERVER_CHANNEL_SESSIONS_OFFSET])
				{
					new_index= index;
					break;
				}
			}
			if (new_index==NONE)
			{
				return NONE;
			}
			network_observer_channel_delete(new_index, observer);
		}
		channel= (byte *)network_observer_channel(observer, new_index);
		address= network_observer_channel_internet_address(unknown14);
		csnzprintf(name, sizeof(name), "%hd.%hd.%hd.%hd", address>>24, (address>>16)&0xFF, (address>>8)&0xFF,
			address&0xFF);
		csnzappendf(name, sizeof(name), "!MAC=%02X:%02X:%02X:%02X:%02X:%02X",
			unknown14[NETWORK_OBSERVER_UNKNOWN14_ETHERNET_OFFSET], unknown14[NETWORK_OBSERVER_UNKNOWN14_ETHERNET_OFFSET+1],
			unknown14[NETWORK_OBSERVER_UNKNOWN14_ETHERNET_OFFSET+2], unknown14[NETWORK_OBSERVER_UNKNOWN14_ETHERNET_OFFSET+3],
			unknown14[NETWORK_OBSERVER_UNKNOWN14_ETHERNET_OFFSET+4], unknown14[NETWORK_OBSERVER_UNKNOWN14_ETHERNET_OFFSET+5]);
		game_channel_index= unknown_00082060(name, NETWORK_OBSERVER_GAME_CHANNEL_UNKNOWN);
		if (game_channel_index==NONE)
		{
			return NONE;
		}
		*(long *)(channel+NETWORK_OBSERVER_CHANNEL_CHANNEL_OFFSET)= game_channel_index;
		*(byte **)(unknown_004d87d4+game_channel_index*NETWORK_CHANNEL_SIZE+NETWORK_CHANNEL_OBSERVER_OFFSET)= observer;
		*(long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)= _network_observer_channel_state2;
		*(long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_TIME_OFFSET)= network_time_since(0);
		*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN10_OFFSET)= NONE;
		memcpy(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN14_OFFSET, unknown14, NETWORK_OBSERVER_CHANNEL_UNKNOWN14_SIZE);
		*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN94_OFFSET)= network_time_since(0);
		*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNUSED_TIME_OFFSET)= 0;
		*(long *)(channel+NETWORK_OBSERVER_CHANNEL_WAITING_TIME_OFFSET)= 0;
		parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
		network_rate_new((struct network_rate *)(channel+NETWORK_OBSERVER_CHANNEL_RATEA0_OFFSET),
			*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_RATE_PERIOD_OFFSET));
		network_rate_new((struct network_rate *)(channel+NETWORK_OBSERVER_CHANNEL_RATE_OFFSET),
			*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_RATE_PERIOD_OFFSET));
		*(long *)(channel+NETWORK_OBSERVER_CHANNEL_WINDOW250_OFFSET)=
			*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_WINDOW_COUNT_OFFSET);
		network_window_reset((struct network_window *)(channel+NETWORK_OBSERVER_CHANNEL_WINDOW250_OFFSET));
		*(long *)(channel+NETWORK_OBSERVER_CHANNEL_WINDOW360_OFFSET)=
			*(const long *)(*(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET)+
			NETWORK_OBSERVER_PARAMETERS_WINDOW_COUNT_OFFSET);
		network_window_reset((struct network_window *)(channel+NETWORK_OBSERVER_CHANNEL_WINDOW360_OFFSET));
		*(short *)(channel+NETWORK_OBSERVER_CHANNEL_TRIES_OFFSET)= 0;
		*(long *)(channel+NETWORK_OBSERVER_CHANNEL_FAILED_SESSIONS_OFFSET)= 0;
		*(long *)(channel+NETWORK_OBSERVER_CHANNEL_SESSION_OFFSET)= NONE;
		network_observer_channel_forget_address(new_index, observer, FALSE, 0);
		channel_index= new_index;
	}
	channel= (byte *)network_observer_channel(observer, channel_index);
	channel[NETWORK_OBSERVER_CHANNEL_SESSIONS_OFFSET]|= (byte)(1u<<(session_index&31));
	if (channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN8_OFFSET]&_network_observer_channel_established_bit)
	{
		struct network_observer_session_owner *owner= *(struct network_observer_session_owner **)(observer+
			NETWORK_OBSERVER_SESSIONS_OFFSET+session_index*NETWORK_OBSERVER_SESSION_SIZE+
			NETWORK_OBSERVER_SESSION_SESSION_OFFSET);

		owner->functions->channel_established(owner, channel_index,
			*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN10_OFFSET), TRUE);
	}

	return channel_index;
}

/* the session of the network game that the voice goes with, as what a
session has in the observer; NULL if there is none */
static inline __attribute__((always_inline)) struct network_observer_session_owner *network_observer_voice_session(
	void)
{
	byte *session= NULL;

	return game_voice_get_session(&session) ? (struct network_observer_session_owner *)session : NULL;
}

/* 0x00076520: whether the voice has something to send to the console that a
channel is for, in the session that the voice goes with (its sixth function
says which console), and how much it is. With a byte, what
game_voice_unknown_00054890 says of the console, and no size */
boolean NETWORK_OBSERVER_CALLBACK network_observer_voice_has_packet(
	long channel_index,
	boolean unknown,
	long *size)
{
	struct network_observer_session_owner *session= network_observer_voice_session();
	boolean result= FALSE;

	if (session)
	{
		long peer= session->functions->channel_peer(session, channel_index);

		if (peer!=NONE)
		{
			if (unknown)
			{
				return game_voice_unknown_00054890(peer);
			}
			if (game_voice_has_packet_for_peer(peer))
			{
				result= TRUE;
				*size= game_voice_packet_size_for_peer(peer);
			}
		}
	}

	return result;
}

/* 0x000765c0: what the voice has to send to the console that a channel is
for is written (game_voice_build_packet); how much it is, and nothing with
no session that the voice goes with or no such console */
long NETWORK_OBSERVER_CALLBACK network_observer_voice_build_packet(
	long channel_index,
	long size,
	byte *packet)
{
	struct network_observer_session_owner *session= network_observer_voice_session();

	if (session)
	{
		long peer= session->functions->channel_peer(session, channel_index);

		if (peer!=NONE)
		{
			return game_voice_build_packet(peer, packet, size);
		}
	}

	return 0;
}

/* 0x00079260: what is done with the interval that the flow is counted in,
each time. Unless the byte of 0x4F3C of the observer says to do it now, only
once it has lasted the number at 0x19C of what the observer has at 0x10.

If a flow went down for good in it (0x4F3D), each channel that is trying
more flow, or has tried and it went well (2 and 3 of its number of 0x4DC),
has the flow that it kept again. If not, once the interval has lasted the
number at 0x1A4, each channel in use that has its byte of 0x48C keeps its
bits a second of the interval (0x510 and 0x514); and unless one of them has
its number of 0x4D8 over nothing, network_observer_unknown_00077f90 is told
how many they are, the bits a second of all, and how many of them did not
have their byte of 0x4A0 and were not short of the number at 0x1A8 with
their byte of 0x4A1.

If a flow went down with the byte (0x4F3E), the number of 0x4E0C goes
towards the bits a second of all the channels, and is that if it was none.
Then a new interval starts (network_observer_new_interval), and the three
bytes are clear */
void network_observer_update_interval(
	byte *observer)
{
	const byte *parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
	long elapsed= network_time_since(*(const long *)(observer+NETWORK_OBSERVER_UNKNOWN4F38_OFFSET));
	long channel_index;

	if (observer[NETWORK_OBSERVER_UNKNOWN4F3C_OFFSET] ||
		elapsed>=*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN19C_OFFSET))
	{
		if (observer[NETWORK_OBSERVER_UNKNOWN4F3D_OFFSET])
		{
			for (channel_index= 0; channel_index<MAXIMUM_NETWORK_OBSERVER_CHANNELS; channel_index++)
			{
				const byte *channel= network_observer_channel(observer, channel_index);

				if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)!=0 &&
					channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48C_OFFSET] &&
					(*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4DC_OFFSET)==2 ||
					*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4DC_OFFSET)==3))
				{
					network_observer_channel_restore_flow(channel_index, observer);
				}
			}
		}
		else if (elapsed>=*(const long *)(*(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET)+
			NETWORK_OBSERVER_PARAMETERS_UNKNOWN1A4_OFFSET))
		{
			long count= 0;
			long good_count= 0;
			long held_count= 0;
			unsigned long total= 0;

			for (channel_index= 0; channel_index<MAXIMUM_NETWORK_OBSERVER_CHANNELS; channel_index++)
			{
				byte *channel= (byte *)network_observer_channel(observer, channel_index);

				if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)!=0 &&
					channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48C_OFFSET])
				{
					long unknown4a4= network_observer_channel_unknown4a4_bits_per_second(channel_index, observer);
					long unknown4a8= network_observer_channel_unknown4a8_bits_per_second(channel_index, observer);
					boolean good= !(channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN4A1_OFFSET] &&
						unknown4a8<*(const long *)(*(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET)+
						NETWORK_OBSERVER_PARAMETERS_UNKNOWN1A8_OFFSET));

					count++;
					if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4D8_OFFSET)>0)
					{
						held_count++;
					}
					if (good && !channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN4A0_OFFSET])
					{
						good_count++;
					}
					*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN510_OFFSET)= unknown4a4;
					/* the game adds as the processor does */
					total+= (unsigned long)unknown4a8;
					*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN514_OFFSET)= unknown4a8;
				}
			}
			if (held_count==0)
			{
				network_observer_unknown_00077f90(count, (long)total, observer, good_count);
			}
		}
		if (observer[NETWORK_OBSERVER_UNKNOWN4F3E_OFFSET])
		{
			long count= 0;
			unsigned long total= 0;

			for (channel_index= 0; channel_index<MAXIMUM_NETWORK_OBSERVER_CHANNELS; channel_index++)
			{
				const byte *channel= network_observer_channel(observer, channel_index);

				if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)!=0 &&
					channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48C_OFFSET])
				{
					count++;
					total+= (unsigned long)network_observer_channel_bits_per_second(observer, channel_index,
						NETWORK_OBSERVER_CHANNEL_UNKNOWN4A4_OFFSET);
				}
			}
			if (count>0)
			{
				long unknown4e0c= *(const long *)(observer+NETWORK_OBSERVER_UNKNOWN4E0C_OFFSET);

				if (unknown4e0c!=NONE)
				{
					total= (unsigned long)network_observer_approach(unknown4e0c, (long)total,
						*(const long *)(*(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET)+
						NETWORK_OBSERVER_PARAMETERS_UNKNOWN1B4_OFFSET));
				}
				*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4E0C_OFFSET)= (long)total;
			}
		}
		network_observer_new_interval(observer);
	}
	observer[NETWORK_OBSERVER_UNKNOWN4F3C_OFFSET]= FALSE;
	observer[NETWORK_OBSERVER_UNKNOWN4F3D_OFFSET]= FALSE;
	observer[NETWORK_OBSERVER_UNKNOWN4F3E_OFFSET]= FALSE;
}

/* 0x000880b0, of the channels of the game: whether some bits are ones that
this console can do. 0x00088360: a channel of the game is given a number of
what it talks with */
boolean unknown_000880b0(unsigned long flags);
void __attribute__((stdcall)) unknown_00088360(byte *game_channel, long identifier);

/* what a console that asks to be talked with is told, when it is not: the
number that it gave, and why */
struct network_observer_connect_refusal
{
	long identifier;
	long reason;
};

/* 0x000785d0: a console asks, from an address of the game, to be talked
with, with a number and some bits. It is told no (a message of the fifth
kind, sent to the address) and why: 4 if the console's libraries are not
talking with the address; 1 if it is this console's own address, the bits
have others than the first six, or they are ones that this console cannot
do (unknown_000880b0, with the first two the other way around); and 3 if no
channel that a session uses is for what the console is known by.

With such a channel: if its channel of the game is past its second state
and talks with another address, or with this one and another number, it is
told 6 (unknown_00088650). Then, if it is not past its second state, the
address is this one from now on, with what the session is known by and its
key and no session of its own; the channel of the game starts to connect
(unknown_00088220, with nothing); and the channel is in its sixth state,
with no tries. The channel of the game is given the number, and what is done
with a channel each time is done with it */
void NETWORK_OBSERVER_CALLBACK network_observer_handle_connect_request(
	byte *observer,
	const struct transport_address *address,
	const byte *request)
{
	struct network_observer_connect_refusal refusal;
	byte identifier[NETWORK_OBSERVER_SESSION_IDENTIFIER_SIZE];
	byte key[NETWORK_OBSERVER_SESSION_KEY_SIZE];
	byte unknown14[NETWORK_OBSERVER_CHANNEL_UNKNOWN14_SIZE];
	long unknown4;
	long reason;
	long channel_index= NONE;

	if (transport_address_get_connect_status(address)!=_network_observer_connect_status2)
	{
		reason= NETWORK_OBSERVER_REFUSAL_NOT_CONNECTED;
	}
	else
	{
		unsigned long flags= *(const unsigned long *)(request+NETWORK_OBSERVER_CONNECT_REQUEST_FLAGS_OFFSET);

		reason= NETWORK_OBSERVER_REFUSAL_UNKNOWN1;
		if (!(address->address_size==sizeof(address->address.ipv4) && address->address.ipv4==NETWORK_OBSERVER_LOOPBACK_ADDRESS) &&
			!(flags&NETWORK_OBSERVER_CONNECT_REQUEST_OTHER_FLAGS) &&
			unknown_000880b0(((flags>>1)&1) | ((flags&1)<<1) | (flags&NETWORK_OBSERVER_CONNECT_REQUEST_SAME_FLAGS)))
		{
			reason= NETWORK_OBSERVER_REFUSAL_NO_CHANNEL;
			if (transport_security_peer_address_from_address(address, 0, &unknown4, identifier, key, unknown14))
			{
				long index;

				for (index= 0; index<MAXIMUM_NETWORK_OBSERVER_CHANNELS; index++)
				{
					const byte *channel= network_observer_channel(observer, index);

					if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)!=0 &&
						memcmp(unknown14, channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN14_OFFSET, sizeof(unknown14))==0 &&
						channel[NETWORK_OBSERVER_CHANNEL_SESSIONS_OFFSET])
					{
						channel_index= index;
						break;
					}
				}
			}
		}
	}
	if (channel_index==NONE)
	{
		refusal.identifier= *(const long *)(request+NETWORK_OBSERVER_CONNECT_REQUEST_IDENTIFIER_OFFSET);
		refusal.reason= reason;
		unknown_0007b140(*(const long *)(observer+NETWORK_OBSERVER_UNKNOWN8_OFFSET), address,
			_network_message_connect_refuse, sizeof(refusal), &refusal);

		return;
	}
	{
		byte *channel= (byte *)network_observer_channel(observer, channel_index);
		byte *game_channel= unknown_004d87d4+
			*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_CHANNEL_OFFSET)*NETWORK_CHANNEL_SIZE;
		struct transport_address current;
		unsigned long difference;
		boolean connect= TRUE;

		network_channel_get_address(game_channel, &current);
		difference= *(const unsigned long *)(request+NETWORK_OBSERVER_CONNECT_REQUEST_IDENTIFIER_OFFSET)-
			*(const unsigned long *)(game_channel+NETWORK_CHANNEL_UNKNOWN50_OFFSET);
		if (*(const long *)(game_channel+NETWORK_CHANNEL_STATE_OFFSET)>_network_channel_state2)
		{
			if (!transport_address_equal(address, &current, FALSE))
			{
				byte other_identifier[NETWORK_OBSERVER_SESSION_IDENTIFIER_SIZE];
				byte other_unknown14[NETWORK_OBSERVER_CHANNEL_UNKNOWN14_SIZE];
				long other_unknown4;

				/* the game asks these two of the address that it had, for
				nothing */
				transport_security_peer_address_from_address(&current, 0, &other_unknown4, other_identifier, NULL,
					other_unknown14);
				transport_address_get_connect_status(&current);
				unknown_00088650(game_channel, _unknown_00088650_unknown6);
			}
			else if (*(const long *)(game_channel+NETWORK_CHANNEL_UNKNOWN50_OFFSET)!=NONE && difference!=0)
			{
				unknown_00088650(game_channel, _unknown_00088650_unknown6);
			}
			connect= *(const long *)(game_channel+NETWORK_CHANNEL_STATE_OFFSET)<=_network_channel_state2;
		}
		if (connect)
		{
			transport_address_connect(address);
			*(struct transport_address *)(channel+NETWORK_OBSERVER_CHANNEL_ADDRESS_OFFSET)= *address;
			*(long *)(channel+NETWORK_OBSERVER_CHANNEL_SESSION_UNKNOWN4_OFFSET)= unknown4;
			memcpy(channel+NETWORK_OBSERVER_CHANNEL_SESSION_IDENTIFIER_OFFSET, identifier, sizeof(identifier));
			memcpy(channel+NETWORK_OBSERVER_CHANNEL_SESSION_KEY_OFFSET, key, sizeof(key));
			*(long *)(channel+NETWORK_OBSERVER_CHANNEL_SESSION_OFFSET)= NONE;
			unknown_00088220(address, game_channel, FALSE);
			network_observer_channel_set_state(_network_observer_channel_state6, observer, channel_index);
			*(short *)(channel+NETWORK_OBSERVER_CHANNEL_TRIES_OFFSET)= 0;
			network_observer_channel_update_game_channel(channel_index, observer);
		}
		unknown_00088360(game_channel, *(const long *)(request+NETWORK_OBSERVER_CONNECT_REQUEST_IDENTIFIER_OFFSET));
		network_observer_channel_update_connection(channel_index, observer);
		network_observer_channel_update_game_channel(channel_index, observer);
	}
}

/* 0x00078a90: whether of two channels the second has more of a real than
the first: what the channels are put in order with, the ones with less first */
long NETWORK_OBSERVER_CALLBACK network_observer_compare_channels(
	long channel_index,
	long other_index,
	const real *values)
{
	return values[other_index]>values[channel_index];
}

/* how much a channel can send at once: what goes in so many packets as
there are in so many milliseconds, and two more, and no less than the number
at 0xE0 of what the observer has at 0x10. The game adds and multiplies as
the processor does */
static inline __attribute__((always_inline)) long network_observer_flow_unknown498(
	const byte *observer,
	long milliseconds,
	real packet_rate,
	long bits_per_second)
{
	long packets= (long)((unsigned long)(long)((real)milliseconds*packet_rate*NETWORK_OBSERVER_SECONDS_PER_MILLISECOND)+1);
	long most= (long)((unsigned long)(packets+1)*(unsigned long)((long)((unsigned long)milliseconds*(unsigned long)bits_per_second)/
		(long)((unsigned long)packets*NETWORK_OBSERVER_BITS_PER_BYTE_MILLISECOND)));
	long least= *(const long *)(*(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET)+
		NETWORK_OBSERVER_PARAMETERS_UNKNOWNE0_OFFSET);

	return most<=least ? least : most;
}

/* 0x00078e60: the flow of a channel of the observer starts. It starts with
the bits a second that the number at 0x158 of what the observer has at 0x10
says, unless that and what the channels that have a flow already do is more
than there is for all: half of the number of 0x4E04 of the observer, or
three quarters with its byte of 0x4E01, no more than the numbers at 0x15C
and 0x160, and for a while after its time of 0x4F2C no less than the number
at 0x164 for each channel. Then all of them, this one too, share what there
is, or what the others did if it is more: this one has its part, no less
than the number at 0x14C, and the others have theirs less in proportion
(network_observer_channel_set_flow).

All that the channel keeps of its flow is nothing, with its byte of 0x48C;
three bytes say what its channel of the game has at 0x3C; its numbers of
0x4F0 and 0x4B0 are the number that its channel of the game has for it, no
less than the one at 0x16C; and its flow is those bits a second, the packets
that they give (network_observer_packet_rate) and how much it can send at
once */
void NETWORK_OBSERVER_CALLBACK network_observer_channel_start_flow(
	long channel_index,
	byte *observer)
{
	byte *channel= (byte *)network_observer_channel(observer, channel_index);
	const byte *game_channel= unknown_004d87d4+
		*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_CHANNEL_OFFSET)*NETWORK_CHANNEL_SIZE;
	const byte *parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
	const byte *unknown3c;
	long count= 0;
	unsigned long total= 0;
	long bits_per_second= *(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN158_OFFSET);
	long budget= *(const long *)(observer+NETWORK_OBSERVER_UNKNOWN4E04_OFFSET);
	long limit;
	long unknown4f0;
	long index;
	real packet_rate;

	for (index= 0; index<MAXIMUM_NETWORK_OBSERVER_CHANNELS; index++)
	{
		const byte *other= network_observer_channel(observer, index);

		if (*(const long *)(other+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)!=0 && other[NETWORK_OBSERVER_CHANNEL_UNKNOWN48C_OFFSET])
		{
			count++;
			total+= *(const unsigned long *)(other+NETWORK_OBSERVER_CHANNEL_BITS_PER_SECOND_OFFSET);
		}
	}
	/* the game adds and multiplies as the processor does */
	if (observer[NETWORK_OBSERVER_UNKNOWN4E01_OFFSET])
	{
		limit= *(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN160_OFFSET);
		budget= (long)((unsigned long)budget*3)/4;
	}
	else
	{
		limit= *(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN15C_OFFSET);
		budget= budget/2;
	}
	if (budget>limit)
	{
		budget= limit;
	}
	if (network_time_since(*(const long *)(observer+NETWORK_OBSERVER_UNKNOWN4F2C_OFFSET))<=
		*(const long *)(*(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET)+
		NETWORK_OBSERVER_PARAMETERS_UNKNOWN154_OFFSET))
	{
		long least= (long)((unsigned long)(count+1)**(const unsigned long *)(*(const byte **)(observer+
			NETWORK_OBSERVER_UNKNOWN10_OFFSET)+NETWORK_OBSERVER_PARAMETERS_UNKNOWN164_OFFSET));

		if (budget<=least)
		{
			budget= least;
		}
	}
	if ((long)((unsigned long)bits_per_second+total)>budget)
	{
		long most= (long)total;

		if (most<=budget)
		{
			most= budget;
		}
		parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
		bits_per_second= most/(count+1);
		if (bits_per_second<=*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN14C_OFFSET))
		{
			bits_per_second= *(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN14C_OFFSET);
		}
		if (count>0 && (real)(long)total>0.0f)
		{
			real scale= (real)(long)((unsigned long)most-(unsigned long)bits_per_second)/(real)(long)total;

			for (index= 0; index<MAXIMUM_NETWORK_OBSERVER_CHANNELS; index++)
			{
				const byte *other= network_observer_channel(observer, index);

				if (*(const long *)(other+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)!=0 &&
					other[NETWORK_OBSERVER_CHANNEL_UNKNOWN48C_OFFSET])
				{
					long other_bits_per_second= lrintf((real)*(const long *)(other+NETWORK_OBSERVER_CHANNEL_BITS_PER_SECOND_OFFSET)*scale);
					long other_unknown498= lrintf((real)*(const long *)(other+NETWORK_OBSERVER_CHANNEL_UNKNOWN498_OFFSET)*scale);

					parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
					if (other_bits_per_second<=*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN14C_OFFSET))
					{
						other_bits_per_second= *(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN14C_OFFSET);
					}
					if (other_unknown498<=*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWNE0_OFFSET))
					{
						other_unknown498= *(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWNE0_OFFSET);
					}
					/* the game gives the bytes of this channel, not of the
					other */
					network_observer_channel_set_flow(index, observer, network_observer_packet_rate(observer,
						other_bits_per_second, channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48E_OFFSET],
						channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN490_OFFSET]), other_bits_per_second, other_unknown498);
				}
			}
		}
	}
	memset(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN48C_OFFSET, 0, NETWORK_OBSERVER_CHANNEL_FLOW_SIZE);
	channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48C_OFFSET]= TRUE;
	unknown3c= *(const byte **)(game_channel+NETWORK_CHANNEL_UNKNOWN3C_OFFSET);
	channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48E_OFFSET]= unknown3c!=NULL;
	channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48F_OFFSET]= unknown3c!=NULL && unknown3c[NETWORK_CHANNEL_UNKNOWN3C_UNKNOWN30_OFFSET];
	channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN490_OFFSET]= unknown3c!=NULL && !unknown3c[NETWORK_CHANNEL_UNKNOWN3C_UNKNOWN30_OFFSET];
	*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4E0_OFFSET)= NONE;
	*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4E4_OFFSET)= NONE;
	*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4E8_OFFSET)= NONE;
	unknown4f0= 0;
	if (*(const long *)(game_channel+NETWORK_CHANNEL_STATE_OFFSET)==_network_channel_state5 &&
		(game_channel[NETWORK_CHANNEL_UNKNOWN48_OFFSET]&_network_channel_unknown48_bit8))
	{
		unknown4f0= *(const long *)(unknown_004d87d8+*(const long *)(game_channel+NETWORK_CHANNEL_UNKNOWN10_OFFSET)*
			NETWORK_CHANNEL_UNKNOWN10_SIZE+NETWORK_CHANNEL_UNKNOWN10_UNKNOWN96C_OFFSET);
	}
	parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
	if (unknown4f0<=*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN16C_OFFSET))
	{
		unknown4f0= *(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN16C_OFFSET);
	}
	*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4F0_OFFSET)= unknown4f0;
	*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4B0_OFFSET)= unknown4f0;
	packet_rate= network_observer_packet_rate(observer, bits_per_second, channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48E_OFFSET],
		channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN490_OFFSET]);
	/* the game has network_observer_channel_set_flow written here */
	network_observer_channel_set_flow(channel_index, observer, packet_rate, bits_per_second,
		network_observer_flow_unknown498(observer, unknown4f0, packet_rate, bits_per_second));
}

#include "cseries/sort.h"

/* 0x00078ac0: what the observer does with the flow of its channels each
time. Each channel in use that has a flow loses it (and the interval is to
be done with now) if the observer's byte of 0x4E00 is not set, its channel
of the game is not in its fifth state, or what that has at 0x3C is not as
the channel's two bytes say; and one that has none starts one
(network_observer_channel_start_flow), with the byte and the channel of the
game in its fifth state. Of each one with a flow, its byte of 0x48D is
whether a session that uses it says so, by its second function, or the voice
has something for its console (game_voice_unknown_00054890).

Then, a channel that has a flow kept, and whose number of 0x4B0, less the
one of 0x4B4, is over the one of 0x504 and a margin, has its flow go down
for good (network_observer_channel_lower_flow, with the byte).

And every so long (the number at 0x148 of what the observer has at 0x10):
no channel is trying more flow, and so many can as the part of the ones with
a flow that the real at 0x1C4 says, one at least and the number at 0x1C0 at
most; the channels with a flow, in order of the real that
network_observer_channel_unknown_0007a2a0 gives each, the ones with less
first, have done what is done with the flow of a channel each time
(network_observer_channel_update_flow); the interval is looked at
(network_observer_update_interval); and a channel that tried more flow and
it went well (3 of its number of 0x4DC) can try again, with nothing gone
well yet. The interval is looked at in any case */
void NETWORK_OBSERVER_CALLBACK network_observer_update_flow(
	byte *observer)
{
	boolean enabled= observer[NETWORK_OBSERVER_UNKNOWN4E00_OFFSET];
	const byte *parameters;
	long channel_index;

	for (channel_index= 0; channel_index<MAXIMUM_NETWORK_OBSERVER_CHANNELS; channel_index++)
	{
		byte *channel= (byte *)network_observer_channel(observer, channel_index);
		const byte *game_channel;
		struct network_observer_session_owner *voice;
		boolean wanted= FALSE;
		boolean connected;
		long session_index;

		if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)==0)
		{
			continue;
		}
		game_channel= unknown_004d87d4+*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_CHANNEL_OFFSET)*NETWORK_CHANNEL_SIZE;
		connected= enabled && *(const long *)(game_channel+NETWORK_CHANNEL_STATE_OFFSET)==_network_channel_state5;
		if (channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48C_OFFSET])
		{
			const byte *unknown3c= *(const byte **)(game_channel+NETWORK_CHANNEL_UNKNOWN3C_OFFSET);

			if (!(connected && (unknown3c!=NULL)==channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48E_OFFSET] &&
				(unknown3c!=NULL && unknown3c[NETWORK_CHANNEL_UNKNOWN3C_UNKNOWN30_OFFSET])==
				channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48F_OFFSET]))
			{
				channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48C_OFFSET]= FALSE;
				observer[NETWORK_OBSERVER_UNKNOWN4F3C_OFFSET]= TRUE;
			}
		}
		if (connected && !channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48C_OFFSET])
		{
			network_observer_channel_start_flow(channel_index, observer);
			observer[NETWORK_OBSERVER_UNKNOWN4F3C_OFFSET]= TRUE;
		}
		if (!channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48C_OFFSET])
		{
			continue;
		}
		for (session_index= 0; session_index<MAXIMUM_NETWORK_OBSERVER_SESSIONS; session_index++)
		{
			if (channel[NETWORK_OBSERVER_CHANNEL_SESSIONS_OFFSET]&(1u<<session_index))
			{
				struct network_observer_session_owner *owner= *(struct network_observer_session_owner **)(observer+
					NETWORK_OBSERVER_SESSIONS_OFFSET+session_index*NETWORK_OBSERVER_SESSION_SIZE+
					NETWORK_OBSERVER_SESSION_SESSION_OFFSET);

				if (owner->functions->channel_unknown4(owner, channel_index))
				{
					wanted= TRUE;
					break;
				}
			}
		}
		/* the game has game_voice_get_session, and what it calls, written
		here */
		voice= network_observer_voice_session();
		if (voice)
		{
			long peer= voice->functions->channel_peer(voice, channel_index);

			if (peer!=NONE && game_voice_unknown_00054890(peer))
			{
				wanted= TRUE;
			}
		}
		channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48D_OFFSET]= wanted;
	}
	for (channel_index= 0; channel_index<MAXIMUM_NETWORK_OBSERVER_CHANNELS; channel_index++)
	{
		const byte *channel= network_observer_channel(observer, channel_index);

		if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)!=0 &&
			channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48C_OFFSET] && channel[NETWORK_OBSERVER_CHANNEL_KEPT_FLOW_OFFSET])
		{
			unsigned long margin;

			parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
			margin= *(const unsigned long *)(parameters+(channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48E_OFFSET] ?
				NETWORK_OBSERVER_PARAMETERS_UNKNOWN1DC_OFFSET : NETWORK_OBSERVER_PARAMETERS_UNKNOWN1D4_OFFSET));
			/* the game adds and takes away as the processor does */
			if ((long)(*(const unsigned long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4B0_OFFSET)-
				*(const unsigned long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4B4_OFFSET))>
				(long)(*(const unsigned long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN504_OFFSET)+margin))
			{
				network_observer_channel_lower_flow(observer, channel_index, TRUE);
			}
		}
	}
	parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
	if (network_time_since(*(const long *)(observer+NETWORK_OBSERVER_UNKNOWN4F30_OFFSET))>=
		*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN148_OFFSET))
	{
		real values[MAXIMUM_NETWORK_OBSERVER_CHANNELS];
		long indices[MAXIMUM_NETWORK_OBSERVER_CHANNELS];
		long count= 0;
		long unused;
		long most;
		long index;

		memset(values, 0, sizeof(values));
		*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4F40_OFFSET)= 0;
		for (channel_index= 0; channel_index<MAXIMUM_NETWORK_OBSERVER_CHANNELS; channel_index++)
		{
			const byte *channel= network_observer_channel(observer, channel_index);

			if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)!=0 &&
				channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48C_OFFSET])
			{
				indices[count++]= channel_index;
				values[channel_index]= network_observer_channel_unknown_0007a2a0(channel_index, observer);
			}
		}
		parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
		most= lrintf((real)count**(const real *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN1C4_OFFSET));
		if (most<1)
		{
			most= 1;
		}
		else if (most>*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN1C0_OFFSET))
		{
			most= *(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN1C0_OFFSET);
		}
		*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4F44_OFFSET)= most;
		qsort_4byte(indices, count, &unused, (sort_compare_4byte_proc)network_observer_compare_channels, values);
		for (index= 0; index<count; index++)
		{
			network_observer_channel_update_flow(observer, indices[index]);
		}
		network_observer_update_interval(observer);
		for (channel_index= 0; channel_index<MAXIMUM_NETWORK_OBSERVER_CHANNELS; channel_index++)
		{
			byte *channel= (byte *)network_observer_channel(observer, channel_index);

			if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)!=0 &&
				channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48C_OFFSET] &&
				*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4DC_OFFSET)==3)
			{
				*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4DC_OFFSET)= 1;
				*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4EC_OFFSET)= 0;
			}
		}
		*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4F30_OFFSET)= network_time_since(0);
	}
	network_observer_update_interval(observer);
}

/* 0x000779f0: what the observer does, every so long (the number at 0x118 of
what it has at 0x10), with how much it lets the channels that the game sends
by have between them.

It counts how badly things go: of the channels in their seventh state whose
second set of last numbers is not older than the number at 0x100, the ones
whose numbers are, on average, the real at 0x104 or more. If they are the
part that the real at 0x11C says or more, this is one more time in a row
that things go badly (0x4F14), and if not, one more that they go well
(0x4F10); neither is counted past the number at 0x130.

With its byte of 0x4E00, and what it has sent of late in units of 1024 bits
a second (its count of 0x4E30): after enough bad times in a row, and not
more often than the number at 0x128 says, what it lets have (0x4E20 and
0x4E28) is no more than what it sent, less the number at 0x124, and no less
than the number at 0x138; and the number of 0x4E24 is no more than what it
sent. After enough good times, the number of 0x4E20 is no less than what it
sent, and what it lets have goes up by the number at 0x140, from what it was
or from what it sent as its byte of 0x4F18 says, if that leaves the number
at 0x13C under the one of 0x4E24.

Then each channel has no part; and the ones in their seventh state whose
channel of the game has something at 0x3C share what it lets have: so many
bits a second each (0x4F20), the packets a second that those give, in full
and with no more than half the times a second that the game goes (0x4F24 and
0x4F28, which a channel has as what its channel of the game has at 0x3C
says), and how much it can send at once, by two numbers that its channel of
the game has for it */
void network_observer_update_bandwidth(
	byte *observer)
{
	const byte *parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
	long indices[MAXIMUM_NETWORK_OBSERVER_CHANNELS];
	long count= 0;
	long channel_index;
	long index;
	boolean bad= FALSE;

	if (network_time_since(*(const long *)(observer+NETWORK_OBSERVER_UNKNOWN4F08_OFFSET))<
		*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN118_OFFSET))
	{
		return;
	}
	if (observer[NETWORK_OBSERVER_UNKNOWN4E00_OFFSET])
	{
		long total_count= 0;
		long bad_count= 0;

		for (channel_index= 0; channel_index<MAXIMUM_NETWORK_OBSERVER_CHANNELS; channel_index++)
		{
			const byte *channel= network_observer_channel(observer, channel_index);
			const struct network_window *window=
				(const struct network_window *)(channel+NETWORK_OBSERVER_CHANNEL_WINDOW360_OFFSET);

			if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)==_network_observer_channel_state7 &&
				window->span<=*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN100_OFFSET))
			{
				total_count++;
				if ((real)(long)window->sum/(real)window->count>=
					*(const real *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN104_OFFSET))
				{
					bad_count++;
				}
			}
		}
		bad= total_count>0 && (real)bad_count/(real)total_count>=
			*(const real *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN11C_OFFSET);
	}
	/* the game adds as the processor does */
	if (bad)
	{
		long times= (long)(*(const unsigned long *)(observer+NETWORK_OBSERVER_UNKNOWN4F14_OFFSET)+1);

		*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4F10_OFFSET)= 0;
		if (times>*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN130_OFFSET))
		{
			times= *(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN130_OFFSET);
		}
		*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4F14_OFFSET)= times;
	}
	else
	{
		long times= (long)(*(const unsigned long *)(observer+NETWORK_OBSERVER_UNKNOWN4F10_OFFSET)+1);

		*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4F14_OFFSET)= 0;
		if (times>*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN130_OFFSET))
		{
			times= *(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN130_OFFSET);
		}
		*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4F10_OFFSET)= times;
	}
	if (observer[NETWORK_OBSERVER_UNKNOWN4E00_OFFSET])
	{
		struct network_rate *rate= (struct network_rate *)(observer+NETWORK_OBSERVER_UNKNOWN4E30_OFFSET);
		long sent;

		network_rate_update(rate);
		sent= lrintf((real)(long)rate->spans_size*rate->per_second*NETWORK_OBSERVER_BITS_PER_BYTE*
			NETWORK_OBSERVER_UNITS_PER_BIT);
		parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
		if (*(const long *)(observer+NETWORK_OBSERVER_UNKNOWN4F14_OFFSET)>=
			*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN120_OFFSET))
		{
			if (network_time_since(*(const long *)(observer+NETWORK_OBSERVER_UNKNOWN4F0C_OFFSET))>=
				*(const long *)(*(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET)+
				NETWORK_OBSERVER_PARAMETERS_UNKNOWN128_OFFSET))
			{
				long unknown4e20= *(const long *)(observer+NETWORK_OBSERVER_UNKNOWN4E20_OFFSET);
				long less;

				parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
				less= (long)((unsigned long)sent-*(const unsigned long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN124_OFFSET));
				if (unknown4e20>less)
				{
					unknown4e20= less;
				}
				if (unknown4e20<=*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN138_OFFSET))
				{
					unknown4e20= *(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN138_OFFSET);
				}
				*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4E20_OFFSET)= unknown4e20;
				*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4E28_OFFSET)= unknown4e20;
				if (*(const long *)(observer+NETWORK_OBSERVER_UNKNOWN4E24_OFFSET)>sent)
				{
					*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4E24_OFFSET)= sent;
				}
				*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4F0C_OFFSET)= network_time_since(0);
			}
		}
		else if (*(const long *)(observer+NETWORK_OBSERVER_UNKNOWN4F10_OFFSET)>=
			*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN12C_OFFSET))
		{
			long more;

			if (sent>*(const long *)(observer+NETWORK_OBSERVER_UNKNOWN4E20_OFFSET))
			{
				*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4E20_OFFSET)= sent;
			}
			more= (long)(*(const unsigned long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN140_OFFSET)+
				(observer[NETWORK_OBSERVER_UNKNOWN4F18_OFFSET] ?
				*(const unsigned long *)(observer+NETWORK_OBSERVER_UNKNOWN4E28_OFFSET) : (unsigned long)sent));
			if (more>*(const long *)(observer+NETWORK_OBSERVER_UNKNOWN4E28_OFFSET) &&
				(long)(*(const unsigned long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN13C_OFFSET)+(unsigned long)more)<
				*(const long *)(observer+NETWORK_OBSERVER_UNKNOWN4E24_OFFSET))
			{
				*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4E28_OFFSET)= more;
				*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4F10_OFFSET)= 0;
			}
		}
	}
	for (channel_index= 0; channel_index<MAXIMUM_NETWORK_OBSERVER_CHANNELS; channel_index++)
	{
		byte *channel= (byte *)network_observer_channel(observer, channel_index);
		long game_channel_index= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_CHANNEL_OFFSET);

		*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN480_OFFSET)= NONE;
		*(real *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN484_OFFSET)= -1.0f;
		*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN488_OFFSET)= NONE;
		if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)==_network_observer_channel_state7 &&
			game_channel_index!=NONE &&
			*(const byte **)(unknown_004d87d4+game_channel_index*NETWORK_CHANNEL_SIZE+NETWORK_CHANNEL_UNKNOWN3C_OFFSET)!=NULL)
		{
			indices[count++]= channel_index;
		}
	}
	if (count>0)
	{
		*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4F1C_OFFSET)= count;
		if (observer[NETWORK_OBSERVER_UNKNOWN4E00_OFFSET])
		{
			/* the game multiplies as the processor does, and has the three
			functions that give these written here */
			long bits_per_second= (long)(*(const unsigned long *)(observer+NETWORK_OBSERVER_UNKNOWN4E28_OFFSET)<<10)/count;
			real packet_rate;

			*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4F20_OFFSET)= bits_per_second;
			packet_rate= network_observer_packet_rate(observer, bits_per_second, TRUE, FALSE);
			*(real *)(observer+NETWORK_OBSERVER_UNKNOWN4F24_OFFSET)= packet_rate;
			*(real *)(observer+NETWORK_OBSERVER_UNKNOWN4F28_OFFSET)=
				network_observer_packet_rate(observer, bits_per_second, TRUE, TRUE);
			observer[NETWORK_OBSERVER_UNKNOWN4F18_OFFSET]= network_observer_packet_rate_low(TRUE, packet_rate, observer, TRUE, FALSE);
		}
		else
		{
			*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4F20_OFFSET)= NONE;
			*(real *)(observer+NETWORK_OBSERVER_UNKNOWN4F24_OFFSET)= -1.0f;
			*(real *)(observer+NETWORK_OBSERVER_UNKNOWN4F28_OFFSET)= -1.0f;
			observer[NETWORK_OBSERVER_UNKNOWN4F18_OFFSET]= FALSE;
		}
	}
	for (index= 0; index<count; index++)
	{
		byte *channel= (byte *)network_observer_channel(observer, indices[index]);
		const byte *game_channel= unknown_004d87d4+
			*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_CHANNEL_OFFSET)*NETWORK_CHANNEL_SIZE;
		const byte *unknown3c= *(const byte **)(game_channel+NETWORK_CHANNEL_UNKNOWN3C_OFFSET);
		long bits_per_second= *(const long *)(observer+NETWORK_OBSERVER_UNKNOWN4F20_OFFSET);

		*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN480_OFFSET)= bits_per_second;
		*(real *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN484_OFFSET)=
			*(const real *)(observer+(unknown3c!=NULL && unknown3c[NETWORK_CHANNEL_UNKNOWN3C_UNKNOWN30_OFFSET] ?
			NETWORK_OBSERVER_UNKNOWN4F24_OFFSET : NETWORK_OBSERVER_UNKNOWN4F28_OFFSET));
		if (bits_per_second>=0)
		{
			unsigned long unknown96c= 0;
			unsigned long unknown970= 0;
			long unknown488;

			if (*(const long *)(game_channel+NETWORK_CHANNEL_STATE_OFFSET)==_network_channel_state5 &&
				(game_channel[NETWORK_CHANNEL_UNKNOWN48_OFFSET]&_network_channel_unknown48_bit8))
			{
				const byte *own= unknown_004d87d8+*(const long *)(game_channel+NETWORK_CHANNEL_UNKNOWN10_OFFSET)*
					NETWORK_CHANNEL_UNKNOWN10_SIZE;

				unknown96c= *(const unsigned long *)(own+NETWORK_CHANNEL_UNKNOWN10_UNKNOWN96C_OFFSET);
				unknown970= *(const unsigned long *)(own+NETWORK_CHANNEL_UNKNOWN10_UNKNOWN970_OFFSET);
			}
			/* the game adds and multiplies as the processor does */
			unknown488= (long)((unknown96c+unknown970*2)*(unsigned long)bits_per_second)/NETWORK_OBSERVER_BITS_PER_BYTE_MILLISECOND;
			parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
			if (unknown488<=*(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWNE0_OFFSET))
			{
				unknown488= *(const long *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWNE0_OFFSET);
			}
			*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN488_OFFSET)= unknown488;
		}
		else
		{
			*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN488_OFFSET)= NONE;
		}
	}
	*(long *)(observer+NETWORK_OBSERVER_UNKNOWN4F08_OFFSET)= network_time_since(0);
}

/* 0x00089030, 0x00088fd0, 0x00089000, of the channels of the game: three
numbers of one of them: something that it has waiting, how many bytes it has
sent that were not answered, and in how many packets */
long unknown_00089030(const byte *game_channel);
long unknown_00088fd0(const byte *game_channel);
long unknown_00089000(const byte *game_channel);

/* 0x004cf5d0: the packets a second of a channel that has no flow and whose
channel of the game has nothing at 0x3C, when it has something of the voice
to send */
real unknown_004cf5d0;

/* 0x00075fc0: whether a channel of the game can send a packet now, and of
what size; FALSE if no channel of the observer in use is that one.

The packets a second that it can do: with something to send that is not
urgent (the channel of the game has something at 0x3C, or the voice has
something for its console), the ones of its flow, or of its part of what the
observer lets have, or unknown_004cf5d0; with nothing of that, none with the
first byte, and 4 or nothing as the second says; and any when it is urgent
(network_observer_channel_unknown_00077940). A channel of the game in its
fifth state with little waiting does no fewer than what two reals of what
the observer has at 0x10 give for how much it is. It cannot send until as
many pictures have gone by, since it last could, as there are between two
packets; nor, with bits a second to keep to, until those give 48 bytes after
the 45 that a packet has more; nor if what it sent and was not answered
leaves less room than that in what it can send at once (and then that is
noted of its flow).

Then the voice writes what it has, in a part of the room that a real says,
no more than it is given room for; and what is left is the size, no less
than 48 bytes, in eights, and no more than what is left of 0x516. With all
that go whether the third byte is so, which it is when there was something
to send, and whether the channel wanted more flow than it has */
boolean NETWORK_OBSERVER_CALLBACK network_observer_game_channel_can_send(
	byte *observer,
	long game_channel_index,
	boolean unknown4,
	boolean unknown8,
	boolean unknownC,
	boolean *unknown10,
	boolean *wanted,
	long *size,
	long *voice_size,
	long maximum_voice_size,
	byte *voice_packet)
{
	long channel_index= NONE;
	byte *channel;
	const byte *game_channel;
	const byte *parameters;
	const byte *unknown3c;
	long long frame;
	long now;
	long refresh_rate;
	long frames;
	long period_frames= NONE;
	long bits_per_second= NONE;
	long unknown498= NONE;
	long available= NONE;
	long bytes= NONE;
	long voice_bytes;
	long voice_written= 0;
	long most;
	long index;
	real packet_rate= -1.0f;
	boolean urgent= FALSE;
	boolean limited;
	boolean has_voice;
	boolean has_flow;
	boolean wants= FALSE;
	boolean noted= FALSE;
	boolean result;

	for (index= 0; index<MAXIMUM_NETWORK_OBSERVER_CHANNELS; index++)
	{
		channel= (byte *)network_observer_channel(observer, index);
		if (*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_STATE_OFFSET)!=0 &&
			*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_CHANNEL_OFFSET)==game_channel_index)
		{
			channel_index= index;
			break;
		}
	}
	if (channel_index==NONE)
	{
		return FALSE;
	}
	channel= (byte *)network_observer_channel(observer, channel_index);
	game_channel= unknown_004d87d4+game_channel_index*NETWORK_CHANNEL_SIZE;
	if (network_observer_channel_unknown_00077940(observer, channel_index))
	{
		unknown8= TRUE;
		unknownC= TRUE;
		urgent= TRUE;
	}
	has_voice= network_observer_voice_has_packet(channel_index, FALSE, &voice_bytes);
	now= network_time_since(0);
	frame= unknown_004e6398;
	unknown3c= *(const byte **)(game_channel+NETWORK_CHANNEL_UNKNOWN3C_OFFSET);
	limited= (unknown3c!=NULL || has_voice) && !urgent;
	has_flow= channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48C_OFFSET];
	if (!urgent)
	{
		if (has_flow)
		{
			if (channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48D_OFFSET] &&
				*(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN4DC_OFFSET)==2)
			{
				wants= TRUE;
				limited= TRUE;
			}
			else if (channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48E_OFFSET])
			{
				wants= TRUE;
			}
		}
		if (limited)
		{
			if (has_flow)
			{
				packet_rate= *(const real *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN49C_OFFSET);
			}
			else if (unknown3c!=NULL)
			{
				packet_rate= *(const real *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN484_OFFSET);
			}
			else
			{
				packet_rate= unknown_004cf5d0;
			}
		}
		else if (!unknown4)
		{
			packet_rate= unknown8 ? NETWORK_OBSERVER_UNKNOWN8_PACKET_RATE : 0.0f;
		}
	}
	if (*(const long *)(game_channel+NETWORK_CHANNEL_STATE_OFFSET)==_network_channel_state5 && !urgent)
	{
		long unit= (game_channel[NETWORK_CHANNEL_UNKNOWN48_OFFSET]&_network_channel_unknown48_bit8) ?
			NETWORK_OBSERVER_WAITING_UNIT : 0;
		real waiting= (real)unknown_00089030(game_channel);

		parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
		waiting= waiting/((real)unit**(const real *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN90_OFFSET));
		if (1.0f>waiting)
		{
			real period= (*(const real *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN98_OFFSET)-
				*(const real *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN94_OFFSET))*(1.0f-waiting)+
				*(const real *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWN94_OFFSET);

			if (!(packet_rate>0.0f) || !(1.0f/period>packet_rate))
			{
				packet_rate= 1.0f/period;
			}
		}
	}
	if (limited)
	{
		if (has_flow)
		{
			bits_per_second= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_BITS_PER_SECOND_OFFSET);
			unknown498= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN498_OFFSET);
		}
		else
		{
			bits_per_second= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN480_OFFSET);
			unknown498= *(const long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN488_OFFSET);
		}
		unknownC= TRUE;
	}
	/* the game adds, takes away and multiplies as the processor does */
	frames= (long)((unsigned long)unknown_004e6398-*(const unsigned long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN478_OFFSET));
	refresh_rate= xbox_video_settings.refresh_rate;
	if (refresh_rate<=0)
	{
		refresh_rate= NETWORK_OBSERVER_DEFAULT_REFRESH_RATE;
	}
	if (frames<0)
	{
		*(long long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN478_OFFSET)= unknown_004e6398;
		*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN470_OFFSET)= now;
		frames= 0;
	}
	if (packet_rate>0.0f)
	{
		period_frames= lrintf((real)refresh_rate/packet_rate);
		if (frames<period_frames)
		{
			return FALSE;
		}
	}
	else if (packet_rate==0.0f)
	{
		return FALSE;
	}
	if (bits_per_second>=0)
	{
		available= (long)((unsigned long)frames*(unsigned long)bits_per_second)/refresh_rate/8;
		bytes= (long)((unsigned long)available-NETWORK_OBSERVER_PACKET_HEADER_BYTES);
		if (bytes<NETWORK_OBSERVER_MINIMUM_PACKET_SIZE)
		{
			return FALSE;
		}
	}
	if (period_frames>=0)
	{
		unsigned long long stamp= *(const unsigned long long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN478_OFFSET)+
			(unsigned long long)(long long)period_frames;

		*(long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN470_OFFSET)= now;
		*(unsigned long long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN478_OFFSET)= stamp;
		if ((long long)(stamp+(unsigned long long)(long long)period_frames)<=frame)
		{
			*(long long *)(channel+NETWORK_OBSERVER_CHANNEL_UNKNOWN478_OFFSET)= frame;
		}
	}
	result= TRUE;
	if (wants)
	{
		noted= TRUE;
	}
	if (unknown498>=0 && available>=0)
	{
		long waiting_bytes= unknown_00088fd0(game_channel);
		long waiting_count= unknown_00089000(game_channel);

		if (waiting_count>0 && (long)((unsigned long)waiting_bytes+(unsigned long)available)>unknown498)
		{
			long room= (long)((unsigned long)unknown498-(unsigned long)waiting_bytes);

			if (room<waiting_bytes/waiting_count)
			{
				result= FALSE;
			}
			else if ((long)((unsigned long)room-NETWORK_OBSERVER_PACKET_HEADER_BYTES)<NETWORK_OBSERVER_MINIMUM_PACKET_SIZE)
			{
				result= FALSE;
			}
			else
			{
				bytes= (long)((unsigned long)room-NETWORK_OBSERVER_PACKET_HEADER_BYTES);
			}
			if (channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48C_OFFSET])
			{
				channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN4A2_OFFSET]= TRUE;
			}
			if (!result)
			{
				return FALSE;
			}
		}
	}
	if (has_voice)
	{
		real part= 1.0f;
		long voice_room;

		if (unknown4)
		{
			parameters= *(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET);
			if (*(const byte **)(game_channel+NETWORK_CHANNEL_UNKNOWN3C_OFFSET)==NULL)
			{
				part= *(const real *)(parameters+NETWORK_OBSERVER_PARAMETERS_UNKNOWNEC_OFFSET);
			}
			else if (network_unknown_00075910(game_channel))
			{
				part= *(const real *)(*(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET)+
					NETWORK_OBSERVER_PARAMETERS_UNKNOWNE4_OFFSET);
			}
			else
			{
				part= *(const real *)(*(const byte **)(observer+NETWORK_OBSERVER_UNKNOWN10_OFFSET)+
					NETWORK_OBSERVER_PARAMETERS_UNKNOWNE8_OFFSET);
			}
		}
		voice_room= bytes>=0 ? lrintf((real)bytes*part) : NETWORK_OBSERVER_DEFAULT_VOICE_ROOM;
		if (voice_room>maximum_voice_size)
		{
			voice_room= maximum_voice_size;
		}
		if (channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN48C_OFFSET] && voice_room<voice_bytes)
		{
			channel[NETWORK_OBSERVER_CHANNEL_UNKNOWN4A1_OFFSET]= TRUE;
		}
		voice_written= network_observer_voice_build_packet(channel_index, voice_room, voice_packet);
	}
	if (bytes>=0)
	{
		bytes= (long)((unsigned long)bytes-(unsigned long)voice_written);
		if (bytes<=NETWORK_OBSERVER_MINIMUM_PACKET_SIZE)
		{
			bytes= NETWORK_OBSERVER_MINIMUM_PACKET_SIZE;
		}
	}
	else
	{
		bytes= NETWORK_OBSERVER_DEFAULT_PACKET_SIZE;
	}
	most= (long)(NETWORK_OBSERVER_MAXIMUM_PACKET_SIZE-(unsigned long)voice_written);
	most-= most%8;
	if (most<0)
	{
		return FALSE;
	}
	if (bytes>8)
	{
		bytes-= bytes%8;
	}
	if (bytes>most)
	{
		bytes= most;
	}
	*size= bytes;
	*voice_size= voice_written;
	*unknown10= unknownC;
	*wanted= noted;

	return result;
}
