/*
NETWORK_MESSAGE_HANDLER.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The functions that the messages that come by a channel of the game are
given to, when they are for the view that the game in the network has of
that channel: each one looks for the view, and if it is of the kind that
the message is for, gives it what the message has. They are just before the
ones of the channel itself (network_channel_handle_refused and the two after
it). The name of the file is a guess, and so are the names of its functions.
*/

#include "network_message_handler.h"

#include "main/main_game.h"
#include "networking/network_message_gateway.h"
#include "networking/network_observer.h"
#include "networking/network_session.h"
#include "networking/simulation_view.h"
#include "networking/simulation_world.h"

/* ---------- prototypes */

/* 0x00085e70: what a view of the third kind does with what its message has;
whether it could, most likely */
boolean __attribute__((stdcall)) simulation_view_events_receive(byte *view, long unknown0, long unknown4, const long *unknown8);

/* ---------- code */

/* the view that the game in the network has of a channel of the game, by the
number of the channel in the list; NULL if there is no game in the network,
or it is not in use, or it has none. The game has this written at the start
of each function */
static inline __attribute__((always_inline)) byte *network_message_handler_get_view(
	long channel_index)
{
	if (unknown_004cf770.unknown0 && *(void *const *)(unknown_004cf770.unknownC+SIMULATION_WORLD_STATE_OFFSET))
	{
		return simulation_world_get_channel_view(unknown_004cf770.unknownC, channel_index);
	}

	return NULL;
}

/* 0x00094910: a message with two numbers, for the view of the channel, of
whatever kind: the state of the other end of the view
(simulation_view_handle_remote_state) */
void network_message_handler_unknown_00094910(
	const long *message,
	long channel_index)
{
	byte *view= network_message_handler_get_view(channel_index);

	if (view)
	{
		simulation_view_handle_remote_state(view, message[0], message[1]);
	}
}

/* 0x00094940: a message with two numbers and something more, for a view of
the second kind or of the fourth: the players that the other console knows
of (simulation_view_set_players) */
void network_message_handler_unknown_00094940(
	const long *message,
	long channel_index)
{
	byte *view= network_message_handler_get_view(channel_index);

	if (view && ((1u<<(view[SIMULATION_VIEW_KIND_OFFSET]&31))&((1u<<_simulation_view_kind2)|(1u<<_simulation_view_kind4))))
	{
		simulation_view_set_players(view, message[0], message[1], &message[2]);
	}
}

/* 0x00094990: a message for the thing that a view of the first kind is of,
if it is going: something that came for it to take
(simulation_world_unknown_000698e0) */
void __attribute__((stdcall)) network_message_handler_unknown_00094990(
	const long *message,
	long channel_index)
{
	byte *view= network_message_handler_get_view(channel_index);

	if (view && *(const word *)(view+SIMULATION_VIEW_KIND_OFFSET)==_simulation_view_kind1)
	{
		byte *world= *(byte *const *)(view+SIMULATION_VIEW_WORLD_OFFSET);

		if (world[SIMULATION_WORLD_GOING_OFFSET])
		{
			simulation_world_unknown_000698e0(world, message);
		}
	}
}

/* 0x000949d0: a message with two numbers, a byte, one more number and
something more, for a view of the second kind: what the other console says
of its players (simulation_view_unknown_00085f50) */
void network_message_handler_unknown_000949d0(
	const long *message,
	long channel_index)
{
	byte *view= network_message_handler_get_view(channel_index);

	if (view && *(const word *)(view+SIMULATION_VIEW_KIND_OFFSET)==_simulation_view_kind2)
	{
		simulation_view_unknown_00085f50(view, *(const boolean *)&message[2], message[1], message[0], message[3],
			&message[4]);
	}
}

/* 0x00094a10: a message with a number, for a view of the first kind: the
number that the game goes on from (simulation_view_unknown_00085dc0) */
void __attribute__((stdcall)) network_message_handler_unknown_00094a10(
	const long *message,
	long channel_index)
{
	byte *view= network_message_handler_get_view(channel_index);

	if (view && *(const word *)(view+SIMULATION_VIEW_KIND_OFFSET)==_simulation_view_kind1)
	{
		simulation_view_unknown_00085dc0(view, message[0]);
	}
}

/* 0x00094a50: a message with two numbers, of which the second has to be a
number that is given, for a view of the first kind, with one more thing that
is given: a piece of the state of the game, where it goes, its size, and its
bytes (simulation_view_unknown_00085ed0) */
void __attribute__((stdcall)) network_message_handler_unknown_00094a50(
	const long *message,
	long unknown,
	long unknown2,
	long channel_index)
{
	byte *view= network_message_handler_get_view(channel_index);

	if (unknown==message[1] && view && *(const word *)(view+SIMULATION_VIEW_KIND_OFFSET)==_simulation_view_kind1)
	{
		simulation_view_unknown_00085ed0(view, message[0], message[1], (const void *)unknown2);
	}
}

/* 0x00094aa0: a message with two numbers and something more, for a view of
the third kind */
void network_message_handler_unknown_00094aa0(
	const long *message,
	long channel_index)
{
	byte *view= network_message_handler_get_view(channel_index);

	if (view && *(const word *)(view+SIMULATION_VIEW_KIND_OFFSET)==_simulation_view_kind3)
	{
		simulation_view_events_receive(view, message[0], message[1], &message[2]);
	}
}

/* ---------- the messages of the sessions

The functions from here on (0x93f60 to 0x948c0 in the game, before those of
the views) are given a message that names a session by what it starts with.
Each looks for the session among those of whoever the message came to
(network_sessions_find, with what that has at 0x14), and gives it the message
if there is one; some only if this console is the host of the session, or if
the message came by the channel of the host. The second thing that they take
is the address or the channel that the message came from. They give back
nothing that is the same on every path */

/* the session that a message is for, or NULL */
static inline __attribute__((always_inline)) byte *network_message_handler_get_session(
	const byte *owner,
	const byte *identifier)
{
	return (byte *)network_sessions_find(*(byte *const *)(owner+0x14), identifier);
}

/* whether a session is in one of the four states that only the host has */
static inline __attribute__((always_inline)) boolean network_message_handler_session_is_host(
	const byte *session)
{
	long state= *(const long *)(session+NETWORK_SESSION_STATE_OFFSET);

	return state==5 || state==6 || state==7 || state==8;
}

/* 0x00094310: a message from an address (the third thing is the address) */
void __attribute__((stdcall)) network_message_handler_unknown_00094310(
	const byte *owner,
	const byte *message,
	long unknown)
{
	byte *session= network_message_handler_get_session(owner, message);

	if (session)
	{
		network_session_unknown_0005e150((const struct transport_address *)unknown, session);
	}
}

/* 0x00094330: another message from an address (the third thing is the address) */
void __attribute__((stdcall)) network_message_handler_unknown_00094330(
	const byte *owner,
	const byte *message,
	long unknown)
{
	byte *session= network_message_handler_get_session(owner, message);

	if (session)
	{
		network_session_unknown_0005e1a0((const struct transport_address *)unknown, session);
	}
}

/* 0x00094350: a message from the channel of the host (the third thing is the channel) */
void __attribute__((stdcall)) network_message_handler_unknown_00094350(
	const byte *owner,
	const byte *message,
	long unknown)
{
	byte *session= network_message_handler_get_session(owner, message);

	if (session && network_session_channel_is_host(session, unknown))
	{
		network_session_unknown_0005e210(session, message);
	}
}

/* 0x00094610: the host says who is in the session (the third thing is the channel) */
void __attribute__((stdcall)) network_message_handler_unknown_00094610(
	const byte *owner,
	const byte *message,
	long unknown)
{
	byte *session= network_message_handler_get_session(owner, message);

	if (session && network_session_channel_is_host(session, unknown))
	{
		network_session_membership_update_received(session, message);
	}
}

/* 0x00094800: the host says how the session is set (the third thing is the channel) */
void __attribute__((stdcall)) network_message_handler_unknown_00094800(
	const byte *owner,
	const byte *message,
	long unknown)
{
	byte *session= network_message_handler_get_session(owner, message);

	if (session && network_session_channel_is_host(session, unknown))
	{
		network_session_parameters_update_received(session, message);
	}
}

/* 0x00094380: a message for the host (the third thing is the channel) */
void __attribute__((stdcall)) network_message_handler_unknown_00094380(
	const byte *owner,
	const byte *message,
	long unknown)
{
	byte *session= network_message_handler_get_session(owner, message);

	if (session && network_message_handler_session_is_host(session))
	{
		network_session_unknown_0005e340(session, unknown, message);
	}
}

/* 0x00094640: a console tells the host how it is (the third thing is the channel) */
void __attribute__((stdcall)) network_message_handler_unknown_00094640(
	const byte *owner,
	const byte *message,
	long unknown)
{
	byte *session= network_message_handler_get_session(owner, message);

	if (session && network_message_handler_session_is_host(session))
	{
		network_session_host_set_peer_properties(session, unknown, message);
	}
}

/* 0x00094680: the host is asked for another leader (the third thing is the channel) */
void __attribute__((stdcall)) network_message_handler_unknown_00094680(
	const byte *owner,
	const byte *message,
	long unknown)
{
	byte *session= network_message_handler_get_session(owner, message);

	if (session && network_message_handler_session_is_host(session))
	{
		network_session_host_change_leader(session, unknown, message);
	}
}

/* 0x000946c0: the leader asks the host to take a console out (the third thing is the channel) */
void __attribute__((stdcall)) network_message_handler_unknown_000946c0(
	const byte *owner,
	const byte *message,
	long unknown)
{
	byte *session= network_message_handler_get_session(owner, message);

	if (session && network_message_handler_session_is_host(session))
	{
		network_session_host_leader_removes_peer(session, unknown, message);
	}
}

/* 0x00094700: a console asks the host for a player more (the third thing is the channel) */
void __attribute__((stdcall)) network_message_handler_unknown_00094700(
	const byte *owner,
	const byte *message,
	long unknown)
{
	byte *session= network_message_handler_get_session(owner, message);

	if (session && network_message_handler_session_is_host(session))
	{
		network_session_host_add_peer_player(session, unknown, message);
	}
}

/* 0x00094780: a console asks the host to take a player out (the third thing is the channel) */
void __attribute__((stdcall)) network_message_handler_unknown_00094780(
	const byte *owner,
	const byte *message,
	long unknown)
{
	byte *session= network_message_handler_get_session(owner, message);

	if (session && network_message_handler_session_is_host(session))
	{
		network_session_host_remove_peer_player(session, unknown, message);
	}
}

/* 0x000947c0: a console tells the host how a player is (the third thing is the channel) */
void __attribute__((stdcall)) network_message_handler_unknown_000947c0(
	const byte *owner,
	const byte *message,
	long unknown)
{
	byte *session= network_message_handler_get_session(owner, message);

	if (session && network_message_handler_session_is_host(session))
	{
		network_session_host_set_peer_player(session, unknown, message);
	}
}

/* 0x00094830: a console asks the host to set the session (the third thing is the channel) */
void __attribute__((stdcall)) network_message_handler_unknown_00094830(
	const byte *owner,
	const byte *message,
	long unknown)
{
	byte *session= network_message_handler_get_session(owner, message);

	if (session && network_message_handler_session_is_host(session))
	{
		network_session_host_parameters_requested(message, session, unknown);
	}
}

/* 0x00094870: another thing that a console asks the host (the third thing is the channel) */
void __attribute__((stdcall)) network_message_handler_unknown_00094870(
	const byte *owner,
	const byte *message,
	long unknown)
{
	byte *session= network_message_handler_get_session(owner, message);

	if (session && network_message_handler_session_is_host(session))
	{
		network_session_host_unknown_0005dea0(message, session, unknown);
	}
}

/* 0x00094420: one more message from an address (the third thing is the address) */
void __attribute__((stdcall)) network_message_handler_unknown_00094420(
	const byte *owner,
	const byte *message,
	long unknown)
{
	byte *session= network_message_handler_get_session(owner, message);

	if (session)
	{
		network_session_unknown_0005e3f0(session, (const struct transport_address *)unknown);
	}
}

/* 0x00094440: a message from a channel (the third thing is the channel) */
void __attribute__((stdcall)) network_message_handler_unknown_00094440(
	const byte *owner,
	const byte *message,
	long unknown)
{
	byte *session= network_message_handler_get_session(owner, message);

	if (session)
	{
		network_session_unknown_0005e5b0(session, unknown);
	}
}

/* 0x00094460: a message from a channel, for the host (the third thing is the channel) */
void __attribute__((stdcall)) network_message_handler_unknown_00094460(
	const byte *owner,
	const byte *message,
	long unknown)
{
	byte *session= network_message_handler_get_session(owner, message);

	if (session && network_message_handler_session_is_host(session))
	{
		network_session_unknown_0005e640(session, unknown);
	}
}

/* 0x000945d0: the last message from an address (the third thing is the address) */
void __attribute__((stdcall)) network_message_handler_unknown_000945d0(
	const byte *owner,
	const byte *message,
	long unknown)
{
	byte *session= network_message_handler_get_session(owner, message);

	if (session)
	{
		network_session_unknown_0005ece0(session, (const struct transport_address *)unknown);
	}
}

/* 0x000945f0: the time of the session, from an address (the third thing is the address) */
void __attribute__((stdcall)) network_message_handler_unknown_000945f0(
	const byte *owner,
	const byte *message,
	long unknown)
{
	byte *session= network_message_handler_get_session(owner, message);

	if (session)
	{
		network_session_synchronize_time(session, (const struct transport_address *)unknown, message);
	}
}

/* 0x00094740: the host took a player of this console (the third thing is the channel) */
void __attribute__((stdcall)) network_message_handler_unknown_00094740(
	const byte *owner,
	const byte *message,
	long unknown)
{
	byte *session= network_message_handler_get_session(owner, message);

	if (session && network_session_channel_is_host(session, unknown))
	{
		network_session_local_player_accepted(session, message, unknown);
	}
}

/* 0x000948c0: a console answers the host (the third thing is the unknown) */
void __attribute__((stdcall)) network_message_handler_unknown_000948c0(
	const byte *owner,
	const byte *message,
	long unknown)
{
	byte *session= network_message_handler_get_session(owner, message);

	if (session && network_message_handler_session_is_host(session))
	{
		network_session_peer_answered(session, unknown, message);
	}
}

/* 0x000943d0: a message from the channel of the host, for a session in which
this console is the one at 0x44: the session goes into its eighth state
(network_session_enter_state8), with whether it has 4 at 0x4980 */
void __attribute__((stdcall)) network_message_handler_unknown_000943d0(
	const byte *owner,
	const byte *message,
	long channel)
{
	byte *session= network_message_handler_get_session(owner, message);

	if (session && network_session_channel_is_host(session, channel))
	{
		if (*(const long *)(session+NETWORK_SESSION_UNKNOWN44_OFFSET)==*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET))
		{
			network_session_enter_state8(session, *(const long *)(session+NETWORK_SESSION_UNKNOWN4980_OFFSET)==4);
		}
	}
}

/* 0x00094100: a console asks to come into a session
(network_session_host_join_requested). The message says 2 first, and names
the session after that */
void __attribute__((stdcall)) network_message_handler_unknown_00094100(
	const byte *message,
	const byte *owner,
	const struct transport_address *address)
{
	if (*(const short *)message==2)
	{
		byte *session= network_message_handler_get_session(owner, message+4);

		if (session && network_message_handler_session_is_host(session))
		{
			network_session_host_join_requested(message, session, address);
		}
	}
}

/* 0x00094270: for the host: the console that has an address, if the session
has it and it is not this one, is taken out
(network_session_remove_peer_with_message_c) */
void __attribute__((stdcall)) network_message_handler_unknown_00094270(
	const byte *owner,
	const byte *message,
	const struct transport_address *address)
{
	byte *session= network_message_handler_get_session(owner, message);

	if (session && network_message_handler_session_is_host(session))
	{
		long peer_index= network_session_find_peer_by_transport_address(session, address);

		if (peer_index!=NONE && peer_index!=*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET))
		{
			network_session_remove_peer_with_message_c(session, peer_index);
		}
	}
}

/* 0x000942d0: a session in its fourth state, that
network_session_unknown_00058d90 says yes to, is left
(network_session_disconnect) if the message comes from the console that it
has at 0x40 */
void __attribute__((stdcall)) network_message_handler_unknown_000942d0(
	const byte *owner,
	const byte *message,
	const struct transport_address *address)
{
	byte *session= network_message_handler_get_session(owner, message);

	if (session && network_session_unknown_00058d90(session))
	{
		if (*(const long *)(session+NETWORK_SESSION_STATE_OFFSET)==4)
		{
			if (network_session_find_peer_by_transport_address(session, address)==
				*(const long *)(session+NETWORK_SESSION_UNKNOWN40_OFFSET))
			{
				network_session_disconnect(session);
			}
		}
	}
}

/* what network_message_handler_unknown_00093f60 sends */
struct network_message_handler_unknown_00093f60_message
{
	short unknown0;
	short pad;
	long unknown4;
	long unknown8;
};

/* 0x00093f60: a message of the first kind goes to an address, through what
the owner has at 0xC: the short and the number that something starts with,
and 2 */
void __attribute__((stdcall)) network_message_handler_unknown_00093f60(
	const byte *unknown,
	const byte *owner,
	const struct transport_address *address)
{
	struct network_message_handler_unknown_00093f60_message message;

	message.unknown0= *(const short *)unknown;
	message.unknown4= *(const long *)(unknown+4);
	message.unknown8= 2;
	network_message_gateway_send(*(byte *const *)(owner+0xC), address, 1, sizeof(message), &message);
}
