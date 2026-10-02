/*
GAME.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The game as a whole: a table of its parts, and what goes through the table
when the game starts and ends, with a new map and with a new structure bsp.
The names are those of Halo 1's game.c where it has them; there, each of
these functions calls the parts by name.
*/

#include "game.h"

#include "main/main.h"
#include "saved games/game_state.h"
#include "shell/shell.h"
#include "scenario/scenario.h"

#include <string.h>

/* ---------- prototypes of functions of the console's libraries */

/* 0x003212d6 */
unsigned int _control87(unsigned int value, unsigned int mask);

/* ---------- prototypes of functions not decompiled yet */

/* 0x0019d650: sets up a part of the options of a multiplayer game */
void unknown_0019d650(void *unknown);

/* 0x0007ad80: fills a buffer with bytes made at random */
void unknown_0007ad80(long size, void *buffer);

/* 0x00137fe0: a tick of the game */
void unknown_00137fe0(void);

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
void unknown_00067e30(void);
void unknown_00067eb0(void);
void unknown_00067ee0(void);
void unknown_00067f60(void);
void unknown_0008d690(void);
void unknown_0008d6d0(void);
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
void unknown_00109300(void);
void unknown_00109350(void);
void unknown_00109360(void);
void unknown_00109380(void);
void unknown_0010a980(void);
void unknown_0010a9d0(void);
void unknown_0010a9e0(void);
void unknown_0010aab0(void);
void unknown_0011be20(void);
void unknown_0011be40(void);
void unknown_0011cc10(void);
void unknown_001253d0(void);
void unknown_00125600(void);
void unknown_00125690(void);
void unknown_001257b0(void);
void unknown_00125920(void);
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
void unknown_00146550(void);
void unknown_001465b0(void);
void unknown_00146610(void);
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
void unknown_00175bd0(void);
void unknown_00175c10(void);
void unknown_00175c70(void);
void unknown_00175d70(void);
void unknown_0017b7b0(void);
void unknown_0017b850(void);
void unknown_0017b870(void);
void unknown_0017b8a0(void);
void unknown_0017b940(void);
void unknown_0017d220(void);
void unknown_0017d2a0(void);
void unknown_0017d2e0(void);
void unknown_0017d330(void);
void unknown_0017d350(void);
void unknown_0017d440(void);
void unknown_0017d5e0(void);
void unknown_00183e40(void);
void unknown_00183ec0(void);
void unknown_00183ee0(void);
void unknown_00183f00(void);
void unknown_00185ab0(void);
void unknown_00185b00(void);
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
void unknown_001df460(void);
void unknown_001df4b0(void);
void unknown_001e6a40(void);
void unknown_001e6a80(void);
void unknown_001e6ae0(void);
void unknown_001eb550(void);
void unknown_001eb5e0(void);
void unknown_001eb5f0(void);
void unknown_001ec420(void);
void unknown_001ec470(void);
void unknown_001ec480(void);
void unknown_001ec4b0(void);
void unknown_001fb2f0(void);
void unknown_001fb330(void);
void unknown_001fb350(void);
void unknown_0020a700(void);
void unknown_0020a750(void);
void unknown_0020a7d0(void);
void unknown_0020a810(void);
void unknown_0020aee0(void);
void unknown_0020af50(void);
void unknown_0020af90(void);
void unknown_00210db0(void);
void unknown_00210df0(void);
void unknown_00210e10(void);
void unknown_00211030(void);
void unknown_00215690(void);
void unknown_00215720(void);
void unknown_00215730(void);
void unknown_00217bc0(void);
void unknown_00217c20(void);
void unknown_00221850(void);
void unknown_002218a0(void);
void unknown_002218e0(void);
void unknown_002218f0(void);
void unknown_002229d0(void);
void unknown_00222a10(void);
void unknown_00222a60(void);
void unknown_00225700(void);
void unknown_00225a40(void);
void unknown_00225a80(void);
void unknown_00225f80(void);
void unknown_00225fc0(void);
void unknown_003361a4(void);

/* ---------- constants */

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
	{unknown_0008d690, unknown_0008d6d0},
	{no_operation, no_operation},
	{no_operation, no_operation, unknown_0011cc10, no_operation},
	{unknown_00146240, no_operation},
	{unknown_0008d9f0, unknown_0008dd70},
	{no_operation, no_operation},
	{no_operation, no_operation},
	{no_operation, no_operation},
	{no_operation, no_operation},
	{unknown_00215690, unknown_00215730, no_operation, unknown_00215720},
	{unknown_00217bc0, unknown_00217c20},
	{unknown_00155ea0, unknown_00155ed0},
	{no_operation, no_operation},
	{unknown_0019bcd0, unknown_0019bd50, unknown_0019c190, unknown_0019c1d0},
	{unknown_00131ff0, unknown_001320b0},
	{unknown_00225700, no_operation},
	{game_state_initialize, game_state_dispose, game_state_initialize_for_new_map, no_operation, game_state_set_structure_bsp_index, no_operation},
	{unknown_00146550, no_operation, unknown_001465b0, unknown_00146610},
	{unknown_001df460, no_operation, unknown_001df4b0, no_operation},
	{unknown_0014b4b0, unknown_0014b560, unknown_0014b580, unknown_0014b600, no_operation, no_operation},
	{unknown_00185ab0, no_operation, unknown_00185b00, no_operation},
	{unknown_001e6a40, no_operation, unknown_001e6a80, unknown_001e6ae0},
	{unknown_00157450, no_operation, unknown_001574c0, unknown_00157bb0},
	{unknown_00067e30, unknown_00067eb0, unknown_00067ee0, unknown_00067f60},
	{no_operation, no_operation, unknown_0011be20, unknown_0011be40},
	{unknown_001eb550, unknown_001eb5e0, unknown_001eb5f0, no_operation},
	{no_operation, no_operation},
	{unknown_000b67c0, unknown_000b68c0, unknown_000b69d0, unknown_000b6ab0, unknown_000b6bb0, unknown_000b6c50, NULL, NULL, (game_system_activation_proc)unknown_000b6df0},
	{unknown_00109300, unknown_00109350, unknown_00109360, unknown_00109380},
	{unknown_0010a980, unknown_0010a9d0, unknown_0010a9e0, unknown_0010aab0},
	{unknown_001ec420, unknown_001ec470, unknown_001ec480, unknown_001ec4b0},
	{unknown_00012560, unknown_000141c0, unknown_00022590, unknown_000226c0, unknown_000226a0, unknown_000226b0},
	{unknown_0002b4a0, unknown_0002b5c0, unknown_0002b540, unknown_0002b5a0},
	{no_operation, no_operation, no_operation, no_operation},
	{unknown_00183e40, no_operation, unknown_00183ec0, no_operation, unknown_00183ee0, unknown_00183f00},
	{unknown_001552e0, no_operation, unknown_00155380, unknown_00155490},
	{unknown_0016efb0, no_operation, unknown_0016f0e0, no_operation, unknown_0016f120, no_operation},
	{unknown_0017d220, unknown_0017d2a0, unknown_0017d2e0, unknown_0017d330, unknown_0017d350, unknown_0017d440, NULL, (game_system_activation_proc)unknown_0017d5e0},
	{no_operation, no_operation, no_operation, no_operation},
	{unknown_0017b7b0, unknown_0017b850, unknown_0017b870, unknown_0017b8a0, unknown_0017b940, no_operation},
	{unknown_00175b50, unknown_00175bd0, unknown_00175c10, unknown_00175c70, unknown_00175d70, no_operation},
	{no_operation, no_operation, no_operation, no_operation, unknown_0003f2b0, no_operation},
	{no_operation, no_operation, unknown_00211030, no_operation},
	{unknown_00221850, unknown_002218f0, unknown_002218a0, unknown_002218e0},
	{unknown_001253d0, unknown_00125600, unknown_00125690, unknown_001257b0, unknown_00125920, no_operation},
	{unknown_0020aee0, unknown_0020af50, unknown_0020af90, no_operation},
	{unknown_001887d0, unknown_00188850, unknown_00188870, unknown_00188940, unknown_00188980, no_operation},
	{unknown_00225a40, no_operation, unknown_00225a80, no_operation},
	{unknown_002229d0, no_operation, unknown_00222a10, unknown_00222a60},
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
	if (game->options.game_mode==_game_mode_multiplayer || game->options.unknown178)
	{
		unknown_0019d650(game->options.unknown134);
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
	unknown_0007ad80(sizeof(options->game_instance), options->game_instance);
	options->random_seed= GAME_DEFAULT_RANDOM_SEED;
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
		unknown_00137fe0();
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

/* 0x00138820: whether the game is a campaign with unknown12C set */
long game_is_campaign_unknown12C(
	void)
{
	return main_game_globals->options.game_mode==_game_mode_campaign && main_game_globals->options.unknown12C;
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
