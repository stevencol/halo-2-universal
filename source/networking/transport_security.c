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
long __attribute__((stdcall)) unknown_003cd0cb(byte *identifier, byte *key);

/* 0x003cd0d6: the library is told what a session is known by and its key;
zero if it could */
long __attribute__((stdcall)) unknown_003cd0d6(const byte *identifier, const byte *key);

/* 0x003cd17d: what this console is known by in the network, and what there
is of it: see _unknown_003cd17d_pending */
unsigned long __attribute__((stdcall)) unknown_003cd17d(byte *address);

/* ---------- globals */

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
	else if (unknown_003cd0cb(identifier, key)==0)
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
		unknown_003cd0e1(entry->identifier);
		entry->valid= FALSE;
	}
	if (unknown4==0 && unknown_003cd0d6(identifier, key)!=0)
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
		status= unknown_003cd17d(address);
		if (status!=_unknown_003cd17d_pending)
		{
			if (status==_unknown_003cd17d_none)
			{
				unknown_004cf792= FALSE;
			}
			else
			{
				unknown_004cf792= TRUE;
				memcpy(unknown_004cf793, address, sizeof(unknown_004cf793));
				if (status&UNKNOWN_003CD17D_ADDRESS_FLAGS)
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
