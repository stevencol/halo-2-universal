/*
OBJECT_UNKNOWN_0010A980.H

a part of the game that keeps, in the state of the game, thirty-two things of 0x14 bytes, thirty-two numbers that are NONE for a new
map, and a byte that says that a map is loaded.
Worked out from halo2ship.exe; what it is for is not known, and the name of
the file is only the address of its first function.
*/

#ifndef __OBJECT_UNKNOWN_0010A980_H
#define __OBJECT_UNKNOWN_0010A980_H

#include "cseries/cseries.h"

/* ---------- constants */

enum
{
	OBJECT_UNKNOWN_0010A980_COUNT= 32
};

/* ---------- structures */

struct object_unknown_0010a980_globals
{
	byte unknown0[OBJECT_UNKNOWN_0010A980_COUNT*0x14];
	long unknown280[OBJECT_UNKNOWN_0010A980_COUNT];
	boolean map_initialized;
	byte pad[3];
};

typedef char verify_object_unknown_0010a980_globals_size[
	sizeof(struct object_unknown_0010a980_globals) == 0x304 ? 1 : -1];

/* ---------- globals */

extern struct object_unknown_0010a980_globals *object_unknown_0010a980_globals;

/* ---------- prototypes/OBJECT_UNKNOWN_0010A980.C */

void object_unknown_0010a980_initialize(void);
void object_unknown_0010a980_dispose(void);
void object_unknown_0010a980_initialize_for_new_map(void);
void object_unknown_0010a980_dispose_from_old_map(void);

#endif
