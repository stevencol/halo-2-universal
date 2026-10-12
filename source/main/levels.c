/*
LEVELS.C

the levels that there are to play. Worked out from halo2ship.exe; the name of
the file is a guess, by the names that the game gives to its two tables of
data ("level handles", "multiplayer level handles").
*/

#include "cseries/cseries.h"
#include "memory/data.h"

/* ---------- globals */

/* 0x004ee4e4, 0x004ee4e8: the two tables; which is which is not known */
struct data_array *unknown_004ee4e4;
struct data_array *unknown_004ee4e8;

/* ---------- code */

/* 0x0019c1d0: when a map goes, neither table can be used. It is the fourth
of the functions of a part of the game (game.c) */
void levels_dispose_from_old_map(
	void)
{
	unknown_004ee4e8->valid= FALSE;
	unknown_004ee4e4->valid= FALSE;
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
	unknown_004ee4e8->valid= TRUE;
	data_delete_all(unknown_004ee4e8);
	unknown_004ee4e4->valid= TRUE;
	data_delete_all(unknown_004ee4e4);
	unknown_0019c120();
	unknown_0019c6d0(unknown_004ee4e8, 1);
	unknown_0019c7e0(unknown_004ee4e4);
}
