/*
MAIN_GAME.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The names are guesses. Not here: 0x0018efa0, 0x0018f0a0 and 0x0018f1c0 on.
*/

#include "main_game.h"

#include "cache/animation_cache.h"
#include "cache/cache_files.h"
#include "cache/geometry_cache.h"
#include "cache/texture_cache.h"
#include "interface/user_interface_memory.h"
#include "memory/data.h"
#include "memory/physical_memory.h"
#include "scenario/scenario.h"

#include <string.h>

/* ---------- prototypes of functions of the console's libraries */

/* 0x0033039a */
long XGetLanguage(void);

/* ---------- prototypes of functions not decompiled yet */

/* those that take memory when a map is loaded: what gives it back, and what
takes it */
void unknown_00081f80(void);
void unknown_00081780(long configuration);
void unknown_00215810(void);
void unknown_002157e0(long configuration);
void unknown_000533e0(void);
void unknown_00053310(long configuration);
void unknown_000b3db0(void);
void unknown_000b3d30(long configuration);
void unknown_0021ebe0(void);
void unknown_0021ebc0(long configuration);

/* 0x000693a0: ends what unknown_004cf770 has going; of the network game */
void unknown_000693a0(void *unknown, void *also);

/* 0x0006cd50: the state of a session of the network game */
long unknown_0006cd50(long session_index);

/* 0x001a01e0: reads the texts of a language of the globals tag */
void unknown_001a01e0(void *language_texts);

/* 0x002186f0: before the caches of a map are closed */
void unknown_002186f0(void);

/* 0x0011be50: after the caches of a map are closed */
void unknown_0011be50(void);

/* what a new game sets up after its systems */
void unknown_000d5560(long unknown);
void unknown_000c09c0(long game_mode);
void unknown_00137e40(void);
void unknown_00158530(void);
void unknown_00138f10(void);
void unknown_00200d80(void);

/* 0x00163890 */
boolean unknown_00163890(long unknown, long two);

/* 0x00138640: asks for a game with these options */
void unknown_00138640(long one, struct game_options *options);

/* ---------- constants */

enum
{
	/* in the globals tag: where the texts of the first language are, and
	how much each language has: an address, a size and whether they are
	loaded */
	GAME_GLOBALS_LANGUAGE_TEXTS_OFFSET= 0x188,
	GAME_GLOBALS_LANGUAGE_TEXTS_SIZE= 0x1C,
	GAME_GLOBALS_LANGUAGE_TEXTS_LOADED_OFFSET= 0x18,

	/* where unknown_004cf770 says that a game is on */
	UNKNOWN_004CF770_ACTIVE_OFFSET= 0x2F,

	_unknown_0006cd50_state1= 1
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
	{unknown_00215810, (void (MAIN_GAME_CALLBACK *)(long))unknown_002157e0},
	{unknown_000533e0, (void (MAIN_GAME_CALLBACK *)(long))unknown_00053310},
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
				!main_game_load_globals.unknown10C)
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
		unknown_0011be50();
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
		unknown_001a01e0(main_game_language_texts());
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
		unknown_002186f0();
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
	unknown_002186f0();
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
	unknown_00137e40();
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

/* 0x0018f100: does something that needs the texture cache when no map is
loaded (0x00163890): the cache is opened for it and closed after */
boolean main_game_unknown_0018f100(
	long unknown)
{
	boolean result;

	physical_memory_stage_push();
	texture_cache_open();
	main_game_load_globals.state= _main_game_load_texture_cache;
	result= unknown_00163890(unknown, 2);
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
	unknown_00138640(1, options);
}
