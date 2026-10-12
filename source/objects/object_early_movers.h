/*
OBJECT_EARLY_MOVERS.H

the objects that move before the others. Worked out from halo2ship.exe; the
name of the file is a guess, by the one that later games have.
*/

#ifndef __OBJECT_EARLY_MOVERS_H
#define __OBJECT_EARLY_MOVERS_H

#include "cseries/cseries.h"

/* ---------- constants */

enum
{
	MAXIMUM_OBJECT_EARLY_MOVERS= 32,
	/* of an object: its bits, and the one that says that it moves before
	the others. Of what an object is: a byte whose seventh bit makes it one */
	OBJECT_EARLY_MOVER_OBJECT_FLAGS_OFFSET= 0xC0,
	_object_early_mover_object_bit= 0x8,
	OBJECT_EARLY_MOVER_DEFINITION_FLAGS_OFFSET= 0x2,
	_object_early_mover_definition_bit= 0x40
};

/* ---------- structures */

struct object_early_movers_globals
{
	byte unknown0[0x1D00];
	long object_indices[MAXIMUM_OBJECT_EARLY_MOVERS];
	long count;
	/* a map is loaded */
	boolean map_initialized;
	byte pad1D85[3];
};

typedef char verify_object_early_movers_globals_size[
	sizeof(struct object_early_movers_globals) == 0x1D88 ? 1 : -1];

/* ---------- globals */

extern struct object_early_movers_globals *object_early_movers_globals;

/* ---------- prototypes/OBJECT_EARLY_MOVERS.C */

void object_early_movers_initialize(void);
void object_early_movers_dispose(void);
void object_early_movers_initialize_for_new_map(void);
void object_early_movers_dispose_from_old_map(void);
void object_early_movers_add(long object_index);

#endif
