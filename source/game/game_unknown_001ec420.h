/*
GAME_UNKNOWN_001EC420.H

a part of the game that keeps, in the state of the game, eight numbers that are nothing for a new map, and a byte that says that a
map is loaded.
Worked out from halo2ship.exe; what it is for is not known, and the name of
the file is only the address of its first function.
*/

#ifndef __GAME_UNKNOWN_001EC420_H
#define __GAME_UNKNOWN_001EC420_H

#include "cseries/cseries.h"

/* ---------- constants */

enum
{
	GAME_UNKNOWN_001EC420_COUNT= 8
};

/* ---------- structures */

struct game_unknown_001ec420_globals
{
	long unknown0[GAME_UNKNOWN_001EC420_COUNT];
	boolean map_initialized;
	byte pad[3];
};

typedef char verify_game_unknown_001ec420_globals_size[
	sizeof(struct game_unknown_001ec420_globals) == 0x24 ? 1 : -1];

/* ---------- globals */

extern struct game_unknown_001ec420_globals *game_unknown_001ec420_globals;

/* ---------- prototypes/GAME_UNKNOWN_001EC420.C */

void game_unknown_001ec420_initialize(void);
void game_unknown_001ec420_dispose(void);
void game_unknown_001ec420_initialize_for_new_map(void);
void game_unknown_001ec420_dispose_from_old_map(void);

#endif
