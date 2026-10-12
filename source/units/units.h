/*
UNITS.H

the bipeds and the vehicles: what they carry. Worked out from halo2ship.exe;
the name of the file is a guess, by the one that the first game has.
*/

#ifndef __UNITS_H
#define __UNITS_H

#include "cseries/cseries.h"

/* ---------- constants */

enum
{
	/* of a unit: the weapons that it carries, each an object */
	UNIT_WEAPONS_OFFSET= 0x218,
	/* another unit, or NONE: they make a chain */
	UNIT_UNKNOWN24C_OFFSET= 0x24C
};

/* ---------- prototypes/UNITS.C */

long unit_get_weapon(long unit_index, short weapon_index);
long unit_get_ultimate_unknown24C(long unit_index);

#endif
