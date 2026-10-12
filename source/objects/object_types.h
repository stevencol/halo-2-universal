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
	/* six more functions of a type that are given an object and nothing
	else: what each is for is not known */
	OBJECT_TYPE_UNKNOWN20_OFFSET= 0x20,
	OBJECT_TYPE_UNKNOWN24_OFFSET= 0x24,
	OBJECT_TYPE_UNKNOWN38_OFFSET= 0x38,
	OBJECT_TYPE_UNKNOWN3C_OFFSET= 0x3C,
	OBJECT_TYPE_UNKNOWN50_OFFSET= 0x50,
	OBJECT_TYPE_UNKNOWN78_OFFSET= 0x78,
	OBJECT_TYPE_TYPES_OFFSET= 0x84
};

/* ---------- globals */

extern byte *object_type_definitions[NUMBER_OF_OBJECT_TYPES];

/* ---------- prototypes/OBJECT_TYPES.C */

void object_types_call_unknown54(long object_index);
void object_types_call_unknown20(long object_index);
void object_types_call_unknown24(long object_index);
void object_types_call_unknown38(long object_index);
void object_types_call_unknown3C(long object_index);
void object_types_call_unknown50(long object_index);
void object_types_call_unknown78(long object_index);

#endif
