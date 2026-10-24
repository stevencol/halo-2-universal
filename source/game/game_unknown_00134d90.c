/*
GAME_UNKNOWN_00134D90.C

a part of the game: 0x204 bytes of the state of the game, that are zeros in a new map.
Worked out from halo2ship.exe; what it is for is not known, and the name of
the file is only the address of its first function.
*/

#include "cseries/cseries.h"

#include <string.h>

#include "saved games/game_state.h"
#include "scenario/scenario.h"
#include "cache/cache_files.h"

/* ---------- constants */

enum
{
	GAME_UNKNOWN_00134D90_SIZE= 0x204
};

/* ---------- globals */

/* 0x004e6740: the 0x204 bytes; NULL when the part is closed */
byte *unknown_004e6740;

/* ---------- code */

/* 0x00134d90: the bytes take their memory in the state of the game
(game_state_malloc, which the game has written here) */
void game_unknown_00134d90_initialize(
	void)
{
	unknown_004e6740= game_state_malloc(GAME_UNKNOWN_00134D90_SIZE);
}

/* 0x00134dd0: the part is closed: there are no bytes */
void game_unknown_00134d90_dispose(
	void)
{
	unknown_004e6740= NULL;
}

/* 0x00134de0: for a new map the bytes are zeros, if they are there */
void game_unknown_00134d90_initialize_for_new_map(
	void)
{
	if (unknown_004e6740)
	{
		memset(unknown_004e6740, 0, GAME_UNKNOWN_00134D90_SIZE);
	}
}

/* 0x00135750: the bytes have one thing of 0x20 bytes for each of what the
scenario counts at 0x3C0, and each loses the bit 0 of its byte at 0x20
(after the first number of the bytes, its byte at 0x1C). Nothing if the part
is closed or there is no scenario */
void game_unknown_00134d90_clear_bits(
	void)
{
	byte *unknown= unknown_004e6740;

	if (unknown && global_scenario)
	{
		long index;

		for (index= 0; index<*(const long *)((const byte *)global_scenario+0x3C0); index++)
		{
			unknown[0x20+index*0x20]&= ~1;
		}
	}
}

/* 0x00134d20: the entry (0x20 bytes, after the first long) of the 0x204
bytes for one of the entries (0x18 bytes, of the block at 0x3C0) of the
scenario that starts with a number, and which it is; the number is kept in
the first long. Nothing and NONE when it is not there */
byte *game_unknown_00134d90_find(
	long *index,
	long number)
{
	byte *data= unknown_004e6740;

	if (data && global_scenario && number)
	{
		const long *entries= *(const long *const *)((const byte *)global_scenario+0x3C4);
		long count= *(const long *)((const byte *)global_scenario+0x3C0);
		long which;

		for (which= 0; which<count; which++)
		{
			if (entries[which*6] && entries[which*6]==number)
			{
				*(long *)data= number;
				if (index)
				{
					*index= which;
				}
				return data+which*0x20+4;
			}
		}
	}
	if (index)
	{
		*index= NONE;
	}

	return NULL;
}

/* 0x00137650: a count of a tag: 1 with the bit 0x04 of its byte at 0x04,
else the long at 0x04 of the element (0x0C bytes, at 0x38) that its byte
0x3C+which names plus its long at 0x48; and the byte at 0x25 of the element
(0x5C bytes, at 0x28) of each of so many bytes (as many as at 0x1C; 0xFF
for none) */
long tag_unknown_00137650(
	long tag_index,
	long which,
	const byte *elements)
{
	const byte *tag= (const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address;
	long count;
	long index;

	if (tag[0x04]&4)
	{
		count= 1;
	}
	else
	{
		count= *(const long *)(*(const byte *const *)(tag+0x38)+(signed char)tag[which+0x3C]*0x0C+0x04)
			+ *(const long *)(tag+0x48);
	}
	for (index= 0; index<*(const long *)(tag+0x1C); index++)
	{
		if (elements[index]!=0xFF)
		{
			count+= (*(const byte *const *)(tag+0x28))[elements[index]*0x5C+0x25];
		}
	}

	return count;
}
