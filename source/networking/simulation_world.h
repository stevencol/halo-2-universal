/*
SIMULATION_WORLD.H

What unknown_004cf770 has going of a game in the network, and the views
that it has of it: one for each channel of the game that it talks by. The
name of the file is a guess, and so are the names of its functions.
*/

#ifndef __SIMULATION_WORLD_H
#define __SIMULATION_WORLD_H

#include "cseries/cseries.h"

/* ---------- constants */

enum
{
	/* of the thing: its views, of which each is there or not */
	SIMULATION_WORLD_VIEWS_OFFSET= 0x40,
	NUMBER_OF_SIMULATION_WORLD_VIEWS= 15,
	/* of a view: its kind, the thing that it is of, and the channel of the
	game that it is of, by its number in the list */
	SIMULATION_VIEW_KIND_OFFSET= 0x2,
	SIMULATION_VIEW_WORLD_OFFSET= 0xC,
	SIMULATION_VIEW_CHANNEL_OFFSET= 0x3C,
	/* more of the thing: what it has of each player of the game, which
	starts with a number that is NONE if there is no such player, and has a
	byte */
	SIMULATION_WORLD_PLAYERS_OFFSET= 0x7C,
	SIMULATION_WORLD_PLAYER_SIZE= 0x88,
	NUMBER_OF_SIMULATION_WORLD_PLAYERS= 16,
	SIMULATION_WORLD_PLAYER_UNKNOWN25_OFFSET= 0x25,
	/* the kinds of view that simulation_world_unknown_0006acb0 looks for */
	SIMULATION_WORLD_UNKNOWN_0006ACB0_KINDS= 0xA,
	/* more of the thing: memory that the texture cache lent, and a number
	that is NONE with none */
	SIMULATION_WORLD_UNKNOWN1200_OFFSET= 0x1200,
	SIMULATION_WORLD_MEMORY_OFFSET= 0x1204,
	/* more of the thing: its state, which is nothing when it is not in use;
	a number; a byte that says that it is going, one that says that the game
	is to be brought up to what has come, and one that says that something
	came that it could not take; and the numbers of the first and of the last
	of the things that have come and wait */
	SIMULATION_WORLD_STATE_OFFSET= 0x8,
	SIMULATION_WORLD_UNKNOWN18_OFFSET= 0x18,
	SIMULATION_WORLD_GOING_OFFSET= 0x24,
	SIMULATION_WORLD_CATCH_UP_OFFSET= 0x25,
	SIMULATION_WORLD_UNKNOWN2C_OFFSET= 0x2C,
	SIMULATION_WORLD_FIRST_UPDATE_OFFSET= 0x120C,
	SIMULATION_WORLD_LAST_UPDATE_OFFSET= 0x1210,
	_simulation_world_state2= 2,
	_simulation_world_state3= 3,
	_simulation_world_unknown18_4= 4,
	/* more of a view: a number that goes with its state (see
	simulation_view.h); and a number of something else that is counted */
	SIMULATION_VIEW_UNKNOWN34_OFFSET= 0x34,
	SIMULATION_WORLD_UNKNOWN_0006A2A0_COUNT_OFFSET= 0x38,
	/* more of the thing: a number that the ones of its views of the second
	kind are counted from, and how far before it one of them can be */
	SIMULATION_WORLD_UNKNOWN28_OFFSET= 0x28,
	SIMULATION_WORLD_UNKNOWN_00069790_REACH= 0x81,
	_simulation_view_kind2_bit= 0x4,
	/* the views of the second kind and of the fourth; of a view, six bytes
	that it is known by, and a number that it is known by too */
	SIMULATION_VIEW_KINDS_2_AND_4= 0x14,
	SIMULATION_VIEW_UNKNOWN14_OFFSET= 0x14,
	SIMULATION_VIEW_UNKNOWN14_SIZE= 6,
	SIMULATION_VIEW_UNKNOWN1C_OFFSET= 0x1C,
	/* more of the thing: how many things have come and wait, the first of
	them and the last, each in memory of the heap of the network, of so many
	bytes and then where the next one is */
	SIMULATION_WORLD_UPDATE_COUNT_OFFSET= 0x1214,
	SIMULATION_WORLD_FIRST_UPDATE_POINTER_OFFSET= 0x1218,
	SIMULATION_WORLD_LAST_UPDATE_POINTER_OFFSET= 0x121C,
	SIMULATION_UPDATE_SIZE= 0x4048,
	/* more of the thing: a count that starts over when it starts to go,
	and the time of something that happens to it when the number that it has
	at 0x18 is 4 */
	SIMULATION_WORLD_UNKNOWN30_OFFSET= 0x30,
	SIMULATION_WORLD_UNKNOWN34_OFFSET= 0x34,
	_simulation_world_unknown18_3= 3,
	/* more of the thing: when the number that it has at 0x18 was last 3,
	and a number that it was given then */
	SIMULATION_WORLD_UNKNOWN1C_OFFSET= 0x1C,
	SIMULATION_WORLD_UNKNOWN20_OFFSET= 0x20,
	/* more values of the number that the thing has at 0x18; and a byte of a
	view that says that it is ready */
	_simulation_world_unknown18_1= 1,
	_simulation_world_unknown18_2= 2,
	_simulation_world_unknown18_6= 6,
	SIMULATION_VIEW_READY_OFFSET= 0x75,
	/* the most bytes of the state of the game that the thing is sent */
	MAXIMUM_SIMULATION_WORLD_GAME_STATE_SIZE= 0x40000,
	/* of what the thing has of a player: which player of the game it is,
	its state, when what it does last came, and what it does */
	SIMULATION_WORLD_PLAYER_INDEX_OFFSET= 0x4,
	SIMULATION_WORLD_PLAYER_STATE_OFFSET= 0x8,
	SIMULATION_WORLD_PLAYER_ACTIONS_TIME_OFFSET= 0x28,
	SIMULATION_WORLD_PLAYER_ACTIONS_OFFSET= 0x2C,
	SIMULATION_PLAYER_ACTIONS_SIZE= 0x5C,
	_simulation_world_player_state3= 3,
	MAXIMUM_SIMULATION_CONSOLE_PLAYERS= 4,
	/* of players_globals: a bit for each console that there is, and the six
	bytes that each is known by. Of a player of the game: its size, the
	console that it is of, and which of the players of that console it is */
	PLAYERS_GLOBALS_MACHINE_MASK_OFFSET= 0x2C,
	PLAYERS_GLOBALS_MACHINES_OFFSET= 0x30,
	NUMBER_OF_SIMULATION_MACHINES= 16,
	SIMULATION_PLAYER_DATUM_SIZE= 0x21C,
	SIMULATION_PLAYER_DATUM_MACHINE_OFFSET= 0x1A,
	SIMULATION_PLAYER_DATUM_LOCAL_INDEX_OFFSET= 0x1C,
	/* the size of the whole state of the game */
	SIMULATION_GAME_STATE_SIZE= 0x3FE000
};

/* ---------- structures */

/* what goes through the views of the thing: a bit for each kind of view
that is wanted, and the place of the next one to look at */
struct simulation_view_iterator
{
	unsigned long kind_mask;
	long index;
};

/* one of the things that have come and wait, in memory of the heap of the
network */
struct simulation_update_node
{
	byte update[SIMULATION_UPDATE_SIZE];
	struct simulation_update_node *next;
};

/* ---------- prototypes/SIMULATION_WORLD.C */

/* whether the thing is being sent the state of the game: it is in its third
state and has a count of the bytes that have come. The game has it written
wherever it asks, with more states looked at than there is need for */
static inline __attribute__((always_inline)) boolean simulation_world_receiving_game_state(
	const byte *world)
{
	return *(const long *)(world+SIMULATION_WORLD_STATE_OFFSET)==_simulation_world_state3 &&
		*(const long *)(world+SIMULATION_WORLD_UNKNOWN1200_OFFSET)!=NONE;
}

boolean __attribute__((stdcall)) simulation_world_next_view(struct simulation_view_iterator *iterator,
	const byte *world, byte **view_out);
byte *simulation_world_get_channel_view(const byte *world, long channel_index);
boolean simulation_world_player_unknown25(long player, const byte *world);
void simulation_world_players_clear_unknown25(byte *world);
byte *simulation_world_unknown_0006acb0(const byte *world);
void simulation_world_unknown_0006aae0(byte *world);
long simulation_world_unknown_00069300(byte *world, boolean *counted);
void simulation_world_unknown_00069350(boolean catch_up, byte *world);
boolean simulation_world_unknown_000698e0(byte *world, const long *update);
void simulation_world_unknown_0006a2a0(byte *view, byte *other);
long simulation_world_unknown_00069790(byte *world);
void __attribute__((stdcall)) simulation_world_unknown_00069880(byte *world, long unknown);
byte *simulation_world_get_view_by_unknown1c(const byte *world, long unknown1c);
byte *__attribute__((stdcall)) simulation_world_get_view_by_unknown14(const byte *world, const word *unknown14);
void simulation_world_take_update(byte *world, void *update);
void simulation_world_unknown_00069eb0(byte *world);
void simulation_world_unknown_0006b2a0(byte *world);
void __attribute__((stdcall)) simulation_world_unknown_0006b110(byte *world, boolean unknown, const byte *view);
void simulation_world_set_unknown18(byte *world, long unknown18);
void __attribute__((stdcall)) simulation_world_unknown_0006b1f0(byte *world, long unknown);
void simulation_world_unknown_0006b040(byte *world);
void simulation_world_unknown_00069f90(byte *world);
void simulation_world_unknown_00069fe0(byte *world);
void __attribute__((stdcall)) simulation_world_unknown_0006b090(boolean ready, byte *world, const byte *view);
void __attribute__((stdcall)) simulation_world_clear_updates(byte *world);
boolean __attribute__((stdcall)) simulation_world_add_update(byte *world, const long *update);
boolean __attribute__((stdcall)) simulation_world_receive_game_state(byte *world, long offset, long size,
	const void *data);
void simulation_world_unknown_00069f10(byte *world);
byte *__attribute__((stdcall)) simulation_world_find_player(byte *world, const word *unknown14, long local_index);
void __attribute__((stdcall)) simulation_world_set_player_actions(byte *world, const word *unknown14, long players,
	const void *actions);
boolean __attribute__((stdcall)) simulation_world_finish_game_state(byte *world, long size);

/* ---------- globals */

extern long unknown_004ced4c;
extern long unknown_004ced58;
extern long unknown_004ced5c;

#endif
