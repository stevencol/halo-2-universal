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
	/* how the game is run among consoles: _game_simulation_local on one
	console. A state of the game that is gone back to gets that one */
	char simulation;
	byte unknown5;
	/* half the refresh rate of the picture */
	short tick_rate;
	/* eight bytes made at random for each game */
	byte game_instance[8];
	unsigned long random_seed;
	long unknown14;
	long unknown18;
	byte unknown1C[0x108];
	boolean unknown124;
	byte unknown125[3];
	boolean unknown128;
	byte unknown129;
	short difficulty;
	/* a campaign with this set is not written to a saved game, at least */
	boolean unknown12C;
	byte unknown12D[7];
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
	/* ticks until the game goes back to the checkpoint */
	long lost_ticks;
	/* the game is over, and will end in so many ticks */
	boolean finished;
	boolean unknown1129;
	byte unknown112A[2];
	long finished_ticks;
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

/* the values of the simulation of a game. The names are guesses, by what
game_is_distributed, game_is_authoritative and game_is_predicted tell */
enum
{
	_game_simulation_none,
	_game_simulation_local,
	_game_simulation_distributed_first,
	_game_simulation_predicted= 4,
	_game_simulation_distributed_last= 5
};

enum
{
	_game_mode_campaign= 1,
	_game_mode_multiplayer= 2,

	_game_difficulty_normal= 1,
	_game_difficulty_legendary= 3,

	GAME_DEFAULT_RANDOM_SEED= 0x78A8,
	GAME_DEFAULT_REFRESH_RATE= 60
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
typedef char verify_game_options_difficulty_offset[
	offsetof(struct game_options, difficulty) == 0x12A ? 1 : -1];
typedef char verify_game_options_unknown134_offset[
	offsetof(struct game_options, unknown134) == 0x134 ? 1 : -1];
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
void game_options_new_default(struct game_options *options);
void game_run_ticks(long tick_count, float *seconds);
boolean game_in_progress(void);
long game_is_campaign_unknown12C(void);
boolean game_is_distributed(void);
boolean game_is_authoritative(void);
boolean game_is_predicted(void);
long game_unknown_001388a0(void);
void game_lost(boolean lost);
void game_finish(void);
long game_finished_wait_over(void);

/* ---------- prototypes of functions not decompiled yet */

/* the game's functions take their arguments off the stack; the tests stand
in for these, and take the arguments off for them */
#define GAME_STDCALL

/* 0x001384a0: whether two games are the same game. The name is a guess */
boolean GAME_STDCALL game_options_match(const struct game_options *options1, const struct game_options *options2);

/* 0x00138180: whether the options are of a game that can be played. The name
is a guess */
boolean GAME_STDCALL game_options_verify(const struct game_options *options);

/* ---------- globals */

extern struct main_game_globals *main_game_globals;
extern struct game_time_globals *game_time_globals;
extern struct game_system game_systems[NUMBER_OF_GAME_SYSTEMS];
extern unsigned long *global_random_seed_reference;

#endif
