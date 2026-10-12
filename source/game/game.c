/*
GAME.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The game as a whole: a table of its parts, and what goes through the table
when the game starts and ends, with a new map and with a new structure bsp.
The names are those of Halo 1's game.c where it has them; there, each of
these functions calls the parts by name.
*/

#include "sound/sound_definitions.h"
#include "game.h"
#include "objects/object_early_movers.h"
#include "objects/object_unknown_0010a980.h"
#include "game/game_unknown_001ec420.h"

#include "networking/transport.h"
#include "networking/transport_security.h"

#include "input/input_abstraction.h"

#include "cache/animation_cache.h"
#include "main/main.h"
#include "main/main_game.h"
#include "saved games/game_state.h"
#include "shell/shell.h"
#include "scenario/scenario.h"

#include <stdlib.h>
#include <string.h>

/* ---------- prototypes of functions of the console's libraries */

/* 0x003212d6 */
unsigned int _control87(unsigned int value, unsigned int mask);

/* 0x003207ed: of the game's C library, with characters of 16 bits */
wchar *wcsncpy(wchar *destination, const wchar *source, size_t count);

/* ---------- prototypes of functions not decompiled yet */

/* 0x0019d650: sets up a part of the options of a multiplayer game */
void unknown_0019d650(void *unknown);

/* 0x0007ad80 is transport_security_random_bytes, of transport_security.c */

/* 0x0017bc40, 0x0017d690, 0x000d4db0, 0x0018c060, 0x002118c0, 0x00134e00,
0x00039e50 and 0x00176f60: what is updated by the frame, with its seconds
or not */
void __attribute__((stdcall)) unknown_0017bc40(real seconds);
void unknown_0017d690(void);
void __attribute__((stdcall)) game_unknown_000d4db0(real seconds);
void __attribute__((stdcall)) unknown_0018c060(real seconds);
void unknown_002118c0(void);
void __attribute__((stdcall)) unknown_00134e00(real seconds);
void __attribute__((stdcall)) unknown_00039e50(real seconds);
void __attribute__((stdcall)) unknown_00176f60(real seconds);

/* 0x00014980, 0x000128c0 and 0x00014600: the display is set up again with
some settings */
void unknown_00014980(void);
void unknown_000128c0(const struct display_settings *settings);
void unknown_00014600(void);

/* 0x000682e0, 0x000683a0, 0x0014d3a0, 0x00138db0 and 0x000684e0: what a
tick does with what it has on the stack, before and after the systems are
updated; the second says whether they are */
struct game_tick_state;
void simulation_build_update(struct game_tick_state *state);
void __attribute__((stdcall)) simulation_apply_update_changes(struct game_tick_state *state);
void __attribute__((stdcall)) unknown_0014d3a0(struct game_tick_state *state);
void unknown_00138db0(struct game_tick_state *state);
void simulation_apply_update(struct game_tick_state *state);

/* what is updated in a tick, in this order; animation_cache_idle is between
the twenty-first and the twenty-second */
void unknown_001c83c0(void);
void unknown_001fb510(void);
void unknown_0020b660(void);
void unknown_0015c150(void);
void unknown_00195ed0(void);
void unknown_00209310(void);
void unknown_00138c10(void);
void unknown_000b6f10(void);
void unknown_001c3d30(void);
void unknown_000b7060(void);
void unknown_000b7150(void);
void unknown_002268e0(void);
void unknown_00176e40(void);
void unknown_000c1400(void);
void unknown_0015ca10(void);
void unknown_0014d8b0(void);
void unknown_001df500(void);
void game_update_lost(void);
void game_update_finished(void);
void unknown_00226050(void);
void unknown_0013c680(void);
void unknown_00225730(void);
void unknown_00165dc1(void);
void unknown_00153950(void);
void unknown_001917b2(void);
void unknown_0016f200(void);

long lrintf(float value);


/* what the parts of the game do: their names are not known. Those of the
last three fields of a part take arguments, which are not given here */
void unknown_00012560(void);
void unknown_000141c0(void);
void unknown_00022590(void);
void unknown_000226a0(void);
void unknown_000226b0(void);
void unknown_000226c0(void);
void unknown_0002b4a0(void);
void unknown_0002b540(void);
void unknown_0002b5a0(void);
void unknown_0002b5c0(void);
void unknown_0003f2b0(void);
void simulation_initialize(void);
void simulation_dispose(void);
void simulation_initialize_for_new_map(void);
void simulation_dispose_from_old_map(void);
void unknown_0008d9f0(void);
void unknown_0008dd70(void);
void unknown_000b67c0(void);
void unknown_000b68c0(void);
void unknown_000b69d0(void);
void unknown_000b6ab0(void);
void unknown_000b6bb0(void);
void unknown_000b6c50(void);
void unknown_000b6df0(void);
void unknown_000d4de0(void);
void unknown_000d4e20(void);
void unknown_000d4e30(void);
void unknown_000d4e60(void);
void game_unknown_0011be20_initialize_for_new_map(void);
void game_unknown_0011be20_dispose_from_old_map(void);
void unknown_0011cc10(void);
void sound_initialize_for_new_map(void);
void sound_dispose_from_old_map(void);
void sound_initialize_for_new_structure_bsp(void);
void unknown_00131ff0(void);
void unknown_001320b0(void);
void unknown_00134d90(void);
void unknown_00134dd0(void);
void unknown_00134de0(void);
void unknown_0013bf00(void);
void unknown_0013bf60(void);
void unknown_0013bfc0(void);
void unknown_0013e5c7(void);
void unknown_0013e5d6(void);
void unknown_0013e610(void);
void unknown_00146240(void);
void game_time_initialize(void);
void game_time_initialize_for_new_map(void);
void game_time_dispose_from_old_map(void);
void unknown_001476c7(void);
void unknown_0014783f(void);
void unknown_0014784e(void);
void unknown_00147920(void);
void unknown_0014b4b0(void);
void unknown_0014b560(void);
void unknown_0014b580(void);
void unknown_0014b600(void);
void unknown_0014b6a0(void);
void unknown_00153870(void);
void unknown_001538b0(void);
void unknown_001552e0(void);
void unknown_00155380(void);
void unknown_00155490(void);
void unknown_00155ea0(void);
void unknown_00155ed0(void);
void unknown_00157450(void);
void unknown_001574c0(void);
void unknown_00157bb0(void);
void unknown_0016efb0(void);
void unknown_0016f0e0(void);
void unknown_0016f120(void);
void unknown_00175b50(void);
void effects_dispose(void);
void effects_initialize_for_new_map(void);
void effects_dispose_from_old_map(void);
void unknown_00175d70(void);
void unknown_0017b7b0(void);
void contrails_dispose(void);
void contrails_initialize_for_new_map(void);
void contrails_dispose_from_old_map(void);
void unknown_0017b940(void);
void unknown_0017d220(void);
void unknown_0017d2a0(void);
void decals_initialize_for_new_map(void);
void unknown_0017d330(void);
void unknown_0017d350(void);
void unknown_0017d440(void);
void unknown_0017d5e0(void);
void breakable_surfaces_initialize(void);
void breakable_surfaces_initialize_for_new_map(void);
void breakable_surfaces_unknown_00183ee0(void);
void breakable_surfaces_unknown_00183f00(void);
void sound_unknown_00185ab0(void);
void sound_unknown_00185b00(void);
void unknown_001887d0(void);
void unknown_00188850(void);
void unknown_00188870(void);
void unknown_00188940(void);
void unknown_00188980(void);
void unknown_0019bcd0(void);
void unknown_0019bd50(void);
void unknown_0019c190(void);
void unknown_0019c1d0(void);
void unknown_001c25a0(void);
void unknown_001c2600(void);
void unknown_001c2670(void);
void unknown_001c2690(void);
void unknown_001c27a0(void);
void unknown_001c7790(void);
void unknown_001c79c0(void);
void unknown_001c79e0(void);
void unknown_001c7b20(void);
void unknown_001c7bb0(void);
void unknown_001c7f60(void);
void game_unknown_001df460_initialize(void);
void game_unknown_001df460_initialize_for_new_map(void);
void game_unknown_001e6a40_initialize(void);
void game_unknown_001e6a40_initialize_for_new_map(void);
void game_unknown_001e6a40_dispose_from_old_map(void);
void game_unknown_001eb550_initialize(void);
void game_unknown_001eb550_dispose(void);
void game_unknown_001eb550_initialize_for_new_map(void);
void unknown_001fb2f0(void);
void unknown_001fb330(void);
void unknown_001fb350(void);
void unknown_0020a700(void);
void unknown_0020a750(void);
void unknown_0020a7d0(void);
void unknown_0020a810(void);
void game_unknown_0020aee0_initialize(void);
void game_unknown_0020aee0_dispose(void);
void game_unknown_0020aee0_initialize_for_new_map(void);
void unknown_00210db0(void);
void unknown_00210df0(void);
void unknown_00210e10(void);
void unknown_00211030(void);
void unknown_00215690(void);
void saved_game_files_dispose_from_old_map(void);
void unknown_00215730(void);
void sound_classes_initialize(void);
void sound_classes_initialize_for_new_map(void);
void sound_classes_dispose_from_old_map(void);
void sound_classes_dispose(void);
void sound_unknown_002229d0(void);
void sound_unknown_00222a10(void);
void sound_unknown_00222a60(void);
void sound_unknown_00225700(void);
void sound_unknown_00225a40(void);
void sound_unknown_00225a80(void);
void unknown_00225f80(void);
void unknown_00225fc0(void);
void unknown_003361a4(void);

/* 0x0014c630, 0x0014c880 and 0x0014bc00: the consoles of a game, this one,
and a player that is made; 0x00083510 and 0x001e73b0 are after them */
void __attribute__((stdcall)) unknown_0014c630(unsigned long machine_valid_mask, const void *machine_identifiers);
void __attribute__((stdcall)) unknown_0014c880(const void *local_machine_identifier);
void unknown_0014bc00(const struct game_player_options *player);
void simulation_players_refresh(long unknown);
void unknown_001e73b0(void);

/* 0x0019043f: told, when the campaign is won, each of four things and the
difficulty */
void __attribute__((stdcall)) user_interface_controller_raise_profile_unknownEC(long unknown, long index, long difficulty);

/* ---------- structures */

/* what a tick has on the stack: 0x4048 bytes */
struct game_tick_state
{
	byte unknown0[4];
	/* the systems are updated */
	boolean update;
	byte unknown5[0x4043];
};

/* 0x00510c6c points to it */
struct unknown_00510c6c
{
	byte unknown0[4];
	/* the time of the ticks that it has seen */
	real seconds;
	byte unknown8[0xC];
	/* less one each tick, down to 0 */
	long countdown;
};

/* ---------- constants */

/* how many things game_won tells the difficulty to */
#define GAME_WON_COUNT 4

enum
{
	/* _control87: single precision, round to nearest, no exceptions */
	FLOATING_POINT_CONTROL= 0x9001F,
	FLOATING_POINT_CONTROL_MASK= 0xFFFFF
};

/* how long after the game is lost it goes back to the checkpoint, and how
long after it is over it ends */
#define GAME_LOST_SECONDS 5.0f
#define GAME_FINISHED_SECONDS 7.0f

/* ---------- globals */

/* 0x004ed39c: no players are made for a game while it is set; and
0x004cf780, which is given to 0x00083510. What they are is not known */
boolean unknown_004ed39c;
long unknown_004cf780;

/* 0x004e6948: in the state of the game */
struct main_game_globals *main_game_globals;

/* 0x00510c54 */
struct game_time_globals *game_time_globals;

/* 0x004e7408: where the seed of the game's random numbers is. The name is a
guess */
unsigned long *global_random_seed_reference;

/* 0x00510c49 and 0x004f55e7: what game_options_new_default and
game_unknown_001388a0 look at. What they are is not known */
boolean unknown_00510c49;
boolean unknown_004f55e7;

/* 0x00440dd8: the parts of the game, in the order that they are set up; they
are taken down in the opposite order */
struct game_system game_systems[NUMBER_OF_GAME_SYSTEMS]=
{
	{transport_initialize, transport_dispose},
	{no_operation, no_operation},
	{no_operation, no_operation, unknown_0011cc10, no_operation},
	{unknown_00146240, no_operation},
	{unknown_0008d9f0, unknown_0008dd70},
	{no_operation, no_operation},
	{no_operation, no_operation},
	{no_operation, no_operation},
	{no_operation, no_operation},
	{unknown_00215690, unknown_00215730, no_operation, saved_game_files_dispose_from_old_map},
	{input_abstraction_initialize, input_abstraction_dispose},
	{unknown_00155ea0, unknown_00155ed0},
	{no_operation, no_operation},
	{unknown_0019bcd0, unknown_0019bd50, unknown_0019c190, unknown_0019c1d0},
	{unknown_00131ff0, unknown_001320b0},
	{sound_unknown_00225700, no_operation},
	{game_state_initialize, game_state_dispose, game_state_initialize_for_new_map, no_operation, game_state_set_structure_bsp_index, no_operation},
	{game_time_initialize, no_operation, game_time_initialize_for_new_map, game_time_dispose_from_old_map},
	{game_unknown_001df460_initialize, no_operation, game_unknown_001df460_initialize_for_new_map, no_operation},
	{unknown_0014b4b0, unknown_0014b560, unknown_0014b580, unknown_0014b600, no_operation, no_operation},
	{sound_unknown_00185ab0, no_operation, sound_unknown_00185b00, no_operation},
	{game_unknown_001e6a40_initialize, no_operation, game_unknown_001e6a40_initialize_for_new_map, game_unknown_001e6a40_dispose_from_old_map},
	{unknown_00157450, no_operation, unknown_001574c0, unknown_00157bb0},
	{simulation_initialize, simulation_dispose, simulation_initialize_for_new_map, simulation_dispose_from_old_map},
	{no_operation, no_operation, game_unknown_0011be20_initialize_for_new_map, game_unknown_0011be20_dispose_from_old_map},
	{game_unknown_001eb550_initialize, game_unknown_001eb550_dispose, game_unknown_001eb550_initialize_for_new_map, no_operation},
	{no_operation, no_operation},
	{unknown_000b67c0, unknown_000b68c0, unknown_000b69d0, unknown_000b6ab0, unknown_000b6bb0, unknown_000b6c50, NULL, NULL, (game_system_activation_proc)unknown_000b6df0},
	{object_early_movers_initialize, object_early_movers_dispose, object_early_movers_initialize_for_new_map, object_early_movers_dispose_from_old_map},
	{object_unknown_0010a980_initialize, object_unknown_0010a980_dispose, object_unknown_0010a980_initialize_for_new_map, object_unknown_0010a980_dispose_from_old_map},
	{game_unknown_001ec420_initialize, game_unknown_001ec420_dispose, game_unknown_001ec420_initialize_for_new_map, game_unknown_001ec420_dispose_from_old_map},
	{unknown_00012560, unknown_000141c0, unknown_00022590, unknown_000226c0, unknown_000226a0, unknown_000226b0},
	{unknown_0002b4a0, unknown_0002b5c0, unknown_0002b540, unknown_0002b5a0},
	{no_operation, no_operation, no_operation, no_operation},
	{breakable_surfaces_initialize, no_operation, breakable_surfaces_initialize_for_new_map, no_operation, breakable_surfaces_unknown_00183ee0, breakable_surfaces_unknown_00183f00},
	{unknown_001552e0, no_operation, unknown_00155380, unknown_00155490},
	{unknown_0016efb0, no_operation, unknown_0016f0e0, no_operation, unknown_0016f120, no_operation},
	{unknown_0017d220, unknown_0017d2a0, decals_initialize_for_new_map, unknown_0017d330, unknown_0017d350, unknown_0017d440, NULL, (game_system_activation_proc)unknown_0017d5e0},
	{no_operation, no_operation, no_operation, no_operation},
	{unknown_0017b7b0, contrails_dispose, contrails_initialize_for_new_map, contrails_dispose_from_old_map, unknown_0017b940, no_operation},
	{unknown_00175b50, effects_dispose, effects_initialize_for_new_map, effects_dispose_from_old_map, unknown_00175d70, no_operation},
	{no_operation, no_operation, no_operation, no_operation, unknown_0003f2b0, no_operation},
	{no_operation, no_operation, unknown_00211030, no_operation},
	{sound_classes_initialize, sound_classes_dispose, sound_classes_initialize_for_new_map, sound_classes_dispose_from_old_map},
	{sound_initialize, sound_dispose, sound_initialize_for_new_map, sound_dispose_from_old_map, sound_initialize_for_new_structure_bsp, no_operation},
	{game_unknown_0020aee0_initialize, game_unknown_0020aee0_dispose, game_unknown_0020aee0_initialize_for_new_map, no_operation},
	{unknown_001887d0, unknown_00188850, unknown_00188870, unknown_00188940, unknown_00188980, no_operation},
	{sound_unknown_00225a40, no_operation, sound_unknown_00225a80, no_operation},
	{sound_unknown_002229d0, no_operation, sound_unknown_00222a10, sound_unknown_00222a60},
	{unknown_00153870, no_operation, unknown_001538b0, no_operation},
	{unknown_001476c7, unknown_0014783f, unknown_0014784e, unknown_00147920},
	{unknown_0013e5c7, unknown_003361a4, unknown_0013e5d6, unknown_0013e610},
	{no_operation, no_operation, no_operation, no_operation},
	{unknown_0013bf00, no_operation, unknown_0013bf60, unknown_0013bfc0},
	{unknown_0020a700, unknown_0020a810, unknown_0020a750, unknown_0020a7d0},
	{unknown_001fb2f0, no_operation, unknown_001fb330, unknown_001fb350},
	{no_operation, no_operation, no_operation, no_operation},
	{unknown_000d4de0, no_operation, unknown_000d4e20, no_operation, unknown_000d4e30, no_operation, (game_system_structure_bsp_proc)unknown_000d4e60},
	{unknown_001c7790, unknown_001c79c0, unknown_001c79e0, unknown_001c7b20, unknown_001c7bb0, unknown_001c7f60},
	{unknown_001c25a0, unknown_001c2600, unknown_001c2670, no_operation, unknown_001c2690, unknown_001c27a0},
	{no_operation, no_operation, no_operation, no_operation, unknown_0014b6a0, no_operation},
	{no_operation, no_operation, no_operation, no_operation, no_operation, no_operation},
	{no_operation, no_operation, no_operation, no_operation, no_operation, no_operation},
	{unknown_00210db0, no_operation, no_operation, no_operation, unknown_00210df0, unknown_00210e10},
	{unknown_00134d90, unknown_00134dd0, unknown_00134de0, no_operation},
	{no_operation, no_operation, no_operation, no_operation, no_operation, no_operation},
	{unknown_00225f80, no_operation, unknown_00225fc0, no_operation},
	{no_operation, no_operation, no_operation, no_operation, no_operation, no_operation}
};

/* ---------- code */

/* 0x00137c20: takes the memory of the game's globals and sets up every part
of the game */
void game_initialize(
	void)
{
	long index;

	game_state_initialize();
	main_game_globals= game_state_malloc(sizeof(struct main_game_globals));
	memset(main_game_globals, 0, sizeof(struct main_game_globals));
	main_game_globals->structure_bsp_index= NONE;
	_control87(FLOATING_POINT_CONTROL, FLOATING_POINT_CONTROL_MASK);

	for (index= 0; index<NUMBER_OF_GAME_SYSTEMS; index++)
	{
		game_systems[index].initialize();
	}
}

/* takes down every part of the game. The game has no function for it: the
code is in main_loop */
void game_dispose(
	void)
{
	long index;

	main_game_globals= NULL;
	for (index= NUMBER_OF_GAME_SYSTEMS-1; index>=0; index--)
	{
		game_systems[index].dispose();
	}
}

/* 0x00137dd0: keeps the options of a game that starts. The name is a guess */
void game_options_new(
	const struct game_options *options)
{
	struct main_game_globals *game= main_game_globals;

	memcpy(&game->options, options, sizeof(game->options));
	game->options.unknown124= FALSE;
	if (game->options.game_mode==_game_mode_multiplayer || game->options.variant.unknown44)
	{
		unknown_0019d650(game->options.variant.unknown0);
		game= main_game_globals;
	}
	*global_random_seed_reference= game->options.random_seed;
	game->lost= FALSE;
	game->finished= FALSE;
	game->unknown11FA= 0;
	game->unknown1130= 0;
}

/* 0x00137ca0 */
void game_initialize_for_new_map(
	const struct game_options *options)
{
	long index;

	_control87(FLOATING_POINT_CONTROL, FLOATING_POINT_CONTROL_MASK);
	main_game_globals->initializing= TRUE;
	game_options_new(options);
	for (index= 0; index<NUMBER_OF_GAME_SYSTEMS; index++)
	{
		if (game_systems[index].initialize_for_new_map)
		{
			game_systems[index].initialize_for_new_map();
		}
	}
	main_game_globals->initializing= FALSE;
	main_game_globals->map_initialized= TRUE;
}

/* 0x00137d00 */
void game_dispose_from_old_map(
	void)
{
	long index;

	main_game_globals->active= FALSE;
	for (index= NUMBER_OF_GAME_SYSTEMS-1; index>=0; index--)
	{
		if (game_systems[index].dispose_from_old_map)
		{
			game_systems[index].dispose_from_old_map();
		}
	}
	main_game_globals->map_initialized= FALSE;
}

/* 0x00137d40: the structure bsp of the scenario that is loaded changed. The
name is a guess, as that of the next function */
void game_initialize_for_new_structure_bsp(
	void)
{
	struct main_game_globals *game= main_game_globals;
	long index;

	memset(game->unknown1138, 0, sizeof(game->unknown1138));
	memset(game->unknown1178, 0, sizeof(game->unknown1178));
	memset(game->unknown11B8, 0, sizeof(game->unknown11B8));
	game->structure_bsp_index= global_structure_bsp_index;
	for (index= 0; index<NUMBER_OF_GAME_SYSTEMS; index++)
	{
		if (game_systems[index].initialize_for_new_structure_bsp)
		{
			game_systems[index].initialize_for_new_structure_bsp();
		}
	}
}

/* 0x00137da0 */
void game_dispose_from_old_structure_bsp(
	void)
{
	long index;

	for (index= NUMBER_OF_GAME_SYSTEMS-1; index>=0; index--)
	{
		if (game_systems[index].dispose_from_old_structure_bsp)
		{
			game_systems[index].dispose_from_old_structure_bsp();
		}
	}
	main_game_globals->structure_bsp_index= NONE;
}

/* 0x00138110: the options of a game with nothing chosen: the campaign, on
one console, in the normal difficulty. The name is a guess */
void game_options_new_default(
	struct game_options *options)
{
	long refresh_rate;

	memset(options, 0, sizeof(*options));
	options->game_mode= _game_mode_campaign;
	options->unknown128= unknown_00510c49;
	options->simulation= _game_simulation_local;
	options->unknown5= 0;
	refresh_rate= xbox_video_settings.refresh_rate;
	if (refresh_rate<=0)
	{
		refresh_rate= GAME_DEFAULT_REFRESH_RATE;
	}
	options->difficulty= _game_difficulty_normal;
	options->tick_rate= (short)(refresh_rate/2);
	options->unknown14= NONE;
	options->unknown18= NONE;
	transport_security_random_bytes(sizeof(options->game_instance), options->game_instance);
	options->random_seed= GAME_DEFAULT_RANDOM_SEED;
}

/* 0x00510c6c */
struct unknown_00510c6c *unknown_00510c6c;

/* 0x00137fe0: a tick of the game: the systems are updated, if the first
functions say so, and the time goes on a tick. The name is Halo 1's */
void game_tick(
	void)
{
	struct game_tick_state state;

	_control87(FLOATING_POINT_CONTROL, FLOATING_POINT_CONTROL_MASK);
	simulation_build_update(&state);
	simulation_apply_update_changes(&state);
	if (state.update)
	{
		unknown_0014d3a0(&state);
		unknown_00138db0(&state);
		unknown_001c83c0();
		unknown_001fb510();
		unknown_0020b660();
		unknown_0015c150();
		unknown_00195ed0();
		unknown_00209310();
		unknown_00138c10();
		unknown_000b6f10();
		unknown_001c3d30();
		unknown_000b7060();
		unknown_000b7150();
		unknown_002268e0();
		unknown_00176e40();
		unknown_000c1400();
		unknown_0015ca10();
		unknown_0014d8b0();
		unknown_001df500();
		game_update_lost();
		game_update_finished();
		unknown_00226050();
		unknown_0013c680();
		animation_cache_idle();
		unknown_00225730();
		unknown_00165dc1();
		unknown_00153950();
		unknown_001917b2();
		unknown_0016f200();
		{
			/* the game takes one away as an unsigned number */
			long countdown= (long)((unsigned long)unknown_00510c6c->countdown-1);

			unknown_00510c6c->countdown= countdown<0 ? 0 : countdown;
		}
		unknown_00510c6c->seconds= game_time_globals->seconds_per_tick + unknown_00510c6c->seconds;
	}
	simulation_apply_update(&state);
	if (state.update)
	{
		game_time_globals->time++;
	}
}

/* 0x004e61cc: something of each of the four controllers: those that have
it are given to the local players of a game first */
long unknown_004e61cc[MAXIMUM_CONTROLLERS];

/* the controller after one, or NONE after the last */
static inline __attribute__((always_inline)) long game_next_controller(
	long controller_index)
{
	return (controller_index>=0 && controller_index<MAXIMUM_CONTROLLERS-1) ? controller_index+1 : NONE;
}

/* 0x00138600: the order of the controllers of the local players: by their
index, with NONE last. The name is a guess */
int game_options_controller_compare(
	const void *a,
	const void *b)
{
	short first= *(const short *)a;
	short second= *(const short *)b;

	if (first==NONE)
	{
		return second==NONE ? 0 : 1;
	}
	if (second==NONE)
	{
		return -1;
	}
	if (first>second)
	{
		return 1;
	}

	return first>=second ? 0 : -1;
}

/* 0x00138640: a game of this console alone with some local players: each
gets a controller (one of unknown_004e61cc that is set if there is one,
else the first that no other has) and the rest of its options as a game
starts them. The name is a guess */
void game_options_setup_local_players(
	long count,
	struct game_options *options)
{
	boolean taken[MAXIMUM_CONTROLLERS];
	long controllers[MAXIMUM_CONTROLLERS];
	long controller_index;
	long player_index;

	options->machine_valid_mask= 1;
	memset(options->machine_identifiers[0], 0, GAME_MACHINE_IDENTIFIER_SIZE);
	memcpy(options->local_machine_identifier, options->machine_identifiers[0], GAME_MACHINE_IDENTIFIER_SIZE);
	options->local_machine_exists= TRUE;

	for (controller_index= 0; controller_index!=NONE; controller_index= game_next_controller(controller_index))
	{
		taken[controller_index]= FALSE;
		controllers[controller_index]= NONE;
	}
	for (player_index= 0; player_index<count; player_index++)
	{
		long controller= NONE;

		for (controller_index= 0; controller_index!=NONE; controller_index= game_next_controller(controller_index))
		{
			if (unknown_004e61cc[(short)controller_index] && !taken[controller_index])
			{
				controller= controller_index;
				break;
			}
		}
		if (controller==NONE)
		{
			controller= 0;
		}
		if (taken[controller])
		{
			for (controller_index= 0; controller_index!=NONE; controller_index= game_next_controller(controller_index))
			{
				if (!taken[controller_index])
				{
					controller= controller_index;
					break;
				}
			}
		}
		controllers[player_index]= controller;
		taken[controller]= TRUE;
	}
	qsort(controllers, MAXIMUM_CONTROLLERS, sizeof(controllers[0]), game_options_controller_compare);

	memset(options->players, 0, sizeof(options->players));
	for (player_index= 0; player_index<count; player_index++)
	{
		struct game_player_options *player= &options->players[player_index];

		player->valid= TRUE;
		player->unknown1= FALSE;
		memcpy(player->machine_identifier, options->local_machine_identifier, GAME_MACHINE_IDENTIFIER_SIZE);
		player->unknown4= controllers[player_index];
		player->controller_index= (short)player_index;
		memset(player->identifier, 0, sizeof(player->identifier));
		player->identifier[0]= (byte)player_index;
		player->identifier[1]= 0xFF;
		player->identifier[2]= 0xFF;
		player->identifier[3]= 0xFF;
		wcsncpy(player->name, L"", GAME_PLAYER_NAME_LENGTH);
		player->unknown5A= 0;
		memset(player->unknown5C, 0, sizeof(player->unknown5C));
		player->unknown98= options->game_mode==_game_mode_multiplayer ? (byte)player_index : 1;
	}
}

/* 0x00485b20: the seconds of the last frame of the game that took any */
real unknown_00485b20;

/* 0x004ba040: the seconds of all the frames of the game. The name is a
guess */
double game_seconds;

/* 0x00137ed0: what the game does in a frame of some seconds, after its
ticks: if the frame took any time the systems that go by the frame are
updated, and the display is set up again if its settings are not those
that the game starts with. The name is Halo 1's */
void __attribute__((stdcall)) game_frame(
	real seconds)
{
	if (seconds>0.0f)
	{
		unknown_0017bc40(seconds);
		unknown_0017d690();
		game_unknown_000d4db0(seconds);
		unknown_0018c060(seconds);
		unknown_002118c0();
		unknown_00134e00(seconds);
		if (memcmp(&display_settings, &display_settings_default, sizeof(display_settings)))
		{
			unknown_00014980();
			unknown_000128c0(&display_settings_default);
			unknown_00014600();
		}
		unknown_00485b20= seconds;
		unknown_00039e50(seconds);
	}
	unknown_00176f60(seconds);
	game_seconds+= seconds;
}

/* 0x00137f70: runs so many ticks of the game. If the main loop gets
something to do, the ticks that are left are not run, and their time is taken
off the seconds. The name is a guess */
void game_run_ticks(
	long tick_count,
	float *seconds)
{
	long tick;

	for (tick= 0; tick<tick_count; tick++)
	{
		if (main_action_pending())
		{
			break;
		}
		game_tick();
	}

	if (tick<tick_count)
	{
		if (tick==0)
		{
			*seconds= 0.0f;
		}
		else
		{
			float left= *seconds-(float)(tick_count-tick)*game_time_globals->seconds_per_tick;

			if (!(left>0.0f))
			{
				left= 0.0f;
			}
			*seconds= left;
		}
	}
}

/* 0x00138800: whether a map is loaded and a game goes on. As in Halo 1 */
boolean game_in_progress(
	void)
{
	return main_game_globals && main_game_globals->active;
}

/* 0x00138820: whether the game is a campaign of two players. The name is a
guess */
long game_is_coop(
	void)
{
	return main_game_globals->options.game_mode==_game_mode_campaign && main_game_globals->options.coop;
}

/* 0x00138840: whether the game is run among several consoles. The name is a
guess, as those of the next two */
boolean game_is_distributed(
	void)
{
	long simulation= main_game_globals->options.simulation;

	return simulation>=_game_simulation_distributed_first && simulation<=_game_simulation_distributed_last;
}

/* 0x00138860 */
boolean game_is_authoritative(
	void)
{
	return main_game_globals->options.simulation!=_game_simulation_predicted;
}

/* 0x00138880 */
boolean game_is_predicted(
	void)
{
	return main_game_globals->options.simulation==_game_simulation_predicted;
}

/* 0x001388a0: false in a campaign if unknown_004f55e7 is set or the
difficulty is legendary. What it is asked for is not known */
long game_unknown_001388a0(
	void)
{
	if (main_game_globals->options.game_mode==_game_mode_campaign)
	{
		if (unknown_004f55e7 || main_game_globals->options.difficulty==_game_difficulty_legendary)
		{
			return FALSE;
		}
	}

	return TRUE;
}

/* 0x00138960: the game is lost, or is not any more. It goes back to the last
checkpoint a while after it is lost. As in Halo 1 */
void game_lost(
	boolean lost)
{
	struct main_game_globals *game= main_game_globals;

	if (lost)
	{
		if (!game->lost)
		{
			game->lost= TRUE;
			game->lost_ticks= lrintf((float)game_time_globals->ticks_per_second*GAME_LOST_SECONDS);
			game_state_note_death();
		}
	}
	else if (game->lost)
	{
		game->lost= FALSE;
	}
}

/* 0x00137e40: the consoles of the game that starts are told to the network,
and this one if it is in the game, and each player of the game is made.
The name is a guess */
void game_create_players(
	void)
{
	struct game_options *options= &main_game_globals->options;
	long index;

	if (unknown_004ed39c)
	{
		return;
	}
	unknown_0014c630(options->machine_valid_mask, options->machine_identifiers);
	unknown_0014c880(options->local_machine_exists ? options->local_machine_identifier : NULL);
	for (index= 0; index<MAXIMUM_GAME_PLAYERS; index++)
	{
		if (options->players[index].valid)
		{
			unknown_0014bc00(&options->players[index]);
		}
	}
	if (unknown_004cf770.unknown0 && !unknown_004cf770.unknownB)
	{
		simulation_players_refresh(unknown_004cf780);
	}
	unknown_001e73b0();
}

/* 0x001388e0: the game is won: in the campaign, each of the four is told the
difficulty (if the game has a 0x18 of its options), the players are told
that it is being saved, and the next map is loaded; in multiplayer, the game
is over. As Halo 1's of this name */
void game_won(
	void)
{
	if (!main_game_globals || !main_game_globals->active)
	{
		return;
	}
	if (main_game_globals->options.game_mode==_game_mode_campaign)
	{
		if (main_game_globals->options.unknown18!=NONE)
		{
			long index= 0;

			do
			{
				user_interface_controller_raise_profile_unknownEC(main_game_globals->options.unknown18, index, main_game_globals->options.difficulty);
				index= (index>=0 && index<GAME_WON_COUNT-1) ? index+1 : NONE;
			}
			while (index!=NONE);
			main_print_saving_message();
		}
		main_game_next_map();
	}
	else if (main_game_globals->options.game_mode==_game_mode_multiplayer)
	{
		game_finish();
	}
}

/* 0x001389c0: the game is over: it ends a while after. The name is a
guess */
void game_finish(
	void)
{
	struct main_game_globals *game= main_game_globals;

	if (!game->finished)
	{
		game->finished= TRUE;
		game->finished_ticks= lrintf((float)game_time_globals->ticks_per_second*GAME_FINISHED_SECONDS);
		game->unknown1129= FALSE;
	}
}

/* 0x00138a10: whether the game is over and the while has gone by. The name is
a guess */
long game_finished_wait_over(
	void)
{
	return main_game_globals->finished && main_game_globals->finished_ticks==0;
}

/* 0x00138180: whether the options are of a game that can be played: its
values are in their ranges, no two consoles and no two players are the same,
each player is at a console of the game, and the players are as many as the
kind of game takes. The name is a guess */
boolean game_options_verify(
	const struct game_options *options)
{
	static const byte zeros[GAME_PLAYER_IDENTIFIER_SIZE]= {0};
	unsigned long mask= options->machine_valid_mask;
	long player_count= 0;
	boolean valid;
	long index;

	if (options->game_mode<0 || options->game_mode>=NUMBER_OF_GAME_MODES ||
		options->simulation<0 || options->simulation>=NUMBER_OF_GAME_SIMULATIONS ||
		options->tick_rate<=0 || options->tick_rate>MAXIMUM_GAME_TICK_RATE ||
		!options->map_name[0] ||
		options->unknown120<0 || options->unknown120>=16)
	{
		return FALSE;
	}

	valid= (mask&0xFFFF0000)==0;
	for (index= 0; index<MAXIMUM_GAME_MACHINES; index++)
	{
		if (mask&(1UL<<index))
		{
			long other_index;

			for (other_index= index+1; other_index<MAXIMUM_GAME_MACHINES; other_index++)
			{
				if (mask&(1UL<<other_index))
				{
					valid= valid && memcmp(options->machine_identifiers[index],
						options->machine_identifiers[other_index], GAME_MACHINE_IDENTIFIER_SIZE)!=0;
				}
			}
		}
	}

	for (index= 0; index<MAXIMUM_GAME_PLAYERS; index++)
	{
		const struct game_player_options *player= &options->players[index];
		boolean player_valid;
		long other_index;

		if (!player->valid)
		{
			continue;
		}
		player_count++;

		player_valid= valid &&
			player->controller_index>=0 && player->controller_index<MAXIMUM_CONTROLLERS &&
			memcmp(player->identifier, zeros, GAME_PLAYER_IDENTIFIER_SIZE)!=0;
		for (other_index= index+1; other_index<MAXIMUM_GAME_PLAYERS; other_index++)
		{
			const struct game_player_options *other= &options->players[other_index];

			if (other->valid)
			{
				player_valid= player_valid &&
					memcmp(player->identifier, other->identifier, GAME_PLAYER_IDENTIFIER_SIZE)!=0;
			}
		}

		if (player->unknown1)
		{
			valid= player_valid && memcmp(player->machine_identifier, zeros, GAME_MACHINE_IDENTIFIER_SIZE)==0;
		}
		else
		{
			long machine_index= NONE;

			for (other_index= 0; other_index<MAXIMUM_GAME_MACHINES; other_index++)
			{
				if ((mask&(1UL<<other_index)) &&
					memcmp(player->machine_identifier, options->machine_identifiers[other_index],
						GAME_MACHINE_IDENTIFIER_SIZE)==0)
				{
					machine_index= other_index;
					break;
				}
			}
			valid= player_valid && machine_index!=NONE;
		}
	}

	switch (options->game_mode)
	{
	case _game_mode_campaign:
		valid= valid && options->difficulty>=0 && options->difficulty<NUMBER_OF_GAME_DIFFICULTIES;
		if (options->coop)
		{
			if (!valid || player_count!=2)
			{
				return FALSE;
			}
		}
		else if (!valid || player_count!=1)
		{
			return FALSE;
		}
		break;
	case _game_mode_multiplayer:
		if (!valid || player_count<1 || player_count>MAXIMUM_GAME_PLAYERS)
		{
			return FALSE;
		}
		break;
	case _game_mode_3:
		if (!valid || player_count<1)
		{
			return FALSE;
		}
		break;
	case _game_mode_4:
	case _game_mode_5:
		/* nothing is asked of how many players there are, or of this
		console */
		return valid;
	default:
		return FALSE;
	}

	return options->local_machine_exists!=0;
}

static inline __attribute__((always_inline)) unsigned long game_options_local_controllers(
	const struct game_options *options)
{
	unsigned long controllers= 0;

	if (options->local_machine_exists)
	{
		long index;

		for (index= 0; index<MAXIMUM_GAME_PLAYERS; index++)
		{
			const struct game_player_options *player= &options->players[index];

			if (player->valid && !player->unknown1 &&
				memcmp(player->machine_identifier, options->local_machine_identifier,
					GAME_MACHINE_IDENTIFIER_SIZE)==0)
			{
				controllers|= 1UL<<(player->controller_index&31);
			}
		}
	}

	return controllers;
}

/* 0x001384a0: whether two games are the same game: the same map, kind of
game and tick rate, the same controllers of this console, and the same
difficulty or rules. The name is a guess */
boolean game_options_match(
	const struct game_options *options1,
	const struct game_options *options2)
{
	if (strcmp(options1->map_name, options2->map_name)!=0 ||
		options1->game_mode!=options2->game_mode ||
		options1->tick_rate!=options2->tick_rate ||
		game_options_local_controllers(options1)!=game_options_local_controllers(options2))
	{
		return FALSE;
	}

	switch (options1->game_mode)
	{
	case _game_mode_campaign:
		return options1->coop==options2->coop && options1->difficulty==options2->difficulty;
	case _game_mode_multiplayer:
		return memcmp(&options1->variant, &options2->variant, sizeof(options1->variant))==0;
	}

	return TRUE;
}

/* 0x00547f6f: the game that was lost has waited long enough: it is to go
back to the last checkpoint */
boolean unknown_00547f6f;

/* 0x00188cd0: something is done two seconds before a game that is over
ends */
void unknown_00188cd0(void);

/* 0x00138e40: each tick of a campaign that this console does not predict:
the game is lost or is not, by what the players say (one thing of theirs, or
another in a cooperative game that game_unknown_001388a0 says nothing of);
and a game that is lost counts its ticks down, and at the last one is to go
back to the checkpoint */
void game_update_lost(
	void)
{
	struct main_game_globals *globals= main_game_globals;

	if (globals->options.game_mode==_game_mode_campaign && globals->options.simulation!=_game_simulation_predicted)
	{
		boolean lost;

		if (globals->options.coop && !(boolean)game_unknown_001388a0())
		{
			lost= players_globals->unknown0[5];
		}
		else
		{
			lost= players_globals->unknown0[4];
		}
		game_lost(lost);
		if (globals->lost)
		{
			if (globals->lost_ticks>0)
			{
				globals->lost_ticks--;
			}
			if (globals->lost_ticks==0)
			{
				unknown_00547f6f= TRUE;
			}
		}
	}
}

/* 0x00138eb0: each tick, a game that is over counts its ticks down; when
two seconds or less are left, something is done, once */
void game_update_finished(
	void)
{
	struct main_game_globals *globals= main_game_globals;

	if (globals->finished && globals->finished_ticks>0)
	{
		globals->finished_ticks--;
		if (!globals->unknown1129 && (real)globals->finished_ticks*game_time_globals->seconds_per_tick<=2.0f)
		{
			unknown_00188cd0();
			main_game_globals->unknown1129= TRUE;
		}
	}
}

/* 0x00161c90: a number of the rules of a game, by its number of 0x44: 1 for
5 and 6, and for what is not from 1 to 9; 2 for the others. The game has it
far from the rest of this file */
long game_variant_unknown_00161c90(
	const struct game_variant *variant)
{
	switch (variant->unknown44)
	{
	case 1:
	case 2:
	case 3:
	case 4:
	case 7:
	case 8:
	case 9:
		return 2;
	}

	return 1;
}

/* 0x004674b8: three things; each starts with what is called each tick, or
NULL */
byte unknown_004674b8[NUMBER_OF_GAME_UNKNOWN_004674B8][GAME_UNKNOWN_004674B8_SIZE];

/* 0x000d4db0: each tick, each of three things that has a function is told
how long the tick is. The game has it far from the rest of this file */
void __attribute__((stdcall)) game_unknown_000d4db0(
	real seconds)
{
	long index;

	for (index= 0; index<NUMBER_OF_GAME_UNKNOWN_004674B8; index++)
	{
		void (__attribute__((stdcall)) *update)(real seconds)=
			*(void (__attribute__((stdcall)) *const *)(real))unknown_004674b8[index];

		if (update)
		{
			update(seconds);
		}
	}
}
