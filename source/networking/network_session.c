/*
NETWORK_SESSION.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

A session of the network game. The name of the file is a guess, and so are
the names of its functions.
*/

#include "network_session.h"

#include <string.h>

/* ---------- code */

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
