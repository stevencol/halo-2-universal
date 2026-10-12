/*
GAME_TIME.C

the time of the game: how it starts and stops. Worked out from
halo2ship.exe; the name of the file is a guess, by the one that the first
game has. What it keeps is declared in game.h, and its global is of game.c.
*/

#include "game/game.h"
#include "saved games/game_state.h"

#include <math.h>
#include <string.h>

/* ---------- code */

/* 0x00146550: the time of the game takes its memory of the state of the
game, and is nothing. The game has game_state_malloc written here */
void game_time_initialize(
	void)
{
	struct game_time_globals *globals= game_state_malloc(sizeof(struct game_time_globals));

	memset(globals, 0, sizeof(struct game_time_globals));
	game_time_globals= globals;
}

/* 0x001465b0: for a new map, the time is nothing: it has the ticks in a
second that the options of the game say, how long a tick is by them, and
goes at its own speed; and it is set up */
void game_time_initialize_for_new_map(
	void)
{
	struct game_time_globals *globals= game_time_globals;
	short tick_rate;

	memset(globals, 0, sizeof(struct game_time_globals));
	tick_rate= main_game_globals->options.tick_rate;
	globals->ticks_per_second= tick_rate;
	globals->seconds_per_tick= 1.0f/(float)tick_rate;
	globals->speed= 1.0f;
	globals->initialized= TRUE;
}

/* 0x00146610: when a map goes, the time is not set up, if it has memory and
was */
void game_time_dispose_from_old_map(
	void)
{
	struct game_time_globals *globals= game_time_globals;

	if (globals && globals->initialized)
	{
		globals->initialized= FALSE;
	}
}

/* 0x00146630: the time of the game in seconds; zero when there is no time
of the game yet */
float game_time_get_seconds(
	void)
{
	const struct game_time_globals *globals= game_time_globals;

	if (globals && globals->initialized)
	{
		return (float)(long)globals->time*globals->seconds_per_tick;
	}

	return 0.0f;
}

/* 0x00146650: the time of the game, in ticks */
long game_time_get(
	void)
{
	return (long)game_time_globals->time;
}

/* 0x00146840: whether the time of the game is there and stopped */
boolean game_time_get_paused(
	void)
{
	return game_time_globals->initialized && game_time_globals->paused;
}

/* 0x001469f0: how many ticks some seconds are, to the nearest */
long __attribute__((stdcall)) game_seconds_to_ticks(
	float seconds)
{
	return lrintf((float)game_time_globals->ticks_per_second*seconds);
}
