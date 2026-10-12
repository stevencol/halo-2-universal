/*
OBJECT_TYPES.H

what each type of object does. Worked out from halo2ship.exe; the name of the
file is a guess, by the one that the first game has.
*/

#ifndef __OBJECT_TYPES_H
#define __OBJECT_TYPES_H

#include "cseries/cseries.h"

/* ---------- constants */

enum
{
	/* how many types of object the table is declared for: a guess */
	NUMBER_OF_OBJECT_TYPES= 13,
	/* of what a type of object is: the function for an object that lets go
	of the thing that the simulation tells of it, and the types that the
	type is of, from the first to itself, with nothing after the last */
	OBJECT_TYPE_UNKNOWN54_OFFSET= 0x54,
	OBJECT_TYPE_TYPES_OFFSET= 0x84
};

/* ---------- globals */

extern byte *object_type_definitions[NUMBER_OF_OBJECT_TYPES];

/* ---------- prototypes/OBJECT_TYPES.C */

void object_types_call_unknown54(long object_index);

#endif
