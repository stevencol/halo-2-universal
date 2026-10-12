/*
HS_RUNTIME.C

what runs the scripts of the levels: their threads and their variables.
Worked out from halo2ship.exe; the name of the file is a guess, by the names
that the game gives to its two tables of data ("hs thread", "hs globals").
*/

#include "cseries/cseries.h"
#include "game/game.h"
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

enum
{
	HS_THREAD_SIZE= 0x418,
	/* of a thread: its kind, a byte (those of the kind 4 are not run by the
	tick), and the time of the game that it sleeps until (below zero: for
	ever) */
	HS_THREAD_TYPE_OFFSET= 2,
	HS_THREAD_SLEEP_UNTIL_OFFSET= 8
};

/* 0x00209850: a thread runs until it sleeps or ends */
void unknown_00209850(long thread_index);

/* 0x001deed0: what is done after the threads have run (of the lists of
objects of the scripts, by where it is) */
void unknown_001deed0(void);

/* 0x00209310: each tick of the game, while the scripts run (unknown_004f9388:
a thread can stop them all): each thread that is not of the kind 4 and whose
time to sleep is over runs. Then unknown_001deed0 */
void hs_runtime_update(
	void)
{
	if (unknown_004f9388)
	{
		long time= (long)game_time_globals->time;
		long thread_index;

		for (thread_index= data_next_index(hs_thread_data, NONE); thread_index!=NONE; )
		{
			const byte *thread= (const byte *)hs_thread_data->data+(thread_index&0xFFFF)*HS_THREAD_SIZE;

			if (thread[HS_THREAD_TYPE_OFFSET]!=4 && *(const long *)(thread+HS_THREAD_SLEEP_UNTIL_OFFSET)>=0 &&
				*(const long *)(thread+HS_THREAD_SLEEP_UNTIL_OFFSET)<=time)
			{
				unknown_00209850(thread_index);
			}
			thread_index= data_next_index(hs_thread_data, thread_index);
			if (!unknown_004f9388)
			{
				break;
			}
		}
		unknown_001deed0();
	}
}
