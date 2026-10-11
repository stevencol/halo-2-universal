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
	SIMULATION_PLAYERS_UPDATE_PROPERTIES_OFFSET= 0x28
};

/* ---------- prototypes/SIMULATION_PLAYERS.C */

void simulation_players_collection_clear(byte *collection);
void __attribute__((stdcall)) simulation_players_collection_build(byte *collection);
unsigned long simulation_players_collection_get_active_mask(const byte *collection);
boolean simulation_players_update_remove_player(const byte *update);
boolean simulation_players_update_unknown_000850c0(const byte *update);
boolean __attribute__((stdcall)) simulation_players_update_set_properties(const byte *update);
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
