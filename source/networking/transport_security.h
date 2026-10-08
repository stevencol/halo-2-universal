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

	/* what XNetGetTitleXnAddr says of what this console is known by: not yet,
	nothing, or bits; with any of these three its address is kept */
	_xnet_get_xnaddr_pending= 0,
	_xnet_get_xnaddr_none= 1,
	XNET_GET_XNADDR_ADDRESS_FLAGS= 0x1C,

	/* of an address of four bytes, and of one of sixteen */
	TRANSPORT_ADDRESS_IPV4_SIZE= 4,
	TRANSPORT_ADDRESS_IPV6_SIZE= 16,
	/* what transport_address_get_connect_status says when the console's
	library says nothing: it says 0 to 3 */
	MAXIMUM_XNET_CONNECT_STATUS= 3,
	_transport_address_connect_status_none= 4,
	/* in what a console is known by: six bytes of it */
	TRANSPORT_SECURITY_ADDRESS_UNKNOWNA_OFFSET= 0xA,
	TRANSPORT_SECURITY_ADDRESS_UNKNOWNA_SIZE= 6,
	/* how the bytes made at random without the console's library are made */
	TRANSPORT_SECURITY_RANDOM_MULTIPLIER= 0x19660D,
	TRANSPORT_SECURITY_RANDOM_INCREMENT= 0x3C6EF35F,

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
		word words[8];
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

/* 0x003cd0e1: what a session is known by is taken out of the library. The
names of these functions are the ones that Ghidra gives them */
long __attribute__((stdcall)) XNetUnregisterKey(const byte *identifier);

/* ---------- prototypes/TRANSPORT_SECURITY.C */

boolean TRANSPORT_SECURITY_CALLBACK transport_security_create_key(long unknown4, long index, boolean unknown);
boolean TRANSPORT_SECURITY_CALLBACK transport_security_set_key(long index, const byte *key, long unknown4,
	boolean unknown1, const byte *identifier);
void TRANSPORT_SECURITY_CALLBACK transport_security_forget_address(long unused);
boolean transport_security_get_address(void);
boolean transport_security_get_addresses(byte *address, struct transport_address *transport_address);
boolean TRANSPORT_SECURITY_CALLBACK transport_security_address_from_peer_address(long index,
	struct transport_address *address, const byte *peer_address, long unknown, long port);
boolean TRANSPORT_SECURITY_CALLBACK transport_security_peer_address_from_address(
	const struct transport_address *address, long unknown, long *index, byte *identifier, byte *key,
	byte *peer_address);
boolean transport_address_connect(const struct transport_address *address);
long transport_address_get_connect_status(const struct transport_address *address);
void transport_security_random_identifier(byte *identifier);
void transport_security_random_bytes(long size, void *buffer);
boolean TRANSPORT_SECURITY_CALLBACK transport_security_address_from_key(long index, long unknown,
	struct transport_address *address, const byte *peer_address, long port);
void transport_security_get_unknown7cc(void);
boolean transport_address_get_secure_in_address(const struct transport_address *address, unsigned long *in_address);
boolean transport_address_valid(const struct transport_address *address);
boolean TRANSPORT_SECURITY_CALLBACK transport_address_equal(const struct transport_address *address,
	const struct transport_address *other, boolean with_port);

/* ---------- globals */

extern const byte *unknown_0046725c;
extern boolean unknown_004cf790;
extern boolean unknown_004cf791;
extern byte unknown_004cf7cc[TRANSPORT_SECURITY_ADDRESS_UNKNOWNA_SIZE];
extern boolean unknown_004cf792;
extern byte unknown_004cf793[NETWORK_SESSION_ADDRESS_SIZE];
extern struct transport_address unknown_004cf7b8;
extern struct network_session_key unknown_004cf7d4[MAXIMUM_NETWORK_SESSION_KEYS];
extern boolean unknown_004cf8d4;

#endif
