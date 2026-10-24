/*
LIQUID.C

the liquids: one of the three kinds of thing that hang from an object. Worked out from halo2ship.exe; the name of the file is a guess, by
the name that the game gives to its table of data ("liquid").
*/

#include "cseries/cseries.h"
#include "memory/data.h"
#include "saved games/game_state.h"

/* ---------- globals */

/* 0x004e031c: the table ("liquid": 0x40 of 0x110 bytes) */
struct data_array *liquid_data;

/* ---------- code */

/* 0x000c3a40: the table is made, in the memory of the state of the game
(data_new, which the game has written here) */
void liquid_initialize(
	void)
{
	liquid_data= data_new("liquid", 0x40, 0x110, 0, game_state_allocation);
}

/* 0x000c3a80: for a new map, the table can be used, and is empty
(data_delete_all) */
void liquid_initialize_for_new_map(
	void)
{
	struct data_array *data= liquid_data;

	data->valid= TRUE;
	data_delete_all(data);
}

/* 0x000c3aa0: when a map goes, the table cannot be used */
void liquid_dispose_from_old_map(
	void)
{
	liquid_data->valid= FALSE;
}

/* 0x000c4210: the which-th part (0x50 bytes, after 0x20) of a liquid
(0x110 bytes); nothing without the table, when it is not on (0x29), or for
NONE */
byte *liquid_unknown_000c4210(
	long liquid_index,
	long which)
{
	if (!liquid_data || !*((const byte *)liquid_data+0x29) || liquid_index==NONE)
	{
		return NULL;
	}

	return (byte *)liquid_data->data+(liquid_index&0xFFFF)*0x110+0x20+which*0x50;
}

/* 0x000c4250: the long at 0x0C of a liquid; NONE as before */
long liquid_unknown_000c4250(
	long liquid_index)
{
	if (!liquid_data || !*((const byte *)liquid_data+0x29) || liquid_index==NONE)
	{
		return NONE;
	}

	return *(const long *)((const byte *)liquid_data->data+(liquid_index&0xFFFF)*0x110+0x0C);
}
