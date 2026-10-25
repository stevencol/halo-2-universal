/*
MAIN.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The game's main loop. Not here yet: most of what the game can ask it for,
and the parts of the game that a frame calls. The names are guesses, after those of
Halo 1's main.c.
*/

#include "text/unicode_string_list.h"
#include "main.h"
#include <string.h>

#include "cache/geometry_cache.h"
#include "cache/texture_cache.h"
#include "main/main_game.h"
#include "networking/network_life_cycle.h"
#include "networking/network_globals.h"
#include "networking/network_link.h"

#include "cache/cache_files.h"
#include "input/input.h"
#include "input/input_abstraction.h"
#include "cseries/async.h"
#include "game/game.h"
#include "main/global_preferences.h"
#include "saved games/game_state.h"
#include "scenario/scenario.h"
#include "scenario/scenario_definitions.h"
#include "objects/objects.h"
#include "shell/shell.h"
#include "tag_files/tag_groups.h"
#include "text/font_cache.h"
#include "text/unicode.h"

/* ---------- prototypes of functions of the console's libraries */

/* 0x003314b0 */
long GetTickCount(void);

/* ---------- prototypes of functions not decompiled yet */

/* 0x00163b60: whether something stops the game from going back to a
checkpoint now */
boolean main_map_precache_next_map_not_ready(void);

/* 0x0012b2a0: what Direct3D is given to call; it keeps how long a frame
took */
void unknown_0012b2a0(void);

/* 0x0014a224: something of a game whose mode is 3 */
boolean user_interface_unknown_0014a224(void);

/* what a frame calls, in the order that it calls them. Their names are not
known; the three with a name are what the game asked the loop for */
void unknown_002237d4(void);
void unknown_0012bbd0(void);
void main_map_precache_update(void);
void unknown_001479c3(float seconds);
void unknown_00155f80(void);
void simulation_idle(void);
boolean simulation_in_progress(void);
void unknown_00146660(float seconds, float *unknown_real, long *unknown_long);
void unknown_001870a0(float seconds, float unknown_real);
void unknown_001554b0(float seconds);
void unknown_0016f280(float seconds);
void unknown_00222a70(float seconds);
void unknown_00223240(void);
/* 0x00125b40 */
void sound_update(void);
void simulation_update(void);
void unknown_00222d80(void);

/* 0x0024cdaf and 0x0024cbee: the messages that the game shows the players.
The names are guesses */
void hud_messages_unknown_0024cdaf(void);
void hud_print_message(long local_player_index, const wchar *text);

/* 0x00190e37: what main_unknown4E_private does before its message */
void user_interface_controllers_write_changed_profiles(void);

/* 0x001896c0: plays a sound, as loud as it is told */
void __attribute__((stdcall)) sound_unknown_001896c0(long sound_index, real scale);

/* ---------- constants */

enum
{
	HUD_MESSAGE_LENGTH= 256,

	/* the string ids of the messages: hud_autosave_end, hud_loading_end and
	hud_saving_end */
	_string_id_hud_autosave= 0x120006A0,
	_string_id_hud_autosave_end= 0x100006A1,
	_string_id_hud_loading= 0x110006A2,
	_string_id_hud_loading_end= 0x0F0006A3,
	_string_id_hud_saving= 0x100006A4,
	_string_id_hud_saving_end= 0x0E0006A5,

	MINIMUM_FRAME_VBLANKS= 2,
	DEFAULT_REFRESH_RATE= 60,
	UNKNOWN_0051EA14_OFFSET= 0xBEF8
};

#define MAXIMUM_FRAME_SECONDS 10.0f

/* ---------- structures */

/* 0x00510c94 points to it: where the text of the messages comes from. What
it is is not known */
struct unknown_00510c94
{
	byte unknown0[0x394];
	/* the sound that is played when the game is saved; NONE if none */
	long save_sound_index;
	byte unknown398[0x64];
	long unknown3FC;
};

/* 0x00510c50 points to it. What it is is not known */
struct unknown_00510c50
{
	byte unknown0[5];
	boolean unknown5;
	boolean unknown6;
	byte unknown7[0x1B];
	boolean unknown22;
};

/* ---------- globals */

/* 0x00547f28 */
struct main_globals main_globals;

/* 0x004ed39d, 0x004ed39e and 0x004ed3a0: of another part, told that files of
the game are damaged, and when; what they are is not known */
boolean unknown_004ed39d;
boolean unknown_004ed39e;
long unknown_004ed3a0;

/* 0x004e6388 */
struct main_time_globals main_time_globals;

/* 0x00485ab0: goes up with each vertical blank of the picture. What makes
it go up is not decompiled, and the name is a guess: a frame's seconds are
what it went up, divided by the refresh rate */
long long main_vblank_count;

/* 0x00485ac8: how the frames are shown. The name is a guess */
struct display_settings display_settings;

/* 0x0043ed80: those that the game starts with: a vertical blank a frame */
const struct display_settings display_settings_default= { 0, 0, 1, 3, 1 };

/* what a frame looks at, of other parts of the game. What they are is not
known: 0x0055c14d, 0x0051ea14, 0x0055c150, 0x004d8ba0, 0x00510548,
0x0051054c and 0x00510560 */
boolean unknown_0055c14d;
void *unknown_0051ea14;
unsigned long unknown_0055c150;
boolean unknown_004d8ba0;
boolean unknown_00510548;
long unknown_0051054c;
long unknown_00510560;

/* 0x00510c50 */
struct unknown_00510c50 *unknown_00510c50;

/* 0x00509415: what main_unknown_000335c0 gives when unknown_00510c50 has
its byte at 5 */
boolean unknown_00509415;

/* 0x004e8c20 */
struct players_globals *players_globals;

/* 0x004e8c24: the players. The name is a guess */
struct data_array *player_data;

/* 0x00510c94 */
struct unknown_00510c94 *unknown_00510c94;

/* 0x0055c14c: what main_cancel_map_copy_and_wait waits for. What it is is
not known */
boolean unknown_0055c14c;

/* ---------- code */

/* 0x0012b690: the game, from when it is set up to when it ends */
void main_loop(
	void)
{
	main_globals.unknown0= TRUE;
	main_time_initialize();
	game_initialize();

	while (!*(volatile boolean *)&main_globals.exit_game)
	{
		main_loop_body();
	}

	main_game_unload();
	game_dispose();
}

/* 0x0012b070: starts to count the time */
void main_time_initialize(
	void)
{
	long index;

	main_time_globals.tick_count= (unsigned long)GetTickCount();
	main_time_globals.tick_count_high= 0;
	main_time_globals.last_vblank= 0;
	d3d_device->frame_callback= unknown_0012b2a0;

	for (index= 0; index<NUMBER_OF_MAIN_FRAME_TIME_SAMPLES; index++)
	{
		main_time_globals.frame_time_samples.samples[index]= 0;
	}
	main_time_globals.frame_time_samples.next_index= 0;
}

/* 0x0012b0e0: how many seconds the frame that starts is to count for: from
the last frame to now, or to when the frame is due if the game is given a
number of vertical blanks for each. From 0 to 10 */
float main_time_update(
	void)
{
	struct main_time_globals *time= &main_time_globals;
	long long last_vblank= time->last_vblank;
	long long vblank= time->target_vblank;
	unsigned long tick_count;
	unsigned long tick_count_high;
	long game_time= 0;
	long refresh_rate;
	float seconds;

	/* the game does nothing with this one */
	GetTickCount();

	if (last_vblank>vblank)
	{
		vblank= last_vblank;
	}
	if (main_game_globals && main_game_globals->active)
	{
		game_time= (long)game_time_globals->time;
	}

	if (display_settings.frame_vblanks &&
		main_game_globals &&
		main_game_globals->active &&
		(!time->unknown0 || time->unknown1))
	{
		short frame_vblanks= display_settings.frame_vblanks;

		time->unknown32= frame_vblanks;
		if (main_game_globals->active)
		{
			if (main_game_globals->options.game_mode!=_game_mode_3 || user_interface_unknown_0014a224())
			{
				frame_vblanks= time->unknown32;
				if (frame_vblanks<=MINIMUM_FRAME_VBLANKS)
				{
					frame_vblanks= MINIMUM_FRAME_VBLANKS;
				}
				time->unknown32= frame_vblanks;
			}
			else
			{
				frame_vblanks= time->unknown32;
			}
		}
		time->frame_vblanks= frame_vblanks;
		vblank+= frame_vblanks;
	}

	/* the tick count goes back to zero every 49 days */
	tick_count= (unsigned long)GetTickCount();
	tick_count_high= time->tick_count_high;
	if (tick_count<time->tick_count)
	{
		tick_count_high++;
	}

	if (main_vblank_count>vblank)
	{
		vblank= main_vblank_count;
	}
	refresh_rate= xbox_video_settings.refresh_rate;
	if (refresh_rate<=0)
	{
		refresh_rate= DEFAULT_REFRESH_RATE;
	}

	/* a division of the x87 registers, in the precision that the game sets
	them to */
	seconds= (float)((long double)(vblank-last_vblank)/(long double)refresh_rate);
	if (0.0f>seconds)
	{
		seconds= 0.0f;
	}
	else if (seconds>MAXIMUM_FRAME_SECONDS)
	{
		seconds= MAXIMUM_FRAME_SECONDS;
	}

	time->last_vblank= vblank;
	time->tick_count= tick_count;
	time->tick_count_high= tick_count_high;
	time->game_time= game_time;
	time->vblank_count= main_vblank_count;

	return seconds;
}

/* 0x0012b450: a frame of the game. First, what the game asked the loop for;
then the parts that run with or without a game; then, unless the game is
paused, the time and the parts of a game */
void main_loop_body(
	void)
{
	float unknown_real= 0.0f;
	long unknown_long= 0;
	float frame_seconds;
	float seconds;

	if (main_globals.unknown2)
	{
		main_globals.unknown2= FALSE;
		unknown_002237d4();
	}
	if (main_game_load_globals.state!=_main_game_load_map &&
		main_game_load_globals.state!=_main_game_load_texture_cache && !main_game_load_globals.change_pending)
	{
		main_game_goto_main_menu(0);
	}
	main_game_change_update();

	if (main_globals.revert_map_if_possible)
	{
		main_revert_map_if_possible_private();
	}
	if (main_globals.revert_map)
	{
		main_revert_map_private();
	}
	if (main_globals.reset_map)
	{
		main_reset_map_private();
	}
	if (main_globals.switch_structure_bsp)
	{
		main_switch_structure_bsp_private();
	}
	if (main_globals.save_map)
	{
		main_save_map_private();
	}
	if (main_globals.unknown49)
	{
		unknown_0012bbd0();
	}
	if (main_globals.load_core && main_game_globals && main_game_globals->active)
	{
		game_state_load_core(main_globals.core_name);
		main_globals.load_core= FALSE;
	}
	if (main_globals.unknown4E)
	{
		main_unknown4E_private();
	}

	input_update();
	input_abstraction_update();
	if (unknown_0055c14d && unknown_0051ea14 && (unknown_0055c150&1))
	{
		saved_game_files_find_unit_files(0,
			(struct saved_game_file_table *)((byte *)unknown_0051ea14+UNKNOWN_0051EA14_OFFSET));
		unknown_0055c150&= ~1UL;
	}
	/* the preferences changed: they are written */
	if (global_preferences_globals.dirty)
	{
		global_preferences_flush();
	}
	font_cache_frame();
	async_globals.tasks_added= async_task_count();
	/* a step of the copy of a map to the hard disk, if the last one is done */
	if (cached_map_file_copy_globals.done)
	{
		cached_map_file_copy_update();
	}
	main_map_precache_update();

	if (application_paused)
	{
		return;
	}

	frame_seconds= main_time_update();
	seconds= frame_seconds;
	if (application_paused)
	{
		seconds= 0.0f;
	}
	unknown_001479c3(frame_seconds);
	unknown_00155f80();
	if (unknown_004d8ba0)
	{
		unknown_00510548= TRUE;
		unknown_0051054c= GetTickCount();
		network_link_receive((byte *)unknown_00510560);
		unknown_00510548= FALSE;
		unknown_0051054c= GetTickCount();
	}
	network_update();
	simulation_idle();
	network_life_cycle_update();

	if (simulation_in_progress())
	{
		unknown_00146660(seconds, &unknown_real, &unknown_long);
		unknown_001870a0(seconds, unknown_real);
		game_run_ticks(unknown_long, &unknown_real);
		network_send();
		game_frame(unknown_real);
		unknown_001554b0(seconds);
		unknown_0016f280(seconds);
		unknown_00222a70(seconds);
		unknown_00223240();
		sound_update();
	}
	else
	{
		simulation_update();
		network_send();
		unknown_00222d80();
		unknown_00223240();
		sound_update();
	}
}

/* 0x0012ba30: goes back to the last checkpoint */
void main_revert_map_private(
	void)
{
	unsigned long flags= main_globals.revert_flag0!=0;

	if (main_globals.revert_flag1)
	{
		flags|= 2;
	}
	/* the structure bsp is that of the checkpoint */
	if (main_globals.switch_structure_bsp)
	{
		main_globals.switch_structure_bsp= FALSE;
		main_globals.switch_structure_bsp_index= NONE;
	}

	game_state_revert(flags);
	main_globals.revert_flag0= FALSE;
	main_globals.revert_flag1= FALSE;
	main_globals.revert_map= FALSE;
}

static inline __attribute__((always_inline)) void main_get_message(
	unsigned long string_id,
	wchar *text)
{
	text[0]= 0;
	if (unknown_00510c94 && unknown_00510c94->unknown3FC!=NONE)
	{
		unicode_string_list_get_string(unknown_00510c94->unknown3FC, string_id, text);
	}
}

static inline __attribute__((always_inline)) long main_first_local_player(
	void)
{
	long index;

	for (index= 0; index<MAXIMUM_LOCAL_PLAYERS; index++)
	{
		if (players_globals->local_players[index]!=NONE)
		{
			return index;
		}
	}

	return NONE;
}

/* 0x0012bb20: makes a checkpoint, and tells each player of this console, in
the campaign */
void main_save_map_private(
	void)
{
	if (main_game_globals && main_game_globals->active)
	{
		game_state_save();
	}

	if (main_game_globals->options.game_mode==_game_mode_campaign)
	{
		long index;

		hud_messages_unknown_0024cdaf();
		for (index= 0; index<MAXIMUM_LOCAL_PLAYERS; index++)
		{
			if (players_globals->local_players[index]!=NONE)
			{
				wchar text[HUD_MESSAGE_LENGTH];

				main_get_message(_string_id_hud_autosave_end, text);
				hud_print_message(index, text);
			}
		}
	}
	main_globals.save_map= FALSE;
}

/* 0x0012bd40: makes another structure bsp of the scenario the one that is
loaded, and tells the first player of this console, in the campaign */
void main_switch_structure_bsp_private(
	void)
{
	boolean switched= scenario_switch_structure_bsp((unsigned short)main_globals.switch_structure_bsp_index, TRUE);

	if (main_game_globals->options.game_mode==_game_mode_campaign)
	{
		wchar text[HUD_MESSAGE_LENGTH];
		long local_player_index;

		hud_messages_unknown_0024cdaf();
		local_player_index= main_first_local_player();
		main_get_message(_string_id_hud_loading_end, text);
		hud_print_message(local_player_index, text);
	}
	if (!switched)
	{
		main_game_load_failed();
	}
	main_globals.switch_structure_bsp= FALSE;
	main_globals.switch_structure_bsp_index= NONE;
}

/* 0x0012bf00: what is done when files of the game are found to be damaged:
several things of the game are told so, and when, and something more is
done in two states of another part. Which files is not known; the fonts use
it. The name is a guess */
void main_files_damaged(
	void)
{
	main_globals.unknown1= TRUE;
	unknown_004ed39e= TRUE;
	unknown_004ed39d= TRUE;
	unknown_004ed3a0= GetTickCount();
	if (unknown_00527330 && (unknown_00527334==3 || unknown_00527334==8))
	{
		network_life_cycle_end();
	}
}

/* 0x0012b980: tells the first player of this console that the game is
being saved, in the campaign, once until it is saved. The name is a guess */
void main_print_saving_message(
	void)
{
	if (main_globals.unknown4E)
	{
		return;
	}
	main_globals.unknown4E= TRUE;
	if (main_game_globals && main_game_globals->active &&
		main_game_globals->options.game_mode==_game_mode_campaign)
	{
		wchar text[HUD_MESSAGE_LENGTH];
		long local_player_index;

		hud_messages_unknown_0024cdaf();
		local_player_index= main_first_local_player();
		main_get_message(_string_id_hud_saving, text);
		hud_print_message(local_player_index, text);
	}
}

/* 0x0012b790: the game is to be saved at the start of the next frame; in the
campaign a sound is played and each player of this console is told. As
Halo 1's of this name */
void main_save_map(
	void)
{
	main_globals.save_map= TRUE;
	if (main_game_globals->options.game_mode==_game_mode_campaign)
	{
		long index;

		hud_messages_unknown_0024cdaf();
		if (unknown_00510c94->save_sound_index!=NONE)
		{
			sound_unknown_001896c0(unknown_00510c94->save_sound_index, 1.0f);
		}
		for (index= 0; index<MAXIMUM_LOCAL_PLAYERS; index++)
		{
			if (players_globals->local_players[index]!=NONE)
			{
				wchar text[HUD_MESSAGE_LENGTH];

				main_get_message(_string_id_hud_autosave, text);
				hud_print_message(index, text);
			}
		}
	}
}

/* 0x0012b850: another structure bsp of the scenario is to be loaded at the
start of the next frame, and in the campaign the first player of this
console is told. Asking for the one that is loaded undoes what was asked
for before, and the player is told that it is loaded; asking for it when
nothing was asked for does nothing. An index that is not of the scenario
does nothing. As Halo 1's of this name */
void main_switch_structure_bsp(
	short structure_bsp_index)
{
	const struct tag_block *structure_bsps;
	unsigned long string_id;
	wchar text[HUD_MESSAGE_LENGTH];
	long local_player_index;

	if (structure_bsp_index<0)
	{
		return;
	}
	structure_bsps= (const struct tag_block *)((const byte *)global_scenario+SCENARIO_STRUCTURE_BSPS_OFFSET);
	if (structure_bsp_index>=structure_bsps->count)
	{
		return;
	}
	if (structure_bsp_index==global_structure_bsp_index)
	{
		if (!main_globals.switch_structure_bsp)
		{
			return;
		}
		main_globals.switch_structure_bsp_index= structure_bsp_index;
		main_globals.switch_structure_bsp= FALSE;
		string_id= _string_id_hud_loading_end;
	}
	else
	{
		main_globals.switch_structure_bsp_index= structure_bsp_index;
		main_globals.switch_structure_bsp= TRUE;
		string_id= _string_id_hud_loading;
	}
	if (main_game_globals->options.game_mode!=_game_mode_campaign)
	{
		return;
	}
	hud_messages_unknown_0024cdaf();
	local_player_index= main_first_local_player();
	main_get_message(string_id, text);
	hud_print_message(local_player_index, text);
}

/* 0x0012bdf0: does something that is not known and tells the first player of
this console that the game was saved, in the campaign */
void main_unknown4E_private(
	void)
{
	user_interface_controllers_write_changed_profiles();
	main_globals.unknown4E= FALSE;

	if (main_game_globals && main_game_globals->active &&
		main_game_globals->options.game_mode==_game_mode_campaign)
	{
		wchar text[HUD_MESSAGE_LENGTH];
		long local_player_index;

		hud_messages_unknown_0024cdaf();
		local_player_index= main_first_local_player();
		main_get_message(_string_id_hud_saving_end, text);
		hud_print_message(local_player_index, text);
	}
}

/* 0x0012ba90: starts the map again, if a game goes on and its time is not
stopped */
void main_reset_map_private(
	void)
{
	if (main_game_globals && main_game_globals->active)
	{
		if (!game_time_globals->initialized || !game_time_globals->paused)
		{
			main_game_restart();
			main_globals.reset_map= FALSE;
		}
	}
}

/* 0x0012bad0: asks for the game to go back to the last checkpoint, if it
can */
void main_revert_map_if_possible_private(
	void)
{
	boolean possible= unknown_00510c50->unknown6;

	if (possible)
	{
		if (unknown_00510c50->unknown22)
		{
			possible= !main_map_precache_next_map_not_ready();
		}
		main_globals.revert_map_if_possible= FALSE;
		if (possible)
		{
			main_globals.revert_flag1= FALSE;
			main_globals.revert_map= TRUE;
			main_globals.revert_flag0= TRUE;
		}
	}
	else
	{
		main_globals.revert_map_if_possible= FALSE;
	}
}

/* 0x0012be90: whether the loop has something to do before the next frame */
boolean main_action_pending(
	void)
{
	boolean pending= main_game_load_globals.change_pending!=0;

	if (main_globals.switch_structure_bsp ||
		main_globals.reset_map ||
		main_globals.revert_map ||
		main_globals.revert_map_if_possible ||
		main_globals.save_map ||
		main_globals.unknown49 ||
		main_globals.unknown4E ||
		texture_cache_globals.blocking_count>0 ||
		geometry_cache_globals.blocking_count>0)
	{
		pending= TRUE;
	}

	return pending;
}

/* 0x0012bf40: stops the copy of a map to the hard disk and waits for
something that is not known */
void main_cancel_map_copy_and_wait(
	void)
{
	cached_map_file_copy_cancel();
	while (!*(volatile boolean *)&unknown_0055c14c)
	{
		SwitchToThread();
		sound_idle();
	}
}

/* 0x000335c0: unknown_00509415 if there is unknown_00510c50 and it has its
byte at 5; true if not */
boolean main_unknown_000335c0(
	void)
{
	if (unknown_00510c50 && unknown_00510c50->unknown5)
	{
		return unknown_00509415;
	}

	return TRUE;
}

/* 0x0012b3c0: FALSE unless the display settings have frames to wait for
and a game is active; then TRUE, or the second byte of the time globals when
their first is set */
boolean main_unknown_0012b3c0(
	void)
{
	if (!display_settings.frame_vblanks || !main_game_globals || !main_game_globals->active)
	{
		return FALSE;
	}

	return main_time_globals.unknown0 ? main_time_globals.unknown1 : TRUE;
}

/* 0x00138a40: the game globals get at 0x11FA and 0x11FC the cluster of the
point at 0x28 of an element (0x40 bytes, of the block at 0x1EC) of the
scenario: 2 and the cluster, or 0 for NONE or a point in no cluster */
void main_game_unknown_00138a40(
	short index)
{
	struct scenario_location location;

	if (index==NONE)
	{
		main_game_globals->unknown11FA= 0;
		return;
	}
	scenario_location_from_point(&location,
		(const real_point3d *)(*(const byte *const *)((const byte *)global_scenario+0x1EC)+index*0x40+0x28));
	if (location.cluster_index==NONE)
	{
		main_game_globals->unknown11FA= 0;
		return;
	}
	main_game_globals->unknown11FA= 2;
	*(short *)main_game_globals->unknown11FC= location.cluster_index;
}

/* 0x0013da30: the long of the element (0x0C bytes, of the block at 0x08)
that the first short of what the block at 0x60 points to names, of the tag
that the tag named at 0x0C of something names at 0x24; NONE without it */
long tag_unknown_0013da30(
	const byte *data)
{
	const byte *tag;
	short index;

	if (!data || *(const long *)(data+0x0C)==NONE)
	{
		return NONE;
	}
	tag= *(const byte *const *)((const byte *)(uintptr_t)global_tag_instances[*(const long *)(data+0x0C)&0xFFFF].address+0x24);
	index= **(const short *const *)(tag+0x60);
	if (index==NONE)
	{
		return NONE;
	}

	return *(const long *)(*(const byte *const *)(tag+0x08)+index*0x0C);
}

/* 0x0047989c and 0x004798a0 (havok.c): two things of Havok */
extern void *unknown_0047989c;
extern void *unknown_004798a0;

/* the sum of the second longs of the pairs that a thing of Havok has (as
many as at 0x20, at 0x18 plus 0x1C, from the last) */
static inline __attribute__((always_inline)) long havok_unknown_sum(
	const byte *thing)
{
	long sum= 0;

	if (thing)
	{
		long count= *(const long *)(thing+0x20);
		const long *pair= (const long *)(*(const byte *const *)(thing+0x1C)+*(const long *)(thing+0x18)-4);

		for (; count>0; count--)
		{
			sum+= *pair;
			pair-= 2;
		}
	}

	return sum;
}

/* 0x00147090: those sums of the two */
long havok_unknown_00147090(
	void)
{
	long first= havok_unknown_sum((const byte *)unknown_0047989c);

	return havok_unknown_sum((const byte *)unknown_004798a0)+first;
}

/* 0x004ed284 and 0x00502120 (sound_manager.c): two blocks of the state of
the game */
extern byte *unknown_004ed284;
extern byte *unknown_00502120;

/* the short at 0x28 of a player (0x21C bytes) */
#define PLAYER_UNKNOWN28(player_index) \
	(*(const short *)((const byte *)player_data->data+(player_index&0xFFFF)*0x21C+0x28))

/* 0x001520f0: the entry (0x94 bytes) of unknown_004ed284 that a player names
at 0x28 gets NONE at 0x3C, and 1 at 0xA3 if its byte at 0xA2 is set */
void player_unknown_001520f0(
	long player_index)
{
	short index= PLAYER_UNKNOWN28(player_index);

	if (index!=NONE)
	{
		byte *entry= unknown_004ed284+index*0x94+0x14;

		if (entry[0x8E])
		{
			entry[0x8F]= 1;
		}
		*(short *)(entry+0x28)= NONE;
	}
}

/* 0x000b58c0: something of a simulation entity */
void __attribute__((stdcall)) unknown_000b58c0(long entity_index, unsigned long flags);

/* 0x00152240: with 0, the unit of a player (0x2C) gets the bit 0x08 at 0x134
and 0.25 at 0x2B8, and its simulation entity (0xD4) is told of 0x800000 */
void __attribute__((stdcall)) player_unknown_00152240(
	long player_index,
	short value)
{
	byte *unit;
	long unit_index;

	if (value)
	{
		return;
	}
	unit_index= *(const long *)((const byte *)player_data->data+(player_index&0xFFFF)*0x21C+0x2C);
	unit= (byte *)((const struct object_header_datum *)object_header_data->data)[unit_index&0xFFFF].datum;
	*(unsigned long *)(unit+0x134)|= 8;
	*(real *)(unit+0x2B8)= 0.25f;
	if (*(const long *)(unit+0xD4)!=NONE)
	{
		unknown_000b58c0(*(const long *)(unit+0xD4), 0x800000);
	}
}

/* 0x00510c5c (game_unknown_00153870.c) */
extern byte *unknown_00510c5c;

/* 0x00153b80: eight longs from 0x48 of an entry (0xA0 bytes, after 0x30)
of unknown_00510c5c, its short at 0xAE and the bit 0x01 of its byte at 0xB8
are cleared; nothing for NONE */
void unknown_00510c5c_clear_entry(
	long index)
{
	byte *entry;

	if (index==NONE)
	{
		return;
	}
	entry= unknown_00510c5c+index*0xA0+0x30;
	{
		long index_;

		for (index_= 0; index_<8; index_++)
		{
			((long *)(entry+0x18))[index_]= 0;
		}
	}
	*(short *)(entry+0x7E)= 0;
	entry[0x88]&= ~1;
}

/* 0x00153cd0: the entry (0x88 bytes) of unknown_00502120 that a player names
at 0x28 gets 0.0 at 0x80 and 0x84 */
void player_unknown_00153cd0(
	long player_index)
{
	short index= PLAYER_UNKNOWN28(player_index);

	if (index!=NONE)
	{
		byte *entry= unknown_00502120+index*0x88;

		*(real *)(entry+0x80)= 0.0f;
		*(real *)(entry+0x84)= 0.0f;
	}
}

/* 0x00510c64: how many of unknown_004e9148 there are */
long unknown_00510c64;
unsigned long unknown_004e9148[16];

/* 0x00156690: whether none of them is set */
boolean unknown_004e9148_empty(
	void)
{
	boolean empty= TRUE;
	short index;

	for (index= 0; index<unknown_00510c64; index++)
	{
		if (unknown_004e9148[index])
		{
			empty= FALSE;
		}
	}

	return empty;
}

/* 0x00152340: the players' globals get at 0x04 whether no player in use has
a unit (0x2C) or the bit 0x08 of its byte at 0x02, and at 0x05 whether any
player has neither */
void players_unknown_00152340(
	void)
{
	byte *globals= (byte *)players_globals;
	const struct data_array *players= player_data;
	long index;

	globals[4]= TRUE;
	globals[5]= FALSE;
	for (index= 0; index>=0 && index<players->first_unallocated; index++)
	{
		const byte *player;

		if (!(players->in_use_bit_vector[index>>5]&(1UL<<(index&0x1F))))
		{
			continue;
		}
		player= (const byte *)players->data+index*players->size;
		if (*(const long *)(player+0x2C)!=NONE || (player[2]&8))
		{
			globals[4]= FALSE;
		}
		else
		{
			globals[5]= TRUE;
		}
	}
}

/* 0x00138ab0: the cluster the game globals keep (0x11FA: 2 and the cluster
at 0x11FC), or, with 1 and an object at 0x11FC, the short at 0x2C of its
last parent when that has the bit 8 of its flags; the object is forgotten
(0) when it is not there. NONE otherwise */
short main_game_unknown_00138ab0(
	void)
{
	byte *globals= (byte *)main_game_globals;
	short state= *(const short *)(globals+0x11FA);
	long object_index;
	const byte *object;

	if (state==2)
	{
		return *(const short *)(globals+0x11FC);
	}
	if (state!=1)
	{
		return NONE;
	}
	object_index= *(const long *)(globals+0x11FC);
	if (!object_try_and_get_and_verify_type(object_index, 0xFFFFFFFF))
	{
		*(short *)(globals+0x11FA)= 0;
		return NONE;
	}
	object= (const byte *)((const struct object_header_datum *)object_header_data->data)
		[object_get_ultimate_parent(object_index)&0xFFFF].datum;
	if (((*(const unsigned long *)(object+4)>>8)&1) && *(const short *)(object+0x2C)!=NONE)
	{
		return *(const short *)(object+0x2C);
	}

	return NONE;
}

/* 0x00162c50: for a player without a unit, the player it follows (0x1B0)
when that one is valid, has something at 0xC4, nothing at its short at 0x28
and not the bit 0x02 at 0x02, for ten seconds from when it started (the bit
12 of the player's flags and the time at 0x190); FALSE, and the bit and the
time cleared, when not */
boolean __attribute__((stdcall)) player_unknown_00162c50(
	long player_index,
	long *result)
{
	byte *player= (byte *)player_data->data+(player_index&0xFFFF)*0x21C;
	long other_index= *(const long *)(player+0x1B0);
	const byte *other;

	if (*(const long *)(player+0x2C)!=NONE || other_index==NONE
		|| !(other= (const byte *)datum_try_and_get(player_data, other_index))
		|| !other[0xC4] || *(const short *)(other+0x28)!=NONE || (other[2]&2))
	{
		player[3]&= ~0x10;
		*(long *)(player+0x190)= 0;
		return FALSE;
	}
	if (!(*(const unsigned short *)(player+2)&0x1000))
	{
		*(unsigned short *)(player+2)|= 0x1000;
		*(long *)(player+0x190)= game_time_globals->time;
	}
	if ((long)(game_time_globals->time-*(const long *)(player+0x190))>=game_time_globals->ticks_per_second*10)
	{
		return FALSE;
	}
	*result= other_index;
	return TRUE;
}

/* 0x0055e504 and 0x0055e508: what havok_unknown_00147090 gave and a number
it was asked with */
long unknown_0055e504;
long unknown_0055e508;

/* 0x00278e60: the number is kept and what havok_unknown_00147090 gives */
void unknown_00278e60(
	long value)
{
	unknown_0055e508= value;
	unknown_0055e504= havok_unknown_00147090();
}

/* 0x0055e4fc: set when havok_unknown_00147090 has changed */
boolean unknown_0055e4fc;

/* 0x00278e80: the number is cleared; when havok_unknown_00147090 gives
another than it gave, the byte at 0xAE of something and unknown_0055e4fc are
set */
void unknown_00278e80(
	byte *data)
{
	unknown_0055e508= 0;
	if (havok_unknown_00147090()!=unknown_0055e504)
	{
		data[0xAE]= TRUE;
		unknown_0055e4fc= TRUE;
	}
}

/* 0x00052020: 0x50 bytes are copied (the second argument is not used);
TRUE */
boolean __attribute__((stdcall)) unknown_00052020(
	void *destination,
	long unused,
	const void *source)
{
	(void)unused;
	memcpy(destination, source, 0x50);
	return TRUE;
}

/* 0x0024c5d0: in the list of something (at 0x14, each to the next at
0x18), the first whose long at 0x70 is a number; NULL if none */
byte *__attribute__((stdcall)) unknown_0024c5d0(
	const byte *data,
	long value)
{
	byte *node= *(byte *const *)(data+0x14);

	while (node && *(const long *)(node+0x70)!=value)
	{
		node= *(byte *const *)(node+0x18);
	}

	return node;
}

/* 0x00470a3c: eight numbers, NONE until they are set */
long unknown_00470a3c[8]= {NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE};

/* 0x0023ecd0: they are 1 to 8 */
void unknown_0023ecd0(
	void)
{
	long index;

	for (index= 0; index<8; index++)
	{
		unknown_00470a3c[index]= index+1;
	}
}

/* 0x0017b270: whether a player (its flags at 0x02, two shorts at 0x10 and
0x12 of something) may with a number (0 or 1), when the players globals
have 1 at 0x08; otherwise only whether the short at 0x10 is not 1 (with the
bit 5 of the flags) or not 2 */
boolean unknown_0017b270(
	const byte *data,
	const byte *player,
	long number)
{
	boolean result= (player[2]>>5)&1 ? *(const short *)(data+0x10)!=1 : *(const short *)(data+0x10)!=2;
	short kind;

	if (*(const short *)((const byte *)players_globals+0x08)!=1)
	{
		return result;
	}
	if (!result)
	{
		return FALSE;
	}
	kind= *(const short *)(data+0x12);
	if (kind==2 && number)
	{
		return FALSE;
	}
	if (kind==1 && number!=1)
	{
		return FALSE;
	}

	return TRUE;
}

/* 0x0014f270: a player gets a number (0x24, NONE for none) and the players
globals the player for it (at 0x1C) and how many have one (0x0A); one that
had another first leaves it */
void __attribute__((stdcall)) player_unknown_0014f270(
	long player_index,
	long number)
{
	byte *player= (byte *)player_data->data+(player_index&0xFFFF)*0x21C;
	byte *globals= (byte *)players_globals;
	long old= *(const long *)(player+0x24);

	if (old==number)
	{
		return;
	}
	if (number==NONE)
	{
		*(long *)(globals+0x1C+old*4)= NONE;
		(*(short *)(globals+0x0A))--;
		*(long *)(player+0x24)= NONE;
		return;
	}
	player_unknown_0014f270(player_index, NONE);
	*(long *)(player+0x24)= number;
	*(long *)(globals+0x1C+number*4)= player_index;
	(*(short *)(globals+0x0A))++;
}

/* 0x0016b500: some bits of flags (19, 18, 30, 20, 22 and 21) as the bits 1
to 6 of a number whose bit 0 is set */
unsigned long unknown_0016b500(
	unsigned long flags)
{
	unsigned long result= ((flags&0x80000)|0x40000)>>18;

	result= flags&0x40000 ? result|4 : result&0xFFFB;
	result= flags&0x40000000 ? result|8 : result&0xFFF7;
	result= flags&0x100000 ? result|0x10 : result&0xFFEF;
	result= flags&0x400000 ? result|0x20 : result&0xFFDF;

	return flags&0x200000 ? result|0x40 : result&0xFFBF;
}
