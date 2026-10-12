/*
SQUADS.C

the squads of the characters, and the groups of squads. Worked out from halo2ship.exe; the name of the file is a guess, by
the names that the game gives to its tables of data ("squad", "squad group").
*/

#include "cseries/cseries.h"
#include "memory/data.h"
#include "saved games/game_state.h"

/* ---------- globals */

/* 0x0051e9d8: the squads ("squad": 0x14F of 0x98 bytes). 0x0051e9dc: their
groups ("squad group": 0x64 of 0x38 bytes). 0x00557c74: five numbers that are
NONE when the squads start */
struct data_array *squad_data;
struct data_array *squad_group_data;
long unknown_00557c74[5];

/* ---------- code */

/* 0x00200930: the five numbers are NONE, and the two tables are made, in the
memory of the state of the game (data_new, which the game has written here) */
void squads_initialize(
	void)
{
	long index;

	for (index= 0; index<5; index++)
	{
		unknown_00557c74[index]= NONE;
	}
	squad_data= data_new("squad", 0x14F, 0x98, 0, game_state_allocation);
	squad_group_data= data_new("squad group", 0x64, 0x38, 0, game_state_allocation);
}
