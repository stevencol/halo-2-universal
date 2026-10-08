/*
TRANSPORT_SECURITY.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

What this console is known by in the network, and what each session is known
by and its key. The name of the file is a guess, and so are the names of its
functions: the game has them from 0x0007a8a0 on.
*/

#include "transport_security.h"

#include <string.h>

/* ---------- prototypes of functions of the console's libraries */

/* 0x003cd0cb: makes what a session is known by, and its key; zero if it
could */
long __attribute__((stdcall)) XNetCreateKey(byte *identifier, byte *key);

/* 0x003cd0d6: the library is told what a session is known by and its key;
zero if it could */
long __attribute__((stdcall)) XNetRegisterKey(const byte *identifier, const byte *key);

/* 0x003cd17d: what this console is known by in the network, and what there
is of it: see _xnet_get_xnaddr_pending */
unsigned long __attribute__((stdcall)) XNetGetTitleXnAddr(byte *address);

/* 0x003cd0c0: bytes made at random */
long __attribute__((stdcall)) XNetRandom(byte *buffer, unsigned long size);

/* 0x003cd0ec: the address that goes with what a console is known by, for a
session; zero if it could */
long __attribute__((stdcall)) XNetXnAddrToInAddr(const byte *address, const byte *identifier,
	unsigned long *in_address);

/* 0x003cd34f, 0x003cd35a: starts talking with an address, zero if it could;
and how that goes, 0 to 3 */
long __attribute__((stdcall)) XNetConnect(unsigned long in_address);
unsigned long __attribute__((stdcall)) XNetGetConnectStatus(unsigned long in_address);

/* 0x003314b0 */
long GetTickCount(void);

/* of the C library */
long time(long *timer);
int rand(void);

/* ---------- globals */

/* 0x004d8b18, 0x004d8b19, of network_session.c: with both, the console is
asked what it is known by */
extern boolean unknown_004d8b18;
extern boolean unknown_004d8b19;

/* 0x0046725c: eight bytes that what is made at random to know something by
cannot be: all zeros */
static const byte transport_security_no_identifier[NETWORK_SESSION_IDENTIFIER_SIZE]= { 0 };
const byte *unknown_0046725c= transport_security_no_identifier;

/* 0x004cf790: whether the console's library makes the bytes at random.
0x004cf7cc: six bytes of what this console is known by */
boolean unknown_004cf790;
byte unknown_004cf7cc[TRANSPORT_SECURITY_ADDRESS_UNKNOWNA_SIZE];

/* 0x004cf791: whether the console has been asked what it is known by in the
network. 0x004cf792, 0x004cf793: whether it is known, and what it is.
0x004cf7b8: its address, as the game has one */
boolean unknown_004cf791;
boolean unknown_004cf792;
byte unknown_004cf793[NETWORK_SESSION_ADDRESS_SIZE];
struct transport_address unknown_004cf7b8;

/* 0x004cf7d4: what each session is known by in the network, and its key; by
the number that a session has at 0x38. 0x004cf8d4: with it, the console's
library is told when a session starts and stops being known by something */
struct network_session_key unknown_004cf7d4[MAXIMUM_NETWORK_SESSION_KEYS];
boolean unknown_004cf8d4;

/* ---------- code */

/* the four bytes of a number the other way round */
static inline __attribute__((always_inline)) unsigned long transport_security_swap(
	unsigned long value)
{
	return (value>>24) | ((value>>8)&0xFF00) | ((value<<8)&0xFF0000) | (value<<24);
}

/* 0x0007a8a0: makes what a session is known by and its key, and keeps them
at a place; whether it could. With a first thing that is not zero they are
all zeros; with zero, the console's library makes them, and the top bit of
the first byte is what the last thing says. If the library cannot make them,
the game goes on with what there was in the stack */
boolean TRANSPORT_SECURITY_CALLBACK transport_security_create_key(
	long unknown4,
	long index,
	boolean unknown)
{
	byte identifier[NETWORK_SESSION_IDENTIFIER_SIZE];
	byte key[NETWORK_SESSION_KEY_SIZE];

	if (unknown4)
	{
		memset(identifier, 0, sizeof(identifier));
		memset(key, 0, sizeof(key));
	}
	else if (XNetCreateKey(identifier, key)==0)
	{
		identifier[0]= (unknown ? TRANSPORT_SECURITY_IDENTIFIER_UNKNOWN_BIT : 0) |
			(identifier[0]&TRANSPORT_SECURITY_IDENTIFIER_KEPT_BITS);
	}

	return transport_security_set_key(index, key, unknown4, TRUE, identifier)!=FALSE;
}

/* 0x0007a920: keeps what a session is known by and its key at a place, with
two things; whether it could. What was there is taken out of the console's
library first; and the library is told of the new ones if the first of the
two things is zero: if it cannot take them, nothing is kept */
boolean TRANSPORT_SECURITY_CALLBACK transport_security_set_key(
	long index,
	const byte *key,
	long unknown4,
	boolean unknown1,
	const byte *identifier)
{
	struct network_session_key *entry= &unknown_004cf7d4[index];

	if (entry->valid)
	{
		XNetUnregisterKey(entry->identifier);
		entry->valid= FALSE;
	}
	if (unknown4==0 && XNetRegisterKey(identifier, key)!=0)
	{
		return FALSE;
	}
	memcpy(entry->identifier, identifier, sizeof(entry->identifier));
	memcpy(entry->key, key, sizeof(entry->key));
	entry->unknown1= unknown1;
	entry->unknown4= unknown4;
	entry->valid= TRUE;

	return TRUE;
}

/* 0x0007a9a0: what this console is known by in the network is not known,
and has to be asked for again. What it is given is not used */
void TRANSPORT_SECURITY_CALLBACK transport_security_forget_address(
	long unused)
{
	(void)unused;
	unknown_004cf791= FALSE;
	unknown_004cf792= FALSE;
}

/* 0x0007a9b0: asks the console what it is known by in the network, if it
has not been asked yet; whether it has been by the end. While the console
says not yet, it will be asked again; if it says that there is nothing, it is
not known but it has been asked. In the game's address go the first four
bytes, the other way round, if they are an address */
boolean transport_security_get_address(
	void)
{
	if (!unknown_004cf791)
	{
		byte address[NETWORK_SESSION_ADDRESS_SIZE];
		unsigned long status;

		memset(unknown_004cf793, 0, sizeof(unknown_004cf793));
		memset(&unknown_004cf7b8, 0, sizeof(unknown_004cf7b8));
		status= XNetGetTitleXnAddr(address);
		if (status!=_xnet_get_xnaddr_pending)
		{
			if (status==_xnet_get_xnaddr_none)
			{
				unknown_004cf792= FALSE;
			}
			else
			{
				unknown_004cf792= TRUE;
				memcpy(unknown_004cf793, address, sizeof(unknown_004cf793));
				if (status&XNET_GET_XNADDR_ADDRESS_FLAGS)
				{
					unsigned long ipv4= *(const unsigned long *)address;

					unknown_004cf7b8.address_size= TRANSPORT_ADDRESS_IPV4_SIZE;
					unknown_004cf7b8.address.ipv4= (ipv4>>24) | ((ipv4>>8)&0xFF00) | ((ipv4<<8)&0xFF0000) | (ipv4<<24);
					unknown_004cf7b8.port= 0;
				}
				unknown_004cf791= TRUE;

				return TRUE;
			}
		}
	}

	return unknown_004cf791;
}

/* 0x0007aaa0: what this console is known by in the network and its address
as the game has one, each if there is where to put it; whether they are
known. The console is asked first, with the two bytes that say to */
boolean transport_security_get_addresses(
	byte *address,
	struct transport_address *transport_address)
{
	if (unknown_004d8b18 && unknown_004d8b19)
	{
		transport_security_get_address();
	}
	if (address)
	{
		memcpy(address, unknown_004cf793, sizeof(unknown_004cf793));
	}
	if (transport_address)
	{
		*transport_address= unknown_004cf7b8;
	}

	return unknown_004cf792;
}

/* 0x0007ab10: the address of the game of a console that is known by
something, for a session: by the place of the session, or with NONE by the
first place that gives one; whether there is one */
boolean TRANSPORT_SECURITY_CALLBACK transport_security_address_from_peer_address(
	long index,
	struct transport_address *address,
	const byte *peer_address,
	long unknown,
	long port)
{
	if (index==NONE)
	{
		for (index= 0; index<MAXIMUM_NETWORK_SESSION_KEYS; index++)
		{
			if (transport_security_address_from_key(index, unknown, address, peer_address, port))
			{
				return TRUE;
			}
		}

		return FALSE;
	}

	return transport_security_address_from_key(index, unknown, address, peer_address, port);
}

/* 0x0007acc0: starts talking with an address of the game, if it is one that
the console's library gave; whether it could */
boolean transport_address_connect(
	const struct transport_address *address)
{
	unsigned long in_address;

	if (transport_address_get_secure_in_address(address, &in_address) && XNetConnect(in_address)==0)
	{
		return TRUE;
	}

	return FALSE;
}

/* 0x0007acf0: how talking with an address of the game goes, as the
console's library says: 0 to 3, and 4 if it says something else or the
address is not one that it gave */
long transport_address_get_connect_status(
	const struct transport_address *address)
{
	unsigned long in_address;

	if (transport_address_get_secure_in_address(address, &in_address))
	{
		unsigned long status= XNetGetConnectStatus(in_address);

		if (status<=MAXIMUM_XNET_CONNECT_STATUS)
		{
			return status;
		}
	}

	return _transport_address_connect_status_none;
}

/* 0x0007ad50: eight bytes made at random that are not all zeros */
void transport_security_random_identifier(
	byte *identifier)
{
	do
	{
		transport_security_random_bytes(NETWORK_SESSION_IDENTIFIER_SIZE, identifier);
	}
	while (memcmp(unknown_0046725c, identifier, NETWORK_SESSION_IDENTIFIER_SIZE)==0);
}

/* 0x0007ad80: fills a buffer with bytes made at random: by the console's
library if unknown_004cf790 says so, and if not from the time, rand and
GetTickCount */
void transport_security_random_bytes(
	long size,
	void *buffer)
{
	if (unknown_004cf790)
	{
		XNetRandom(buffer, size);
	}
	else
	{
		unsigned long seed= time(NULL);
		unsigned long other= rand();
		long index;

		other^= GetTickCount();
		seed^= other;
		for (index= 0; index<size; index++)
		{
			seed= seed*TRANSPORT_SECURITY_RANDOM_MULTIPLIER+TRANSPORT_SECURITY_RANDOM_INCREMENT;
			((byte *)buffer)[index]= (byte)(seed>>24);
		}
	}
}

/* 0x0007adf0: the address of the game of a console that is known by
something, for the session of a place: the console's library gives it from
what the session is known by. Only if the second thing is zero, and the place
has something that the library was told of. The port is the one that it is
given; whether the address is one */
boolean TRANSPORT_SECURITY_CALLBACK transport_security_address_from_key(
	long index,
	long unknown,
	struct transport_address *address,
	const byte *peer_address,
	long port)
{
	const struct network_session_key *entry= &unknown_004cf7d4[index];
	unsigned long in_address;

	if (unknown || !entry->valid || entry->unknown4)
	{
		return FALSE;
	}
	if (XNetXnAddrToInAddr(peer_address, entry->identifier, &in_address)!=0)
	{
		return FALSE;
	}
	address->address.ipv4= transport_security_swap(in_address);
	address->port= (word)port;
	address->address_size= TRANSPORT_ADDRESS_IPV4_SIZE;

	return transport_address_valid(address);
}

/* 0x0007ae70: asks the console what it is known by, and keeps six bytes of
it */
void transport_security_get_unknown7cc(
	void)
{
	byte address[NETWORK_SESSION_ADDRESS_SIZE];

	memset(address, 0, sizeof(address));
	XNetGetTitleXnAddr(address);
	memcpy(unknown_004cf7cc, address+TRANSPORT_SECURITY_ADDRESS_UNKNOWNA_OFFSET, sizeof(unknown_004cf7cc));
}

/* 0x0007aec0: an address of the game of four bytes, as the console's
library has one; whether it is one that the library gave, which starts with
a zero. It is not given if the address is not one of four bytes */
boolean transport_address_get_secure_in_address(
	const struct transport_address *address,
	unsigned long *in_address)
{
	unsigned long value;

	if (!transport_address_valid(address) || address->address_size!=TRANSPORT_ADDRESS_IPV4_SIZE)
	{
		return FALSE;
	}
	value= transport_security_swap(address->address.ipv4);
	*in_address= value;

	return (byte)value==0;
}

/* 0x0007af40: whether an address of the game is one: of four bytes (or of
NONE) and not zero, or of sixteen and not all zeros */
boolean transport_address_valid(
	const struct transport_address *address)
{
	if (address)
	{
		long index;

		switch (address->address_size)
		{
		case NONE:
		case TRANSPORT_ADDRESS_IPV4_SIZE:
			return address->address.ipv4!=0;
		case TRANSPORT_ADDRESS_IPV6_SIZE:
			for (index= 0; index<8; index++)
			{
				if (address->address.words[index]!=0)
				{
					return TRUE;
				}
			}
			break;
		}
	}

	return FALSE;
}
