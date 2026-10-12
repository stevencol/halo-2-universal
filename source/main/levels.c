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
