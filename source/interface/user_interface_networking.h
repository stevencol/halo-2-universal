/*
USER_INTERFACE_NETWORKING.H

What the interface asks of the network game. Worked out from halo2ship.exe;
the name of the file is a guess, and so are the names of its functions.
*/

#ifndef __USER_INTERFACE_NETWORKING_H
#define __USER_INTERFACE_NETWORKING_H

#include "cseries/cseries.h"

/* ---------- constants */

enum
{
	/* how many players a network game can have */
	USER_INTERFACE_NETWORKING_MAXIMUM_PLAYERS= 16
};

/* ---------- prototypes/USER_INTERFACE_NETWORKING.C */

boolean __attribute__((stdcall)) user_interface_networking_unknown_001999bf(long unknown);
boolean user_interface_networking_unknown_001999d7(void);
void __attribute__((stdcall)) user_interface_networking_unknown_00199e2e(boolean unknown);
boolean user_interface_networking_unknown_0019a015(void);
long user_interface_networking_unknown_00199fd6(void);

#endif
