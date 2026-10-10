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

/* ---------- prototypes */

long GetTickCount(void);

/* 0x000861c0: what follows from the state of a view and the one of the
other console is seen to */
void unknown_000861c0(byte *view);

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
