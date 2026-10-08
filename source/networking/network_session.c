/*
NETWORK_SESSION.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

A session of the network game. The name of the file is a guess, and so are
the names of its functions.
*/

#include "network_session.h"

#include "cache/cache_files.h"
#include "networking/network_observer.h"
#include "text/unicode.h"

#include <string.h>

/* ---------- prototypes of functions of the console's libraries */

/* 0x0033039a */
long XGetLanguage(void);

/* 0x003314b0 */
long GetTickCount(void);

/* of the C library */
long time(long *timer);
wchar *wcsncpy(wchar *destination, const wchar *source, size_t count);
int wcsncmp(const wchar *a, const wchar *b, size_t count);

/* ---------- prototypes of functions not decompiled yet */

/* 0x003cd147, of the console's libraries: something that is started (1) or
ended (0x10) for what a session is known by */
long __attribute__((stdcall)) XNetQosListen(const byte *identifier, const void *unknown4, unsigned long unknown8,
	unsigned long unknownC, unsigned long flags);

/* 0x00075e80: sends something of a kind and a size to the console of one of
the things of 0x528 bytes of what a session was given when it was set up, in
one of two ways */
void __attribute__((stdcall)) unknown_00075e80(boolean unknown, long channel_index, byte *unknown8, long index,
	long type, long size, const void *data);

/* 0x0011ca80, of cseries.c */
long language_from_xbox_language(long xbox_language);

/* 0x0005a520: something that is done with a session: see
network_life_cycle.c */
void unknown_0005a520(byte *session);

/* 0x00062990, 0x00062480, 0x000623e0, 0x00062ab0: what a session does each
time in its third state, in its fourth, in another and when something comes
from the console that runs it */
void unknown_00062990(byte *session);
void unknown_00062480(byte *session);
void unknown_000623e0(byte *session);
void unknown_00062ab0(byte *session);

/* 0x0005df30: something that the console that runs a session does for a
console of it, with a byte, two numbers and a thing or none; whether it did */
boolean __attribute__((stdcall)) unknown_0005df30(byte *session, boolean unknown8, long unknownC, long unknown10,
	long peer_index, const byte *unknown14);

/* 0x0005a400: something that is done with a session, with a number: see
network_life_cycle.c */
void __attribute__((stdcall)) unknown_0005a400(byte *session, long unknown);

/* 0x0005a220: something that the console that runs a session does with a
number */
void unknown_0005a220(byte *session, long unknown);

/* 0x000614a0: something that the console that runs a session does with it
in its fifth state, with a byte and a number */
void unknown_000614a0(boolean unknown, byte *session, long unknown4);

/* ---------- globals */

/* 0x00510548, 0x0051054c, of main.c: whether the time is the one that was
kept, and that time */
extern boolean unknown_00510548;
extern long unknown_0051054c;

/* 0x004cf4a4, 0x004cf4ac: how long a session waits in two of its states
before unknown_0005a520 is done with it, in milliseconds */
long unknown_004cf4a4;
long unknown_004cf4ac;

/* 0x004d8b18, 0x004d8b19: with both, a session that is started asks what
the console is known by in the network */
boolean unknown_004d8b18;
boolean unknown_004d8b19;

/* 0x004cf792, 0x004cf793, 0x004cf7d4 and 0x004cf8d4 are of
transport_security.c */

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
network_session_add_peer puts first; and what it will be says nothing. At last, if its
last number is NONE or more than none, the players that it is given ask to
come in: network_session_request_players */
boolean NETWORK_SESSION_CALLBACK network_session_start(
	byte *session,
	long unknown18,
	long unknown8,
	long unknownC,
	long unknown10,
	long player_count,
	const byte *identifiers,
	const long *unknown1C,
	long *unknown20,
	long unknown24)
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
			transport_security_get_address();
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
	transport_security_random_bytes(NETWORK_SESSION_RANDOM_SIZE, session+NETWORK_SESSION_RANDOM_OFFSET);
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
	network_session_add_peer(session, *(const long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET), address, 0, NONE,
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
	network_session_enter_state5(session);
	if (unknown24==NONE || unknown24>0)
	{
		network_session_request_players(identifiers, unknown20, session, player_count, unknown24, unknown1C);
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
			XNetQosListen(session+NETWORK_SESSION_IDENTIFIER_OFFSET, NULL, 0, 0, _xnet_qos_listen_release);
		}
		key= &unknown_004cf7d4[*(const long *)(session+NETWORK_SESSION_UNKNOWN38_OFFSET)];
		if (key->valid)
		{
			XNetUnregisterKey(key->identifier);
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
	network_observer_set_session(*(const long *)(session+NETWORK_SESSION_INDEX_OFFSET),
		*(byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET), session+NETWORK_SESSION_IDENTIFIER_OFFSET,
		session+NETWORK_SESSION_KEY_OFFSET, *(const long *)(session+NETWORK_SESSION_UNKNOWN38_OFFSET), unknown3C);
	if (unknown_004cf8d4)
	{
		XNetQosListen(session+NETWORK_SESSION_IDENTIFIER_OFFSET, NULL, 0, 0, _xnet_qos_listen_enable);
	}
}

/* 0x0005fac0: a session is known in the network by what it is given, with
the key that it is given; whether transport_security_set_key could keep them. The
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
	if (transport_security_set_key(*(const long *)(session+NETWORK_SESSION_UNKNOWN38_OFFSET), key, unknown8, FALSE,
		identifier))
	{
		network_session_set_identifier(session, identifier, key, unknown8);

		return TRUE;
	}

	return FALSE;
}

/* 0x0005fa30: a session is known in the network by something that
transport_security_create_key makes, with its key; whether it could. What it makes is
taken from the place of the session in unknown_004cf7d4 */
boolean NETWORK_SESSION_CALLBACK network_session_create_identifier(
	byte *session,
	long unknown8,
	long unknown18)
{
	network_session_forget_identifier(session);
	if (transport_security_create_key(unknown8, *(const long *)(session+NETWORK_SESSION_UNKNOWN38_OFFSET), unknown18==2))
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

/* 0x0005f900: what a session keeps of a console that comes in, at a place:
there is one, since now, with a byte and a number that it is given. With the
byte and a byte of the session, another byte is set */
void NETWORK_SESSION_CALLBACK network_session_peer_state_new(
	boolean unknown1,
	byte *session,
	long peer_index,
	long unknown4)
{
	struct network_session_peer_state *state=
		(struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET)+peer_index;

	memset(state, 0, sizeof(*state));
	state->unknown4= unknown4;
	state->valid= TRUE;
	state->unknown1= unknown1;
	state->unknown8= NONE;
	state->unknownC= NONE;
	state->unknown3= FALSE;
	if (unknown1 && session[NETWORK_SESSION_UNKNOWN765C_OFFSET])
	{
		state->unknown2= TRUE;
	}
	state->time= network_session_time();
}

/* 0x0005fbd0: a console comes into a session, at a place: there is one more
in it, and all that is known of this one is what it is known by in the
network and, if they are given, two numbers. Its texts are empty. What the
session keeps of it apart is started too */
void NETWORK_SESSION_CALLBACK network_session_add_peer(
	byte *session,
	long peer_index,
	const byte *address,
	boolean unknown1,
	long unknown4,
	const long *unknownF0)
{
	static const wchar empty[]= { 0 };
	byte *peer= session+NETWORK_SESSION_PEER_ARRAY_OFFSET+peer_index*NETWORK_SESSION_PEER_SIZE;
	wchar *text;
	long index;

	(*(long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET))++;
	memset(peer, 0, NETWORK_SESSION_PEER_SIZE);
	text= (wchar *)(peer+NETWORK_SESSION_PEER_UNKNOWN28_OFFSET);
	wcsncpy(text, empty, NETWORK_SESSION_PEER_UNKNOWN28_LENGTH-1);
	text[NETWORK_SESSION_PEER_UNKNOWN28_LENGTH-1]= 0;
	text= (wchar *)(peer+NETWORK_SESSION_PEER_UNKNOWN48_OFFSET);
	wcsncpy(text, empty, NETWORK_SESSION_PEER_UNKNOWN48_LENGTH-1);
	text[NETWORK_SESSION_PEER_UNKNOWN48_LENGTH-1]= 0;
	*(long *)(peer+NETWORK_SESSION_PEER_UNKNOWN88_OFFSET)= 0;
	*(long *)(peer+NETWORK_SESSION_PEER_UNKNOWN8C_OFFSET)= 0;
	for (index= 0; index<MAXIMUM_NETWORK_SESSION_PEER_PLAYERS; index++)
	{
		*(long *)(peer+NETWORK_SESSION_PEER_PLAYERS_OFFSET+index*sizeof(long))= NONE;
	}
	memset(peer+NETWORK_SESSION_PEER_UNKNOWN90_OFFSET, 0, NETWORK_SESSION_PEER_UNKNOWN90_SIZE);
	memset(peer+NETWORK_SESSION_PEER_UNKNOWNAC_OFFSET, 0, NETWORK_SESSION_PEER_UNKNOWNAC_SIZE);
	*(long *)(peer+NETWORK_SESSION_PEER_UNKNOWNEC_OFFSET)= 0;
	memcpy(peer+NETWORK_SESSION_PEER_ADDRESS_OFFSET, address, NETWORK_SESSION_ADDRESS_SIZE);
	if (unknownF0)
	{
		*(long *)(peer+NETWORK_SESSION_PEER_UNKNOWNF0_OFFSET)= unknownF0[0];
		*(long *)(peer+NETWORK_SESSION_PEER_UNKNOWNF0_OFFSET+sizeof(long))= unknownF0[1];
	}
	network_session_peer_state_new(unknown1, session, peer_index, unknown4);
}

/* 0x0005f890: which player of a session is known by twelve bytes; NONE if
none is, or if the session has no state or says nothing of who is in it */
long NETWORK_SESSION_CALLBACK network_session_find_player(
	const byte *session,
	const byte *identifier)
{
	if (network_session_state(session)!=_network_session_state_none &&
		*(const long *)(session+NETWORK_SESSION_MEMBERSHIP_OFFSET)!=NONE)
	{
		const byte *membership= session+NETWORK_SESSION_MEMBERSHIP_OFFSET;
		unsigned long players= *(const unsigned long *)(membership+NETWORK_SESSION_MEMBERSHIP_PLAYERS_OFFSET);
		long player_index;

		for (player_index= 0; player_index<MAXIMUM_NETWORK_SESSION_PLAYERS; player_index++)
		{
			if ((players&(1u<<player_index)) && memcmp(identifier,
				membership+NETWORK_SESSION_MEMBERSHIP_PLAYER_ARRAY_OFFSET+player_index*NETWORK_SESSION_PLAYER_SIZE,
				NETWORK_SESSION_PLAYER_IDENTIFIER_SIZE)==0)
			{
				return player_index;
			}
		}
	}

	return NONE;
}

/* 0x00060fa0: what a session will be says nothing, nobody asks to come into
it, and what it keeps of each console goes back to how it was when it came
in, but for its first byte and its number. With the thing that it is given,
its two numbers go back to 16 and its first number counts one more */
void NETWORK_SESSION_CALLBACK network_session_reset_requests(
	byte *session,
	boolean unknown)
{
	long peer_index;

	memset(session+NETWORK_SESSION_OTHER_MEMBERSHIP_OFFSET, 0, NETWORK_SESSION_MEMBERSHIP_SIZE);
	*(long *)(session+NETWORK_SESSION_OTHER_MEMBERSHIP_OFFSET)= NONE;
	memset(session+NETWORK_SESSION_UNKNOWN5E28_OFFSET, 0, NETWORK_SESSION_UNKNOWN4978_SIZE);
	memset(session+NETWORK_SESSION_UNKNOWN7668_OFFSET, 0, NETWORK_SESSION_UNKNOWN7668_SIZE);
	*(long *)(session+NETWORK_SESSION_UNKNOWN5E28_OFFSET)= NONE;
	session[NETWORK_SESSION_UNKNOWN765C_OFFSET]= FALSE;
	*(long *)(session+NETWORK_SESSION_UNKNOWN44_OFFSET)= NONE;
	for (peer_index= 0; peer_index<*(const long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET); peer_index++)
	{
		struct network_session_peer_state *state=
			(struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET)+peer_index;

		state->unknown2= FALSE;
		state->unknown8= NONE;
		state->unknownC= NONE;
		state->unknown3= FALSE;
	}
	if (unknown)
	{
		*(long *)(session+NETWORK_SESSION_UNKNOWN4990_OFFSET)= NETWORK_SESSION_UNKNOWN4990_DEFAULT;
		*(long *)(session+NETWORK_SESSION_UNKNOWN4994_OFFSET)= NETWORK_SESSION_UNKNOWN4990_DEFAULT;
		(*(long *)(session+NETWORK_SESSION_UNKNOWN4978_OFFSET))++;
	}
}

/* 0x00061390: a session goes into its fifth state, and what it has from its
state on starts over. From a state before the fifth, network_session_reset_requests
is done first; the third byte of what is kept of each console is its second,
and if any has it the session keeps a number and the time; and the console
that runs the session at 0x40 is this one */
void network_session_enter_state5(
	byte *session)
{
	if (!network_session_unknown_00058d20(session))
	{
		boolean any= FALSE;
		long peer_index;

		network_session_reset_requests(session, TRUE);
		for (peer_index= 0; peer_index<*(const long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET); peer_index++)
		{
			struct network_session_peer_state *state=
				(struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET)+peer_index;

			state->unknown2= state->unknown1;
			if (state->unknown1)
			{
				any= TRUE;
			}
		}
		if (any)
		{
			*(long *)(session+NETWORK_SESSION_UNKNOWN7660_OFFSET)=
				*(const long *)(session+NETWORK_SESSION_UNKNOWN4980_OFFSET);
			session[NETWORK_SESSION_UNKNOWN765C_OFFSET]= TRUE;
			*(long *)(session+NETWORK_SESSION_UNKNOWN7664_OFFSET)= network_session_time();
		}
		*(long *)(session+NETWORK_SESSION_UNKNOWN40_OFFSET)= *(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET);
	}
	memset(session+NETWORK_SESSION_STATE_DATA_OFFSET, 0, NETWORK_SESSION_STATE_DATA_SIZE);
	*(long *)(session+NETWORK_SESSION_STATE_OFFSET)= _network_session_state5;
}

/* 0x00062e70: each of a number of players asks to come into a session, with
a number of its own, until one cannot; whether all could */
boolean NETWORK_SESSION_CALLBACK network_session_request_players(
	const byte *identifiers,
	const long *unknown2,
	byte *session,
	long player_count,
	long unknown20,
	const long *unknown18s)
{
	boolean result= TRUE;
	long index;

	for (index= 0; index<player_count && result; index++)
	{
		result= network_session_request_player(identifiers+index*NETWORK_SESSION_PLAYER_IDENTIFIER_SIZE, unknown2,
			session, unknown20, unknown18s[index]);
	}

	return result;
}

/* 0x00062eb0: a player asks to come into a session: it takes the first
place that is free of the sixteen, with what the player is known by, eight
bytes, two numbers and the time; and whether the session has the player
already is looked at. FALSE if no place is free */
boolean NETWORK_SESSION_CALLBACK network_session_request_player(
	const byte *identifier,
	const long *unknown2,
	byte *session,
	long unknown20,
	long unknown18)
{
	struct network_session_player_request *request=
		(struct network_session_player_request *)(session+NETWORK_SESSION_UNKNOWN7668_OFFSET);
	const struct network_session_player_request *end= request+MAXIMUM_NETWORK_SESSION_PLAYER_REQUESTS;

	for (; request<end; request++)
	{
		if (!request->valid)
		{
			memcpy(request->identifier, identifier, sizeof(request->identifier));
			request->time= network_session_time();
			request->unknown20= unknown20;
			memcpy(request->unknown2, unknown2, sizeof(request->unknown2));
			request->unknown18= unknown18;
			request->valid= TRUE;
			request->has_player= network_session_find_player(session, request->identifier)!=NONE;

			return TRUE;
		}
	}

	return FALSE;
}

/* 0x0005f760: which console of a session is known in the network by what it
is given; NONE if none is, or if the session has no state or says nothing of
who is in it */
long NETWORK_SESSION_CALLBACK network_session_find_peer(
	const byte *session,
	const byte *address)
{
	if (network_session_state(session)!=_network_session_state_none &&
		*(const long *)(session+NETWORK_SESSION_MEMBERSHIP_OFFSET)!=NONE)
	{
		long peer_count= *(const long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET);
		long peer_index;

		for (peer_index= 0; peer_index<peer_count; peer_index++)
		{
			if (memcmp(address, session+NETWORK_SESSION_PEER_ARRAY_OFFSET+peer_index*NETWORK_SESSION_PEER_SIZE,
				NETWORK_SESSION_ADDRESS_SIZE)==0)
			{
				return peer_index;
			}
		}
	}

	return NONE;
}

/* 0x0005f810: whether each player of a session is a player of another too.
FALSE if either does not say who is in it */
boolean NETWORK_SESSION_CALLBACK network_session_has_players_of(
	const byte *session,
	const byte *other_session)
{
	if (network_session_established(other_session) && network_session_established(session))
	{
		const byte *membership= session+NETWORK_SESSION_MEMBERSHIP_OFFSET;
		unsigned long players= *(const unsigned long *)(membership+NETWORK_SESSION_MEMBERSHIP_PLAYERS_OFFSET);
		long player_index;

		for (player_index= 0; player_index<MAXIMUM_NETWORK_SESSION_PLAYERS; player_index++)
		{
			if ((players&(1u<<player_index)) && network_session_find_player(other_session,
				membership+NETWORK_SESSION_MEMBERSHIP_PLAYER_ARRAY_OFFSET+player_index*NETWORK_SESSION_PLAYER_SIZE)==NONE)
			{
				return FALSE;
			}
		}

		return TRUE;
	}

	return FALSE;
}

/* 0x0005f970: what a session keeps of a console is no more. If it had a
number, the thing of that number in what the session was given when it was
set up loses the bit of the session. Before that the game looks, in the last
four states, for another console that has the three first bytes set, and
does nothing with what it finds */
void network_session_peer_state_delete(
	byte *session,
	long peer_index)
{
	struct network_session_peer_state *state=
		(struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET)+peer_index;

	if (state->unknown4!=NONE)
	{
		byte *channel= *(byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET)+
			state->unknown4*NETWORK_SESSION_UNKNOWN8_CHANNEL_SIZE;

		channel[NETWORK_SESSION_UNKNOWN8_CHANNEL_SESSIONS_OFFSET]&=
			(byte)~(1u<<*(const long *)(session+NETWORK_SESSION_INDEX_OFFSET));
		state->unknown4= NONE;
	}
	memset(state, 0, sizeof(*state));
	state->unknown4= NONE;
}

/* whether a session is in its first state or says who is in it: the game
has this in the functions that ask whether something comes from the console
that runs the session */
static inline __attribute__((always_inline)) boolean network_session_has_host(
	const byte *session)
{
	long state= network_session_state(session);

	return (state>_network_session_state2 && state<=_network_session_state8) || state==_network_session_state1;
}

/* 0x0005f670: which console of a session has a number in what the session
keeps of each; NONE if none has it or the number is NONE */
long network_session_find_peer_by_channel(
	const byte *session,
	long channel_index)
{
	if (channel_index!=NONE)
	{
		const struct network_session_peer_state *state=
			(const struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET);
		long peer_index;

		for (peer_index= 0; peer_index<MAXIMUM_NETWORK_SESSION_PLAYERS; peer_index++, state++)
		{
			if (state->valid && state->unknown4==channel_index)
			{
				return peer_index;
			}
		}
	}

	return NONE;
}

/* 0x0005f600: whether something comes from the console that runs a session:
by a number of it, which network_observer_find_channel finds the thing of the console
with. Never in the last four states, which are those of the console that
runs the session itself */
boolean NETWORK_SESSION_CALLBACK network_session_channel_is_host(
	const byte *session,
	long unknown)
{
	if (network_session_has_host(session) && !network_session_unknown_00058d20(session))
	{
		long channel_index= network_observer_find_channel(*(const byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET),
			*(const long *)(session+NETWORK_SESSION_INDEX_OFFSET), unknown);
		long peer_index= network_session_find_peer_by_channel(session, channel_index);

		if (peer_index!=NONE && peer_index==*(const long *)(session+NETWORK_SESSION_UNKNOWN40_OFFSET))
		{
			return TRUE;
		}
	}

	return FALSE;
}

/* 0x0005f6a0: which console of a session has an address of the game; NONE
if none, if the session has no state or is known by nothing, or if
transport_security_peer_address_from_address cannot say what the console of the address is known by */
long network_session_find_peer_by_transport_address(
	const byte *session,
	const struct transport_address *address)
{
	if (network_session_state(session)!=_network_session_state_none && session[NETWORK_SESSION_HAS_IDENTIFIER_OFFSET])
	{
		byte peer_address[NETWORK_SESSION_ADDRESS_SIZE];

		if (transport_security_peer_address_from_address(address, *(const long *)(session+NETWORK_SESSION_UNKNOWN3C_OFFSET), NULL, NULL, NULL,
			peer_address))
		{
			return network_session_find_peer(session, peer_address);
		}
	}

	return NONE;
}

/* 0x0005f6f0: which console of a session has six bytes in what it is known
by in the network; NONE if none has them, or if the session has no state or
says nothing of who is in it */
long NETWORK_SESSION_CALLBACK network_session_find_peer_by_unknownA(
	const byte *session,
	const byte *unknownA)
{
	if (network_session_state(session)!=_network_session_state_none &&
		*(const long *)(session+NETWORK_SESSION_MEMBERSHIP_OFFSET)!=NONE)
	{
		long peer_count= *(const long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET);
		long peer_index;

		for (peer_index= 0; peer_index<peer_count; peer_index++)
		{
			if (memcmp(session+NETWORK_SESSION_PEER_ARRAY_OFFSET+peer_index*NETWORK_SESSION_PEER_SIZE+
				NETWORK_SESSION_PEER_ADDRESS_UNKNOWNA_OFFSET, unknownA, NETWORK_SESSION_PEER_ADDRESS_UNKNOWNA_SIZE)==0)
			{
				return peer_index;
			}
		}
	}

	return NONE;
}

/* 0x0005f7c0: whether an address of the game is that of the console that
runs a session: the one that the session keeps in its first state, or that of
a console of the session that is not this one and is the one at 0x40 */
boolean network_session_address_is_host(
	const byte *session,
	const struct transport_address *address)
{
	if (network_session_matches_state1_address(session, address))
	{
		return TRUE;
	}
	if (network_session_has_host(session))
	{
		long peer_index= network_session_find_peer_by_transport_address(session, address);

		if (peer_index!=NONE && peer_index!=*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET) &&
			peer_index==*(const long *)(session+NETWORK_SESSION_UNKNOWN40_OFFSET))
		{
			return TRUE;
		}
	}

	return FALSE;
}

/* 0x0005c8f0: whether an address of the game is the one that a session in
its first state keeps: of the same size, which is more than none, and with
the same bytes */
boolean network_session_matches_state1_address(
	const byte *session,
	const struct transport_address *address)
{
	if (network_session_state(session)==_network_session_state1)
	{
		const struct transport_address *kept=
			(const struct transport_address *)(session+NETWORK_SESSION_STATE1_ADDRESS_OFFSET);

		if (address->address_size>0 && address->address_size==kept->address_size &&
			memcmp(address, kept, address->address_size)==0)
		{
			return TRUE;
		}
	}

	return FALSE;
}

/* a console is sent something of a kind, with what the session is known by,
and taken out of the session. Only a console that has the second byte of
what the session keeps of it is sent anything; it is sent in the two ways if
its thing is in the seventh state. The game has this in each of the two
functions that do it */
static inline __attribute__((always_inline)) void network_session_remove_peer_with_message(
	long peer_index,
	byte *session,
	long type)
{
	const struct network_session_peer_state *state=
		(const struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET)+peer_index;
	byte identifier[NETWORK_SESSION_IDENTIFIER_SIZE];

	memcpy(identifier, session+NETWORK_SESSION_IDENTIFIER_OFFSET, sizeof(identifier));
	if (state->unknown1)
	{
		byte *unknown8= *(byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET);

		if (*(const long *)(unknown8+state->unknown4*NETWORK_SESSION_UNKNOWN8_CHANNEL_SIZE+
			NETWORK_SESSION_UNKNOWN8_CHANNEL_STATE_OFFSET)==_network_session_unknown8_channel_state7)
		{
			unknown_00075e80(FALSE, state->unknown4, unknown8, *(const long *)(session+NETWORK_SESSION_INDEX_OFFSET),
				type, sizeof(identifier), identifier);
		}
		unknown_00075e80(TRUE, state->unknown4, *(byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET),
			*(const long *)(session+NETWORK_SESSION_INDEX_OFFSET), type, sizeof(identifier), identifier);
	}
	network_session_remove_peer(session, peer_index);
}

/* 0x0005fd10: a console is sent something of the kind 0xE and taken out of
a session. Nothing in the last three states */
void network_session_remove_peer_with_message_e(
	long peer_index,
	byte *session)
{
	long state= network_session_state(session);

	if (state!=_network_session_state7 && state!=_network_session_state6 && state!=_network_session_state8)
	{
		network_session_remove_peer_with_message(peer_index, session, _network_session_message_unknownE);
	}
}

/* 0x0005fda0: a console is sent something of the kind 0xD and taken out of
a session, in any state */
void network_session_remove_peer_with_message_d(
	long peer_index,
	byte *session)
{
	network_session_remove_peer_with_message(peer_index, session, _network_session_message_unknownD);
}

/* 0x00062f40: whether a player that is known by twelve bytes asks to come
into a session; and which of those that ask it is, if there is where to say
it */
boolean NETWORK_SESSION_CALLBACK network_session_find_request(
	byte *session,
	const byte *identifier,
	struct network_session_player_request **found)
{
	struct network_session_player_request *request=
		(struct network_session_player_request *)(session+NETWORK_SESSION_UNKNOWN7668_OFFSET);
	const struct network_session_player_request *end= request+MAXIMUM_NETWORK_SESSION_PLAYER_REQUESTS;

	for (; request<end; request++)
	{
		if (request->valid && memcmp(identifier, request->identifier, sizeof(request->identifier))==0)
		{
			if (found)
			{
				*found= request;
			}

			return TRUE;
		}
	}

	return FALSE;
}

/* 0x00060200: a player goes out of a session: its console has one player
less and no player at its place, and the session has one less and not its
bit. In the last four states, if the player asked to come in, what it asked
says that the session does not have it any more */
void NETWORK_SESSION_CALLBACK network_session_remove_player(
	byte *session,
	long player_index)
{
	byte *membership= session+NETWORK_SESSION_MEMBERSHIP_OFFSET;
	const byte *player= membership+NETWORK_SESSION_MEMBERSHIP_PLAYER_ARRAY_OFFSET+player_index*NETWORK_SESSION_PLAYER_SIZE;

	if (network_session_unknown_00058d20(session))
	{
		struct network_session_player_request *request= NULL;

		if (network_session_find_request(session, player, &request))
		{
			request->has_player= FALSE;
		}
	}
	*(long *)(session+NETWORK_SESSION_PEER_ARRAY_OFFSET+
		*(const long *)(player+NETWORK_SESSION_PLAYER_PEER_OFFSET)*NETWORK_SESSION_PEER_SIZE+
		NETWORK_SESSION_PEER_PLAYERS_OFFSET+
		*(const long *)(player+NETWORK_SESSION_PLAYER_PEER_PLAYER_OFFSET)*sizeof(long))= NONE;
	(*(long *)(session+NETWORK_SESSION_PEER_ARRAY_OFFSET+
		*(const long *)(player+NETWORK_SESSION_PLAYER_PEER_OFFSET)*NETWORK_SESSION_PEER_SIZE+
		NETWORK_SESSION_PEER_PLAYER_COUNT_OFFSET))--;
	*(unsigned long *)(membership+NETWORK_SESSION_MEMBERSHIP_PLAYERS_OFFSET)&= ~(1u<<player_index);
	(*(long *)(membership+NETWORK_SESSION_MEMBERSHIP_PLAYER_COUNT_OFFSET))--;
}

/* 0x0005fe20: a console goes out of a session, and its players with it. The
consoles after it move one place down, and so does what the session keeps of
each: the players of those consoles, the console that runs the session, this
console and the one at 0x40 are told. If it was the console that runs the
session, the first one is now. The first number of who is in the session and
the one at 0x7618 count one more for each player and one more at the end;
and network_session_stop_waiting_if_done is done */
void network_session_remove_peer(
	byte *session,
	long peer_index)
{
	byte *membership= session+NETWORK_SESSION_MEMBERSHIP_OFFSET;
	long remaining= *(const long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET)-peer_index-1;
	byte *peer= session+NETWORK_SESSION_PEER_ARRAY_OFFSET+peer_index*NETWORK_SESSION_PEER_SIZE;
	struct network_session_peer_state *state=
		(struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET)+peer_index;
	long peer_count;
	long index;

	for (index= 0; index<MAXIMUM_NETWORK_SESSION_PEER_PLAYERS; index++)
	{
		long player_index= *(const long *)(peer+NETWORK_SESSION_PEER_PLAYERS_OFFSET+index*sizeof(long));

		if (player_index!=NONE)
		{
			network_session_remove_player(session, player_index);
			(*(long *)membership)++;
			(*(long *)(session+NETWORK_SESSION_UNKNOWN7618_OFFSET))++;
		}
	}
	for (index= 0; index<MAXIMUM_NETWORK_SESSION_PLAYERS; index++)
	{
		if (*(const unsigned long *)(membership+NETWORK_SESSION_MEMBERSHIP_PLAYERS_OFFSET)&(1u<<index))
		{
			long *player_peer= (long *)(membership+NETWORK_SESSION_MEMBERSHIP_PLAYER_ARRAY_OFFSET+
				index*NETWORK_SESSION_PLAYER_SIZE+NETWORK_SESSION_PLAYER_PEER_OFFSET);

			if (*player_peer>peer_index)
			{
				(*player_peer)--;
			}
		}
	}
	if (*(const long *)(session+NETWORK_SESSION_HOST_PEER_OFFSET)==peer_index)
	{
		*(long *)(session+NETWORK_SESSION_HOST_PEER_OFFSET)= 0;
	}
	network_session_peer_state_delete(session, peer_index);
	if (remaining>0)
	{
		memmove(peer, peer+NETWORK_SESSION_PEER_SIZE, remaining*NETWORK_SESSION_PEER_SIZE);
		memmove(state, state+1, remaining*sizeof(*state));
		if (*(const long *)(session+NETWORK_SESSION_UNKNOWN40_OFFSET)>peer_index)
		{
			(*(long *)(session+NETWORK_SESSION_UNKNOWN40_OFFSET))--;
		}
		if (*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET)>peer_index)
		{
			(*(long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET))--;
		}
		if (*(const long *)(session+NETWORK_SESSION_HOST_PEER_OFFSET)>peer_index)
		{
			(*(long *)(session+NETWORK_SESSION_HOST_PEER_OFFSET))--;
		}
	}
	peer_count= *(const long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET);
	memset(session+NETWORK_SESSION_PEER_ARRAY_OFFSET+(peer_count-1)*NETWORK_SESSION_PEER_SIZE, 0,
		NETWORK_SESSION_PEER_SIZE);
	memset((struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET)+(peer_count-1), 0,
		sizeof(*state));
	(*(long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET))--;
	(*(long *)membership)++;
	(*(long *)(session+NETWORK_SESSION_UNKNOWN7618_OFFSET))++;
	network_session_stop_waiting_if_done(session);
}

/* 0x0005a2e0: in the last four states, a session that waits (the byte at
0x765C) stops waiting if no console has the third byte of what the session
keeps of it: it looks like the byte says that the console has not answered
yet */
void network_session_stop_waiting_if_done(
	byte *session)
{
	if (network_session_established(session) && network_session_unknown_00058d20(session) &&
		session[NETWORK_SESSION_UNKNOWN765C_OFFSET])
	{
		long peer_index;

		for (peer_index= 0; peer_index<*(const long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET); peer_index++)
		{
			const struct network_session_peer_state *state=
				(const struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET)+peer_index;

			if (state->unknown2)
			{
				return;
			}
		}
		session[NETWORK_SESSION_UNKNOWN765C_OFFSET]= FALSE;
	}
}

/* 0x0005a340: a console answers a session that waits: by a number of it,
which network_observer_find_channel finds the thing of the console with. It counts if the
console is of the session and is not this one, the session waits, the two
numbers of the answer are those of the session and the console had not
answered: then it has, and if no console that has the second byte is left
to answer, the session stops waiting. Whether it counted */
boolean NETWORK_SESSION_CALLBACK network_session_peer_answered(
	byte *session,
	long unknown,
	const byte *answer)
{
	long channel_index= network_observer_find_channel(*(const byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET),
		*(const long *)(session+NETWORK_SESSION_INDEX_OFFSET), unknown);
	long peer_index= network_session_find_peer_by_channel(session, channel_index);
	struct network_session_peer_state *state;
	long index;

	if (peer_index==NONE || peer_index==*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET) ||
		!session[NETWORK_SESSION_UNKNOWN765C_OFFSET] ||
		*(const long *)(answer+NETWORK_SESSION_ANSWER_UNKNOWNC_OFFSET)!=
			*(const long *)(session+NETWORK_SESSION_UNKNOWN497C_OFFSET) ||
		*(const long *)(answer+NETWORK_SESSION_ANSWER_UNKNOWN8_OFFSET)!=
			*(const long *)(session+NETWORK_SESSION_UNKNOWN4980_OFFSET))
	{
		return FALSE;
	}
	state= (struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET)+peer_index;
	if (!state->unknown2)
	{
		return FALSE;
	}
	state->unknown2= FALSE;
	for (index= 0; index<*(const long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET); index++)
	{
		state= (struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET)+index;
		if (state->unknown1 && state->unknown2)
		{
			return TRUE;
		}
	}
	if (session[NETWORK_SESSION_UNKNOWN765C_OFFSET])
	{
		session[NETWORK_SESSION_UNKNOWN765C_OFFSET]= FALSE;
	}

	return TRUE;
}

/* 0x000600f0: a player comes into a session, at a place: it is known by
twelve bytes, and it is of a console, at one of the places that the console
has for its players. The console has one player more, and so does the
session, with its bit. In the last four states, if the player had asked to
come in, what it asked says that the session has it */
void NETWORK_SESSION_CALLBACK network_session_add_player(
	const byte *identifier,
	byte *session,
	long peer_index,
	long player_index,
	long peer_player_index)
{
	byte *membership= session+NETWORK_SESSION_MEMBERSHIP_OFFSET;
	byte *player= membership+NETWORK_SESSION_MEMBERSHIP_PLAYER_ARRAY_OFFSET+player_index*NETWORK_SESSION_PLAYER_SIZE;
	byte *peer= session+NETWORK_SESSION_PEER_ARRAY_OFFSET+peer_index*NETWORK_SESSION_PEER_SIZE;

	memcpy(player, identifier, NETWORK_SESSION_PLAYER_IDENTIFIER_SIZE);
	*(long *)(player+NETWORK_SESSION_PLAYER_PEER_PLAYER_OFFSET)= peer_player_index;
	*(long *)(player+NETWORK_SESSION_PLAYER_PEER_OFFSET)= peer_index;
	*(long *)(player+NETWORK_SESSION_PLAYER_UNKNOWN14_OFFSET)= NONE;
	memset(player+NETWORK_SESSION_PLAYER_UNKNOWN18_OFFSET, 0, NETWORK_SESSION_PLAYER_UNKNOWN18_SIZE);
	memset(player+NETWORK_SESSION_PLAYER_UNKNOWNA8_OFFSET, 0, NETWORK_SESSION_PLAYER_UNKNOWN18_SIZE);
	*(long *)(peer+NETWORK_SESSION_PEER_PLAYERS_OFFSET+peer_player_index*sizeof(long))= player_index;
	(*(long *)(peer+NETWORK_SESSION_PEER_PLAYER_COUNT_OFFSET))++;
	*(unsigned long *)(membership+NETWORK_SESSION_MEMBERSHIP_PLAYERS_OFFSET)|= 1u<<player_index;
	(*(long *)(membership+NETWORK_SESSION_MEMBERSHIP_PLAYER_COUNT_OFFSET))++;
	if (network_session_unknown_00058d20(session))
	{
		struct network_session_player_request *request= NULL;

		if (network_session_find_request(session, player, &request))
		{
			request->has_player= TRUE;
		}
	}
}

/* 0x000601e0: a player goes out of a session, and the two numbers that
count what changes in it count one more */
void network_session_remove_player_and_count(
	byte *session,
	long player_index)
{
	network_session_remove_player(session, player_index);
	(*(long *)(session+NETWORK_SESSION_MEMBERSHIP_OFFSET))++;
	(*(long *)(session+NETWORK_SESSION_UNKNOWN7618_OFFSET))++;
}

/* 0x00062e30: how many more players a session has room for: the number at
0x4994, less the players that it has and those that ask to come in and are
not in it yet */
long network_session_free_player_places(
	const byte *session)
{
	const struct network_session_player_request *request=
		(const struct network_session_player_request *)(session+NETWORK_SESSION_UNKNOWN7668_OFFSET);
	const struct network_session_player_request *end= request+MAXIMUM_NETWORK_SESSION_PLAYER_REQUESTS;
	long waiting= 0;

	for (; request<end; request++)
	{
		if (request->valid && !request->has_player)
		{
			waiting++;
		}
	}

	return *(const long *)(session+NETWORK_SESSION_UNKNOWN4994_OFFSET)-
		*(const long *)(session+NETWORK_SESSION_MEMBERSHIP_OFFSET+NETWORK_SESSION_MEMBERSHIP_PLAYER_COUNT_OFFSET)-waiting;
}

/* 0x00060000: the player that is known by twelve bytes is the one that a
console has at one of its places; which player of the session it is, or NONE
if it cannot be. If the console has it there already, nothing changes. If it
has another, that one goes out. The player does not come in if it is in the
session already, if what the session has at 0x78A8 says no, or if it had not
asked to come in and there is no room; if it does, it takes the first place
that is free */
long NETWORK_SESSION_CALLBACK network_session_add_peer_player(
	byte *session,
	long peer_index,
	long peer_player_index,
	const byte *identifier)
{
	byte *membership= session+NETWORK_SESSION_MEMBERSHIP_OFFSET;
	long player_index= *(const long *)(session+NETWORK_SESSION_PEER_ARRAY_OFFSET+peer_index*NETWORK_SESSION_PEER_SIZE+
		NETWORK_SESSION_PEER_PLAYERS_OFFSET+peer_player_index*sizeof(long));
	struct network_session_unknown78A8 *object;
	struct network_session_player_request *request;
	unsigned long players;
	long index;

	if (player_index!=NONE)
	{
		if (memcmp(membership+NETWORK_SESSION_MEMBERSHIP_PLAYER_ARRAY_OFFSET+player_index*NETWORK_SESSION_PLAYER_SIZE,
			identifier, NETWORK_SESSION_PLAYER_IDENTIFIER_SIZE)==0)
		{
			return player_index;
		}
		network_session_remove_player(session, player_index);
		(*(long *)membership)++;
		(*(long *)(session+NETWORK_SESSION_UNKNOWN7618_OFFSET))++;
		player_index= NONE;
	}
	if (network_session_find_player(session, identifier)!=NONE)
	{
		return player_index;
	}
	object= *(struct network_session_unknown78A8 **)(session+NETWORK_SESSION_UNKNOWN78A8_OFFSET);
	if (object && !object->functions->player_can_join(object, identifier))
	{
		return player_index;
	}
	if (!network_session_find_request(session, identifier, &request) && network_session_free_player_places(session)<=0)
	{
		return player_index;
	}
	players= *(const unsigned long *)(membership+NETWORK_SESSION_MEMBERSHIP_PLAYERS_OFFSET);
	for (index= 0; index<MAXIMUM_NETWORK_SESSION_PLAYERS; index++)
	{
		if (!(players&(1u<<index)))
		{
			player_index= index;
			break;
		}
	}
	network_session_add_player(identifier, session, peer_index, player_index, peer_player_index);
	(*(long *)membership)++;
	(*(long *)(session+NETWORK_SESSION_UNKNOWN7618_OFFSET))++;

	return player_index;
}

/* 0x000602b0: what has changed of what a session knows of a console, from
what it was: each part that is not the same is copied, and its byte set.
With nothing to compare with, all of it has changed. What has not changed is
left as it was */
void NETWORK_SESSION_CALLBACK network_session_peer_properties_get_update(
	struct network_session_peer_properties_update *update,
	const struct network_session_peer_properties *properties,
	const struct network_session_peer_properties *baseline)
{
	if (!baseline || wcsncmp(properties->unknown0, baseline->unknown0, NETWORK_SESSION_PEER_UNKNOWN28_LENGTH)!=0 ||
		wcsncmp(properties->unknown20, baseline->unknown20, NETWORK_SESSION_PEER_UNKNOWN48_LENGTH)!=0)
	{
		update->has_texts= TRUE;
		wcsncpy(update->unknown0, properties->unknown0, NETWORK_SESSION_PEER_UNKNOWN28_LENGTH-1);
		update->unknown0[NETWORK_SESSION_PEER_UNKNOWN28_LENGTH-1]= 0;
		wcsncpy(update->unknown20, properties->unknown20, NETWORK_SESSION_PEER_UNKNOWN48_LENGTH-1);
		update->unknown20[NETWORK_SESSION_PEER_UNKNOWN48_LENGTH-1]= 0;
	}
	if (!baseline || properties->unknown60!=baseline->unknown60 || properties->unknown64!=baseline->unknown64)
	{
		update->has_unknown60= TRUE;
		update->unknown60= properties->unknown60;
		update->unknown64= properties->unknown64;
	}
	if (!baseline || properties->unknown68!=baseline->unknown68 || properties->unknown6C!=baseline->unknown6C ||
		properties->unknown70!=baseline->unknown70 ||
		memcmp(properties->unknown74, baseline->unknown74, sizeof(properties->unknown74))!=0)
	{
		update->has_unknown68= TRUE;
		update->unknown68= properties->unknown68;
		update->unknown6C= properties->unknown6C;
		update->unknown70= properties->unknown70;
		memcpy(update->unknown74, properties->unknown74, sizeof(update->unknown74));
	}
	if (!baseline || memcmp(properties->unknown84, baseline->unknown84, sizeof(properties->unknown84))!=0)
	{
		update->has_unknown84= TRUE;
		memcpy(update->unknown84, properties->unknown84, sizeof(update->unknown84));
	}
	if (!baseline || properties->unknownC4!=baseline->unknownC4)
	{
		update->has_unknownC4= TRUE;
		update->unknownC4= properties->unknownC4;
	}
}

/* 0x0005a600: what a session is known by in the network; NULL if it has no
state or is known by nothing */
const byte *network_session_get_identifier(
	const byte *session)
{
	if (network_session_state(session)!=_network_session_state_none && session[NETWORK_SESSION_HAS_IDENTIFIER_OFFSET])
	{
		return session+NETWORK_SESSION_IDENTIFIER_OFFSET;
	}

	return NULL;
}

/* 0x0005a620: what a session is known by in the network, its key and the
two numbers that go with them, each if there is where to put it; FALSE, and
nothing, if the session has no state or is known by nothing */
boolean NETWORK_SESSION_CALLBACK network_session_get_security(
	const byte *session,
	byte *key,
	long *unknown38,
	long *unknown3C,
	byte *identifier)
{
	if (network_session_state(session)==_network_session_state_none || !session[NETWORK_SESSION_HAS_IDENTIFIER_OFFSET])
	{
		return FALSE;
	}
	if (identifier)
	{
		memcpy(identifier, session+NETWORK_SESSION_IDENTIFIER_OFFSET, NETWORK_SESSION_IDENTIFIER_SIZE);
	}
	if (key)
	{
		memcpy(key, session+NETWORK_SESSION_KEY_OFFSET, NETWORK_SESSION_KEY_SIZE);
	}
	if (unknown38)
	{
		*unknown38= *(const long *)(session+NETWORK_SESSION_UNKNOWN38_OFFSET);
	}
	if (unknown3C)
	{
		*unknown3C= *(const long *)(session+NETWORK_SESSION_UNKNOWN3C_OFFSET);
	}

	return TRUE;
}

/* 0x0005a680: who is in a session; and which console this one is and which
one runs the session, each if there is where to put it */
byte *network_session_get_membership(
	long *local_peer,
	byte *session,
	long *host_peer)
{
	if (local_peer)
	{
		*local_peer= *(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET);
	}
	if (host_peer)
	{
		*host_peer= *(const long *)(session+NETWORK_SESSION_UNKNOWN40_OFFSET);
	}

	return session+NETWORK_SESSION_MEMBERSHIP_OFFSET;
}

/* 0x0005a6a0: whether a console of a session has a channel; 1 or 0. The
game has network_session_find_peer_by_channel in the function itself */
long network_session_has_channel_peer(
	const byte *session,
	long channel_index)
{
	return network_session_find_peer_by_channel(session, channel_index)!=NONE;
}

/* 0x0005ade0: the console that runs a session takes a console out of it;
whether it is the one that runs it (the last four states). Another console
is told and taken out (network_session_remove_peer_with_message_e); if it is
this console itself, the session goes into its tenth state, with a 1 at the
start of what it has from its state on */
boolean network_session_host_remove_peer(
	byte *session,
	long peer_index)
{
	if (!network_session_unknown_00058d20(session))
	{
		return FALSE;
	}
	if (*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET)==peer_index)
	{
		memset(session+NETWORK_SESSION_STATE_DATA_OFFSET, 0, NETWORK_SESSION_STATE_DATA_SIZE);
		*(long *)(session+NETWORK_SESSION_STATE_DATA_OFFSET)= 1;
		*(long *)(session+NETWORK_SESSION_STATE_OFFSET)= _network_session_state10;
	}
	else
	{
		network_session_remove_peer_with_message_e(peer_index, session);
	}

	return TRUE;
}

/* 0x0005ae50: in the last four states, the console that runs a session, as
who is in it says, is this one: if it was not, the two numbers that count
what changes count one more. Whether the session is in one of those states */
boolean network_session_host_set_local(
	byte *session)
{
	if (!network_session_established(session) || !network_session_unknown_00058d20(session))
	{
		return FALSE;
	}
	if (*(const long *)(session+NETWORK_SESSION_HOST_PEER_OFFSET)!=*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET))
	{
		*(long *)(session+NETWORK_SESSION_HOST_PEER_OFFSET)= *(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET);
		(*(long *)(session+NETWORK_SESSION_MEMBERSHIP_OFFSET))++;
		(*(long *)(session+NETWORK_SESSION_UNKNOWN7618_OFFSET))++;
	}

	return TRUE;
}

/* 0x0005aed0: in the fifth state, the second of the two things of 0x90
bytes of a player is what it is given, and the two numbers that count what
changes count one more. Whether the session is in that state */
boolean NETWORK_SESSION_CALLBACK network_session_host_set_player_unknownA8(
	long player_index,
	byte *session,
	const byte *unknownA8)
{
	if (network_session_state(session)!=_network_session_state5)
	{
		return FALSE;
	}
	memcpy(session+NETWORK_SESSION_MEMBERSHIP_OFFSET+NETWORK_SESSION_MEMBERSHIP_PLAYER_ARRAY_OFFSET+
		player_index*NETWORK_SESSION_PLAYER_SIZE+NETWORK_SESSION_PLAYER_UNKNOWNA8_OFFSET, unknownA8,
		NETWORK_SESSION_PLAYER_UNKNOWN18_SIZE);
	(*(long *)(session+NETWORK_SESSION_MEMBERSHIP_OFFSET))++;
	(*(long *)(session+NETWORK_SESSION_UNKNOWN7618_OFFSET))++;

	return TRUE;
}

/* 0x0005af50: whether a session is in one of its last three states */
boolean network_session_in_last_three_states(
	const byte *session)
{
	long state= network_session_state(session);

	return state==_network_session_state7 || state==_network_session_state6 || state==_network_session_state8;
}

/* 0x0005af70: whether a session that says who is in it has no room for a
number of consoles more, or for a number of players more: the most that it
can have of each are at 0x4990 and 0x4994 */
boolean NETWORK_SESSION_CALLBACK network_session_is_full_for(
	const byte *session,
	long peer_count,
	long player_count)
{
	if (!network_session_established(session))
	{
		return FALSE;
	}

	return *(const long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET)+peer_count>
			*(const long *)(session+NETWORK_SESSION_UNKNOWN4990_OFFSET) ||
		*(const long *)(session+NETWORK_SESSION_MEMBERSHIP_OFFSET+NETWORK_SESSION_MEMBERSHIP_PLAYER_COUNT_OFFSET)+
			player_count>*(const long *)(session+NETWORK_SESSION_UNKNOWN4994_OFFSET);
}

/* 0x0005afc0: whether a session has room for a number of players: those of
them that had asked to come in have their room already, unless the first
thing says that none had. The game has network_session_free_player_places in
the function itself */
boolean NETWORK_SESSION_CALLBACK network_session_has_room_for_players(
	boolean all_new,
	const byte *identifiers,
	byte *session,
	long player_count)
{
	long asked= 0;

	if (!all_new)
	{
		long index;

		for (index= 0; index<player_count; index++)
		{
			if (network_session_find_request(session, identifiers+index*NETWORK_SESSION_PLAYER_IDENTIFIER_SIZE, NULL))
			{
				asked++;
			}
		}
	}

	return player_count<=network_session_free_player_places(session)+asked;
}

/* 0x0005aeb0: in the fifth state, unknown_000614a0 is done with a number
and no byte; whether the session is in that state */
boolean NETWORK_SESSION_CALLBACK network_session_host_unknown_0005aeb0(
	byte *session,
	long unknown)
{
	if (network_session_state(session)!=_network_session_state5)
	{
		return FALSE;
	}
	unknown_000614a0(FALSE, session, unknown);

	return TRUE;
}

/* 0x0005b040: a byte of a session that says who is in it; FALSE if it does
not */
boolean network_session_get_unknown49c4(
	const byte *session)
{
	return network_session_established(session) ? session[NETWORK_SESSION_UNKNOWN49C4_OFFSET] : FALSE;
}

/* 0x0005b060: a number of a session that says who is in it; NONE if it does
not */
long network_session_get_unknown49c8(
	const byte *session)
{
	return network_session_established(session) ? *(const long *)(session+NETWORK_SESSION_UNKNOWN49C8_OFFSET) : NONE;
}

/* 0x0005b080: a thing of a session that says who is in it, if a byte says
that it is there; NULL if not */
const byte *network_session_get_unknown4a00(
	const byte *session)
{
	if (network_session_established(session) && session[NETWORK_SESSION_UNKNOWN49FD_OFFSET])
	{
		return session+NETWORK_SESSION_UNKNOWN4A00_OFFSET;
	}

	return NULL;
}

/* 0x0005b0b0: the console that a session that says who is in it has chosen
for something (the voice goes through it: see game_voice.c); NONE if it does
not say who is in it */
long network_session_get_chosen_peer(
	const byte *session)
{
	return network_session_established(session) ? *(const long *)(session+NETWORK_SESSION_UNKNOWN5E20_OFFSET) : NONE;
}

/* 0x0005b0d0: two numbers and a thing of a session that says who is in it;
FALSE, and nothing, if it does not, if the second number is NONE or if the
first byte of the thing is zero */
boolean NETWORK_SESSION_CALLBACK network_session_get_unknown4d08(
	const byte *session,
	long *unknown4d08,
	long *unknown4d0c,
	const byte **unknown4d10)
{
	if (!network_session_established(session) || *(const long *)(session+NETWORK_SESSION_UNKNOWN4D0C_OFFSET)==NONE ||
		!session[NETWORK_SESSION_UNKNOWN4D10_OFFSET])
	{
		return FALSE;
	}
	*unknown4d08= *(const long *)(session+NETWORK_SESSION_UNKNOWN4D08_OFFSET);
	*unknown4d0c= *(const long *)(session+NETWORK_SESSION_UNKNOWN4D0C_OFFSET);
	*unknown4d10= session+NETWORK_SESSION_UNKNOWN4D10_OFFSET;

	return TRUE;
}

/* 0x0005b120: the eight bytes made at random of a session that says who is
in it; all ones if it does not */
struct network_session_random network_session_get_random(
	const byte *session)
{
	struct network_session_random random= { NONE, NONE };

	if (network_session_established(session))
	{
		random.low= *(const unsigned long *)(session+NETWORK_SESSION_RANDOM_OFFSET);
		random.high= *(const unsigned long *)(session+NETWORK_SESSION_RANDOM_OFFSET+sizeof(long));
	}

	return random;
}

/* 0x0005b150: the language of a session that says who is in it; the
language of the game if it does not */
long network_session_get_language(
	const byte *session)
{
	if (global_language==NONE)
	{
		global_language= language_from_xbox_language(XGetLanguage());
	}

	return network_session_established(session) ? *(const long *)(session+NETWORK_SESSION_LANGUAGE_OFFSET) :
		global_language;
}

/* 0x0005b180: the most players that a session that says who is in it can
have; 16 if it does not */
long network_session_get_maximum_players(
	const byte *session)
{
	return network_session_established(session) ? *(const long *)(session+NETWORK_SESSION_UNKNOWN4994_OFFSET) :
		NETWORK_SESSION_UNKNOWN4990_DEFAULT;
}

/* 0x0005b1a0: a thing of 0x44 bytes of a session that says who is in it, if
a byte says that it is there; FALSE, and nothing, if not */
boolean NETWORK_SESSION_CALLBACK network_session_get_unknown5ddc(
	const byte *session,
	byte *unknown5ddc)
{
	if (!network_session_established(session) || !session[NETWORK_SESSION_UNKNOWN5DD8_OFFSET])
	{
		return FALSE;
	}
	memcpy(unknown5ddc, session+NETWORK_SESSION_UNKNOWN5DDC_OFFSET, NETWORK_SESSION_UNKNOWN5DDC_SIZE);

	return TRUE;
}

/* whether a session is in one of the four states of the console that runs
it: the game asks first whether it says who is in it, in each function that
changes what the session knows */
static inline __attribute__((always_inline)) boolean network_session_is_host(
	const byte *session)
{
	return network_session_established(session) && network_session_unknown_00058d20(session);
}

/* what the session knows has changed: the number at its start counts it */
static inline __attribute__((always_inline)) void network_session_parameters_changed(
	byte *session)
{
	(*(long *)(session+NETWORK_SESSION_UNKNOWN4978_OFFSET))++;
}

/* whether an address of the game is one that a session keeps: of the same
size, which is more than none, and with the same bytes */
static inline __attribute__((always_inline)) boolean network_session_address_equal(
	const struct transport_address *address,
	const struct transport_address *kept)
{
	return address->address_size>0 && address->address_size==kept->address_size &&
		memcmp(address, kept, address->address_size)==0;
}

/* 0x0005c190: FALSE if a session has a state and a thing that it is given
has, after its first four bytes, what the session is known by; TRUE if not,
or with no thing */
boolean network_session_unknown_0005c190(
	const byte *unknown,
	const byte *session)
{
	if (network_session_state(session)!=_network_session_state_none && unknown &&
		memcmp(unknown+NETWORK_SESSION_UNKNOWN5DDC_IDENTIFIER_OFFSET, session+NETWORK_SESSION_IDENTIFIER_OFFSET,
			NETWORK_SESSION_IDENTIFIER_SIZE)==0)
	{
		return FALSE;
	}

	return TRUE;
}

/* 0x0005c1c0: the console that runs a session sets the two numbers of
0x49F0, or says that there are none; whether it is the one that runs it */
boolean network_session_host_set_unknown49f0(
	const long *unknown49f0,
	byte *session)
{
	if (!network_session_is_host(session))
	{
		return FALSE;
	}
	session[NETWORK_SESSION_UNKNOWN49E8_OFFSET]= unknown49f0!=NULL;
	if (unknown49f0)
	{
		*(long *)(session+NETWORK_SESSION_UNKNOWN49F0_OFFSET)= unknown49f0[0];
		*(long *)(session+NETWORK_SESSION_UNKNOWN49F0_OFFSET+sizeof(long))= unknown49f0[1];
	}
	else
	{
		*(long *)(session+NETWORK_SESSION_UNKNOWN49F0_OFFSET)= 0;
		*(long *)(session+NETWORK_SESSION_UNKNOWN49F0_OFFSET+sizeof(long))= 0;
	}
	network_session_parameters_changed(session);

	return TRUE;
}

/* 0x0005c240: the console that runs a session clears the byte of 0x49FC,
if it was set; whether it is the one that runs it */
boolean network_session_host_clear_unknown49fc(
	byte *session)
{
	if (!network_session_is_host(session))
	{
		return FALSE;
	}
	if (session[NETWORK_SESSION_UNKNOWN49FC_OFFSET])
	{
		session[NETWORK_SESSION_UNKNOWN49FC_OFFSET]= FALSE;
		network_session_parameters_changed(session);
	}

	return TRUE;
}

/* 0x0005c290: the console that runs a session sets the number of 0x49F8, if
it is another; whether it is the one that runs it */
boolean network_session_host_set_unknown49f8(
	byte *session,
	long unknown49f8)
{
	if (!network_session_is_host(session))
	{
		return FALSE;
	}
	if (*(const long *)(session+NETWORK_SESSION_UNKNOWN49F8_OFFSET)!=unknown49f8)
	{
		*(long *)(session+NETWORK_SESSION_UNKNOWN49F8_OFFSET)= unknown49f8;
		network_session_parameters_changed(session);
	}

	return TRUE;
}

/* 0x0005c2e0: the console that runs a session sets the 0x1C bytes of
0x49CC, if they are others; whether it is the one that runs it */
boolean network_session_host_set_unknown49cc(
	byte *session,
	const byte *unknown49cc)
{
	if (!network_session_is_host(session))
	{
		return FALSE;
	}
	if (memcmp(unknown49cc, session+NETWORK_SESSION_UNKNOWN49CC_OFFSET, NETWORK_SESSION_UNKNOWN49CC_SIZE)!=0)
	{
		memcpy(session+NETWORK_SESSION_UNKNOWN49CC_OFFSET, unknown49cc, NETWORK_SESSION_UNKNOWN49CC_SIZE);
		network_session_parameters_changed(session);
	}

	return TRUE;
}

/* 0x0005c350: the console that runs a session sets the thing of 0x4A00, or
says that there is none; whether it could: it has to be the one that runs
it, and network_session_roster_valid has to say that the thing is right */
boolean NETWORK_SESSION_CALLBACK network_session_host_set_unknown4a00(
	const byte *unknown4a00,
	byte *session)
{
	if (!network_session_is_host(session))
	{
		return FALSE;
	}
	if (unknown4a00)
	{
		if (!network_session_roster_valid((const struct network_session_roster *)unknown4a00))
		{
			return FALSE;
		}
		session[NETWORK_SESSION_UNKNOWN49FD_OFFSET]= TRUE;
		memcpy(session+NETWORK_SESSION_UNKNOWN4A00_OFFSET, unknown4a00, NETWORK_SESSION_UNKNOWN4A00_SIZE);
	}
	else
	{
		session[NETWORK_SESSION_UNKNOWN49FD_OFFSET]= FALSE;
		memset(session+NETWORK_SESSION_UNKNOWN4A00_OFFSET, 0, NETWORK_SESSION_UNKNOWN4A00_SIZE);
	}
	network_session_parameters_changed(session);

	return TRUE;
}

/* 0x0005c3f0: the console that runs a session sets the thing of 0x5DDC, or
says that there is none; whether it could: it has to be the one that runs
it, and the thing cannot have what this session is known by */
boolean NETWORK_SESSION_CALLBACK network_session_host_set_unknown5ddc(
	const byte *unknown5ddc,
	byte *session)
{
	if (!network_session_is_host(session))
	{
		return FALSE;
	}
	if (unknown5ddc)
	{
		if (!network_session_unknown_0005c190(unknown5ddc, session))
		{
			return FALSE;
		}
		session[NETWORK_SESSION_UNKNOWN5DD8_OFFSET]= TRUE;
		memcpy(session+NETWORK_SESSION_UNKNOWN5DDC_OFFSET, unknown5ddc, NETWORK_SESSION_UNKNOWN5DDC_SIZE);
	}
	else
	{
		session[NETWORK_SESSION_UNKNOWN5DD8_OFFSET]= FALSE;
		memset(session+NETWORK_SESSION_UNKNOWN5DDC_OFFSET, 0, NETWORK_SESSION_UNKNOWN5DDC_SIZE);
	}
	network_session_parameters_changed(session);

	return TRUE;
}

/* 0x0005c490: the console that runs a session sets the most players that
it can have; whether it is the one that runs it */
boolean NETWORK_SESSION_CALLBACK network_session_host_set_maximum_players(
	byte *session,
	long maximum_players)
{
	if (!network_session_is_host(session))
	{
		return FALSE;
	}
	*(long *)(session+NETWORK_SESSION_UNKNOWN4994_OFFSET)= maximum_players;
	network_session_parameters_changed(session);

	return TRUE;
}

/* 0x0005c4e0: the console that runs a session sets its eight bytes made at
random; whether it is the one that runs it */
boolean NETWORK_SESSION_CALLBACK network_session_host_set_random(
	byte *session,
	unsigned long low,
	unsigned long high)
{
	if (!network_session_is_host(session))
	{
		return FALSE;
	}
	*(unsigned long *)(session+NETWORK_SESSION_RANDOM_OFFSET)= low;
	network_session_parameters_changed(session);
	*(unsigned long *)(session+NETWORK_SESSION_RANDOM_OFFSET+sizeof(long))= high;

	return TRUE;
}

/* 0x0005c530: the console that runs a session clears the byte of 0x49C4;
whether it is the one that runs it */
boolean network_session_host_clear_unknown49c4(
	byte *session)
{
	if (!network_session_is_host(session))
	{
		return FALSE;
	}
	session[NETWORK_SESSION_UNKNOWN49C4_OFFSET]= FALSE;
	network_session_parameters_changed(session);

	return TRUE;
}

/* 0x0005c570: the console that runs a session sets the number of 0x4DA8;
whether it is the one that runs it */
boolean NETWORK_SESSION_CALLBACK network_session_host_set_unknown4da8(
	byte *session,
	long unknown4da8)
{
	if (!network_session_is_host(session))
	{
		return FALSE;
	}
	*(long *)(session+NETWORK_SESSION_UNKNOWN4DA8_OFFSET)= unknown4da8;
	network_session_parameters_changed(session);

	return TRUE;
}

/* 0x0005c5c0: the console that runs a session sets the two things of 0x4F24
and 0x4F90, or says that there are none; whether it is the one that runs
it */
boolean NETWORK_SESSION_CALLBACK network_session_host_set_unknown4f24(
	const byte *unknown4f24,
	byte *session,
	const byte *unknown4f90)
{
	if (!network_session_is_host(session))
	{
		return FALSE;
	}
	if (unknown4f24)
	{
		session[NETWORK_SESSION_UNKNOWN4F20_OFFSET]= TRUE;
		memcpy(session+NETWORK_SESSION_UNKNOWN4F24_OFFSET, unknown4f24, NETWORK_SESSION_UNKNOWN4F24_SIZE);
		memcpy(session+NETWORK_SESSION_UNKNOWN4F90_OFFSET, unknown4f90, NETWORK_SESSION_UNKNOWN4F90_SIZE);
	}
	else
	{
		session[NETWORK_SESSION_UNKNOWN4F20_OFFSET]= FALSE;
		memset(session+NETWORK_SESSION_UNKNOWN4F24_OFFSET, 0, NETWORK_SESSION_UNKNOWN4F24_SIZE);
		memset(session+NETWORK_SESSION_UNKNOWN4F90_OFFSET, 0, NETWORK_SESSION_UNKNOWN4F90_SIZE);
	}
	network_session_parameters_changed(session);

	return TRUE;
}

/* 0x0005c660: the console that runs a session sets the eight bytes of
0x4999, or says that there are none; whether it is the one that runs it */
boolean network_session_host_set_unknown4999(
	const byte *unknown4999,
	byte *session)
{
	if (!network_session_is_host(session))
	{
		return FALSE;
	}
	if (unknown4999)
	{
		session[NETWORK_SESSION_UNKNOWN4998_OFFSET]= TRUE;
		memcpy(session+NETWORK_SESSION_UNKNOWN4999_OFFSET, unknown4999, NETWORK_SESSION_UNKNOWN4999_SIZE);
	}
	else
	{
		session[NETWORK_SESSION_UNKNOWN4998_OFFSET]= FALSE;
	}
	network_session_parameters_changed(session);

	return TRUE;
}

/* 0x0005c6d0: the console that runs a session chooses a console for
something: see network_session_get_chosen_peer. Nothing if it is not the one
that runs it */
void network_session_host_set_chosen_peer(
	byte *session,
	long peer_index)
{
	if (network_session_is_host(session))
	{
		*(long *)(session+NETWORK_SESSION_UNKNOWN5E20_OFFSET)= peer_index;
		network_session_parameters_changed(session);
	}
}

/* 0x0005c940: whether an address of the game is the one that a session in
its second state keeps */
boolean network_session_matches_state2_address(
	const struct transport_address *address,
	const byte *session)
{
	return network_session_state(session)==_network_session_state2 && network_session_address_equal(address,
		(const struct transport_address *)(session+NETWORK_SESSION_STATE2_ADDRESS_OFFSET));
}

/* 0x0005c990: unknown_0005a520 is done with a session in its second state,
if an address of the game is the one that it keeps. The game has
network_session_matches_state2_address in the function itself */
void network_session_unknown_0005c990(
	const struct transport_address *address,
	byte *session)
{
	if (network_session_state(session)==_network_session_state2 && network_session_address_equal(address,
		(const struct transport_address *)(session+NETWORK_SESSION_STATE2_ADDRESS_OFFSET)))
	{
		unknown_0005a520(session);
	}
}

/* 0x00062b40: the 0x34 bytes of 0x761C of a session start over, and the
number after them counts one more; the two after that are NONE */
void network_session_reset_unknown761c(
	byte *session)
{
	(*(long *)(session+NETWORK_SESSION_UNKNOWN7650_OFFSET))++;
	memset(session+NETWORK_SESSION_UNKNOWN761C_OFFSET, 0, NETWORK_SESSION_UNKNOWN761C_SIZE);
	*(long *)(session+NETWORK_SESSION_UNKNOWN7654_OFFSET)= NONE;
	*(long *)(session+NETWORK_SESSION_UNKNOWN7658_OFFSET)= NONE;
}

/* 0x00062d20: something of a kind and a size is sent to a console of a
session, in one way, in the other or in both: see
_network_session_send_both. Only to a console that has the second byte of
what the session keeps of it */
void NETWORK_SESSION_CALLBACK network_session_send_to_peer(
	long peer_index,
	long mode,
	byte *session,
	long type,
	long size,
	const void *data)
{
	const struct network_session_peer_state *state=
		(const struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET)+peer_index;

	if (!state->unknown1)
	{
		return;
	}
	if (mode==_network_session_send_unknown0 || (mode==_network_session_send_both &&
		*(const long *)(*(byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET)+
			state->unknown4*NETWORK_SESSION_UNKNOWN8_CHANNEL_SIZE+NETWORK_SESSION_UNKNOWN8_CHANNEL_STATE_OFFSET)==
			_network_session_unknown8_channel_state7))
	{
		unknown_00075e80(FALSE, state->unknown4, *(byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET),
			*(const long *)(session+NETWORK_SESSION_INDEX_OFFSET), type, size, data);
	}
	if (mode==_network_session_send_unknown1 || mode==_network_session_send_both)
	{
		unknown_00075e80(TRUE, state->unknown4, *(byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET),
			*(const long *)(session+NETWORK_SESSION_INDEX_OFFSET), type, size, data);
	}
}

/* 0x00062da0: something is sent to each console of a session */
void NETWORK_SESSION_CALLBACK network_session_send_to_peers(
	long mode,
	byte *session,
	long type,
	long size,
	const void *data)
{
	long peer_index;

	for (peer_index= 0; peer_index<*(const long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET); peer_index++)
	{
		network_session_send_to_peer(peer_index, mode, session, type, size, data);
	}
}

/* 0x00062de0: the players that asked to come into a session longer ago than
their last number says, in milliseconds, do not ask any more. With NONE for
that number, they always do */
void network_session_requests_expire(
	byte *session)
{
	struct network_session_player_request *request=
		(struct network_session_player_request *)(session+NETWORK_SESSION_UNKNOWN7668_OFFSET);
	const struct network_session_player_request *end= request+MAXIMUM_NETWORK_SESSION_PLAYER_REQUESTS;

	for (; request<end; request++)
	{
		if (request->valid && request->unknown20!=NONE &&
			(unsigned long)(network_session_time()-request->time)>(unsigned long)request->unknown20)
		{
			request->valid= FALSE;
		}
	}
}

/* 0x00062fa0: the players that asked to come into a session with eight
bytes of their console do not ask any more; how many they were */
long NETWORK_SESSION_CALLBACK network_session_requests_remove(
	byte *session,
	const byte *unknown2)
{
	struct network_session_player_request *request=
		(struct network_session_player_request *)(session+NETWORK_SESSION_UNKNOWN7668_OFFSET);
	const struct network_session_player_request *end= request+MAXIMUM_NETWORK_SESSION_PLAYER_REQUESTS;
	long count= 0;

	for (; request<end; request++)
	{
		if (request->valid && memcmp(unknown2, request->unknown2, sizeof(request->unknown2))==0)
		{
			request->valid= FALSE;
			count++;
		}
	}

	return count;
}

/* 0x00062fe0: whether a console that asks to come into a session can: 0 if
it can, and if not why. Only the console that runs the session says, and
only in the fifth state; not with more than 1 at 0x498C. A console that is
in the session already can; another needs room for one console and for its
players, of which those that had asked have theirs */
long network_session_host_check_join(
	byte *session,
	const byte *join)
{
	long state= network_session_state(session);
	long player_count;

	if (!network_session_unknown_00058d20(session))
	{
		return _network_session_join_not_host;
	}
	if (*(const long *)(session+NETWORK_SESSION_UNKNOWN498C_OFFSET)>1)
	{
		return _network_session_join_unknown2;
	}
	if (state==_network_session_state7 || state==_network_session_state6 || state==_network_session_state8)
	{
		return _network_session_join_not_host;
	}
	if (network_session_find_peer(session, join+NETWORK_SESSION_JOIN_ADDRESS_OFFSET)!=NONE)
	{
		return _network_session_join_ok;
	}
	player_count= *(const long *)(join+NETWORK_SESSION_JOIN_PLAYER_COUNT_OFFSET);
	if (network_session_is_full_for(session, 1, player_count) ||
		!network_session_has_room_for_players(join[NETWORK_SESSION_JOIN_UNKNOWN14C_OFFSET],
			join+NETWORK_SESSION_JOIN_PLAYERS_OFFSET, session, player_count))
	{
		return _network_session_join_full;
	}

	return _network_session_join_ok;
}

/* 0x00063190: which player of a roster is known by twelve bytes: the first
that is; NONE if none */
long NETWORK_SESSION_CALLBACK network_session_roster_find_player(
	const struct network_session_roster *roster,
	const byte *identifier)
{
	long found= NONE;
	long index;

	for (index= 0; index<roster->player_count; index++)
	{
		if (found==NONE && memcmp(roster->player_identifiers[index], identifier, sizeof(roster->player_identifiers[index]))==0)
		{
			found= index;
		}
	}

	return found;
}

/* 0x000631f0: the most that the players of a group of a roster have in the
first of their two numbers; NONE if the group has no player */
long network_session_roster_group_maximum_unknown248(
	const struct network_session_roster *roster,
	long group_index)
{
	long maximum= NONE;
	long index;

	for (index= 0; index<roster->player_count; index++)
	{
		if (roster->player_groups[index]==group_index && maximum<=roster->player_unknown248[index])
		{
			maximum= roster->player_unknown248[index];
		}
	}

	return maximum;
}

/* 0x00063280: which group of a roster is known by twelve bytes: the first
that is; NONE if none */
long NETWORK_SESSION_CALLBACK network_session_roster_find_group_by_unknown84(
	const struct network_session_roster *roster,
	const byte *unknown84)
{
	long found= NONE;
	long index;

	for (index= 0; index<roster->group_count; index++)
	{
		if (found==NONE && memcmp(roster->group_unknown84[index], unknown84, sizeof(roster->group_unknown84[index]))==0)
		{
			found= index;
		}
	}

	return found;
}

/* 0x000632e0: which group of a roster is known by eight bytes: the first
that is; NONE if none */
long NETWORK_SESSION_CALLBACK network_session_roster_find_group_by_unknown4(
	const struct network_session_roster *roster,
	const byte *unknown4)
{
	long found= NONE;
	long index;

	for (index= 0; index<roster->group_count; index++)
	{
		if (found==NONE && memcmp(roster->group_unknown4[index], unknown4, sizeof(roster->group_unknown4[index]))==0)
		{
			found= index;
		}
	}

	return found;
}

/* 0x00063340: whether a roster is right: no more than sixteen groups and
sixteen players, and each player in a group that there is and with its two
numbers NONE or in what they can be. The game stops at the first player that
is not right */
boolean network_session_roster_valid(
	const struct network_session_roster *roster)
{
	boolean valid= roster->group_count>=0 && roster->group_count<=MAXIMUM_NETWORK_SESSION_PLAYERS &&
		roster->player_count>=0 && roster->player_count<=MAXIMUM_NETWORK_SESSION_PLAYERS;
	long index;

	for (index= 0; index<roster->player_count && valid; index++)
	{
		long group_index= roster->player_groups[index];
		long unknown248= roster->player_unknown248[index];
		long unknown288= roster->player_unknown288[index];

		valid= group_index>=0 && group_index<roster->group_count &&
			(unknown248==NONE || (unknown248>=0 && unknown248<=MAXIMUM_NETWORK_SESSION_ROSTER_UNKNOWN248)) &&
			(unknown288==NONE || (unknown288>=0 && unknown288<=MAXIMUM_NETWORK_SESSION_ROSTER_UNKNOWN288));
	}

	return valid;
}

/* 0x00063510: whether any of a number of players, each known by twelve
bytes, is in a roster */
boolean NETWORK_SESSION_CALLBACK network_session_roster_has_any_player(
	const byte *identifiers,
	const struct network_session_roster *roster,
	long count)
{
	boolean found= FALSE;
	long index;

	for (index= 0; index<count && !found; index++)
	{
		if (network_session_roster_find_player(roster, identifiers+index*NETWORK_SESSION_PLAYER_IDENTIFIER_SIZE)!=NONE)
		{
			found= TRUE;
		}
	}

	return found;
}

/* 0x00063ca0: how many bits of a number are set */
long network_count_bits(
	unsigned long value)
{
	value= ((value>>1)&0x55555555)+(value&0x55555555);
	value= ((value>>2)&0x33333333)+(value&0x33333333);
	value= ((value>>4)&0x0F0F0F0F)+(value&0x0F0F0F0F);
	value= ((value>>8)&0x00FF00FF)+(value&0x00FF00FF);

	return (value>>16)+(value&0xFFFF);
}

/* 0x00063d00: whether two players are known by the same: the first eight of
their twelve bytes, and with the last thing two bits of the rest too. FALSE
with no player */
boolean NETWORK_SESSION_CALLBACK network_player_identifier_equal(
	const unsigned long *identifier,
	const unsigned long *other,
	boolean with_flags)
{
	if (!identifier || !other)
	{
		return FALSE;
	}

	return identifier[0]==other[0] && identifier[1]==other[1] &&
		(!with_flags || !((identifier[2]^other[2])&NETWORK_SESSION_PLAYER_IDENTIFIER_FLAGS_MASK));
}

/* what a session has from its state on starts over, and it goes into a
state: the game has this in each function that changes the state */
static inline __attribute__((always_inline)) byte *network_session_set_state(
	byte *session,
	long state)
{
	byte *data= session+NETWORK_SESSION_STATE_DATA_OFFSET;

	memset(data, 0, NETWORK_SESSION_STATE_DATA_SIZE);
	*(long *)(session+NETWORK_SESSION_STATE_OFFSET)= state;

	return data;
}

/* how many milliseconds ago a time was: the game takes one from the other
as the processor does, so that it can come out less than nothing */
static inline __attribute__((always_inline)) long network_session_elapsed(
	long time)
{
	return (long)((unsigned long)network_session_time()-(unsigned long)time);
}

/* 0x0005e150: something comes from an address of the game: whether it is
that of the console that runs a session. If it is, and it is the one that the
session keeps in its first state, unknown_0005a520 is done; if it is not
that one, unknown_00062ab0 is. The game asks twice whether it is the one
that the session keeps */
boolean network_session_unknown_0005e150(
	const struct transport_address *address,
	byte *session)
{
	if (!network_session_address_is_host(session, address))
	{
		return FALSE;
	}
	if (network_session_matches_state1_address(session, address))
	{
		if (network_session_matches_state1_address(session, address))
		{
			unknown_0005a520(session);
		}
	}
	else
	{
		unknown_00062ab0(session);
	}

	return TRUE;
}

/* 0x0005e1a0: the same, but if the address is not the one that the session
keeps in its first state, the session goes into its tenth state with a 1 */
boolean network_session_unknown_0005e1a0(
	const struct transport_address *address,
	byte *session)
{
	if (!network_session_address_is_host(session, address))
	{
		return FALSE;
	}
	if (network_session_matches_state1_address(session, address))
	{
		if (network_session_matches_state1_address(session, address))
		{
			unknown_0005a520(session);
		}
	}
	else
	{
		*(long *)network_session_set_state(session, _network_session_state10)= 1;
	}

	return TRUE;
}

/* 0x0005f5c0: which console of a session has a channel; NONE if none: it is
network_session_find_peer_by_channel, with its arguments in other places */
long NETWORK_SESSION_CALLBACK network_session_get_channel_peer(
	const byte *session,
	long channel_index)
{
	return network_session_find_peer_by_channel(session, channel_index);
}

/* 0x000612c0: a session goes into its third state: the 0x34 bytes of 0x761C
start over (network_session_reset_unknown761c, which the game has here), the
state starts now, and unknown_00062990 is done */
void network_session_enter_state3(
	byte *session)
{
	long now= network_session_time();

	network_session_reset_unknown761c(session);
	*(long *)network_session_set_state(session, _network_session_state3)= now;
	unknown_00062990(session);
}

/* 0x00061330: a session goes into its fourth state: it starts now, with
NONE after the time, and unknown_00062480 is done */
void network_session_enter_state4(
	byte *session)
{
	long now= network_session_time();
	long *data= (long *)network_session_set_state(session, _network_session_state4);

	data[0]= now;
	data[1]= NONE;
	unknown_00062480(session);
}

/* 0x00061450: a session goes into its sixth state: each console is sent
something of the kind 0xD in both ways, with what the session is known by */
void network_session_enter_state6(
	byte *session)
{
	byte identifier[NETWORK_SESSION_IDENTIFIER_SIZE];

	memcpy(identifier, session+NETWORK_SESSION_IDENTIFIER_OFFSET, sizeof(identifier));
	network_session_send_to_peers(_network_session_send_both, session, _network_session_message_unknownD,
		sizeof(identifier), identifier);
	network_session_set_state(session, _network_session_state6);
}

/* 0x000616b0: a session goes into its tenth state, with a number */
void NETWORK_SESSION_CALLBACK network_session_enter_state10(
	byte *session,
	long unknown)
{
	*(long *)network_session_set_state(session, _network_session_state10)= unknown;
}

/* 0x000618d0: unknown_0005a520 is done with a session if the time of 0x7488
was more than unknown_004cf4a4 milliseconds ago; if not, unknown_000623e0 */
void network_session_unknown_000618d0(
	byte *session)
{
	if (network_session_elapsed(*(const long *)(session+NETWORK_SESSION_STATE_DATA_UNKNOWN68_OFFSET))>unknown_004cf4a4)
	{
		unknown_0005a520(session);
	}
	else
	{
		unknown_000623e0(session);
	}
}

/* 0x00061910: unknown_0005a520 is done with a session if its state started
more than unknown_004cf4ac milliseconds ago; if not, unknown_00062480 */
void network_session_unknown_00061910(
	byte *session)
{
	if (network_session_elapsed(*(const long *)(session+NETWORK_SESSION_STATE_DATA_OFFSET))>unknown_004cf4ac)
	{
		unknown_0005a520(session);
	}
	else
	{
		unknown_00062480(session);
	}
}

/* 0x00061950: in what a session has from its state on, a console loses its
bit; and if it is the one that the session has there, there is none any
more, since now */
void network_session_unknown_00061950(
	long peer_index,
	byte *session)
{
	*(unsigned long *)(session+NETWORK_SESSION_STATE_DATA_UNKNOWN4_OFFSET)&= ~(1u<<peer_index);
	if (*(const long *)(session+NETWORK_SESSION_STATE_DATA_UNKNOWNC_OFFSET)==peer_index)
	{
		session[NETWORK_SESSION_STATE_DATA_UNKNOWN1C_OFFSET]= FALSE;
		session[NETWORK_SESSION_STATE_DATA_UNKNOWN10_OFFSET]= FALSE;
		*(long *)(session+NETWORK_SESSION_STATE_DATA_UNKNOWNC_OFFSET)= NONE;
		*(long *)(session+NETWORK_SESSION_STATE_DATA_UNKNOWN8_OFFSET)= network_session_time();
	}
}

/* which console of a session something that comes by one of the game's
channels is from; NONE if none: the game has this in each function that
takes something that comes */
static inline __attribute__((always_inline)) long network_session_channel_peer(
	const byte *session,
	long channel)
{
	return network_session_find_peer_by_channel(session, network_observer_find_channel(
		*(const byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET), *(const long *)(session+NETWORK_SESSION_INDEX_OFFSET),
		channel));
}

/* 0x0005bec0: the console that runs a session does unknown_0005df30 for the
console that runs it, as who is in it says, with nothing; whether it is the
one that runs it */
boolean network_session_host_unknown_0005bec0(
	byte *session)
{
	if (!network_session_is_host(session))
	{
		return FALSE;
	}
	unknown_0005df30(session, FALSE, 0, 0, *(const long *)(session+NETWORK_SESSION_HOST_PEER_OFFSET), NULL);

	return TRUE;
}

/* 0x0005dea0: something comes to the console that runs a session from
another console of it: unknown_0005df30 is done for that console with what
comes. FALSE if this console does not run the session or what comes is not
from another console of it */
boolean NETWORK_SESSION_CALLBACK network_session_host_unknown_0005dea0(
	const byte *message,
	byte *session,
	long channel)
{
	long peer_index;
	long unknown10;

	if (!network_session_is_host(session))
	{
		return FALSE;
	}
	peer_index= network_session_channel_peer(session, channel);
	if (peer_index==NONE || peer_index==*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET))
	{
		return FALSE;
	}
	unknown10= *(const long *)(message+NETWORK_SESSION_MESSAGE_UNKNOWN10_OFFSET);
	unknown_0005df30(session, message[NETWORK_SESSION_MESSAGE_UNKNOWN8_OFFSET],
		*(const long *)(message+NETWORK_SESSION_MESSAGE_UNKNOWNC_OFFSET), unknown10, peer_index,
		unknown10==1 ? message+NETWORK_SESSION_MESSAGE_UNKNOWN14_OFFSET : NULL);

	return TRUE;
}

/* 0x0005e5b0: something comes by one of the game's channels. If it is from
the console that runs a session: unknown_00062ab0 is done, in a session that
says who is in it and that this console does not run; and unknown_0005a400,
in a session in its first state. TRUE */
boolean network_session_unknown_0005e5b0(
	byte *session,
	long channel)
{
	long peer_index= network_session_channel_peer(session, channel);

	if (network_session_established(session) && !network_session_unknown_00058d20(session))
	{
		if (peer_index==*(const long *)(session+NETWORK_SESSION_UNKNOWN40_OFFSET))
		{
			unknown_00062ab0(session);
		}
	}
	else if (network_session_state(session)==_network_session_state1 &&
		peer_index==*(const long *)(session+NETWORK_SESSION_UNKNOWN40_OFFSET))
	{
		unknown_0005a400(session, 0);
	}

	return TRUE;
}

/* 0x0005e640: in the eighth state, something comes from a console of a
session that is neither this one nor the one at 0x40: it gets its bit in
what the session has from its state on, and the fourth byte of what the
session keeps of it is cleared. Whether it did */
boolean NETWORK_SESSION_CALLBACK network_session_unknown_0005e640(
	byte *session,
	long channel)
{
	if (network_session_state(session)==_network_session_state8)
	{
		long peer_index= network_session_channel_peer(session, channel);

		if (peer_index!=NONE && peer_index!=*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET) &&
			peer_index!=*(const long *)(session+NETWORK_SESSION_UNKNOWN40_OFFSET))
		{
			*(unsigned long *)(session+NETWORK_SESSION_STATE8_PEERS_OFFSET)|= 1u<<peer_index;
			((struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET)+peer_index)->unknown3= FALSE;

			return TRUE;
		}
	}

	return FALSE;
}

/* 0x0005f120: the console that runs a session says that a player of this
console comes in, at one of its four places: if the place is one and this
console has no player there, what the player is known by is kept at 0x761C.
Only in a session that says who is in it, and if what comes is from the
console that runs it. FALSE */
boolean NETWORK_SESSION_CALLBACK network_session_local_player_accepted(
	byte *session,
	const byte *message,
	long channel)
{
	if (network_session_established(session) && network_session_channel_is_host(session, channel))
	{
		long peer_player_index= *(const long *)(message+NETWORK_SESSION_MESSAGE_PEER_PLAYER_OFFSET);

		if (peer_player_index>=0 && peer_player_index<MAXIMUM_NETWORK_SESSION_PEER_PLAYERS &&
			*(const long *)(session+NETWORK_SESSION_PEER_ARRAY_OFFSET+
				*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET)*NETWORK_SESSION_PEER_SIZE+
				NETWORK_SESSION_PEER_PLAYERS_OFFSET+peer_player_index*sizeof(long))==NONE)
		{
			struct network_session_local_player *player=
				(struct network_session_local_player *)(session+NETWORK_SESSION_UNKNOWN761C_OFFSET)+peer_player_index;

			player->valid= TRUE;
			memcpy(player->identifier, message+NETWORK_SESSION_MESSAGE_IDENTIFIER_OFFSET, sizeof(player->identifier));
		}
	}

	return FALSE;
}

/* 0x0005f190: another console of a session tells the console that runs it
that the player at one of its four places goes out: if it has a player
there, it goes out (network_session_remove_player_and_count). Whether one
went out */
boolean NETWORK_SESSION_CALLBACK network_session_host_remove_peer_player(
	byte *session,
	long channel,
	const byte *message)
{
	if (network_session_is_host(session))
	{
		long peer_index= network_session_channel_peer(session, channel);

		if (peer_index!=NONE && peer_index!=*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET))
		{
			long peer_player_index= *(const long *)(message+NETWORK_SESSION_MESSAGE_PEER_PLAYER_OFFSET);

			if (peer_player_index>=0 && peer_player_index<MAXIMUM_NETWORK_SESSION_PEER_PLAYERS)
			{
				long player_index= *(const long *)(session+NETWORK_SESSION_PEER_ARRAY_OFFSET+
					peer_index*NETWORK_SESSION_PEER_SIZE+NETWORK_SESSION_PEER_PLAYERS_OFFSET+
					peer_player_index*sizeof(long));

				if (player_index!=NONE)
				{
					network_session_remove_player_and_count(session, player_index);

					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

/* what the console that leads a session sends to ask for a change: empty,
with what the session is known by */
static inline __attribute__((always_inline)) void network_session_parameters_request_new(
	const byte *session,
	byte *request)
{
	memset(request, 0, NETWORK_SESSION_PARAMETERS_REQUEST_SIZE);
	memcpy(request, session+NETWORK_SESSION_IDENTIFIER_OFFSET, NETWORK_SESSION_IDENTIFIER_SIZE);
}

/* it is sent to the console that runs the session, the one at 0x40, if the
session keeps the second byte of it */
static inline __attribute__((always_inline)) void network_session_parameters_request_send(
	byte *session,
	const byte *request)
{
	const struct network_session_peer_state *state=
		(const struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET)+
		*(const long *)(session+NETWORK_SESSION_UNKNOWN40_OFFSET);

	if (state->unknown1)
	{
		unknown_00075e80(FALSE, state->unknown4, *(byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET),
			*(const long *)(session+NETWORK_SESSION_INDEX_OFFSET), _network_session_message_parameters_request,
			NETWORK_SESSION_PARAMETERS_REQUEST_SIZE, request);
	}
}

/* 0x0005b1e0: the console that leads a session (the one that who is in it
says runs it) asks for something with a number: if it does run it,
unknown_0005a220 is done; if not, it is asked of the one that does. FALSE if
this console does not lead the session */
boolean network_session_leader_unknown_0005b1e0(
	byte *session,
	long unknown)
{
	if (!network_session_local_peer_is_host(session))
	{
		return FALSE;
	}
	if (network_session_unknown_00058d20(session))
	{
		unknown_0005a220(session, unknown);
	}
	else
	{
		byte request[NETWORK_SESSION_PARAMETERS_REQUEST_SIZE];

		network_session_parameters_request_new(session, request);
		request[NETWORK_SESSION_PARAMETERS_REQUEST_HAS_UNKNOWNC_OFFSET]= TRUE;
		*(long *)(request+NETWORK_SESSION_PARAMETERS_REQUEST_UNKNOWNC_OFFSET)= unknown;
		network_session_parameters_request_send(session, request);
	}

	return TRUE;
}

/* 0x0005b2c0: the console that leads a session sets the number of 0x49A4,
or asks the one that runs it to; FALSE if it does not lead it */
boolean network_session_leader_set_unknown49a4(
	byte *session,
	long unknown49a4)
{
	if (!network_session_local_peer_is_host(session))
	{
		return FALSE;
	}
	if (network_session_unknown_00058d20(session))
	{
		network_session_parameters_changed(session);
		*(long *)(session+NETWORK_SESSION_UNKNOWN49A4_OFFSET)= unknown49a4;
	}
	else
	{
		byte request[NETWORK_SESSION_PARAMETERS_REQUEST_SIZE];

		network_session_parameters_request_new(session, request);
		request[NETWORK_SESSION_PARAMETERS_REQUEST_HAS_UNKNOWN49A4_OFFSET]= TRUE;
		*(long *)(request+NETWORK_SESSION_PARAMETERS_REQUEST_UNKNOWN49A4_OFFSET)= unknown49a4;
		network_session_parameters_request_send(session, request);
	}

	return TRUE;
}

/* 0x0005b3a0: the console that leads a session sets the byte of 0x49C4, or
asks the one that runs it to; FALSE if it does not lead it */
boolean network_session_leader_set_unknown49c4(
	byte *session)
{
	if (!network_session_local_peer_is_host(session))
	{
		return FALSE;
	}
	if (network_session_unknown_00058d20(session))
	{
		network_session_parameters_changed(session);
		session[NETWORK_SESSION_UNKNOWN49C4_OFFSET]= TRUE;
	}
	else
	{
		byte request[NETWORK_SESSION_PARAMETERS_REQUEST_SIZE];

		network_session_parameters_request_new(session, request);
		request[NETWORK_SESSION_PARAMETERS_REQUEST_HAS_UNKNOWN49C4_OFFSET]= TRUE;
		request[NETWORK_SESSION_PARAMETERS_REQUEST_UNKNOWN49C4_OFFSET]= TRUE;
		network_session_parameters_request_send(session, request);
	}

	return TRUE;
}

/* 0x0005b490: the console that leads a session sets the number of 0x49C8,
or asks the one that runs it to; FALSE if it does not lead it */
boolean network_session_leader_set_unknown49c8(
	byte *session,
	long unknown49c8)
{
	if (!network_session_local_peer_is_host(session))
	{
		return FALSE;
	}
	if (network_session_unknown_00058d20(session))
	{
		network_session_parameters_changed(session);
		*(long *)(session+NETWORK_SESSION_UNKNOWN49C8_OFFSET)= unknown49c8;
	}
	else
	{
		byte request[NETWORK_SESSION_PARAMETERS_REQUEST_SIZE];

		network_session_parameters_request_new(session, request);
		request[NETWORK_SESSION_PARAMETERS_REQUEST_HAS_UNKNOWN49C8_OFFSET]= TRUE;
		*(long *)(request+NETWORK_SESSION_PARAMETERS_REQUEST_UNKNOWN49C8_OFFSET)= unknown49c8;
		network_session_parameters_request_send(session, request);
	}

	return TRUE;
}

/* 0x0005b570: the console that leads a session sets the number of 0x49F8 to
zero, or asks the one that runs it to; FALSE if it does not lead it */
boolean network_session_leader_clear_unknown49f8(
	byte *session)
{
	if (!network_session_local_peer_is_host(session))
	{
		return FALSE;
	}
	if (network_session_unknown_00058d20(session))
	{
		network_session_parameters_changed(session);
		*(long *)(session+NETWORK_SESSION_UNKNOWN49F8_OFFSET)= 0;
	}
	else
	{
		byte request[NETWORK_SESSION_PARAMETERS_REQUEST_SIZE];

		network_session_parameters_request_new(session, request);
		request[NETWORK_SESSION_PARAMETERS_REQUEST_HAS_UNKNOWN49F8_OFFSET]= TRUE;
		*(long *)(request+NETWORK_SESSION_PARAMETERS_REQUEST_UNKNOWN49F8_OFFSET)= 0;
		network_session_parameters_request_send(session, request);
	}

	return TRUE;
}

/* 0x0005b8e0: the console that leads a session sets the number of 0x4DAC,
or asks the one that runs it to; FALSE if it does not lead it */
boolean network_session_leader_set_unknown4dac(
	byte *session,
	long unknown4dac)
{
	if (!network_session_local_peer_is_host(session))
	{
		return FALSE;
	}
	if (network_session_unknown_00058d20(session))
	{
		network_session_parameters_changed(session);
		*(long *)(session+NETWORK_SESSION_UNKNOWN4DAC_OFFSET)= unknown4dac;
	}
	else
	{
		byte request[NETWORK_SESSION_PARAMETERS_REQUEST_SIZE];

		network_session_parameters_request_new(session, request);
		request[NETWORK_SESSION_PARAMETERS_REQUEST_HAS_UNKNOWN4DAC_OFFSET]= TRUE;
		*(long *)(request+NETWORK_SESSION_PARAMETERS_REQUEST_UNKNOWN4DAC_OFFSET)= unknown4dac;
		network_session_parameters_request_send(session, request);
	}

	return TRUE;
}

/* 0x0005bb20: the console that leads a session sets the byte of 0x49A1 from
the one that it is given a place of, or asks the one that runs it to; FALSE
if it does not lead it */
boolean network_session_leader_set_unknown49a1(
	byte *session,
	const byte *unknown49a1)
{
	if (!network_session_local_peer_is_host(session))
	{
		return FALSE;
	}
	if (network_session_unknown_00058d20(session))
	{
		session[NETWORK_SESSION_UNKNOWN49A1_OFFSET]= *unknown49a1;
		network_session_parameters_changed(session);
	}
	else
	{
		byte request[NETWORK_SESSION_PARAMETERS_REQUEST_SIZE];

		network_session_parameters_request_new(session, request);
		request[NETWORK_SESSION_PARAMETERS_REQUEST_HAS_UNKNOWN49A1_OFFSET]= TRUE;
		request[NETWORK_SESSION_PARAMETERS_REQUEST_UNKNOWN49A1_OFFSET]= *unknown49a1;
		network_session_parameters_request_send(session, request);
	}

	return TRUE;
}

/* 0x0005bc10: the console that leads a session sets the 16 bits of 0x5DD0,
or asks the one that runs it to; FALSE if it does not lead it */
boolean network_session_leader_set_unknown5dd0(
	byte *session,
	short unknown5dd0)
{
	if (!network_session_local_peer_is_host(session))
	{
		return FALSE;
	}
	if (network_session_unknown_00058d20(session))
	{
		network_session_parameters_changed(session);
		*(short *)(session+NETWORK_SESSION_UNKNOWN5DD0_OFFSET)= unknown5dd0;
	}
	else
	{
		byte request[NETWORK_SESSION_PARAMETERS_REQUEST_SIZE];

		network_session_parameters_request_new(session, request);
		request[NETWORK_SESSION_PARAMETERS_REQUEST_HAS_UNKNOWN5DD0_OFFSET]= TRUE;
		*(short *)(request+NETWORK_SESSION_PARAMETERS_REQUEST_UNKNOWN5DD0_OFFSET)= unknown5dd0;
		network_session_parameters_request_send(session, request);
	}

	return TRUE;
}

/* 0x0005bd00: the console that leads a session sets its language, or asks
the one that runs it to; FALSE if it does not lead it */
boolean network_session_leader_set_language(
	byte *session,
	long language)
{
	if (!network_session_local_peer_is_host(session))
	{
		return FALSE;
	}
	if (network_session_unknown_00058d20(session))
	{
		network_session_parameters_changed(session);
		*(long *)(session+NETWORK_SESSION_LANGUAGE_OFFSET)= language;
	}
	else
	{
		byte request[NETWORK_SESSION_PARAMETERS_REQUEST_SIZE];

		network_session_parameters_request_new(session, request);
		request[NETWORK_SESSION_PARAMETERS_REQUEST_HAS_LANGUAGE_OFFSET]= TRUE;
		*(long *)(request+NETWORK_SESSION_PARAMETERS_REQUEST_LANGUAGE_OFFSET)= language;
		network_session_parameters_request_send(session, request);
	}

	return TRUE;
}

/* 0x0005bde0: the console that leads a session sets the number of 0x498C,
or asks the one that runs it to; FALSE if it does not lead it */
boolean network_session_leader_set_unknown498c(
	byte *session,
	long unknown498c)
{
	if (!network_session_local_peer_is_host(session))
	{
		return FALSE;
	}
	if (network_session_unknown_00058d20(session))
	{
		network_session_parameters_changed(session);
		*(long *)(session+NETWORK_SESSION_UNKNOWN498C_OFFSET)= unknown498c;
	}
	else
	{
		byte request[NETWORK_SESSION_PARAMETERS_REQUEST_SIZE];

		network_session_parameters_request_new(session, request);
		request[NETWORK_SESSION_PARAMETERS_REQUEST_HAS_UNKNOWN498C_OFFSET]= TRUE;
		*(long *)(request+NETWORK_SESSION_PARAMETERS_REQUEST_UNKNOWN498C_OFFSET)= unknown498c;
		network_session_parameters_request_send(session, request);
	}

	return TRUE;
}

/* 0x0005b650: the console that leads a session sets its roster, or says
that there is none, or asks the one that runs it to; whether it could: it
has to lead the session, and the roster has to be right */
boolean network_session_leader_set_roster(
	const struct network_session_roster *roster,
	byte *session)
{
	if (!network_session_local_peer_is_host(session) || (roster && !network_session_roster_valid(roster)))
	{
		return FALSE;
	}
	if (network_session_unknown_00058d20(session))
	{
		if (roster)
		{
			session[NETWORK_SESSION_UNKNOWN49FD_OFFSET]= TRUE;
			memcpy(session+NETWORK_SESSION_UNKNOWN4A00_OFFSET, roster, sizeof(*roster));
		}
		else
		{
			session[NETWORK_SESSION_UNKNOWN49FD_OFFSET]= FALSE;
			memset(session+NETWORK_SESSION_UNKNOWN4A00_OFFSET, 0, sizeof(*roster));
		}
		network_session_parameters_changed(session);
	}
	else
	{
		byte request[NETWORK_SESSION_PARAMETERS_REQUEST_SIZE];

		network_session_parameters_request_new(session, request);
		request[NETWORK_SESSION_PARAMETERS_REQUEST_HAS_ROSTER_OFFSET]= TRUE;
		if (roster)
		{
			request[NETWORK_SESSION_PARAMETERS_REQUEST_ROSTER_VALID_OFFSET]= TRUE;
			memcpy(request+NETWORK_SESSION_PARAMETERS_REQUEST_ROSTER_OFFSET, roster, sizeof(*roster));
		}
		network_session_parameters_request_send(session, request);
	}

	return TRUE;
}

/* 0x0005b790: the console that leads a session sets the two numbers of
0x4D08 and the text of 0x4D10, or asks the one that runs it to; FALSE if it
does not lead it */
boolean NETWORK_SESSION_CALLBACK network_session_leader_set_unknown4d08(
	byte *session,
	const char *unknown4d10,
	long unknown4d08,
	long unknown4d0c)
{
	if (!network_session_local_peer_is_host(session))
	{
		return FALSE;
	}
	if (network_session_unknown_00058d20(session))
	{
		char *text= (char *)(session+NETWORK_SESSION_UNKNOWN4D10_OFFSET);

		*(long *)(session+NETWORK_SESSION_UNKNOWN4D08_OFFSET)= unknown4d08;
		*(long *)(session+NETWORK_SESSION_UNKNOWN4D0C_OFFSET)= unknown4d0c;
		strncpy(text, unknown4d10, NETWORK_SESSION_UNKNOWN4D10_SIZE);
		text[NETWORK_SESSION_UNKNOWN4D10_SIZE-1]= 0;
		network_session_parameters_changed(session);
	}
	else
	{
		byte request[NETWORK_SESSION_PARAMETERS_REQUEST_SIZE];
		char *text= (char *)(request+NETWORK_SESSION_PARAMETERS_REQUEST_UNKNOWN4D10_OFFSET);

		network_session_parameters_request_new(session, request);
		request[NETWORK_SESSION_PARAMETERS_REQUEST_HAS_UNKNOWN4D08_OFFSET]= TRUE;
		*(long *)(request+NETWORK_SESSION_PARAMETERS_REQUEST_UNKNOWN4D08_OFFSET)= unknown4d08;
		*(long *)(request+NETWORK_SESSION_PARAMETERS_REQUEST_UNKNOWN4D0C_OFFSET)= unknown4d0c;
		strncpy(text, unknown4d10, NETWORK_SESSION_UNKNOWN4D10_SIZE);
		text[NETWORK_SESSION_UNKNOWN4D10_SIZE-1]= 0;
		network_session_parameters_request_send(session, request);
	}

	return TRUE;
}

/* 0x0005b9d0: the console that leads a session sets the thing of 0x4DB0, or
asks the one that runs it to; FALSE if it does not lead it. With no thing,
or with one whose number at 0x44 is zero, it is all zeros */
boolean network_session_leader_set_unknown4db0(
	const byte *unknown4db0,
	byte *session)
{
	byte thing[NETWORK_SESSION_UNKNOWN4DB0_SIZE];

	if (unknown4db0 && *(const long *)(unknown4db0+NETWORK_SESSION_UNKNOWN4DB0_UNKNOWN44_OFFSET)!=0)
	{
		memcpy(thing, unknown4db0, sizeof(thing));
	}
	else
	{
		memset(thing, 0, sizeof(thing));
	}
	if (!network_session_local_peer_is_host(session))
	{
		return FALSE;
	}
	if (network_session_unknown_00058d20(session))
	{
		network_session_parameters_changed(session);
		memcpy(session+NETWORK_SESSION_UNKNOWN4DB0_OFFSET, thing, sizeof(thing));
	}
	else
	{
		byte request[NETWORK_SESSION_PARAMETERS_REQUEST_SIZE];

		network_session_parameters_request_new(session, request);
		request[NETWORK_SESSION_PARAMETERS_REQUEST_HAS_UNKNOWN4DB0_OFFSET]= TRUE;
		memcpy(request+NETWORK_SESSION_PARAMETERS_REQUEST_UNKNOWN4DB0_OFFSET, thing, sizeof(thing));
		network_session_parameters_request_send(session, request);
	}

	return TRUE;
}

/* 0x0005bf10: unknown_0005df30 is done for this console, with a byte, two
numbers and a thing: by this console if it runs the session, and if not it
is asked of the one that does, with something of the kind 0x23 that
network_session_host_unknown_0005dea0 takes there. FALSE if the session does
not say who is in it */
boolean NETWORK_SESSION_CALLBACK network_session_unknown_0005bf10(
	byte *session,
	boolean unknown8,
	long unknownC,
	long unknown10,
	const byte *unknown14)
{
	if (!network_session_established(session))
	{
		return FALSE;
	}
	if (network_session_unknown_00058d20(session))
	{
		unknown_0005df30(session, unknown8, unknownC, unknown10, *(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET),
			unknown14);
	}
	else
	{
		byte message[NETWORK_SESSION_MESSAGE_UNKNOWN23_SIZE];
		const struct network_session_peer_state *state=
			(const struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET)+
			*(const long *)(session+NETWORK_SESSION_UNKNOWN40_OFFSET);

		memset(message, 0, sizeof(message));
		memcpy(message, session+NETWORK_SESSION_IDENTIFIER_OFFSET, NETWORK_SESSION_IDENTIFIER_SIZE);
		message[NETWORK_SESSION_MESSAGE_UNKNOWN8_OFFSET]= unknown8;
		*(long *)(message+NETWORK_SESSION_MESSAGE_UNKNOWNC_OFFSET)= unknownC;
		*(long *)(message+NETWORK_SESSION_MESSAGE_UNKNOWN10_OFFSET)= unknown10;
		if (unknown10==1)
		{
			memcpy(message+NETWORK_SESSION_MESSAGE_UNKNOWN14_OFFSET, unknown14, NETWORK_SESSION_MESSAGE_UNKNOWN14_SIZE);
		}
		if (state->unknown1)
		{
			unknown_00075e80(FALSE, state->unknown4, *(byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET),
				*(const long *)(session+NETWORK_SESSION_INDEX_OFFSET), _network_session_message_unknown23,
				sizeof(message), message);
		}
	}

	return TRUE;
}

/* 0x0005c010: the console that leads a session sets the thing of 0x5DDC, or
says that there is none, or asks the one that runs it to; whether it could:
it has to lead the session, and the thing cannot have what this session is
known by */
boolean NETWORK_SESSION_CALLBACK network_session_leader_set_unknown5ddc(
	byte *session,
	const byte *unknown5ddc)
{
	if (!network_session_established(session) || !network_session_unknown_0005c190(unknown5ddc, session) ||
		!network_session_local_peer_is_host(session))
	{
		return FALSE;
	}
	if (network_session_unknown_00058d20(session))
	{
		if (unknown5ddc)
		{
			session[NETWORK_SESSION_UNKNOWN5DD8_OFFSET]= TRUE;
			memcpy(session+NETWORK_SESSION_UNKNOWN5DDC_OFFSET, unknown5ddc, NETWORK_SESSION_UNKNOWN5DDC_SIZE);
		}
		else
		{
			session[NETWORK_SESSION_UNKNOWN5DD8_OFFSET]= FALSE;
			memset(session+NETWORK_SESSION_UNKNOWN5DDC_OFFSET, 0, NETWORK_SESSION_UNKNOWN5DDC_SIZE);
		}
		network_session_parameters_changed(session);
	}
	else
	{
		byte request[NETWORK_SESSION_PARAMETERS_REQUEST_SIZE];

		network_session_parameters_request_new(session, request);
		request[NETWORK_SESSION_PARAMETERS_REQUEST_HAS_UNKNOWN5DDC_OFFSET]= TRUE;
		if (unknown5ddc)
		{
			request[NETWORK_SESSION_PARAMETERS_REQUEST_UNKNOWN5DDC_VALID_OFFSET]= TRUE;
			memcpy(request+NETWORK_SESSION_PARAMETERS_REQUEST_UNKNOWN5DDC_OFFSET, unknown5ddc,
				NETWORK_SESSION_UNKNOWN5DDC_SIZE);
		}
		network_session_parameters_request_send(session, request);
	}

	return TRUE;
}
