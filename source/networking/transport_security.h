/*
TRANSPORT_SECURITY.H

What this console is known by in the network, and what each session is known
by and its key. The name of the file is a guess, and so are the names of its
functions.
*/

#ifndef __TRANSPORT_SECURITY_H
#define __TRANSPORT_SECURITY_H

#include "cseries/cseries.h"

/* ---------- constants */

enum
{
	/* what this console is known by in the network: the same size as the
	console's libraries have for it. Its first four bytes are its address,
	the first byte first */
	NETWORK_SESSION_ADDRESS_SIZE= 0x24,
	/* what a session is known by in the network, and its key */
	NETWORK_SESSION_IDENTIFIER_SIZE= 0x8,
	NETWORK_SESSION_KEY_SIZE= 0x10,
	MAXIMUM_NETWORK_SESSION_KEYS= 8,

	/* what unknown_003cd17d says of what this console is known by: not yet,
	nothing, or bits; with any of these three its address is kept */
	_unknown_003cd17d_pending= 0,
	_unknown_003cd17d_none= 1,
	UNKNOWN_003CD17D_ADDRESS_FLAGS= 0x1C,

	/* of an address of four bytes */
	TRANSPORT_ADDRESS_IPV4_SIZE= 4,

	/* the first byte of what a session that this console makes is known by:
	its low four bits are kept, and the top one says something of the
	session */
	TRANSPORT_SECURITY_IDENTIFIER_KEPT_BITS= 0xF,
	TRANSPORT_SECURITY_IDENTIFIER_UNKNOWN_BIT= 0x80
};

/* ---------- structures */

/* what a session is known by in the network and its key, as they are kept
out of the session; with two things that they are kept with */
struct network_session_key
{
	boolean valid;
	boolean unknown1;
	byte pad2[2];
	long unknown4;
	byte identifier[NETWORK_SESSION_IDENTIFIER_SIZE];
	byte key[NETWORK_SESSION_KEY_SIZE];
};

/* an address of the network, as the game has it: up to sixteen bytes, a
port and how many of the bytes count */
struct transport_address
{
	union
	{
		unsigned long ipv4;
		byte bytes[16];
	} address;
	word port;
	short address_size;
};

typedef char verify_network_session_key_size[
	sizeof(struct network_session_key) == 0x20 ? 1 : -1];
typedef char verify_transport_address_size[
	sizeof(struct transport_address) == 0x14 ? 1 : -1];

/* a function that takes its arguments off the stack */
#define TRANSPORT_SECURITY_CALLBACK __attribute__((stdcall))

/* ---------- prototypes of functions of the console's libraries that other files call */

/* 0x003cd0e1: what a session is known by is taken out of the library */
long __attribute__((stdcall)) unknown_003cd0e1(const byte *identifier);

/* ---------- prototypes/TRANSPORT_SECURITY.C */

boolean TRANSPORT_SECURITY_CALLBACK transport_security_create_key(long unknown4, long index, boolean unknown);
boolean TRANSPORT_SECURITY_CALLBACK transport_security_set_key(long index, const byte *key, long unknown4,
	boolean unknown1, const byte *identifier);
void TRANSPORT_SECURITY_CALLBACK transport_security_forget_address(long unused);
boolean transport_security_get_address(void);

/* ---------- globals */

extern boolean unknown_004cf791;
extern boolean unknown_004cf792;
extern byte unknown_004cf793[NETWORK_SESSION_ADDRESS_SIZE];
extern struct transport_address unknown_004cf7b8;
extern struct network_session_key unknown_004cf7d4[MAXIMUM_NETWORK_SESSION_KEYS];
extern boolean unknown_004cf8d4;

#endif
