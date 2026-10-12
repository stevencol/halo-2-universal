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
