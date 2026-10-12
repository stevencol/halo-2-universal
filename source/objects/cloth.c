/*
CLOTH.C

the cloths: one of the three kinds of thing that hang from an object. Worked out from halo2ship.exe; the name of the file is a guess, by
the name that the game gives to its table of data ("cloth").
*/

#include "cseries/cseries.h"
#include "memory/data.h"
#include "saved games/game_state.h"

/* ---------- globals */

/* 0x004e0338: the table ("cloth": 0x8 of 0x1258 bytes) */
struct data_array *cloth_data;

/* ---------- code */

/* 0x00116a10: the table is made, in the memory of the state of the game
(data_new, which the game has written here) */
void cloth_initialize(
	void)
{
	cloth_data= data_new("cloth", 0x8, 0x1258, 0, game_state_allocation);
}

/* 0x00116a50: for a new map, the table can be used, and is empty
(data_delete_all) */
void cloth_initialize_for_new_map(
	void)
{
	struct data_array *data= cloth_data;

	data->valid= TRUE;
	data_delete_all(data);
}

/* 0x00116a70: when a map goes, the table cannot be used */
void cloth_dispose_from_old_map(
	void)
{
	cloth_data->valid= FALSE;
}
