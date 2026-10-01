/*
GAME.H

What the game keeps of the game that is being played. Only what the code
decompiled so far needs: the rest is not understood yet.
*/

#ifndef __GAME_H
#define __GAME_H

#include "cseries/cseries.h"

/* ---------- structures */

/* what a game is started with: the map, the difficulty, the players... */
struct game_options
{
	byte unknown0[4];
	/* set in the options of a state of the game that is gone back to */
	boolean reverted;
	byte unknown5[0x1113];
};

/* 0x004e6948 points to it. Not the tag of the same name in Halo 1, which is
struct game_globals here */
struct main_game_globals
{
	byte unknown0[8];
	struct game_options options;
	/* a map is loaded and a game goes on */
	boolean active;
	/* the game is lost, and will go back to the last checkpoint */
	boolean lost;
	byte unknown1122[2];
	long unknown1124;
};

typedef char verify_game_options_size[
	sizeof(struct game_options) == 0x1118 ? 1 : -1];
typedef char verify_main_game_globals_active_offset[
	offsetof(struct main_game_globals, active) == 0x1120 ? 1 : -1];

/* 0x00510c54 points to it */
struct game_time_globals
{
	boolean initialized;
	boolean paused;
	short ticks_per_second;
	float seconds_per_tick;
	/* in ticks */
	unsigned long time;
};

/* ---------- prototypes of functions not decompiled yet */

/* the game's functions take their arguments off the stack; the tests stand
in for these, and take the arguments off for them */
#define GAME_STDCALL

/* 0x001384a0: whether two games are the same game. The name is a guess */
boolean GAME_STDCALL game_options_match(const struct game_options *options1, const struct game_options *options2);

/* 0x00138180: whether the options are of a game that can be played. The name
is a guess */
boolean GAME_STDCALL game_options_verify(const struct game_options *options);

/* 0x0011c1b0: makes a structure bsp of the scenario the one that is loaded;
whether it is. The name is a guess */
boolean GAME_STDCALL scenario_switch_structure_bsp(long structure_bsp_index, long unknown);

/* ---------- globals */

extern struct main_game_globals *main_game_globals;
extern struct game_time_globals *game_time_globals;
extern boolean main_reset_map_requested;

#endif
