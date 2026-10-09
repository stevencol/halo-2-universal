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

/* 0x00065770: something that is done with a session that has just started */
void unknown_00065770(byte *session);


/* 0x0006dec0: what the network game is doing does its work of the frame: it
is given where what it is doing is kept. 0x00073040, 0x000737b0: the work of
the frame of unknown_0047708c and unknown_0047712c. 0x00075800: something
that unknown_00527360 has for the eight bytes of a session; 0 if nothing */
void unknown_0006dec0(long *life_cycle);
void unknown_00073040(byte *unknown);
void unknown_000737b0(byte *unknown);
long __attribute__((stdcall)) unknown_00075800(byte *unknown, const byte *identifier);

/* 0x0006f200, 0x0006f3a0: two things that are asked of unknown_005273f8 */
void __attribute__((stdcall)) unknown_0006f200(boolean unknownC, byte *unknown, long unknown4, long unknown0,
	long unknown8);
void __attribute__((stdcall)) unknown_0006f3a0(byte *unknown, long unknown4, long unknown0, long unknown8);

/* 0x000705f0: more that unknown_00527500 says, with three numbers */
boolean __attribute__((stdcall)) unknown_000705f0(byte *unknown, long unknown0, long unknown4, long unknown8);

/* 0x000641f0: something of the network game; NONE if none. 0x00192e60: what
there is for it; NULL if nothing. 0x00193470: something of that */
long unknown_000641f0(void);
byte *unknown_00192e60(long unknown);
boolean unknown_00193470(byte *unknown);

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

/* 0x0047708c, 0x0047712c: two things of the same kind that work each frame;
the second has functions of its own. Their sizes are guesses. 0x00527360:
something that says whether the session that is watched is still there */
byte unknown_0047708c[0xA0];
byte unknown_0047712c[0xA0];
byte *unknown_00527360;

/* 0x005273f8: something of the network game that two things are asked of.
What unknown_00527400 points to is at 8 of it */
byte unknown_005273f8[8];

/* 0x00527400, 0x00527fa0: two things of the network game that have a
session, at 0x30 and at 0x38 */
byte *unknown_00527400;
byte *unknown_00527fa0;

/* 0x005274fc, 0x005274e1: with the first at 0 in the fifth state, the second
is set when this console runs the session or there is none */
long unknown_005274fc;
boolean unknown_005274e1;

/* 0x00527338: what does each of the things that the network game can be
doing */
struct network_life_cycle_handler *network_life_cycle_handlers[NUMBER_OF_NETWORK_LIFE_CYCLE_STATES];

/* 0x0052737c, 0x0052737d: the first is set while network_life_cycle_end
works; the second is cleared by it */
boolean unknown_0052737c;
boolean unknown_0052737d;

/* 0x00510528: eight bytes that a session is known by */
byte unknown_00510528[NETWORK_SESSION_IDENTIFIER_SIZE];

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

/* 0x000591e0: starts the session, if there is none, in one way; whether it
was started */
boolean network_life_cycle_unknown_000591e0(
	void)
{
	return network_life_cycle_unknown_00059890(0, 0, 0);
}

/* 0x000591f0: starts the session, if there is none, in another way; whether
it was started */
boolean network_life_cycle_unknown_000591f0(
	void)
{
	return network_life_cycle_unknown_00059890(1, 0, 0);
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
		network_life_cycle_unknown_000598f0(session);
		if (unknown)
		{
			network_session_disconnect(session);
		}
		else
		{
			network_session_leave(session, 0);
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
		network_session_leader_clear_unknown49f8(session);
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
		network_session_request_unknown49fc(*(byte *const *)(unknown+UNKNOWN_00527508_SESSION_OFFSET));
	}
}

/* 0x00059610: in the first state, yes; in the others but 0, what
network_session_leader_unknown_0005b1e0 says of the session, if this console runs it */
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
			if (network_session_leader_unknown_0005b1e0(session, 1))
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

/* 0x000593e0: the network game stops what it is doing: the two sessions are
left, and what it was doing is told that it goes to doing nothing, and doing
nothing that it comes from that */
void network_life_cycle_end(
	void)
{
	if (*(const long *)(network_life_cycle_other_session+NETWORK_SESSION_STATE_OFFSET)!=_network_session_state_none)
	{
		network_life_cycle_unknown_000598f0(network_life_cycle_other_session);
		network_session_disconnect(network_life_cycle_other_session);
	}
	if (*(const long *)(network_life_cycle_session+NETWORK_SESSION_STATE_OFFSET)!=_network_session_state_none)
	{
		network_life_cycle_unknown_000598f0(network_life_cycle_session);
		network_session_disconnect(network_life_cycle_session);
	}
	unknown_0052737d= FALSE;
	unknown_0052737c= TRUE;
	if (unknown_00527334!=_network_life_cycle_state_none)
	{
		struct network_life_cycle_handler *handler= network_life_cycle_handlers[unknown_00527334];
		struct network_life_cycle_handler *none= network_life_cycle_handlers[_network_life_cycle_state_none];

		handler->functions->exit(handler, none);
		unknown_00527334= _network_life_cycle_state_none;
		none->functions->enter(none, handler, 0, 0);
	}
	unknown_0052737c= FALSE;
}

/* 0x000594a0: what unknown_000705f0 says with three numbers, when
network_life_cycle_unknown_000594f0 says 3 and
network_life_cycle_unknown_00059570 says 2 or 4; FALSE if not. The game has
those two in the function itself */
boolean NETWORK_LIFE_CYCLE_CALLBACK network_life_cycle_unknown_000594a0(
	long unknown0,
	long unknown4,
	long unknown8)
{
	if (network_life_cycle_unknown_000594f0()==_network_life_cycle_unknown_000594f0_3)
	{
		long unknown= network_life_cycle_unknown_00059570();

		if (unknown==_network_life_cycle_unknown_00059570_2 || unknown==_network_life_cycle_unknown_00059570_4)
		{
			return unknown_000705f0(unknown_00527500, unknown0, unknown4, unknown8);
		}
	}

	return FALSE;
}

/* 0x000596d0: the session that goes with what the network game is doing;
whether there is one. In the first five states, the session; in the last
three, the other one. In the sixth, the session, or the other one when
unknown_000641f0 has something that unknown_00193470 says no of and there is
another session: then it says that there is one, and gives it if a place for
it is given */
boolean network_life_cycle_get_current_session(
	byte **session)
{
	boolean result= FALSE;

	switch (network_life_cycle_get_state())
	{
	case _network_life_cycle_state1:
	case _network_life_cycle_state2:
	case _network_life_cycle_state3:
	case _network_life_cycle_state4:
	case _network_life_cycle_state5:
		result= network_life_cycle_get_session(session);
		break;
	case _network_life_cycle_state6:
		{
			long unknown= unknown_000641f0();

			result= network_life_cycle_get_session(session);
			if (unknown!=NONE)
			{
				byte *thing= unknown_00192e60(unknown);

				if (thing && !unknown_00193470(thing))
				{
					byte *other_session;

					if (network_life_cycle_get_other_session(&other_session))
					{
						result= TRUE;
						if (session)
						{
							*session= other_session;
						}
					}
				}
			}
		}
		break;
	case _network_life_cycle_state7:
	case _network_life_cycle_state8:
	case _network_life_cycle_state9:
		result= network_life_cycle_get_other_session(session);
		break;
	}

	return result;
}

/* 0x00059780: a session of the network game, the other one first; whether
there is one */
boolean network_life_cycle_get_any_session(
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
	if (!result && unknown_00527330 &&
		*(const long *)(network_life_cycle_session+NETWORK_SESSION_STATE_OFFSET)!=_network_session_state_none)
	{
		if (session)
		{
			*session= network_life_cycle_session;
		}
		result= TRUE;
	}

	return result;
}

/* 0x000597d0: the session of what the network game is doing: the session in
the first six states, and the other one in the last three; whether there is
one */
boolean network_life_cycle_get_state_session(
	byte **session)
{
	byte *state_session;

	switch (network_life_cycle_get_state())
	{
	case _network_life_cycle_state1:
	case _network_life_cycle_state2:
	case _network_life_cycle_state3:
	case _network_life_cycle_state4:
	case _network_life_cycle_state5:
	case _network_life_cycle_state6:
		state_session= network_life_cycle_session;
		break;
	case _network_life_cycle_state7:
	case _network_life_cycle_state8:
	case _network_life_cycle_state9:
		state_session= network_life_cycle_other_session;
		break;
	default:
		return FALSE;
	}
	if (*(const long *)(state_session+NETWORK_SESSION_STATE_OFFSET)!=_network_session_state_none)
	{
		if (session)
		{
			*session= state_session;
		}

		return TRUE;
	}

	return FALSE;
}

/* 0x00059840: the session, if it is in one of its last four states and a
number of it is 1; whether it is so */
boolean network_life_cycle_unknown_00059840(
	byte **session)
{
	if (unknown_00527330)
	{
		long state= *(const long *)(network_life_cycle_session+NETWORK_SESSION_STATE_OFFSET);

		if ((state==_network_session_state5 || state==_network_session_state6 || state==_network_session_state7 ||
			state==_network_session_state8) &&
			*(const long *)(network_life_cycle_session+NETWORK_SESSION_UNKNOWN18_OFFSET)==1)
		{
			if (session)
			{
				*session= network_life_cycle_session;
			}

			return TRUE;
		}
	}

	return FALSE;
}

/* 0x000598f0: with unknown_00510518 not 0: if a session is the one that
unknown_00510528 says, the two flags are set. The game does not see whether
the session has what it is known by */
void network_life_cycle_unknown_000598f0(
	const byte *session)
{
	if (unknown_00510518)
	{
		const byte *identifier= NULL;

		if (*(const long *)(session+NETWORK_SESSION_STATE_OFFSET)!=_network_session_state_none &&
			session[NETWORK_SESSION_HAS_IDENTIFIER_OFFSET])
		{
			identifier= session+NETWORK_SESSION_IDENTIFIER_OFFSET;
		}
		if (!memcmp(unknown_00510528, identifier, NETWORK_SESSION_IDENTIFIER_SIZE))
		{
			unknown_0051051c= TRUE;
			unknown_0051051d= TRUE;
		}
	}
}

/* a number, no less than 0 and no more than the most */
static inline __attribute__((always_inline)) long network_life_cycle_request_pin(
	long value)
{
	if (value<0)
	{
		return 0;
	}

	return value>MAXIMUM_NETWORK_LIFE_CYCLE_REQUEST_VALUE ? MAXIMUM_NETWORK_LIFE_CYCLE_REQUEST_VALUE : value;
}

/* 0x000599b0: a request of the second kind, with its number */
void network_life_cycle_request_new2(
	struct network_life_cycle_request *request,
	long value)
{
	memset(request, 0, sizeof(*request));
	request->kind= 2;
	request->unknown14= network_life_cycle_request_pin(value);
}

/* 0x000599f0: a request of the third kind, with its number */
void network_life_cycle_request_new3(
	struct network_life_cycle_request *request,
	long value)
{
	memset(request, 0, sizeof(*request));
	request->kind= 3;
	request->unknown18= network_life_cycle_request_pin(value);
}

/* 0x00059ab0: a request of the fourth kind */
void network_life_cycle_request_new4(
	struct network_life_cycle_request *request)
{
	memset(request, 0, sizeof(*request));
	request->kind= 4;
}

/* a number, no less than 0 and no more than a most */
static inline __attribute__((always_inline)) long network_life_cycle_pin(
	long value,
	long maximum)
{
	if (value<0)
	{
		return 0;
	}

	return value>maximum ? maximum : value;
}

/* 0x00059a30: a request of the first kind, with its four numbers */
void NETWORK_LIFE_CYCLE_CALLBACK network_life_cycle_request_new1(
	long unknown4,
	long unknown8,
	long unknownC,
	long unknown10,
	struct network_life_cycle_request *request)
{
	memset(request, 0, sizeof(*request));
	request->kind= 1;
	request->unknown4= network_life_cycle_pin(unknown4, MAXIMUM_NETWORK_LIFE_CYCLE_REQUEST_UNKNOWN4);
	request->unknown8= network_life_cycle_pin(unknown8, MAXIMUM_NETWORK_LIFE_CYCLE_REQUEST_UNKNOWN8);
	request->unknownC= network_life_cycle_pin(unknownC, MAXIMUM_NETWORK_LIFE_CYCLE_REQUEST_UNKNOWNC);
	request->unknown10= network_life_cycle_pin(unknown10, MAXIMUM_NETWORK_LIFE_CYCLE_REQUEST_VALUE);
}

/* 0x00059890: starts the session, if there is none, with three numbers;
whether it was started */
boolean NETWORK_LIFE_CYCLE_CALLBACK network_life_cycle_unknown_00059890(
	long unknown0,
	long unknown4,
	long unknown8)
{
	byte *session= network_life_cycle_session;
	boolean result= FALSE;

	if (*(const long *)(session+NETWORK_SESSION_STATE_OFFSET)==_network_session_state_none)
	{
		long unknown[2];

		unknown[0]= 0;
		unknown[1]= 0;
		if (network_session_start(session, unknown0, 0, unknown4, unknown8, 0, 0, 0, unknown, 0))
		{
			result= TRUE;
			unknown_00065770(session);
		}
	}

	return result;
}

/* the two flags are set, if unknown_00510518 is not 0: the game has this
before some things are done */
static inline __attribute__((always_inline)) void network_life_cycle_set_flags(
	void)
{
	if (unknown_00510518)
	{
		unknown_0051051c= TRUE;
		unknown_0051051d= TRUE;
	}
}

/* 0x00059200: something that is asked of unknown_005273f8, with three
numbers and a yes or a no */
void NETWORK_LIFE_CYCLE_CALLBACK network_life_cycle_unknown_00059200(
	long unknown0,
	long unknown4,
	long unknown8,
	boolean unknownC)
{
	network_life_cycle_set_flags();
	unknown_0006f200(unknownC, unknown_005273f8, unknown4, unknown0, unknown8);
}

/* 0x00059230: another thing that is asked of unknown_005273f8, with three
numbers */
void NETWORK_LIFE_CYCLE_CALLBACK network_life_cycle_unknown_00059230(
	long unknown0,
	long unknown4,
	long unknown8)
{
	network_life_cycle_set_flags();
	unknown_0006f3a0(unknown_005273f8, unknown4, unknown0, unknown8);
}

/* 0x00059940: the network game, each frame, if there is one: what it is
doing works, and so do two more things. And if a session is watched and
unknown_00527360 has nothing for it, the first flag is set and the second
cleared */
void network_life_cycle_update(
	void)
{
	if (unknown_00527330)
	{
		unknown_0006dec0(&unknown_00527334);
		unknown_00073040(unknown_0047708c);
		unknown_00073040(unknown_0047712c);
		unknown_000737b0(unknown_0047712c);
		if (unknown_00510518)
		{
			if (!unknown_00075800(unknown_00527360, unknown_00510528))
			{
				if (unknown_00510518)
				{
					unknown_0051051c= TRUE;
					unknown_0051051d= FALSE;
				}
			}
		}
	}
}

/* 0x00063e00: whether the session of the network game has room and takes
players: it says who is in it, its number of 0x498C is zero, it has fewer
players than it can have, the network game is not in its states from the
sixth to the ninth, and the 16 bits of 0x5DD0 of the session are NONE. FALSE
with no network game. The game has it far from the rest of this file */
boolean network_life_cycle_session_is_open(
	void)
{
	if (unknown_00527330)
	{
		const byte *session= network_life_cycle_session;
		long state= *(const long *)(session+NETWORK_SESSION_STATE_OFFSET);

		if (state!=_network_session_state_none && state>=_network_session_state3 && state<=_network_session_state8 &&
			*(const long *)(session+NETWORK_SESSION_UNKNOWN498C_OFFSET)==0 &&
			*(const long *)(session+NETWORK_SESSION_MEMBERSHIP_OFFSET+NETWORK_SESSION_MEMBERSHIP_PLAYER_COUNT_OFFSET)<
				*(const long *)(session+NETWORK_SESSION_UNKNOWN4994_OFFSET) &&
			unknown_00527334!=_network_life_cycle_state6 && unknown_00527334!=_network_life_cycle_state7 &&
			unknown_00527334!=_network_life_cycle_state8 && unknown_00527334!=_network_life_cycle_state9 &&
			*(const short *)(session+NETWORK_SESSION_UNKNOWN5DD0_OFFSET)==NONE)
		{
			return TRUE;
		}
	}

	return FALSE;
}

/* 0x00063f00: whether this console runs the session of the network game,
as who is in it says, and the number of 0x4980 of the session is 1. FALSE
with no network game, or with a session that does not say who is in it */
boolean network_life_cycle_unknown_00063f00(
	void)
{
	if (unknown_00527330)
	{
		const byte *session= network_life_cycle_session;
		long state= *(const long *)(session+NETWORK_SESSION_STATE_OFFSET);

		if (state!=_network_session_state_none && state>=_network_session_state3 && state<=_network_session_state8 &&
			*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET)==
				*(const long *)(session+NETWORK_SESSION_HOST_PEER_OFFSET) &&
			*(const long *)(session+NETWORK_SESSION_UNKNOWN4980_OFFSET)==1)
		{
			return TRUE;
		}
	}

	return FALSE;
}

/* the session of the network game, if there is a network game and the
session has a state; NULL if not. The game has this in each function that
uses it */
static inline __attribute__((always_inline)) byte *network_life_cycle_session_or_null(
	void)
{
	if (unknown_00527330 && *(const long *)(network_life_cycle_session+NETWORK_SESSION_STATE_OFFSET)!=
		_network_session_state_none)
	{
		return network_life_cycle_session;
	}

	return NULL;
}

/* 0x00063f50: the map of the session of the network game is set, or asked
for, if this console leads it (network_session_leader_set_map); TRUE too if
it does not lead it, so that the game starts the map itself. FALSE with no
network game, or with a session that does not say who is in it */
boolean network_life_cycle_set_map(
	long campaign,
	long map,
	const char *map_name)
{
	if (unknown_00527330)
	{
		byte *session= network_life_cycle_session;
		long state= *(const long *)(session+NETWORK_SESSION_STATE_OFFSET);

		if (state!=_network_session_state_none && state>=_network_session_state3 && state<=_network_session_state8)
		{
			if (*(const long *)(session+NETWORK_SESSION_LOCAL_PEER_OFFSET)!=
				*(const long *)(session+NETWORK_SESSION_HOST_PEER_OFFSET))
			{
				return TRUE;
			}

			return network_session_leader_set_map(session, map_name, campaign, map);
		}
	}

	return FALSE;
}

/* 0x00063fb0: the number of 0x49A4 of the session of the network game is
set, or asked for, if network_life_cycle_unknown_00063f00 says so; FALSE if
it does not */
boolean network_life_cycle_set_unknown49a4(
	long unknown49a4)
{
	if (!network_life_cycle_unknown_00063f00())
	{
		return FALSE;
	}

	return network_session_leader_set_unknown49a4(network_life_cycle_session_or_null(), unknown49a4);
}

/* 0x00063ff0: the map of the session of the network game is set, and then
network_session_unknown_0005bf10 is done with nothing, if
network_life_cycle_unknown_00063f00 says so; whether both could */
boolean NETWORK_LIFE_CYCLE_CALLBACK network_life_cycle_set_map_and_unknown49a8(
	long campaign,
	long map,
	const char *map_name)
{
	byte *session;

	if (!network_life_cycle_unknown_00063f00())
	{
		return FALSE;
	}
	session= network_life_cycle_session_or_null();

	return network_session_leader_set_map(session, map_name, campaign, map) &&
		network_session_unknown_0005bf10(session, FALSE, 0, 0, NULL);
}
