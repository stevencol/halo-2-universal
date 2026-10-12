/*
CONTRAILS.C

the trails that some things leave. Worked out from halo2ship.exe; the name of the file is a guess, by
the name that the game gives to its table of data ("contrail", "contrail point").
*/

#include "cseries/cseries.h"
#include "memory/data.h"
#include "saved games/game_state.h"

/* ---------- globals */

/* 0x004ea944: the trails ("contrail": 0x100 of 0x48 bytes). 0x004ea940:
their points ("contrail point": 0x400 of 0x38 bytes) */
struct data_array *contrail_data;
struct data_array *contrail_point_data;

/* ---------- code */

/* 0x0017b850: the trails have no tables */
void contrails_dispose(
	void)
{
	if (contrail_point_data)
	{
		contrail_point_data= NULL;
	}
	if (contrail_data)
	{
		contrail_data= NULL;
	}
}

/* 0x0017b870: for a new map, the two tables can be used, and are empty
(data_delete_all) */
void contrails_initialize_for_new_map(
	void)
{
	contrail_data->valid= TRUE;
	data_delete_all(contrail_data);
	contrail_point_data->valid= TRUE;
	data_delete_all(contrail_point_data);
}

/* 0x0017b8a0: when a map goes, neither can be used */
void contrails_dispose_from_old_map(
	void)
{
	contrail_point_data->valid= FALSE;
	contrail_data->valid= FALSE;
}

/* 0x0017b7b0: the trails make their two tables, in the memory of the state
of the game (data_new, which the game has written here): 0x100 trails of
0x48 bytes and 0x400 points of 0x38. If there is memory for only one of
them, there is neither */
void contrails_initialize(
	void)
{
	struct data_array *points;

	contrail_data= data_new("contrail", 0x100, 0x48, 0, game_state_allocation);
	points= data_new("contrail point", 0x400, 0x38, 0, game_state_allocation);
	contrail_point_data= points;
	if (contrail_data)
	{
		if (!points)
		{
			contrail_data= NULL;
		}
	}
	else if (points)
	{
		contrail_point_data= NULL;
	}
}
