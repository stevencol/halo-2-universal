/*
MAIN.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The game's main loop. Not here yet: most of what the game can ask it for,
and the parts of the game that a frame calls. The names are guesses, after those of
Halo 1's main.c.
*/

#include "main.h"

#include "cache/cache_files.h"
#include "cseries/async.h"
#include "game/game.h"
#include "saved games/game_state.h"
#include "scenario/scenario.h"
#include "shell/shell.h"
#include "text/unicode.h"

/* ---------- prototypes of functions of the console's libraries */

/* 0x003314b0 */
long GetTickCount(void);

/* ---------- prototypes of functions not decompiled yet */

/* 0x0018eb80: ends the game that is being played, if there is one */
void unknown_0018eb80(void);

/* 0x0018e700: starts the map again */
void unknown_0018e700(void);

/* 0x00163b60: whether something stops the game from going back to a
checkpoint now */
boolean unknown_00163b60(void);

/* 0x0012b2a0: what Direct3D is given to call; it keeps how long a frame
took */
void unknown_0012b2a0(void);

/* 0x0014a224: something of a game whose mode is 3 */
boolean unknown_0014a224(void);

/* what a frame calls, in the order that it calls them. Their names are not
known; the three with a name are what the game asked the loop for */
void unknown_002237d4(void);
void unknown_0018f1c0(long unknown);
void unknown_0018e810(void);
void unknown_0012bbd0(void);
void game_state_load_core(const char *name);
void unknown_00124950(void);
void unknown_00217c40(void);
void unknown_002164c0(long unknown, void *unknown_pointer);
void unknown_00121120(void);
void unknown_001406b0(void);
void unknown_00163820(void);
void unknown_001479c3(float seconds);
void unknown_00155f80(void);
void unknown_00092f80(long unknown);
void unknown_0008df50(void);
void unknown_000680f0(void);
void unknown_00059940(void);
boolean unknown_00068250(void);
void unknown_00146660(float seconds, float *unknown_real, long *unknown_long);
void unknown_001870a0(float seconds, float unknown_real);
void unknown_0008dfc0(void);
void unknown_00137ed0(float unknown_real);
void unknown_001554b0(float seconds);
void unknown_0016f280(float seconds);
void unknown_00222a70(float seconds);
void unknown_00223240(void);
void unknown_00125b40(void);
void unknown_000681e0(void);
void unknown_00222d80(void);

/* 0x0024cdaf, 0x001a0180 and 0x0024cbee: the messages that the game shows
the players. The names are guesses */
void hud_messages_unknown_0024cdaf(void);
void hud_get_text(unsigned long string_id, wchar *text);
void hud_print_message(long local_player_index, const wchar *text);

/* 0x0018efa0: what the loop does when a structure bsp cannot be loaded */
void unknown_0018efa0(void);

/* 0x00190e37: what main_unknown4E_private does before its message */
void unknown_00190e37(void);

/* ---------- constants */

enum
{
	_game_mode_3= 3,
	MAXIMUM_LOCAL_PLAYERS= 4,
	HUD_MESSAGE_LENGTH= 256,

	/* the string ids of the messages: hud_autosave_end, hud_loading_end and
	hud_saving_end */
	_string_id_hud_autosave_end= 0x100006A1,
	_string_id_hud_loading_end= 0x0F0006A3,
	_string_id_hud_saving_end= 0x0E0006A5,

	MINIMUM_FRAME_VBLANKS= 2,
	DEFAULT_REFRESH_RATE= 60,
	UNKNOWN_0051EA14_OFFSET= 0xBEF8
};

#define MAXIMUM_FRAME_SECONDS 10.0f

/* ---------- structures */

/* 0x004e8c20 points to it: the players. Only what the code here uses */
struct players_globals
{
	byte unknown0[0xC];
	/* the players of this console; NONE where there is none */
	long local_players[MAXIMUM_LOCAL_PLAYERS];
};

/* 0x00510c94 points to it: where the text of the messages comes from. What
it is is not known */
struct unknown_00510c94
{
	byte unknown0[0x3FC];
	long unknown3FC;
};

/* 0x00510c50 points to it. What it is is not known */
struct unknown_00510c50
{
	byte unknown0[6];
	boolean unknown6;
	byte unknown7[0x1B];
	boolean unknown22;
};

/* ---------- globals */

/* 0x00547f28 */
struct main_globals main_globals;

/* 0x004e6388 */
struct main_time_globals main_time_globals;

/* 0x00485ab0: goes up with each vertical blank of the picture. What makes
it go up is not decompiled, and the name is a guess: a frame's seconds are
what it went up, divided by the refresh rate */
long long main_vblank_count;

/* 0x00485aca: how many vertical blanks to give a frame; 0 for no limit.
The name is a guess */
short main_frame_vblanks;

/* what a frame looks at, of other parts of the game. What they are is not
known: 0x004ed294, 0x0055c14d, 0x0051ea14, 0x0055c150, 0x004d8ba0,
0x00510548, 0x0051054c and 0x00510560 */
long unknown_004ed294;
boolean unknown_0055c14d;
void *unknown_0051ea14;
unsigned long unknown_0055c150;
boolean unknown_004d8ba0;
boolean unknown_00510548;
long unknown_0051054c;
long unknown_00510560;

/* 0x00510c50 */
struct unknown_00510c50 *unknown_00510c50;

/* 0x004e8c20 */
struct players_globals *players_globals;

/* 0x00510c94 */
struct unknown_00510c94 *unknown_00510c94;

/* 0x004ed39d, 0x004e6470 and 0x004e64a0: what else main_action_pending
looks at; and 0x0055c14c, what main_cancel_map_copy_and_wait waits for. What
they are is not known */
boolean unknown_004ed39d;
long unknown_004e6470;
long unknown_004e64a0;
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

	unknown_0018eb80();
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

	if (main_frame_vblanks &&
		main_game_globals &&
		main_game_globals->active &&
		(!time->unknown0 || time->unknown1))
	{
		short frame_vblanks= main_frame_vblanks;

		time->unknown32= frame_vblanks;
		if (main_game_globals->active)
		{
			if (main_game_globals->options.game_mode!=_game_mode_3 || unknown_0014a224())
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
	if (unknown_004ed294!=2 && unknown_004ed294!=5 && !unknown_004ed39d)
	{
		unknown_0018f1c0(0);
	}
	unknown_0018e810();

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

	unknown_00124950();
	unknown_00217c40();
	if (unknown_0055c14d && unknown_0051ea14 && (unknown_0055c150&1))
	{
		unknown_002164c0(0, (byte *)unknown_0051ea14+UNKNOWN_0051EA14_OFFSET);
		unknown_0055c150&= ~1UL;
	}
	/* the preferences changed: they are written */
	if (cached_map_files_state_changed)
	{
		unknown_00121120();
	}
	unknown_001406b0();
	async_globals.tasks_added= async_task_count();
	/* a step of the copy of a map to the hard disk, if the last one is done */
	if (cached_map_file_copy_globals.done)
	{
		cached_map_file_copy_update();
	}
	unknown_00163820();

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
		unknown_00092f80(unknown_00510560);
		unknown_00510548= FALSE;
		unknown_0051054c= GetTickCount();
	}
	unknown_0008df50();
	unknown_000680f0();
	unknown_00059940();

	if (unknown_00068250())
	{
		unknown_00146660(seconds, &unknown_real, &unknown_long);
		unknown_001870a0(seconds, unknown_real);
		game_run_ticks(unknown_long, &unknown_real);
		unknown_0008dfc0();
		unknown_00137ed0(unknown_real);
		unknown_001554b0(seconds);
		unknown_0016f280(seconds);
		unknown_00222a70(seconds);
		unknown_00223240();
		unknown_00125b40();
	}
	else
	{
		unknown_000681e0();
		unknown_0008dfc0();
		unknown_00222d80();
		unknown_00223240();
		unknown_00125b40();
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
		hud_get_text(string_id, text);
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
		unknown_0018efa0();
	}
	main_globals.switch_structure_bsp= FALSE;
	main_globals.switch_structure_bsp_index= NONE;
}

/* 0x0012bdf0: does something that is not known and tells the first player of
this console that the game was saved, in the campaign */
void main_unknown4E_private(
	void)
{
	unknown_00190e37();
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
			unknown_0018e700();
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
			possible= !unknown_00163b60();
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
	boolean pending= unknown_004ed39d!=0;

	if (main_globals.switch_structure_bsp ||
		main_globals.reset_map ||
		main_globals.revert_map ||
		main_globals.revert_map_if_possible ||
		main_globals.save_map ||
		main_globals.unknown49 ||
		main_globals.unknown4E ||
		unknown_004e6470>0 ||
		unknown_004e64a0>0)
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
		main_loading_idle();
	}
}
