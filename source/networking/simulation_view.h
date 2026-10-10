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
	_simulation_view_unknown28_players_missing= 8,
	/* more of a view: the state of the other end of it and the number that
	goes with it, as it last said; and something that a view that is not in
	its sixth state has to take the things that come. The kind of the
	message that one of those things goes in, its size, and the state of a
	view in which it is sent on as it is. How a view is after the other end
	went back to its first state, or to a state that does not follow */
	SIMULATION_VIEW_REMOTE_STATE_OFFSET= 0x34,
	SIMULATION_VIEW_REMOTE_STATE_NUMBER_OFFSET= 0x38,
	SIMULATION_VIEW_UNKNOWN94_OFFSET= 0x94,
	_network_message_simulation_update= 0x27,
	SIMULATION_VIEW_UPDATE_SIZE= 0x4048,
	_simulation_view_state1= 1,
	_simulation_view_state5= 5,
	_simulation_view_unknown28_remote_reset= 6,
	/* the kind of the message that says what the players of a console do */
	_network_message_simulation_player_actions= 0x28,
	/* more of a view: something that a view of the third kind or of the
	fourth has, of which unknown_00086b40 is given what is at 0x6060; memory
	that the texture cache lent, and three numbers that go with it; and a
	number of a view of the first kind */
	SIMULATION_VIEW_UNKNOWN8_OFFSET= 0x8,
	SIMULATION_VIEW_UNKNOWN8_UNKNOWN6060_OFFSET= 0x6060,
	SIMULATION_VIEW_UNKNOWN98_OFFSET= 0x98,
	SIMULATION_VIEW_UNKNOWN9C_OFFSET= 0x9C,
	SIMULATION_VIEW_UNKNOWNA0_OFFSET= 0xA0,
	SIMULATION_VIEW_UNKNOWNB0_OFFSET= 0xB0,
	/* how much of the state of the game a view has left to send */
	SIMULATION_VIEW_UNKNOWNAC_OFFSET= 0xAC,
	/* of what a view has at 0x8: the view that it has of the things that the
	consoles share (replication_entity_view.h). Of the thing: something at
	0x4, which has at 0x2098 what the things are: at 0x10 of that, the ones
	that each kind of thing is seen to by, from the second place on; and
	from 0x14, 0x20 bytes of each thing, with its kind at 0x4 */
	SIMULATION_VIEW_UNKNOWN8_ENTITY_VIEW_OFFSET= 0x30,
	SIMULATION_WORLD_UNKNOWN4_OFFSET= 0x4,
	SIMULATION_WORLD_UNKNOWN4_DATABASE_OFFSET= 0x2098,
	SIMULATION_DATABASE_CLIENTS_OFFSET= 0x10,
	SIMULATION_DATABASE_ENTITIES_OFFSET= 0x14,
	SIMULATION_DATABASE_ENTITY_SIZE= 0x20,
	SIMULATION_DATABASE_ENTITY_TYPE_OFFSET= 0x4,
	SIMULATION_DATABASE_ENTITY_INDEX_MASK= 0x3FF,
	/* of what a view has at 0x6060 of what it has at 0x8: the view; a byte
	that is set while a view of the second kind or of the fourth is ready,
	and one for any other; a number; what is emptied when the view is ready,
	and a number in it */
	SIMULATION_VIEW_EVENTS_VIEW_OFFSET= 0x0,
	SIMULATION_VIEW_EVENTS_UNKNOWN5_OFFSET= 0x5,
	SIMULATION_VIEW_EVENTS_UNKNOWN6_OFFSET= 0x6,
	SIMULATION_VIEW_EVENTS_UNKNOWN8_OFFSET= 0x8,
	SIMULATION_VIEW_EVENTS_DATA_OFFSET= 0xC,
	SIMULATION_VIEW_EVENTS_DATA_SIZE= 0x4CC0,
	SIMULATION_VIEW_EVENTS_UNKNOWN4CCC_OFFSET= 0x4CCC,
	/* the ring of a view of the second kind, in the memory at 0x94; the
	kinds of what it keeps, the most bytes of a piece of the state of the
	game, and how hard the state is packed */
	SIMULATION_VIEW_RING_OFFSET= 0x9C,
	_simulation_view_game_state_record_start= 0,
	_simulation_view_game_state_record_piece= 1,
	MAXIMUM_SIMULATION_VIEW_GAME_STATE_PIECE_SIZE= 0x400,
	SIMULATION_VIEW_GAME_STATE_COMPRESSION_LEVEL= 9,
	/* the size of the memory of the ring, and of the memory that the state
	of the game is packed into */
	SIMULATION_VIEW_RING_SIZE= 0x80000,
	SIMULATION_VIEW_PACKED_GAME_STATE_SIZE= 0x40000,
	_simulation_view_kind_1= 1,
	_simulation_view_kind_2= 2,
	_simulation_view_kind_3= 3,
	_simulation_view_kind_4= 4
};

/* ---------- structures */

/* what a view says to the other console when its state is another */
struct simulation_view_state_message
{
	long state;
	long number;
};

/* what a view of the first kind says of what its players do: a count of
these messages, the number of the last thing that the thing took, whether
something came that it could not take, a bit for each player that it says
of, and what each of the four does */
struct simulation_view_player_actions_message
{
	long count;
	long update;
	boolean failed;
	byte pad9[3];
	unsigned long players;
	byte actions[4][0x5C];
};

/* what goes before each thing that a view of the second kind keeps in its
ring to send to a console that comes in: its kind, the size of what follows,
and a number: for the first kind, the one that the game goes on from; for the
second, where a piece of the state of the game goes */
struct simulation_view_game_state_record
{
	short type;
	short size;
	long number;
};

typedef char verify_simulation_view_player_actions_message_size[
	sizeof(struct simulation_view_player_actions_message) == 0x180 ? 1 : -1];

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
void simulation_view_send_update(byte *view, const void *update);
boolean simulation_view_handle_remote_state(byte *view, long state, long number);
void simulation_view_update_ready(byte *view);
void simulation_view_unknown_00086590(byte *view);
boolean simulation_view_nothing_to_send(byte *view);
void __attribute__((stdcall)) simulation_view_send_player_actions(byte *view, unsigned long players,
	const void *actions);
boolean simulation_view_entities_pending(byte *view);
void simulation_view_events_set_ready(boolean ready, byte *events);
void __attribute__((stdcall)) simulation_view_reclaim_game_state(void *address, byte *view);
boolean __attribute__((stdcall)) simulation_view_queue_game_state(long capacity, byte *view, long size,
	const void *buffer);
boolean simulation_view_begin_send_game_state(byte *view);

/* ---------- globals */

extern boolean unknown_00510ca2;

#endif
