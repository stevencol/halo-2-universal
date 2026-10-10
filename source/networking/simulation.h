/*
SIMULATION.H

The game in the network, as the rest of the game sees it: what
unknown_004cf770 has going (simulation_world.h) and its views
(simulation_view.h). The name of the file is a guess, and so are the names of
its functions.
*/

#ifndef __SIMULATION_H
#define __SIMULATION_H

#include "cseries/cseries.h"

/* ---------- constants */

enum
{
	/* of the thing that unknown_004cf770 has going: the six bytes that this
	console is known by */
	SIMULATION_WORLD_OWN_UNKNOWN14_OFFSET= 0xD,
	/* the size of one of the things that come and wait */
	SIMULATION_UPDATE_DECODED_SIZE= 0x4048
};

/* ---------- prototypes/SIMULATION.C */

void __attribute__((stdcall)) simulation_view_disconnected(byte *view);
boolean __attribute__((stdcall)) simulation_update_encode(void *buffer, long capacity, const void *update,
	long *size_out);
boolean simulation_update_decode(const void *data, long size, void *update);
boolean simulation_machine_established(const word *unknown14);
void simulation_fail(void);
long simulation_unknown_000687e0(void);

#endif
