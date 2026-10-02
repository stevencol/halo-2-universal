/*
SCENARIO.H

The scenario that is loaded. As in Halo 1.
*/

#ifndef __SCENARIO_H
#define __SCENARIO_H

#include "cseries/cseries.h"

/* ---------- prototypes/SCENARIO.C */

boolean scenario_switch_structure_bsp(long structure_bsp_index, long unload_old);

/* ---------- globals */

struct scenario;
struct game_globals;

extern long global_scenario_index;
extern short global_structure_bsp_index;
extern struct scenario *global_scenario;
extern struct game_globals *global_game_globals;
extern void *global_structure_bsp;
extern void *global_structure_lightmap;
extern void *global_collision_bsp;
extern void *global_bsp3d;

#endif
