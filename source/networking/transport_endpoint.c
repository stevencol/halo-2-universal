/*
TRANSPORT_ENDPOINT.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The sockets of the game. The name of the file is a guess, and so are the
names of its functions.
*/

#include "transport_endpoint.h"

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
	WSAEHOSTUNREACH= 10065
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
int WSAGetLastError(void);

/* ---------- globals */

/* 0x004d8b18, 0x004d8b19, of network_session.c: the network of the console
is there, and so is the one of the game */
extern boolean unknown_004d8b18;
extern boolean unknown_004d8b19;

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
