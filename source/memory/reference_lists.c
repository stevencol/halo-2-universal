/*
REFERENCE_LISTS.C

tables of data whose things are in lists: each thing starts with what it is
of and the next of its list. Worked out from halo2ship.exe; the name of the
file is a guess, by the name that the game gives to each table
("<name> reference").
*/

#include "cseries/cseries.h"

#include <string.h>

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

/* 0x002962e0: in a list of a table of references, the one of a thing gets
what is kept after it: so many bytes. Nothing is done if the thing is not in
the list */
void reference_list_set_extra(
	long *first_reference_index,
	long index,
	struct data_array *references,
	long extra_size,
	const void *extra)
{
	long reference_index= *first_reference_index;

	while (reference_index!=NONE)
	{
		byte *reference= (byte *)references->data+(reference_index&0xFFFF)*references->size;

		if (*(long *)(reference+4)==index)
		{
			memcpy(reference+0xC, extra, (unsigned long)extra_size);
			break;
		}
		reference_index= *(long *)(reference+8);
	}
}
