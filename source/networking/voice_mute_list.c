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
#include "networking/online_tasks.h"

#include <string.h>

/* ---------- prototypes of functions of the console's libraries */

/* 0x003a084c, 0x003a086e: the player of a gamepad wants not to hear a
player, and wants to hear it again; less than 0 if it could not be done */
long unknown_003a084c(long controller_index, struct voice_mute_list_player player);
long unknown_003a086e(long controller_index, struct voice_mute_list_player player);

/* 0x003a0841: the library gets ready to keep the lists. 0x003a0832: asks for
the list of a gamepad: up to so many players, in room that is given, with a
place for how many there are. Both give something to ask how it goes with,
and less than 0 if they could not start */
long unknown_003a0841(long unknown, long *handle);
long unknown_003a0832(long controller_index, long maximum_count, long unknown, long *handle, void *players,
	long *count);

/* ---------- prototypes of functions not decompiled yet */

/* 0x0006cd50: what a session of the network game is doing */
long online_tasks_get_logon_state(long session_index);

/* 0x0006b730: something new that the library of the network is asked;
NONE if there is no room, or with no session in its first state.
0x0006b910: where it is; NULL if it is no more. 0x0006b640: it is asked no
more. 0x0006b890: something of a kind that is asked; 0 if nothing is */

/* ---------- constants */

enum
{
	_online_tasks_get_logon_state_state1= 1,

	/* something that the library is asked: its state, its kind, who it is
	for and what the library gave to ask how it goes with */
	ONLINE_TASK_STATE_OFFSET= 0x2,
	ONLINE_TASK_KIND_OFFSET= 0x4,
	ONLINE_TASK_OWNER_OFFSET= 0x8,
	ONLINE_TASK_HANDLE_OFFSET= 0xC,
	/* the two kinds of this file, and the state that each starts in */
	_online_task_kind_mute_list_startup= 0xB,
	_online_task_kind_mute_list_get= 0xC,
	_online_task_mute_list_startup_state= 1,
	_online_task_mute_list_get_state= 9,
	/* what online_task_exists is asked with, when the gamepad does not matter */
	ONLINE_TASK_ANY_OWNER= 0xFF
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

	if (unknown_00467214!=NONE && online_tasks_get_logon_state(unknown_00467214)==_online_tasks_get_logon_state_state1 && count!=NONE)
	{
		if (change(controller_index, *player)>=0)
		{
			voice_mute_list_request(controller_index);
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

/* 0x00058a90: the library gets ready to keep the lists, if it is not asked
to yet. If the library cannot start, the game lets go of what unknown_004c99c0
has, which is nothing, and not of what it has just taken */
void VOICE_MUTE_LIST_CALLBACK voice_mute_list_startup(
	long unknown)
{
	if (unknown_004c99c0==NONE)
	{
		long task= online_task_new_signed_in();

		if (task!=NONE)
		{
			byte *datum= (byte *)online_task_get(task);

			if (datum)
			{
				if (unknown_003a0841(0, (long *)(datum+ONLINE_TASK_HANDLE_OFFSET))>=0)
				{
					*(word *)(datum+ONLINE_TASK_STATE_OFFSET)= _online_task_mute_list_startup_state;
					*(long *)(datum+ONLINE_TASK_KIND_OFFSET)= _online_task_kind_mute_list_startup;
					*(long *)(datum+ONLINE_TASK_OWNER_OFFSET)= unknown;
					unknown_004c99c0= task;
				}
				else
				{
					online_task_close(unknown_004c99c0);
					unknown_004c99c0= NONE;
				}
			}
		}
	}
}

/* 0x00058b00: the list of a gamepad is asked for no more, and is not known;
and if no list is asked for, the library keeps the lists no more */
void voice_mute_list_stop(
	long controller_index)
{
	long startup_task= unknown_004c99c0;
	long task= voice_mute_list_tasks[controller_index];

	if (task!=NONE)
	{
		online_task_close(task);
		voice_mute_list_tasks[controller_index]= NONE;
		game_voice_muted_player_counts[controller_index]= NONE;
		memset(game_voice_muted_players[controller_index], 0, sizeof(game_voice_muted_players[controller_index]));
	}
	if (startup_task!=NONE)
	{
		if (!online_task_exists(ONLINE_TASK_ANY_OWNER, _online_task_kind_mute_list_get))
		{
			online_task_close(startup_task);
			unknown_004c99c0= NONE;
		}
	}
}

/* 0x00058b70: asks the library for the list of a gamepad, if it is not
asked for already. The list is not known until the library gives it */
void voice_mute_list_request(
	long controller_index)
{
	long *task= &voice_mute_list_tasks[controller_index];
	long new_task;
	byte *datum;

	if (*task!=NONE && online_task_get(*task))
	{
		return;
	}
	new_task= online_task_new_signed_in();
	if (new_task==NONE)
	{
		return;
	}
	datum= (byte *)online_task_get(new_task);
	if (!datum)
	{
		return;
	}
	game_voice_muted_player_counts[controller_index]= NONE;
	if (unknown_003a0832(controller_index, MAXIMUM_GAME_VOICE_MUTED_PLAYERS, 0,
		(long *)(datum+ONLINE_TASK_HANDLE_OFFSET), game_voice_muted_players[controller_index],
		&game_voice_muted_player_counts[controller_index])>=0)
	{
		*(long *)(datum+ONLINE_TASK_OWNER_OFFSET)= controller_index;
		*(word *)(datum+ONLINE_TASK_STATE_OFFSET)= _online_task_mute_list_get_state;
		*(long *)(datum+ONLINE_TASK_KIND_OFFSET)= _online_task_kind_mute_list_get;
		*task= new_task;
	}
	else
	{
		online_task_close(*task);
		*task= NONE;
	}
}
