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

#endif
