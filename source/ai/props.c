/*
PROPS.C

what the characters know of the things around them. Worked out from halo2ship.exe; the name of the file is a guess, by
the names that the game gives to its tables of data ("prop", "prop_ref",
"tracking").
*/

#include "cseries/cseries.h"
#include "memory/data.h"
#include "saved games/game_state.h"

/* ---------- globals */

/* 0x0050241c: the things ("prop": 0x100 of 0xC4 bytes). 0x00502418: who
knows of each ("prop_ref": 0x400 of 0x3C bytes). 0x00502414: "tracking":
0x64 of 0x124 bytes */
struct data_array *prop_data;
struct data_array *prop_ref_data;
struct data_array *tracking_data;

/* ---------- code */

/* 0x0025c170: the three tables are made, in the memory of the state of the
game (data_new, which the game has written here) */
void props_initialize(
	void)
{
	prop_data= data_new("prop", 0x100, 0xC4, 0, game_state_allocation);
	prop_ref_data= data_new("prop_ref", 0x400, 0x3C, 0, game_state_allocation);
	tracking_data= data_new("tracking", 0x64, 0x124, 0, game_state_allocation);
}
