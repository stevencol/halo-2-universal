/*
MAIN.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The game's main loop. Not here yet: what it does each frame (0x0012b450) and
most of what the game can ask it for. The names are guesses, after those of
Halo 1's main.c.
*/

#include "main.h"

#include "cache/cache_files.h"
#include "cseries/async.h"
#include "game/game.h"
#include "saved games/game_state.h"
#include "shell/shell.h"

/* ---------- prototypes of functions of the console's libraries */

/* 0x003314b0 */
long GetTickCount(void);

/* ---------- prototypes of functions not decompiled yet */

/* 0x0012b450: a frame of the game */
void main_loop_body(void);

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

/* ---------- structures */

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

/* 0x004e6390: the tick count of the console when the loop started, in 64
bits */
unsigned long main_time_start[2];

/* 0x004e6400 */
struct main_frame_time_samples main_frame_time_samples;

/* 0x00510c50 */
struct unknown_00510c50 *unknown_00510c50;

/* 0x004ed39d, 0x004e6470 and 0x004e64a0: what else main_action_pending
looks at; and 0x0055c14c, what main_cancel_map_copy_and_wait waits for. What
they are is not known */
boolean unknown_004ed39d;
long unknown_004e6470;
long unknown_004e64a0;
boolean unknown_0055c14c;

/* 0x004e6398: defined with the state of the game, which also sets it */
extern unsigned long unknown_004e6398[2];

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

	main_time_start[0]= (unsigned long)GetTickCount();
	main_time_start[1]= 0;
	unknown_004e6398[0]= 0;
	unknown_004e6398[1]= 0;
	d3d_device->frame_callback= unknown_0012b2a0;

	for (index= 0; index<NUMBER_OF_MAIN_FRAME_TIME_SAMPLES; index++)
	{
		main_frame_time_samples.samples[index]= 0;
	}
	main_frame_time_samples.next_index= 0;
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
