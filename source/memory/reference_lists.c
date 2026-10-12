/*
REFERENCE_LISTS.C

tables of data whose things are in lists: each thing starts with what it is
of and the next of its list. Worked out from halo2ship.exe; the name of the
file is a guess, by the name that the game gives to each table
("<name> reference").
*/

#include "cseries/cseries.h"
#include "memory/data.h"
#include "saved games/game_state.h"

/* ---------- code */

/* 0x00296270: a table of data in the memory of the state of the game, called
"<name> reference", for so many things of 0xC bytes and so many more. Things
of 0x20 bytes are at addresses that are multiples of 0x20 */
struct data_array *__attribute__((stdcall)) reference_list_new(
	const char *name,
	long maximum_count,
	long extra_size)
{
	char buffer[0x100];
	long size;

	csnzprintf(buffer, sizeof(buffer), "%s reference", name);
	size= extra_size+0xC;
	if (size==0x20)
	{
		return data_new(buffer, maximum_count, size, 5, game_state_allocation);
	}

	return data_new(buffer, maximum_count, size, 0, game_state_allocation);
}
