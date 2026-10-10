/*
SIMULATION_VIEW.H

A view that the game in the network has of itself: what it talks with
another console by, one for each channel of the game (see
simulation_world.h). The name of the file is a guess, and so are the names of
its functions.
*/

#ifndef __SIMULATION_VIEW_H
#define __SIMULATION_VIEW_H

#include "cseries/cseries.h"

/* ---------- constants */

enum
{
	/* of a view: a number that says how it is, which is nothing while all
	is well; a byte that says that it counts, and its number, for the things
	that wait (simulation_world_unknown_00069790); and a byte that says that
	it keeps the game from going on, and since when */
	SIMULATION_VIEW_UNKNOWN28_OFFSET= 0x28,
	SIMULATION_VIEW_UNKNOWN78_OFFSET= 0x78,
	SIMULATION_VIEW_UNKNOWN84_OFFSET= 0x84,
	SIMULATION_VIEW_BLOCKED_OFFSET= 0x88,
	SIMULATION_VIEW_BLOCKED_TIME_OFFSET= 0x8C,
	/* how long a view can keep the game from going on, in milliseconds, and
	what it is given and how it is after that */
	SIMULATION_VIEW_BLOCKED_TIMEOUT= 2000,
	_simulation_view_unknown28_blocked= 4
};

/* ---------- prototypes/SIMULATION_VIEW.C */

void simulation_view_set_blocked(byte *view, boolean blocked);

/* ---------- prototypes of functions not decompiled yet */

/* 0x00085a70: a view is given two numbers */
void __attribute__((stdcall)) unknown_00085a70(byte *view, long unknown, long unknown2);

#endif
