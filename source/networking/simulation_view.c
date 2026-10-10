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

/* ---------- globals */

/* 0x00510548, 0x0051054c, of main.c: whether the time is the one that was
kept, and that time */
extern boolean unknown_00510548;
extern long unknown_0051054c;

/* ---------- code */

/* 0x00086140: a view is told whether it keeps the game from going on. When
it starts to, the time is noted. And one that has done so for two seconds,
and is well, is given nothing and NONE (unknown_00085a70) and is then as 4
says. The game has the time of the network, and network_time_since, written
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
		unknown_00085a70(view, 0, NONE);
		*(long *)(view+SIMULATION_VIEW_UNKNOWN28_OFFSET)= _simulation_view_unknown28_blocked;
	}
}
