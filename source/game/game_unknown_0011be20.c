/*
GAME_UNKNOWN_0011BE20.C

a part of the game: it keeps 0xD0C bytes that are not of the state of the game, the
first of which says that a map is loaded.
Worked out from halo2ship.exe; what it is for is not known, and the name of
the file is only the address of its first function.
*/

#include "cseries/cseries.h"

#include <string.h>

/* ---------- globals */

/* 0x004f93b8: what it keeps */
byte unknown_004f93b8[0xD0C];

/* 0x00211cc0: what more is done for a new map */
void unknown_00211cc0(void);

/* ---------- code */

/* 0x0011be20: for a new map, what it keeps is nothing, a map is loaded, and
unknown_00211cc0 is done */
void game_unknown_0011be20_initialize_for_new_map(
	void)
{
	memset(unknown_004f93b8, 0, sizeof(unknown_004f93b8));
	unknown_004f93b8[0]= TRUE;
	unknown_00211cc0();
}

/* 0x0011be40: no map is loaded */
void game_unknown_0011be20_dispose_from_old_map(
	void)
{
	unknown_004f93b8[0]= FALSE;
}
