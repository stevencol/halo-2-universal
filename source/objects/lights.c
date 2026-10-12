/*
LIGHTS.C

the lights of the world. Worked out from halo2ship.exe; the name of the file
is a guess, by the name that the game gives to its table of data ("lights").
*/

#include "cseries/cseries.h"

#include <string.h>

#include "memory/data.h"
#include "saved games/game_state.h"

/* ---------- constants */

enum
{
	MAXIMUM_LIGHTS= 0x15E,
	LIGHT_SIZE= 0x110,

	/* of what light_globals points to */
	LIGHT_GLOBALS_SIZE= 0xC,

	/* of the first thing of unknown_004e0310: 0x200 numbers */
	LIGHT_UNKNOWN_SIZE= 0x800
};

/* ---------- structures */

/* what unknown_001cabc0 fills for the lights, with the name "light": 0x800
bytes of the state of the game and two tables of data. It may be where the
lights are in the world, cluster by cluster, but that is a guess */
struct light_unknown_004e0310
{
	long *unknown0;
	struct data_array *unknown4;
	struct data_array *unknown8;
};

/* ---------- globals */

/* 0x004e030c: the lights ("lights": 0x15E of 0x110 bytes). 0x005107e8: 0xC
bytes of the state of the game (the name is a guess): a byte, a number at 4
and a byte at 8 that is set when the lights start. 0x004e0310: see the
structure */
struct data_array *light_data;
byte *light_globals;
struct light_unknown_004e0310 unknown_004e0310;

/* ---------- prototypes */

void __attribute__((stdcall)) unknown_001cabc0(struct light_unknown_004e0310 *unknown, const char *name,
	long unknown3);

/* ---------- code */

/* 0x000bffa0: the table is made, in the memory of the state of the game
(data_new, which the game has written here), and the 0xC bytes are taken
there too (game_state_malloc, written here as well). If there is a table,
unknown_001cabc0 fills the three things */
void lights_initialize(
	void)
{
	light_data= data_new("lights", MAXIMUM_LIGHTS, LIGHT_SIZE, 0, game_state_allocation);
	light_globals= game_state_malloc(LIGHT_GLOBALS_SIZE);
	light_globals[8]= TRUE;
	if (light_data)
	{
		unknown_001cabc0(&unknown_004e0310, "light", 0);
	}
}

/* 0x000c0040: for a new map the table is good to use, with nothing in it,
and so are the two of the three things, whose 0x200 numbers are all NONE */
void lights_initialize_for_new_map(
	void)
{
	data_make_valid(light_data);
	light_globals[8]= TRUE;
	memset(unknown_004e0310.unknown0, 0xFF, LIGHT_UNKNOWN_SIZE);
	data_make_valid(unknown_004e0310.unknown8);
	data_make_valid(unknown_004e0310.unknown4);
	light_globals[0]= FALSE;
	*(long *)(light_globals+4)= 0;
}
