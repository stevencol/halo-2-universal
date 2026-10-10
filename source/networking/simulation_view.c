/*
SIMULATION_VIEW.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

A view that the game in the network has of itself: what it talks with
another console by, one for each channel of the game. The name of the file
is a guess, and so are the names of its functions.
*/

#include "simulation_view.h"

#include "networking/network_observer.h"
#include "networking/simulation_world.h"

/* ---------- prototypes */

long GetTickCount(void);

/* 0x000861c0: what follows from the state of a view and the one of the
other console is seen to */
void unknown_000861c0(byte *view);

/* 0x0006a7f0: the thing takes what some players of the other console do, as
bits say which. 0x0006a990: it takes a piece of the state of the game, of a
size, that goes at a place; whether it could. 0x0006aa20: all of the state of
the game has come, so many bytes: it is the one of the game now; whether it
could */
void __attribute__((stdcall)) unknown_0006a7f0(byte *world, byte *unknown14, long players, const void *actions);
boolean __attribute__((stdcall)) unknown_0006aa20(byte *world, long size);

/* 0x0006a8a0: the thing makes ready for the game to go; whether it could.
0x0006ab10: the things that have come and wait are let go of, most likely */
boolean __attribute__((stdcall)) unknown_0006a8a0(byte *world);

/* ---------- globals */

/* 0x00510548, 0x0051054c, of main.c: whether the time is the one that was
kept, and that time */
extern boolean unknown_00510548;
extern long unknown_0051054c;

/* ---------- code */

/* 0x00086140: a view is told whether it keeps the game from going on. When
it starts to, the time is noted. And one that has done so for two seconds,
and is well, goes to its first state (simulation_view_set_state) and is then
as 4 says. The game has the time of the network, and network_time_since, written
here */
void simulation_view_set_blocked(
	byte *view,
	boolean blocked)
{
	if (blocked && !view[SIMULATION_VIEW_BLOCKED_OFFSET])
	{
		*(long *)(view+SIMULATION_VIEW_BLOCKED_TIME_OFFSET)= unknown_00510548 ? unknown_0051054c : GetTickCount();
	}
	view[SIMULATION_VIEW_BLOCKED_OFFSET]= blocked;
	if (blocked && network_time_since(*(const long *)(view+SIMULATION_VIEW_BLOCKED_TIME_OFFSET))>=SIMULATION_VIEW_BLOCKED_TIMEOUT &&
		*(const long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)==0)
	{
		simulation_view_set_state(view, _simulation_view_state0, NONE);
		*(long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)= _simulation_view_unknown28_blocked;
	}
}

/* 0x00085a70: a view goes to a state, with a number. It can if the state is
one of the first two and the number is NONE; if it is the third and the
number is not less than nothing; or if it is the one after the state that
the view is in, with the same number. If it cannot, a view that is well
goes to its first state, and is then as 7 says.

With the state and the number that it has already, nothing is done. If not,
they are the ones of the view; the other console is told, if the view talks
by a channel of the observer (a message of the kind 0x25, with the two); and
what follows from it is seen to (unknown_000861c0) */
void __attribute__((stdcall)) simulation_view_set_state(
	byte *view,
	long state,
	long number)
{
	struct simulation_view_state_message message= {0, 0};
	boolean good;

	if (state<_simulation_view_state2)
	{
		good= number==NONE;
	}
	else if (state==_simulation_view_state2)
	{
		good= number>=0;
	}
	else
	{
		good= number==*(const long *)(view+SIMULATION_VIEW_STATE_NUMBER_OFFSET) &&
			state==(long)(*(const unsigned long *)(view+SIMULATION_VIEW_STATE_OFFSET)+1);
	}
	if (!good)
	{
		if (*(const long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)==0)
		{
			simulation_view_set_state(view, _simulation_view_state0, NONE);
			*(long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)= _simulation_view_unknown28_bad_state;
		}

		return;
	}
	if (*(const long *)(view+SIMULATION_VIEW_STATE_OFFSET)==state &&
		*(const long *)(view+SIMULATION_VIEW_STATE_NUMBER_OFFSET)==number)
	{
		return;
	}
	*(long *)(view+SIMULATION_VIEW_STATE_NUMBER_OFFSET)= number;
	message.number= number;
	*(long *)(view+SIMULATION_VIEW_STATE_OFFSET)= state;
	message.state= state;
	if (*(const long *)(view+SIMULATION_VIEW_OBSERVER_CHANNEL_OFFSET)!=NONE)
	{
		network_observer_channel_send(FALSE, *(const long *)(view+SIMULATION_VIEW_OBSERVER_CHANNEL_OFFSET),
			*(byte *const *)(view+SIMULATION_VIEW_OBSERVER_OFFSET), SIMULATION_VIEW_SESSION_INDEX,
			_network_message_simulation_view_state, sizeof(message), &message);
	}
	unknown_000861c0(view);
}

/* 0x000862c0: a view that is well goes to its first state, and is then as a
number says. The game has this written in most of the places that do it */
void __attribute__((stdcall)) simulation_view_fail(
	byte *view,
	long reason)
{
	if (*(const long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)==0)
	{
		simulation_view_set_state(view, _simulation_view_state0, NONE);
		*(long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)= reason;
	}
}

/* 0x00085f50: a view of the second kind is told what the other console
says of its players: that it could not go on, or two numbers, of which the
second is the one of what it is up to, and what some of its players do;
whether that was heeded.

If it could not go on, the view goes to its first state and is as 2 says
(simulation_view_fail, written here). If not: numbers before the last that
came are not heeded, nor one that is not before the one of the thing. The
rest are; and if a number of the thing is 4, the two numbers are kept and
the thing takes what the players do (unknown_0006a7f0) */
boolean __attribute__((stdcall)) simulation_view_unknown_00085f50(
	byte *view,
	boolean failed,
	long update,
	long unknown0,
	long players,
	const void *actions)
{
	byte *world;

	if (failed)
	{
		if (*(const long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)==0)
		{
			simulation_view_set_state(view, _simulation_view_state0, NONE);
			*(long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)= _simulation_view_unknown28_remote_failed;
		}

		return TRUE;
	}
	if (unknown0<*(const long *)(view+SIMULATION_VIEW_UNKNOWN80_OFFSET) ||
		update<*(const long *)(view+SIMULATION_VIEW_UNKNOWN84_OFFSET))
	{
		return FALSE;
	}
	world= *(byte *const *)(view+SIMULATION_VIEW_WORLD_OFFSET);
	if (update>=*(const long *)(world+SIMULATION_WORLD_UNKNOWN28_OFFSET))
	{
		return FALSE;
	}
	if (*(const long *)(world+SIMULATION_WORLD_UNKNOWN18_OFFSET)==_simulation_world_unknown18_4)
	{
		*(long *)(view+SIMULATION_VIEW_UNKNOWN84_OFFSET)= update;
		*(long *)(view+SIMULATION_VIEW_UNKNOWN80_OFFSET)= unknown0;
		unknown_0006a7f0(world, view+SIMULATION_VIEW_UNKNOWN14_OFFSET, players, actions);
	}

	return TRUE;
}

/* 0x00085ed0: a view of the first kind is given a piece of the state of
the game that the other console sends, which goes at a place and is of a
size; whether it was taken. It is not if the thing is not being sent the
state of the game. A piece of some size is one more piece (simulation_world_receive_game_state);
one of none says that it has all come, so many bytes (unknown_0006aa20). If
the thing could not take it, the view goes to its first state and is as 5
says (simulation_view_fail, written here) */
boolean __attribute__((stdcall)) simulation_view_unknown_00085ed0(
	byte *view,
	long offset,
	long size,
	const void *data)
{
	byte *world= *(byte *const *)(view+SIMULATION_VIEW_WORLD_OFFSET);
	boolean good;

	if (!simulation_world_receiving_game_state(world))
	{
		return FALSE;
	}
	if (size>0)
	{
		good= simulation_world_receive_game_state(world, offset, size, data);
	}
	else
	{
		good= unknown_0006aa20(world, offset);
	}
	if (!good && *(const long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)==0)
	{
		simulation_view_set_state(view, _simulation_view_state0, NONE);
		*(long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)= _simulation_view_unknown28_game_state_failed;
	}

	return good;
}

/* 0x00085dc0: a view of the first kind is told the number that the game
goes on from; whether that was heeded. It is only if the number that the
thing has at 0x18 is 3 and the thing is not being sent the state of the
game. Then the thing makes ready (unknown_0006a8a0); if it could, the number
is the one of the thing, it starts to go as simulation_world_unknown_0006b2a0
has it (written here), and the game is brought up to what has come
(simulation_world_unknown_00069350). If it could not, the view goes to its
first state and is as 5 says (simulation_view_fail, written here) */
boolean __attribute__((stdcall)) simulation_view_unknown_00085dc0(
	byte *view,
	long number)
{
	byte *world= *(byte *const *)(view+SIMULATION_VIEW_WORLD_OFFSET);

	if (*(const long *)(world+SIMULATION_WORLD_UNKNOWN18_OFFSET)!=_simulation_world_unknown18_3 ||
		simulation_world_receiving_game_state(world))
	{
		return FALSE;
	}
	if (unknown_0006a8a0(world))
	{
		world= *(byte *const *)(view+SIMULATION_VIEW_WORLD_OFFSET);
		*(long *)(world+SIMULATION_WORLD_UNKNOWN28_OFFSET)= number;
		if (*(const long *)(world+SIMULATION_WORLD_STATE_OFFSET)==_simulation_world_state3)
		{
			simulation_world_clear_updates(world);
			*(long *)(world+SIMULATION_WORLD_LAST_UPDATE_OFFSET)= (long)((unsigned long)number-1);
			*(long *)(world+SIMULATION_WORLD_FIRST_UPDATE_OFFSET)= number;
		}
		world[SIMULATION_WORLD_GOING_OFFSET]= TRUE;
		simulation_world_unknown_00069350(TRUE, *(byte *const *)(view+SIMULATION_VIEW_WORLD_OFFSET));

		return TRUE;
	}
	if (*(const long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)==0)
	{
		simulation_view_set_state(view, _simulation_view_state0, NONE);
		*(long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)= _simulation_view_unknown28_game_state_failed;
	}

	return FALSE;
}
