/*
GAME_UNKNOWN_001991F0.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

A part of the game with sixteen entries of 0xB bytes, each known by six
bytes (the address of a network card, most likely), with a byte that says
that it is in use and another that marks one of them. Three of the functions
are given what has the entries, at 0xDC24; one uses the entries at
0x0051e8d4, which are where they would be in something at 0x00510cb0. What
it is for is not known, and the name of the file is only the address of its
first function.
*/

#include "cseries/cseries.h"
#include "networking/transport_security.h"

#include <string.h>

/* ---------- constants */

enum
{
	GAME_UNKNOWN_001991F0_ENTRIES= 16,
	GAME_UNKNOWN_001991F0_ENTRY_SIZE= 0xB,
	/* where the entries are in what three of the functions are given */
	GAME_UNKNOWN_001991F0_ENTRIES_OFFSET= 0xDC24,
	/* in an entry: the six bytes that it is known by, whether it is in use,
	and the byte that marks it */
	GAME_UNKNOWN_001991F0_IDENTIFIER_SIZE= 6,
	GAME_UNKNOWN_001991F0_VALID_OFFSET= 6,
	GAME_UNKNOWN_001991F0_UNKNOWN8_OFFSET= 8,
	GAME_UNKNOWN_001991F0_UNKNOWN9_OFFSET= 9
};

/* ---------- prototypes */

long time(long *timer);

/* ---------- globals */

/* 0x00510cb0 (game_results.c): what has the entries of 0x0051e8d4, at
0xDC24 */
extern boolean unknown_00510cb0;

/* 0x0051e8d4: sixteen entries */
byte unknown_0051e8d4[GAME_UNKNOWN_001991F0_ENTRIES][GAME_UNKNOWN_001991F0_ENTRY_SIZE];

/* 0x00511020, 0x00511024: that 0x00199460 was done, and when */
boolean unknown_00511020;
long unknown_00511024;

/* 0x00511028, 0x0051102c: that 0x00196430 was done, and when */
boolean unknown_00511028;
long unknown_0051102c;

/* ---------- code */

/* 0x001991f0: takes the first entry that is not in use of what is given,
empties it, marks it in use and gives it the six bytes; its place, or NONE
if all are in use */
long game_unknown_001991f0_new(
	byte *data,
	const byte *identifier)
{
	byte *entries= data+GAME_UNKNOWN_001991F0_ENTRIES_OFFSET;
	long index;

	for (index= 0; index<GAME_UNKNOWN_001991F0_ENTRIES; index++)
	{
		byte *entry= entries+index*GAME_UNKNOWN_001991F0_ENTRY_SIZE;

		if (!entry[GAME_UNKNOWN_001991F0_VALID_OFFSET])
		{
			memset(entry, 0, GAME_UNKNOWN_001991F0_ENTRY_SIZE);
			entry[GAME_UNKNOWN_001991F0_VALID_OFFSET]= TRUE;
			memcpy(entry, identifier, GAME_UNKNOWN_001991F0_IDENTIFIER_SIZE);

			return index;
		}
	}

	return NONE;
}

/* 0x00199250: the place of the first entry in use of what is given that is
known by the six bytes; NONE if there is none */
long __attribute__((stdcall)) game_unknown_001991f0_find(
	const byte *data,
	const byte *identifier)
{
	const byte *entries= data+GAME_UNKNOWN_001991F0_ENTRIES_OFFSET;
	long index;

	for (index= 0; index<GAME_UNKNOWN_001991F0_ENTRIES; index++)
	{
		const word *entry= (const word *)(entries+index*GAME_UNKNOWN_001991F0_ENTRY_SIZE);
		const word *other= (const word *)identifier;

		if (((const byte *)entry)[GAME_UNKNOWN_001991F0_VALID_OFFSET] &&
			entry[0]==other[0] && entry[1]==other[1] && entry[2]==other[2])
		{
			return index;
		}
	}

	return NONE;
}

/* 0x00199290: the place of the first entry of what is given that is in use
and marked; NONE if there is none */
long game_unknown_001991f0_find_marked(
	const byte *data)
{
	const byte *entries= data+GAME_UNKNOWN_001991F0_ENTRIES_OFFSET;
	long index;

	for (index= 0; index<GAME_UNKNOWN_001991F0_ENTRIES; index++)
	{
		const byte *entry= entries+index*GAME_UNKNOWN_001991F0_ENTRY_SIZE;

		if (entry[GAME_UNKNOWN_001991F0_VALID_OFFSET] && entry[GAME_UNKNOWN_001991F0_UNKNOWN8_OFFSET])
		{
			return index;
		}
	}

	return NONE;
}

/* 0x001994a0: of the entries of 0x0051e8d4 that are in use, marks the one of
a place and unmarks the others */
void game_unknown_001991f0_mark(
	long marked_index)
{
	long index;

	for (index= 0; index<GAME_UNKNOWN_001991F0_ENTRIES; index++)
	{
		if (unknown_0051e8d4[index][GAME_UNKNOWN_001991F0_VALID_OFFSET])
		{
			unknown_0051e8d4[index][GAME_UNKNOWN_001991F0_UNKNOWN8_OFFSET]= index==marked_index;
		}
	}
}

/* 0x00199560: the four bytes that are given, as a number with its bytes the
other way round; 0 if there are fewer than four */
unsigned long __attribute__((stdcall)) game_unknown_001991f0_read_swapped(
	unsigned long size,
	const unsigned long *value)
{
	unsigned long bytes;

	if (size<4)
	{
		return 0;
	}
	bytes= *value;

	return bytes<<24 | (bytes&0xFF00)<<8 | (bytes>>8&0xFF00) | bytes>>24;
}

/* 0x00196430: it is noted that this was done, and when; if no entry of
0x0051e8d4 is marked, the one of this console (unknown_004cf7cc) is */
void game_unknown_001991f0_mark_own(
	void)
{
	const byte *data= (const byte *)&unknown_00510cb0;
	long index= game_unknown_001991f0_find_marked(data);

	unknown_00511028= TRUE;
	unknown_0051102c= time(NULL);
	if (index==NONE)
	{
		game_unknown_001991f0_mark(game_unknown_001991f0_find(data, unknown_004cf7cc));
	}
}

/* 0x00199460: it is noted that this was done, and when, and the byte of 9 of
the marked entry of 0x0051e8d4 (if one is) is set */
void game_unknown_001991f0_unknown_00199460(
	void)
{
	long index= game_unknown_001991f0_find_marked((const byte *)&unknown_00510cb0);

	unknown_00511020= TRUE;
	unknown_00511024= time(NULL);
	if (index!=NONE)
	{
		unknown_0051e8d4[index][GAME_UNKNOWN_001991F0_UNKNOWN9_OFFSET]= TRUE;
	}
}
