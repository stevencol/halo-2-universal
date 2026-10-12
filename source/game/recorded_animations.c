/*
RECORDED_ANIMATIONS.C

the animations that were recorded for the scripts to play. Worked out from halo2ship.exe; the name of the file is a guess, by
the name that the game gives to its table of data ("recorded animations").
*/

#include "cseries/cseries.h"
#include "memory/data.h"
#include "saved games/game_state.h"

/* ---------- globals */

/* 0x004f5724: the table ("recorded animations": 0x1 of 0xA0 bytes) */
struct data_array *recorded_animation_data;

/* ---------- code */

/* 0x001fb2f0: the table is made, in the memory of the state of the game
(data_new, which the game has written here) */
void recorded_animations_initialize(
	void)
{
	recorded_animation_data= data_new("recorded animations", 0x1, 0xA0, 0, game_state_allocation);
}

/* 0x001fb330: for a new map, the table can be used, and is empty
(data_delete_all) */
void recorded_animations_initialize_for_new_map(
	void)
{
	struct data_array *data= recorded_animation_data;

	data->valid= TRUE;
	data_delete_all(data);
}

/* 0x001fb350: when a map goes, the table cannot be used */
void recorded_animations_dispose_from_old_map(
	void)
{
	recorded_animation_data->valid= FALSE;
}
