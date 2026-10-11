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
