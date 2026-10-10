/*
TRANSPORT_ENDPOINT.H

The sockets of the game: one of the console's libraries, with what the game
keeps of it. The name of the file is a guess, and so are the names of its
functions.
*/

#ifndef __TRANSPORT_ENDPOINT_H
#define __TRANSPORT_ENDPOINT_H

#include "cseries/cseries.h"

/* ---------- constants */

enum
{
	/* the bits of a socket: it talks with another, which is what is read
	and written by without an address; and the ones that it loses, with
	that one, when the other closes */
	_transport_endpoint_connected_bit= 0x1,
	_transport_endpoint_closed_bits= 0x25,
	/* the options of a socket, as the game names them: the fifth is given
	as where its value is, and the others as the value */
	_transport_endpoint_option0= 0,
	_transport_endpoint_option1,
	_transport_endpoint_option2,
	_transport_endpoint_option3,
	_transport_endpoint_option4,
	_transport_endpoint_option5,
	NUMBER_OF_TRANSPORT_ENDPOINT_OPTIONS,
	/* what reading and writing say when they could not: there is nothing
	now, the other console cannot be found, and anything else */
	_transport_endpoint_unreachable= -1,
	_transport_endpoint_would_block= -2,
	_transport_endpoint_failed= -3
};

/* ---------- structures */

/* in memory of its own: the socket of the console's libraries, or NONE; its
bits; and its kind */
struct transport_endpoint
{
	long socket;
	word flags;
	short type;
};

/* ---------- prototypes/TRANSPORT_ENDPOINT.C */

struct transport_endpoint *__attribute__((stdcall)) transport_endpoint_create(short type);
long __attribute__((stdcall)) transport_endpoint_option_name(short option);
long __attribute__((stdcall)) transport_endpoint_get_option(struct transport_endpoint *endpoint, long option);
boolean __attribute__((stdcall)) transport_endpoint_set_option(long option, struct transport_endpoint *endpoint,
	long value);
void transport_endpoint_close(struct transport_endpoint *endpoint);
short __attribute__((stdcall)) transport_endpoint_read(struct transport_endpoint *endpoint, byte *bytes, short size);
short __attribute__((stdcall)) transport_endpoint_write(struct transport_endpoint *endpoint, const byte *bytes,
	short size);

#endif
