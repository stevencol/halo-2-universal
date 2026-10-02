/*
MAIN_GAME.H

Loading a map and letting go of it: the memory that each part of the game
takes for a map, the caches that are opened for it, and the game that is
started on it. Worked out from halo2ship.exe; the names are guesses.
*/

#ifndef __MAIN_GAME_H
#define __MAIN_GAME_H

#include "cseries/cseries.h"
#include "cache/texture_cache.h"
#include "game/game.h"
#include "memory/data.h"

/* ---------- constants */

enum
{
	MAIN_GAME_MAP_NAME_LENGTH= 0x104,
	NUMBER_OF_MAIN_GAME_MEMORY_USERS= 8
};

/* how far the loading of a map is */
enum
{
	_main_game_load_none,
	/* started: the map is being read */
	_main_game_load_started,
	/* the map is loaded, and the caches are open */
	_main_game_load_map,
	/* the caches were closed; the tags are still there */
	_main_game_load_closed,
	_main_game_load_unknown4,
	/* no map: the texture cache alone is open, for what is shown meanwhile */
	_main_game_load_texture_cache
};

/* how the memory is shared out among those that take it for a map: by the
kind of game */
enum
{
	_main_game_memory_none,
	/* anything but a campaign */
	_main_game_memory_default,
	/* a campaign over the network (0x0006cd50 says 1): alone, and with a
	second player */
	_main_game_memory_campaign_session,
	_main_game_memory_campaign_session_coop,
	_main_game_memory_campaign,
	_main_game_memory_campaign_coop
};

/* ---------- structures */

#define MAIN_GAME_CALLBACK __attribute__((stdcall))

/* one of those that take memory when a map is loaded: either function can
be missing */
struct main_game_memory_user
{
	void (*free_memory)(void);
	void (MAIN_GAME_CALLBACK *allocate_memory)(long configuration);
};

/* 0x004ed290 */
struct main_game_load_globals
{
	long memory_configuration;
	long state;
	char map_name[MAIN_GAME_MAP_NAME_LENGTH];
	/* the game on the map is being started again: what unknown_004cf770 has
	going is left alone */
	boolean restarting;
	/* another game was asked for: it is started at the start of a frame */
	boolean change_pending;
	/* and it is none: the map is let go of */
	boolean change_to_nothing;
	byte pad10F;
	/* GetTickCount: when it was asked for */
	unsigned long change_time;
	long unknown114;
	struct game_options change_options;
};

/* 0x004cf770: of the network game, most likely; what main_game_unload and
main_game_start look at */
struct unknown_004cf770
{
	boolean unknown0;
	byte unknown1[9];
	boolean unknownA;
	byte unknownB;
	/* a boolean at +0x2F says that a game is on */
	byte *unknownC;
};

typedef char verify_main_game_load_globals_size[
	sizeof(struct main_game_load_globals) == 0x1230 ? 1 : -1];
typedef char verify_main_game_load_globals_change_options_offset[
	offsetof(struct main_game_load_globals, change_options) == 0x118 ? 1 : -1];
#if defined(__i386__)
typedef char verify_unknown_004cf770_size[
	sizeof(struct unknown_004cf770) == 0x10 ? 1 : -1];
#endif

/* an allocation whose memory is lent by the texture cache */
struct main_game_texture_allocation
{
	struct allocation allocation;
	texture_cache_stolen_memory_proc idle_proc;
	texture_cache_stolen_memory_proc reclaim_proc;
};

/* ---------- prototypes/MAIN_GAME.C */

void main_game_restart(void);
void main_game_change(const struct game_options *options);
void main_game_change_update(void);
boolean main_game_change_immediate(const struct game_options *options);
boolean main_game_load_map(const struct game_options *options);
void main_game_load_failed(void);
void main_game_campaign_started(const struct game_options *options);
void main_game_goto_main_menu(long unknown);
void main_game_next_map(void);
void *ALLOCATION_CONVENTION main_game_texture_allocation_allocate(struct allocation *allocation, unsigned long size);
void ALLOCATION_CONVENTION main_game_texture_allocation_deallocate(struct allocation *allocation, void *memory);
void main_game_unload(void);
void main_game_load_begin(boolean keep_memory);
void main_game_load_end(boolean keep_caches, const char *map_name);
void main_game_load_fail(boolean loaded);
void main_game_close_caches(void);
void main_game_open_texture_cache(void);
boolean main_game_memory_configuration_is_session(long configuration);
long main_game_memory_configuration(const struct game_options *options);
void main_game_configure_memory(const struct game_options *options);
boolean main_game_start(const struct game_options *options);
boolean main_game_unknown_0018f100(const char *map_name);
void main_game_launch_main_menu(struct game_options *options, long unknown);

/* ---------- globals */

extern struct main_game_load_globals main_game_load_globals;
extern struct main_game_memory_user main_game_memory_users[NUMBER_OF_MAIN_GAME_MEMORY_USERS];
extern struct unknown_004cf770 unknown_004cf770;
extern long unknown_00467214;
extern boolean main_game_loading_map;
extern unsigned long main_game_loading_map_time;
extern boolean unknown_0051ea00;
extern long unknown_0051ea10;
extern boolean unknown_00527330;
extern long unknown_00527334;
extern boolean main_game_load_failing;
extern boolean unknown_0054e7cc;
extern const struct allocation_functions main_game_texture_allocation_functions;
extern struct data_array *unknown_00502104;

#endif
