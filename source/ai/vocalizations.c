/*
VOCALIZATIONS.C

the records of what the characters say. Worked out from halo2ship.exe; the
name of the file is a guess, by the name that the game gives to its table of
data ("vocalization records").
*/

#include "cseries/cseries.h"

#include <string.h>

#include "memory/data.h"
#include "saved games/game_state.h"
#include "objects/objects.h"

/* ---------- constants */

enum
{
	/* of what unknown_004f939c points to: two things of 0x7DC bytes, that
	end with a number, a short and another number; the numbers are NONE in a
	new map */
	VOCALIZATION_UNKNOWN_COUNT= 2,
	VOCALIZATION_UNKNOWN_SIZE= 0x7DC,
	VOCALIZATION_UNKNOWN_UNKNOWN7D0_OFFSET= 0x7D0,
	VOCALIZATION_UNKNOWN_UNKNOWN7D4_OFFSET= 0x7D4,
	VOCALIZATION_UNKNOWN_UNKNOWN7D8_OFFSET= 0x7D8
};

/* ---------- globals */

/* 0x004f9398: the records ("vocalization records": 0xF of 0x54 bytes).
0x004f939c: the two things */
struct data_array *vocalization_record_data;
byte *unknown_004f939c;

/* ---------- code */

/* 0x0020b930: the two things take their memory in the state of the game
(game_state_malloc, which the game has written here), and the table is made
there too (data_new, written here as well) */
void vocalizations_initialize(
	void)
{
	unknown_004f939c= game_state_malloc(VOCALIZATION_UNKNOWN_COUNT*VOCALIZATION_UNKNOWN_SIZE);
	vocalization_record_data= data_new("vocalization records", 0xF, 0x54, 0, game_state_allocation);
}

/* 0x0020b9b0: the two things are all zeros, but for their two numbers, which
are NONE; and the table is good to use, with nothing in it */
void vocalizations_initialize_for_new_map(
	void)
{
	byte *unknown;
	long index;

	memset(unknown_004f939c, 0, VOCALIZATION_UNKNOWN_COUNT*VOCALIZATION_UNKNOWN_SIZE);
	data_make_valid(vocalization_record_data);
	unknown= unknown_004f939c;
	for (index= 0; index<VOCALIZATION_UNKNOWN_COUNT; index++)
	{
		*(long *)(unknown+VOCALIZATION_UNKNOWN_UNKNOWN7D0_OFFSET)= NONE;
		*(short *)(unknown+VOCALIZATION_UNKNOWN_UNKNOWN7D4_OFFSET)= 0;
		*(long *)(unknown+VOCALIZATION_UNKNOWN_UNKNOWN7D8_OFFSET)= NONE;
		unknown+= VOCALIZATION_UNKNOWN_SIZE;
	}
}

/* 0x0020fe20: of the vocalization records (0x54 bytes) of something (from
0x7D0, each to the next at 0x50; how many at 0x7D4), those with the byte at
0x48 are deleted, and their unit (0x04) forgets them (at 0x1C past its
short at 0x342) */
void __attribute__((stdcall)) vocalization_records_unknown_0020fe20(
	byte *owner)
{
	long *link= (long *)(owner+0x7D0);

	while (*link!=NONE)
	{
		long index= *link;
		byte *record= (byte *)vocalization_record_data->data+(index&0xFFFF)*0x54;

		if (record[0x48])
		{
			long next= *(const long *)(record+0x50);
			long unit_index= *(const long *)(record+0x04);

			if (unit_index!=NONE)
			{
				byte *unit= (byte *)((const struct object_header_datum *)object_header_data->data)[unit_index&0xFFFF].datum;
				byte *place= unit+*(const short *)(unit+0x342);

				if (*(const long *)(place+0x1C)==index)
				{
					*(long *)(place+0x1C)= NONE;
				}
			}
			datum_delete(vocalization_record_data, index);
			*link= next;
			(*(short *)(owner+0x7D4))--;
		}
		else
		{
			link= (long *)(record+0x50);
		}
	}
}
