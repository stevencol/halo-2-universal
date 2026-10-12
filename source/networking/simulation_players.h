/*
SIMULATION_PLAYERS.H

What the simulation of a game keeps of its players and of the consoles that
they are on: what unknown_004cf780 has. Worked out from halo2ship.exe; the
name of the file is a guess, and so are the names of its functions.
*/

#ifndef __SIMULATION_PLAYERS_H
#define __SIMULATION_PLAYERS_H

#include "cseries/cseries.h"

/* ---------- constants */

enum
{
	/* of the thing: its world and its session; four numbers that start as
	NONE; the bits of the consoles that there are, and what each is known
	by, in six bytes; whether something changed; the bits of the players
	that there are, and what there is of each; and a number and 0x60 bytes
	of players_globals, and a byte */
	SIMULATION_PLAYERS_WORLD_OFFSET= 0x4,
	SIMULATION_PLAYERS_SESSION_OFFSET= 0xC,
	SIMULATION_PLAYERS_UNKNOWN10_OFFSET= 0x10,
	SIMULATION_PLAYERS_UNKNOWN14_OFFSET= 0x14,
	SIMULATION_PLAYERS_UNKNOWN18_OFFSET= 0x18,
	SIMULATION_PLAYERS_MACHINE_MASK_OFFSET= 0x1C,
	SIMULATION_PLAYERS_UNKNOWN20_OFFSET= 0x20,
	SIMULATION_PLAYERS_MACHINES_OFFSET= 0x24,
	SIMULATION_PLAYERS_MACHINE_IDENTIFIER_SIZE= 6,
	MAXIMUM_SIMULATION_PLAYERS_MACHINES= 16,
	SIMULATION_PLAYERS_CHANGED_OFFSET= 0x84,
	SIMULATION_PLAYERS_COLLECTION_OFFSET= 0x88,
	SIMULATION_PLAYERS_PLAYERS_OFFSET= 0x8C,
	SIMULATION_PLAYERS_PLAYER_SIZE= 0xB4,
	SIMULATION_PLAYERS_PLAYER_IDENTIFIER_SIZE= 12,
	SIMULATION_PLAYERS_PLAYER_UNKNOWNC_OFFSET= 0xC,
	MAXIMUM_SIMULATION_PLAYERS= 16,
	SIMULATION_PLAYERS_UNKNOWNBCC_OFFSET= 0xBCC,
	SIMULATION_PLAYERS_UNKNOWNBD0_OFFSET= 0xBD0,
	SIMULATION_PLAYERS_UNKNOWNBD0_SIZE= 0x60,
	SIMULATION_PLAYERS_UNKNOWNC30_OFFSET= 0xC30,
	/* of the world: what it is at, where its players are, and of one of
	them two bytes */
	SIMULATION_PLAYERS_WORLD_STATE_OFFSET= 0x8,
	SIMULATION_PLAYERS_WORLD_PLAYERS_OFFSET= 0x7C,
	SIMULATION_PLAYERS_WORLD_PLAYER_SIZE= 0x88,
	SIMULATION_PLAYERS_WORLD_PLAYER_UNKNOWN24_OFFSET= 0x24,
	SIMULATION_PLAYERS_WORLD_PLAYER_UNKNOWN25_OFFSET= 0x25,
	/* of the session: something that is 4 when the game is played, and a
	number that the thing keeps a copy of */
	SIMULATION_PLAYERS_SESSION_UNKNOWN4980_OFFSET= 0x4980,
	SIMULATION_PLAYERS_SESSION_UNKNOWN7618_OFFSET= 0x7618,
	/* of what another console says of a player, of 0x13C bytes: who it is,
	its console, and two numbers; and of what is compared with it */
	SIMULATION_PLAYERS_UPDATE_SIZE= 0x13C,
	SIMULATION_PLAYERS_UPDATE_MACHINE_OFFSET= 0xC,
	SIMULATION_PLAYERS_UPDATE_UNKNOWN10_OFFSET= 0x10,
	SIMULATION_PLAYERS_NEW_MACHINE_OFFSET= 0x14,
	SIMULATION_PLAYERS_NEW_UNKNOWN1C_OFFSET= 0x1C,
	/* of what there is of the players: the bits of the ones that there
	are, and of each: who it is; whether it left; a number of one that left;
	its console; two numbers; and what the others are told of it */
	SIMULATION_PLAYERS_COLLECTION_SIZE= 0xB44,
	SIMULATION_PLAYERS_COLLECTION_PLAYERS_OFFSET= 0x4,
	SIMULATION_PLAYERS_PLAYER_LEFT_OFFSET= 0xC,
	SIMULATION_PLAYERS_PLAYER_UNKNOWN10_OFFSET= 0x10,
	SIMULATION_PLAYERS_PLAYER_MACHINE_OFFSET= 0x14,
	SIMULATION_PLAYERS_PLAYER_UNKNOWN1C_OFFSET= 0x1C,
	SIMULATION_PLAYERS_PLAYER_UNKNOWN20_OFFSET= 0x20,
	SIMULATION_PLAYERS_PLAYER_PROPERTIES_OFFSET= 0x24,
	SIMULATION_PLAYERS_PLAYER_PROPERTIES_SIZE= 0x90,
	/* of a player of the game: its bits, of which the second says that it
	left; who it is; and the same things */
	PLAYER_DATUM_FLAGS_OFFSET= 0x2,
	_player_datum_left_bit= 1,
	PLAYER_DATUM_IDENTIFIER_OFFSET= 0x4,
	PLAYER_DATUM_UNKNOWN10_OFFSET= 0x10,
	PLAYER_DATUM_MACHINE_OFFSET= 0x14,
	PLAYER_DATUM_UNKNOWN1C_OFFSET= 0x1C,
	PLAYER_DATUM_UNKNOWN20_OFFSET= 0x20,
	PLAYER_DATUM_PROPERTIES_OFFSET= 0xD4,
	/* of what another console says of a player: its place, and who it is */
	SIMULATION_PLAYERS_UPDATE_IDENTIFIER_OFFSET= 0x4,
	/* and what the others are told of it */
	SIMULATION_PLAYERS_UPDATE_PROPERTIES_OFFSET= 0x28,
	/* and, in what it says of two players, the other place and who is
	there */
	SIMULATION_PLAYERS_UPDATE_OTHER_INDEX_OFFSET= 0xB8,
	SIMULATION_PLAYERS_UPDATE_OTHER_IDENTIFIER_OFFSET= 0xBC,
	/* the kinds of what is said of a player, and where the kind is: it
	left; it and another change places; it is not there any more; it is
	new; and what the others are told of it is something else. And of what
	is said of a new one: whether it has left already, its console and its
	two numbers */
	SIMULATION_PLAYERS_UPDATE_TYPE_OFFSET= 0x10,
	_simulation_players_update_left= 0,
	_simulation_players_update_swap,
	_simulation_players_update_remove,
	_simulation_players_update_add,
	_simulation_players_update_properties,
	SIMULATION_PLAYERS_UPDATE_NEW_MACHINE_OFFSET= 0x14,
	SIMULATION_PLAYERS_UPDATE_NEW_UNKNOWN1C_OFFSET= 0x1C,
	SIMULATION_PLAYERS_UPDATE_NEW_UNKNOWN20_OFFSET= 0x20,
	SIMULATION_PLAYERS_UPDATE_NEW_LEFT_OFFSET= 0x24,
	/* more of it: something of the game in the network, which gets it
	at 0x80 and NONE for two numbers after that; and of the thing that is
	going, a byte that says that it has the six bytes of this console,
	those bytes, and which console this is */
	SIMULATION_PLAYERS_UNKNOWN8_OFFSET= 0x8,
	SIMULATION_PLAYERS_UNKNOWN8_PLAYERS_OFFSET= 0x80,
	SIMULATION_PLAYERS_UNKNOWN8_UNKNOWN84_OFFSET= 0x84,
	SIMULATION_PLAYERS_UNKNOWN8_UNKNOWN88_OFFSET= 0x88,
	SIMULATION_PLAYERS_WORLD_HAS_MACHINE_OFFSET= 0xC,
	SIMULATION_PLAYERS_WORLD_MACHINE_OFFSET= 0xD,
	SIMULATION_PLAYERS_WORLD_MACHINE_INDEX_OFFSET= 0x14,
	/* of a session of the network: its state, the consoles in it (a
	number, how many, and from 0x16 the six bytes of each, 0x10C bytes
	from one to the next) and which of them this one is */
	SIMULATION_PLAYERS_SESSION_STATE_OFFSET= 0x741C,
	SIMULATION_PLAYERS_SESSION_MEMBERSHIP_OFFSET= 0x4C,
	SIMULATION_PLAYERS_SESSION_LOCAL_PEER_OFFSET= 0x72D8,
	SIMULATION_PLAYERS_MEMBERSHIP_PEER_COUNT_OFFSET= 0x8,
	SIMULATION_PLAYERS_MEMBERSHIP_PEER_MACHINE_OFFSET= 0x16,
	SIMULATION_PLAYERS_MEMBERSHIP_PEER_SIZE= 0x10C,
	/* of the players of the game: which consoles there are, the six bytes
	of each, whether this console is one of them, and its six bytes */
	SIMULATION_PLAYERS_GLOBALS_MACHINE_MASK_OFFSET= 0x2C,
	SIMULATION_PLAYERS_GLOBALS_MACHINES_OFFSET= 0x30,
	SIMULATION_PLAYERS_GLOBALS_LOCAL_MACHINE_EXISTS_OFFSET= 0x90,
	SIMULATION_PLAYERS_GLOBALS_LOCAL_MACHINE_OFFSET= 0x91
};

/* ---------- prototypes/SIMULATION_PLAYERS.C */

void __attribute__((stdcall)) simulation_players_initialize_for_new_map(byte *players);
void __attribute__((stdcall)) simulation_players_after_load(byte *players, unsigned long flags);

boolean __attribute__((stdcall)) simulation_players_take_update(byte *identifiers, byte *machines, boolean always,
	byte *players, long *machine_mask, long *player_mask, long *active_mask);

void simulation_players_collection_clear(byte *collection);
void __attribute__((stdcall)) simulation_players_collection_build(byte *collection);
unsigned long simulation_players_collection_get_active_mask(const byte *collection);
void __attribute__((stdcall)) simulation_players_collection_apply_update(byte *collection, const byte *update);
void __attribute__((stdcall)) simulation_players_collection_swap(byte *update, long player_index, byte *collection,
	long other_player_index);
boolean simulation_players_update_remove_player(const byte *update);
boolean simulation_players_update_unknown_000850c0(const byte *update);
boolean __attribute__((stdcall)) simulation_players_update_set_properties(const byte *update);
boolean __attribute__((stdcall)) simulation_players_update_unknown_00084fb0(const byte *update);
void simulation_players_reset(byte *players);
void simulation_players_refresh(byte *players);
void simulation_players_mark_player(const byte *players, long player_index);
boolean simulation_players_changed(const byte *players);
long __attribute__((stdcall)) simulation_players_find_machine(const byte *players, const byte *identifier);
boolean __attribute__((stdcall)) simulation_players_has_player(long player_index, const byte *players,
	const byte *identifier);
boolean __attribute__((stdcall)) simulation_players_player_differs(long player_index, const byte *player,
	const byte *machines, unsigned long mask, const byte *updates);

#endif
