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

/* 0x0007b140: sends something of a kind and a size to an address of the
game */
void __attribute__((stdcall)) unknown_0007b140(long unknown4, const struct transport_address *address, long type,
	long size, const void *data);

/* ---------- globals */

/* 0x00510548, 0x0051054c, of main.c: whether the time is the one that was
kept, and that time */
extern boolean unknown_00510548;
extern long unknown_0051054c;

/* 0x004cf4a4, 0x004cf4ac: how long a session waits in two of its states
before it is disconnected, in milliseconds. 0x004cf4a0, 0x004cf4b0: how long
it waits in those states before it sends again what it sends */
long unknown_004cf4a4;
long unknown_004cf4ac;
long unknown_004cf4a0;
long unknown_004cf4b0;

/* 0x004cf4d8, 0x004cf4dc: how long a session in its third state waits after
the time of 0x7654, and after the last time, before it sends again what it
sends there */
long unknown_004cf4d8;
long unknown_004cf4dc;

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

/* 0x0005aeb0: in the fifth state, network_session_enter_state7 is done with a number
and no byte; whether the session is in that state */
boolean NETWORK_SESSION_CALLBACK network_session_host_unknown_0005aeb0(
	byte *session,
	long unknown)
{
	if (network_session_state(session)!=_network_session_state5)
	{
		return FALSE;
	}
	network_session_enter_state7(FALSE, session, unknown);

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

/* 0x0005c990: network_session_disconnect is done with a session in its second state,
if an address of the game is the one that it keeps. The game has
network_session_matches_state2_address in the function itself */
void network_session_unknown_0005c990(
	const struct transport_address *address,
	byte *session)
{
	if (network_session_state(session)==_network_session_state2 && network_session_address_equal(address,
		(const struct transport_address *)(session+NETWORK_SESSION_STATE2_ADDRESS_OFFSET)))
	{
		network_session_disconnect(session);
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
session keeps in its first state, network_session_disconnect is done; if it is not
that one, network_session_host_lost is. The game asks twice whether it is the one
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
			network_session_disconnect(session);
		}
	}
	else
	{
		network_session_host_lost(session);
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
			network_session_disconnect(session);
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
state starts now, and network_session_state3_update is done */
void network_session_enter_state3(
	byte *session)
{
	long now= network_session_time();

	network_session_reset_unknown761c(session);
	*(long *)network_session_set_state(session, _network_session_state3)= now;
	network_session_state3_update(session);
}

/* 0x00061330: a session goes into its fourth state: it starts now, with
NONE after the time, and network_session_state4_update is done */
void network_session_enter_state4(
	byte *session)
{
	long now= network_session_time();
	long *data= (long *)network_session_set_state(session, _network_session_state4);

	data[0]= now;
	data[1]= NONE;
	network_session_state4_update(session);
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

/* 0x000618d0: network_session_disconnect is done with a session if the time of 0x7488
was more than unknown_004cf4a4 milliseconds ago; if not, network_session_unknown_000623e0 */
void network_session_unknown_000618d0(
	byte *session)
{
	if (network_session_elapsed(*(const long *)(session+NETWORK_SESSION_STATE_DATA_UNKNOWN68_OFFSET))>unknown_004cf4a4)
	{
		network_session_disconnect(session);
	}
	else
	{
		network_session_unknown_000623e0(session);
	}
}

/* 0x00061910: network_session_disconnect is done with a session if its state started
more than unknown_004cf4ac milliseconds ago; if not, network_session_state4_update */
void network_session_unknown_00061910(
	byte *session)
{
	if (network_session_elapsed(*(const long *)(session+NETWORK_SESSION_STATE_DATA_OFFSET))>unknown_004cf4ac)
	{
		network_session_disconnect(session);
	}
	else
	{
		network_session_state4_update(session);
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

/* 0x0005bec0: the console that runs a session does network_session_host_set_unknown49a8 for the
console that runs it, as who is in it says, with nothing; whether it is the
one that runs it */
boolean network_session_host_unknown_0005bec0(
	byte *session)
{
	if (!network_session_is_host(session))
	{
		return FALSE;
	}
	network_session_host_set_unknown49a8(session, FALSE, 0, 0, *(const long *)(session+NETWORK_SESSION_HOST_PEER_OFFSET), NULL);

	return TRUE;
}

/* 0x0005dea0: something comes to the console that runs a session from
another console of it: network_session_host_set_unknown49a8 is done for that console with what
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
	network_session_host_set_unknown49a8(session, message[NETWORK_SESSION_MESSAGE_UNKNOWN8_OFFSET],
		*(const long *)(message+NETWORK_SESSION_MESSAGE_UNKNOWNC_OFFSET), unknown10, peer_index,
		unknown10==1 ? message+NETWORK_SESSION_MESSAGE_UNKNOWN14_OFFSET : NULL);

	return TRUE;
}

/* 0x0005e5b0: something comes by one of the game's channels. If it is from
the console that runs a session: network_session_host_lost is done, in a session that
says who is in it and that this console does not run; and network_session_leave,
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
			network_session_host_lost(session);
		}
	}
	else if (network_session_state(session)==_network_session_state1 &&
		peer_index==*(const long *)(session+NETWORK_SESSION_UNKNOWN40_OFFSET))
	{
		network_session_leave(session, 0);
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
network_session_host_set_unknown4980 is done; if not, it is asked of the one that does. FALSE if
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
		network_session_host_set_unknown4980(session, unknown);
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
boolean NETWORK_SESSION_CALLBACK network_session_leader_set_map(
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

/* 0x0005bf10: network_session_host_set_unknown49a8 is done for this console, with a byte, two
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
		network_session_host_set_unknown49a8(session, unknown8, unknownC, unknown10, *(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET),
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

/* 0x0005a220: the console that runs a session sets the number of 0x4980, if
it is another: the session waits for each console that it talks with to
answer, since now (see network_session_peer_answered); the number of 0x497C
and the one that counts what changes count one more, and the time of what
the session knows is now. With no console to wait for, it does not wait */
void network_session_host_set_unknown4980(
	byte *session,
	long unknown4980)
{
	boolean any= FALSE;
	long peer_index;

	if (*(const long *)(session+NETWORK_SESSION_UNKNOWN4980_OFFSET)==unknown4980)
	{
		return;
	}
	for (peer_index= 0; peer_index<*(const long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET); peer_index++)
	{
		struct network_session_peer_state *state=
			(struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET)+peer_index;

		if (state->unknown1)
		{
			state->unknown2= TRUE;
			any= TRUE;
		}
	}
	*(long *)(session+NETWORK_SESSION_UNKNOWN7660_OFFSET)= *(const long *)(session+NETWORK_SESSION_UNKNOWN4980_OFFSET);
	session[NETWORK_SESSION_UNKNOWN765C_OFFSET]= TRUE;
	*(long *)(session+NETWORK_SESSION_UNKNOWN7664_OFFSET)= network_session_time();
	*(long *)(session+NETWORK_SESSION_UNKNOWN4980_OFFSET)= unknown4980;
	(*(long *)(session+NETWORK_SESSION_UNKNOWN497C_OFFSET))++;
	*(long *)(session+NETWORK_SESSION_START_TIME_OFFSET)= network_session_time();
	network_session_parameters_changed(session);
	if (!any && session[NETWORK_SESSION_UNKNOWN765C_OFFSET])
	{
		session[NETWORK_SESSION_UNKNOWN765C_OFFSET]= FALSE;
	}
}

/* 0x0005a400: this console goes out of a session, by its state. Nothing
with no state, if network_session_unknown_00058d90 says so, or with the byte
of 0x48. In the first state network_session_enter_state2 is done; from the third it
goes into the fourth; in the fifth and the seventh, with the thing that it
is given it goes into the sixth, and without it network_session_enter_state7 is done (in
the seventh, only a byte is set); in the eighth a byte is set; in the ninth,
with the thing the other consoles are sent something of the kind 0x17, and
without it network_session_disconnect is done; and in the tenth, network_session_disconnect
without it */
void NETWORK_SESSION_CALLBACK network_session_leave(
	byte *session,
	boolean unknown)
{
	long state= network_session_state(session);

	if (state==_network_session_state_none || network_session_unknown_00058d90(session) ||
		session[NETWORK_SESSION_UNKNOWN48_OFFSET])
	{
		return;
	}
	switch (state)
	{
	case _network_session_state1:
		network_session_enter_state2(session);
		break;
	case _network_session_state3:
		network_session_enter_state4(session);
		break;
	case _network_session_state5:
	case _network_session_state7:
		if (unknown)
		{
			network_session_enter_state6(session);
		}
		else if (state==_network_session_state7)
		{
			session[NETWORK_SESSION_UNKNOWN7420_OFFSET]= TRUE;
		}
		else
		{
			network_session_enter_state7(TRUE, session, NONE);
		}
		break;
	case _network_session_state8:
		session[NETWORK_SESSION_UNKNOWN7420_OFFSET]= TRUE;
		break;
	case _network_session_state9:
		if (unknown)
		{
			byte message[NETWORK_SESSION_MESSAGE_UNKNOWN17_SIZE];

			memset(message, 0, sizeof(message));
			memcpy(message, session+NETWORK_SESSION_IDENTIFIER_OFFSET, NETWORK_SESSION_IDENTIFIER_SIZE);
			*(long *)(message+NETWORK_SESSION_MESSAGE_UNKNOWN17_UNKNOWN8_OFFSET)= NETWORK_SESSION_MESSAGE_UNKNOWN17_UNKNOWN8;
			network_session_send_to_peers(_network_session_send_both, session, _network_session_message_unknown17,
				sizeof(message), message);
		}
		else
		{
			network_session_disconnect(session);
		}
		break;
	case _network_session_state10:
		if (!unknown)
		{
			network_session_disconnect(session);
		}
		break;
	}
}

/* 0x0005a6e0: the byte of 0x49FC of a session is set: by this console if
it runs the session, and if not it is asked of the one that does. FALSE if
the session does not say who is in it */
boolean network_session_request_unknown49fc(
	byte *session)
{
	if (!network_session_established(session))
	{
		return FALSE;
	}
	if (network_session_unknown_00058d20(session))
	{
		network_session_parameters_changed(session);
		session[NETWORK_SESSION_UNKNOWN49FC_OFFSET]= TRUE;
	}
	else
	{
		byte request[NETWORK_SESSION_PARAMETERS_REQUEST_SIZE];

		network_session_parameters_request_new(session, request);
		request[NETWORK_SESSION_PARAMETERS_REQUEST_HAS_UNKNOWN49FC_OFFSET]= TRUE;
		request[NETWORK_SESSION_PARAMETERS_REQUEST_UNKNOWN49FC_OFFSET]= TRUE;
		network_session_parameters_request_send(session, request);
	}

	return TRUE;
}

/* 0x0005a520: a session is disconnected: it goes back to how it is when it
is set up, with no state. First this console goes out of it
(network_session_leave) and what the session has at 0x78A8 is told; then
nothing is kept of any console, it is known by nothing, nobody is in it and
nobody asks to come in. Nothing with no state */
void network_session_disconnect(
	byte *session)
{
	struct network_session_unknown78A8 *object;
	long peer_index;

	if (network_session_state(session)==_network_session_state_none)
	{
		return;
	}
	network_session_leave(session, TRUE);
	object= *(struct network_session_unknown78A8 **)(session+NETWORK_SESSION_UNKNOWN78A8_OFFSET);
	if (object)
	{
		object->functions->disconnected(object);
	}
	for (peer_index= 0; peer_index<MAXIMUM_NETWORK_SESSION_PLAYERS; peer_index++)
	{
		if (((const struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET)+peer_index)->valid)
		{
			network_session_peer_state_delete(session, peer_index);
		}
	}
	(*(long *)(session+NETWORK_SESSION_UNKNOWN7650_OFFSET))++;
	*(long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET)= NONE;
	memset(session+NETWORK_SESSION_UNKNOWN761C_OFFSET, 0, NETWORK_SESSION_UNKNOWN761C_SIZE);
	*(long *)(session+NETWORK_SESSION_UNKNOWN7654_OFFSET)= NONE;
	*(long *)(session+NETWORK_SESSION_UNKNOWN7658_OFFSET)= NONE;
	session[NETWORK_SESSION_UNKNOWN78AC_OFFSET]= FALSE;
	session[NETWORK_SESSION_UNKNOWN765C_OFFSET]= FALSE;
	network_session_forget_identifier(session);
	memset(session+NETWORK_SESSION_IDENTIFIER_OFFSET, 0, NETWORK_SESSION_IDENTIFIER_SIZE);
	session[NETWORK_SESSION_UNKNOWN48_OFFSET]= FALSE;
	*(long *)(session+NETWORK_SESSION_UNKNOWN40_OFFSET)= NONE;
	memset(session+NETWORK_SESSION_MEMBERSHIP_OFFSET, 0, NETWORK_SESSION_MEMBERSHIP_SIZE);
	*(long *)(session+NETWORK_SESSION_MEMBERSHIP_OFFSET)= NONE;
	memset(session+NETWORK_SESSION_UNKNOWN4978_OFFSET, 0, NETWORK_SESSION_UNKNOWN4978_SIZE);
	*(long *)(session+NETWORK_SESSION_UNKNOWN4978_OFFSET)= NONE;
	memset(session+NETWORK_SESSION_UNKNOWN7668_OFFSET, 0, NETWORK_SESSION_UNKNOWN7668_SIZE);
	*(long *)(session+NETWORK_SESSION_STATE_OFFSET)= _network_session_state_none;
}

/* 0x000614a0: the console that runs a session goes into its seventh state:
what it has from its state on starts with a byte that it is given, a bit for
a console (or, with NONE, for each console but this one), the time and NONE.
With no more than one console there is no seventh state: with the byte, this
console goes out of the session, and without it nothing is done */
void network_session_enter_state7(
	boolean unknown,
	byte *session,
	long peer_index)
{
	long peer_count= *(const long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET);
	long now;
	unsigned long peers;
	byte *data;

	if (peer_count<=1)
	{
		if (unknown)
		{
			network_session_leave(session, TRUE);
		}

		return;
	}
	now= network_session_time();
	if (peer_index==NONE)
	{
		peers= ~(1u<<*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET))&((1u<<peer_count)-1);
	}
	else
	{
		peers= 1u<<peer_index;
	}
	data= network_session_set_state(session, _network_session_state7);
	data[0]= unknown;
	*(unsigned long *)(data+4)= peers;
	*(long *)(data+8)= now;
	*(long *)(data+0xC)= NONE;
}

/* 0x00062ab0: the console that runs a session is lost, or says that the
session is over. In a session that says who is in it, that another console
runs and that has more than one console, the channel of that console is told
(network_observer_channel_unknown_00075e40) and network_session_enter_state9 is done: but
not in the fourth state, nor in the sixth, nor in the last two with their
first byte set. In any other case the session is disconnected */
void network_session_host_lost(
	byte *session)
{
	long state= network_session_state(session);
	boolean disconnect;

	switch (state)
	{
	case _network_session_state2:
	case _network_session_state4:
	case _network_session_state6:
		disconnect= TRUE;
		break;
	case _network_session_state7:
	case _network_session_state8:
		disconnect= session[NETWORK_SESSION_UNKNOWN7420_OFFSET]!=0;
		break;
	default:
		disconnect= FALSE;
		break;
	}
	if (!disconnect && network_session_established(session) && !network_session_unknown_00058d20(session) &&
		*(const long *)(session+NETWORK_SESSION_MEMBERSHIP_OFFSET)!=NONE &&
		*(const long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET)>1)
	{
		const struct network_session_peer_state *host=
			(const struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET)+
			*(const long *)(session+NETWORK_SESSION_UNKNOWN40_OFFSET);

		network_observer_channel_unknown_00075e40(host->unknown4, *(const byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET));
		network_session_enter_state9(session);
	}
	else
	{
		network_session_disconnect(session);
	}
}

/* 0x00062480: in its fourth state, a session sends the console that runs it
something of the kind 0xB, with what the session is known by, in both ways:
the first time, and again each time that unknown_004cf4b0 milliseconds have
gone by since the last. When it sent it is kept after the time of the
state */
void network_session_state4_update(
	byte *session)
{
	long last= *(const long *)(session+NETWORK_SESSION_STATE_DATA_UNKNOWN4_OFFSET);

	if (last==NONE || network_session_elapsed(last)>unknown_004cf4b0)
	{
		byte identifier[NETWORK_SESSION_IDENTIFIER_SIZE];

		memcpy(identifier, session+NETWORK_SESSION_IDENTIFIER_OFFSET, sizeof(identifier));
		network_session_send_to_peer(*(const long *)(session+NETWORK_SESSION_UNKNOWN40_OFFSET), _network_session_send_both,
			session, _network_session_message_unknownB, sizeof(identifier), identifier);
		*(long *)(session+NETWORK_SESSION_STATE_DATA_UNKNOWN4_OFFSET)= network_session_time();
	}
}

/* 0x000623e0: a session sends the address that it keeps in its second
state something of the kind 9, with what the session is known by and two
numbers: each time that unknown_004cf4a0 milliseconds have gone by since the
last */
void network_session_unknown_000623e0(
	byte *session)
{
	if (network_session_elapsed(*(const long *)(session+NETWORK_SESSION_STATE_DATA_UNKNOWN6C_OFFSET))>unknown_004cf4a0)
	{
		byte message[NETWORK_SESSION_MESSAGE_UNKNOWN9_SIZE];

		memset(message, 0, sizeof(message));
		memcpy(message, session+NETWORK_SESSION_IDENTIFIER_OFFSET, NETWORK_SESSION_IDENTIFIER_SIZE);
		*(long *)(message+8)= *(const long *)(session+NETWORK_SESSION_STATE_DATA_UNKNOWN5C_OFFSET);
		*(long *)(message+0xC)= *(const long *)(session+NETWORK_SESSION_STATE_DATA_UNKNOWN60_OFFSET);
		unknown_0007b140(*(const long *)(session+NETWORK_SESSION_UNKNOWN4_OFFSET),
			(const struct transport_address *)(session+NETWORK_SESSION_STATE2_ADDRESS_OFFSET), _network_session_message_unknown9,
			sizeof(message), message);
		*(long *)(session+NETWORK_SESSION_STATE_DATA_UNKNOWN6C_OFFSET)= network_session_time();
	}
}

/* 0x000616e0: a session goes into its ninth state, which is what it does
when the console that ran it is lost: it starts now, network_session_election_new fills a
thing of it and gives a number, and it has a bit for this console and one for
the console that ran it. The 0x34 bytes of 0x761C start over */
void NETWORK_SESSION_CALLBACK network_session_enter_state9(
	byte *session)
{
	byte data[NETWORK_SESSION_STATE9_SIZE];

	memset(data, 0, sizeof(data));
	*(long *)(data+NETWORK_SESSION_STATE9_START_OFFSET)= network_session_time();
	*(long *)(data+NETWORK_SESSION_STATE9_UNKNOWNC_OFFSET)= network_session_election_new(session,
		(struct network_session_election *)(data+NETWORK_SESSION_STATE9_UNKNOWN10_OFFSET));
	*(long *)(data+NETWORK_SESSION_STATE9_UNKNOWN8_OFFSET)= network_session_time();
	*(unsigned long *)(data+NETWORK_SESSION_STATE9_LOCAL_PEERS_OFFSET)=
		1u<<*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET);
	*(unsigned long *)(data+NETWORK_SESSION_STATE9_HOST_PEERS_OFFSET)=
		1u<<*(const long *)(session+NETWORK_SESSION_UNKNOWN40_OFFSET);
	network_session_reset_unknown761c(session);
	memcpy(network_session_set_state(session, _network_session_state9), data, sizeof(data));
}

/* 0x00061180: a session goes from its first state into its second, which is
how this console goes out of a session that it was coming into: what it had
from its state on is kept, in other places, with the time. Nobody is in the
session any more, it knows nothing, nothing is kept of any console, and
network_session_unknown_000623e0 sends for the first time */
void network_session_enter_state2(
	byte *session)
{
	byte data[NETWORK_SESSION_STATE2_SIZE];
	long peer_index;

	memset(data, 0, sizeof(data));
	memcpy(data+NETWORK_SESSION_STATE2_UNKNOWN0_OFFSET, session+NETWORK_SESSION_STATE1_UNKNOWN4_OFFSET,
		NETWORK_SESSION_ADDRESS_SIZE);
	memcpy(data+NETWORK_SESSION_STATE2_ADDRESS, session+NETWORK_SESSION_STATE1_ADDRESS_OFFSET,
		sizeof(struct transport_address));
	memcpy(data+NETWORK_SESSION_STATE2_UNKNOWN38_OFFSET, session+NETWORK_SESSION_STATE1_UNKNOWN1C4_OFFSET,
		NETWORK_SESSION_ADDRESS_SIZE);
	*(long *)(data+NETWORK_SESSION_STATE2_UNKNOWN5C_OFFSET)=
		*(const long *)(session+NETWORK_SESSION_STATE1_UNKNOWN1BC_OFFSET);
	*(long *)(data+NETWORK_SESSION_STATE2_UNKNOWN60_OFFSET)=
		*(const long *)(session+NETWORK_SESSION_STATE1_UNKNOWN1C0_OFFSET);
	*(long *)(data+NETWORK_SESSION_STATE2_UNKNOWN64_OFFSET)=
		*(const long *)(session+NETWORK_SESSION_STATE1_UNKNOWN1E8_OFFSET);
	*(long *)(data+NETWORK_SESSION_STATE2_START_OFFSET)= network_session_time();
	network_session_reset_unknown761c(session);
	memset(session+NETWORK_SESSION_UNKNOWN4978_OFFSET, 0, NETWORK_SESSION_UNKNOWN4978_SIZE);
	*(long *)(session+NETWORK_SESSION_UNKNOWN4978_OFFSET)= NONE;
	memset(session+NETWORK_SESSION_MEMBERSHIP_OFFSET, 0, NETWORK_SESSION_MEMBERSHIP_SIZE);
	*(long *)(session+NETWORK_SESSION_MEMBERSHIP_OFFSET)= NONE;
	*(long *)(session+NETWORK_SESSION_UNKNOWN40_OFFSET)= NONE;
	for (peer_index= 0; peer_index<MAXIMUM_NETWORK_SESSION_PLAYERS; peer_index++)
	{
		if (((const struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET)+peer_index)->valid)
		{
			network_session_peer_state_delete(session, peer_index);
		}
	}
	*(long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET)= NONE;
	memcpy(network_session_set_state(session, _network_session_state2), data, sizeof(data));
	network_session_unknown_000623e0(session);
}

/* 0x00062990: in its third state, a session sends the console that runs it
something of the kind 0x18, in the second way: only if the channel of that
console is in its seventh state and network_session_unknown_00058d90 says no,
when the time of 0x7654 is NONE or more than unknown_004cf4d8 milliseconds
ago, and the last time that it sent (0x7658) is NONE or more than
unknown_004cf4dc milliseconds ago */
void network_session_state3_update(
	byte *session)
{
	const struct network_session_peer_state *host=
		(const struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET)+
		*(const long *)(session+NETWORK_SESSION_UNKNOWN40_OFFSET);
	byte message[NETWORK_SESSION_MESSAGE_UNKNOWN18_SIZE];
	long time;

	if (*(const long *)(*(byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET)+
		host->unknown4*NETWORK_SESSION_UNKNOWN8_CHANNEL_SIZE+NETWORK_SESSION_UNKNOWN8_CHANNEL_STATE_OFFSET)!=
		_network_session_unknown8_channel_state7 || network_session_unknown_00058d90(session))
	{
		return;
	}
	time= *(const long *)(session+NETWORK_SESSION_UNKNOWN7654_OFFSET);
	if (time!=NONE && network_session_elapsed(time)<=unknown_004cf4d8)
	{
		return;
	}
	time= *(const long *)(session+NETWORK_SESSION_UNKNOWN7658_OFFSET);
	if (time!=NONE && network_time_since(time)<=unknown_004cf4dc)
	{
		return;
	}
	memset(message, 0, sizeof(message));
	memcpy(message, session+NETWORK_SESSION_IDENTIFIER_OFFSET, NETWORK_SESSION_IDENTIFIER_SIZE);
	*(long *)(message+0x8)= NONE;
	*(long *)(message+0xC)= NONE;
	*(long *)(message+0x10)= NONE;
	*(long *)(message+0x14)= NONE;
	unknown_00075e80(TRUE, host->unknown4, *(byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET),
		*(const long *)(session+NETWORK_SESSION_INDEX_OFFSET), _network_session_message_unknown18, sizeof(message),
		message);
	*(long *)(session+NETWORK_SESSION_UNKNOWN7658_OFFSET)= network_session_time();
}

/* 0x0005df30: the console that runs a session sets the byte of 0x49A8 and
the three numbers and twelve bytes after it, for a console; whether anything
changed. The byte is the one that it is given only for the console that who
is in the session says runs it: for another, it stays. First, if the second
number is not zero and the third is more than the first that comes, the
second, the third and the bytes are emptied. The second number and the bytes
are kept only if the byte is set and was set; and the bytes only with 1 for
the number */
boolean NETWORK_SESSION_CALLBACK network_session_host_set_unknown49a8(
	byte *session,
	boolean unknown49a8,
	long unknown49ac,
	long unknown49b0,
	long peer_index,
	const byte *unknown49b8)
{
	boolean changed= FALSE;
	boolean keep;

	if (peer_index!=*(const long *)(session+NETWORK_SESSION_HOST_PEER_OFFSET))
	{
		unknown49a8= session[NETWORK_SESSION_UNKNOWN49A8_OFFSET];
	}
	if (*(const long *)(session+NETWORK_SESSION_UNKNOWN49B0_OFFSET)!=0 &&
		*(const long *)(session+NETWORK_SESSION_UNKNOWN49B4_OFFSET)>unknown49ac)
	{
		*(long *)(session+NETWORK_SESSION_UNKNOWN49B0_OFFSET)= 0;
		*(long *)(session+NETWORK_SESSION_UNKNOWN49B4_OFFSET)= NONE;
		memset(session+NETWORK_SESSION_UNKNOWN49B8_OFFSET, 0, NETWORK_SESSION_UNKNOWN49B8_SIZE);
		network_session_parameters_changed(session);
		changed= TRUE;
	}
	keep= unknown49b0!=0 && unknown49a8 && session[NETWORK_SESSION_UNKNOWN49A8_OFFSET];
	if (session[NETWORK_SESSION_UNKNOWN49A8_OFFSET]==unknown49a8 &&
		*(const long *)(session+NETWORK_SESSION_UNKNOWN49AC_OFFSET)==unknown49ac &&
		*(const long *)(session+NETWORK_SESSION_UNKNOWN49B0_OFFSET)==unknown49b0)
	{
		return changed;
	}
	session[NETWORK_SESSION_UNKNOWN49A8_OFFSET]= unknown49a8;
	*(long *)(session+NETWORK_SESSION_UNKNOWN49AC_OFFSET)= unknown49ac;
	if (keep)
	{
		*(long *)(session+NETWORK_SESSION_UNKNOWN49B0_OFFSET)= unknown49b0;
		*(long *)(session+NETWORK_SESSION_UNKNOWN49B4_OFFSET)= unknown49ac;
		if (unknown49b0==1)
		{
			memcpy(session+NETWORK_SESSION_UNKNOWN49B8_OFFSET, unknown49b8, NETWORK_SESSION_UNKNOWN49B8_SIZE);
		}
		else
		{
			memset(session+NETWORK_SESSION_UNKNOWN49B8_OFFSET, 0, NETWORK_SESSION_UNKNOWN49B8_SIZE);
		}
	}
	else
	{
		*(long *)(session+NETWORK_SESSION_UNKNOWN49B0_OFFSET)= 0;
		*(long *)(session+NETWORK_SESSION_UNKNOWN49B4_OFFSET)= NONE;
		memset(session+NETWORK_SESSION_UNKNOWN49B8_OFFSET, 0, NETWORK_SESSION_UNKNOWN49B8_SIZE);
	}
	network_session_parameters_changed(session);

	return TRUE;
}

/* 0x00062b70: what a session has in its ninth state starts: with what the
console that ran it and this one are known by, the first number of who is in
it, which console this one is and a number of it, and one console that is
known of, this one, with its bit in the two numbers. Which console this one
is */
long network_session_election_new(
	const byte *session,
	struct network_session_election *election)
{
	const byte *peers= session+NETWORK_SESSION_PEER_ARRAY_OFFSET;
	long local_peer= *(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET);
	const byte *local= peers+local_peer*NETWORK_SESSION_PEER_SIZE;

	memset(election, 0, sizeof(*election));
	memcpy(election->host_address, peers+*(const long *)(session+NETWORK_SESSION_UNKNOWN40_OFFSET)*NETWORK_SESSION_PEER_SIZE,
		sizeof(election->host_address));
	memcpy(election->local_address, local, sizeof(election->local_address));
	election->membership_unknown0= *(const long *)(session+NETWORK_SESSION_MEMBERSHIP_OFFSET);
	election->local_peer= local_peer;
	election->unknown50= *(const long *)(local+NETWORK_SESSION_PEER_UNKNOWN94_OFFSET);
	memcpy(election->unknownA[0], local+NETWORK_SESSION_PEER_ADDRESS_UNKNOWNA_OFFSET, sizeof(election->unknownA[0]));
	election->count= 1;
	election->unknownB8= 1;
	election->unknownBC= 1;

	return local_peer;
}

/* 0x00063ba0: which of the consoles that are known of in the ninth state is
the one that is known by something: by its six bytes, and the last one that
has them; NONE if none */
long network_session_election_find(
	const struct network_session_election *election,
	const byte *address)
{
	long found= NONE;
	long index;

	for (index= 0; index<election->count; index++)
	{
		if (memcmp(address+NETWORK_SESSION_PEER_ADDRESS_UNKNOWNA_OFFSET, election->unknownA[index],
			sizeof(election->unknownA[index]))==0)
		{
			found= index;
		}
	}

	return found;
}

/* the console that is known by something is one of those that are known of
in the ninth state: a new one, if it was not and there is room for it. Which
it is; NONE if there is no room */
static inline __attribute__((always_inline)) long network_session_election_add(
	const byte *address,
	struct network_session_election *election)
{
	long index= network_session_election_find(election, address);

	if (index==NONE && (unsigned long)election->count<MAXIMUM_NETWORK_SESSION_PLAYERS)
	{
		index= election->count++;
		memcpy(election->unknownA[index], address+NETWORK_SESSION_PEER_ADDRESS_UNKNOWNA_OFFSET,
			sizeof(election->unknownA[index]));
	}

	return index;
}

/* 0x00063c00: a console is known of in the ninth state, and has its bit in
the first of the two numbers; which it is, or NONE */
long network_session_election_add_unknownB8(
	const byte *address,
	struct network_session_election *election)
{
	long index= network_session_election_add(address, election);

	if (index!=NONE)
	{
		election->unknownB8|= 1u<<index;
	}

	return index;
}

/* 0x00063c50: the same, with the bit in the second of the two numbers */
long network_session_election_add_unknownBC(
	const byte *address,
	struct network_session_election *election)
{
	long index= network_session_election_add(address, election);

	if (index!=NONE)
	{
		election->unknownBC|= 1u<<index;
	}

	return index;
}

/* 0x00062c20: of two consoles, each known by something, one is this one:
the other gets its bit in the first of the two numbers of the ninth state,
and is known of from now on if it was not. FALSE if neither is this one, if
there is no other, if it had its bit already or if there is no room for it */
boolean NETWORK_SESSION_CALLBACK network_session_election_add_other(
	const byte *address,
	struct network_session_election *election,
	const byte *other_address)
{
	const byte *other;
	long index;

	if (memcmp(election->local_address, address, sizeof(election->local_address))==0)
	{
		other= other_address;
	}
	else if (memcmp(election->local_address, other_address, sizeof(election->local_address))==0)
	{
		other= address;
	}
	else
	{
		return FALSE;
	}
	if (!other)
	{
		return FALSE;
	}
	index= network_session_election_find(election, other);
	if (index!=NONE && (election->unknownB8&(1u<<index)))
	{
		return FALSE;
	}

	return network_session_election_add_unknownB8(other, election)!=NONE;
}

/* 0x00062ca0: a console gets its bit in the second of the two numbers of
the ninth state, or loses it; whether that changes anything. To get it, it
cannot have it; to lose it, it has to have it */
boolean NETWORK_SESSION_CALLBACK network_session_election_set_unknownBC(
	struct network_session_election *election,
	const byte *address,
	boolean set)
{
	long index= network_session_election_find(election, address);
	boolean has= index!=NONE && (election->unknownBC&(1u<<index));

	if (set)
	{
		if (has)
		{
			return FALSE;
		}
		network_session_election_add_unknownBC(address, election);
	}
	else
	{
		if (!has)
		{
			return FALSE;
		}
		index= network_session_election_find(election, address);
		if (index!=NONE)
		{
			election->unknownBC&= ~(1u<<index);
		}
	}

	return TRUE;
}

/* 0x0005c9e0: a console is taken out of a session: it is sent something of
the kind 0xC in the second way, with what the session is known by, it goes
out (network_session_remove_peer), and what the session has at 0x78A8 is
told, with the two numbers that the console came in with. TRUE. But in the
last three states it is not taken out, and FALSE: in the seventh
network_session_unknown_00061950 is done for it, and in the eighth it gets
its bit in what the session has from its state on */
boolean network_session_remove_peer_with_message_c(
	byte *session,
	long peer_index)
{
	long state= network_session_state(session);
	const byte *peer= session+NETWORK_SESSION_PEER_ARRAY_OFFSET+peer_index*NETWORK_SESSION_PEER_SIZE;
	struct network_session_unknown78A8 *object;
	byte identifier[NETWORK_SESSION_IDENTIFIER_SIZE];
	long unknownF0[2];

	if (state==_network_session_state7)
	{
		network_session_unknown_00061950(peer_index, session);

		return FALSE;
	}
	if (state==_network_session_state6)
	{
		return FALSE;
	}
	if (state==_network_session_state8)
	{
		*(unsigned long *)(session+NETWORK_SESSION_STATE_DATA_UNKNOWN14_OFFSET)|= 1u<<peer_index;

		return FALSE;
	}
	unknownF0[0]= *(const long *)(peer+NETWORK_SESSION_PEER_UNKNOWNF0_OFFSET);
	unknownF0[1]= *(const long *)(peer+NETWORK_SESSION_PEER_UNKNOWNF0_OFFSET+sizeof(long));
	memcpy(identifier, session+NETWORK_SESSION_IDENTIFIER_OFFSET, sizeof(identifier));
	unknown_00075e80(TRUE,
		((const struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET)+peer_index)->unknown4,
		*(byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET), *(const long *)(session+NETWORK_SESSION_INDEX_OFFSET),
		_network_session_message_unknownC, sizeof(identifier), identifier);
	network_session_remove_peer(session, peer_index);
	object= *(struct network_session_unknown78A8 **)(session+NETWORK_SESSION_UNKNOWN78A8_OFFSET);
	if (object)
	{
		object->functions->peer_removed(object, unknownF0);
	}

	return TRUE;
}

/* 0x0005cac0: what has changed of what a session knows of a console is put
in it: each part whose byte says that it has. It is what
network_session_peer_properties_get_update makes, the other way round */
void NETWORK_SESSION_CALLBACK network_session_peer_properties_apply_update(
	struct network_session_peer_properties *properties,
	const struct network_session_peer_properties_update *update)
{
	if (update->has_texts)
	{
		wcsncpy(properties->unknown0, update->unknown0, NETWORK_SESSION_PEER_UNKNOWN28_LENGTH-1);
		properties->unknown0[NETWORK_SESSION_PEER_UNKNOWN28_LENGTH-1]= 0;
		wcsncpy(properties->unknown20, update->unknown20, NETWORK_SESSION_PEER_UNKNOWN48_LENGTH-1);
		properties->unknown20[NETWORK_SESSION_PEER_UNKNOWN48_LENGTH-1]= 0;
	}
	if (update->has_unknown60)
	{
		properties->unknown60= update->unknown60;
		properties->unknown64= update->unknown64;
	}
	if (update->has_unknown68)
	{
		properties->unknown68= update->unknown68;
		properties->unknown6C= update->unknown6C;
		properties->unknown70= update->unknown70;
		memcpy(properties->unknown74, update->unknown74, sizeof(properties->unknown74));
	}
	if (update->has_unknown84)
	{
		memcpy(properties->unknown84, update->unknown84, sizeof(properties->unknown84));
	}
	if (update->has_unknownC4)
	{
		properties->unknownC4= update->unknownC4;
	}
}

/* 0x0005e340: in the seventh state, with the byte of what the session has
from its state on at 0x10, something comes from a console that is neither
this one nor the one at 0x40: if it says the console that the session has
there, by which it is and by what it is known by, the console that sent it
gets its bit at 0x18. Whether it did */
boolean NETWORK_SESSION_CALLBACK network_session_unknown_0005e340(
	byte *session,
	long channel,
	const byte *message)
{
	if (network_session_state(session)==_network_session_state7 && session[NETWORK_SESSION_STATE_DATA_UNKNOWN10_OFFSET])
	{
		long peer_index= network_session_channel_peer(session, channel);

		if (peer_index!=NONE && peer_index!=*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET) &&
			peer_index!=*(const long *)(session+NETWORK_SESSION_UNKNOWN40_OFFSET))
		{
			long said= *(const short *)(message+NETWORK_SESSION_MESSAGE_PEER_INDEX_OFFSET);

			if (said>=0 && said<*(const long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET) &&
				said==*(const long *)(session+NETWORK_SESSION_STATE_DATA_UNKNOWNC_OFFSET) &&
				memcmp(message+NETWORK_SESSION_MESSAGE_PEER_ADDRESS_OFFSET,
					session+NETWORK_SESSION_PEER_ARRAY_OFFSET+said*NETWORK_SESSION_PEER_SIZE,
					NETWORK_SESSION_ADDRESS_SIZE)==0)
			{
				*(unsigned long *)(session+NETWORK_SESSION_STATE_DATA_UNKNOWN18_OFFSET)|= 1u<<peer_index;

				return TRUE;
			}
		}
	}

	return FALSE;
}

/* 0x0005ece0: in the ninth state, something comes from an address of the
game that is of another console of the session: the console loses its bit in
the first of the two numbers that the session has after what it has for that
state, has it in the second, and loses it in the second of those of the
consoles that are known of (network_session_election_set_unknownBC); if that
changes anything, the time is now. TRUE; FALSE out of the ninth state or if
the address is of no other console */
boolean network_session_unknown_0005ece0(
	byte *session,
	const struct transport_address *address)
{
	long peer_index;
	unsigned long bit;
	byte *data= session+NETWORK_SESSION_STATE_DATA_OFFSET;

	if (network_session_state(session)!=_network_session_state9)
	{
		return FALSE;
	}
	peer_index= network_session_find_peer_by_transport_address(session, address);
	if (peer_index==NONE || peer_index==*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET))
	{
		return FALSE;
	}
	bit= 1u<<peer_index;
	*(unsigned long *)(data+NETWORK_SESSION_STATE9_LOCAL_PEERS_OFFSET)&= ~bit;
	*(unsigned long *)(data+NETWORK_SESSION_STATE9_HOST_PEERS_OFFSET)|= bit;
	if (network_session_election_set_unknownBC((struct network_session_election *)(data+NETWORK_SESSION_STATE9_UNKNOWN10_OFFSET),
		session+NETWORK_SESSION_PEER_ARRAY_OFFSET+peer_index*NETWORK_SESSION_PEER_SIZE, FALSE))
	{
		*(long *)(data+NETWORK_SESSION_STATE9_UNKNOWN8_OFFSET)= network_session_time();
	}

	return TRUE;
}

/* 0x0005ee70: the console that leads a session tells the one that runs it
that another console leads it from now on: who is in the session says so,
and the two numbers that count what changes count one more if it is another.
Only if what comes is from the console that leads the session, which is not
this one, and the console that it says is in the session. Whether it was
so */
boolean NETWORK_SESSION_CALLBACK network_session_host_change_leader(
	byte *session,
	long channel,
	const byte *message)
{
	long peer_index;
	long said;

	if (!network_session_is_host(session))
	{
		return FALSE;
	}
	peer_index= network_session_channel_peer(session, channel);
	said= network_session_find_peer(session, message+NETWORK_SESSION_MESSAGE_PEER_ADDRESS_OFFSET);
	if (peer_index!=*(const long *)(session+NETWORK_SESSION_HOST_PEER_OFFSET) ||
		peer_index==*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET) || said==NONE)
	{
		return FALSE;
	}
	if (*(const long *)(session+NETWORK_SESSION_HOST_PEER_OFFSET)!=said)
	{
		*(long *)(session+NETWORK_SESSION_HOST_PEER_OFFSET)= said;
		(*(long *)(session+NETWORK_SESSION_MEMBERSHIP_OFFSET))++;
		(*(long *)(session+NETWORK_SESSION_UNKNOWN7618_OFFSET))++;
	}

	return TRUE;
}

/* 0x0005ef20: the console that leads a session tells the one that runs it
to take a console out: another one goes out with something of the kind 0xE,
and if it is this one the session goes into its tenth state with a 1. Only
if what comes is from the console that leads the session, which is not this
one, and the console that it says is in the session and is not itself.
Whether it was so */
boolean NETWORK_SESSION_CALLBACK network_session_host_leader_removes_peer(
	byte *session,
	long channel,
	const byte *message)
{
	long peer_index;
	long said;

	if (!network_session_is_host(session))
	{
		return FALSE;
	}
	peer_index= network_session_channel_peer(session, channel);
	said= network_session_find_peer(session, message+NETWORK_SESSION_MESSAGE_PEER_ADDRESS_OFFSET);
	if (peer_index!=*(const long *)(session+NETWORK_SESSION_HOST_PEER_OFFSET) ||
		peer_index==*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET) || said==NONE || said==peer_index)
	{
		return FALSE;
	}
	if (said==*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET))
	{
		network_session_enter_state10(session, 1);
	}
	else
	{
		network_session_remove_peer_with_message_e(said, session);
	}

	return TRUE;
}

/* 0x0005f3c0: something of the console of a session that has a channel.
FALSE if no console has it. For the console that runs the session, TRUE,
unless the console has no players and the last thing is not given. For
another, TRUE only if network_session_unknown_00058d90 says no */
boolean NETWORK_SESSION_CALLBACK network_session_unknown_0005f3c0(
	const byte *session,
	long channel_index,
	boolean unknown)
{
	long peer_index= network_session_find_peer_by_channel(session, channel_index);
	boolean host= network_session_unknown_00058d20(session);

	if (peer_index==NONE)
	{
		return FALSE;
	}
	if (host && !unknown && *(const long *)(session+NETWORK_SESSION_PEER_ARRAY_OFFSET+
		peer_index*NETWORK_SESSION_PEER_SIZE+NETWORK_SESSION_PEER_PLAYER_COUNT_OFFSET)==0)
	{
		return FALSE;
	}

	return !network_session_unknown_00058d90(session) || host;
}

/* 0x0005ed90: another console of a session tells the console that runs it
what the session is to know of it apart from what it is known by and its
players: the byte of the console at 0x24 is set, what comes is kept if it is
not what the session knew, and the two numbers that count what changes count
one more if anything did. Whether what comes is from another console of a
session that this one runs */
boolean NETWORK_SESSION_CALLBACK network_session_host_set_peer_properties(
	byte *session,
	long channel,
	const byte *message)
{
	long peer_index;
	byte *peer;
	boolean changed= FALSE;

	if (!network_session_is_host(session))
	{
		return FALSE;
	}
	peer_index= network_session_channel_peer(session, channel);
	if (peer_index==NONE || peer_index==*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET))
	{
		return FALSE;
	}
	peer= session+NETWORK_SESSION_PEER_ARRAY_OFFSET+peer_index*NETWORK_SESSION_PEER_SIZE;
	if (!peer[NETWORK_SESSION_PEER_UNKNOWN24_OFFSET])
	{
		peer[NETWORK_SESSION_PEER_UNKNOWN24_OFFSET]= TRUE;
		changed= TRUE;
	}
	if (memcmp(peer+NETWORK_SESSION_PEER_UNKNOWN28_OFFSET, message+NETWORK_SESSION_MESSAGE_PEER_PROPERTIES_OFFSET,
		sizeof(struct network_session_peer_properties))!=0)
	{
		memcpy(peer+NETWORK_SESSION_PEER_UNKNOWN28_OFFSET, message+NETWORK_SESSION_MESSAGE_PEER_PROPERTIES_OFFSET,
			sizeof(struct network_session_peer_properties));
		changed= TRUE;
	}
	if (changed)
	{
		(*(long *)(session+NETWORK_SESSION_MEMBERSHIP_OFFSET))++;
		(*(long *)(session+NETWORK_SESSION_UNKNOWN7618_OFFSET))++;
	}

	return TRUE;
}

/* 0x0005efd0: another console of a session tells the console that runs it
that a player comes in at one of its four places (network_session_add_peer_player).
If the player comes in, the number and the first thing of 0x90 bytes of the
player, and the number at its end, are what comes. If not, the console is
sent something of the kind 0x1E, if the second byte of what the session
keeps of it is set: what the session is known by, the place and what the
player is known by. Whether what comes is from another console of a session
that this one runs */
boolean NETWORK_SESSION_CALLBACK network_session_host_add_peer_player(
	byte *session,
	long channel,
	const byte *message)
{
	long peer_index;
	long player_index;

	if (!network_session_is_host(session))
	{
		return FALSE;
	}
	peer_index= network_session_channel_peer(session, channel);
	if (peer_index==NONE || peer_index==*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET))
	{
		return FALSE;
	}
	player_index= network_session_add_peer_player(session, peer_index,
		*(const long *)(message+NETWORK_SESSION_MESSAGE_PEER_PLAYER_OFFSET), message+NETWORK_SESSION_MESSAGE_IDENTIFIER_OFFSET);
	if (player_index!=NONE)
	{
		byte *player= session+NETWORK_SESSION_MEMBERSHIP_OFFSET+NETWORK_SESSION_MEMBERSHIP_PLAYER_ARRAY_OFFSET+
			player_index*NETWORK_SESSION_PLAYER_SIZE;

		*(long *)(player+NETWORK_SESSION_PLAYER_UNKNOWN14_OFFSET)=
			*(const long *)(message+NETWORK_SESSION_MESSAGE_ADD_PLAYER_UNKNOWN14_OFFSET);
		memcpy(player+NETWORK_SESSION_PLAYER_UNKNOWN18_OFFSET, message+NETWORK_SESSION_MESSAGE_ADD_PLAYER_UNKNOWN18_OFFSET,
			NETWORK_SESSION_PLAYER_UNKNOWN18_SIZE);
		*(long *)(player+NETWORK_SESSION_PLAYER_UNKNOWN138_OFFSET)=
			*(const long *)(message+NETWORK_SESSION_MESSAGE_ADD_PLAYER_UNKNOWN138_OFFSET);
	}
	else
	{
		const struct network_session_peer_state *state=
			(const struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET)+peer_index;
		byte answer[NETWORK_SESSION_MESSAGE_UNKNOWN1E_SIZE];

		memset(answer, 0, sizeof(answer));
		memcpy(answer, session+NETWORK_SESSION_IDENTIFIER_OFFSET, NETWORK_SESSION_IDENTIFIER_SIZE);
		*(long *)(answer+NETWORK_SESSION_MESSAGE_PEER_PLAYER_OFFSET)=
			*(const long *)(message+NETWORK_SESSION_MESSAGE_PEER_PLAYER_OFFSET);
		memcpy(answer+NETWORK_SESSION_MESSAGE_IDENTIFIER_OFFSET, message+NETWORK_SESSION_MESSAGE_IDENTIFIER_OFFSET,
			NETWORK_SESSION_PLAYER_IDENTIFIER_SIZE);
		if (state->unknown1)
		{
			unknown_00075e80(FALSE, state->unknown4, *(byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET),
				*(const long *)(session+NETWORK_SESSION_INDEX_OFFSET), _network_session_message_unknown1E,
				sizeof(answer), answer);
		}
	}

	return TRUE;
}

/* 0x0005f230: another console of a session tells the console that runs it
what the player at one of its four places is like now: the number and the
first thing of 0x90 bytes of the player, and the number at its end, are
what comes, and the two numbers that count what changes count one more if
any of them was another. Whether what comes is from another console of a
session that this one runs and the console has a player there */
boolean NETWORK_SESSION_CALLBACK network_session_host_set_peer_player(
	byte *session,
	long channel,
	const byte *message)
{
	long peer_index;
	long peer_player_index;
	long player_index;
	byte *player;
	boolean changed= FALSE;

	if (!network_session_is_host(session))
	{
		return FALSE;
	}
	peer_index= network_session_channel_peer(session, channel);
	if (peer_index==NONE || peer_index==*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET))
	{
		return FALSE;
	}
	peer_player_index= *(const long *)(message+NETWORK_SESSION_MESSAGE_PEER_PLAYER_OFFSET);
	if (peer_player_index<0 || peer_player_index>=MAXIMUM_NETWORK_SESSION_PEER_PLAYERS)
	{
		return FALSE;
	}
	player_index= *(const long *)(session+NETWORK_SESSION_PEER_ARRAY_OFFSET+peer_index*NETWORK_SESSION_PEER_SIZE+
		NETWORK_SESSION_PEER_PLAYERS_OFFSET+peer_player_index*sizeof(long));
	if (player_index==NONE)
	{
		return FALSE;
	}
	player= session+NETWORK_SESSION_MEMBERSHIP_OFFSET+NETWORK_SESSION_MEMBERSHIP_PLAYER_ARRAY_OFFSET+
		player_index*NETWORK_SESSION_PLAYER_SIZE;
	if (*(const long *)(player+NETWORK_SESSION_PLAYER_UNKNOWN14_OFFSET)!=
		*(const long *)(message+NETWORK_SESSION_MESSAGE_SET_PLAYER_UNKNOWN14_OFFSET))
	{
		*(long *)(player+NETWORK_SESSION_PLAYER_UNKNOWN14_OFFSET)=
			*(const long *)(message+NETWORK_SESSION_MESSAGE_SET_PLAYER_UNKNOWN14_OFFSET);
		changed= TRUE;
	}
	if (memcmp(player+NETWORK_SESSION_PLAYER_UNKNOWN18_OFFSET, message+NETWORK_SESSION_MESSAGE_SET_PLAYER_UNKNOWN18_OFFSET,
		NETWORK_SESSION_PLAYER_UNKNOWN18_SIZE)!=0)
	{
		memcpy(player+NETWORK_SESSION_PLAYER_UNKNOWN18_OFFSET, message+NETWORK_SESSION_MESSAGE_SET_PLAYER_UNKNOWN18_OFFSET,
			NETWORK_SESSION_PLAYER_UNKNOWN18_SIZE);
		changed= TRUE;
	}
	if (*(const long *)(player+NETWORK_SESSION_PLAYER_UNKNOWN138_OFFSET)!=
		*(const long *)(message+NETWORK_SESSION_MESSAGE_SET_PLAYER_UNKNOWN138_OFFSET))
	{
		*(long *)(player+NETWORK_SESSION_PLAYER_UNKNOWN138_OFFSET)=
			*(const long *)(message+NETWORK_SESSION_MESSAGE_SET_PLAYER_UNKNOWN138_OFFSET);
		changed= TRUE;
	}
	if (changed)
	{
		(*(long *)(session+NETWORK_SESSION_MEMBERSHIP_OFFSET))++;
		(*(long *)(session+NETWORK_SESSION_UNKNOWN7618_OFFSET))++;
	}

	return TRUE;
}

/* 0x0005e030: something of the kind 0x18 comes from an address of the game;
its four numbers look like times. In a session that this console runs, if it
asks, is from another console of the session and network_session_unknown_00058d90
says no, the same is sent back to the address as an answer. In another that
is in its first state or says who is in it, if it answers and is from the
console that runs the session: half of what the third number is over the
first plus what the fourth is over the second is kept at 0x78B0, the byte of
0x78AC is set, and the time of 0x7654 is now. Whether any of that was done */
boolean NETWORK_SESSION_CALLBACK network_session_synchronize_time(
	byte *session,
	const struct transport_address *address,
	const byte *message)
{
	if (!network_session_has_host(session))
	{
		return FALSE;
	}
	if (network_session_unknown_00058d20(session))
	{
		long peer_index= network_session_find_peer_by_transport_address(session, address);

		if (peer_index!=NONE && peer_index!=*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET) &&
			*(const short *)(message+NETWORK_SESSION_MESSAGE_UNKNOWN18_ANSWER_OFFSET)==0 &&
			!network_session_unknown_00058d90(session))
		{
			byte answer[NETWORK_SESSION_MESSAGE_UNKNOWN18_SIZE];

			memcpy(answer, message, sizeof(answer));
			*(short *)(answer+NETWORK_SESSION_MESSAGE_UNKNOWN18_ANSWER_OFFSET)= 1;
			unknown_0007b140(*(const long *)(session+NETWORK_SESSION_UNKNOWN4_OFFSET), address,
				_network_session_message_unknown18, sizeof(answer), answer);

			return TRUE;
		}
	}
	else if (network_session_address_is_host(session, address) &&
		*(const short *)(message+NETWORK_SESSION_MESSAGE_UNKNOWN18_ANSWER_OFFSET)==1)
	{
		unsigned long first= *(const unsigned long *)(message+NETWORK_SESSION_MESSAGE_UNKNOWN18_UNKNOWN10_OFFSET)-
			*(const unsigned long *)(message+NETWORK_SESSION_MESSAGE_UNKNOWN18_UNKNOWN8_OFFSET);
		unsigned long second= *(const unsigned long *)(message+NETWORK_SESSION_MESSAGE_UNKNOWN18_UNKNOWN14_OFFSET)-
			*(const unsigned long *)(message+NETWORK_SESSION_MESSAGE_UNKNOWN18_UNKNOWNC_OFFSET);

		*(long *)(session+NETWORK_SESSION_UNKNOWN78B0_OFFSET)= (long)(first+second)/2;
		session[NETWORK_SESSION_UNKNOWN78AC_OFFSET]= TRUE;
		*(long *)(session+NETWORK_SESSION_UNKNOWN7654_OFFSET)= network_session_time();

		return TRUE;
	}

	return FALSE;
}

/* 0x0005e210: a console is told of a console of a session, by which it is
and by what it is known by. A session that does not say who is in it is left
(network_session_leave with nothing), and FALSE. FALSE too if the console
is not one of the session, is not known by that, or is the one at 0x40. If
not, TRUE: and unless it is this console and network_session_unknown_00058d90 says yes,
the console is kept at 0x44 and the console at 0x40 is sent something of the
kind 0x10 with the same, if the second byte of what the session keeps of it
is set */
boolean NETWORK_SESSION_CALLBACK network_session_unknown_0005e210(
	byte *session,
	const byte *message)
{
	long said;

	if (!network_session_established(session))
	{
		network_session_leave(session, 0);

		return FALSE;
	}
	said= *(const short *)(message+NETWORK_SESSION_MESSAGE_PEER_INDEX_OFFSET);
	if (said<0 || said>=*(const long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET) ||
		memcmp(message+NETWORK_SESSION_MESSAGE_PEER_ADDRESS_OFFSET,
			session+NETWORK_SESSION_PEER_ARRAY_OFFSET+said*NETWORK_SESSION_PEER_SIZE, NETWORK_SESSION_ADDRESS_SIZE)!=0 ||
		said==*(const long *)(session+NETWORK_SESSION_UNKNOWN40_OFFSET))
	{
		return FALSE;
	}
	if (said!=*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET) || !network_session_unknown_00058d90(session))
	{
		const struct network_session_peer_state *host;
		byte passed[NETWORK_SESSION_MESSAGE_UNKNOWN10_SIZE];

		memset(passed, 0, sizeof(passed));
		memcpy(passed, session+NETWORK_SESSION_IDENTIFIER_OFFSET, NETWORK_SESSION_IDENTIFIER_SIZE);
		*(long *)(session+NETWORK_SESSION_UNKNOWN44_OFFSET)= said;
		*(short *)(passed+NETWORK_SESSION_MESSAGE_PEER_INDEX_OFFSET)=
			*(const short *)(message+NETWORK_SESSION_MESSAGE_PEER_INDEX_OFFSET);
		memcpy(passed+NETWORK_SESSION_MESSAGE_PEER_ADDRESS_OFFSET, message+NETWORK_SESSION_MESSAGE_PEER_ADDRESS_OFFSET,
			NETWORK_SESSION_ADDRESS_SIZE);
		host= (const struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET)+
			*(const long *)(session+NETWORK_SESSION_UNKNOWN40_OFFSET);
		if (host->unknown1)
		{
			unknown_00075e80(FALSE, host->unknown4, *(byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET),
				*(const long *)(session+NETWORK_SESSION_INDEX_OFFSET), _network_session_message_unknown10,
				sizeof(passed), passed);
		}
	}

	return TRUE;
}

/* 0x0005e6b0: something comes by one of the game's channels. In a session
in one of its last four states: if it is from another console of the
session, the fourth byte of what the session keeps of it is cleared, and
TRUE; if not, FALSE. In any other state, something of the kind 0x13 is sent
back by the channel, and TRUE: what the session is known by, a byte that is
1, whether the channel is of a console of the session, and, in a session that
says who is in it, a byte that is 1 and what the console at 0x40 is known
by */
boolean network_session_unknown_0005e6b0(
	byte *session,
	long channel)
{
	long channel_index= network_observer_find_channel(*(const byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET),
		*(const long *)(session+NETWORK_SESSION_INDEX_OFFSET), channel);
	long peer_index= network_session_find_peer_by_channel(session, channel_index);
	byte answer[NETWORK_SESSION_MESSAGE_UNKNOWN13_SIZE];

	if (network_session_unknown_00058d20(session))
	{
		if (peer_index==NONE || peer_index==*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET))
		{
			return FALSE;
		}
		((struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET)+peer_index)->unknown3= FALSE;

		return TRUE;
	}
	memset(answer, 0, sizeof(answer));
	memcpy(answer, session+NETWORK_SESSION_IDENTIFIER_OFFSET, NETWORK_SESSION_IDENTIFIER_SIZE);
	answer[NETWORK_SESSION_MESSAGE_UNKNOWN13_UNKNOWN8_OFFSET]= TRUE;
	answer[NETWORK_SESSION_MESSAGE_UNKNOWN13_IS_PEER_OFFSET]= peer_index!=NONE;
	if (network_session_established(session))
	{
		answer[NETWORK_SESSION_MESSAGE_UNKNOWN13_HAS_HOST_OFFSET]= TRUE;
		memcpy(answer+NETWORK_SESSION_MESSAGE_UNKNOWN13_HOST_OFFSET, session+NETWORK_SESSION_PEER_ARRAY_OFFSET+
			*(const long *)(session+NETWORK_SESSION_UNKNOWN40_OFFSET)*NETWORK_SESSION_PEER_SIZE, NETWORK_SESSION_ADDRESS_SIZE);
	}
	unknown_00075e80(FALSE, channel_index, *(byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET),
		*(const long *)(session+NETWORK_SESSION_INDEX_OFFSET), _network_session_message_unknown13, sizeof(answer),
		answer);

	return TRUE;
}

/* who is in a session, or what it knows of them, has changed: the two
numbers that count it count one more. The game has this in each function
that changes it */
static inline __attribute__((always_inline)) void network_session_membership_changed(
	byte *session)
{
	(*(long *)(session+NETWORK_SESSION_MEMBERSHIP_OFFSET))++;
	(*(long *)(session+NETWORK_SESSION_UNKNOWN7618_OFFSET))++;
}

/* something is sent to the console that runs a session, the one at 0x40, if
the session keeps the second byte of it: the game has this in each function
that tells that console something */
static inline __attribute__((always_inline)) void network_session_send_to_host(
	byte *session,
	long type,
	long size,
	const void *data)
{
	const struct network_session_peer_state *state=
		(const struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET)+
		*(const long *)(session+NETWORK_SESSION_UNKNOWN40_OFFSET);

	if (state->unknown1)
	{
		unknown_00075e80(FALSE, state->unknown4, *(byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET),
			*(const long *)(session+NETWORK_SESSION_INDEX_OFFSET), type, size, data);
	}
}

/* 0x0005a790: this console says what a session is to know of it apart from
what it is known by and its players. If it runs the session, that is kept,
the byte of the console at 0x24 is set and the two numbers that count what
changes count one more; if not, the console that runs it is sent it
(network_session_host_set_peer_properties takes it there). FALSE in a session that does
not say who is in it */
boolean network_session_set_local_peer_properties(
	const struct network_session_peer_properties *properties,
	byte *session)
{
	if (!network_session_established(session))
	{
		return FALSE;
	}
	if (network_session_unknown_00058d20(session))
	{
		byte *peer= session+NETWORK_SESSION_PEER_ARRAY_OFFSET+
			*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET)*NETWORK_SESSION_PEER_SIZE;

		peer[NETWORK_SESSION_PEER_UNKNOWN24_OFFSET]= TRUE;
		memcpy(peer+NETWORK_SESSION_PEER_UNKNOWN28_OFFSET, properties, sizeof(*properties));
		network_session_membership_changed(session);
	}
	else
	{
		byte message[NETWORK_SESSION_MESSAGE_PEER_PROPERTIES_SIZE];

		memset(message, 0, sizeof(message));
		memcpy(message, session+NETWORK_SESSION_IDENTIFIER_OFFSET, NETWORK_SESSION_IDENTIFIER_SIZE);
		memcpy(message+NETWORK_SESSION_MESSAGE_PEER_PROPERTIES_OFFSET, properties, sizeof(*properties));
		network_session_send_to_host(session, _network_session_message_peer_properties, sizeof(message), message);
	}

	return TRUE;
}

/* 0x0005a880: a player of this console comes into a session at one of its
four places, with a number, its first thing of 0x90 bytes and its last
number. If this console runs the session, the player comes in
(network_session_add_peer_player) and those are kept: FALSE if it cannot.
If not, the console that runs it is sent it
(network_session_host_add_peer_player takes it there). FALSE in a session
that does not say who is in it */
boolean NETWORK_SESSION_CALLBACK network_session_add_local_player(
	byte *session,
	const byte *unknown18,
	const byte *identifier,
	long peer_player_index,
	long unknown14,
	long unknown138)
{
	if (!network_session_established(session))
	{
		return FALSE;
	}
	if (network_session_unknown_00058d20(session))
	{
		long player_index= network_session_add_peer_player(session,
			*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET), peer_player_index, identifier);
		byte *player;

		if (player_index==NONE)
		{
			return FALSE;
		}
		player= session+NETWORK_SESSION_MEMBERSHIP_OFFSET+NETWORK_SESSION_MEMBERSHIP_PLAYER_ARRAY_OFFSET+
			player_index*NETWORK_SESSION_PLAYER_SIZE;
		*(long *)(player+NETWORK_SESSION_PLAYER_UNKNOWN14_OFFSET)= unknown14;
		memcpy(player+NETWORK_SESSION_PLAYER_UNKNOWN18_OFFSET, unknown18, NETWORK_SESSION_PLAYER_UNKNOWN18_SIZE);
		*(long *)(player+NETWORK_SESSION_PLAYER_UNKNOWN138_OFFSET)= unknown138;
	}
	else
	{
		byte message[NETWORK_SESSION_MESSAGE_ADD_PLAYER_SIZE];

		memset(message, 0, sizeof(message));
		memcpy(message, session+NETWORK_SESSION_IDENTIFIER_OFFSET, NETWORK_SESSION_IDENTIFIER_SIZE);
		*(long *)(message+NETWORK_SESSION_MESSAGE_PEER_PLAYER_OFFSET)= peer_player_index;
		memcpy(message+NETWORK_SESSION_MESSAGE_IDENTIFIER_OFFSET, identifier, NETWORK_SESSION_PLAYER_IDENTIFIER_SIZE);
		*(long *)(message+NETWORK_SESSION_MESSAGE_ADD_PLAYER_UNKNOWN14_OFFSET)= unknown14;
		memcpy(message+NETWORK_SESSION_MESSAGE_ADD_PLAYER_UNKNOWN18_OFFSET, unknown18,
			NETWORK_SESSION_PLAYER_UNKNOWN18_SIZE);
		*(long *)(message+NETWORK_SESSION_MESSAGE_ADD_PLAYER_UNKNOWN138_OFFSET)= unknown138;
		network_session_send_to_host(session, _network_session_message_add_player, sizeof(message), message);
	}

	return TRUE;
}

/* 0x0005a9c0: the player that this console has at one of its four places is
like this now: a number, its first thing of 0x90 bytes and its last number.
If this console runs the session, those are kept and the two numbers that
count what changes count one more: FALSE if it has no player there. If not,
the console that runs it is sent it (network_session_host_set_peer_player
takes it there). FALSE in a session that does not say who is in it */
boolean NETWORK_SESSION_CALLBACK network_session_set_local_player(
	byte *session,
	const byte *unknown18,
	long unknown138,
	long peer_player_index,
	long unknown14)
{
	if (!network_session_established(session))
	{
		return FALSE;
	}
	if (network_session_unknown_00058d20(session))
	{
		long player_index= *(const long *)(session+NETWORK_SESSION_PEER_ARRAY_OFFSET+
			*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET)*NETWORK_SESSION_PEER_SIZE+
			NETWORK_SESSION_PEER_PLAYERS_OFFSET+peer_player_index*sizeof(long));
		byte *player;

		if (player_index==NONE)
		{
			return FALSE;
		}
		player= session+NETWORK_SESSION_MEMBERSHIP_OFFSET+NETWORK_SESSION_MEMBERSHIP_PLAYER_ARRAY_OFFSET+
			player_index*NETWORK_SESSION_PLAYER_SIZE;
		*(long *)(player+NETWORK_SESSION_PLAYER_UNKNOWN14_OFFSET)= unknown14;
		memcpy(player+NETWORK_SESSION_PLAYER_UNKNOWN18_OFFSET, unknown18, NETWORK_SESSION_PLAYER_UNKNOWN18_SIZE);
		*(long *)(player+NETWORK_SESSION_PLAYER_UNKNOWN138_OFFSET)= unknown138;
		network_session_membership_changed(session);
	}
	else
	{
		byte message[NETWORK_SESSION_MESSAGE_SET_PLAYER_SIZE];

		memset(message, 0, sizeof(message));
		memcpy(message, session+NETWORK_SESSION_IDENTIFIER_OFFSET, NETWORK_SESSION_IDENTIFIER_SIZE);
		*(long *)(message+NETWORK_SESSION_MESSAGE_PEER_PLAYER_OFFSET)= peer_player_index;
		*(long *)(message+NETWORK_SESSION_MESSAGE_SET_PLAYER_UNKNOWN14_OFFSET)= unknown14;
		memcpy(message+NETWORK_SESSION_MESSAGE_SET_PLAYER_UNKNOWN18_OFFSET, unknown18,
			NETWORK_SESSION_PLAYER_UNKNOWN18_SIZE);
		*(long *)(message+NETWORK_SESSION_MESSAGE_SET_PLAYER_UNKNOWN138_OFFSET)= unknown138;
		network_session_send_to_host(session, _network_session_message_set_player, sizeof(message), message);
	}

	return TRUE;
}

/* 0x0005aae0: the player that this console has at one of its four places
goes out of a session. If this console runs the session, the player goes out
and the two numbers that count what changes count one more: FALSE if it has
no player there. If not, the console that runs it is sent it
(network_session_host_remove_peer_player takes it there). FALSE in a session
that does not say who is in it */
boolean network_session_remove_local_player(
	byte *session,
	long peer_player_index)
{
	if (!network_session_established(session))
	{
		return FALSE;
	}
	if (network_session_unknown_00058d20(session))
	{
		long player_index= *(const long *)(session+NETWORK_SESSION_PEER_ARRAY_OFFSET+
			*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET)*NETWORK_SESSION_PEER_SIZE+
			NETWORK_SESSION_PEER_PLAYERS_OFFSET+peer_player_index*sizeof(long));

		if (player_index==NONE)
		{
			return FALSE;
		}
		network_session_remove_player(session, player_index);
		network_session_membership_changed(session);
	}
	else
	{
		byte message[NETWORK_SESSION_MESSAGE_REMOVE_PLAYER_SIZE];

		memset(message, 0, sizeof(message));
		memcpy(message, session+NETWORK_SESSION_IDENTIFIER_OFFSET, NETWORK_SESSION_IDENTIFIER_SIZE);
		*(long *)(message+NETWORK_SESSION_MESSAGE_PEER_PLAYER_OFFSET)= peer_player_index;
		network_session_send_to_host(session, _network_session_message_remove_player, sizeof(message), message);
	}

	return TRUE;
}

/* 0x0005aba0: the console that leads a session, which is this one, says
that the console that is known by something leads it from now on. If this
console runs the session too, who is in it says so and the two numbers that
count what changes count one more; if not, the console that runs it is sent
it (network_session_host_change_leader takes it there). FALSE if this
console does not lead the session, or the other is not in it or is this
one */
boolean NETWORK_SESSION_CALLBACK network_session_change_leader(
	byte *session,
	const byte *peer_address)
{
	long leader;
	long peer_index;

	if (!network_session_established(session) || !network_session_local_peer_is_host(session))
	{
		return FALSE;
	}
	leader= *(const long *)(session+NETWORK_SESSION_HOST_PEER_OFFSET);
	peer_index= network_session_find_peer(session, peer_address);
	if (peer_index==NONE || peer_index==leader)
	{
		return FALSE;
	}
	if (network_session_unknown_00058d20(session))
	{
		*(long *)(session+NETWORK_SESSION_HOST_PEER_OFFSET)= peer_index;
		network_session_membership_changed(session);
	}
	else
	{
		byte message[NETWORK_SESSION_MESSAGE_CHANGE_LEADER_SIZE];

		memset(message, 0, sizeof(message));
		memcpy(message, session+NETWORK_SESSION_IDENTIFIER_OFFSET, NETWORK_SESSION_IDENTIFIER_SIZE);
		memcpy(message+NETWORK_SESSION_MESSAGE_PEER_ADDRESS_OFFSET, peer_address, NETWORK_SESSION_ADDRESS_SIZE);
		network_session_send_to_host(session, _network_session_message_change_leader, sizeof(message), message);
	}

	return TRUE;
}

/* 0x0005acc0: the console that leads a session, which is this one, takes
out of it the console that is known by something. If this console runs the
session too, the console goes out with something of the kind 0xE; if not,
the console that runs it is sent it (network_session_host_leader_removes_peer
takes it there). FALSE if this console does not lead the session, or the
other is not in it or is this one */
boolean NETWORK_SESSION_CALLBACK network_session_leader_remove_peer(
	byte *session,
	const byte *peer_address)
{
	long local;
	long peer_index;

	if (!network_session_established(session) || !network_session_local_peer_is_host(session))
	{
		return FALSE;
	}
	local= *(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET);
	peer_index= network_session_find_peer(session, peer_address);
	if (peer_index==NONE || peer_index==local)
	{
		return FALSE;
	}
	if (network_session_unknown_00058d20(session))
	{
		network_session_remove_peer_with_message_e(peer_index, session);
	}
	else
	{
		byte message[NETWORK_SESSION_MESSAGE_CHANGE_LEADER_SIZE];

		memset(message, 0, sizeof(message));
		memcpy(message, session+NETWORK_SESSION_IDENTIFIER_OFFSET, NETWORK_SESSION_IDENTIFIER_SIZE);
		memcpy(message+NETWORK_SESSION_MESSAGE_PEER_ADDRESS_OFFSET, peer_address, NETWORK_SESSION_ADDRESS_SIZE);
		network_session_send_to_host(session, _network_session_message_leader_removes_peer, sizeof(message), message);
	}

	return TRUE;
}

/* 0x000769a0, of network_observer.c: something of one of the channels of
what a session was given when it was set up, that has to do with how long
ago things were */
boolean __attribute__((stdcall)) unknown_000769a0(long channel_index, const byte *unknown8);

/* 0x00078580, of network_observer.c: a number that says how something of
one of those channels is */
long unknown_00078580(const byte *unknown8, long channel_index);

/* 0x004cf494: how long a session in its first state waits before it sends
again what it sends there, in milliseconds. 0x004cf498, 0x004cf49c: how long
it waits in that state before it is left: since it went into it, while
unknown_00078580 does not say 2, and since it first did, after */
long unknown_004cf494;
long unknown_004cf498;
long unknown_004cf49c;

/* 0x0005e3f0: something comes from an address of the game that is of
another console of a session, which that console is taken to run from now
on: in the ninth state; or, in a session that says who is in it, if it is
not the one at 0x40 already and either it is the one at 0x44, or this
console does not run the session and unknown_000769a0 says no of the channel
of the one at 0x40. Then: if this console ran the session, the others are
sent something of the kind 0x13 with what that console is known by, and the
players that ask to come in are forgotten; the console is the one at 0x40,
there is none at 0x44, and the session goes into its third state unless it
is in the fourth. After that, in any case: FALSE if the address is not of
the console that runs the session; if it is, that console is sent something
of the kind 0x14 and TRUE, and the session is left if it was in the seventh
state with its byte set when this console ran it */
boolean NETWORK_SESSION_CALLBACK network_session_unknown_0005e3f0(
	byte *session,
	const struct transport_address *address)
{
	long peer_index= network_session_find_peer_by_transport_address(session, address);
	boolean leave= FALSE;
	byte identifier[NETWORK_SESSION_IDENTIFIER_SIZE];

	if (peer_index!=NONE && peer_index!=*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET))
	{
		boolean take= FALSE;

		if (network_session_state(session)==_network_session_state9)
		{
			take= TRUE;
		}
		else if (network_session_established(session))
		{
			long host= *(const long *)(session+NETWORK_SESSION_UNKNOWN40_OFFSET);

			if (peer_index!=host)
			{
				take= peer_index==*(const long *)(session+NETWORK_SESSION_UNKNOWN44_OFFSET) ||
					(!network_session_unknown_00058d20(session) && !unknown_000769a0(
						((const struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET)+host)->unknown4,
						*(const byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET)));
			}
		}
		if (take)
		{
			boolean was_host= FALSE;

			if (network_session_unknown_00058d20(session))
			{
				byte message[NETWORK_SESSION_MESSAGE_UNKNOWN13_SIZE];

				memset(message, 0, sizeof(message));
				memcpy(message, session+NETWORK_SESSION_IDENTIFIER_OFFSET, NETWORK_SESSION_IDENTIFIER_SIZE);
				message[NETWORK_SESSION_MESSAGE_UNKNOWN13_UNKNOWN8_OFFSET]= TRUE;
				message[NETWORK_SESSION_MESSAGE_UNKNOWN13_IS_PEER_OFFSET]= TRUE;
				message[NETWORK_SESSION_MESSAGE_UNKNOWN13_HAS_HOST_OFFSET]= TRUE;
				memcpy(message+NETWORK_SESSION_MESSAGE_UNKNOWN13_HOST_OFFSET,
					session+NETWORK_SESSION_PEER_ARRAY_OFFSET+peer_index*NETWORK_SESSION_PEER_SIZE,
					NETWORK_SESSION_ADDRESS_SIZE);
				network_session_send_to_peers(_network_session_send_unknown0, session, _network_session_message_unknown13,
					sizeof(message), message);
				was_host= TRUE;
				if (network_session_state(session)==_network_session_state7 && session[NETWORK_SESSION_STATE_DATA_OFFSET])
				{
					leave= TRUE;
				}
			}
			*(long *)(session+NETWORK_SESSION_UNKNOWN40_OFFSET)= peer_index;
			*(long *)(session+NETWORK_SESSION_UNKNOWN44_OFFSET)= NONE;
			if (was_host)
			{
				network_session_reset_requests(session, 0);
			}
			if (network_session_state(session)!=_network_session_state4)
			{
				network_session_enter_state3(session);
			}
		}
	}
	if (!network_session_address_is_host(session, address))
	{
		return FALSE;
	}
	memcpy(identifier, session+NETWORK_SESSION_IDENTIFIER_OFFSET, sizeof(identifier));
	network_session_send_to_host(session, _network_session_message_unknown14, sizeof(identifier), identifier);
	if (leave)
	{
		network_session_leave(session, 0);
	}

	return TRUE;
}

/* 0x00062300: in its first state, a session sends the address that it keeps
something of the kind 8, if unknown_00078580 says 2 of the number that it has there and
it last did more than unknown_004cf494 milliseconds ago: a 2, what the
session is known by and the 0x1AC bytes that it has at 0x3C from its state
on. It counts one more and the time is now */
void network_session_state1_send(
	byte *session)
{
	byte message[NETWORK_SESSION_MESSAGE_UNKNOWN8_SIZE];

	if (unknown_00078580(*(const byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET),
			*(const long *)(session+NETWORK_SESSION_STATE1_UNKNOWN0_OFFSET))!=_network_session_state1_unknown0_ready ||
		network_session_time()-*(const long *)(session+NETWORK_SESSION_STATE1_UNKNOWN1F4_OFFSET)<=unknown_004cf494)
	{
		return;
	}
	memset(message, 0, sizeof(message));
	*(short *)message= NETWORK_SESSION_MESSAGE_UNKNOWN8_UNKNOWN0;
	memcpy(message+NETWORK_SESSION_MESSAGE_UNKNOWN8_IDENTIFIER_OFFSET, session+NETWORK_SESSION_IDENTIFIER_OFFSET,
		NETWORK_SESSION_IDENTIFIER_SIZE);
	memcpy(message+NETWORK_SESSION_MESSAGE_UNKNOWN8_UNKNOWNC_OFFSET, session+NETWORK_SESSION_STATE1_UNKNOWN3C_OFFSET,
		NETWORK_SESSION_STATE1_UNKNOWN3C_SIZE);
	unknown_0007b140(*(const long *)(session+NETWORK_SESSION_UNKNOWN4_OFFSET),
		(const struct transport_address *)(session+NETWORK_SESSION_STATE1_ADDRESS_OFFSET),
		_network_session_message_unknown8, sizeof(message), message);
	(*(long *)(session+NETWORK_SESSION_STATE1_UNKNOWN1F0_OFFSET))++;
	*(long *)(session+NETWORK_SESSION_STATE1_UNKNOWN1F4_OFFSET)= network_session_time();
}

/* 0x000617c0: what a session does again and again in its first state.
While unknown_00078580 does not say 2 of the number that it has there, it is left
(network_session_leave with nothing) if it went into the state more than
unknown_004cf498 milliseconds ago. Once it says 2: the time of 0x1EC is
now, if it had none; if the session has who is in it and what it knows, it
goes into its third state and the console that leads it is sent something of
the kind 0x15; if not, it is left if that time was more than
unknown_004cf49c milliseconds ago. A session that is not left and is still
in the first state does network_session_state1_send */
void network_session_state1_update(
	byte *session)
{
	long since= 0;
	long limit= 0;
	boolean entered= FALSE;

	if (unknown_00078580(*(const byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET),
		*(const long *)(session+NETWORK_SESSION_STATE1_UNKNOWN0_OFFSET))!=_network_session_state1_unknown0_ready)
	{
		since= *(const long *)(session+NETWORK_SESSION_STATE1_UNKNOWN1E8_OFFSET);
		limit= unknown_004cf498;
	}
	else
	{
		if (*(const long *)(session+NETWORK_SESSION_STATE1_UNKNOWN1EC_OFFSET)==0)
		{
			*(long *)(session+NETWORK_SESSION_STATE1_UNKNOWN1EC_OFFSET)= network_session_time();
		}
		if (*(const long *)(session+NETWORK_SESSION_MEMBERSHIP_OFFSET)!=NONE &&
			*(const long *)(session+NETWORK_SESSION_UNKNOWN4978_OFFSET)!=NONE)
		{
			const struct network_session_peer_state *leader;
			byte identifier[NETWORK_SESSION_IDENTIFIER_SIZE];

			network_session_enter_state3(session);
			memcpy(identifier, session+NETWORK_SESSION_IDENTIFIER_OFFSET, sizeof(identifier));
			leader= (const struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET)+
				*(const long *)(session+NETWORK_SESSION_HOST_PEER_OFFSET);
			if (leader->unknown1)
			{
				unknown_00075e80(FALSE, leader->unknown4, *(byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET),
					*(const long *)(session+NETWORK_SESSION_INDEX_OFFSET), _network_session_message_unknown15,
					sizeof(identifier), identifier);
			}
			entered= TRUE;
		}
		else
		{
			since= *(const long *)(session+NETWORK_SESSION_STATE1_UNKNOWN1EC_OFFSET);
			limit= unknown_004cf49c;
		}
	}
	if (!entered && network_session_time()-since>limit)
	{
		network_session_leave(session, 0);

		return;
	}
	if (network_session_state(session)==_network_session_state1)
	{
		network_session_state1_send(session);
	}
}

/* 0x00076a40, of network_observer.c: something is done with one of the
channels of what a session was given when it was set up */
void unknown_00076a40(long channel_index, byte *unknown8);

/* 0x004cf4c4: how long a session in its eighth state waits for the consoles
before it sends something of the kind 0x12 to the one that ran it too, in
milliseconds */
long unknown_004cf4c4;

/* the first thing of the channel that a session has for a console */
static inline __attribute__((always_inline)) long network_session_channel_state(
	const byte *session,
	long channel_index)
{
	return *(const long *)(*(const byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET)+
		channel_index*NETWORK_SESSION_UNKNOWN8_CHANNEL_SIZE+NETWORK_SESSION_UNKNOWN8_CHANNEL_STATE_OFFSET);
}

/* 0x00062240: in a session that is in any state, unknown_00076a40 is done with the
channel of each console that the session keeps the second byte of, if the
first thing of the channel is not 1. In a session that this console does not
run, only if network_session_unknown_00058d90 says no and the session has who
is in it; and, if it is in its first state or says who is in it, only for
the consoles that have their byte at 0x24 set */
void network_session_unknown_00062240(
	byte *session)
{
	long peer_index;

	if (network_session_state(session)==_network_session_state_none)
	{
		return;
	}
	for (peer_index= 0; peer_index<*(const long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET); peer_index++)
	{
		const struct network_session_peer_state *state=
			(const struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET)+peer_index;

		if (!state->unknown1)
		{
			continue;
		}
		if (!network_session_unknown_00058d20(session))
		{
			if (network_session_unknown_00058d90(session) ||
				*(const long *)(session+NETWORK_SESSION_MEMBERSHIP_OFFSET)==NONE)
			{
				continue;
			}
			if (network_session_has_host(session) && !session[NETWORK_SESSION_PEER_ARRAY_OFFSET+
				peer_index*NETWORK_SESSION_PEER_SIZE+NETWORK_SESSION_PEER_UNKNOWN24_OFFSET])
			{
				continue;
			}
		}
		if (network_session_channel_state(session, state->unknown4)!=_network_session_unknown8_channel_state1)
		{
			unknown_00076a40(state->unknown4, *(byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET));
		}
	}
}

/* 0x00062550: in its eighth state, a session sends something
of the kind 0x12, with what the session is known by, to each console that it
keeps the second byte of, whose channel has 7 as its first thing and that
has not been sent it yet (the bits at 0xC say which have). The console at
0x4 is sent it only when the bit of that console, those at 0x10 and those
at 0x14 are those of all the consoles, or the state started
unknown_004cf4c4 milliseconds ago or more */
void network_session_send_unknown12(
	byte *session)
{
	byte *data= session+NETWORK_SESSION_STATE_DATA_OFFSET;
	long peer_count= *(const long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET);
	boolean all= TRUE;
	long peer_index;

	if (((1u<<*(const long *)(data+NETWORK_SESSION_STATE8_HOST_OFFSET))|
		*(const unsigned long *)(data+NETWORK_SESSION_STATE8_UNKNOWN14_OFFSET)|
		*(const unsigned long *)(data+NETWORK_SESSION_STATE8_UNKNOWN10_OFFSET))!=(1u<<peer_count)-1)
	{
		all= network_session_time()-*(const long *)(data+NETWORK_SESSION_STATE8_START_OFFSET)>=unknown_004cf4c4;
	}
	for (peer_index= 0; peer_index<*(const long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET); peer_index++)
	{
		const struct network_session_peer_state *state=
			(const struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET)+peer_index;
		unsigned long bit= 1u<<peer_index;

		if ((peer_index!=*(const long *)(data+NETWORK_SESSION_STATE8_HOST_OFFSET) || all) &&
			!(*(const unsigned long *)(data+NETWORK_SESSION_STATE8_SENT_OFFSET)&bit) && state->unknown1 &&
			network_session_channel_state(session, state->unknown4)==_network_session_unknown8_channel_state7)
		{
			byte identifier[NETWORK_SESSION_IDENTIFIER_SIZE];

			memcpy(identifier, session+NETWORK_SESSION_IDENTIFIER_OFFSET, sizeof(identifier));
			unknown_00075e80(FALSE, state->unknown4, *(byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET),
				*(const long *)(session+NETWORK_SESSION_INDEX_OFFSET), _network_session_message_unknown12,
				sizeof(identifier), identifier);
			*(unsigned long *)(data+NETWORK_SESSION_STATE8_SENT_OFFSET)|= bit;
		}
	}
}

/* 0x00061570: a session goes into its eighth state: what it has from its
state on starts with a byte that it is given (the second), the console that
ran the session, the time, and the bit of this console at 0xC and at 0x10.
From a state that is not one of the last four, network_session_reset_requests
is done first and the console at 0x40 is this one. Then unknown_00076a40 is done with the
channel of each console that the session keeps the second byte of and whose
first thing is 1, and network_session_send_unknown12. With no more than one
console there is no eighth state: the session goes into the fifth */
void NETWORK_SESSION_CALLBACK network_session_enter_state8(
	byte *session,
	boolean unknown)
{
	long host;
	long now;
	unsigned long local_bit;
	long peer_index;
	byte *data;

	if (*(const long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET)<=1)
	{
		network_session_enter_state5(session);

		return;
	}
	host= *(const long *)(session+NETWORK_SESSION_UNKNOWN40_OFFSET);
	if (!network_session_unknown_00058d20(session))
	{
		network_session_reset_requests(session, TRUE);
		*(long *)(session+NETWORK_SESSION_UNKNOWN40_OFFSET)= *(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET);
	}
	now= network_session_time();
	local_bit= 1u<<*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET);
	data= network_session_set_state(session, _network_session_state8);
	data[NETWORK_SESSION_STATE8_UNKNOWN1_OFFSET]= unknown;
	*(long *)(data+NETWORK_SESSION_STATE8_HOST_OFFSET)= host;
	*(long *)(data+NETWORK_SESSION_STATE8_START_OFFSET)= now;
	*(unsigned long *)(data+NETWORK_SESSION_STATE8_SENT_OFFSET)= local_bit;
	*(unsigned long *)(data+NETWORK_SESSION_STATE8_UNKNOWN10_OFFSET)= local_bit;
	for (peer_index= 0; peer_index<*(const long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET); peer_index++)
	{
		const struct network_session_peer_state *state=
			(const struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET)+peer_index;

		if (state->unknown1 &&
			network_session_channel_state(session, state->unknown4)==_network_session_unknown8_channel_state1)
		{
			unknown_00076a40(state->unknown4, *(byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET));
		}
	}
	network_session_send_unknown12(session);
}

/* 0x00060400: makes what is sent to a console to tell it who is in a
session: all of it, or with a baseline what has changed since.
network_session_parameters_get_update does the same with what the session knows */
void __attribute__((stdcall)) unknown_00060400(byte *session, const byte *membership, const byte *baseline,
	byte *update);

/* 0x000768b0, 0x00076930, of network_observer.c: whether one of the channels
of what a session was given when it was set up still has something of a kind
to send, and something that is done with it then */
boolean __attribute__((stdcall)) unknown_000768b0(long channel_index, byte *unknown8, long type);
void __attribute__((stdcall)) unknown_00076930(long channel_index, byte *unknown8, long type);

/* 0x00062640: the console that runs a session tells the others who is in it.
Each console that the session keeps the second byte of and not the fourth,
whose channel has 7 as its first thing and that does not have who is in the
session as it is now (the number at 0x8 of what the session keeps of it is
not the one at the start of who is in it) is told: unless its channel still
has something of the kind 0x19 to send (unknown_00076930 then), or the session is in
its eighth state. A console that has who was in it the last time is sent
what has changed since; any other, all of it. Each is sent something of the
kind 0x19 and has the number of now. And who is in the session is kept as
who was in it the last time */
void network_session_send_membership_updates(
	byte *session)
{
	byte changes[NETWORK_SESSION_MESSAGE_MEMBERSHIP_UPDATE_SIZE];
	byte all[NETWORK_SESSION_MESSAGE_MEMBERSHIP_UPDATE_SIZE];
	unsigned long changes_peers= 0;
	unsigned long all_peers= 0;
	long peer_index;

	for (peer_index= 0; peer_index<*(const long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET); peer_index++)
	{
		const struct network_session_peer_state *state=
			(const struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET)+peer_index;
		byte *unknown8= *(byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET);

		if (!state->unknown1 || state->unknown3 ||
			network_session_channel_state(session, state->unknown4)!=_network_session_unknown8_channel_state7 ||
			state->unknown8==*(const long *)(session+NETWORK_SESSION_MEMBERSHIP_OFFSET))
		{
			continue;
		}
		if (unknown_000768b0(state->unknown4, unknown8, _network_session_message_membership_update))
		{
			unknown_00076930(state->unknown4, unknown8, _network_session_message_membership_update);
		}
		else if (network_session_state(session)!=_network_session_state8)
		{
			if (state->unknown8!=NONE && state->unknown8==*(const long *)(session+NETWORK_SESSION_OTHER_MEMBERSHIP_OFFSET))
			{
				changes_peers|= 1u<<peer_index;
			}
			else
			{
				all_peers|= 1u<<peer_index;
			}
		}
	}
	if (all_peers)
	{
		unknown_00060400(session, session+NETWORK_SESSION_MEMBERSHIP_OFFSET, NULL, all);
	}
	if (changes_peers)
	{
		unknown_00060400(session, session+NETWORK_SESSION_MEMBERSHIP_OFFSET,
			session+NETWORK_SESSION_OTHER_MEMBERSHIP_OFFSET, changes);
	}
	for (peer_index= 0; peer_index<*(const long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET); peer_index++)
	{
		struct network_session_peer_state *state=
			(struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET)+peer_index;
		unsigned long bit= 1u<<peer_index;
		const byte *update;

		if (changes_peers&bit)
		{
			update= changes;
		}
		else if (all_peers&bit)
		{
			update= all;
		}
		else
		{
			continue;
		}
		state->unknown8= *(const long *)(session+NETWORK_SESSION_MEMBERSHIP_OFFSET);
		if (state->unknown1)
		{
			unknown_00075e80(FALSE, state->unknown4, *(byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET),
				*(const long *)(session+NETWORK_SESSION_INDEX_OFFSET), _network_session_message_membership_update,
				NETWORK_SESSION_MESSAGE_MEMBERSHIP_UPDATE_SIZE, update);
		}
	}
	memcpy(session+NETWORK_SESSION_OTHER_MEMBERSHIP_OFFSET, session+NETWORK_SESSION_MEMBERSHIP_OFFSET,
		NETWORK_SESSION_MEMBERSHIP_SIZE);
}

/* 0x000627e0: the console that runs a session tells the others what the
session knows, as network_session_send_membership_updates tells them who is
in it: with the number at 0xC of what the session keeps of each console, the
kind 0x21, and what the session knew the last time at 0x5E28 */
void network_session_send_parameters_updates(
	byte *session)
{
	byte changes[NETWORK_SESSION_MESSAGE_PARAMETERS_UPDATE_SIZE];
	byte all[NETWORK_SESSION_MESSAGE_PARAMETERS_UPDATE_SIZE];
	unsigned long changes_peers= 0;
	unsigned long all_peers= 0;
	long peer_index;

	for (peer_index= 0; peer_index<*(const long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET); peer_index++)
	{
		const struct network_session_peer_state *state=
			(const struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET)+peer_index;
		byte *unknown8= *(byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET);

		if (!state->unknown1 || state->unknown3 ||
			network_session_channel_state(session, state->unknown4)!=_network_session_unknown8_channel_state7 ||
			state->unknownC==*(const long *)(session+NETWORK_SESSION_UNKNOWN4978_OFFSET))
		{
			continue;
		}
		if (unknown_000768b0(state->unknown4, unknown8, _network_session_message_parameters_update))
		{
			unknown_00076930(state->unknown4, unknown8, _network_session_message_parameters_update);
		}
		else if (network_session_state(session)!=_network_session_state8)
		{
			if (state->unknownC!=NONE && state->unknownC==*(const long *)(session+NETWORK_SESSION_UNKNOWN5E28_OFFSET))
			{
				changes_peers|= 1u<<peer_index;
			}
			else
			{
				all_peers|= 1u<<peer_index;
			}
		}
	}
	if (all_peers)
	{
		network_session_parameters_get_update(session, session+NETWORK_SESSION_UNKNOWN4978_OFFSET, NULL, all);
	}
	if (changes_peers)
	{
		network_session_parameters_get_update(session, session+NETWORK_SESSION_UNKNOWN4978_OFFSET,
			session+NETWORK_SESSION_UNKNOWN5E28_OFFSET, changes);
	}
	for (peer_index= 0; peer_index<*(const long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET); peer_index++)
	{
		struct network_session_peer_state *state=
			(struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET)+peer_index;
		unsigned long bit= 1u<<peer_index;
		const byte *update;

		if (changes_peers&bit)
		{
			update= changes;
		}
		else if (all_peers&bit)
		{
			update= all;
		}
		else
		{
			continue;
		}
		state->unknownC= *(const long *)(session+NETWORK_SESSION_UNKNOWN4978_OFFSET);
		if (state->unknown1)
		{
			unknown_00075e80(FALSE, state->unknown4, *(byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET),
				*(const long *)(session+NETWORK_SESSION_INDEX_OFFSET), _network_session_message_parameters_update,
				NETWORK_SESSION_MESSAGE_PARAMETERS_UPDATE_SIZE, update);
		}
	}
	memcpy(session+NETWORK_SESSION_UNKNOWN5E28_OFFSET, session+NETWORK_SESSION_UNKNOWN4978_OFFSET,
		NETWORK_SESSION_UNKNOWN4978_SIZE);
}

/* 0x00061ac0, 0x00061ef0: what a session does again and again in its
seventh and ninth states */
void __attribute__((stdcall)) network_session_state7_update(byte *session);
void __attribute__((stdcall)) network_session_state9_update(byte *session);

/* 0x004cf4c0: how long a session in its eighth state waits for the consoles
that have not answered, in milliseconds */
long unknown_004cf4c0;

/* 0x00061e00: what a session does again and again in its eighth state.
network_session_send_unknown12; and then nothing more until each console has
its bit at 0x10 or at 0x14, or the state started unknown_004cf4c0
milliseconds ago or more. Then the session goes into its fifth state; each
console but this one that had no bit at 0x10, or had it at 0x14, is taken
out with something of the kind 0xD; the others are told who is in the
session and what it knows; and, with the first byte that the state had, the
session is left (network_session_leave with nothing), or, with the second,
it goes into its seventh state for each console */
void network_session_state8_update(
	byte *session)
{
	const byte *data= session+NETWORK_SESSION_STATE_DATA_OFFSET;
	unsigned long everyone;
	unsigned long gone;
	boolean leave;
	boolean unknown1;
	long peer_index;

	network_session_send_unknown12(session);
	everyone= (1u<<*(const long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET))-1;
	if ((*(const unsigned long *)(data+NETWORK_SESSION_STATE8_UNKNOWN10_OFFSET)|
			*(const unsigned long *)(data+NETWORK_SESSION_STATE8_UNKNOWN14_OFFSET))!=everyone &&
		network_session_time()-*(const long *)(data+NETWORK_SESSION_STATE8_START_OFFSET)<unknown_004cf4c0)
	{
		return;
	}
	everyone= (1u<<*(const long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET))-1;
	gone= (everyone&~*(const unsigned long *)(data+NETWORK_SESSION_STATE8_UNKNOWN10_OFFSET))|
		*(const unsigned long *)(data+NETWORK_SESSION_STATE8_UNKNOWN14_OFFSET);
	leave= data[0];
	unknown1= data[NETWORK_SESSION_STATE8_UNKNOWN1_OFFSET];
	network_session_enter_state5(session);
	if (gone)
	{
		for (peer_index= *(const long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET)-1; peer_index>=0; peer_index--)
		{
			if (peer_index!=*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET) && (gone&(1u<<peer_index)))
			{
				network_session_remove_peer_with_message_d(peer_index, session);
			}
		}
	}
	network_session_send_membership_updates(session);
	network_session_send_parameters_updates(session);
	if (leave)
	{
		network_session_leave(session, 0);
	}
	else if (unknown1)
	{
		network_session_enter_state7(FALSE, session, NONE);
	}
}

/* 0x0005a090: what a session does again and again, as its state says; a
session that is in no state, or in the tenth, does nothing. First what each
state has of its own. Then, in a session that this console runs: the players
that asked to come in too long ago are forgotten, and, if it is not in one of
its last three states, each console that the session keeps the second byte of
and whose channel has 1 as its first thing is taken out with something of
the kind 0xD. In another that says who is in it: if the channel of the
console that runs it has 1 as its first thing, network_session_host_lost; and
network_session_state3_update, if it still says who is in it. Then
network_session_unknown_00062240; and the console that runs the session tells the
others who is in it and what it knows */
void network_session_update(
	byte *session)
{
	long state= network_session_state(session);
	long peer_index;

	if (state==_network_session_state_none || state==_network_session_state10)
	{
		return;
	}
	switch (state)
	{
	case _network_session_state1:
		network_session_state1_update(session);
		break;
	case _network_session_state2:
		network_session_unknown_000618d0(session);
		break;
	case _network_session_state4:
		network_session_unknown_00061910(session);
		break;
	case _network_session_state6:
		network_session_disconnect(session);
		break;
	case _network_session_state7:
		network_session_state7_update(session);
		break;
	case _network_session_state8:
		network_session_state8_update(session);
		break;
	case _network_session_state9:
		network_session_state9_update(session);
		break;
	}
	if (network_session_unknown_00058d20(session))
	{
		network_session_requests_expire(session);
		state= network_session_state(session);
		if (state!=_network_session_state7 && state!=_network_session_state6 && state!=_network_session_state8)
		{
			for (peer_index= 0; peer_index<*(const long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET); peer_index++)
			{
				const struct network_session_peer_state *peer=
					(const struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET)+peer_index;

				if (peer->unknown1 &&
					network_session_channel_state(session, peer->unknown4)==_network_session_unknown8_channel_state1)
				{
					network_session_remove_peer_with_message_d(peer_index, session);
				}
			}
		}
	}
	else if (network_session_established(session))
	{
		const struct network_session_peer_state *host=
			(const struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET)+
			*(const long *)(session+NETWORK_SESSION_UNKNOWN40_OFFSET);

		if (network_session_channel_state(session, host->unknown4)==_network_session_unknown8_channel_state1)
		{
			network_session_host_lost(session);
		}
		if (network_session_established(session))
		{
			network_session_state3_update(session);
		}
	}
	network_session_unknown_00062240(session);
	if (network_session_unknown_00058d20(session))
	{
		network_session_send_membership_updates(session);
		network_session_send_parameters_updates(session);
	}
}

/* 0x000619b0: whether, of two consoles of those that are in a session, the
second is better than the first to run it */
boolean __attribute__((stdcall)) unknown_000619b0(long best, long candidate, const byte *peers, long peer_count);

/* 0x004cf4b8: how long a session in its seventh state waits with no console
chosen before it gives up, in milliseconds. 0x004cf4b4: how long it waits for
the consoles to answer about the one that is chosen. 0x004cf4bc: how long it
waits for the one that is chosen once it has told it */
long unknown_004cf4b8;
long unknown_004cf4b4;
long unknown_004cf4bc;

/* the console that a session in its seventh state has chosen; NONE with
none */
static inline __attribute__((always_inline)) long network_session_state7_chosen(
	const byte *session)
{
	return *(const long *)(session+NETWORK_SESSION_STATE_DATA_UNKNOWNC_OFFSET);
}

/* 0x00061ac0: what a session does again and again in its seventh state, in
which the console that runs it looks for another to run it.

With no console chosen: of those that have their bit at 0x4 and have who is
in the session and what it knows as they are now (and, without the first
byte of the state, whose number at 0x9C is a bit for each console), the best
is chosen, as unknown_000619b0 says; without that byte this console counts too, and
if it is the best none is chosen. The one that is chosen loses its bit. The
state is over if no console has its bit, or if this console is the best and
all those that have their bit were looked at.

With a console chosen: the others are sent something of the kind 0xF that
says which, once; when all have answered (the bits at 0x18), or after
unknown_004cf4b4 milliseconds if the one that is chosen has, that one is sent
something of the kind 0x11 and is the console at 0x44; if it has not
answered by then, or unknown_004cf4bc milliseconds after it is sent that,
there is none chosen any more (network_session_unknown_00061950). With none
chosen, the state is over too after unknown_004cf4b8 milliseconds.

When the state is over the session goes into its fifth state, and with the
first byte that the state had it is left (network_session_leave with 1) */
void NETWORK_SESSION_CALLBACK network_session_state7_update(
	byte *session)
{
	boolean done= FALSE;

	if (network_session_state7_chosen(session)==NONE)
	{
		if (*(const unsigned long *)(session+NETWORK_SESSION_STATE_DATA_UNKNOWN4_OFFSET)==0)
		{
			done= TRUE;
		}
		else
		{
			long best= session[NETWORK_SESSION_STATE_DATA_OFFSET] ? NONE :
				*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET);
			unsigned long looked_at= 0;
			long peer_index;

			for (peer_index= 0; peer_index<*(const long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET); peer_index++)
			{
				const struct network_session_peer_state *state=
					(const struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET)+peer_index;
				unsigned long bit= 1u<<peer_index;

				if (!(*(const unsigned long *)(session+NETWORK_SESSION_STATE_DATA_UNKNOWN4_OFFSET)&bit) ||
					state->unknown8!=*(const long *)(session+NETWORK_SESSION_MEMBERSHIP_OFFSET) ||
					state->unknownC!=*(const long *)(session+NETWORK_SESSION_UNKNOWN4978_OFFSET))
				{
					continue;
				}
				if (!session[NETWORK_SESSION_STATE_DATA_OFFSET] &&
					*(const unsigned long *)(session+NETWORK_SESSION_PEER_ARRAY_OFFSET+peer_index*NETWORK_SESSION_PEER_SIZE+
						NETWORK_SESSION_PEER_UNKNOWN9C_OFFSET)!=
					(1u<<*(const long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET))-1)
				{
					continue;
				}
				looked_at|= bit;
				if (best==NONE || unknown_000619b0(best, peer_index, session+NETWORK_SESSION_PEER_ARRAY_OFFSET,
					*(const long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET)))
				{
					best= peer_index;
				}
			}
			if (best!=*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET))
			{
				if (best!=NONE)
				{
					unsigned long peers= *(const unsigned long *)(session+NETWORK_SESSION_STATE_DATA_UNKNOWN4_OFFSET);

					*(long *)(session+NETWORK_SESSION_STATE_DATA_UNKNOWNC_OFFSET)= best;
					*(unsigned long *)(session+NETWORK_SESSION_STATE_DATA_UNKNOWN4_OFFSET)= peers&~(1u<<best);
				}
			}
			else if (looked_at==*(const unsigned long *)(session+NETWORK_SESSION_STATE_DATA_UNKNOWN4_OFFSET))
			{
				done= TRUE;
			}
		}
	}
	if (network_session_state7_chosen(session)==NONE)
	{
		if (network_session_time()-*(const long *)(session+NETWORK_SESSION_STATE_DATA_UNKNOWN8_OFFSET)>=unknown_004cf4b8)
		{
			done= TRUE;
		}
	}
	else
	{
		if (!session[NETWORK_SESSION_STATE_DATA_UNKNOWN10_OFFSET])
		{
			long chosen= network_session_state7_chosen(session);
			byte message[NETWORK_SESSION_MESSAGE_UNKNOWN10_SIZE];

			memset(message, 0, sizeof(message));
			memcpy(message, session+NETWORK_SESSION_IDENTIFIER_OFFSET, NETWORK_SESSION_IDENTIFIER_SIZE);
			*(short *)(message+NETWORK_SESSION_MESSAGE_PEER_INDEX_OFFSET)= (short)chosen;
			memcpy(message+NETWORK_SESSION_MESSAGE_PEER_ADDRESS_OFFSET,
				session+NETWORK_SESSION_PEER_ARRAY_OFFSET+chosen*NETWORK_SESSION_PEER_SIZE, NETWORK_SESSION_ADDRESS_SIZE);
			network_session_send_to_peers(_network_session_send_unknown0, session, _network_session_message_unknownF,
				sizeof(message), message);
			session[NETWORK_SESSION_STATE_DATA_UNKNOWN10_OFFSET]= TRUE;
			*(long *)(session+NETWORK_SESSION_STATE_DATA_UNKNOWN14_OFFSET)= network_session_time();
			*(unsigned long *)(session+NETWORK_SESSION_STATE_DATA_UNKNOWN18_OFFSET)=
				1u<<*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET);
		}
		if (!session[NETWORK_SESSION_STATE_DATA_UNKNOWN1C_OFFSET])
		{
			long since= *(const long *)(session+NETWORK_SESSION_STATE_DATA_UNKNOWN14_OFFSET);
			long now= network_session_time();
			unsigned long answered= *(const unsigned long *)(session+NETWORK_SESSION_STATE_DATA_UNKNOWN18_OFFSET);
			boolean tell= answered==(1u<<*(const long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET))-1;

			if (!tell && now-since>=unknown_004cf4b4)
			{
				if (answered&(1u<<network_session_state7_chosen(session)))
				{
					tell= TRUE;
				}
				else
				{
					network_session_unknown_00061950(network_session_state7_chosen(session), session);
				}
			}
			if (tell)
			{
				const struct network_session_peer_state *state=
					(const struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET)+
					network_session_state7_chosen(session);
				byte identifier[NETWORK_SESSION_IDENTIFIER_SIZE];

				memcpy(identifier, session+NETWORK_SESSION_IDENTIFIER_OFFSET, sizeof(identifier));
				if (state->unknown1)
				{
					unknown_00075e80(FALSE, state->unknown4, *(byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET),
						*(const long *)(session+NETWORK_SESSION_INDEX_OFFSET), _network_session_message_unknown11,
						sizeof(identifier), identifier);
				}
				session[NETWORK_SESSION_STATE_DATA_UNKNOWN1C_OFFSET]= TRUE;
				*(long *)(session+NETWORK_SESSION_STATE_DATA_UNKNOWN20_OFFSET)= network_session_time();
				*(long *)(session+NETWORK_SESSION_UNKNOWN44_OFFSET)= network_session_state7_chosen(session);
			}
		}
		if (session[NETWORK_SESSION_STATE_DATA_UNKNOWN1C_OFFSET] &&
			network_session_time()-*(const long *)(session+NETWORK_SESSION_STATE_DATA_UNKNOWN20_OFFSET)>=unknown_004cf4bc)
		{
			network_session_unknown_00061950(network_session_state7_chosen(session), session);
		}
	}
	if (done)
	{
		boolean leave= session[NETWORK_SESSION_STATE_DATA_OFFSET];

		network_session_enter_state5(session);
		if (leave)
		{
			network_session_leave(session, TRUE);
		}
	}
}

/* 0x004cf4cc: how long a session in its ninth state waits for a console
before it takes it as one that does not answer, in milliseconds.
0x004cf4d0: how long it waits after the console that it has chosen changed.
0x004cf4d4: how often it sends what it sends there. 0x004cf4c8: how long
the ninth state lasts at most */
long unknown_004cf4cc;
long unknown_004cf4d0;
long unknown_004cf4d4;
long unknown_004cf4c8;

/* 0x00061ef0: what a session does again and again in its ninth state, in
which the consoles that are left without the one that ran the session agree
on which runs it.

Each console but this one that has had its bit in the first number of the
state (at 0xD0) for unknown_004cf4cc milliseconds, as the time that the
session keeps of it says, loses it and has it in the second (at 0xD4); and
one with a bit in neither has it in the second once the state started that
long ago.

If the console that the session has chosen (at 0xC) is this one: the session
goes into its eighth state, with 1, when each console either has its bit in
the second number or is one of those that are known of and has its bit in
the second number of those; but with any console in the second number of
the state, only once the state started unknown_004cf4cc milliseconds ago
and the time of 0x8 was unknown_004cf4d0 milliseconds ago. If it is another
and has its bit in the second number of the state, what the session has of
those that are known of starts over, the console that is chosen is this one
and the time of 0x8 is now.

Then, if it is still in the ninth state: each console that the session keeps
the second byte of is sent, in the other way, something of the kind 0x16
with what the session is known by and what it has of the consoles that are
known of: the first time, every unknown_004cf4d4 milliseconds, and half a
second after it last did if the time of 0x8 is later than that. And the
session is disconnected once the state started unknown_004cf4c8 milliseconds
ago */
void NETWORK_SESSION_CALLBACK network_session_state9_update(
	byte *session)
{
	byte *data= session+NETWORK_SESSION_STATE_DATA_OFFSET;
	struct network_session_election *election=
		(struct network_session_election *)(data+NETWORK_SESSION_STATE9_UNKNOWN10_OFFSET);
	unsigned long *local_peers= (unsigned long *)(data+NETWORK_SESSION_STATE9_LOCAL_PEERS_OFFSET);
	unsigned long *host_peers= (unsigned long *)(data+NETWORK_SESSION_STATE9_HOST_PEERS_OFFSET);
	long peer_index;
	long chosen;
	long last;
	boolean send;

	for (peer_index= 0; peer_index<*(const long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET); peer_index++)
	{
		unsigned long bit= 1u<<peer_index;

		if (peer_index==*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET))
		{
			continue;
		}
		if (*local_peers&bit)
		{
			if (network_session_time()-*(const long *)(data+NETWORK_SESSION_STATE9_PEER_TIMES_OFFSET+
				peer_index*sizeof(long))>=unknown_004cf4cc)
			{
				*local_peers&= ~bit;
				*host_peers|= bit;
			}
			if (*local_peers&bit)
			{
				continue;
			}
		}
		if (!(*host_peers&bit) &&
			network_session_time()-*(const long *)(data+NETWORK_SESSION_STATE9_START_OFFSET)>=unknown_004cf4cc)
		{
			*host_peers|= bit;
		}
	}
	chosen= *(const long *)(data+NETWORK_SESSION_STATE9_UNKNOWNC_OFFSET);
	if (chosen==*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET))
	{
		long gone= 0;
		long waiting= 0;

		for (peer_index= 0; peer_index<*(const long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET); peer_index++)
		{
			if (*host_peers&(1u<<peer_index))
			{
				gone++;
			}
			else
			{
				long index= network_session_election_find(election,
					session+NETWORK_SESSION_PEER_ARRAY_OFFSET+peer_index*NETWORK_SESSION_PEER_SIZE);

				if (index==NONE || !(election->unknownBC&(1u<<index)))
				{
					waiting++;
				}
			}
		}
		if ((gone<=0 ||
				(network_session_time()-*(const long *)(data+NETWORK_SESSION_STATE9_START_OFFSET)>=unknown_004cf4cc &&
				network_session_time()-*(const long *)(data+NETWORK_SESSION_STATE9_UNKNOWN8_OFFSET)>=unknown_004cf4d0)) &&
			waiting==0)
		{
			network_session_enter_state8(session, TRUE);
		}
	}
	else if (*host_peers&(1u<<chosen))
	{
		*(long *)(data+NETWORK_SESSION_STATE9_UNKNOWNC_OFFSET)= network_session_election_new(session, election);
		*(long *)(data+NETWORK_SESSION_STATE9_UNKNOWN8_OFFSET)= network_session_time();
	}
	if (network_session_state(session)!=_network_session_state9)
	{
		return;
	}
	last= *(const long *)(data+NETWORK_SESSION_STATE9_SENT_OFFSET);
	if (last==0 || network_session_time()-last>=unknown_004cf4d4)
	{
		send= TRUE;
	}
	else
	{
		last= *(const long *)(data+NETWORK_SESSION_STATE9_SENT_OFFSET);
		send= *(const long *)(data+NETWORK_SESSION_STATE9_UNKNOWN8_OFFSET)-last>0 &&
			network_session_time()-last>=NETWORK_SESSION_STATE9_SEND_AGAIN_MILLISECONDS;
	}
	if (send)
	{
		byte message[NETWORK_SESSION_MESSAGE_UNKNOWN16_SIZE];

		memset(message, 0, sizeof(message));
		memcpy(message, session+NETWORK_SESSION_IDENTIFIER_OFFSET, NETWORK_SESSION_IDENTIFIER_SIZE);
		memcpy(message+NETWORK_SESSION_MESSAGE_UNKNOWN16_ELECTION_OFFSET, election, sizeof(*election));
		for (peer_index= 0; peer_index<*(const long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET); peer_index++)
		{
			const struct network_session_peer_state *state=
				(const struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET)+peer_index;

			if (state->unknown1)
			{
				unknown_00075e80(TRUE, state->unknown4, *(byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET),
					*(const long *)(session+NETWORK_SESSION_INDEX_OFFSET), _network_session_message_unknown16,
					sizeof(message), message);
			}
		}
		*(long *)(data+NETWORK_SESSION_STATE9_SENT_OFFSET)= network_session_time();
	}
	if (network_session_time()-*(const long *)(data+NETWORK_SESSION_STATE9_START_OFFSET)>=unknown_004cf4c8)
	{
		network_session_disconnect(session);
	}
}

/* 0x00076aa0, of network_observer.c: the channel that what a session was
given when it was set up has, or gets, for a console that is known by
something; NONE if it has none to give */
long __attribute__((stdcall)) unknown_00076aa0(byte *unknown8, long index, const byte *peer_address);

/* 0x00075800: the session that the table of sessions has for the eight bytes
that one is known by; 0 if none */
long __attribute__((stdcall)) unknown_00075800(byte *unknown, const byte *identifier);

/* where something that a session has from its state on is, in a copy of
it */
static inline __attribute__((always_inline)) byte *network_session_state_data_at(
	byte *data,
	long session_offset)
{
	return data+(session_offset-NETWORK_SESSION_STATE_DATA_OFFSET);
}

/* 0x00061050: a session goes into its first state, that of a console that
asks to come into the session of another: what it has from its state on
starts with the channel that it has for the console that runs the session,
what that console is known by, its address of the game, what this console
sends to ask (with eight bytes made at random at 0x180 of it), and the time.
The session counts one more at 0x7650 and has no players of its own at
0x761C; who is in it is the console that runs it, first, and this one,
second; and network_session_state1_send is done */
void NETWORK_SESSION_CALLBACK network_session_enter_state1(
	const struct transport_address *address,
	byte *session,
	long channel_index,
	const byte *host_address,
	const byte *request)
{
	byte data[NETWORK_SESSION_STATE_DATA_SIZE];

	memset(data, 0, sizeof(data));
	*(long *)network_session_state_data_at(data, NETWORK_SESSION_STATE1_UNKNOWN0_OFFSET)= channel_index;
	memcpy(network_session_state_data_at(data, NETWORK_SESSION_STATE1_UNKNOWN4_OFFSET), host_address,
		NETWORK_SESSION_ADDRESS_SIZE);
	memcpy(network_session_state_data_at(data, NETWORK_SESSION_STATE1_ADDRESS_OFFSET), address, sizeof(*address));
	*(long *)network_session_state_data_at(data, NETWORK_SESSION_STATE1_UNKNOWN1E8_OFFSET)= network_session_time();
	*(long *)network_session_state_data_at(data, NETWORK_SESSION_STATE1_UNKNOWN1EC_OFFSET)= 0;
	memcpy(network_session_state_data_at(data, NETWORK_SESSION_STATE1_UNKNOWN3C_OFFSET), request,
		NETWORK_SESSION_STATE1_UNKNOWN3C_SIZE);
	transport_security_random_identifier(network_session_state_data_at(data, NETWORK_SESSION_STATE1_UNKNOWN1BC_OFFSET));
	(*(long *)(session+NETWORK_SESSION_UNKNOWN7650_OFFSET))++;
	memset(session+NETWORK_SESSION_UNKNOWN761C_OFFSET, 0, NETWORK_SESSION_UNKNOWN761C_SIZE);
	*(long *)(session+NETWORK_SESSION_UNKNOWN40_OFFSET)= 0;
	*(long *)(session+NETWORK_SESSION_UNKNOWN7654_OFFSET)= NONE;
	*(long *)(session+NETWORK_SESSION_UNKNOWN7658_OFFSET)= NONE;
	network_session_add_peer(session, 0, host_address, TRUE, channel_index, NULL);
	network_session_add_peer(session, 1, request+NETWORK_SESSION_JOIN_ADDRESS_OFFSET, FALSE, NONE, NULL);
	*(long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET)= 1;
	memcpy(network_session_set_state(session, _network_session_state1), data, sizeof(data));
	network_session_state1_send(session);
}

/* 0x00059e50: this console asks to come into the session of another console,
which is known by something in the network, with a key and a number, and is
run by a console that is known by something; whether it could start to. A
session that is known by the same already is disconnected first. The console
has to be known by something in the network; the key has to be taken
(transport_security_set_key); the console that runs the session has to have
an address of the game, which is not this console's own; and there has to be
a channel for it. If any of that fails, the session is known by nothing.

If not: who is in the session and what it knows say nothing yet, the session
has no consoles, and what the console sends to ask is made: how many players
and what each is known by, two numbers of each, eight bytes, a byte and a
number that goes with it, 0x2C bytes that may not be given, and what this
console is known by. And the session goes into its first state */
boolean NETWORK_SESSION_CALLBACK network_session_join(
	byte *session,
	long unknown18,
	long unknown3C,
	const byte *identifier,
	const byte *key,
	const byte *host_address,
	long player_count,
	const byte *identifiers,
	const long *unknownC4,
	const long *unknown104,
	boolean has_unknown150,
	long unknown150,
	const byte *unknown144,
	const byte *unknown154)
{
	byte address[NETWORK_SESSION_ADDRESS_SIZE];
	struct transport_address host_transport_address;
	byte request[NETWORK_SESSION_STATE1_UNKNOWN3C_SIZE];
	byte *other;

	other= (byte *)unknown_00075800(*(byte **)(session+NETWORK_SESSION_TABLE_OFFSET), identifier);
	if (other)
	{
		network_session_disconnect(other);
	}
	if (unknown_004d8b18 && unknown_004d8b19)
	{
		transport_security_get_address();
	}
	memcpy(address, unknown_004cf793, sizeof(address));
	if (unknown_004cf792)
	{
		network_session_forget_identifier(session);
		if (transport_security_set_key(*(const long *)(session+NETWORK_SESSION_UNKNOWN38_OFFSET), key, unknown3C, FALSE,
			identifier))
		{
			network_session_set_identifier(session, identifier, key, unknown3C);
			if (transport_security_address_from_peer_address(*(const long *)(session+NETWORK_SESSION_UNKNOWN38_OFFSET),
					&host_transport_address, host_address, unknown3C, NETWORK_SESSION_JOIN_PORT) &&
				!(host_transport_address.address_size==sizeof(host_transport_address.address.ipv4) &&
					host_transport_address.address.ipv4==NETWORK_SESSION_JOIN_LOOPBACK))
			{
				long channel_index= unknown_00076aa0(*(byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET),
					*(const long *)(session+NETWORK_SESSION_INDEX_OFFSET), host_address);

				if (channel_index!=NONE)
				{
					*(long *)(session+NETWORK_SESSION_UNKNOWN18_OFFSET)= unknown18;
					memset(session+NETWORK_SESSION_UNKNOWN4978_OFFSET, 0, NETWORK_SESSION_UNKNOWN4978_SIZE);
					*(long *)(session+NETWORK_SESSION_UNKNOWN4978_OFFSET)= NONE;
					session[NETWORK_SESSION_UNKNOWN48_OFFSET]= 0;
					memset(session+NETWORK_SESSION_MEMBERSHIP_OFFSET, 0, NETWORK_SESSION_MEMBERSHIP_SIZE);
					*(long *)(session+NETWORK_SESSION_MEMBERSHIP_OFFSET)= NONE;
					*(long *)(session+NETWORK_SESSION_UNKNOWN40_OFFSET)= NONE;
					memset(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET, 0, NETWORK_SESSION_UNKNOWN72DC_SIZE);
					*(long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET)= NONE;
					*(long *)(session+NETWORK_SESSION_UNKNOWN78B0_OFFSET)= -network_session_time();
					session[NETWORK_SESSION_UNKNOWN78AC_OFFSET]= TRUE;
					memset(request, 0, sizeof(request));
					*(long *)(request+NETWORK_SESSION_JOIN_PLAYER_COUNT_OFFSET)= player_count;
					memcpy(request+NETWORK_SESSION_JOIN_PLAYERS_OFFSET, identifiers,
						player_count*NETWORK_SESSION_PLAYER_IDENTIFIER_SIZE);
					memcpy(request+NETWORK_SESSION_JOIN_UNKNOWNC4_OFFSET, unknownC4, player_count*sizeof(long));
					memcpy(request+NETWORK_SESSION_JOIN_UNKNOWN104_OFFSET, unknown104, player_count*sizeof(long));
					request[NETWORK_SESSION_JOIN_UNKNOWN14C_OFFSET]= has_unknown150;
					if (has_unknown150)
					{
						*(long *)(request+NETWORK_SESSION_JOIN_UNKNOWN150_OFFSET)= unknown150;
					}
					memcpy(request+NETWORK_SESSION_JOIN_UNKNOWN144_OFFSET, unknown144, NETWORK_SESSION_JOIN_UNKNOWN144_SIZE);
					memcpy(request+NETWORK_SESSION_JOIN_ADDRESS_OFFSET, address, sizeof(address));
					if (unknown154)
					{
						memcpy(request+NETWORK_SESSION_JOIN_UNKNOWN154_OFFSET, unknown154,
							NETWORK_SESSION_JOIN_UNKNOWN154_SIZE);
					}
					network_session_enter_state1(&host_transport_address, session, channel_index, host_address, request);

					return TRUE;
				}
			}
		}
	}
	network_session_forget_identifier(session);

	return FALSE;
}

/* 0x004cf4a8: how long after a console came into a session what it sends
again to ask to come in is taken as the same asking, in milliseconds */
long unknown_004cf4a8;

/* 0x0005c720: the console that runs a session lets a console that asks come
in; whether it did, and why not if not. A console that is in the session
already, by the six bytes of what it is known by: with nothing kept of its
channel, no (5); if it is known by the same, has not said yet what the
session is to know of it and came in less than unknown_004cf4a8 milliseconds
ago, it is the same asking and nothing is done; if not, it goes out first,
and the channel that it gets is told (network_observer_channel_unknown_00075e40).
With no channel for the console, no (7). With the byte that it is given,
each of its players asks to come in, after what it had asked before is
forgotten. Then the console is the last of the session, unknown_00076a40 is done with
its channel, and the two numbers that count what changes count one more */
boolean NETWORK_SESSION_CALLBACK network_session_host_accept_join(
	byte *session,
	const byte *peer_address,
	long player_count,
	const byte *identifiers,
	const long *unknownC4,
	boolean has_unknown150,
	long unknown150,
	const byte *unknown144,
	long *reason)
{
	byte unknownA[NETWORK_SESSION_PEER_ADDRESS_UNKNOWNA_SIZE];
	long peer_index;
	long channel_index;
	long index;

	memcpy(unknownA, peer_address+NETWORK_SESSION_PEER_ADDRESS_UNKNOWNA_OFFSET, sizeof(unknownA));
	peer_index= network_session_find_peer_by_unknownA(session, unknownA);
	if (peer_index!=NONE)
	{
		const struct network_session_peer_state *state=
			(const struct network_session_peer_state *)(session+NETWORK_SESSION_UNKNOWN72DC_OFFSET)+peer_index;
		const byte *peer= session+NETWORK_SESSION_PEER_ARRAY_OFFSET+peer_index*NETWORK_SESSION_PEER_SIZE;

		if (!state->unknown1)
		{
			*reason= _network_session_join_unknown5;

			return FALSE;
		}
		if (memcmp(peer, peer_address, NETWORK_SESSION_ADDRESS_SIZE)==0)
		{
			long since= state->time;
			long now= network_session_time();

			if (!peer[NETWORK_SESSION_PEER_UNKNOWN24_OFFSET] && now-since<unknown_004cf4a8)
			{
				return TRUE;
			}
		}
		network_session_remove_peer(session, peer_index);
	}
	channel_index= unknown_00076aa0(*(byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET),
		*(const long *)(session+NETWORK_SESSION_INDEX_OFFSET), peer_address);
	if (channel_index==NONE)
	{
		*reason= _network_session_join_unknown7;

		return FALSE;
	}
	if (peer_index!=NONE)
	{
		network_observer_channel_unknown_00075e40(channel_index, *(const byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET));
	}
	if (has_unknown150)
	{
		for (index= 0; index<player_count; index++)
		{
			const byte *identifier= identifiers+index*NETWORK_SESSION_PLAYER_IDENTIFIER_SIZE;
			struct network_session_player_request *request;

			if (network_session_find_request(session, identifier, &request))
			{
				request= NULL;
				if (network_session_find_request(session, identifier, &request))
				{
					request->valid= FALSE;
				}
			}
			network_session_request_player(identifier, (const long *)unknown144, session, unknown150, unknownC4[index]);
		}
	}
	network_session_add_peer(session, *(const long *)(session+NETWORK_SESSION_PEER_COUNT_OFFSET), peer_address, TRUE,
		channel_index, (const long *)unknown144);
	unknown_00076a40(channel_index, *(byte **)(session+NETWORK_SESSION_UNKNOWN8_OFFSET));
	network_session_membership_changed(session);

	return TRUE;
}

/* 0x000630f0: the console that runs a session answers a console that asked
to come in from an address of the game. With no reason against it,
network_session_host_accept_join, and TRUE if it lets it in. If not, the
address is sent something of the kind 0xA with what the session is known by
and why not, and FALSE */
boolean NETWORK_SESSION_CALLBACK network_session_host_answer_join(
	const byte *request,
	byte *session,
	const struct transport_address *address,
	long reason)
{
	byte message[NETWORK_SESSION_MESSAGE_JOIN_REFUSED_SIZE];

	if (reason==_network_session_join_ok && network_session_host_accept_join(session,
		request+NETWORK_SESSION_JOIN_ADDRESS_OFFSET, *(const long *)(request+NETWORK_SESSION_JOIN_PLAYER_COUNT_OFFSET),
		request+NETWORK_SESSION_JOIN_PLAYERS_OFFSET, (const long *)(request+NETWORK_SESSION_JOIN_UNKNOWNC4_OFFSET),
		request[NETWORK_SESSION_JOIN_UNKNOWN14C_OFFSET], *(const long *)(request+NETWORK_SESSION_JOIN_UNKNOWN150_OFFSET),
		request+NETWORK_SESSION_JOIN_UNKNOWN144_OFFSET, &reason))
	{
		return TRUE;
	}
	memset(message, 0, sizeof(message));
	memcpy(message, session+NETWORK_SESSION_IDENTIFIER_OFFSET, NETWORK_SESSION_IDENTIFIER_SIZE);
	*(long *)(message+NETWORK_SESSION_MESSAGE_JOIN_REFUSED_REASON_OFFSET)= reason;
	unknown_0007b140(*(const long *)(session+NETWORK_SESSION_UNKNOWN4_OFFSET), address,
		_network_session_message_join_refused, sizeof(message), message);

	return FALSE;
}

/* 0x00063080: what a console sends to ask to come into a session comes to
the console that runs it, from an address of the game.
network_session_host_check_join says whether it can. If it can and the
session has something at 0x78A8, that is told, by its first function, with
the address and what was sent, and it is left to it. If not,
network_session_host_answer_join. TRUE */
boolean NETWORK_SESSION_CALLBACK network_session_host_join_requested(
	const byte *message,
	byte *session,
	const struct transport_address *address)
{
	byte request[NETWORK_SESSION_STATE1_UNKNOWN3C_SIZE];
	struct network_session_unknown78A8 *object;
	long reason;

	memcpy(request, message+NETWORK_SESSION_MESSAGE_UNKNOWN8_UNKNOWNC_OFFSET, sizeof(request));
	reason= network_session_host_check_join(session, request);
	object= *(struct network_session_unknown78A8 **)(session+NETWORK_SESSION_UNKNOWN78A8_OFFSET);
	if (reason==_network_session_join_ok && object)
	{
		object->functions->join_requested(object, address, request);
	}
	else
	{
		network_session_host_answer_join(request, session, address, reason);
	}

	return TRUE;
}

/* 0x0005dac0: what a console sends to ask for a change in what a session
knows comes to the console that runs it. From the console that leads the
session, each thing that it says changes is taken, if it is not what the
session had already: the number of 0x4980 (network_session_host_set_unknown4980),
the language (which it says with a number that is not zero), the numbers of
0x498C, 0x49A4, 0x49C8, 0x49F8 and 0x4DAC, the bytes of 0x49C4 and 0x49A1,
the 16 bits of 0x5DD0, the two numbers and the text of 0x4D08, the thing of
0x4DB0, the text of characters of 16 bits of 0x4EE0, and the two things that
may be there or not: the one of 0x4A00, which has to be right
(network_session_roster_valid), and the one of 0x5DDC, which has to be too
(network_session_unknown_0005c190). From any console of the session, the byte of 0x49FC.
If anything changed, the number at the start of what the session knows
counts one more. FALSE if this console does not run the session, if what
comes is not from another console of it, or if one of those two things was
not right */
boolean NETWORK_SESSION_CALLBACK network_session_host_parameters_requested(
	const byte *request,
	byte *session,
	long channel)
{
	long peer_index;
	boolean changed= FALSE;
	boolean result= TRUE;

	if (!network_session_is_host(session))
	{
		return FALSE;
	}
	peer_index= network_session_channel_peer(session, channel);
	if (peer_index==NONE || peer_index==*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET))
	{
		return FALSE;
	}
	if (peer_index==*(const long *)(session+NETWORK_SESSION_HOST_PEER_OFFSET))
	{
		long value;

		if (request[NETWORK_SESSION_PARAMETERS_REQUEST_HAS_UNKNOWNC_OFFSET])
		{
			value= *(const long *)(request+NETWORK_SESSION_PARAMETERS_REQUEST_UNKNOWNC_OFFSET);
			if (*(const long *)(session+NETWORK_SESSION_UNKNOWN4980_OFFSET)!=value)
			{
				network_session_host_set_unknown4980(session, value);
			}
		}
		value= *(const long *)(request+NETWORK_SESSION_PARAMETERS_REQUEST_LANGUAGE_OFFSET);
		if (value!=0 && *(const long *)(session+NETWORK_SESSION_LANGUAGE_OFFSET)!=value)
		{
			*(long *)(session+NETWORK_SESSION_LANGUAGE_OFFSET)= value;
			changed= TRUE;
		}
		if (request[NETWORK_SESSION_PARAMETERS_REQUEST_HAS_UNKNOWN498C_OFFSET])
		{
			value= *(const long *)(request+NETWORK_SESSION_PARAMETERS_REQUEST_UNKNOWN498C_OFFSET);
			if (*(const long *)(session+NETWORK_SESSION_UNKNOWN498C_OFFSET)!=value)
			{
				*(long *)(session+NETWORK_SESSION_UNKNOWN498C_OFFSET)= value;
				changed= TRUE;
			}
		}
		if (request[NETWORK_SESSION_PARAMETERS_REQUEST_HAS_UNKNOWN49A4_OFFSET])
		{
			value= *(const long *)(request+NETWORK_SESSION_PARAMETERS_REQUEST_UNKNOWN49A4_OFFSET);
			if (*(const long *)(session+NETWORK_SESSION_UNKNOWN49A4_OFFSET)!=value)
			{
				*(long *)(session+NETWORK_SESSION_UNKNOWN49A4_OFFSET)= value;
				changed= TRUE;
			}
		}
		if (request[NETWORK_SESSION_PARAMETERS_REQUEST_HAS_UNKNOWN49C4_OFFSET] &&
			session[NETWORK_SESSION_UNKNOWN49C4_OFFSET]!=request[NETWORK_SESSION_PARAMETERS_REQUEST_UNKNOWN49C4_OFFSET])
		{
			session[NETWORK_SESSION_UNKNOWN49C4_OFFSET]= request[NETWORK_SESSION_PARAMETERS_REQUEST_UNKNOWN49C4_OFFSET];
			changed= TRUE;
		}
		if (request[NETWORK_SESSION_PARAMETERS_REQUEST_HAS_UNKNOWN49C8_OFFSET])
		{
			value= *(const long *)(request+NETWORK_SESSION_PARAMETERS_REQUEST_UNKNOWN49C8_OFFSET);
			if (*(const long *)(session+NETWORK_SESSION_UNKNOWN49C8_OFFSET)!=value)
			{
				*(long *)(session+NETWORK_SESSION_UNKNOWN49C8_OFFSET)= value;
				changed= TRUE;
			}
		}
		if (request[NETWORK_SESSION_PARAMETERS_REQUEST_HAS_UNKNOWN49F8_OFFSET])
		{
			value= *(const long *)(request+NETWORK_SESSION_PARAMETERS_REQUEST_UNKNOWN49F8_OFFSET);
			if (*(const long *)(session+NETWORK_SESSION_UNKNOWN49F8_OFFSET)!=value)
			{
				*(long *)(session+NETWORK_SESSION_UNKNOWN49F8_OFFSET)= value;
				changed= TRUE;
			}
		}
		if (request[NETWORK_SESSION_PARAMETERS_REQUEST_HAS_ROSTER_OFFSET])
		{
			boolean valid= request[NETWORK_SESSION_PARAMETERS_REQUEST_ROSTER_VALID_OFFSET];
			const byte *roster= request+NETWORK_SESSION_PARAMETERS_REQUEST_ROSTER_OFFSET;

			if (valid!=session[NETWORK_SESSION_UNKNOWN49FD_OFFSET] ||
				memcmp(session+NETWORK_SESSION_UNKNOWN4A00_OFFSET, roster, NETWORK_SESSION_UNKNOWN4A00_SIZE)!=0)
			{
				if (!valid)
				{
					session[NETWORK_SESSION_UNKNOWN49FD_OFFSET]= FALSE;
					memset(session+NETWORK_SESSION_UNKNOWN4A00_OFFSET, 0, NETWORK_SESSION_UNKNOWN4A00_SIZE);
					changed= TRUE;
				}
				else if (network_session_roster_valid((const struct network_session_roster *)roster))
				{
					session[NETWORK_SESSION_UNKNOWN49FD_OFFSET]= TRUE;
					memcpy(session+NETWORK_SESSION_UNKNOWN4A00_OFFSET, roster, NETWORK_SESSION_UNKNOWN4A00_SIZE);
					changed= TRUE;
				}
				else
				{
					result= FALSE;
				}
			}
		}
		if (request[NETWORK_SESSION_PARAMETERS_REQUEST_HAS_UNKNOWN4D08_OFFSET])
		{
			long unknown4d08= *(const long *)(request+NETWORK_SESSION_PARAMETERS_REQUEST_UNKNOWN4D08_OFFSET);
			long unknown4d0c= *(const long *)(request+NETWORK_SESSION_PARAMETERS_REQUEST_UNKNOWN4D0C_OFFSET);
			const char *text= (const char *)(request+NETWORK_SESSION_PARAMETERS_REQUEST_UNKNOWN4D10_OFFSET);
			char *kept= (char *)(session+NETWORK_SESSION_UNKNOWN4D10_OFFSET);

			if (*(const long *)(session+NETWORK_SESSION_UNKNOWN4D08_OFFSET)!=unknown4d08 ||
				*(const long *)(session+NETWORK_SESSION_UNKNOWN4D0C_OFFSET)!=unknown4d0c ||
				strncmp(kept, text, NETWORK_SESSION_UNKNOWN4D10_SIZE)!=0)
			{
				*(long *)(session+NETWORK_SESSION_UNKNOWN4D08_OFFSET)= unknown4d08;
				*(long *)(session+NETWORK_SESSION_UNKNOWN4D0C_OFFSET)= unknown4d0c;
				strncpy(kept, text, NETWORK_SESSION_UNKNOWN4D10_SIZE);
				kept[NETWORK_SESSION_UNKNOWN4D10_SIZE-1]= 0;
				changed= TRUE;
			}
		}
		if (request[NETWORK_SESSION_PARAMETERS_REQUEST_HAS_UNKNOWN4DAC_OFFSET])
		{
			value= *(const long *)(request+NETWORK_SESSION_PARAMETERS_REQUEST_UNKNOWN4DAC_OFFSET);
			if (*(const long *)(session+NETWORK_SESSION_UNKNOWN4DAC_OFFSET)!=value)
			{
				*(long *)(session+NETWORK_SESSION_UNKNOWN4DAC_OFFSET)= value;
				changed= TRUE;
			}
		}
		if (request[NETWORK_SESSION_PARAMETERS_REQUEST_HAS_UNKNOWN4DB0_OFFSET] &&
			memcmp(session+NETWORK_SESSION_UNKNOWN4DB0_OFFSET, request+NETWORK_SESSION_PARAMETERS_REQUEST_UNKNOWN4DB0_OFFSET,
				NETWORK_SESSION_UNKNOWN4DB0_SIZE)!=0)
		{
			memcpy(session+NETWORK_SESSION_UNKNOWN4DB0_OFFSET, request+NETWORK_SESSION_PARAMETERS_REQUEST_UNKNOWN4DB0_OFFSET,
				NETWORK_SESSION_UNKNOWN4DB0_SIZE);
			changed= TRUE;
		}
		if (request[NETWORK_SESSION_PARAMETERS_REQUEST_HAS_UNKNOWN4EE0_OFFSET])
		{
			const wchar *text= (const wchar *)(request+NETWORK_SESSION_PARAMETERS_REQUEST_UNKNOWN4EE0_OFFSET);
			wchar *kept= (wchar *)(session+NETWORK_SESSION_UNKNOWN4EE0_OFFSET);

			if (wcsncmp(kept, text, NETWORK_SESSION_UNKNOWN4EE0_LENGTH)!=0)
			{
				wcsncpy(kept, text, NETWORK_SESSION_UNKNOWN4EE0_LENGTH-1);
				kept[NETWORK_SESSION_UNKNOWN4EE0_LENGTH-1]= 0;
				changed= TRUE;
			}
		}
		if (request[NETWORK_SESSION_PARAMETERS_REQUEST_HAS_UNKNOWN49A1_OFFSET] &&
			session[NETWORK_SESSION_UNKNOWN49A1_OFFSET]!=request[NETWORK_SESSION_PARAMETERS_REQUEST_UNKNOWN49A1_OFFSET])
		{
			session[NETWORK_SESSION_UNKNOWN49A1_OFFSET]= request[NETWORK_SESSION_PARAMETERS_REQUEST_UNKNOWN49A1_OFFSET];
			changed= TRUE;
		}
		if (request[NETWORK_SESSION_PARAMETERS_REQUEST_HAS_UNKNOWN5DD0_OFFSET] &&
			*(const short *)(session+NETWORK_SESSION_UNKNOWN5DD0_OFFSET)!=
			*(const short *)(request+NETWORK_SESSION_PARAMETERS_REQUEST_UNKNOWN5DD0_OFFSET))
		{
			*(short *)(session+NETWORK_SESSION_UNKNOWN5DD0_OFFSET)=
				*(const short *)(request+NETWORK_SESSION_PARAMETERS_REQUEST_UNKNOWN5DD0_OFFSET);
			changed= TRUE;
		}
		if (request[NETWORK_SESSION_PARAMETERS_REQUEST_HAS_UNKNOWN5DDC_OFFSET])
		{
			boolean valid= request[NETWORK_SESSION_PARAMETERS_REQUEST_UNKNOWN5DDC_VALID_OFFSET];
			const byte *unknown5ddc= request+NETWORK_SESSION_PARAMETERS_REQUEST_UNKNOWN5DDC_OFFSET;

			if (valid!=session[NETWORK_SESSION_UNKNOWN5DD8_OFFSET] ||
				memcmp(session+NETWORK_SESSION_UNKNOWN5DDC_OFFSET, unknown5ddc, NETWORK_SESSION_UNKNOWN5DDC_SIZE)!=0)
			{
				if (!valid)
				{
					session[NETWORK_SESSION_UNKNOWN5DD8_OFFSET]= FALSE;
					memset(session+NETWORK_SESSION_UNKNOWN5DDC_OFFSET, 0, NETWORK_SESSION_UNKNOWN5DDC_SIZE);
					changed= TRUE;
				}
				else if (network_session_unknown_0005c190(unknown5ddc, session))
				{
					session[NETWORK_SESSION_UNKNOWN5DD8_OFFSET]= TRUE;
					memcpy(session+NETWORK_SESSION_UNKNOWN5DDC_OFFSET, unknown5ddc, NETWORK_SESSION_UNKNOWN5DDC_SIZE);
					changed= TRUE;
				}
				else
				{
					result= FALSE;
				}
			}
		}
	}
	if (request[NETWORK_SESSION_PARAMETERS_REQUEST_HAS_UNKNOWN49FC_OFFSET] &&
		session[NETWORK_SESSION_UNKNOWN49FC_OFFSET]!=request[NETWORK_SESSION_PARAMETERS_REQUEST_UNKNOWN49FC_OFFSET])
	{
		session[NETWORK_SESSION_UNKNOWN49FC_OFFSET]= request[NETWORK_SESSION_PARAMETERS_REQUEST_UNKNOWN49FC_OFFSET];
		changed= TRUE;
	}
	if (changed)
	{
		network_session_parameters_changed(session);
	}

	return result;
}

/* 0x00059a30, 0x000599b0, 0x000599f0, 0x00059ab0, of network_life_cycle.c:
the four kinds of the thing that a session knows at 0x49CC */
struct network_life_cycle_request;
void __attribute__((stdcall)) network_life_cycle_request_new1(long unknown4, long unknown8, long unknownC,
	long unknown10, struct network_life_cycle_request *request);
void network_life_cycle_request_new2(struct network_life_cycle_request *request, long value);
void network_life_cycle_request_new3(struct network_life_cycle_request *request, long value);
void network_life_cycle_request_new4(struct network_life_cycle_request *request);

/* where something that a session knows is, in a copy of what it knows */
static inline __attribute__((always_inline)) byte *network_session_parameters_at(
	byte *parameters,
	long session_offset)
{
	return parameters+(session_offset-NETWORK_SESSION_UNKNOWN4978_OFFSET);
}

/* 0x0005d5a0: what a session knows, as it is after what the console that
runs it sends to tell what it knows now; whether it is right. It starts as
what the session knows; and FALSE, with nothing more, if what comes is what
has changed since a number that is not the one that the session has. Then
it has the number that comes, and each thing that comes. With the numbers of
0x4980 and 0x497C, the time is now. FALSE too, with the rest done, if the
thing of 0x4A00 is not right (network_session_roster_valid) or the one of
0x5DDC is not (network_session_unknown_0005c190): those are left as they were */
boolean NETWORK_SESSION_CALLBACK network_session_parameters_apply_update(
	const byte *session,
	const byte *update,
	byte *parameters)
{
	boolean result= TRUE;
	long baseline;

	memcpy(parameters, session+NETWORK_SESSION_UNKNOWN4978_OFFSET, NETWORK_SESSION_UNKNOWN4978_SIZE);
	baseline= *(const long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_BASELINE_OFFSET);
	if (baseline!=NONE && baseline!=*(const long *)(session+NETWORK_SESSION_UNKNOWN4978_OFFSET))
	{
		return FALSE;
	}
	*(long *)parameters= *(const long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_VERSION_OFFSET);
	if (update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN4980_OFFSET])
	{
		*(long *)network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN4980_OFFSET)=
			*(const long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN4980_OFFSET);
		*(long *)network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN497C_OFFSET)=
			*(const long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN497C_OFFSET);
		*(long *)network_session_parameters_at(parameters, NETWORK_SESSION_START_TIME_OFFSET)= network_session_time();
	}
	if (update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_LANGUAGE_OFFSET])
	{
		*(long *)network_session_parameters_at(parameters, NETWORK_SESSION_LANGUAGE_OFFSET)=
			*(const long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_LANGUAGE_OFFSET);
	}
	if (update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN498C_OFFSET])
	{
		*(long *)network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN498C_OFFSET)=
			*(const long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN498C_OFFSET);
	}
	if (update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN4990_OFFSET])
	{
		*(long *)network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN4990_OFFSET)=
			*(const long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN4990_OFFSET);
		*(long *)network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN4994_OFFSET)=
			*(const long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN4994_OFFSET);
	}
	if (update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN4998_OFFSET])
	{
		*network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN4998_OFFSET)=
			update[NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN4998_OFFSET];
		if (update[NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN4998_OFFSET])
		{
			memcpy(network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN4999_OFFSET),
				update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN4999_OFFSET, NETWORK_SESSION_UNKNOWN4999_SIZE);
		}
	}
	if (update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN49A4_OFFSET])
	{
		*(long *)network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN49A4_OFFSET)=
			*(const long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN49A4_OFFSET);
	}
	if (update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN49C4_OFFSET])
	{
		*network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN49C4_OFFSET)=
			update[NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN49C4_OFFSET];
	}
	if (update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN49C8_OFFSET])
	{
		*(long *)network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN49C8_OFFSET)=
			*(const long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN49C8_OFFSET);
	}
	if (update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN49FC_OFFSET])
	{
		*network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN49FC_OFFSET)=
			update[NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN49FC_OFFSET];
	}
	if (update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN49F8_OFFSET])
	{
		*(long *)network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN49F8_OFFSET)=
			*(const long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN49F8_OFFSET);
	}
	if (update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN49CC_OFFSET])
	{
		struct network_life_cycle_request *request=
			(struct network_life_cycle_request *)network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN49CC_OFFSET);
		const long *numbers= (const long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN49CC_KIND1_OFFSET);

		switch (*(const long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN49CC_KIND_OFFSET))
		{
		case 1:
			network_life_cycle_request_new1(numbers[0], numbers[1], numbers[2], numbers[3], request);
			break;
		case 2:
			network_life_cycle_request_new2(request,
				*(const long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN49CC_KIND2_OFFSET));
			break;
		case 3:
			network_life_cycle_request_new3(request,
				*(const long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN49CC_KIND3_OFFSET));
			break;
		case 4:
			network_life_cycle_request_new4(request);
			break;
		default:
			memset(request, 0, NETWORK_SESSION_UNKNOWN49CC_SIZE);
			break;
		}
	}
	if (update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_ROSTER_OFFSET])
	{
		const byte *roster= update+NETWORK_SESSION_PARAMETERS_UPDATE_ROSTER_OFFSET;

		if (!update[NETWORK_SESSION_PARAMETERS_UPDATE_ROSTER_VALID_OFFSET])
		{
			*network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN49FD_OFFSET)= FALSE;
			memset(network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN4A00_OFFSET), 0,
				NETWORK_SESSION_UNKNOWN4A00_SIZE);
		}
		else if (network_session_roster_valid((const struct network_session_roster *)roster))
		{
			*network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN49FD_OFFSET)= TRUE;
			memcpy(network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN4A00_OFFSET), roster,
				NETWORK_SESSION_UNKNOWN4A00_SIZE);
		}
		else
		{
			result= FALSE;
		}
	}
	if (update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN49E8_OFFSET])
	{
		long *unknown49f0= (long *)network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN49F0_OFFSET);

		*network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN49E8_OFFSET)=
			update[NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN49E8_OFFSET];
		if (update[NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN49E8_OFFSET])
		{
			unknown49f0[0]= *(const long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN49F0_OFFSET);
			unknown49f0[1]= *(const long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN49F0_OFFSET+sizeof(long));
		}
		else
		{
			unknown49f0[0]= 0;
			unknown49f0[1]= 0;
		}
	}
	if (update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN4D08_OFFSET])
	{
		char *text= (char *)network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN4D10_OFFSET);

		*(long *)network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN4D08_OFFSET)=
			*(const long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN4D08_OFFSET);
		*(long *)network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN4D0C_OFFSET)=
			*(const long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN4D0C_OFFSET);
		strncpy(text, (const char *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN4D10_OFFSET),
			NETWORK_SESSION_UNKNOWN4D10_SIZE);
		text[NETWORK_SESSION_UNKNOWN4D10_SIZE-1]= 0;
	}
	if (update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_RANDOM_OFFSET])
	{
		memcpy(network_session_parameters_at(parameters, NETWORK_SESSION_RANDOM_OFFSET),
			update+NETWORK_SESSION_PARAMETERS_UPDATE_RANDOM_OFFSET, NETWORK_SESSION_RANDOM_SIZE);
	}
	if (update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN4DA8_OFFSET])
	{
		*(long *)network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN4DA8_OFFSET)=
			*(const long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN4DA8_OFFSET);
	}
	if (update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN4DAC_OFFSET])
	{
		*(long *)network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN4DAC_OFFSET)=
			*(const long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN4DAC_OFFSET);
	}
	if (update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN4DB0_OFFSET])
	{
		memcpy(network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN4DB0_OFFSET),
			update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN4DB0_OFFSET, NETWORK_SESSION_UNKNOWN4DB0_SIZE);
	}
	if (update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN4EE0_OFFSET])
	{
		wchar *text= (wchar *)network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN4EE0_OFFSET);

		wcsncpy(text, (const wchar *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN4EE0_OFFSET),
			NETWORK_SESSION_UNKNOWN4EE0_LENGTH-1);
		text[NETWORK_SESSION_UNKNOWN4EE0_LENGTH-1]= 0;
	}
	if (update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN4F20_OFFSET])
	{
		*network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN4F20_OFFSET)=
			update[NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN4F20_OFFSET];
		memcpy(network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN4F24_OFFSET),
			update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN4F24_OFFSET, NETWORK_SESSION_UNKNOWN4F24_SIZE);
		memcpy(network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN4F90_OFFSET),
			update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN4F90_OFFSET, NETWORK_SESSION_UNKNOWN4F90_SIZE);
	}
	if (update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN49A1_OFFSET])
	{
		*network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN49A1_OFFSET)=
			update[NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN49A1_OFFSET];
	}
	if (update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN5DD0_OFFSET])
	{
		*(short *)network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN5DD0_OFFSET)=
			*(const short *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN5DD0_OFFSET);
	}
	if (update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN49A8_OFFSET])
	{
		long unknown49b0= *(const long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN49B0_OFFSET);
		byte *unknown49b8= network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN49B8_OFFSET);

		*network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN49A8_OFFSET)=
			update[NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN49A8_OFFSET];
		*(long *)network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN49AC_OFFSET)=
			*(const long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN49AC_OFFSET);
		*(long *)network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN49B0_OFFSET)= unknown49b0;
		if (unknown49b0==1)
		{
			*(long *)network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN49B4_OFFSET)=
				*(const long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN49B4_OFFSET);
			memcpy(unknown49b8, update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN49B8_OFFSET,
				NETWORK_SESSION_UNKNOWN49B8_SIZE);
		}
		else
		{
			*(long *)network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN49B4_OFFSET)= unknown49b0==2 ?
				*(const long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN49B4_OFFSET) : NONE;
			memset(unknown49b8, 0, NETWORK_SESSION_UNKNOWN49B8_SIZE);
		}
	}
	if (update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN5DDC_OFFSET])
	{
		const byte *unknown5ddc= update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN5DDC_OFFSET;

		if (!update[NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN5DDC_VALID_OFFSET])
		{
			*network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN5DD8_OFFSET)= FALSE;
			memset(network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN5DDC_OFFSET), 0,
				NETWORK_SESSION_UNKNOWN5DDC_SIZE);
		}
		else if (network_session_unknown_0005c190(unknown5ddc, session))
		{
			*network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN5DD8_OFFSET)= TRUE;
			memcpy(network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN5DDC_OFFSET), unknown5ddc,
				NETWORK_SESSION_UNKNOWN5DDC_SIZE);
		}
		else
		{
			result= FALSE;
		}
	}
	if (update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN5E20_OFFSET])
	{
		*(long *)network_session_parameters_at(parameters, NETWORK_SESSION_UNKNOWN5E20_OFFSET)=
			*(const long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN5E20_OFFSET);
	}

	return result;
}

/* 0x0005d9e0: what the console that runs a session sends to tell what the
session knows comes. If it is right (network_session_parameters_apply_update), it is what
the session knows from now on; and if it brings the numbers of 0x4980 and
0x497C and asks for an answer, the console that runs the session is sent
something of the kind 0x24 with those two numbers. If it is not right,
network_session_host_lost. Whether it was right */
boolean NETWORK_SESSION_CALLBACK network_session_parameters_update_received(
	byte *session,
	const byte *update)
{
	byte parameters[NETWORK_SESSION_UNKNOWN4978_SIZE];
	byte answer[NETWORK_SESSION_MESSAGE_PARAMETERS_ANSWER_SIZE];

	if (!network_session_parameters_apply_update(session, update, parameters))
	{
		network_session_host_lost(session);

		return FALSE;
	}
	memcpy(session+NETWORK_SESSION_UNKNOWN4978_OFFSET, parameters, sizeof(parameters));
	if (update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN4980_OFFSET] &&
		update[NETWORK_SESSION_PARAMETERS_UPDATE_ANSWER_OFFSET])
	{
		memset(answer, 0, sizeof(answer));
		memcpy(answer, session+NETWORK_SESSION_IDENTIFIER_OFFSET, NETWORK_SESSION_IDENTIFIER_SIZE);
		*(long *)(answer+NETWORK_SESSION_ANSWER_UNKNOWN8_OFFSET)=
			*(const long *)(session+NETWORK_SESSION_UNKNOWN4980_OFFSET);
		*(long *)(answer+NETWORK_SESSION_ANSWER_UNKNOWNC_OFFSET)=
			*(const long *)(session+NETWORK_SESSION_UNKNOWN497C_OFFSET);
		network_session_send_to_host(session, _network_session_message_parameters_answer, sizeof(answer), answer);
	}

	return TRUE;
}

/* where something that a session knows is, in what it knows or in a copy of
it that is only read */
static inline __attribute__((always_inline)) const byte *network_session_parameters_get(
	const byte *parameters,
	long session_offset)
{
	return parameters+(session_offset-NETWORK_SESSION_UNKNOWN4978_OFFSET);
}

/* whether a thing of a size is not the same in what a session knows and in
what it knew: the game has this for each thing, and with nothing that it
knew every thing is taken as another */
static inline __attribute__((always_inline)) boolean network_session_parameters_differ(
	const byte *parameters,
	const byte *baseline,
	long session_offset,
	long size)
{
	return !baseline || memcmp(network_session_parameters_get(parameters, session_offset),
		network_session_parameters_get(baseline, session_offset), size)!=0;
}

/* 0x000609e0: what the console that runs a session sends to a console to
tell it what the session knows: what the session is known by, the number
that what it knows has, and each thing, after a byte that says that it
comes. With what the session knew the last time, the number of that goes
too, and only the things that are not the same come; without it, the number
is NONE and every thing comes. With the numbers of 0x4980 and 0x497C goes
the byte of 0x765C of the session, which asks for an answer. The thing of
0x49CC goes as its kind and the numbers that its kind is made with.
network_session_parameters_apply_update takes all of it on the other side */
void NETWORK_SESSION_CALLBACK network_session_parameters_get_update(
	const byte *session,
	const byte *parameters,
	const byte *baseline,
	byte *update)
{
	memset(update, 0, NETWORK_SESSION_MESSAGE_PARAMETERS_UPDATE_SIZE);
	memcpy(update, session+NETWORK_SESSION_IDENTIFIER_OFFSET, NETWORK_SESSION_IDENTIFIER_SIZE);
	*(long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_VERSION_OFFSET)= *(const long *)parameters;
	*(long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_BASELINE_OFFSET)= baseline ? *(const long *)baseline : NONE;
	if (network_session_parameters_differ(parameters, baseline, NETWORK_SESSION_UNKNOWN497C_OFFSET, 2*sizeof(long)))
	{
		update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN4980_OFFSET]= TRUE;
		*(long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN4980_OFFSET)=
			*(const long *)network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN4980_OFFSET);
		*(long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN497C_OFFSET)=
			*(const long *)network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN497C_OFFSET);
		update[NETWORK_SESSION_PARAMETERS_UPDATE_ANSWER_OFFSET]= session[NETWORK_SESSION_UNKNOWN765C_OFFSET];
	}
	if (network_session_parameters_differ(parameters, baseline, NETWORK_SESSION_LANGUAGE_OFFSET, sizeof(long)))
	{
		update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_LANGUAGE_OFFSET]= TRUE;
		*(long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_LANGUAGE_OFFSET)=
			*(const long *)network_session_parameters_get(parameters, NETWORK_SESSION_LANGUAGE_OFFSET);
	}
	if (network_session_parameters_differ(parameters, baseline, NETWORK_SESSION_UNKNOWN498C_OFFSET, sizeof(long)))
	{
		update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN498C_OFFSET]= TRUE;
		*(long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN498C_OFFSET)=
			*(const long *)network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN498C_OFFSET);
	}
	if (network_session_parameters_differ(parameters, baseline, NETWORK_SESSION_UNKNOWN4990_OFFSET, 2*sizeof(long)))
	{
		update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN4990_OFFSET]= TRUE;
		*(long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN4990_OFFSET)=
			*(const long *)network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN4990_OFFSET);
		*(long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN4994_OFFSET)=
			*(const long *)network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN4994_OFFSET);
	}
	if (network_session_parameters_differ(parameters, baseline, NETWORK_SESSION_UNKNOWN4998_OFFSET,
		1+NETWORK_SESSION_UNKNOWN4999_SIZE))
	{
		boolean unknown4998= *network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN4998_OFFSET);

		update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN4998_OFFSET]= TRUE;
		update[NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN4998_OFFSET]= unknown4998;
		if (unknown4998)
		{
			memcpy(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN4999_OFFSET,
				network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN4999_OFFSET),
				NETWORK_SESSION_UNKNOWN4999_SIZE);
		}
	}
	if (network_session_parameters_differ(parameters, baseline, NETWORK_SESSION_UNKNOWN49A4_OFFSET, sizeof(long)))
	{
		update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN49A4_OFFSET]= TRUE;
		*(long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN49A4_OFFSET)=
			*(const long *)network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN49A4_OFFSET);
	}
	if (network_session_parameters_differ(parameters, baseline, NETWORK_SESSION_UNKNOWN49C4_OFFSET, 1))
	{
		update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN49C4_OFFSET]= TRUE;
		update[NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN49C4_OFFSET]=
			*network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN49C4_OFFSET);
	}
	if (network_session_parameters_differ(parameters, baseline, NETWORK_SESSION_UNKNOWN49C8_OFFSET, sizeof(long)))
	{
		update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN49C8_OFFSET]= TRUE;
		*(long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN49C8_OFFSET)=
			*(const long *)network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN49C8_OFFSET);
	}
	if (network_session_parameters_differ(parameters, baseline, NETWORK_SESSION_UNKNOWN49FC_OFFSET, 1))
	{
		update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN49FC_OFFSET]= TRUE;
		update[NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN49FC_OFFSET]=
			*network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN49FC_OFFSET);
	}
	if (network_session_parameters_differ(parameters, baseline, NETWORK_SESSION_UNKNOWN49F8_OFFSET, sizeof(long)))
	{
		update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN49F8_OFFSET]= TRUE;
		*(long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN49F8_OFFSET)=
			*(const long *)network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN49F8_OFFSET);
	}
	if (network_session_parameters_differ(parameters, baseline, NETWORK_SESSION_UNKNOWN49CC_OFFSET,
		NETWORK_SESSION_UNKNOWN49CC_SIZE))
	{
		const long *request= (const long *)network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN49CC_OFFSET);
		long *numbers= (long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN49CC_KIND1_OFFSET);

		update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN49CC_OFFSET]= TRUE;
		*(long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN49CC_KIND_OFFSET)= request[0];
		switch (request[0])
		{
		case 1:
			numbers[0]= request[1];
			numbers[1]= request[2];
			numbers[2]= request[3];
			numbers[3]= request[4];
			break;
		case 2:
			*(long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN49CC_KIND2_OFFSET)= request[5];
			break;
		case 3:
			*(long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN49CC_KIND3_OFFSET)= request[6];
			break;
		}
	}
	if (!baseline ||
		*network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN49FD_OFFSET)!=
			*network_session_parameters_get(baseline, NETWORK_SESSION_UNKNOWN49FD_OFFSET) ||
		network_session_parameters_differ(parameters, baseline, NETWORK_SESSION_UNKNOWN4A00_OFFSET,
			NETWORK_SESSION_UNKNOWN4A00_SIZE))
	{
		update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_ROSTER_OFFSET]= TRUE;
		update[NETWORK_SESSION_PARAMETERS_UPDATE_ROSTER_VALID_OFFSET]=
			*network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN49FD_OFFSET);
		memcpy(update+NETWORK_SESSION_PARAMETERS_UPDATE_ROSTER_OFFSET,
			network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN4A00_OFFSET), NETWORK_SESSION_UNKNOWN4A00_SIZE);
	}
	if (!baseline ||
		*network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN49E8_OFFSET)!=
			*network_session_parameters_get(baseline, NETWORK_SESSION_UNKNOWN49E8_OFFSET) ||
		network_session_parameters_differ(parameters, baseline, NETWORK_SESSION_UNKNOWN49F0_OFFSET, 2*sizeof(long)))
	{
		const long *unknown49f0= (const long *)network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN49F0_OFFSET);

		update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN49E8_OFFSET]= TRUE;
		update[NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN49E8_OFFSET]=
			*network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN49E8_OFFSET);
		*(long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN49F0_OFFSET)= unknown49f0[0];
		*(long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN49F0_OFFSET+sizeof(long))= unknown49f0[1];
	}
	if (network_session_parameters_differ(parameters, baseline, NETWORK_SESSION_UNKNOWN4D08_OFFSET, 2*sizeof(long)) ||
		strncmp((const char *)network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN4D10_OFFSET),
			(const char *)network_session_parameters_get(baseline, NETWORK_SESSION_UNKNOWN4D10_OFFSET),
			NETWORK_SESSION_UNKNOWN4D10_SIZE)!=0)
	{
		char *text= (char *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN4D10_OFFSET);

		update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN4D08_OFFSET]= TRUE;
		*(long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN4D08_OFFSET)=
			*(const long *)network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN4D08_OFFSET);
		*(long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN4D0C_OFFSET)=
			*(const long *)network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN4D0C_OFFSET);
		strncpy(text, (const char *)network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN4D10_OFFSET),
			NETWORK_SESSION_UNKNOWN4D10_SIZE);
		text[NETWORK_SESSION_UNKNOWN4D10_SIZE-1]= 0;
	}
	if (network_session_parameters_differ(parameters, baseline, NETWORK_SESSION_RANDOM_OFFSET, NETWORK_SESSION_RANDOM_SIZE))
	{
		update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_RANDOM_OFFSET]= TRUE;
		memcpy(update+NETWORK_SESSION_PARAMETERS_UPDATE_RANDOM_OFFSET,
			network_session_parameters_get(parameters, NETWORK_SESSION_RANDOM_OFFSET), NETWORK_SESSION_RANDOM_SIZE);
	}
	if (network_session_parameters_differ(parameters, baseline, NETWORK_SESSION_UNKNOWN4DA8_OFFSET, sizeof(long)))
	{
		update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN4DA8_OFFSET]= TRUE;
		*(long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN4DA8_OFFSET)=
			*(const long *)network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN4DA8_OFFSET);
	}
	if (network_session_parameters_differ(parameters, baseline, NETWORK_SESSION_UNKNOWN4DAC_OFFSET, sizeof(long)))
	{
		update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN4DAC_OFFSET]= TRUE;
		*(long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN4DAC_OFFSET)=
			*(const long *)network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN4DAC_OFFSET);
	}
	if (network_session_parameters_differ(parameters, baseline, NETWORK_SESSION_UNKNOWN4DB0_OFFSET,
		NETWORK_SESSION_UNKNOWN4DB0_SIZE))
	{
		update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN4DB0_OFFSET]= TRUE;
		memcpy(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN4DB0_OFFSET,
			network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN4DB0_OFFSET), NETWORK_SESSION_UNKNOWN4DB0_SIZE);
	}
	if (!baseline ||
		wcsncmp((const wchar *)network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN4EE0_OFFSET),
			(const wchar *)network_session_parameters_get(baseline, NETWORK_SESSION_UNKNOWN4EE0_OFFSET),
			NETWORK_SESSION_UNKNOWN4EE0_LENGTH)!=0)
	{
		wchar *text= (wchar *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN4EE0_OFFSET);

		update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN4EE0_OFFSET]= TRUE;
		wcsncpy(text, (const wchar *)network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN4EE0_OFFSET),
			NETWORK_SESSION_UNKNOWN4EE0_LENGTH-1);
		text[NETWORK_SESSION_UNKNOWN4EE0_LENGTH-1]= 0;
	}
	if (!baseline ||
		*network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN4F20_OFFSET)!=
			*network_session_parameters_get(baseline, NETWORK_SESSION_UNKNOWN4F20_OFFSET) ||
		network_session_parameters_differ(parameters, baseline, NETWORK_SESSION_UNKNOWN4F24_OFFSET,
			NETWORK_SESSION_UNKNOWN4F24_SIZE) ||
		network_session_parameters_differ(parameters, baseline, NETWORK_SESSION_UNKNOWN4F90_OFFSET,
			NETWORK_SESSION_UNKNOWN4F90_SIZE))
	{
		update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN4F20_OFFSET]= TRUE;
		update[NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN4F20_OFFSET]=
			*network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN4F20_OFFSET);
		memcpy(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN4F24_OFFSET,
			network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN4F24_OFFSET), NETWORK_SESSION_UNKNOWN4F24_SIZE);
		memcpy(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN4F90_OFFSET,
			network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN4F90_OFFSET), NETWORK_SESSION_UNKNOWN4F90_SIZE);
	}
	if (network_session_parameters_differ(parameters, baseline, NETWORK_SESSION_UNKNOWN49A1_OFFSET, 1))
	{
		update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN49A1_OFFSET]= TRUE;
		update[NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN49A1_OFFSET]=
			*network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN49A1_OFFSET);
	}
	if (network_session_parameters_differ(parameters, baseline, NETWORK_SESSION_UNKNOWN5DD0_OFFSET, sizeof(short)))
	{
		update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN5DD0_OFFSET]= TRUE;
		*(short *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN5DD0_OFFSET)=
			*(const short *)network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN5DD0_OFFSET);
	}
	if (!baseline ||
		*network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN49A8_OFFSET)!=
			*network_session_parameters_get(baseline, NETWORK_SESSION_UNKNOWN49A8_OFFSET) ||
		network_session_parameters_differ(parameters, baseline, NETWORK_SESSION_UNKNOWN49AC_OFFSET,
			3*sizeof(long)+NETWORK_SESSION_UNKNOWN49B8_SIZE))
	{
		update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN49A8_OFFSET]= TRUE;
		update[NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN49A8_OFFSET]=
			*network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN49A8_OFFSET);
		*(long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN49AC_OFFSET)=
			*(const long *)network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN49AC_OFFSET);
		*(long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN49B0_OFFSET)=
			*(const long *)network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN49B0_OFFSET);
		*(long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN49B4_OFFSET)=
			*(const long *)network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN49B4_OFFSET);
		memcpy(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN49B8_OFFSET,
			network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN49B8_OFFSET), NETWORK_SESSION_UNKNOWN49B8_SIZE);
	}
	if (!baseline ||
		*network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN5DD8_OFFSET)!=
			*network_session_parameters_get(baseline, NETWORK_SESSION_UNKNOWN5DD8_OFFSET) ||
		network_session_parameters_differ(parameters, baseline, NETWORK_SESSION_UNKNOWN5DDC_OFFSET,
			NETWORK_SESSION_UNKNOWN5DDC_SIZE))
	{
		update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN5DDC_OFFSET]= TRUE;
		update[NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN5DDC_VALID_OFFSET]=
			*network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN5DD8_OFFSET);
		memcpy(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN5DDC_OFFSET,
			network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN5DDC_OFFSET), NETWORK_SESSION_UNKNOWN5DDC_SIZE);
	}
	if (network_session_parameters_differ(parameters, baseline, NETWORK_SESSION_UNKNOWN5E20_OFFSET, sizeof(long)))
	{
		update[NETWORK_SESSION_PARAMETERS_UPDATE_HAS_UNKNOWN5E20_OFFSET]= TRUE;
		*(long *)(update+NETWORK_SESSION_PARAMETERS_UPDATE_UNKNOWN5E20_OFFSET)=
			*(const long *)network_session_parameters_get(parameters, NETWORK_SESSION_UNKNOWN5E20_OFFSET);
	}
}
