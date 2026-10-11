/*
MAIN_MAP_PRECACHE.H

The map that is copied to the hard disk ahead of time: the one that the game
will most likely load next. Worked out from halo2ship.exe; the name of the
file is a guess, and so are the names of its functions.
*/

#ifndef __MAIN_MAP_PRECACHE_H
#define __MAIN_MAP_PRECACHE_H

#include "cseries/cseries.h"

/* ---------- structures */

/* a map to copy: its name, how much it matters, and whether it is only
asked for if nothing is known of it yet */
struct main_map_precache_request
{
	const char *name;
	long priority;
	boolean only_if_unknown;
};

/* ---------- prototypes/MAIN_MAP_PRECACHE.C */

void main_map_precache_update(void);
boolean main_map_precache_copy_idle(void);
boolean main_map_precache_next_map_not_ready(void);

#endif
