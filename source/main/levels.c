/*
LEVELS.C

the levels that there are to play. Worked out from halo2ship.exe; the name of
the file is a guess, by the names that the game gives to its two tables of
data ("level handles", "multiplayer level handles").
*/

#include "cseries/cseries.h"
#include "memory/data.h"

/* ---------- globals */

/* 0x004ee4e8: the levels of the campaign ("level handles": 0x14 of 8
bytes). 0x004ee4e4: those of the games of several ("multiplayer level
handles": 0x32 of 8 bytes) */
struct data_array *multiplayer_level_data;
struct data_array *campaign_level_data;

/* ---------- code */

/* 0x0019c1d0: when a map goes, neither table can be used. It is the fourth
of the functions of a part of the game (game.c) */
void levels_dispose_from_old_map(
	void)
{
	campaign_level_data->valid= FALSE;
	multiplayer_level_data->valid= FALSE;
}

/* 0x0019c120: the levels that there are are looked for. 0x0019c6d0,
0x0019c7e0: each table is filled */
void unknown_0019c120(void);
void __attribute__((stdcall)) unknown_0019c6d0(struct data_array *data, long unknown);
void __attribute__((stdcall)) unknown_0019c7e0(struct data_array *data);

/* 0x0019c190: for a new map, the two tables can be used and are empty
(data_delete_all); then the levels are looked for and each table is filled */
void levels_initialize_for_new_map(
	void)
{
	campaign_level_data->valid= TRUE;
	data_delete_all(campaign_level_data);
	multiplayer_level_data->valid= TRUE;
	data_delete_all(multiplayer_level_data);
	unknown_0019c120();
	unknown_0019c6d0(campaign_level_data, 1);
	unknown_0019c7e0(multiplayer_level_data);
}

/* 0x0019bcd0: the levels make their two tables, in memory of the heap
(data_new, which the game has written here) */
void levels_initialize(
	void)
{
	campaign_level_data= data_new("level handles", 0x14, 8, 0, heap_allocation);
	multiplayer_level_data= data_new("multiplayer level handles", 0x32, 8, 0, heap_allocation);
}
