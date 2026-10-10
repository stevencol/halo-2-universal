/*
TRANSPORT_ENDPOINT.H

The sockets of the game: one of the console's libraries, with what the game
keeps of it. The name of the file is a guess, and so are the names of its
functions.
*/

#ifndef __TRANSPORT_ENDPOINT_H
#define __TRANSPORT_ENDPOINT_H

#include "cseries/cseries.h"
#include "networking/transport_security.h"

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
	_transport_endpoint_failed= -3,
	/* more bits of a socket: the one of the libraries waits when it is
	asked for what is not there yet, which the game does not want; one more
	that it has when it talks with another; and one that says that it gets
	the fifth option before it talks */
	_transport_endpoint_blocking_bit= 0x10,
	_transport_endpoint_connected_bits= 0x21,
	_transport_endpoint_unknown40_bit= 0x40,
	/* the kinds of socket, and the sizes of the two kinds of address */
	_transport_endpoint_type_udp= 2,
	_transport_endpoint_type_vdp= 3,
	_transport_endpoint_type_tcp= 4,
	TRANSPORT_ENDPOINT_IPV4_SIZE= 4,
	TRANSPORT_ENDPOINT_IPV6_SIZE= 16,
	/* of the console's libraries: the two families of address, and the size
	of what says an address of each; and what is asked of a socket so that
	it does not wait */
	AF_INET= 2,
	AF_INET6= 23,
	TRANSPORT_SOCKADDR_IPV4_SIZE= 0x10,
	TRANSPORT_SOCKADDR_IPV6_SIZE= 0x1C,
	FIONBIO= 0x8004667E
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

/* an address as the console's libraries have it: its family, its port and,
for the first family, its four bytes; for the second, its sixteen. The port
and the bytes have their most significant byte first */
struct transport_sockaddr
{
	short family;
	word port;
	unsigned long ipv4;
	word words[8];
	unsigned long scope;
};

typedef char verify_transport_sockaddr_size[sizeof(struct transport_sockaddr) == TRANSPORT_SOCKADDR_IPV6_SIZE ? 1 : -1];

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
boolean __attribute__((stdcall)) transport_endpoint_bind(const struct transport_address *address,
	struct transport_endpoint *endpoint);
short __attribute__((stdcall)) transport_endpoint_read_from(struct transport_endpoint *endpoint, byte *bytes,
	short size, struct transport_address *address);
short __attribute__((stdcall)) transport_endpoint_write_to(const struct transport_address *address,
	struct transport_endpoint *endpoint, const byte *bytes, short size);
boolean __attribute__((stdcall)) transport_endpoint_connect(const struct transport_address *address,
	struct transport_endpoint *endpoint);
boolean __attribute__((stdcall)) transport_endpoint_connect_finished(struct transport_endpoint *endpoint,
	boolean *connected);
boolean __attribute__((stdcall)) transport_endpoint_open(struct transport_endpoint *endpoint,
	const struct transport_address *address);
boolean transport_address_to_sockaddr(struct transport_sockaddr *sockaddr, int *length,
	const struct transport_address *address);
boolean transport_address_from_sockaddr(struct transport_address *address, long length,
	const struct transport_sockaddr *sockaddr);

/* 0x003cd1a9, 0x003cd718, of the console's libraries: something is asked of
a socket; and why the last thing asked of the sockets was not done */
int __attribute__((stdcall)) ioctlsocket(long socket, long command, unsigned long *value);
int WSAGetLastError(void);

/* ---------- globals */

/* 0x004d8b18, 0x004d8b19, of network_session.c: the network of the console
is there, and so is the one of the game */
extern boolean unknown_004d8b18;
extern boolean unknown_004d8b19;

/* ---------- inline code */

/* a socket does not wait any more; whether it is so. The game has this
where it is done. Nothing is asked if the network is not there */
static inline __attribute__((always_inline)) boolean transport_endpoint_set_nonblocking(
	struct transport_endpoint *endpoint)
{
	if (unknown_004d8b18 && unknown_004d8b19)
	{
		if (endpoint->socket==NONE)
		{
			return FALSE;
		}
		if (endpoint->flags&_transport_endpoint_blocking_bit)
		{
			unsigned long value= 1;

			if (ioctlsocket(endpoint->socket, (long)FIONBIO, &value))
			{
				WSAGetLastError();

				return FALSE;
			}
			endpoint->flags&= ~_transport_endpoint_blocking_bit;
		}
	}

	return TRUE;
}

#endif
