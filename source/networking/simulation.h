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
	SIMULATION_UPDATE_DECODED_SIZE= 0x4048,
	/* more of the thing: a byte that says that the state of the game was
	saved while the thing was not the kind that takes what comes. Of one of
	the things that come: its number first; what the players do, with a
	number before it; something more, with a number before it; and at the
	end the time of the game and its seed, to see that the game is the
	same */
	SIMULATION_WORLD_SAVED_OFFSET= 0x2E,
	SIMULATION_UPDATE_UNKNOWN8_OFFSET= 0x8,
	SIMULATION_UPDATE_PLAYERS_OFFSET= 0xC,
	SIMULATION_UPDATE_UNKNOWN5CC_OFFSET= 0x5CC,
	SIMULATION_UPDATE_UNKNOWN610_OFFSET= 0x610,
	SIMULATION_UPDATE_GAME_TIME_OFFSET= 0x4040,
	SIMULATION_UPDATE_RANDOM_SEED_OFFSET= 0x4044
};

/* ---------- prototypes/SIMULATION.C */

void __attribute__((stdcall)) simulation_view_disconnected(byte *view);
boolean __attribute__((stdcall)) simulation_update_encode(void *buffer, long capacity, const void *update,
	long *size_out);
boolean simulation_update_decode(const void *data, long size, void *update);
boolean simulation_machine_established(const word *unknown14);
void simulation_fail(void);
long simulation_unknown_000687e0(void);
long simulation_updates_waiting(boolean *counted);
boolean simulation_in_progress(void);
boolean simulation_starting(void);
long simulation_failed(void);
void simulation_build_update(long *update);
void simulation_apply_update(const long *update);
void simulation_update(void);

#endif
