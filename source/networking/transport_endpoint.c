/*
TRANSPORT_ENDPOINT.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The sockets of the game. The name of the file is a guess, and so are the
names of its functions.
*/

#include "transport_endpoint.h"

#include <string.h>

/* ---------- constants */

enum
{
	/* of the console's libraries: memory that is taken and can be read and
	written; the options of a socket itself, and the ones that the game
	uses; and why something could not be done: there is nothing now, and the
	other console cannot be found */
	MEM_COMMIT_TOP_DOWN= 0x101000,
	PAGE_READWRITE= 4,
	SOL_SOCKET= 0xFFFF,
	SO_REUSEADDR= 0x4,
	SO_LINGER= 0x80,
	SO_BROADCAST= 0x20,
	SO_SNDBUF= 0x1001,
	SO_RCVBUF= 0x1002,
	SO_UNKNOWN4001= 0x4001,
	SD_BOTH= 2,
	WSAEWOULDBLOCK= 10035,
	WSAEHOSTUNREACH= 10065,
	/* the kinds of socket of the libraries and what they talk in; and how
	many sockets a set of them can have */
	SOCK_STREAM= 1,
	SOCK_DGRAM= 2,
	IPPROTO_TCP= 6,
	IPPROTO_UDP= 17,
	IPPROTO_VDP= 254,
	FD_SETSIZE= 64
};

/* ---------- structures */

/* a set of sockets of the libraries */
struct transport_fd_set
{
	unsigned long count;
	long sockets[FD_SETSIZE];
};

/* ---------- prototypes */

/* 0x003312cc, 0x002d1d3e, of the console's libraries: memory is taken, and
why it was not. 0x003cd1c3, 0x003cd1b4: an option of a socket is asked for,
and set. 0x003cd19e, 0x003cd193: a socket is shut down, and closed.
0x003cd211, 0x003cd23a: bytes are read from one, and written to it.
0x003cd718: why the last thing asked of the sockets was not done */
void *VirtualAlloc(void *address, unsigned long size, unsigned long type, unsigned long protection);
unsigned long GetLastError(void);
int __attribute__((stdcall)) getsockopt(long socket, int level, int name, char *value, int *length);
int __attribute__((stdcall)) setsockopt(long socket, int level, int name, const char *value, int length);
int __attribute__((stdcall)) shutdown(long socket, long how);
int __attribute__((stdcall)) closesocket(long socket);
int __attribute__((stdcall)) recv(long socket, char *bytes, int size, int flags);
int __attribute__((stdcall)) send(long socket, const char *bytes, int size, int flags);

/* 0x003cd188: a socket is made. 0x003cd1d2: it gets an address of its own.
0x003cd1dd: it starts to talk with another. 0x003cd1e8: which sockets of
some sets have something. 0x003cd71d: whether a socket is in a set.
0x003cd22b, 0x003cd254: bytes are read with the address that they came from,
and written to an address */
long __attribute__((stdcall)) socket(int family, int type, int protocol);
int __attribute__((stdcall)) bind(long socket, const struct transport_sockaddr *address, int length);
int __attribute__((stdcall)) connect(long socket, const struct transport_sockaddr *address, int length);
int __attribute__((stdcall)) select(int count, struct transport_fd_set *readable, struct transport_fd_set *writable,
	struct transport_fd_set *failed, const long *timeout);
int __attribute__((stdcall)) __WSAFDIsSet(long socket, struct transport_fd_set *set);
int __attribute__((stdcall)) recvfrom(long socket, char *bytes, int size, int flags, struct transport_sockaddr *address,
	int *length);
int __attribute__((stdcall)) sendto(long socket, const char *bytes, int size, int flags,
	const struct transport_sockaddr *address, int length);

/* ---------- globals */

/* 0x0048480c: where the value of the fifth option is, for the sockets that
get it before they talk */
long unknown_0048480c;

/* ---------- code */

/* 0x000b4d50: a socket of a kind is made, in memory of its own: it does not
have one of the libraries yet. NULL if the network is not there, or there is
no memory */
struct transport_endpoint *__attribute__((stdcall)) transport_endpoint_create(
	short type)
{
	struct transport_endpoint *endpoint= NULL;

	if (unknown_004d8b18 && unknown_004d8b19)
	{
		endpoint= (struct transport_endpoint *)VirtualAlloc(NULL, sizeof(struct transport_endpoint), MEM_COMMIT_TOP_DOWN,
			PAGE_READWRITE);
		if (!endpoint)
		{
			GetLastError();
		}
		if (endpoint)
		{
			endpoint->socket= NONE;
			endpoint->flags= 0;
			endpoint->type= type;
		}
	}

	return endpoint;
}

/* 0x000b4da0: what the libraries call an option of the game; NONE if it is
not one */
long __attribute__((stdcall)) transport_endpoint_option_name(
	short option)
{
	switch (option)
	{
	case _transport_endpoint_option0:
		return SO_REUSEADDR;
	case _transport_endpoint_option1:
		return SO_LINGER;
	case _transport_endpoint_option2:
		return SO_BROADCAST;
	case _transport_endpoint_option3:
		return SO_SNDBUF;
	case _transport_endpoint_option4:
		return SO_RCVBUF;
	case _transport_endpoint_option5:
		return SO_UNKNOWN4001;
	}

	return NONE;
}

/* 0x000b4e00: the value of an option of a socket; nothing if it cannot be
known */
long __attribute__((stdcall)) transport_endpoint_get_option(
	struct transport_endpoint *endpoint,
	long option)
{
	long value= 0;

	if (unknown_004d8b18 && unknown_004d8b19 && endpoint->socket!=NONE)
	{
		long name= transport_endpoint_option_name((short)option);

		if ((short)name!=NONE)
		{
			int length= sizeof(value);

			if (getsockopt(endpoint->socket, SOL_SOCKET, (short)name, (char *)&value, &length))
			{
				WSAGetLastError();
			}
		}
	}

	return value;
}

/* 0x000b4e70: an option of a socket gets a value; whether it did. The fifth
option is given as where its value is */
boolean __attribute__((stdcall)) transport_endpoint_set_option(
	long option,
	struct transport_endpoint *endpoint,
	long value)
{
	if (unknown_004d8b18 && unknown_004d8b19 && endpoint->socket!=NONE)
	{
		long name= transport_endpoint_option_name((short)option);

		if ((short)name!=NONE)
		{
			if (!setsockopt(endpoint->socket, SOL_SOCKET, (short)name,
				(short)option==_transport_endpoint_option5 ? (const char *)value : (const char *)&value, sizeof(value)))
			{
				return TRUE;
			}
			WSAGetLastError();
		}
	}

	return FALSE;
}

/* 0x000b4f50: a socket does not have the one of the libraries any more: if
the network is there, that one is shut down first if it talks with another,
and closed */
void transport_endpoint_close(
	struct transport_endpoint *endpoint)
{
	if (endpoint->socket!=NONE && unknown_004d8b18 && unknown_004d8b19)
	{
		if (endpoint->flags&_transport_endpoint_connected_bit)
		{
			if (shutdown(endpoint->socket, SD_BOTH))
			{
				WSAGetLastError();
			}
		}
		if (closesocket(endpoint->socket))
		{
			WSAGetLastError();
		}
	}
	endpoint->socket= NONE;
	endpoint->flags= 0;
}

/* 0x000b4fa0: bytes are read from a socket that talks with another; how
many, or why not. If there are none because the other closed, the socket
does not talk with it any more */
short __attribute__((stdcall)) transport_endpoint_read(
	struct transport_endpoint *endpoint,
	byte *bytes,
	short size)
{
	short read= 0;

	if (unknown_004d8b18 && unknown_004d8b19 && (endpoint->flags&_transport_endpoint_connected_bit))
	{
		read= (short)recv(endpoint->socket, (char *)bytes, size, 0);
		if (read==NONE)
		{
			return WSAGetLastError()==WSAEWOULDBLOCK ? _transport_endpoint_would_block : _transport_endpoint_failed;
		}
		if (read==0)
		{
			endpoint->flags&= ~_transport_endpoint_closed_bits;
		}
	}

	return read;
}

/* 0x000b5000: bytes are written to a socket that talks with another; how
many, or why not */
short __attribute__((stdcall)) transport_endpoint_write(
	struct transport_endpoint *endpoint,
	const byte *bytes,
	short size)
{
	short written= 0;

	if (unknown_004d8b18 && unknown_004d8b19 && (endpoint->flags&_transport_endpoint_connected_bit))
	{
		written= (short)send(endpoint->socket, (const char *)bytes, size, 0);
		if (written==NONE)
		{
			switch (WSAGetLastError())
			{
			case WSAEWOULDBLOCK:
				return _transport_endpoint_would_block;
			case WSAEHOSTUNREACH:
				return _transport_endpoint_unreachable;
			default:
				return _transport_endpoint_failed;
			}
		}
	}

	return written;
}

/* a word and a number with their bytes the other way round */
static inline __attribute__((always_inline)) word transport_swap_word(
	word value)
{
	return (word)((value>>8)|(value<<8));
}

static inline __attribute__((always_inline)) unsigned long transport_swap_long(
	unsigned long value)
{
	return (value>>24)|((value>>8)&0xFF00)|((value<<8)&0xFF0000)|(value<<24);
}

/* 0x000b5470: an address of the game as the libraries have it, and its
size; whether it is of four bytes or of sixteen */
boolean transport_address_to_sockaddr(
	struct transport_sockaddr *sockaddr,
	int *length,
	const struct transport_address *address)
{
	long index;

	*length= 0;
	switch (address->address_size)
	{
	case TRANSPORT_ENDPOINT_IPV6_SIZE:
		sockaddr->family= AF_INET6;
		for (index= 0; index<8; index++)
		{
			sockaddr->words[index]= transport_swap_word(address->address.words[index]);
		}
		sockaddr->port= transport_swap_word(address->port);
		*length= TRANSPORT_SOCKADDR_IPV6_SIZE;
		return TRUE;
	case TRANSPORT_ENDPOINT_IPV4_SIZE:
		sockaddr->family= AF_INET;
		sockaddr->ipv4= transport_swap_long(address->address.ipv4);
		sockaddr->port= transport_swap_word(address->port);
		*length= TRANSPORT_SOCKADDR_IPV4_SIZE;
		return TRUE;
	}

	return FALSE;
}

/* 0x000b5560: an address of the game from one of the libraries of a size;
whether the size is one of the two. If not, the address is nothing */
boolean transport_address_from_sockaddr(
	struct transport_address *address,
	long length,
	const struct transport_sockaddr *sockaddr)
{
	long index;

	switch (length)
	{
	case TRANSPORT_SOCKADDR_IPV4_SIZE:
		address->address.ipv4= transport_swap_long(sockaddr->ipv4);
		address->address_size= TRANSPORT_ENDPOINT_IPV4_SIZE;
		address->port= transport_swap_word(sockaddr->port);
		return TRUE;
	case TRANSPORT_SOCKADDR_IPV6_SIZE:
		for (index= 0; index<8; index++)
		{
			address->address.words[index]= transport_swap_word(sockaddr->words[index]);
		}
		address->address_size= TRANSPORT_ENDPOINT_IPV6_SIZE;
		address->port= transport_swap_word(sockaddr->port);
		return TRUE;
	}
	memset(address, 0, sizeof(struct transport_address));

	return FALSE;
}

/* 0x000b53e0: a socket has one of the libraries, of its kind and of the
family of an address, if it did not have one yet; whether it does. If the
fourth option of it is not nothing, it has the bit that says that it waits */
boolean __attribute__((stdcall)) transport_endpoint_open(
	struct transport_endpoint *endpoint,
	const struct transport_address *address)
{
	if (endpoint->socket==NONE)
	{
		int type= 0;
		int protocol= 0;
		int family= NONE;

		switch (endpoint->type)
		{
		case _transport_endpoint_type_udp:
			protocol= IPPROTO_UDP;
			type= SOCK_DGRAM;
			break;
		case _transport_endpoint_type_vdp:
			protocol= IPPROTO_VDP;
			type= SOCK_DGRAM;
			break;
		case _transport_endpoint_type_tcp:
			type= SOCK_STREAM;
			protocol= IPPROTO_TCP;
			break;
		}
		switch (address->address_size)
		{
		case TRANSPORT_ENDPOINT_IPV4_SIZE:
			family= AF_INET;
			break;
		case TRANSPORT_ENDPOINT_IPV6_SIZE:
			family= AF_INET6;
			break;
		}
		endpoint->socket= socket(family, type, protocol);
		if (endpoint->socket==NONE)
		{
			WSAGetLastError();

			return FALSE;
		}
	}
	if (transport_endpoint_get_option(endpoint, _transport_endpoint_option4))
	{
		endpoint->flags|= _transport_endpoint_blocking_bit;
	}

	return TRUE;
}

/* 0x000b4ed0: a socket gets an address of its own, and is opened for it if
it was not; whether it did */
boolean __attribute__((stdcall)) transport_endpoint_bind(
	const struct transport_address *address,
	struct transport_endpoint *endpoint)
{
	struct transport_sockaddr sockaddr;
	int length;

	if (unknown_004d8b18 && unknown_004d8b19 && transport_address_to_sockaddr(&sockaddr, &length, address) &&
		transport_endpoint_open(endpoint, address))
	{
		if (!bind(endpoint->socket, &sockaddr, length))
		{
			return TRUE;
		}
		WSAGetLastError();
	}

	return FALSE;
}

/* 0x000b5060: bytes that came for a socket are read, with the address that
they came from; how many, or why not */
short __attribute__((stdcall)) transport_endpoint_read_from(
	struct transport_endpoint *endpoint,
	byte *bytes,
	short size,
	struct transport_address *address)
{
	struct transport_sockaddr sockaddr;
	int length= sizeof(sockaddr);
	short read= _transport_endpoint_failed;

	memset(&sockaddr, 0, sizeof(sockaddr));
	if (unknown_004d8b18 && unknown_004d8b19)
	{
		read= (short)recvfrom(endpoint->socket, (char *)bytes, size, 0, &sockaddr, &length);
		if (read==NONE)
		{
			return WSAGetLastError()==WSAEWOULDBLOCK ? _transport_endpoint_would_block : _transport_endpoint_failed;
		}
		transport_address_from_sockaddr(address, length, &sockaddr);
	}

	return read;
}

/* 0x000b5110: bytes are written by a socket to an address; how many, or why
not */
short __attribute__((stdcall)) transport_endpoint_write_to(
	const struct transport_address *address,
	struct transport_endpoint *endpoint,
	const byte *bytes,
	short size)
{
	struct transport_sockaddr sockaddr;
	int length;
	short written= _transport_endpoint_failed;

	if (unknown_004d8b18 && unknown_004d8b19 && transport_address_to_sockaddr(&sockaddr, &length, address))
	{
		written= (short)sendto(endpoint->socket, (const char *)bytes, size, 0, &sockaddr, length);
		if (written==NONE)
		{
			switch (WSAGetLastError())
			{
			case WSAEWOULDBLOCK:
				return _transport_endpoint_would_block;
			case WSAEHOSTUNREACH:
				return _transport_endpoint_unreachable;
			default:
				return _transport_endpoint_failed;
			}
		}
	}

	return written;
}

/* 0x000b51b0: a socket starts to talk with an address, and is opened for it
if it was not; whether it did. It does not wait: if it talks already, it has
the bits that say so, and if not it is so too, and transport_endpoint_connect_finished
says when it does */
boolean __attribute__((stdcall)) transport_endpoint_connect(
	const struct transport_address *address,
	struct transport_endpoint *endpoint)
{
	struct transport_sockaddr sockaddr;
	int length;

	if (unknown_004d8b18 && unknown_004d8b19 && transport_address_to_sockaddr(&sockaddr, &length, address) &&
		transport_endpoint_open(endpoint, address))
	{
		if (endpoint->flags&_transport_endpoint_unknown40_bit)
		{
			transport_endpoint_set_option(_transport_endpoint_option5, endpoint, unknown_0048480c);
		}
		if (!transport_endpoint_set_nonblocking(endpoint))
		{
			WSAGetLastError();

			return FALSE;
		}
		if (!connect(endpoint->socket, &sockaddr, length))
		{
			endpoint->flags|= _transport_endpoint_connected_bits;

			return TRUE;
		}
		if (WSAGetLastError()==WSAEWOULDBLOCK)
		{
			return TRUE;
		}
	}

	return FALSE;
}

/* a set of sockets has one more, if it is not in it and there is room */
static inline __attribute__((always_inline)) void transport_fd_set_add(
	struct transport_fd_set *set,
	long socket_)
{
	unsigned long index;

	for (index= 0; index<set->count; index++)
	{
		if (set->sockets[index]==socket_)
		{
			break;
		}
	}
	if (index==set->count && set->count<FD_SETSIZE)
	{
		set->sockets[index]= socket_;
		set->count++;
	}
}

/* 0x000b52b0: whether a socket that started to talk with an address could
still do so, and whether it does now: it has the bits that say so then */
boolean __attribute__((stdcall)) transport_endpoint_connect_finished(
	struct transport_endpoint *endpoint,
	boolean *connected)
{
	struct transport_fd_set writable;
	struct transport_fd_set failed;
	long timeout[2];
	boolean result= FALSE;

	memset(&writable, 0, sizeof(writable));
	memset(&failed, 0, sizeof(failed));
	*connected= FALSE;
	timeout[0]= 0;
	timeout[1]= 0;
	if (unknown_004d8b18 && unknown_004d8b19 && endpoint->socket!=NONE)
	{
		transport_fd_set_add(&writable, endpoint->socket);
		transport_fd_set_add(&failed, endpoint->socket);
		if (select(0, NULL, &writable, &failed, timeout)==NONE)
		{
			WSAGetLastError();

			return FALSE;
		}
		if (!__WSAFDIsSet(endpoint->socket, &failed))
		{
			result= TRUE;
			if (__WSAFDIsSet(endpoint->socket, &writable))
			{
				endpoint->flags|= _transport_endpoint_connected_bits;
				*connected= TRUE;
			}
		}
	}

	return result;
}
