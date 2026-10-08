/*
VOICE_MUTE_LIST.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The players that the player of each gamepad wants not to hear. The name of
the file is a guess, and so are the names of its functions.
*/

#include "voice_mute_list.h"

#include "input/input.h"
#include "main/main_game.h"

#include <string.h>

/* ---------- prototypes of functions of the console's libraries */

/* 0x003a084c, 0x003a086e: the player of a gamepad wants not to hear a
player, and wants to hear it again; less than 0 if it could not be done */
long unknown_003a084c(long controller_index, struct voice_mute_list_player player);
long unknown_003a086e(long controller_index, struct voice_mute_list_player player);

/* ---------- prototypes of functions not decompiled yet */

/* 0x0006cd50: what a session of the network game is doing */
long unknown_0006cd50(long session_index);

/* 0x00058b70: of this file: asks the library for the list of a gamepad */
void unknown_00058b70(long controller_index);

/* ---------- constants */

enum
{
	_unknown_0006cd50_state1= 1
};

/* ---------- globals */

/* 0x004c99c0: something that the library is asked, for all the gamepads;
NONE if nothing is */
long unknown_004c99c0= NONE;

/* 0x004c99c4: what the library is asked for the list of each gamepad; NONE
if nothing is */
long voice_mute_list_tasks[MAXIMUM_VOICE_CONTROLLERS];

/* ---------- code */

/* 0x00058a50: the list of a gamepad is not known, and nothing is asked of
the library */
void voice_mute_list_reset(
	long controller_index)
{
	unknown_004c99c0= NONE;
	voice_mute_list_tasks[controller_index]= NONE;
	memset(game_voice_muted_players[controller_index], 0, sizeof(game_voice_muted_players[controller_index]));
	game_voice_muted_player_counts[controller_index]= NONE;
}

/* the library is told of a player, one way or the other, in a session of
the network game in its first state and with the list of the gamepad known;
and then the list is asked for again. The game has this in the two
functions that follow */
static inline __attribute__((always_inline)) void voice_mute_list_change(
	long controller_index,
	const struct voice_mute_list_player *player,
	long (*change)(long controller_index, struct voice_mute_list_player player))
{
	long count= game_voice_muted_player_counts[controller_index];

	if (unknown_00467214!=NONE && unknown_0006cd50(unknown_00467214)==_unknown_0006cd50_state1 && count!=NONE)
	{
		if (change(controller_index, *player)>=0)
		{
			unknown_00058b70(controller_index);
		}
	}
}

/* 0x00058c10: the player of a gamepad wants not to hear a player */
void voice_mute_list_add(
	long controller_index,
	const struct voice_mute_list_player *player)
{
	voice_mute_list_change(controller_index, player, unknown_003a084c);
}

/* 0x00058c60: the player of a gamepad wants to hear a player again */
void voice_mute_list_remove(
	long controller_index,
	const struct voice_mute_list_player *player)
{
	voice_mute_list_change(controller_index, player, unknown_003a086e);
}

/* 0x00058cb0: whether a player is in the list of a gamepad; FALSE if the
list is not known, or without a flag of what the game knows of the player of
the gamepad */
boolean VOICE_MUTE_LIST_CALLBACK voice_mute_list_has_player(
	long controller_index,
	const struct voice_mute_list_player *player)
{
	long count= game_voice_muted_player_counts[controller_index];
	unsigned long flags;

	if (count==NONE)
	{
		return FALSE;
	}
	memcpy(&flags, unknown_0054e8e0[controller_index]+UNKNOWN_0054E8E0_FLAGS_OFFSET, sizeof(flags));
	if ((flags>>_unknown_0054e8e0_unknown5_bit)&1)
	{
		long index;

		for (index= 0; index<count; index++)
		{
			if (!memcmp(player, game_voice_muted_players[controller_index][index], GAME_VOICE_PLAYER_IDENTIFIER_SIZE))
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}
