/*
USER_INTERFACE_NETWORKING.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

What the interface asks of the network game. The names are guesses.
*/

#include "user_interface_networking.h"

#include "networking/network_life_cycle.h"

/* ---------- prototypes */

/* 0x0019989d: what the network game is at, as a number for the interface.
0x00199fd6: how many more players the network game can take, most likely */
long unknown_0019989d(void);
long unknown_00199fd6(void);

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
unknown_00199fd6 gives more than nothing */
boolean user_interface_networking_unknown_0019a015(
	void)
{
	return network_life_cycle_session_is_open() && unknown_00199fd6()>0;
}
