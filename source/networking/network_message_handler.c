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
#include "networking/simulation_world.h"

/* ---------- prototypes */

/* 0x00085b20, 0x00085d00, 0x00085dc0, 0x00085e70, 0x00085ed0 and
0x00085f50: what a view does with what each message has; whether it could,
most likely */
boolean unknown_00085b20(byte *view, long unknown0, long unknown4);
boolean __attribute__((stdcall)) unknown_00085d00(byte *view, long unknown0, long unknown4, const long *unknown8);
boolean __attribute__((stdcall)) unknown_00085dc0(byte *view, long unknown0);
boolean __attribute__((stdcall)) unknown_00085e70(byte *view, long unknown0, long unknown4, const long *unknown8);
boolean __attribute__((stdcall)) unknown_00085ed0(byte *view, long unknown0, long unknown4, long unknown);
boolean __attribute__((stdcall)) unknown_00085f50(byte *view, boolean unknown8, long unknown4, long unknown0,
	long unknownC, const long *unknown10);

/* ---------- code */

/* the view that the game in the network has of a channel of the game, by the
number of the channel in the list; NULL if there is no game in the network,
or it is not going, or it has none. The game has this written at the start
of each function */
static inline __attribute__((always_inline)) byte *network_message_handler_get_view(
	long channel_index)
{
	if (unknown_004cf770.unknown0 && *(void *const *)(unknown_004cf770.unknownC+SIMULATION_WORLD_UNKNOWN8_OFFSET))
	{
		return simulation_world_get_channel_view(unknown_004cf770.unknownC, channel_index);
	}

	return NULL;
}

/* 0x00094910: a message with two numbers, for the view of the channel, of
whatever kind */
void network_message_handler_unknown_00094910(
	const long *message,
	long channel_index)
{
	byte *view= network_message_handler_get_view(channel_index);

	if (view)
	{
		unknown_00085b20(view, message[0], message[1]);
	}
}

/* 0x00094940: a message with two numbers and something more, for a view of
the second kind or of the fourth */
void network_message_handler_unknown_00094940(
	const long *message,
	long channel_index)
{
	byte *view= network_message_handler_get_view(channel_index);

	if (view && ((1u<<(view[SIMULATION_VIEW_KIND_OFFSET]&31))&((1u<<_simulation_view_kind2)|(1u<<_simulation_view_kind4))))
	{
		unknown_00085d00(view, message[0], message[1], &message[2]);
	}
}

/* 0x00094990: a message for what a view of the first kind has at 0xC, if a
byte of that says so */
void __attribute__((stdcall)) network_message_handler_unknown_00094990(
	const long *message,
	long channel_index)
{
	byte *view= network_message_handler_get_view(channel_index);

	if (view && *(const word *)(view+SIMULATION_VIEW_KIND_OFFSET)==_simulation_view_kind1)
	{
		byte *unknown= *(byte *const *)(view+SIMULATION_VIEW_UNKNOWNC_OFFSET);

		if (unknown[SIMULATION_VIEW_UNKNOWNC_UNKNOWN24_OFFSET])
		{
			simulation_world_unknown_000698e0(unknown, message);
		}
	}
}

/* 0x000949d0: a message with two numbers, a byte, one more number and
something more, for a view of the second kind */
void network_message_handler_unknown_000949d0(
	const long *message,
	long channel_index)
{
	byte *view= network_message_handler_get_view(channel_index);

	if (view && *(const word *)(view+SIMULATION_VIEW_KIND_OFFSET)==_simulation_view_kind2)
	{
		unknown_00085f50(view, *(const boolean *)&message[2], message[1], message[0], message[3], &message[4]);
	}
}

/* 0x00094a10: a message with a number, for a view of the first kind */
void __attribute__((stdcall)) network_message_handler_unknown_00094a10(
	const long *message,
	long channel_index)
{
	byte *view= network_message_handler_get_view(channel_index);

	if (view && *(const word *)(view+SIMULATION_VIEW_KIND_OFFSET)==_simulation_view_kind1)
	{
		unknown_00085dc0(view, message[0]);
	}
}

/* 0x00094a50: a message with two numbers, of which the second has to be a
number that is given, for a view of the first kind, with one more thing that
is given */
void __attribute__((stdcall)) network_message_handler_unknown_00094a50(
	const long *message,
	long unknown,
	long unknown2,
	long channel_index)
{
	byte *view= network_message_handler_get_view(channel_index);

	if (unknown==message[1] && view && *(const word *)(view+SIMULATION_VIEW_KIND_OFFSET)==_simulation_view_kind1)
	{
		unknown_00085ed0(view, message[0], message[1], unknown2);
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
		unknown_00085e70(view, message[0], message[1], &message[2]);
	}
}
