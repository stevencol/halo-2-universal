/*
ANTENNA.C

the antennas: one of the three kinds of thing that hang from an object. Worked out from halo2ship.exe; the name of the file is a guess, by
the name that the game gives to its table of data ("antenna").
*/

#include "cseries/cseries.h"
#include "memory/data.h"
#include "saved games/game_state.h"

/* ---------- globals */

/* 0x004e0334: the table ("antenna": 0xC of 0x2BC bytes) */
struct data_array *antenna_data;

/* ---------- code */

/* 0x00115d10: the table is made, in the memory of the state of the game
(data_new, which the game has written here) */
void antenna_initialize(
	void)
{
	antenna_data= data_new("antenna", 0xC, 0x2BC, 0, game_state_allocation);
}

/* 0x00115d50: for a new map, the table can be used, and is empty
(data_delete_all) */
void antenna_initialize_for_new_map(
	void)
{
	struct data_array *data= antenna_data;

	data->valid= TRUE;
	data_delete_all(data);
}

/* 0x00115d70: when a map goes, the table cannot be used */
void antenna_dispose_from_old_map(
	void)
{
	antenna_data->valid= FALSE;
}

/* 0x00115d80: at the end, the table is no more (the name is a guess) */
void antenna_dispose(
	void)
{
	if (antenna_data)
	{
		antenna_data= NULL;
	}
}
