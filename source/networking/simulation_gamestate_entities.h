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
	SIMULATION_ENTITY_TYPE_HAS_OBJECT_PROC= 7
};

/* ---------- prototypes/SIMULATION_GAMESTATE_ENTITIES.C */

long simulation_gamestate_entity_get_object(long entity_index);
long simulation_object_get_created_entity(long object_index);
long simulation_object_get_remote_entity(long object_index);

#endif
