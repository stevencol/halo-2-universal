/*
OBJECT_LISTS.C

the lists of objects that the scripts make. Worked out from halo2ship.exe;
the name of the file is a guess, by the names that the game gives to its two
tables of data ("object list header", "list object").
*/

#include "cseries/cseries.h"
#include "memory/data.h"
#include "saved games/game_state.h"

/* ---------- globals */

/* 0x004f55d8, 0x004f55d4, of hs.c: the lists ("object list header": 0x30 of
0xC bytes) and what is in them ("list object": 0x80) */
extern struct data_array *object_list_header_data;
extern struct data_array *list_object_data;

/* 0x00296270: a table of data of another make, in the memory of the state of
the game: by its name and how many things it has room for */
struct data_array *__attribute__((stdcall)) reference_list_new(const char *name, long maximum_count, long unknown);

/* ---------- code */

/* 0x001ded00: the lists of objects make their two tables, in the memory of
the state of the game: the lists with data_new, which the game has written
here, and what is in them with reference_list_new */
void object_lists_initialize(
	void)
{
	object_list_header_data= data_new("object list header", 0x30, 0xC, 0, game_state_allocation);
	list_object_data= reference_list_new("list object", 0x80, 0);
}
