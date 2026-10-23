/*
USER_INTERFACE_NETWORKING.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

What the interface asks of the network game. The names are guesses.
*/

#include "user_interface_networking.h"

#include "networking/network_hoppers.h"
#include "networking/network_life_cycle.h"
#include "networking/network_session.h"

/* ---------- prototypes */

/* 0x0019989d: what the network game is at, as a number for the interface */
long unknown_0019989d(void);

/* ---------- code */

/* 0x001999bf: whether a number is 2 or 3 */
boolean __attribute__((stdcall)) user_interface_networking_unknown_001999bf(
	long unknown)
{
	return unknown==2 || unknown==3;
}

/* 0x001999d7: whether what the network game is at, for the interface, is 2
or 3 */
boolean user_interface_networking_unknown_001999d7(
	void)
{
	return user_interface_networking_unknown_001999bf(unknown_0019989d());
}

/* 0x00199e2e: two things of the network game are told the same thing, one
after the other */
void __attribute__((stdcall)) user_interface_networking_unknown_00199e2e(
	boolean unknown)
{
	network_life_cycle_unknown_00059360(unknown);
	network_life_cycle_unknown_000593a0(unknown);
}

/* 0x0019a015: whether the session of the network game is open, and
user_interface_networking_unknown_00199fd6 gives more than nothing */
boolean user_interface_networking_unknown_0019a015(
	void)
{
	return network_life_cycle_session_is_open() && user_interface_networking_unknown_00199fd6()>0;
}

/* 0x00199fd6: how many more players the network game can take: sixteen less
the ones that its session has, if there is one and it is established */
long user_interface_networking_unknown_00199fd6(
	void)
{
	byte *session= NULL;
	long player_count= 0;

	if (network_life_cycle_get_session(&session) && network_session_established(session))
	{
		long local_peer;
		long host_peer;
		const byte *membership= network_session_get_membership(&local_peer, session, &host_peer);

		player_count= *(const long *)(membership+NETWORK_SESSION_MEMBERSHIP_PLAYER_COUNT_OFFSET);
	}

	return USER_INTERFACE_NETWORKING_MAXIMUM_PLAYERS-player_count;
}

/* ---------- what the interface makes of the network game */

/* 0x000642d0, 0x00064260, 0x00064ab0, 0x00064c40 (network_life_cycle.c):
four things of the session of the network game */
const byte *network_life_cycle_session_get_unknown49A1(void);
long network_life_cycle_session_get_unknown498C(void);
long network_life_cycle_session_get_unknown49B0(void);
long network_life_cycle_session_get_unknown18(void);

/* 0x00199d7c: what the network game is doing, as a number for the
interface: 1, 8 or 2 for three of the things that it can be doing, and
from 3 to 7 by what network_life_cycle_unknown_00059570 says in the other;
0 for nothing that it knows */
long user_interface_networking_unknown_00199d7c(
	void)
{
	switch (network_life_cycle_unknown_000594f0())
	{
	case 1:
		return 1;
	case 2:
		return 8;
	case 3:
		switch (network_life_cycle_unknown_00059570())
		{
		case 1:
			return 3;
		case 2:
			return 4;
		case 3:
			return 5;
		case 4:
			return 6;
		case 5:
			return 7;
		default:
			return 0;
		}
	case 4:
		return 2;
	default:
		return 0;
	}
}

/* 0x00199e6d: whether what network_life_cycle_unknown_00063e90 says with a
number is nothing */
boolean __attribute__((stdcall)) user_interface_networking_unknown_00199e6d(
	long unknown)
{
	return network_life_cycle_unknown_00063e90(unknown)==0;
}

/* 0x00199eaa: the byte at 0x49A1 of the session of the network game;
nothing if there is none */
byte user_interface_networking_get_unknown49A1(
	void)
{
	const byte *unknown= network_life_cycle_session_get_unknown49A1();

	return unknown ? *unknown : 0;
}

/* 0x0019a161: the number at 0x498C of the session of the network game as 0,
1 or 2 (for anything else) */
long user_interface_networking_get_unknown498C(
	void)
{
	switch (network_life_cycle_session_get_unknown498C())
	{
	case 0:
		return 0;
	case 1:
		return 1;
	default:
		return 2;
	}
}

/* 0x0019a8ef: whether the number at 0x49B0 of the session of the network
game is 2 */
boolean user_interface_networking_unknown49B0_is_2(
	void)
{
	return network_life_cycle_session_get_unknown49B0()==2;
}

/* 0x0019a935: whether the number at 0x18 of the session of the network
game is 2 */
boolean user_interface_networking_unknown18_is_2(
	void)
{
	return network_life_cycle_session_get_unknown18()==2;
}

/* ---------- the network game and the list of games, for the interface */

/* 0x004d8ba0 (main.c): there is a game in the network */
extern boolean unknown_004d8ba0;

/* 0x00199df9: the network game is told twice of something
(user_interface_networking_unknown_00199e2e) and then one of three things
is done with it, by two bytes: network_life_cycle_unknown_000591e0 with the
first and not the second, network_life_cycle_unknown_000591f0 with neither,
and network_life_cycle_unknown_00059890 with 2 and nothing with the second.
Whether it could */
boolean __attribute__((stdcall)) user_interface_networking_unknown_00199df9(
	boolean unknown0,
	boolean unknown4)
{
	user_interface_networking_unknown_00199e2e(TRUE);
	if (unknown0 && !unknown4)
	{
		return network_life_cycle_unknown_000591e0();
	}
	if (!unknown0 && !unknown4)
	{
		return network_life_cycle_unknown_000591f0();
	}

	return network_life_cycle_unknown_00059890(2, 0, 0);
}

/* one of the list of games, the one of the number of 0x49C8 of the session
of the network game; NULL if there is none. The game has this in each
function that uses it */
static inline __attribute__((always_inline)) struct network_hopper *user_interface_networking_hopper(
	void)
{
	long index= network_life_cycle_session_get_unknown49C8();

	return index!=NONE ? network_hoppers_get(index) : NULL;
}

/* 0x0019a0c5: whether the one of the list of games of the network game is
of the second, fourth or fifth kind; FALSE if there is none */
boolean user_interface_networking_hopper_unknown_00193470(
	void)
{
	struct network_hopper *hopper= user_interface_networking_hopper();

	return hopper ? network_hopper_unknown_00193470(hopper) : FALSE;
}

/* 0x0019a0e6: whether it is of the first or third kind */
boolean user_interface_networking_hopper_unknown_00193490(
	void)
{
	struct network_hopper *hopper= user_interface_networking_hopper();

	return hopper ? network_hopper_unknown_00193490(hopper) : FALSE;
}

/* 0x0019a107: whether it is of the fifth kind */
boolean user_interface_networking_hopper_is_type5(
	void)
{
	struct network_hopper *hopper= user_interface_networking_hopper();

	return hopper ? hopper->type==_network_hopper_type5 : FALSE;
}

/* 0x0019a127: whether it is of the third, fourth or fifth kind */
boolean user_interface_networking_hopper_unknown_001934b0(
	void)
{
	struct network_hopper *hopper= user_interface_networking_hopper();

	return hopper ? network_hopper_unknown_001934b0(hopper) : FALSE;
}

/* 0x0019a250: whether, with a game in the network, the network game is
doing nothing, the first, third or fourth thing, or more than the eighth.
FALSE with no game in the network */
boolean user_interface_networking_unknown_0019a250(
	void)
{
	long state;

	if (!unknown_004d8ba0)
	{
		return FALSE;
	}
	state= network_life_cycle_get_state();

	return state<=_network_life_cycle_state1 || state==_network_life_cycle_state3 ||
		state==_network_life_cycle_state4 || state>_network_life_cycle_state8;
}

/* ---------- the two sessions of the network game, for the interface */

/* 0x000643a0 (network_life_cycle.c) */
boolean network_life_cycle_set_unknown49c4(void);

enum
{
	/* a number of who is in a session, after two others */
	USER_INTERFACE_NETWORKING_MEMBERSHIP_UNKNOWN8_OFFSET= 0x8
};

/* 0x00199dc9: network_life_cycle_unknown_000594a0 with three numbers, if what
the network game is doing is the fourth or the sixth for the interface;
FALSE if not */
boolean __attribute__((stdcall)) user_interface_networking_unknown_00199dc9(
	long unknown0,
	long unknown4,
	long unknown8)
{
	if (user_interface_networking_unknown_00199d7c()==4 || user_interface_networking_unknown_00199d7c()==6)
	{
		return network_life_cycle_unknown_000594a0(unknown0, unknown4, unknown8);
	}

	return FALSE;
}

/* 0x00199ebc: how many players the session of the network game has; 0 if
there is none or it is not established */
long user_interface_networking_get_player_count(
	void)
{
	byte *session= NULL;
	long player_count= 0;

	if (network_life_cycle_get_session(&session) && network_session_established(session))
	{
		long local_peer;
		long host_peer;
		const byte *membership= network_session_get_membership(&local_peer, session, &host_peer);

		player_count= *(const long *)(membership+NETWORK_SESSION_MEMBERSHIP_PLAYER_COUNT_OFFSET);
	}

	return player_count;
}

/* 0x00199ef8: the same, of the other session of the network game */
long user_interface_networking_get_other_player_count(
	void)
{
	byte *session= NULL;
	long player_count= 0;

	if (network_life_cycle_get_other_session(&session) && network_session_established(session))
	{
		long local_peer;
		long host_peer;
		const byte *membership= network_session_get_membership(&local_peer, session, &host_peer);

		player_count= *(const long *)(membership+NETWORK_SESSION_MEMBERSHIP_PLAYER_COUNT_OFFSET);
	}

	return player_count;
}

/* 0x00199f34: the number at 8 of who is in the session of the network game;
0 if there is none or it is not established */
long user_interface_networking_get_membership_unknown8(
	void)
{
	byte *session= NULL;
	long unknown8= 0;

	if (network_life_cycle_get_session(&session) && network_session_established(session))
	{
		long local_peer;
		long host_peer;
		const byte *membership= network_session_get_membership(&local_peer, session, &host_peer);

		unknown8= *(const long *)(membership+USER_INTERFACE_NETWORKING_MEMBERSHIP_UNKNOWN8_OFFSET);
	}

	return unknown8;
}

/* 0x0019a1bd: network_life_cycle_set_unknown49c4, if this console runs the
network game; FALSE if not */
boolean user_interface_networking_set_unknown49c4(
	void)
{
	return network_life_cycle_local_peer_is_host() ? network_life_cycle_set_unknown49c4() : FALSE;
}

/* ---------- more that the interface asks of the network game */

/* 0x00064970 (network_life_cycle.c) */
boolean __attribute__((stdcall)) network_life_cycle_get_unknown4d08(long *unknown4d08, long *unknown4d0c,
	const byte **unknown4d10);

/* 0x0019a203: whether, with a game in the network that this console runs,
the network game can go on: it cannot in its first, third, sixth, seventh
and eighth states, and in its ninth only if
user_interface_networking_hopper_unknown_00193470 says no. FALSE with no
game in the network, or if this console does not run it */
boolean user_interface_networking_unknown_0019a203(
	void)
{
	if (!unknown_004d8ba0 || !network_life_cycle_local_peer_is_host())
	{
		return FALSE;
	}

	switch (network_life_cycle_get_state())
	{
	case _network_life_cycle_state_none:
	case _network_life_cycle_state2:
	case _network_life_cycle_state5:
	case _network_life_cycle_state6:
	case _network_life_cycle_state7:
		return FALSE;
	case _network_life_cycle_state8:
		return !user_interface_networking_hopper_unknown_00193470();
	default:
		return TRUE;
	}
}

/* 0x0019a84e: two of the three numbers of network_life_cycle_get_unknown4d08;
the third is given a place of its own and is not kept */
boolean __attribute__((stdcall)) user_interface_networking_get_unknown4d08(
	long *unknown4d08,
	long *unknown4d0c)
{
	const byte *unknown4d10;

	return network_life_cycle_get_unknown4d08(unknown4d08, unknown4d0c, &unknown4d10);
}

/* 0x0019a951: whether the session of the network game has a player in the
place of the low sixteen bits of a number; FALSE for NONE, a place of 16 or
more, or no established session */
boolean user_interface_networking_has_player(
	long player_index)
{
	byte *session= NULL;
	long local_peer;
	long host_peer;
	const byte *membership;
	unsigned long players;
	long index;

	if (player_index==NONE)
	{
		return FALSE;
	}
	index= player_index&0xFFFF;
	if (!network_life_cycle_get_session(&session) || !network_session_established(session))
	{
		return FALSE;
	}
	membership= network_session_get_membership(&local_peer, session, &host_peer);
	players= *(const unsigned long *)(membership+NETWORK_SESSION_MEMBERSHIP_PLAYERS_OFFSET);

	return index<MAXIMUM_NETWORK_SESSION_PLAYERS && (players&(1u<<index))!=0;
}

/* 0x0019a9b4: the same, of the other session of the network game */
boolean user_interface_networking_other_has_player(
	long player_index)
{
	byte *session= NULL;
	long local_peer;
	long host_peer;
	const byte *membership;
	unsigned long players;
	long index;

	if (player_index==NONE)
	{
		return FALSE;
	}
	index= player_index&0xFFFF;
	if (!network_life_cycle_get_other_session(&session) || !network_session_established(session))
	{
		return FALSE;
	}
	membership= network_session_get_membership(&local_peer, session, &host_peer);
	players= *(const unsigned long *)(membership+NETWORK_SESSION_MEMBERSHIP_PLAYERS_OFFSET);

	return index<MAXIMUM_NETWORK_SESSION_PLAYERS && (players&(1u<<index))!=0;
}

/* 0x0019ad39: the place of the first player of who is in a session that is
known by the twelve bytes that are given (network_player_identifier_equal,
with its flags); NONE if there is none, if the session or the bytes are
NULL, or if the session is not established */
long __attribute__((stdcall)) user_interface_networking_find_player(
	const byte *session,
	const unsigned long *identifier)
{
	long found= NONE;

	if (session && identifier && network_session_established(session))
	{
		const byte *membership= session+NETWORK_SESSION_MEMBERSHIP_OFFSET;
		unsigned long players= *(const unsigned long *)(membership+NETWORK_SESSION_MEMBERSHIP_PLAYERS_OFFSET);
		const byte *player= membership+NETWORK_SESSION_MEMBERSHIP_PLAYER_ARRAY_OFFSET;
		long index;

		for (index= 0; index<MAXIMUM_NETWORK_SESSION_PLAYERS; index++, player+= NETWORK_SESSION_PLAYER_SIZE)
		{
			if ((players&(1u<<index)) && network_player_identifier_equal((const unsigned long *)player, identifier, TRUE))
			{
				found= index;
				break;
			}
		}
	}

	return found;
}

/* ---------- what the interface makes of its numbers of the network game */

/* 0x00199951: whether a number is 1, 3 or 5 */
boolean user_interface_networking_unknown_00199951(
	long unknown)
{
	return unknown==1 || unknown==3 || unknown==5;
}

/* 0x00199971: whether what the network game is at, for the interface, is 6 */
boolean user_interface_networking_unknown_00199971(
	void)
{
	return unknown_0019989d()==6;
}

/* 0x0019997f: whether a number is 0, 2 or 4 */
boolean user_interface_networking_unknown_0019997f(
	long unknown)
{
	return unknown==0 || unknown==2 || unknown==4;
}

/* 0x0019999e: whether a number is 0 or 1 */
boolean __attribute__((stdcall)) user_interface_networking_unknown_0019999e(
	long unknown)
{
	return unknown==0 || unknown==1;
}

/* 0x001999e3: whether a number is 4, 5 or 6 */
boolean user_interface_networking_unknown_001999e3(
	long unknown)
{
	return unknown==4 || unknown==5 || unknown==6;
}
