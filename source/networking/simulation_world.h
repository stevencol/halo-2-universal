/*
SIMULATION_WORLD.H

What unknown_004cf770 has going of a game in the network, and the views
that it has of it: one for each channel of the game that it talks by. The
name of the file is a guess, and so are the names of its functions.
*/

#ifndef __SIMULATION_WORLD_H
#define __SIMULATION_WORLD_H

#include "cseries/cseries.h"
#include "memory/data.h"
#include "networking/simulation_view.h"

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
	SIMULATION_GAME_STATE_SIZE= 0x3FE000,
	/* more of what the thing has of a player: a number that is nothing
	when there is none. And after the players, sixteen other things: each
	starts with its own number, or NONE, and has a number, the thing, one
	more number, and something at 0x14 */
	SIMULATION_WORLD_PLAYER_UNKNOWN20_OFFSET= 0x20,
	SIMULATION_WORLD_OTHERS_OFFSET= 0x8FC,
	SIMULATION_WORLD_OTHER_SIZE= 0x90,
	NUMBER_OF_SIMULATION_WORLD_OTHERS= 16,
	SIMULATION_WORLD_OTHER_UNKNOWN4_OFFSET= 0x4,
	SIMULATION_WORLD_OTHER_WORLD_OFFSET= 0x8,
	SIMULATION_WORLD_OTHER_UNKNOWN10_OFFSET= 0x10,
	SIMULATION_WORLD_OTHER_UNKNOWN14_OFFSET= 0x14,
	/* of a player of the game: a byte with a bit that says that the thing
	is not to have it, and a number */
	SIMULATION_PLAYER_DATUM_FLAGS_OFFSET= 0x2,
	_simulation_player_datum_unknown_bit= 0x2,
	SIMULATION_PLAYER_DATUM_UNKNOWN2C_OFFSET= 0x2C,
	/* of a view: a bit for each player of the game that it knows of */
	SIMULATION_VIEW_PLAYER_MASK_OFFSET= 0x7C,
	/* of what the thing has of a player: three numbers that it is known
	by; and what the thing has first: something of the session */
	SIMULATION_WORLD_PLAYER_IDENTIFIER_OFFSET= 0xC,
	SIMULATION_PLAYER_IDENTIFIER_SIZE= 12,
	SIMULATION_WORLD_UNKNOWN0_OFFSET= 0x0,
	/* of what the thing has first: a bit for each console that there is. Of
	the thing: which console it is itself. And one more value of the number
	that it has at 0x18 */
	SIMULATION_WORLD_UNKNOWN0_MACHINE_MASK_OFFSET= 0x1C,
	SIMULATION_WORLD_MACHINE_INDEX_OFFSET= 0x14,
	_simulation_world_unknown18_5= 5,
	_simulation_world_state1= 1,
	/* more of the thing: when it last started to send a view the state of
	the game, or NONE. More of a view: how many times it has been sent it;
	and a byte of what it has at 0x30 of what it has at 0x8. What bringing a
	view on says: nothing yet, there, or too many times */
	SIMULATION_WORLD_GAME_STATE_SEND_TIME_OFFSET= 0x1208,
	SIMULATION_VIEW_GAME_STATE_SEND_COUNT_OFFSET= 0x90,
	SIMULATION_VIEW_UNKNOWN8_UNKNOWN39_OFFSET= 0x39,
	_simulation_world_advance_view_waiting= 0,
	_simulation_world_advance_view_there= 1,
	_simulation_world_advance_view_failed= 3,
	/* of what the thing has first: a bit for each player that there is,
	and what there is of each: the three numbers that it is known by, and a
	byte that says that it is not to be counted yet. Of what the thing has
	of a player: a byte that keeps its other byte from being set. Of the
	thing: a byte that is set while the state of the game is sent */
	SIMULATION_WORLD_UNKNOWN0_PLAYER_MASK_OFFSET= 0x88,
	SIMULATION_WORLD_UNKNOWN0_PLAYERS_OFFSET= 0x8C,
	SIMULATION_WORLD_UNKNOWN0_PLAYER_SIZE= 0xB4,
	SIMULATION_WORLD_UNKNOWN0_PLAYER_UNKNOWNC_OFFSET= 0xC,
	SIMULATION_WORLD_PLAYER_UNKNOWN24_OFFSET= 0x24,
	SIMULATION_WORLD_SENDING_GAME_STATE_OFFSET= 0x11FC,
	/* the size of what one of the other things of the thing has at 0x14 */
	SIMULATION_WORLD_OTHER_UNKNOWN14_SIZE= 0x7C,
	/* of a player of the game: the numbers that it is known by, the bytes of
	its console, and which gamepad it is of, or NONE for a player of another
	console. Of the game: the way that it is played in the network, from 1
	to 5 */
	SIMULATION_PLAYER_DATUM_IDENTIFIER_OFFSET= 0x4,
	SIMULATION_PLAYER_DATUM_MACHINE_IDENTIFIER_OFFSET= 0x14,
	SIMULATION_PLAYER_DATUM_CONTROLLER_OFFSET= 0x28,
	MAIN_GAME_GLOBALS_SIMULATION_OFFSET= 0xC,
	/* of a view: its place in the table of the views, and in the table of
	what the ones of the third kind and the fourth have at 0x8. Of that:
	where the four things in it that have functions have them */
	SIMULATION_VIEW_DATUM_INDEX_OFFSET= 0x4,
	SIMULATION_VIEW_DATA_OFFSET= 0x8,
	SIMULATION_VIEW_DATA_UNKNOWN4_OFFSET= 0x4,
	SIMULATION_VIEW_DATA_UNKNOWN30_OFFSET= 0x30,
	SIMULATION_VIEW_DATA_UNKNOWN5070_OFFSET= 0x5070,
	SIMULATION_VIEW_DATA_UNKNOWN5098_OFFSET= 0x5098,
	/* a byte of one of the other things of the thing */
	SIMULATION_WORLD_OTHER_UNKNOWNC_OFFSET= 0xC,
	/* more of the thing: bytes and numbers that are cleared when it is set up,
	and a number. In that memory: what tells of the things
	that are, of the things that happen, what the things are, and what the
	things that happen are; and of this last, a byte that says that it is
	set up, the thing, what tells of the things that happen, the kinds and
	what the things are */
	SIMULATION_WORLD_UNKNOWNC_OFFSET= 0xC,
	SIMULATION_WORLD_UNKNOWN2E_OFFSET= 0x2E,
	SIMULATION_WORLD_UNKNOWN2F_OFFSET= 0x2F,
	SIMULATION_WORLD_UNKNOWN38_OFFSET= 0x38,
	SIMULATION_WORLD_MEMORY_EVENT_MANAGER_OFFSET= 0x2048,
	SIMULATION_WORLD_MEMORY_ENTITY_DATABASE_OFFSET= 0x2098,
	SIMULATION_WORLD_MEMORY_EVENT_DATABASE_OFFSET= 0xA0AC,
	SIMULATION_EVENT_DATABASE_INITIALIZED_OFFSET= 0x4,
	SIMULATION_EVENT_DATABASE_WORLD_OFFSET= 0x8,
	SIMULATION_EVENT_DATABASE_MANAGER_OFFSET= 0xC,
	SIMULATION_EVENT_DATABASE_TYPES_OFFSET= 0x10,
	SIMULATION_EVENT_DATABASE_ENTITY_DATABASE_OFFSET= 0x14,
	/* the kinds of view that are told of the players (the first and the
	third), and the kind of the message that tells them */
	SIMULATION_VIEW_KINDS_1_AND_3= 0xA,
	_network_message_simulation_players= 0x26
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

/* ---------- structures */

/* what tells another console of the players: which there are, which of
them have not left, and what each is known by */
struct simulation_players_message
{
	long player_mask;
	long active_mask;
	byte identifiers[0xC0];
};

typedef char verify_simulation_players_message_size[
	sizeof(struct simulation_players_message) == 0xC8 ? 1 : -1];

/* ---------- prototypes/SIMULATION_WORLD.C */

struct simulation_type_table;

void __attribute__((stdcall)) simulation_world_initialize(long unknown, byte *memory,
	struct simulation_type_table *types, byte *world);
void simulation_world_restart(byte *world);
boolean simulation_world_become_client(byte *world);
boolean simulation_world_become_authority(byte *world);
void __attribute__((stdcall)) simulation_world_send_players(byte *world, boolean always);
void simulation_world_update(byte *world);
void __attribute__((stdcall)) simulation_world_dispose(byte *world);

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
void __attribute__((stdcall)) simulation_world_unknown_00069880(byte *world, const void *update);
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
void simulation_world_players_clear(byte *world);
void simulation_world_others_clear(byte *world);
long __attribute__((stdcall)) simulation_world_other_new(long index, byte *world, long unknown4);
long simulation_world_unknown_000699a0(long index, const byte *world);
void __attribute__((stdcall)) simulation_world_unknown_0006a770(byte *world);
void simulation_world_player_delete(long player, byte *world);
unsigned long simulation_world_players_unknown25_mask(const byte *world);
unsigned long simulation_world_players_known_mask(const byte *world);
boolean __attribute__((stdcall)) simulation_world_player_matches(long player, const byte *world,
	const long *identifier);
void simulation_world_unknown_0006b310(byte *world);
void simulation_world_unknown_0006b350(byte *world);
void simulation_world_unknown_00069d50(byte *world);
void simulation_world_unknown_00069c80(byte *world);
long simulation_world_advance_view(byte *view, byte *world);
void simulation_world_unknown_00069dd0(byte *world);
boolean __attribute__((stdcall)) simulation_world_begin_game_state(byte *world);
void __attribute__((stdcall)) simulation_world_update_players_unknown25(byte *world);
void __attribute__((stdcall)) simulation_world_before_send_game_state(byte *world, long *size_out);
void simulation_world_unknown_00069c50(byte *world);
void simulation_world_build_update(byte *world, long *update);
void __attribute__((stdcall)) simulation_world_set_local_player_actions(const void *actions, byte *world,
	unsigned long players);
void __attribute__((stdcall)) simulation_world_set_other_unknown14(byte *world, long index, const void *unknown14);
void __attribute__((stdcall)) simulation_world_add_player(byte *world, long player);
void __attribute__((stdcall)) simulation_world_delete_views(void *world);
void simulation_world_construct(byte *world);

/* whether a view is past the states in which it is made ready: one of the
second kind from the sixth state on, and any other from the fourth. The game
has it written wherever it asks */
static inline __attribute__((always_inline)) boolean simulation_view_established(
	const byte *view)
{
	long state= *(const long *)(view+0x2C);

	return *(const word *)(view+SIMULATION_VIEW_KIND_OFFSET)==2 ? state>=5 : state>=3;
}

/* ---------- globals */

extern struct data_array *unknown_004d87ec;
extern struct data_array *unknown_004d87f0;
extern void *unknown_00450c94[1];
extern void *unknown_00450cd0[1];
extern void *unknown_00450cf4[1];
extern void *unknown_00450d1c[1];
extern long unknown_004ced48;
extern long unknown_004ced4c;
extern long unknown_004ced50;
extern long unknown_004ced54;
extern long unknown_004ced58;
extern long unknown_004ced5c;

/* ---------- code */

/* a view is done with, as simulation_world_delete_views tells: the game has
it written in each of the three that do it */
static inline __attribute__((always_inline)) void simulation_world_view_delete(
	byte *view)
{
	byte *data;
	long index;

	if (*(const word *)(view+SIMULATION_VIEW_KIND_OFFSET)!=0)
	{
		if (*(const long *)(view+SIMULATION_VIEW_OBSERVER_CHANNEL_OFFSET)!=NONE)
		{
			simulation_view_release_channel(view);
		}
		if (*(void *const *)(view+SIMULATION_VIEW_WORLD_OFFSET))
		{
			simulation_view_detach(view);
		}
		*(word *)(view+SIMULATION_VIEW_KIND_OFFSET)= 0;
	}
	data= *(byte *const *)(view+SIMULATION_VIEW_DATA_OFFSET);
	index= *(const long *)(view+SIMULATION_VIEW_DATUM_INDEX_OFFSET);
	datum_delete(unknown_004d87ec, index);
	if (data)
	{
		*(void **)(data+SIMULATION_VIEW_DATA_UNKNOWN5098_OFFSET)= unknown_00450c94;
		*(void **)(data+SIMULATION_VIEW_DATA_UNKNOWN5070_OFFSET)= unknown_00450d1c;
		*(void **)(data+SIMULATION_VIEW_DATA_UNKNOWN30_OFFSET)= unknown_00450cd0;
		*(void **)(data+SIMULATION_VIEW_DATA_UNKNOWN4_OFFSET)= unknown_00450cf4;
		datum_delete(unknown_004d87f0, index);
	}
}

#endif
