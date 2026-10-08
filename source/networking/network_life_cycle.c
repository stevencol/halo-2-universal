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

/* ---------- prototypes of functions not decompiled yet */

/* 0x00070570: something that unknown_00527500 says, in the sixth state */
long unknown_00070570(byte *unknown);

/* ---------- globals */

/* 0x00527364, 0x0052736c: the two sessions of the network game */
byte *network_life_cycle_session;
byte *network_life_cycle_other_session;

/* 0x00527e78, 0x00527fa8: a number, and the one that is looked at in its
place when it is 2 */
long unknown_00527e78;
long unknown_00527fa8;

/* 0x00527500: something of the sixth state. Its size is not known */
byte unknown_00527500[4];

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
