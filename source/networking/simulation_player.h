/*
SIMULATION_PLAYER.H

What the game in the network has of a player of the game: one of the
sixteen that the thing has (simulation_world.h). The name of the file is a
guess, and so are the names of its functions.
*/

#ifndef __SIMULATION_PLAYER_H
#define __SIMULATION_PLAYER_H

#include "cseries/cseries.h"
#include "math/real_math.h"

/* ---------- constants */

enum
{
	/* of one of them: three numbers that it is given; the three numbers that
	the player is known by; the six bytes that its console is known by; the
	thing that it is of; two bytes; when what it does last came; and what it
	does */
	SIMULATION_PLAYER_UNKNOWN0_OFFSET= 0x0,
	SIMULATION_PLAYER_INDEX_OFFSET= 0x4,
	SIMULATION_PLAYER_STATE_OFFSET= 0x8,
	SIMULATION_PLAYER_IDENTIFIER_OFFSET= 0xC,
	SIMULATION_PLAYER_IDENTIFIER_LENGTH= 12,
	SIMULATION_PLAYER_MACHINE_OFFSET= 0x18,
	SIMULATION_PLAYER_MACHINE_LENGTH= 6,
	SIMULATION_PLAYER_WORLD_OFFSET= 0x20,
	SIMULATION_PLAYER_UNKNOWN24_OFFSET= 0x24,
	SIMULATION_PLAYER_UNKNOWN25_OFFSET= 0x25,
	SIMULATION_PLAYER_ACTION_TIME_OFFSET= 0x28,
	SIMULATION_PLAYER_ACTION_OFFSET= 0x2C,
	/* the states of one whose doings come from the view of its console, and
	the one of one that is never given them; and what such a view has for it,
	at 0x5098 of what it has at 0x8, and the size of what that gives */
	_simulation_player_state2= 2,
	_simulation_player_state4= 4,
	_simulation_player_state5= 5,
	SIMULATION_PLAYER_VIEW_UNKNOWN8_UNKNOWN5098_OFFSET= 0x5098,
	SIMULATION_PLAYER_PACKED_ACTION_SIZE= 0x34
};

/* ---------- prototypes/SIMULATION_PLAYER.C */

void __attribute__((stdcall)) simulation_player_new(byte *player, const long *identifier, const word *machine,
	byte *world, long unknown0, long index, long state);
boolean __attribute__((stdcall)) simulation_player_get_action(byte *player, byte *action);

/* ---------- globals */

extern real unknown_004ced44;

#endif
