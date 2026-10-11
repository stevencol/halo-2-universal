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
	SIMULATION_UPDATE_RANDOM_SEED_OFFSET= 0x4044,
	/* more of one of the things that come: whether the game in the network
	is being played; whether what unknown_004cf780 has changed, and the
	change; some things that happened, and how many; and whether the state
	of the game was saved. Of what unknown_004cf780 has: a byte that says
	that it changed, a number, and 0x60 bytes */
	SIMULATION_UPDATE_IN_PROGRESS_OFFSET= 0x4,
	SIMULATION_UPDATE_CHANGED_OFFSET= 0xDD0,
	SIMULATION_UPDATE_CHANGE_OFFSET= 0xDD4,
	SIMULATION_UPDATE_EVENT_COUNT_OFFSET= 0xE38,
	SIMULATION_UPDATE_EVENTS_OFFSET= 0xE3C,
	SIMULATION_UPDATE_SAVED_OFFSET= 0x403C,
	SIMULATION_UNKNOWN_004CF780_UNKNOWN1C_OFFSET= 0x1C,
	SIMULATION_UNKNOWN_004CF780_UNKNOWN24_OFFSET= 0x24,
	SIMULATION_UNKNOWN_004CF780_UNKNOWN24_SIZE= 0x60,
	SIMULATION_UNKNOWN_004CF780_CHANGED_OFFSET= 0x84,
	/* and where it has the thing that is going, something that is given up
	when a map goes (a number at 0x80 of it is cleared) and a number */
	SIMULATION_UNKNOWN_004CF780_WORLD_OFFSET= 4,
	SIMULATION_UNKNOWN_004CF780_UNKNOWN8_OFFSET= 8,
	SIMULATION_UNKNOWN_004CF780_UNKNOWNC_OFFSET= 0xC,
	SIMULATION_UNKNOWN_004CF780_UNKNOWN8_UNKNOWN80_OFFSET= 0x80,
	/* the memory of the thing that is going and of what unknown_004cf780
	has, and how many kinds a table of kinds has room for */
	SIMULATION_WORLD_STORAGE_SIZE= 0x1220,
	SIMULATION_UNKNOWN_004CF780_STORAGE_SIZE= 0xC34,
	MAXIMUM_SIMULATION_TYPES_PER_TABLE= 32
};

/* ---------- structures */

/* the kinds of one sort of thing that the simulation tells of: how many,
and what describes each. There are two tables, one after the other */
struct simulation_type_table
{
	long count;
	void *types[MAXIMUM_SIMULATION_TYPES_PER_TABLE];
};

typedef char verify_simulation_type_table_size[
	sizeof(struct simulation_type_table) == 0x84 ? 1 : -1];

/* ---------- prototypes/SIMULATION.C */

void simulation_initialize(void);
void simulation_dispose(void);
void simulation_initialize_for_new_map(void);
void simulation_dispose_from_old_map(void);

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
void __attribute__((stdcall)) simulation_take_unknown_004cf780_change(long *change, boolean *changed);
byte *__attribute__((stdcall)) simulation_view_create(long kind, const word *unknown14, long unknown1c, long unused);

#endif
