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
	/* 1 for the campaign and 2 for multiplayer, at least */
	long game_mode;
	/* set in the options of a state of the game that is gone back to */
	boolean reverted;
	byte unknown5[0xB];
	unsigned long random_seed;
	byte unknown14[0x110];
	boolean unknown124;
	byte unknown125[0xF];
	/* what unknown_0019d650 sets up, for a multiplayer game */
	byte unknown134[0x44];
	long unknown178;
	byte unknown17C[0xF9C];
};

/* 0x004e6948 points to it. Not the tag of the same name in Halo 1, which is
struct game_globals here */
struct main_game_globals
{
	/* the systems are being set up for a new map */
	boolean initializing;
	/* the systems are set up for the map that is loaded */
	boolean map_initialized;
	/* the structure bsp that the systems are set up for; NONE if none */
	short structure_bsp_index;
	byte unknown4[4];
	struct game_options options;
	/* a map is loaded and a game goes on */
	boolean active;
	/* the game is lost, and will go back to the last checkpoint */
	boolean lost;
	byte unknown1122[2];
	long unknown1124;
	boolean unknown1128;
	byte unknown1129[7];
	long unknown1130;
	byte unknown1134[4];
	/* three sets of bits, cleared for a new structure bsp */
	unsigned long unknown1138[16];
	unsigned long unknown1178[16];
	unsigned long unknown11B8[16];
	byte unknown11F8[2];
	short unknown11FA;
	byte unknown11FC[4];
};

/* a part of the game, as the objects or the sound: what it does when the
game starts and ends, with a new map and with a new structure bsp. NULL
where it does nothing. The names of the last three are guesses */
#define GAME_SYSTEM_CALLBACK __attribute__((stdcall))
typedef void (*game_system_proc)(void);
typedef void (GAME_SYSTEM_CALLBACK *game_system_structure_bsp_proc)(long structure_bsp_index);
typedef void (GAME_SYSTEM_CALLBACK *game_system_activation_proc)(const unsigned long *old_bits,
	const unsigned long *new_bits, long bit_count);

struct game_system
{
	game_system_proc initialize;
	game_system_proc dispose;
	game_system_proc initialize_for_new_map;
	game_system_proc dispose_from_old_map;
	game_system_proc initialize_for_new_structure_bsp;
	game_system_proc dispose_from_old_structure_bsp;
	/* before the structure bsp of the scenario that is loaded changes */
	game_system_structure_bsp_proc prepare_for_new_structure_bsp;
	/* when the first and the third sets of bits of the game change */
	game_system_activation_proc unknown1C;
	game_system_activation_proc unknown20;
};

enum
{
	NUMBER_OF_GAME_SYSTEMS= 68
};

typedef char verify_game_options_size[
	sizeof(struct game_options) == 0x1118 ? 1 : -1];
typedef char verify_main_game_globals_active_offset[
	offsetof(struct main_game_globals, active) == 0x1120 ? 1 : -1];
typedef char verify_main_game_globals_size[
	sizeof(struct main_game_globals) == 0x1200 ? 1 : -1];
typedef char verify_game_options_unknown178_offset[
	offsetof(struct game_options, unknown178) == 0x178 ? 1 : -1];
typedef char verify_game_system_size[
	sizeof(struct game_system) == 9*sizeof(void *) ? 1 : -1];

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

/* ---------- prototypes/GAME.C */

void game_initialize(void);
void game_dispose(void);
void game_initialize_for_new_map(const struct game_options *options);
void game_options_new(const struct game_options *options);
void game_dispose_from_old_map(void);
void game_initialize_for_new_structure_bsp(void);
void game_dispose_from_old_structure_bsp(void);

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
extern struct game_system game_systems[NUMBER_OF_GAME_SYSTEMS];
extern unsigned long *global_random_seed_reference;

#endif
