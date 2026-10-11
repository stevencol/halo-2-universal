/*
NETWORK_GLOBALS.H

What the game does of the network each time around, and where it keeps the
things of the network that it has. The name of the file is a guess, and so
are the names of its functions.
*/

#ifndef __NETWORK_GLOBALS_H
#define __NETWORK_GLOBALS_H

#include "cseries/cseries.h"

/* ---------- constants */

enum
{
	/* how many sessions the network has */
	NUMBER_OF_NETWORK_SESSIONS= 3,
	/* two more states of the game in the network */
	_simulation_world_state4= 4,
	_simulation_world_state5= 5
};

/* ---------- prototypes/NETWORK_GLOBALS.C */

void network_update(void);
void network_send(void);
void __attribute__((stdcall)) network_open_sockets(long unknown);
void __attribute__((stdcall)) network_close_sockets(long unknown);

/* ---------- globals */

extern byte *unknown_00510568;
extern byte *unknown_00510570;
extern byte *unknown_00510574;

#endif
