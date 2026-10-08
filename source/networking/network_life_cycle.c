/*
NETWORK_LIFE_CYCLE.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

What the network game is doing now, and its sessions. The name of the file
is a guess, and so are the names of its functions.
*/

#include "network_life_cycle.h"

#include "main/main_game.h"
#include "networking/network_session.h"

#include <string.h>

/* ---------- prototypes of functions not decompiled yet */

/* 0x00070570: something that unknown_00527500 says, in the sixth state */
long unknown_00070570(byte *unknown);

/* 0x00059890: of this file */
void __attribute__((stdcall)) unknown_00059890(long unknown0, long unknown4, long unknown8);

/* 0x000598f0, 0x0005a520, 0x0005a400, 0x0005b570, 0x0005a6e0: things that
are done with a session; 0x0005b1e0: something of it */
void unknown_000598f0(byte *session);
void unknown_0005a520(byte *session);
void __attribute__((stdcall)) unknown_0005a400(byte *session, long unknown);
void unknown_0005b570(byte *session);
void unknown_0005a6e0(byte *session);
boolean unknown_0005b1e0(byte *session, long unknown);

/* ---------- globals */

/* 0x00527364, 0x0052736c: the two sessions of the network game */
byte *network_life_cycle_session;
byte *network_life_cycle_other_session;

/* 0x00527e78, 0x00527fa8: a number, and the one that is looked at in its
place when it is 2 */
long unknown_00527e78;
long unknown_00527fa8;

/* 0x00527500: something of the sixth state: at 8 it points to what has a
session at 0x30. Its size is not known */
byte unknown_00527500[UNKNOWN_00527500_SIZE];

/* 0x00527400, 0x00527fa0: two things of the network game that have a
session, at 0x30 and at 0x38 */
byte *unknown_00527400;
byte *unknown_00527fa0;

/* 0x005274fc, 0x005274e1: with the first at 0 in the fifth state, the second
is set when this console runs the session or there is none */
long unknown_005274fc;
boolean unknown_005274e1;

/* 0x00510518, 0x0051051c, 0x0051051d: if the first is not 0, the other two
are set before some things are done */
long unknown_00510518;
boolean unknown_0051051c;
boolean unknown_0051051d;

/* ---------- code */

/* 0x00059260 */
boolean network_life_cycle_in_state5(
	void)
{
	boolean result= FALSE;

	if (unknown_00527330 && unknown_00527334==_network_life_cycle_state5)
	{
		result= TRUE;
	}

	return result;
}

/* 0x000592d0: what the network game is doing; 0 if there is none */
long network_life_cycle_get_state(
	void)
{
	return unknown_00527330 ? unknown_00527334 : 0;
}

/* 0x000592f0: whether this console runs the session. The game has
network_session_local_peer_is_host in the function itself */
boolean network_life_cycle_local_peer_is_host(
	void)
{
	boolean result= FALSE;

	if (unknown_00527330)
	{
		const byte *session= network_life_cycle_session;
		long state= *(const long *)(session+NETWORK_SESSION_STATE_OFFSET);

		if (state!=_network_session_state_none && state>=_network_session_state3 && state<=_network_session_state8)
		{
			result= *(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET)==
				*(const long *)(session+NETWORK_SESSION_HOST_PEER_OFFSET);
		}
	}

	return result;
}

/* 0x00059330: whether there is a session and
network_session_unknown_00058d90 says no of it */
boolean network_life_cycle_unknown_00059330(
	void)
{
	if (unknown_00527330 && *(const long *)(network_life_cycle_session+NETWORK_SESSION_STATE_OFFSET)!=
		_network_session_state_none)
	{
		if (!network_session_unknown_00058d90(network_life_cycle_session))
		{
			return TRUE;
		}
	}

	return FALSE;
}

/* 0x00059470: whether the session says who is in it and a number of it is
0. The game does not see whether there is a network game */
boolean network_life_cycle_unknown_00059470(
	void)
{
	const byte *session= network_life_cycle_session;
	long state= *(const long *)(session+NETWORK_SESSION_STATE_OFFSET);

	return state>=_network_session_state3 && state<=_network_session_state8 &&
		*(const long *)(session+NETWORK_SESSION_UNKNOWN18_OFFSET)==0;
}

/* 0x000594f0 */
long network_life_cycle_unknown_000594f0(
	void)
{
	long result= unknown_00527e78;

	if (result==_unknown_00527e78_2)
	{
		result= unknown_00527fa8;
	}

	return result;
}

/* 0x00059500: a number of the session, if it says who is in it; 1 if not */
long network_life_cycle_unknown_00059500(
	void)
{
	const byte *session= network_life_cycle_session;
	long state= *(const long *)(session+NETWORK_SESSION_STATE_OFFSET);

	if (state>=_network_session_state3 && state<=_network_session_state8)
	{
		return *(const long *)(session+NETWORK_SESSION_UNKNOWN49F8_OFFSET);
	}

	return 1;
}

/* 0x00059570: what unknown_00070570 says, in the sixth state; 5 in the
seventh; 0 in the others. The game does not see whether there is a network
game */
long network_life_cycle_unknown_00059570(
	void)
{
	if (unknown_00527334==_network_life_cycle_state6)
	{
		return unknown_00070570(unknown_00527500);
	}
	if (unknown_00527334==_network_life_cycle_state7)
	{
		return _network_life_cycle_unknown_00059570_5;
	}

	return 0;
}

/* 0x00059670: the session of the network game; whether there is one. No
session is given if no place for it is given */
boolean network_life_cycle_get_session(
	byte **session)
{
	boolean result= FALSE;

	if (unknown_00527330 && *(const long *)(network_life_cycle_session+NETWORK_SESSION_STATE_OFFSET)!=
		_network_session_state_none)
	{
		if (session)
		{
			*session= network_life_cycle_session;
		}
		result= TRUE;
	}

	return result;
}

/* 0x000596a0: the other session of the network game; whether there is one */
boolean network_life_cycle_get_other_session(
	byte **session)
{
	boolean result= FALSE;

	if (unknown_00527330 && *(const long *)(network_life_cycle_other_session+NETWORK_SESSION_STATE_OFFSET)!=
		_network_session_state_none)
	{
		if (session)
		{
			*session= network_life_cycle_other_session;
		}
		result= TRUE;
	}

	return result;
}

/* 0x000591e0 */
void network_life_cycle_unknown_000591e0(
	void)
{
	unknown_00059890(0, 0, 0);
}

/* 0x000591f0 */
void network_life_cycle_unknown_000591f0(
	void)
{
	unknown_00059890(1, 0, 0);
}

/* 0x00059280: in the fifth state, with unknown_005274fc at 0: sets
unknown_005274e1 if what unknown_00527400 has for a session is none, or one
that this console runs */
void network_life_cycle_unknown_00059280(
	void)
{
	if (unknown_00527330 && unknown_00527334==_network_life_cycle_state5 && unknown_005274fc==0)
	{
		const byte *session= *(byte *const *)(unknown_00527400+UNKNOWN_00527400_SESSION_OFFSET);
		long state= *(const long *)(session+NETWORK_SESSION_STATE_OFFSET);

		if (state==_network_session_state_none ||
			(state>=_network_session_state3 && state<=_network_session_state8 &&
			*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET)==
			*(const long *)(session+NETWORK_SESSION_HOST_PEER_OFFSET)))
		{
			unknown_005274e1= TRUE;
		}
	}
}

/* the two things that are done with a session, one way or the other: the
game has this in the two functions that follow */
static inline __attribute__((always_inline)) void network_life_cycle_unknown_00059360_session(
	byte *session,
	boolean unknown)
{
	if (*(const long *)(session+NETWORK_SESSION_STATE_OFFSET)!=_network_session_state_none)
	{
		unknown_000598f0(session);
		if (unknown)
		{
			unknown_0005a520(session);
		}
		else
		{
			unknown_0005a400(session, 0);
		}
	}
}

/* 0x00059360: with the session */
void NETWORK_LIFE_CYCLE_CALLBACK network_life_cycle_unknown_00059360(
	boolean unknown)
{
	network_life_cycle_unknown_00059360_session(network_life_cycle_session, unknown);
}

/* 0x000593a0: with the other session */
void NETWORK_LIFE_CYCLE_CALLBACK network_life_cycle_unknown_000593a0(
	boolean unknown)
{
	network_life_cycle_unknown_00059360_session(network_life_cycle_other_session, unknown);
}

/* 0x00059530: something that is done with the session if this console runs
it */
void network_life_cycle_unknown_00059530(
	void)
{
	byte *session= network_life_cycle_session;
	long state= *(const long *)(session+NETWORK_SESSION_STATE_OFFSET);

	if (state>=_network_session_state3 && state<=_network_session_state8 &&
		*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET)==
		*(const long *)(session+NETWORK_SESSION_HOST_PEER_OFFSET))
	{
		unknown_0005b570(session);
	}
}

/* 0x00059590: in the seventh state, a number of the session that
unknown_00527fa0 has, if the session says who is in it and has the number;
NONE if not */
long network_life_cycle_unknown_00059590(
	void)
{
	if (unknown_00527334==_network_life_cycle_state7)
	{
		const byte *session= *(byte *const *)(unknown_00527fa0+UNKNOWN_00527FA0_SESSION_OFFSET);
		long state= *(const long *)(session+NETWORK_SESSION_STATE_OFFSET);

		if (state>=_network_session_state3 && state<=_network_session_state8 &&
			session[NETWORK_SESSION_UNKNOWN49A8_OFFSET])
		{
			return *(const long *)(session+NETWORK_SESSION_UNKNOWN49AC_OFFSET);
		}
	}

	return NONE;
}

/* 0x000595e0: in the sixth state, something that is done with the session
that unknown_00527500 leads to */
void network_life_cycle_unknown_000595e0(
	void)
{
	if (unknown_00527334==_network_life_cycle_state6)
	{
		const byte *unknown;

		if (unknown_00510518)
		{
			unknown_0051051c= TRUE;
			unknown_0051051d= TRUE;
		}
		memcpy(&unknown, unknown_00527500+UNKNOWN_00527500_UNKNOWN8_OFFSET, sizeof(unknown));
		unknown_0005a6e0(*(byte *const *)(unknown+UNKNOWN_00527508_SESSION_OFFSET));
	}
}

/* 0x00059610: in the first state, yes; in the others but 0, what
unknown_0005b1e0 says of the session, if this console runs it */
boolean network_life_cycle_unknown_00059610(
	void)
{
	if (unknown_00527334!=_network_life_cycle_state_none)
	{
		byte *session;
		long state;

		if (unknown_00527334==_network_life_cycle_state1)
		{
			return TRUE;
		}
		session= network_life_cycle_session;
		state= *(const long *)(session+NETWORK_SESSION_STATE_OFFSET);
		if (state>=_network_session_state3 && state<=_network_session_state8 &&
			*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET)==
			*(const long *)(session+NETWORK_SESSION_HOST_PEER_OFFSET))
		{
			if (unknown_0005b1e0(session, 1))
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}
