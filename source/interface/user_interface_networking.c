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
