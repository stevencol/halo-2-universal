/*
GAME_UNKNOWN_0016EFB0.C

a part of the game: it keeps five numbers in the state of the game.
Worked out from halo2ship.exe; what it is for is not known, and the name of
the file is only the address of its first function.
*/

#include "cseries/cseries.h"
#include "game/game.h"
#include "main/main.h"
#include "saved games/game_state.h"
#include "scenario/scenario.h"

/* ---------- globals */

/* 0x00510c70: what it keeps */
long *unknown_00510c70;

/* 0x004e9bd4: four things of 0x358 bytes, one for each player of this
console: at 0xB8 of each a point, and at 0xC4 where that is in the level.
0x004ea934: a map is loaded. 0x004ea935, 0x004ea936: two things that are
asked for, and done by game_unknown_0016efb0_unknown_0016f200.
0x004e6470, 0x004e6474, 0x004e64a0, 0x004e64a4, 0x004e64ac: five numbers
that those two set. 0x00510c50, of main.c */
byte unknown_004e9bd4[4][0x358];
boolean unknown_004ea934;
boolean unknown_004ea935;
boolean unknown_004ea936;
long unknown_004e6470;
long unknown_004e6474;
long unknown_004e64a0;
long unknown_004e64a4;
long unknown_004e64ac;
struct unknown_00510c50;
extern struct unknown_00510c50 *unknown_00510c50;

/* 0x0016f4b0: one of the four things is as it starts */
void unknown_0016f4b0(byte *unknown);

/* ---------- code */

/* 0x0016efb0: what it keeps takes its memory of the state of the game. The
game has game_state_malloc written here */
void game_unknown_0016efb0_initialize(
	void)
{
	unknown_00510c70= game_state_malloc(5*sizeof(long));
}

/* 0x0016f0e0: for a new map, a map is loaded, each of the four things is as
it starts (unknown_0016f4b0), and the five numbers are NONE */
void game_unknown_0016efb0_initialize_for_new_map(
	void)
{
	long index;

	unknown_004ea934= TRUE;
	for (index= 0; index<4; index++)
	{
		unknown_0016f4b0(unknown_004e9bd4[index]);
	}
	for (index= 0; index<5; index++)
	{
		unknown_00510c70[index]= NONE;
	}
}

/* 0x0016f120: the fifth of the functions of the part, for when the game
becomes active: where the point of each player of this console is in the
level is worked out (scenario_location_from_point); and the first of the two
things is asked for, if the time of the game is past its start, unless what
unknown_00510c50 points to has its byte at 0x5 set */
void game_unknown_0016efb0_unknown_0016f120(
	void)
{
	const byte *unknown;
	long index;

	for (index= 0; index<4; index++)
	{
		if (players_globals->local_players[index]!=NONE)
		{
			scenario_location_from_point((struct scenario_location *)(unknown_004e9bd4[index]+0xC4),
				(const real_point3d *)(unknown_004e9bd4[index]+0xB8));
		}
	}
	unknown= (const byte *)unknown_00510c50;
	if ((!unknown || !unknown[5]) && (long)game_time_globals->time>0)
	{
		unknown_004ea935= TRUE;
	}
}

/* 0x0016f200: the two things that were asked for are done, each once: but
in a game of the second mode, the five numbers are 3, 1, 3, 1 and nothing
for the first, and 3, nothing, 3, nothing and nothing for the second */
void game_unknown_0016efb0_unknown_0016f200(
	void)
{
	if (unknown_004ea935)
	{
		if (main_game_globals->options.game_mode!=2)
		{
			unknown_004e64a4= 1;
			unknown_004e64a0= 3;
			unknown_004e64ac= 0;
			unknown_004e6470= 3;
			unknown_004e6474= 1;
		}
		unknown_004ea935= FALSE;
	}
	if (unknown_004ea936)
	{
		if (main_game_globals->options.game_mode!=2)
		{
			unknown_004e64a4= 0;
			unknown_004e64a0= 3;
			unknown_004e64ac= 0;
			unknown_004e6470= 3;
			unknown_004e6474= 0;
		}
		unknown_004ea936= FALSE;
	}
}

/* 0x0016f090: a state of the game is about to be loaded: a map is loaded,
and each of the four things is as it starts (unknown_0016f4b0). It is one
of the things that are done before a state is loaded (game_state.c) */
void GAME_STATE_PROC game_unknown_0016efb0_before_load(
	unsigned long flags)
{
	long index;

	(void)flags;
	unknown_004ea934= TRUE;
	for (index= 0; index<4; index++)
	{
		unknown_0016f4b0(unknown_004e9bd4[index]);
	}
}

/* 0x0016f0c0: a state of the game was loaded: the second of the two things
is asked for, but not in a game in the network of kind 2 to 5. It is one of
the things that are done after a state is loaded */
void GAME_STATE_PROC game_unknown_0016efb0_after_load(
	unsigned long flags)
{
	long simulation= main_game_globals->options.simulation;

	(void)flags;
	if (simulation<2 || simulation>5)
	{
		unknown_004ea936= TRUE;
	}
}

/* 0x0054e858, 0x0054e85c and 0x0054e860: three reals that the game works
out when it starts: the least and the most of one range, and the most of
another */
float unknown_0054e858;
float unknown_0054e85c;
float unknown_0054e860;

/* whether a real is a number (no infinity and no NaN) from one to another */
static boolean real_is_in_range(
	float value,
	float minimum,
	float maximum)
{
	union
	{
		float real;
		unsigned long bits;
	} as= {value};

	return (as.bits&0x7F800000)!=0x7F800000 && value>=minimum && maximum>=value;
}

/* 0x001723e0: whether a real is from unknown_0054e858 to unknown_0054e85c.
The three are after this file in the game; whether they are of it is not
known */
boolean real_is_in_range_0054e858(
	float value)
{
	return real_is_in_range(value, unknown_0054e858, unknown_0054e85c);
}

/* 0x00172420: from zero to unknown_0054e860 */
boolean real_is_in_range_0054e860(
	float value)
{
	return real_is_in_range(value, 0.0f, unknown_0054e860);
}

/* 0x00172460: from zero to 3600 */
boolean real_is_in_range_3600(
	float value)
{
	return real_is_in_range(value, 0.0f, 3600.0f);
}

/* 0x0016f3a0: the point of a local player (at 0xB8 of its thing), if there
is a structure bsp; NULL if not, or for NONE */
real_point3d *game_unknown_0016efb0_get_point(
	long local_player_index)
{
	if (local_player_index==NONE || global_structure_bsp_index==NONE)
	{
		return NULL;
	}

	return (real_point3d *)(unknown_004e9bd4[local_player_index]+0xB8);
}

boolean valid_world_real_point3d(const real *point);
boolean valid_real_point3d(const real *point);
long real_math_unknown_000a74c0(const real *forward, const real *up);

/* 0x001724a0: whether a camera (with the bit 0 of its flags) is good: its
two vectors (0x2C and 0x38) one long and perpendicular, two points in the
world (0x04 and 0x10), a point (0x44) and three reals in their ranges (0x24,
0x28 and 0x88); TRUE without the bit, FALSE for none */
boolean game_unknown_001724a0(
	const byte *camera)
{
	if (!camera)
	{
		return FALSE;
	}
	if (!(camera[0]&1))
	{
		return TRUE;
	}

	return real_math_unknown_000a74c0((const real *)(camera+0x2C), (const real *)(camera+0x38))
		&& valid_world_real_point3d((const real *)(camera+0x04))
		&& valid_world_real_point3d((const real *)(camera+0x10))
		&& valid_real_point3d((const real *)(camera+0x44))
		&& real_is_in_range_0054e860(*(const float *)(camera+0x24))
		&& real_is_in_range_0054e858(*(const float *)(camera+0x28))
		&& real_is_in_range_3600(*(const float *)(camera+0x88));
}

/* 0x004e64c8: some flags */
unsigned long unknown_004e64c8;

/* 0x0016e210: whether a cluster of the structure bsp (0xB0 bytes, at 0xA0)
is of something: by its byte at 0x70 (with the high bit, through a short
of the block at 0x68, 0x18 bytes), the thing at 0x08 of an element of the
block at 0x34C of the scenario (0x10 bytes); TRUE with the bit 4 of the
flags, or for NONE */
boolean game_unknown_0016e210(
	long cluster_index,
	long thing)
{
	const byte *bsp= (const byte *)global_structure_bsp;
	signed char value;
	long index;

	if (!global_scenario || !bsp || cluster_index==NONE)
	{
		return FALSE;
	}
	if ((unknown_004e64c8>>4)&1)
	{
		return TRUE;
	}
	value= *(const signed char *)(*(const byte *const *)(bsp+0xA0)+cluster_index*0xB0+0x70);
	if (value==NONE)
	{
		return FALSE;
	}
	if (thing==NONE)
	{
		return TRUE;
	}
	if (value<0)
	{
		index= *(const short *)(*(const byte *const *)(bsp+0x68)+(value&0x7F)*0x18);
	}
	else
	{
		index= value&0x7F;
	}
	if (index==NONE)
	{
		return FALSE;
	}

	return *(const long *)(*(const byte *const *)((const byte *)global_scenario+0x34C)+index*0x10+0x08)==thing;
}
