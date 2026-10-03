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
/* the globals of the game: a tag. Only what the code here uses */
struct game_globals
{
	byte unknown0[0x16C];
	/* the index of a tag that starts with a block whose first element has,
	at 0x1C, the index of a list of the texts of the multiplayer games;
	NONE if there is none */
	long unknown16C;
};

extern long global_scenario_index;
extern short global_structure_bsp_index;
extern struct scenario *global_scenario;
extern struct game_globals *global_game_globals;
extern void *global_structure_bsp;
extern void *global_structure_lightmap;
extern void *global_collision_bsp;
extern void *global_bsp3d;

#endif
