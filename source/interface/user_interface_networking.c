/*
USER_INTERFACE_NETWORKING.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

What the interface asks of the network game. The names are guesses.
*/

#include "user_interface_networking.h"

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
