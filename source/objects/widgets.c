/*
WIDGETS.C

what hangs from the objects and moves by itself: antennas, cloths and liquids.
Worked out from halo2ship.exe; the name of the file is a guess, by the name
that the game gives to its table of data ("widget").
*/

#include "cseries/cseries.h"
#include "memory/data.h"
#include "objects/widgets.h"
#include "saved games/game_state.h"

/* ---------- globals */

/* 0x004e0320: the widgets ("widget": 0x40 of 0xC bytes) */
struct data_array *widget_data;

/* 0x00467498: the kinds, which in the game are the antennas ('ant!'), the
cloths ('clwd') and the liquids ('tdtl'), with the functions of antenna.c,
cloth.c and liquid.c */
struct widget_type_definition widget_type_definitions[NUMBER_OF_WIDGET_TYPES];

/* ---------- code */

/* 0x000d4830: the table is made, in the memory of the state of the game
(data_new, which the game has written here), and each kind starts */
void widgets_initialize(
	void)
{
	long index;

	widget_data= data_new("widget", 0x40, 0xC, 0, game_state_allocation);
	for (index= 0; index<NUMBER_OF_WIDGET_TYPES; index++)
	{
		if (widget_type_definitions[index].initialize)
		{
			widget_type_definitions[index].initialize();
		}
	}
}

/* 0x000d4890: for a new map the table is good to use, with nothing in it,
and each kind is told */
void widgets_initialize_for_new_map(
	void)
{
	long index;

	data_make_valid(widget_data);
	for (index= 0; index<NUMBER_OF_WIDGET_TYPES; index++)
	{
		if (widget_type_definitions[index].initialize_for_new_map)
		{
			widget_type_definitions[index].initialize_for_new_map();
		}
	}
}

/* 0x000d48d0: when the map goes each kind is told, and the table cannot be
used */
void widgets_dispose_from_old_map(
	void)
{
	long index;

	for (index= 0; index<NUMBER_OF_WIDGET_TYPES; index++)
	{
		if (widget_type_definitions[index].dispose_from_old_map)
		{
			widget_type_definitions[index].dispose_from_old_map();
		}
	}
	widget_data->valid= FALSE;
}

/* 0x000d4db0: each tick, each kind that has a function for it is told how
long the tick is */
void __attribute__((stdcall)) widgets_update(
	float seconds)
{
	long index;

	for (index= 0; index<NUMBER_OF_WIDGET_TYPES; index++)
	{
		if (widget_type_definitions[index].update)
		{
			widget_type_definitions[index].update(seconds);
		}
	}
}
