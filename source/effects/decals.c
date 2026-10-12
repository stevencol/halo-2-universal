/*
DECALS.C

the marks that are left on the level. Worked out from halo2ship.exe; the name of the file is a guess, by
the name that the game gives to its table of data ("decals").
*/

#include "cseries/cseries.h"
#include "memory/data.h"

#include <string.h>

/* ---------- constants */

enum
{
	/* of what decal_globals points to, 0x3C2C bytes of the state of the
	game: 0xE00 numbers that are NONE for a new map, and three more: NONE
	and nothing twice */
	DECAL_GLOBALS_SIZE= 0x3C2C,
	DECAL_GLOBALS_UNKNOWN0_COUNT= 0xE00,
	DECAL_GLOBALS_UNKNOWN3800_OFFSET= 0x3800,
	DECAL_GLOBALS_UNKNOWN3804_OFFSET= 0x3804,
	DECAL_GLOBALS_UNKNOWN3808_OFFSET= 0x3808
};

/* ---------- globals */

/* 0x004ea950: the marks ("decals": 0x400 of 0x40 bytes). 0x004ea94c: what
else is kept of them */
struct data_array *decal_data;
byte *decal_globals;

/* ---------- code */

/* 0x0017d2e0: for a new map, what is kept of the marks is nothing, with
NONE for its 0xE00 numbers and for the one after them; and the table can be
used, and is empty (data_delete_all) */
void decals_initialize_for_new_map(
	void)
{
	byte *globals= decal_globals;
	struct data_array *data= decal_data;

	memset(globals, 0, DECAL_GLOBALS_SIZE);
	memset(globals, 0xFF, DECAL_GLOBALS_UNKNOWN0_COUNT*sizeof(long));
	*(long *)(globals+DECAL_GLOBALS_UNKNOWN3800_OFFSET)= NONE;
	*(long *)(globals+DECAL_GLOBALS_UNKNOWN3804_OFFSET)= 0;
	*(long *)(globals+DECAL_GLOBALS_UNKNOWN3808_OFFSET)= 0;
	data->valid= TRUE;
	data_delete_all(data);
}
