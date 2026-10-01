/*
MAIN.H

The game's main loop, and what the game asks it to do between two frames.
After Halo 1's main.c; the fields with a name were worked out from the
functions that use them.
*/

#ifndef __MAIN_H
#define __MAIN_H

#include "cseries/cseries.h"

/* ---------- constants */

enum
{
	MAIN_CORE_NAME_LENGTH= 64,
	NUMBER_OF_MAIN_FRAME_TIME_SAMPLES= 15
};

/* ---------- structures */

/* 0x00547f28 */
struct main_globals
{
	/* set when the loop starts */
	boolean unknown0;
	boolean unknown1;
	boolean unknown2;
	/* ends the loop */
	boolean exit_game;
	boolean unknown4;
	/* what the game asks for, done at the start of a frame */
	boolean load_core;
	char core_name[MAIN_CORE_NAME_LENGTH];
	boolean reset_map;
	boolean revert_map;
	boolean save_map;
	boolean unknown49;
	/* go back to the last checkpoint, if that can be done */
	boolean revert_map_if_possible;
	boolean switch_structure_bsp;
	/* the two flags that game_state_revert gets */
	boolean revert_flag0;
	boolean revert_flag1;
	boolean unknown4E;
	byte pad4F;
	short switch_structure_bsp_index;
	byte unknown52[2];
};

typedef char verify_main_globals_reset_map_offset[
	offsetof(struct main_globals, reset_map) == 0x46 ? 1 : -1];
typedef char verify_main_globals_switch_structure_bsp_index_offset[
	offsetof(struct main_globals, switch_structure_bsp_index) == 0x50 ? 1 : -1];

/* 0x004e6400: how long the last frames took, to tell how the game runs */
struct main_frame_time_samples
{
	short next_index;
	short samples[NUMBER_OF_MAIN_FRAME_TIME_SAMPLES];
};

/* ---------- prototypes/MAIN.C */

void main_loop(void);
void main_time_initialize(void);
void main_revert_map_private(void);
void main_reset_map_private(void);
void main_revert_map_if_possible_private(void);
boolean main_action_pending(void);
void main_cancel_map_copy_and_wait(void);

/* ---------- globals */

extern struct main_globals main_globals;

#endif
