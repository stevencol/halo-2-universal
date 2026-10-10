/*
SIMULATION_PLAYER.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

What the game in the network has of a player of the game. The name of the
file is a guess, and so are the names of its functions.
*/

#include "simulation_player.h"

#include "game/player_action.h"
#include "networking/simulation_world.h"

#include <string.h>

/* ---------- code */

/* 0x00084750: what the thing has of a player is new: its three numbers, the
numbers that the player is known by, the bytes of its console, and the
thing. Its first byte is clear; its second is set from the start in a thing
that follows another console (its third state or its fifth), which does not
wait for anyone to know of the player. What it does is nothing
(player_action_clear), and has not come */
void __attribute__((stdcall)) simulation_player_new(
	byte *player,
	const long *identifier,
	const word *machine,
	byte *world,
	long unknown0,
	long index,
	long state)
{
	long world_state;

	*(long *)(player+SIMULATION_PLAYER_UNKNOWN0_OFFSET)= unknown0;
	*(long *)(player+SIMULATION_PLAYER_INDEX_OFFSET)= index;
	*(long *)(player+SIMULATION_PLAYER_STATE_OFFSET)= state;
	memcpy(player+SIMULATION_PLAYER_IDENTIFIER_OFFSET, identifier, SIMULATION_PLAYER_IDENTIFIER_LENGTH);
	memcpy(player+SIMULATION_PLAYER_MACHINE_OFFSET, machine, SIMULATION_PLAYER_MACHINE_LENGTH);
	player[SIMULATION_PLAYER_UNKNOWN24_OFFSET]= FALSE;
	*(byte **)(player+SIMULATION_PLAYER_WORLD_OFFSET)= world;
	world_state= *(const long *)(world+SIMULATION_WORLD_STATE_OFFSET);
	player[SIMULATION_PLAYER_UNKNOWN25_OFFSET]= world_state==3 || world_state==5;
	*(long *)(player+SIMULATION_PLAYER_ACTION_TIME_OFFSET)= NONE;
	player_action_clear(player+SIMULATION_PLAYER_ACTION_OFFSET);
}
