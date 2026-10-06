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
	/* shows that the game was saved; what else it does is not known */
	boolean unknown4E;
	byte pad4F;
	short switch_structure_bsp_index;
	byte unknown52[2];
};

typedef char verify_main_globals_reset_map_offset[
	offsetof(struct main_globals, reset_map) == 0x46 ? 1 : -1];
typedef char verify_main_globals_switch_structure_bsp_index_offset[
	offsetof(struct main_globals, switch_structure_bsp_index) == 0x50 ? 1 : -1];

/* how long the last frames took, to tell how the game runs */
struct main_frame_time_samples
{
	short next_index;
	short samples[NUMBER_OF_MAIN_FRAME_TIME_SAMPLES];
};

/* 0x004e6388: the time of the main loop. It is counted in vertical blanks
of the picture, which main_vblank_count counts */
struct main_time_globals
{
	boolean unknown0;
	boolean unknown1;
	byte pad2[2];
	/* the game's time, in ticks, at the last frame; 0 with no game */
	long game_time;
	/* the console's tick count at the last frame, in 64 bits */
	unsigned long tick_count;
	unsigned long tick_count_high;
	/* when the last frame was */
	long long last_vblank;
	/* main_vblank_count at the last frame */
	long long vblank_count;
	byte unknown20[8];
	/* a frame is not counted as sooner than this */
	long long target_vblank;
	/* how many vertical blanks a frame is given */
	short frame_vblanks;
	short unknown32;
	byte unknown34[0x44];
	struct main_frame_time_samples frame_time_samples;
};

typedef char verify_main_time_globals_size[
	sizeof(struct main_time_globals) == 0x98 ? 1 : -1];
typedef char verify_main_time_globals_target_vblank_offset[
	offsetof(struct main_time_globals, target_vblank) == 0x28 ? 1 : -1];
typedef char verify_main_time_globals_frame_time_samples_offset[
	offsetof(struct main_time_globals, frame_time_samples) == 0x78 ? 1 : -1];

/* 0x004e8c20 points to it: the players. Only what the code uses */
enum
{
	MAXIMUM_LOCAL_PLAYERS= 4
};

struct players_globals
{
	byte unknown0[0xC];
	/* the players of this console; NONE where there is none */
	long local_players[MAXIMUM_LOCAL_PLAYERS];
	/* the player of each controller; NONE where there is none */
	long unknown1C[MAXIMUM_LOCAL_PLAYERS];
};

/* ---------- prototypes/MAIN.C */

void main_files_damaged(void);
void main_print_saving_message(void);

void main_loop(void);
void main_loop_body(void);
void main_time_initialize(void);
float main_time_update(void);
void main_revert_map_private(void);
void main_save_map_private(void);
void main_save_map(void);
void main_switch_structure_bsp(short structure_bsp_index);
void main_switch_structure_bsp_private(void);
void main_unknown4E_private(void);
void main_reset_map_private(void);
void main_revert_map_if_possible_private(void);
boolean main_action_pending(void);
void main_cancel_map_copy_and_wait(void);

/* how the frames are shown: 0x000128c0 sets them. What the others are is
not known */
struct display_settings
{
	byte unknown0;
	byte unknown1;
	/* how many vertical blanks to give a frame; 0 for no limit */
	short frame_vblanks;
	long unknown4;
	long unknown8;
};

/* ---------- globals */

extern boolean unknown_004ed39d;
extern boolean unknown_004ed39e;
extern long unknown_004ed3a0;

extern struct main_globals main_globals;
extern struct main_time_globals main_time_globals;
extern struct players_globals *players_globals;
extern struct data_array *player_data;
extern long long main_vblank_count;
extern struct display_settings display_settings;
extern const struct display_settings display_settings_default;

/* ---------- inline code */

/* the time that went by since the last frame is not counted: after what
takes long, as writing a checkpoint. The game has no function for it */
static inline void main_time_skip(
	void)
{
	main_time_globals.last_vblank= main_vblank_count;
}

#endif
