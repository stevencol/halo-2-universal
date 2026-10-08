/*
NETWORK_SESSION.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

A session of the network game. The name of the file is a guess, and so are
the names of its functions.
*/

#include "network_session.h"

#include "cache/cache_files.h"

#include <string.h>

/* ---------- prototypes of functions of the console's libraries */

/* 0x0033039a */
long XGetLanguage(void);

/* 0x003314b0 */
long GetTickCount(void);

/* of the C library */
long time(long *timer);

/* ---------- prototypes of functions not decompiled yet */

/* 0x0007a9b0: asks the console what it is known by in the network, if it
has not been said yet: see unknown_004cf792 */
void unknown_0007a9b0(void);

/* 0x0007a920: keeps what a session is known by in the network, and its key,
at a place of unknown_004cf7d4; the console's library is told of them unless
the first thing says not to. Whether it could. 0x0007a8a0: the same with ones
that it makes: all zeros if the first thing is not zero, and if it is zero,
ones that the console's library makes */
boolean __attribute__((stdcall)) unknown_0007a920(long index, const byte *key, long unknown4, boolean unknown1,
	const byte *identifier);
boolean __attribute__((stdcall)) unknown_0007a8a0(long unknown4, long index, boolean unknown);

/* 0x00075a90: tells what a session is given when it is set up what the
session is known by, its key and two numbers; with NONE for the first
number, only that */
void __attribute__((stdcall)) unknown_00075a90(long index, byte *entries, const byte *identifier, const byte *key,
	long unknown38, long unknown3C);

/* 0x003cd147, 0x003cd0e1, of the console's libraries: something that is
started (1) or ended (0x10) for what a session is known by, and something
that what a session is known by is taken out of */
long __attribute__((stdcall)) unknown_003cd147(const byte *identifier, const void *unknown4, unsigned long unknown8,
	unsigned long unknownC, unsigned long flags);
long __attribute__((stdcall)) unknown_003cd0e1(const byte *identifier);

/* 0x0007ad80: fills a buffer with bytes made at random */
void unknown_0007ad80(long size, void *buffer);

/* 0x0005fbd0: a console comes into a session, at a place: it is given what
the console is known by and three more things */
void __attribute__((stdcall)) unknown_0005fbd0(byte *session, long peer_index, const byte *address, long unknownC,
	long unknown10, long *unknown14);

/* 0x00061390: something that is done with a session when who is in it
changes */
void unknown_00061390(byte *session);

/* 0x00062e70: does something in a session with each of a number of things
of twelve bytes, and with a number of each */
void __attribute__((stdcall)) unknown_00062e70(const byte *identifiers, long *unknown20, byte *session,
	long unknown14, long count, const long *unknown1C);

/* 0x0011ca80, of cseries.c */
long language_from_xbox_language(long xbox_language);

/* ---------- globals */

/* 0x00510548, 0x0051054c, of main.c: whether the time is the one that was
kept, and that time */
extern boolean unknown_00510548;
extern long unknown_0051054c;

/* 0x004d8b18, 0x004d8b19: with both, a session that is started asks what
the console is known by in the network */
boolean unknown_004d8b18;
boolean unknown_004d8b19;

/* 0x004cf792, 0x004cf793: whether it is known what this console is known by
in the network, and what it is */
boolean unknown_004cf792;
byte unknown_004cf793[NETWORK_SESSION_ADDRESS_SIZE];

/* 0x004cf7d4: what each session is known by in the network, and its key; by
the number that a session has at 0x38. 0x004cf8d4: with it, the console's
library is told when a session starts and stops being known by something */
struct network_session_key unknown_004cf7d4[MAXIMUM_NETWORK_SESSION_KEYS];
boolean unknown_004cf8d4;

/* ---------- code */

/* the time in milliseconds: the one that main.c kept, if it says so. The
game has this in each function that uses it */
static inline __attribute__((always_inline)) long network_session_time(
	void)
{
	return unknown_00510548 ? unknown_0051054c : GetTickCount();
}

/* the state of a session */
static inline __attribute__((always_inline)) long network_session_state(
	const byte *session)
{
	return *(const long *)(session+NETWORK_SESSION_STATE_OFFSET);
}

/* 0x00058d20: whether a session is in one of its last four states */
boolean network_session_unknown_00058d20(
	const byte *session)
{
	long state= network_session_state(session);

	return state==_network_session_state5 || state==_network_session_state6 || state==_network_session_state7 ||
		state==_network_session_state8;
}

/* 0x00058d50: whether this console runs a session that says who is in it */
boolean network_session_local_peer_is_host(
	const byte *session)
{
	long state= network_session_state(session);
	boolean result= FALSE;

	if (state>=_network_session_state3 && state<=_network_session_state8)
	{
		result= *(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET)==
			*(const long *)(session+NETWORK_SESSION_HOST_PEER_OFFSET);
	}

	return result;
}

/* 0x00058d70: whether a session says who is in it */
boolean network_session_established(
	const byte *session)
{
	long state= network_session_state(session);

	return state>=_network_session_state3 && state<=_network_session_state8;
}

/* 0x00058d90: in the second, fourth and sixth states, yes; in the last two,
what a byte of the session says; in the others, no */
boolean network_session_unknown_00058d90(
	const byte *session)
{
	switch (network_session_state(session))
	{
	case _network_session_state2:
	case _network_session_state4:
	case _network_session_state6:
		return TRUE;
	case _network_session_state7:
	case _network_session_state8:
		return session[NETWORK_SESSION_UNKNOWN7420_OFFSET];
	}

	return FALSE;
}

/* 0x00059ad0: sets a session up, with no state: it is put in the table of
the sessions at its place, and in what it is given, which has something for
each session. Nobody is in it and it does not know which console this one
is. TRUE */
boolean NETWORK_SESSION_CALLBACK network_session_initialize(
	byte *session,
	byte *unknown8,
	byte **table,
	long index,
	long unknown14,
	long unknown38,
	long unknown4)
{
	byte *entry;

	*(long *)(session+NETWORK_SESSION_UNKNOWN14_OFFSET)= unknown14;
	*(long *)(session+NETWORK_SESSION_UNKNOWN38_OFFSET)= unknown38;
	*(long *)(session+NETWORK_SESSION_UNKNOWN4_OFFSET)= unknown4;
	*(byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET)= unknown8;
	*(byte ***)(session+NETWORK_SESSION_TABLE_OFFSET)= table;
	*(long *)(session+NETWORK_SESSION_INDEX_OFFSET)= index;
	table[index]= session;
	entry= unknown8+*(const long *)(session+NETWORK_SESSION_INDEX_OFFSET)*NETWORK_SESSION_UNKNOWN8_ENTRY_SIZE;
	*(long *)(entry+NETWORK_SESSION_UNKNOWN8_ENTRY_UNKNOWN18_OFFSET)= NONE;
	*(long *)(entry+NETWORK_SESSION_UNKNOWN8_ENTRY_UNKNOWN1C_OFFSET)= NONE;
	*(byte **)(entry+NETWORK_SESSION_UNKNOWN8_ENTRY_SESSION_OFFSET)= session;
	*(long *)(session+NETWORK_SESSION_UNKNOWN18_OFFSET)= NONE;
	*(long *)(session+NETWORK_SESSION_UNKNOWN40_OFFSET)= NONE;
	session[NETWORK_SESSION_UNKNOWN48_OFFSET]= 0;
	memset(session+NETWORK_SESSION_MEMBERSHIP_OFFSET, 0, NETWORK_SESSION_MEMBERSHIP_SIZE);
	memset(session+NETWORK_SESSION_OTHER_MEMBERSHIP_OFFSET, 0, NETWORK_SESSION_MEMBERSHIP_SIZE);
	*(long *)(session+NETWORK_SESSION_OTHER_MEMBERSHIP_OFFSET)= NONE;
	*(long *)(session+NETWORK_SESSION_MEMBERSHIP_OFFSET)= NONE;
	memset(session+NETWORK_SESSION_UNKNOWN4978_OFFSET, 0, NETWORK_SESSION_UNKNOWN4978_SIZE);
	memset(session+NETWORK_SESSION_UNKNOWN5E28_OFFSET, 0, NETWORK_SESSION_UNKNOWN4978_SIZE);
	*(long *)(session+NETWORK_SESSION_UNKNOWN4978_OFFSET)= NONE;
	*(long *)(session+NETWORK_SESSION_UNKNOWN5E28_OFFSET)= NONE;
	*(long *)(session+NETWORK_SESSION_STATE_OFFSET)= _network_session_state_none;
	*(long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET)= NONE;
	*(long *)(session+NETWORK_SESSION_UNKNOWN7618_OFFSET)= 0;
	memset(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET, 0, NETWORK_SESSION_UNKNOWN72DC_SIZE);
	memset(session+NETWORK_SESSION_UNKNOWN761C_OFFSET, 0, NETWORK_SESSION_UNKNOWN761C_SIZE);
	*(long *)(session+NETWORK_SESSION_UNKNOWN7650_OFFSET)= 0;
	*(long *)(session+NETWORK_SESSION_UNKNOWN7654_OFFSET)= NONE;
	*(long *)(session+NETWORK_SESSION_UNKNOWN7658_OFFSET)= NONE;
	memset(session+NETWORK_SESSION_UNKNOWN7668_OFFSET, 0, NETWORK_SESSION_UNKNOWN7668_SIZE);
	*(long *)(session+NETWORK_SESSION_UNKNOWN78A8_OFFSET)= 0;
	session[NETWORK_SESSION_UNKNOWN78AC_OFFSET]= 0;

	return TRUE;
}

/* 0x00059bd0: starts a session, with this console alone in it; whether it
could. With a number that is not zero the session is for the network: the
console has to be known by something in it, and network_session_use_identifier (with the
fourth and fifth things) or network_session_create_identifier (without) has to be able to get
it ready. Then what the session knows starts over: when it started, the
language, eight bytes made at random; nobody is in it but this console, which
unknown_0005fbd0 puts first; and what it will be says nothing. At last, if it
is given a number of things that is NONE or more than none, they go to
unknown_00062e70 */
boolean NETWORK_SESSION_CALLBACK network_session_start(
	byte *session,
	long unknown18,
	long unknown8,
	long unknownC,
	long unknown10,
	long unknown14,
	const byte *identifiers,
	const long *unknown1C,
	long *unknown20,
	long count)
{
	byte address[NETWORK_SESSION_ADDRESS_SIZE];

	if (unknown18==0)
	{
		memset(address, 0, sizeof(address));
	}
	else
	{
		if (unknown_004d8b18 && unknown_004d8b19)
		{
			unknown_0007a9b0();
		}
		memcpy(address, unknown_004cf793, sizeof(address));
		if (!unknown_004cf792)
		{
			return FALSE;
		}
		if (unknownC && unknown10)
		{
			if (!network_session_use_identifier(session, (const byte *)unknown10, unknown8, unknown18,
				(const byte *)unknownC))
			{
				return FALSE;
			}
		}
		else if (!network_session_create_identifier(session, unknown8, unknown18))
		{
			return FALSE;
		}
	}
	*(long *)(session+NETWORK_SESSION_UNKNOWN18_OFFSET)= unknown18;
	memset(session+NETWORK_SESSION_UNKNOWN4978_OFFSET, 0, NETWORK_SESSION_UNKNOWN4978_SIZE);
	*(long *)(session+NETWORK_SESSION_UNKNOWN4980_OFFSET)= 1;
	*(long *)(session+NETWORK_SESSION_UNKNOWN497C_OFFSET)= 0;
	*(long *)(session+NETWORK_SESSION_START_TIME_OFFSET)= network_session_time();
	*(long *)(session+NETWORK_SESSION_UNKNOWN4978_OFFSET)= 0;
	if (global_language==NONE)
	{
		global_language= language_from_xbox_language(XGetLanguage());
	}
	*(long *)(session+NETWORK_SESSION_LANGUAGE_OFFSET)= global_language;
	*(long *)(session+NETWORK_SESSION_UNKNOWN498C_OFFSET)= 0;
	*(long *)(session+NETWORK_SESSION_UNKNOWN4990_OFFSET)= NETWORK_SESSION_UNKNOWN4990_DEFAULT;
	*(long *)(session+NETWORK_SESSION_UNKNOWN4994_OFFSET)= NETWORK_SESSION_UNKNOWN4990_DEFAULT;
	*(long *)(session+NETWORK_SESSION_UNKNOWN49A4_OFFSET)= 0;
	*(long *)(session+NETWORK_SESSION_UNKNOWN49C8_OFFSET)= NONE;
	*(long *)(session+NETWORK_SESSION_UNKNOWN4D08_OFFSET)= NONE;
	*(long *)(session+NETWORK_SESSION_UNKNOWN4D0C_OFFSET)= NONE;
	unknown_0007ad80(NETWORK_SESSION_RANDOM_SIZE, session+NETWORK_SESSION_RANDOM_OFFSET);
	*(short *)(session+NETWORK_SESSION_UNKNOWN5DD0_OFFSET)= NONE;
	*(long *)(session+NETWORK_SESSION_UNKNOWN49B4_OFFSET)= NONE;
	*(long *)(session+NETWORK_SESSION_UNKNOWN5E20_OFFSET)= NONE;
	*(unsigned long *)(session+NETWORK_SESSION_UNKNOWN4DA8_OFFSET)= NETWORK_SESSION_UNKNOWN4DA8_VALUE;
	*(long *)(session+NETWORK_SESSION_UNKNOWN4DAC_OFFSET)= 1;
	session[NETWORK_SESSION_UNKNOWN49A8_OFFSET]= 0;
	*(long *)(session+NETWORK_SESSION_UNKNOWN49AC_OFFSET)= 0;
	*(long *)(session+NETWORK_SESSION_UNKNOWN49B0_OFFSET)= 0;
	memset(session+NETWORK_SESSION_MEMBERSHIP_OFFSET, 0, NETWORK_SESSION_MEMBERSHIP_SIZE);
	session[NETWORK_SESSION_UNKNOWN48_OFFSET]= 0;
	*(long *)(session+NETWORK_SESSION_UNKNOWN40_OFFSET)= 0;
	*(long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET)= 0;
	unknown_0005fbd0(session, *(const long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET), address, 0, NONE,
		unknown20);
	(*(long *)(session+NETWORK_SESSION_UNKNOWN7618_OFFSET))++;
	*(long *)(session+NETWORK_SESSION_HOST_PEER_OFFSET)= 0;
	*(long *)(session+NETWORK_SESSION_MEMBERSHIP_OFFSET)= 0;
	memset(session+NETWORK_SESSION_OTHER_MEMBERSHIP_OFFSET, 0, NETWORK_SESSION_MEMBERSHIP_SIZE);
	*(long *)(session+NETWORK_SESSION_OTHER_MEMBERSHIP_OFFSET)= NONE;
	memset(session+NETWORK_SESSION_UNKNOWN5E28_OFFSET, 0, NETWORK_SESSION_UNKNOWN4978_SIZE);
	*(long *)(session+NETWORK_SESSION_UNKNOWN5E28_OFFSET)= NONE;
	session[NETWORK_SESSION_UNKNOWN78AC_OFFSET]= TRUE;
	*(long *)(session+NETWORK_SESSION_UNKNOWN78B0_OFFSET)= -network_session_time();
	*(long *)(session+NETWORK_SESSION_UNKNOWN78B4_OFFSET)= time(NULL);
	unknown_00061390(session);
	if (count==NONE || count>0)
	{
		unknown_00062e70(identifiers, unknown20, session, unknown14, count, unknown1C);
	}

	return TRUE;
}

/* 0x0005fb60: a session stops being known by something in the network, if
it was: the console's library is told, its place in unknown_004cf7d4 is let
go of, and so is the first number of what the session was given when it was
set up */
void network_session_forget_identifier(
	byte *session)
{
	if (session[NETWORK_SESSION_HAS_IDENTIFIER_OFFSET])
	{
		struct network_session_key *key;
		byte *entry;

		if (unknown_004cf8d4)
		{
			unknown_003cd147(session+NETWORK_SESSION_IDENTIFIER_OFFSET, NULL, 0, 0, _unknown_003cd147_end);
		}
		key= &unknown_004cf7d4[*(const long *)(session+NETWORK_SESSION_UNKNOWN38_OFFSET)];
		if (key->valid)
		{
			unknown_003cd0e1(key->identifier);
			key->valid= FALSE;
		}
		entry= *(byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET)+
			*(const long *)(session+NETWORK_SESSION_INDEX_OFFSET)*NETWORK_SESSION_UNKNOWN8_ENTRY_SIZE;
		*(long *)(entry+NETWORK_SESSION_UNKNOWN8_ENTRY_UNKNOWN18_OFFSET)= NONE;
		memset(session+NETWORK_SESSION_IDENTIFIER_OFFSET, 0, NETWORK_SESSION_IDENTIFIER_SIZE);
		memset(session+NETWORK_SESSION_KEY_OFFSET, 0, NETWORK_SESSION_KEY_SIZE);
		*(long *)(session+NETWORK_SESSION_UNKNOWN3C_OFFSET)= NONE;
		session[NETWORK_SESSION_HAS_IDENTIFIER_OFFSET]= FALSE;
	}
}

/* 0x0005fb00: a session is known by something in the network from now on,
with a key and a number: what the session was given when it was set up is
told, and so is the console's library */
void network_session_set_identifier(
	byte *session,
	const byte *identifier,
	const byte *key,
	long unknown3C)
{
	session[NETWORK_SESSION_HAS_IDENTIFIER_OFFSET]= TRUE;
	memcpy(session+NETWORK_SESSION_IDENTIFIER_OFFSET, identifier, NETWORK_SESSION_IDENTIFIER_SIZE);
	memcpy(session+NETWORK_SESSION_KEY_OFFSET, key, NETWORK_SESSION_KEY_SIZE);
	*(long *)(session+NETWORK_SESSION_UNKNOWN3C_OFFSET)= unknown3C;
	unknown_00075a90(*(const long *)(session+NETWORK_SESSION_INDEX_OFFSET),
		*(byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET), session+NETWORK_SESSION_IDENTIFIER_OFFSET,
		session+NETWORK_SESSION_KEY_OFFSET, *(const long *)(session+NETWORK_SESSION_UNKNOWN38_OFFSET), unknown3C);
	if (unknown_004cf8d4)
	{
		unknown_003cd147(session+NETWORK_SESSION_IDENTIFIER_OFFSET, NULL, 0, 0, _unknown_003cd147_start);
	}
}

/* 0x0005fac0: a session is known in the network by what it is given, with
the key that it is given; whether unknown_0007a920 could keep them. The
fourth thing is not used */
boolean NETWORK_SESSION_CALLBACK network_session_use_identifier(
	byte *session,
	const byte *key,
	long unknown8,
	long unknown18,
	const byte *identifier)
{
	(void)unknown18;
	network_session_forget_identifier(session);
	if (unknown_0007a920(*(const long *)(session+NETWORK_SESSION_UNKNOWN38_OFFSET), key, unknown8, FALSE,
		identifier))
	{
		network_session_set_identifier(session, identifier, key, unknown8);

		return TRUE;
	}

	return FALSE;
}

/* 0x0005fa30: a session is known in the network by something that
unknown_0007a8a0 makes, with its key; whether it could. What it makes is
taken from the place of the session in unknown_004cf7d4 */
boolean NETWORK_SESSION_CALLBACK network_session_create_identifier(
	byte *session,
	long unknown8,
	long unknown18)
{
	network_session_forget_identifier(session);
	if (unknown_0007a8a0(unknown8, *(const long *)(session+NETWORK_SESSION_UNKNOWN38_OFFSET), unknown18==2))
	{
		const struct network_session_key *made=
			&unknown_004cf7d4[*(const long *)(session+NETWORK_SESSION_UNKNOWN38_OFFSET)];

		if (made->valid)
		{
			byte identifier[NETWORK_SESSION_IDENTIFIER_SIZE];
			byte key[NETWORK_SESSION_KEY_SIZE];

			memcpy(identifier, made->identifier, sizeof(identifier));
			memcpy(key, made->key, sizeof(key));
			network_session_set_identifier(session, identifier, key, unknown8);

			return TRUE;
		}
	}

	return FALSE;
}
