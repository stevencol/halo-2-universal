/*
HS_RUNTIME.C

what runs the scripts of the levels: their threads and their variables.
Worked out from halo2ship.exe; the name of the file is a guess, by the names
that the game gives to its two tables of data ("hs thread", "hs globals").
*/

#include "cseries/cseries.h"
#include "memory/data.h"
#include "saved games/game_state.h"

/* ---------- constants */

enum
{
	/* how many variables the game itself has for the scripts: they are made
	first, each at its own place */
	NUMBER_OF_HS_EXTERNAL_GLOBALS= 0x41D
};

/* ---------- globals */

/* 0x004f9384: the threads ("hs thread": 0x140 of 0x418 bytes). 0x004f9380,
of hs.c: the variables ("hs globals": 0xC00 of 8 bytes) */
struct data_array *hs_thread_data;
extern struct data_array *hs_global_data;

/* 0x0020a1a0: the rest of what is done to start */
void unknown_0020a1a0(void);

/* ---------- code */

/* 0x00208ff0: what runs the scripts makes its two tables, in the memory of
the state of the game (data_new, which the game has written here). With
both, the variables can be used and are empty (data_delete_all), and each of
those that the game itself has is made, at its own place
(datum_new_at_absolute_index). Then the rest (unknown_0020a1a0) */
void hs_runtime_initialize(
	void)
{
	hs_thread_data= data_new("hs thread", 0x140, 0x418, 0, game_state_allocation);
	hs_global_data= data_new("hs globals", 0xC00, 8, 0, game_state_allocation);
	if (hs_thread_data && hs_global_data)
	{
		long index;

		hs_global_data->valid= TRUE;
		data_delete_all(hs_global_data);
		for (index= 0; index<NUMBER_OF_HS_EXTERNAL_GLOBALS; index++)
		{
			datum_new_at_absolute_index(hs_global_data, index);
		}
	}
	unknown_0020a1a0();
}

enum
{
	/* the globals of the scripts below this one are the game's own; those
	from it on are of the scripts of the map */
	HS_FIRST_MAP_GLOBAL_INDEX= 0x41D
};

/* 0x004f9380, of hs.c: the globals of the scripts */
extern struct data_array *hs_global_data;

/* 0x004f9388: cleared when the map goes (whether the scripts of the map run,
perhaps) */
boolean unknown_004f9388;

/* 0x00209290: when a map goes, the threads cannot be used, the globals of
the scripts of the map are deleted (the game has the walk over the table
written here), and the byte is cleared */
void hs_runtime_dispose_from_old_map(
	void)
{
	long absolute_index= NONE;

	hs_thread_data->valid= FALSE;
	for (;;)
	{
		long global_index;

		absolute_index= data_next_absolute_index(hs_global_data, absolute_index+1);
		if (absolute_index==NONE)
		{
			break;
		}
		global_index= (*(const short *)((const byte *)hs_global_data->data+absolute_index*hs_global_data->size)<<16)|
			absolute_index;
		if ((global_index&0xFFFF)>=HS_FIRST_MAP_GLOBAL_INDEX)
		{
			datum_delete(hs_global_data, global_index);
		}
	}
	unknown_004f9388= FALSE;
}
