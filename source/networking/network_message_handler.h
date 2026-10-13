/*
NETWORK_MESSAGE_HANDLER.H

The functions that the messages that come by a channel of the game are
given to, when they are for the view that the game in the network has of
that channel. The name of the file is a guess, and so are the names of its
functions.
*/

#ifndef __NETWORK_MESSAGE_HANDLER_H
#define __NETWORK_MESSAGE_HANDLER_H

#include "cseries/cseries.h"

/* ---------- constants */

enum
{
	/* the kinds of view that there are functions for */
	_simulation_view_kind1= 1,
	_simulation_view_kind2= 2,
	_simulation_view_kind3= 3,
	_simulation_view_kind4= 4
};

/* ---------- prototypes/NETWORK_MESSAGE_HANDLER.C */

void network_message_handler_unknown_00094910(const long *message, long channel_index);
void network_message_handler_unknown_00094940(const long *message, long channel_index);
void __attribute__((stdcall)) network_message_handler_unknown_00094990(const long *message, long channel_index);
void network_message_handler_unknown_000949d0(const long *message, long channel_index);
void __attribute__((stdcall)) network_message_handler_unknown_00094a10(const long *message, long channel_index);
void __attribute__((stdcall)) network_message_handler_unknown_00094a50(const long *message, long unknown,
	long unknown2, long channel_index);
void network_message_handler_unknown_00094aa0(const long *message, long channel_index);

struct transport_address;

void __attribute__((stdcall)) network_message_handler_unknown_00094310(const byte *owner, const byte *message, long unknown);
void __attribute__((stdcall)) network_message_handler_unknown_00094330(const byte *owner, const byte *message, long unknown);
void __attribute__((stdcall)) network_message_handler_unknown_00094350(const byte *owner, const byte *message, long unknown);
void __attribute__((stdcall)) network_message_handler_unknown_00094610(const byte *owner, const byte *message, long unknown);
void __attribute__((stdcall)) network_message_handler_unknown_00094800(const byte *owner, const byte *message, long unknown);
void __attribute__((stdcall)) network_message_handler_unknown_00094380(const byte *owner, const byte *message, long unknown);
void __attribute__((stdcall)) network_message_handler_unknown_00094640(const byte *owner, const byte *message, long unknown);
void __attribute__((stdcall)) network_message_handler_unknown_00094680(const byte *owner, const byte *message, long unknown);
void __attribute__((stdcall)) network_message_handler_unknown_000946c0(const byte *owner, const byte *message, long unknown);
void __attribute__((stdcall)) network_message_handler_unknown_00094700(const byte *owner, const byte *message, long unknown);
void __attribute__((stdcall)) network_message_handler_unknown_00094780(const byte *owner, const byte *message, long unknown);
void __attribute__((stdcall)) network_message_handler_unknown_000947c0(const byte *owner, const byte *message, long unknown);
void __attribute__((stdcall)) network_message_handler_unknown_00094830(const byte *owner, const byte *message, long unknown);
void __attribute__((stdcall)) network_message_handler_unknown_00094870(const byte *owner, const byte *message, long unknown);
void __attribute__((stdcall)) network_message_handler_unknown_00094420(const byte *owner, const byte *message, long unknown);
void __attribute__((stdcall)) network_message_handler_unknown_00094440(const byte *owner, const byte *message, long unknown);
void __attribute__((stdcall)) network_message_handler_unknown_00094460(const byte *owner, const byte *message, long unknown);
void __attribute__((stdcall)) network_message_handler_unknown_000945d0(const byte *owner, const byte *message, long unknown);
void __attribute__((stdcall)) network_message_handler_unknown_000945f0(const byte *owner, const byte *message, long unknown);
void __attribute__((stdcall)) network_message_handler_unknown_00094740(const byte *owner, const byte *message, long unknown);
void __attribute__((stdcall)) network_message_handler_unknown_000948c0(const byte *owner, const byte *message, long unknown);
void __attribute__((stdcall)) network_message_handler_unknown_000943d0(const byte *owner, const byte *message, long channel);
void __attribute__((stdcall)) network_message_handler_unknown_00094100(const byte *message, const byte *owner,
	const struct transport_address *address);
void __attribute__((stdcall)) network_message_handler_unknown_00094270(const byte *owner, const byte *message,
	const struct transport_address *address);
void __attribute__((stdcall)) network_message_handler_unknown_000942d0(const byte *owner, const byte *message,
	const struct transport_address *address);
void __attribute__((stdcall)) network_message_handler_unknown_00093f60(const byte *unknown, const byte *owner,
	const struct transport_address *address);

#endif
