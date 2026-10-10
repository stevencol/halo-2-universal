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
	_simulation_view_unknown28_blocked= 4,
	/* more of a view: the observer of the network and the channel of it
	that it talks by, or NONE; and its state, with the number that goes with
	it */
	SIMULATION_VIEW_OBSERVER_OFFSET= 0x20,
	SIMULATION_VIEW_OBSERVER_CHANNEL_OFFSET= 0x24,
	SIMULATION_VIEW_STATE_OFFSET= 0x2C,
	SIMULATION_VIEW_STATE_NUMBER_OFFSET= 0x30,
	/* the states that have no number, the one that has any that is not
	less than nothing, and how a view is after it was given a state that it
	cannot go to */
	_simulation_view_state0= 0,
	_simulation_view_state2= 2,
	_simulation_view_unknown28_bad_state= 7,
	/* the kind of the message that says the state of a view, and what it
	is sent for */
	_network_message_simulation_view_state= 0x25,
	SIMULATION_VIEW_SESSION_INDEX= 3,
	/* more of a view: two numbers of the last that came of what the players
	of the other console do (what unknown_0006a7f0 is given is the six bytes
	that the view is known by: see simulation_world.h). How a view is after the other
	console says that it could not go on, and after the state of the game
	that it was sent could not be taken */
	SIMULATION_VIEW_UNKNOWN80_OFFSET= 0x80,
	_simulation_view_unknown28_remote_failed= 2,
	_simulation_view_unknown28_game_state_failed= 5,
	/* how a view is after it is told of fewer players than it has to know */
	_simulation_view_unknown28_players_missing= 8
};

/* ---------- structures */

/* what a view says to the other console when its state is another */
struct simulation_view_state_message
{
	long state;
	long number;
};

/* ---------- prototypes/SIMULATION_VIEW.C */

void simulation_view_set_blocked(byte *view, boolean blocked);
void __attribute__((stdcall)) simulation_view_set_state(byte *view, long state, long number);
void __attribute__((stdcall)) simulation_view_fail(byte *view, long reason);
boolean __attribute__((stdcall)) simulation_view_unknown_00085f50(byte *view, boolean failed, long update,
	long unknown0, long players, const void *actions);
boolean __attribute__((stdcall)) simulation_view_unknown_00085ed0(byte *view, long offset, long size,
	const void *data);
boolean __attribute__((stdcall)) simulation_view_unknown_00085dc0(byte *view, long number);
boolean __attribute__((stdcall)) simulation_view_set_players(byte *view, unsigned long players,
	unsigned long valid, const long *identifiers);

#endif
