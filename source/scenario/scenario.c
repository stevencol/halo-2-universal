/*
SCENARIO.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809).
*/

#include "scenario.h"

/* ---------- constants */

#define NONE -1

/* ---------- globals */

/* 0x004686c0: the tag index of the scenario; NONE if there is none */
long global_scenario_index= NONE;

/* 0x004686c4: which structure bsp of the scenario is loaded. The name is
that of Halo 1's variable */
short global_structure_bsp_index;

/* 0x004e0350: the scenario tag */
struct scenario *global_scenario;

/* 0x004e034c: the globals tag of the cache file */
struct game_globals *global_game_globals;
