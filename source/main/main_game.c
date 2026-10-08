/*
MAIN_GAME.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The names are guesses.
*/

#include "saved games/saved_game_files.h"
#include "text/unicode_string_list.h"
#include "main_game.h"

#include "cache/animation_cache.h"
#include "cache/cache_files.h"
#include "cache/geometry_cache.h"
#include "cache/texture_cache.h"
#include "input/input.h"
#include "interface/user_interface_memory.h"
#include "saved games/game_state.h"
#include "main/main.h"
#include "memory/data.h"
#include "memory/physical_memory.h"
#include "scenario/scenario.h"
#include "sound/sound_memory.h"

#include <string.h>

/* ---------- prototypes of functions of the console's libraries */

/* 0x0033039a */
long XGetLanguage(void);

/* 0x003314b0 */
long GetTickCount(void);

/* ---------- prototypes of functions not decompiled yet */

/* those that take memory when a map is loaded: what gives it back, and what
takes it */
void unknown_00081f80(void);
void unknown_00081780(long configuration);
void unknown_000b3db0(void);
void unknown_000b3d30(long configuration);
void unknown_0021ebe0(void);
void unknown_0021ebc0(long configuration);

/* 0x000693a0: ends what unknown_004cf770 has going; of the network game */
void unknown_000693a0(void *unknown, void *also);

/* 0x0006cd50: the state of a session of the network game */
long unknown_0006cd50(long session_index);

/* 0x002186f0: before the caches of a map are closed */
void sound_cache_close(void);

/* 0x0011be50: after the caches of a map are closed */

/* what a new game sets up after its systems */
void unknown_000d5560(long unknown);
void unknown_000c09c0(long game_mode);
void unknown_00158530(void);
void unknown_00138f10(void);
void unknown_00200d80(void);

/* 0x00163890: waits for a map to be copied to the hard disk, most likely */
boolean unknown_00163890(const char *map_name, long two);

/* 0x0014bac0: before the game on a map is started again */
void unknown_0014bac0(void);

/* 0x000593e0: tells the network game that the game changes */
void unknown_000593e0(void);

/* 0x00149318: whether the change of game has to wait */
boolean unknown_00149318(void);

/* 0x0019040d, 0x0018fada: of the players' profiles: the second gives the
one that there is, if there is only one */
void unknown_0019040d(long unknown);
long unknown_0018fada(void);

/* 0x001c5ca0, 0x000b3ed0: what is told of a game of the campaign to the
network game's server: the first writes it, from the options of the game, and
the second sends it */
void unknown_001c5ca0(const struct game_options *options, void *report);
void unknown_000b3ed0(const void *data, long size, const char *content_type);

/* 0x0019c440, 0x0019c270: of the maps of a campaign: which comes after
one, NONE if it is the last, and what is known of one: its path is 8 bytes
in */
long unknown_0019c440(long campaign, long map);
const char *unknown_0019c270(long campaign, long map);

/* 0x00152450, 0x00152660: of the options of a game that is asked for: the
players that go on to it, most likely */
void unknown_00152450(struct game_options *options);
void unknown_00152660(struct game_options *options);

/* 0x00063f50: whether something else starts the game on the map */
boolean unknown_00063f50(long campaign, long map, const char *map_name);

/* of the C library */
long time(long *timer);
int rand(void);

/* 0x0012bf90, 0x001483c3: after the game of the main menu is started */
boolean unknown_0012bf90(void);
void unknown_001483c3(long unknown);

/* 0x0011c840: after the tags of a map are loaded */
void unknown_0011c840(void);



/* ---------- constants */

enum
{
	/* in the globals tag: how much each language has of texts (a struct
	language_texts: where they are is in unicode_string_list.h) */
	GAME_GLOBALS_LANGUAGE_TEXTS_SIZE= 0x1C,
	GAME_GLOBALS_LANGUAGE_TEXTS_LOADED_OFFSET= 0x18,

	/* where unknown_004cf770 says that a game is on */
	UNKNOWN_004CF770_ACTIVE_OFFSET= 0x2F,

	_unknown_0006cd50_state1= 1,

	/* what unknown_001c5ca0 writes */
	CAMPAIGN_REPORT_SIZE= 0x33B,
	CAMPAIGN_MAP_PATH_OFFSET= 8,

	/* what main_game_goto_main_menu is told when a campaign has no more maps */
	_main_menu_unknown7= 7
};

/* ---------- globals */

/* 0x004ed290 */
struct main_game_load_globals main_game_load_globals;

/* 0x00453c00: those that take memory when a map is loaded, in the order
that they take it */
struct main_game_memory_user main_game_memory_users[NUMBER_OF_MAIN_GAME_MEMORY_USERS]=
{
	{unknown_00081f80, (void (MAIN_GAME_CALLBACK *)(long))unknown_00081780},
	{user_interface_memory_pool_delete, user_interface_memory_pool_new},
	{saved_game_files_free_memory, saved_game_files_allocate_memory},
	{sound_memory_pool_delete, sound_memory_pool_new},
	{unknown_000b3db0, (void (MAIN_GAME_CALLBACK *)(long))unknown_000b3d30},
	{unknown_0021ebe0, (void (MAIN_GAME_CALLBACK *)(long))unknown_0021ebc0},
	{animation_cache_free_memory, animation_cache_allocate_memory},
	{geometry_cache_free_memory, geometry_cache_allocate_memory}
};

/* 0x004cf770 */
struct unknown_004cf770 unknown_004cf770;

/* 0x00467214: the session of the network game; NONE if there is none */
long unknown_00467214= NONE;

/* 0x00502104: a data array that is emptied when a map is loaded */
struct data_array *unknown_00502104;

/* 0x004e9bbc, 0x004e9bc0: the tags of a map are being loaded, and since when */
boolean main_game_loading_map;
unsigned long main_game_loading_map_time;

/* 0x0051ea00: set when there is no map to load */
boolean unknown_0051ea00;

/* 0x0051ea10: while it is more than 0, a game that was asked for waits */
long unknown_0051ea10;

/* 0x00527330, 0x00527334: of the network game, and its state */
boolean unknown_00527330;
long unknown_00527334;

/* 0x0055e757: the game is going to the main menu because a game could not
be started */
boolean main_game_load_failing;

/* 0x0054e7cc: a campaign that starts goes on from the saved game of the
player */
boolean unknown_0054e7cc;

/* 0x00453c40: the functions of an allocation whose memory is lent by the
texture cache. The game has one, at 0x0047d8e4, with no procedures */
const struct allocation_functions main_game_texture_allocation_functions=
{
	main_game_texture_allocation_allocate,
	main_game_texture_allocation_deallocate
};

/* ---------- code */

/* the texts of the console's language in the globals tag. The game has this
where it loads and lets go of a map */
static inline __attribute__((always_inline)) byte *main_game_language_texts(
	void)
{
	if (global_language==NONE)
	{
		global_language= language_from_xbox_language(XGetLanguage());
	}

	return (byte *)global_game_globals+GAME_GLOBALS_LANGUAGE_TEXTS_OFFSET+
		global_language*GAME_GLOBALS_LANGUAGE_TEXTS_SIZE;
}

static inline __attribute__((always_inline)) void main_game_language_texts_unload(
	void)
{
	byte *texts= main_game_language_texts();

	if (texts[GAME_GLOBALS_LANGUAGE_TEXTS_LOADED_OFFSET])
	{
		((unsigned long *)texts)[0]= 0;
		((unsigned long *)texts)[1]= 0;
		texts[GAME_GLOBALS_LANGUAGE_TEXTS_LOADED_OFFSET]= FALSE;
	}
}

static inline __attribute__((always_inline)) void main_game_set_map_name(
	const char *map_name)
{
	strncpy(main_game_load_globals.map_name, map_name, MAIN_GAME_MAP_NAME_LENGTH);
	main_game_load_globals.map_name[MAIN_GAME_MAP_NAME_LENGTH-1]= 0;
}

/* 0x0018eb80: ends the game that is on and lets go of the map, whatever was
loaded of it. The game has nothing for the stages 1 and 3 (it jumps to
nowhere): it is not called then */
void main_game_unload(
	void)
{
	if (main_game_globals && main_game_globals->active)
	{
		if (unknown_004cf770.unknown0)
		{
			byte *unknown= unknown_004cf770.unknownC;

			if (unknown[UNKNOWN_004CF770_ACTIVE_OFFSET] && !unknown_004cf770.unknownA &&
				!main_game_load_globals.restarting)
			{
				unknown_000693a0(unknown, unknown);
				unknown[UNKNOWN_004CF770_ACTIVE_OFFSET]= FALSE;
			}
		}
		scenario_switch_structure_bsp(NONE, TRUE);
		game_dispose_from_old_map();
	}

	switch (main_game_load_globals.state)
	{
	case _main_game_load_map:
	case _main_game_load_unknown4:
		main_game_close_caches();
		scenario_unload();
		physical_memory_globals.stage--;
		main_game_load_globals.state= _main_game_load_none;
		break;
	case _main_game_load_texture_cache:
		texture_cache_close();
		physical_memory_globals.stage--;
		main_game_load_globals.state= _main_game_load_none;
		break;
	}
}

/* 0x0018ec20: a map starts to load: what it takes of the memory is its own
stage, unless the stage is there already */
void main_game_load_begin(
	boolean keep_memory)
{
	main_game_load_globals.state= _main_game_load_started;
	if (!keep_memory)
	{
		physical_memory_stage_push();
	}
}

/* 0x0018ec60: the tags of a map are loaded: the caches are opened for it,
unless they are open already */
void main_game_load_end(
	boolean keep_caches,
	const char *map_name)
{
	if (!keep_caches)
	{
		animation_cache_open();
		language_texts_load((struct language_texts *)main_game_language_texts());
		geometry_cache_open();
		texture_cache_globals.use_preloaded_pixels= FALSE;
		texture_cache_globals.flushing= FALSE;
		texture_cache_globals.blocking_count= 0;
		texture_cache_globals.frame_index= TEXTURE_CACHE_FIRST_FRAME_INDEX;
		texture_cache_load_preloaded_pixels();
		texture_cache_open();
		data_make_valid(unknown_00502104);
	}
	main_game_load_globals.state= _main_game_load_map;
	main_game_set_map_name(map_name);
}

/* 0x0018ed00: the map could not be loaded: what was loaded of it is let go
of, and its memory */
void main_game_load_fail(
	boolean loaded)
{
	if (loaded)
	{
		sound_cache_close();
		texture_cache_close();
		geometry_cache_close();
		main_game_language_texts_unload();
		animation_cache_close();
		scenario_tags_unload();
		global_scenario_index= NONE;
		global_scenario= NULL;
		global_game_globals= NULL;
		global_structure_bsp_index= NONE;
		global_structure_bsp= NULL;
		global_structure_lightmap= NULL;
		global_collision_bsp= NULL;
		global_bsp3d= NULL;
	}
	main_game_load_globals.state= _main_game_load_none;
	physical_memory_globals.stage--;
}

/* 0x0018edb0: closes the caches of the map; its tags stay */
void main_game_close_caches(
	void)
{
	sound_cache_close();
	texture_cache_close();
	geometry_cache_close();
	main_game_language_texts_unload();
	animation_cache_close();
	main_game_load_globals.state= _main_game_load_closed;
	main_game_set_map_name("");
}

/* 0x0018ee60: with no map, the texture cache gets all the memory that is
left */
void main_game_open_texture_cache(
	void)
{
	physical_memory_stage_push();
	texture_cache_open();
	main_game_load_globals.state= _main_game_load_texture_cache;
}

/* 0x0018eea0: whether the memory is shared out as for a campaign in a
session of the network game */
boolean main_game_memory_configuration_is_session(
	long configuration)
{
	return configuration>=_main_game_memory_campaign_session &&
		configuration<=_main_game_memory_campaign_session_coop;
}

/* 0x0018eeb0: how the memory is shared out for a game: a campaign has its
own ways, unless this console only follows the game of another */
long main_game_memory_configuration(
	const struct game_options *options)
{
	if (options && options->game_mode==_game_mode_campaign &&
		!(options->simulation>=_game_simulation_predicted && options->simulation<=_game_simulation_distributed_last))
	{
		if (unknown_00467214!=NONE && unknown_0006cd50(unknown_00467214)==_unknown_0006cd50_state1)
		{
			return (options->coop!=0)+_main_game_memory_campaign_session;
		}

		return (options->coop!=0)+_main_game_memory_campaign;
	}

	return _main_game_memory_default;
}

/* 0x0018ef00: if the game that is about to start shares the memory out in
another way than the last, each of those that take memory for a map gives
its own back, and takes it again in the new way */
void main_game_configure_memory(
	const struct game_options *options)
{
	long configuration= main_game_memory_configuration(options);

	if (main_game_load_globals.memory_configuration!=configuration)
	{
		long index;

		if (main_game_load_globals.memory_configuration>0)
		{
			for (index= 0; index<NUMBER_OF_MAIN_GAME_MEMORY_USERS; index++)
			{
				if (main_game_memory_users[index].free_memory)
				{
					main_game_memory_users[index].free_memory();
				}
			}
			physical_memory_globals.stage--;
		}
		physical_memory_stage_push();
		main_game_load_globals.memory_configuration= configuration;
		for (index= 0; index<NUMBER_OF_MAIN_GAME_MEMORY_USERS; index++)
		{
			if (main_game_memory_users[index].allocate_memory)
			{
				main_game_memory_users[index].allocate_memory(configuration);
			}
		}
	}
}

/* 0x0018f000: starts a game on the map that is loaded; whether it could */
boolean main_game_start(
	const struct game_options *options)
{
	game_initialize_for_new_map(options);
	unknown_000d5560(0);
	unknown_000c09c0(main_game_globals->options.game_mode);
	game_create_players();
	if (scenario_switch_structure_bsp(options->unknown120, TRUE))
	{
		main_game_globals->active= TRUE;
		if (unknown_004cf770.unknown0 && !unknown_004cf770.unknownC[UNKNOWN_004CF770_ACTIVE_OFFSET])
		{
			unknown_004cf770.unknownC[UNKNOWN_004CF770_ACTIVE_OFFSET]= TRUE;
		}
		unknown_00158530();
		unknown_00138f10();
		if (main_game_globals->options.simulation!=_game_simulation_predicted &&
			(main_game_globals->options.game_mode==_game_mode_campaign ||
			main_game_globals->options.game_mode==_game_mode_multiplayer))
		{
			unknown_00200d80();
		}

		return TRUE;
	}
	game_dispose_from_old_map();

	return FALSE;
}

/* 0x0018f100: has a map that is not on the hard disk copied (0x00163890,
most likely, shows how it goes): the texture cache is opened for it and
closed after; whether the map is there */
boolean main_game_unknown_0018f100(
	const char *map_name)
{
	boolean result;

	physical_memory_stage_push();
	texture_cache_open();
	main_game_load_globals.state= _main_game_load_texture_cache;
	result= unknown_00163890(map_name, 2);
	texture_cache_close();
	physical_memory_globals.stage--;
	main_game_load_globals.state= _main_game_load_none;

	return result;
}

/* 0x0018f170: asks for the game of the main menu */
void main_game_launch_main_menu(
	struct game_options *options,
	long unknown)
{
	game_options_new_default(options);
	options->game_mode= _game_mode_3;
	strncpy(options->map_name, "scenarios\\ui\\mainmenu\\mainmenu", GAME_MAP_NAME_LENGTH);
	options->map_name[GAME_MAP_NAME_LENGTH-1]= 0;
	memcpy(options->unknown264, &unknown, sizeof(options->unknown264));
	game_options_setup_local_players(1, options);
}

/* ---------- changing the game */

static inline __attribute__((always_inline)) void main_game_clear_main_actions(
	void)
{
	main_globals.switch_structure_bsp= FALSE;
	main_globals.reset_map= FALSE;
	main_globals.revert_map= FALSE;
	main_globals.revert_map_if_possible= FALSE;
	main_globals.save_map= FALSE;
	main_globals.unknown49= FALSE;
}

/* the network game is told that the game changes, in two of its states */
static inline __attribute__((always_inline)) boolean main_game_unknown_00527330_wants_to_know(
	void)
{
	return unknown_00527330 && (unknown_00527334==3 || unknown_00527334==8);
}

/* 0x0018e700: starts the game on the map again, with the options that it
has; the map and its caches stay as they are */
void main_game_restart(
	void)
{
	struct game_options options;

	main_game_load_globals.restarting= TRUE;
	unknown_0014bac0();
	options= main_game_globals->options;
	if (options.unknown120==global_structure_bsp_index)
	{
		game_dispose_from_old_structure_bsp();
	}
	else
	{
		scenario_switch_structure_bsp(NONE, TRUE);
	}
	game_dispose_from_old_map();
	memset(input_globals.gamepad_states, 0, sizeof(input_globals.gamepad_states));
	if (main_game_start(&options))
	{
		main_game_load_globals.restarting= FALSE;
	}
	else
	{
		main_game_load_globals.restarting= FALSE;
		main_game_load_failed();
	}
}

/* 0x0018e790: asks for another game, which starts at the start of a frame;
with no options, for the map to be let go of */
void main_game_change(
	const struct game_options *options)
{
	if (options)
	{
		main_game_load_globals.change_options= *options;
	}
	main_game_load_globals.change_to_nothing= options==NULL;
	main_game_load_globals.change_pending= TRUE;
	main_game_load_globals.change_time= (unsigned long)GetTickCount();
	if (main_game_unknown_00527330_wants_to_know())
	{
		if (options)
		{
			if (options->unknown5)
			{
				return;
			}
			if (options->game_mode==_game_mode_3)
			{
				long unknown;

				memcpy(&unknown, options->unknown264, sizeof(unknown));
				if (unknown>=2 && unknown<=6)
				{
					return;
				}
			}
		}
		unknown_000593e0();
	}
}

/* 0x0018e810: once a frame: starts the game that was asked for, unless
something says to wait (0x0051ea10, 0x00149318) */
void main_game_change_update(
	void)
{
	boolean pending= main_game_load_globals.change_pending;

	if (pending && !main_game_load_globals.change_to_nothing && unknown_0051ea10>0)
	{
		pending= FALSE;
	}
	if (!unknown_00149318() && pending)
	{
		if (main_game_load_globals.change_to_nothing)
		{
			if (main_game_load_globals.state!=_main_game_load_texture_cache)
			{
				main_game_unload();
				main_game_clear_main_actions();
				main_game_configure_memory(NULL);
				main_game_open_texture_cache();
			}
		}
		else
		{
			main_game_change_immediate(&main_game_load_globals.change_options);
		}
		main_game_load_globals.change_pending= FALSE;
		main_game_load_globals.change_to_nothing= FALSE;
		main_game_load_globals.unknown114= 0;
	}
}

/* 0x0018e8b0: lets go of the game and of the map that there are, loads the
map of the new game and starts it; whether it could. With no options, the
map is let go of and the texture cache is left open. If the map cannot be
loaded or the game started, the game goes to the main menu */
boolean main_game_change_immediate(
	const struct game_options *options)
{
	main_game_unload();
	main_game_clear_main_actions();
	main_game_configure_memory(options);
	if (!options)
	{
		main_game_open_texture_cache();

		return TRUE;
	}

	memset(input_globals.gamepad_states, 0, sizeof(input_globals.gamepad_states));
	if (main_game_load_map(options) && main_game_start(options))
	{
		if (options->game_mode==_game_mode_campaign)
		{
			main_game_campaign_started(options);
		}
		else if (options->game_mode==_game_mode_3)
		{
			if (!unknown_0012bf90())
			{
				long unknown;

				memcpy(&unknown, options->unknown264, sizeof(unknown));
				unknown_001483c3(unknown);
			}
		}

		return TRUE;
	}
	main_game_load_failed();

	return FALSE;
}

/* 0x0018eaa0: loads the map of a game; whether it could. A map that is not
on the hard disk yet is copied first (0x0018f100 waits for it). If there is
no map to load, the game is asked to let go of everything */
boolean main_game_load_map(
	const struct game_options *options)
{
	const char *map_name= options->map_name;
	boolean loaded= FALSE;

	switch (cached_map_file_status(map_name))
	{
	case _cached_map_file_status_none:
	case _cached_map_file_status_pending:
	case _cached_map_file_status_copying:
		if (!main_game_unknown_0018f100(map_name))
		{
			goto failed;
		}
		/* falls through */
	case _cached_map_file_status_cached:
		main_game_loading_map= TRUE;
		main_game_loading_map_time= (unsigned long)GetTickCount();
		main_game_load_begin(FALSE);
		if (scenario_load(map_name))
		{
			unknown_0011c840();
			main_game_load_end(FALSE, map_name);
			loaded= TRUE;
		}
		else
		{
			main_game_load_fail(FALSE);
		}
		main_game_loading_map= FALSE;
		break;
	case _cached_map_file_status_failed:
	failed:
		unknown_0051ea00= TRUE;
		main_globals.unknown1= TRUE;
		main_game_load_globals.change_to_nothing= TRUE;
		main_game_load_globals.change_pending= TRUE;
		main_game_load_globals.change_time= (unsigned long)GetTickCount();
		if (main_game_unknown_00527330_wants_to_know())
		{
			unknown_000593e0();
		}
		break;
	}

	return loaded;
}

/* 0x0018efa0: a game could not be started: the game goes to the main menu.
If it is the main menu that cannot be started, the texture cache is left
open and nothing else */
void main_game_load_failed(
	void)
{
	main_game_unload();
	if (!main_game_load_failing)
	{
		struct game_options options;

		main_game_load_failing= TRUE;
		main_game_launch_main_menu(&options, 0);
		main_game_change_immediate(&options);
		main_game_load_failing= FALSE;
	}
	else
	{
		main_game_open_texture_cache();
		main_files_damaged();
	}
}

/* 0x0018f0a0: after a game of the campaign is started: if it is to go on
from a saved game, its state is loaded: that of the only player there is */
void main_game_campaign_started(
	const struct game_options *options)
{
	if (options->game_mode==_game_mode_campaign && options->unknown18!=NONE)
	{
		unknown_0019040d(options->unknown18);
	}
	if (!options->coop && unknown_0054e7cc)
	{
		long saved_game= NONE;
		long profile_index= unknown_0018fada();

		if (profile_index!=NONE)
		{
			const byte *profile= unknown_0054e8e0[profile_index];

			if (profile[0]&0x10)
			{
				memcpy(&saved_game, profile+UNKNOWN_0054E8E0_SAVED_GAME_OFFSET, sizeof(saved_game));
			}
		}
		game_state_load_saved_game(saved_game);
	}
}

/* 0x0018f1c0: goes to the main menu: asks for its game, unless it is the
one that is on */
void main_game_goto_main_menu(
	long unknown)
{
	struct game_options options;

	if (main_game_globals && main_game_globals->active && main_game_globals->options.game_mode==_game_mode_3)
	{
		unknown_001483c3(unknown);

		return;
	}
	main_game_launch_main_menu(&options, unknown);
	main_game_change(&options);
}

/* 0x0018f260: memory for an allocation, taken from the texture cache: it is
waited for a short time. After the functions of the allocation are the two
procedures that the cache calls about the memory */
void *ALLOCATION_CONVENTION main_game_texture_allocation_allocate(
	struct allocation *allocation,
	unsigned long size)
{
	const struct main_game_texture_allocation *texture_allocation=
		(const struct main_game_texture_allocation *)allocation;

	return texture_cache_steal_memory(_texture_cache_steal_wait_short, (long)size, 0,
		texture_allocation->idle_proc, texture_allocation->reclaim_proc);
}

/* 0x0018f350: gives the memory back to the texture cache */
void ALLOCATION_CONVENTION main_game_texture_allocation_deallocate(
	struct allocation *allocation,
	void *memory)
{
	(void)allocation;

	texture_cache_return_memory(memory);
}

/* 0x0018e960: a map of the campaign is over: asks for a game on the next
one, with the options of this one, or goes to the main menu if there is
none. In a session of the network game, the server is told of the game that
ends */
void main_game_next_map(
	void)
{
	struct game_options options;
	byte report[CAMPAIGN_REPORT_SIZE];

	memcpy(&options, &main_game_globals->options, sizeof(options));
	if (unknown_00467214!=NONE && unknown_0006cd50(unknown_00467214)==_unknown_0006cd50_state1)
	{
		unknown_001c5ca0(&options, report);
		unknown_000b3ed0(report, sizeof(report), "application/x-halo2campaign");
	}

	if (options.unknown14!=NONE)
	{
		long map= unknown_0019c440(options.unknown14, options.unknown18);

		if (map!=NONE)
		{
			const char *data= unknown_0019c270(options.unknown14, map);

			if (data)
			{
				const char *map_name= data+CAMPAIGN_MAP_PATH_OFFSET;
				unsigned long seed;

				options.unknown18= map;
				strncpy(options.map_name, map_name, GAME_MAP_NAME_LENGTH);
				options.map_name[GAME_MAP_NAME_LENGTH-1]= 0;
				options.unknown12D[0]= 0;
				seed= (unsigned long)time(NULL);
				seed^= (unsigned long)rand();
				seed^= GetTickCount();
				options.random_seed= seed;
				unknown_00152450(&options);
				unknown_00152660(&options);
				if (!unknown_00063f50(options.unknown14, map, map_name))
				{
					main_game_change(&options);
				}

				return;
			}
		}
	}
	main_game_goto_main_menu(_main_menu_unknown7);
}
