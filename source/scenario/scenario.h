/*
SCENARIO.H

The scenario that is loaded. As in Halo 1.
*/

#ifndef __SCENARIO_H
#define __SCENARIO_H

/* ---------- globals */

struct scenario;
struct game_globals;

extern long global_scenario_index;
extern short global_structure_bsp_index;
extern struct scenario *global_scenario;
extern struct game_globals *global_game_globals;

#endif
