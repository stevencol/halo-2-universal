/*
SIMULATION_GAMESTATE_ENTITIES.H

the objects of the game and the things that the simulation tells of them:
which is which. Worked out from halo2ship.exe; the name of the file is a
guess, by the one that later games have.
*/

#ifndef __SIMULATION_GAMESTATE_ENTITIES_H
#define __SIMULATION_GAMESTATE_ENTITIES_H

#include "cseries/cseries.h"

/* ---------- constants */

enum
{
	/* of an object: the thing that the simulation tells of it, or NONE */
	SIMULATION_OBJECT_ENTITY_OFFSET= 0xD4,
	/* which of the functions of a kind of thing says that one of them is
	of an object */
	SIMULATION_ENTITY_TYPE_HAS_OBJECT_PROC= 7,
	/* of something that happened in the game, as it goes in a message
	(0x24 bytes): its kind, a number, five numbers that can be NONE (the
	first, second and fourth of them are players), one more number, and a
	short that only the ninth kind has */
	SIMULATION_GAME_EVENT_SIZE= 0x24,
	SIMULATION_GAME_EVENT_NUMBERS= 5,
	_simulation_game_event_kind9= 9
};

/* ---------- prototypes/SIMULATION_GAMESTATE_ENTITIES.C */

struct bitstream;

boolean __attribute__((stdcall)) simulation_game_event_decode(long unused, long *event, struct bitstream *stream);
boolean __attribute__((stdcall)) simulation_game_event_apply(long unused0, long unused1, long unused2,
	const long *event);
long simulation_gamestate_entity_get_object(long entity_index);
long simulation_object_get_created_entity(long object_index);
long simulation_object_get_remote_entity(long object_index);

#endif
