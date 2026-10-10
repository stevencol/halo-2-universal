/*
GAME.H

What the game keeps of the game that is being played. Only what the code
decompiled so far needs: the rest is not understood yet.
*/

#ifndef __GAME_H
#define __GAME_H

#include "cseries/cseries.h"
#include "text/unicode.h"

/* ---------- structures */

/* what a game is started with: the map, the difficulty, the players... */
enum
{
	MAXIMUM_GAME_MACHINES= 16,
	MAXIMUM_GAME_PLAYERS= 16,
	GAME_MACHINE_IDENTIFIER_SIZE= 6,
	GAME_PLAYER_IDENTIFIER_SIZE= 12,
	GAME_PLAYER_NAME_LENGTH= 31,
	GAME_MAP_NAME_LENGTH= 0x104
};

/* a player of a game that starts */
struct game_player_options
{
	boolean valid;
	/* set in a player that has no console: its console's identifier has to
	be all zeros */
	boolean unknown1;
	/* from 0 to 3 */
	short controller_index;
	/* game_options_setup_local_players gives the local players these from
	0 to 3, the ones of the four entries of unknown_004e61cc that are set
	first, and NONE to those it has none for */
	long unknown4;
	/* of the console that the player is at */
	byte machine_identifier[GAME_MACHINE_IDENTIFIER_SIZE];
	/* not all zeros, and of no other player */
	byte identifier[GAME_PLAYER_IDENTIFIER_SIZE];
	byte unknown1A[2];
	wchar name[GAME_PLAYER_NAME_LENGTH];
	short unknown5A;
	byte unknown5C[0x10];
	byte unknown6C[0x2C];
	/* in multiplayer, the index of the local player; 1 in the campaign */
	byte unknown98;
	byte unknown99[0x4B];
};

enum
{
	GAME_VARIANT_NAME_LENGTH= 31
};

/* the rules of a multiplayer game. Only what the code here uses; the
values that the default variant has are told */
struct game_variant
{
	byte unknown0[3];
	/* 0xFF */
	byte unknown3;
	wchar name[GAME_VARIANT_NAME_LENGTH+1];
	/* 2 */
	long unknown44;
	/* 0xF9A */
	unsigned long unknown48;
	long unknown4C;
	/* 25 */
	long unknown50;
	/* 480 */
	long unknown54;
	long unknown58;
	byte unknown5C[0x18];
	/* 16 and 16 */
	long unknown74;
	long unknown78;
	long unknown7C;
	/* 5 and 5 */
	long unknown80;
	long unknown84;
	long unknown88;
	byte unknown8C[0x18];
	long unknownA4;
	/* 2 */
	long unknownA8;
	/* 10 */
	long unknownAC;
	long unknownB0;
	byte unknownB4[0x18];
	byte unknownCC[12];
	byte unknownD8[0x18];
	/* 2 */
	unsigned long unknownF0;
	byte unknownF4[0x3C];
};

typedef char verify_game_variant_size[
	sizeof(struct game_variant) == 0x130 ? 1 : -1];
typedef char verify_game_variant_unknownF0_offset[
	offsetof(struct game_variant, unknownF0) == 0xF0 ? 1 : -1];

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
	/* the path of the scenario */
	char map_name[GAME_MAP_NAME_LENGTH];
	/* from 0 to 15 */
	short unknown120;
	byte pad122[2];
	boolean unknown124;
	byte unknown125[3];
	boolean unknown128;
	byte unknown129;
	short difficulty;
	/* a campaign of two players. It is not written to a saved game */
	boolean coop;
	byte unknown12D[7];
	struct game_variant variant;
	byte unknown264[4];
	/* bit n: the console n of the table is in the game */
	unsigned long machine_valid_mask;
	byte machine_identifiers[MAXIMUM_GAME_MACHINES][GAME_MACHINE_IDENTIFIER_SIZE];
	/* this console is in the game, and which it is */
	boolean local_machine_exists;
	byte local_machine_identifier[GAME_MACHINE_IDENTIFIER_SIZE];
	byte pad2D3;
	struct game_player_options players[MAXIMUM_GAME_PLAYERS];
	byte unknown1114[4];
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
	_game_mode_none,
	_game_mode_campaign,
	_game_mode_multiplayer,
	_game_mode_3,
	_game_mode_4,
	_game_mode_5,
	NUMBER_OF_GAME_MODES,
	NUMBER_OF_GAME_SIMULATIONS= 6,
	NUMBER_OF_GAME_DIFFICULTIES= 4,
	MAXIMUM_GAME_TICK_RATE= 300,
	MAXIMUM_CONTROLLERS= 4,

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

typedef char verify_game_player_options_size[
	sizeof(struct game_player_options) == 0xE4 ? 1 : -1];
typedef char verify_game_player_options_unknown98_offset[
	offsetof(struct game_player_options, unknown98) == 0x98 ? 1 : -1];
typedef char verify_game_options_size[
	sizeof(struct game_options) == 0x1118 ? 1 : -1];
typedef char verify_main_game_globals_active_offset[
	offsetof(struct main_game_globals, active) == 0x1120 ? 1 : -1];
typedef char verify_main_game_globals_size[
	sizeof(struct main_game_globals) == 0x1200 ? 1 : -1];
typedef char verify_game_options_difficulty_offset[
	offsetof(struct game_options, difficulty) == 0x12A ? 1 : -1];
typedef char verify_game_options_variant_offset[
	offsetof(struct game_options, variant) == 0x134 ? 1 : -1];
typedef char verify_game_options_machine_valid_mask_offset[
	offsetof(struct game_options, machine_valid_mask) == 0x268 ? 1 : -1];
typedef char verify_game_options_players_offset[
	offsetof(struct game_options, players) == 0x2D4 ? 1 : -1];
typedef char verify_game_player_options_size[
	sizeof(struct game_player_options) == 0xE4 ? 1 : -1];
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

void __attribute__((stdcall)) game_frame(float seconds);
void game_options_setup_local_players(long count, struct game_options *options);
int game_options_controller_compare(const void *a, const void *b);

void game_create_players(void);
void game_won(void);

void game_initialize(void);
void game_dispose(void);
void game_initialize_for_new_map(const struct game_options *options);
void game_options_new(const struct game_options *options);
void game_dispose_from_old_map(void);
void game_initialize_for_new_structure_bsp(void);
void game_dispose_from_old_structure_bsp(void);
void game_options_new_default(struct game_options *options);
boolean game_options_verify(const struct game_options *options);
boolean game_options_match(const struct game_options *options1, const struct game_options *options2);
void game_run_ticks(long tick_count, float *seconds);
boolean game_in_progress(void);
long game_is_coop(void);
boolean game_is_distributed(void);
boolean game_is_authoritative(void);
boolean game_is_predicted(void);
long game_unknown_001388a0(void);
void game_lost(boolean lost);
void game_finish(void);
long game_finished_wait_over(void);
void game_tick(void);

/* ---------- prototypes of functions not decompiled yet */

/* the game's functions take their arguments off the stack; the tests stand
in for these, and take the arguments off for them */
#define GAME_STDCALL


/* ---------- globals */

extern struct main_game_globals *main_game_globals;
extern struct game_time_globals *game_time_globals;
extern struct game_system game_systems[NUMBER_OF_GAME_SYSTEMS];
extern unsigned long *global_random_seed_reference;

#endif
