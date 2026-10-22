/*
ONLINE_STATISTICS.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

Numbers that the network game keeps of each of its players, in five places
of 0x99C bytes, each with a record of 0x98 bytes per player: a state, a time
and 0x13 numbers, each with a byte that says that it is there. What they
count is not known, and the name of the file is a guess, and so are the
names of its functions.
*/

#include "cseries/cseries.h"

/* ---------- constants */

enum
{
	NUMBER_OF_ONLINE_STATISTICS_PLACES= 5,
	ONLINE_STATISTICS_PLACE_SIZE= 0x99C,
	ONLINE_STATISTICS_RECORD_SIZE= 0x98,
	ONLINE_STATISTICS_RECORD_STATE_OFFSET= 0,
	ONLINE_STATISTICS_RECORD_TIME_OFFSET= 4,
	/* 0x13 numbers of 8 bytes: a byte that says it is there, and the number */
	ONLINE_STATISTICS_RECORD_VALUES_OFFSET= 8,
	ONLINE_STATISTICS_VALUE_SIZE= 8,
	ONLINE_STATISTICS_VALUES_PER_RECORD= 0x13,
	ONLINE_STATISTICS_VALUE_VALID_OFFSET= 0,
	ONLINE_STATISTICS_VALUE_VALUE_OFFSET= 4,

	NUMBER_OF_ONLINE_STATISTICS_KINDS= 6,
	NUMBER_OF_ONLINE_STATISTICS_LEVELS= 0x80
};

enum
{
	_online_statistics_record_state3= 3,
	_online_statistics_record_state4
};

/* ---------- prototypes */

/* the time of the console, in milliseconds */
long GetTickCount(void);

/* ---------- globals */

/* 0x00510548, 0x0051054c (main.c): whether the network keeps a time of its
own, and that time */
extern boolean unknown_00510548;
extern long unknown_0051054c;

/* 0x00509470: the records of the five places, from the first; each place has
more before its records (at 0x00509458 the first) */
byte unknown_00509470[NUMBER_OF_ONLINE_STATISTICS_PLACES*ONLINE_STATISTICS_PLACE_SIZE];

/* 0x0046722c: the smallest and the largest that a number of each of the
first six can be */
long unknown_0046722c[NUMBER_OF_ONLINE_STATISTICS_KINDS][2]=
{
	{0, 0x3FFFFFFF},
	{0, 0x7F},
	{0, 0x7F},
	{0, 0x7FFFFFFF},
	{0, 0x7FFFFFFF},
	{0, 0x7FFFFFFF}
};

/* 0x004ce7b4: from where each level starts, from the lowest */
long unknown_004ce7b4[NUMBER_OF_ONLINE_STATISTICS_LEVELS];

/* ---------- private code */

/* the time in milliseconds: the one that main.c kept, if it says so. The
game has this in each function that uses it */
static inline __attribute__((always_inline)) long online_statistics_time(
	void)
{
	return unknown_00510548 ? unknown_0051054c : GetTickCount();
}

/* a number of the record of a player in a place */
static inline __attribute__((always_inline)) byte *online_statistics_value(
	long record_index,
	long place_index,
	long kind)
{
	return unknown_00509470+place_index*ONLINE_STATISTICS_PLACE_SIZE+ONLINE_STATISTICS_RECORD_VALUES_OFFSET+
		(record_index*ONLINE_STATISTICS_VALUES_PER_RECORD+kind)*ONLINE_STATISTICS_VALUE_SIZE;
}

/* ---------- code */

/* 0x000739a0: the record of a player, in one place (in all with NONE), goes
from its fourth state to its fifth, and the time is kept */
void __attribute__((stdcall)) online_statistics_unknown_000739a0(
	long place_index,
	long record_index)
{
	long index;

	for (index= 0; index<NUMBER_OF_ONLINE_STATISTICS_PLACES; index++)
	{
		if (place_index==NONE || place_index==index)
		{
			byte *record= unknown_00509470+index*ONLINE_STATISTICS_PLACE_SIZE+record_index*ONLINE_STATISTICS_RECORD_SIZE;

			if (*(const long *)(record+ONLINE_STATISTICS_RECORD_STATE_OFFSET)==_online_statistics_record_state3)
			{
				*(long *)(record+ONLINE_STATISTICS_RECORD_STATE_OFFSET)= _online_statistics_record_state4;
				*(long *)(record+ONLINE_STATISTICS_RECORD_TIME_OFFSET)= online_statistics_time();
			}
		}
	}
}

/* 0x00074970: the level of a number: how many levels start at or below it
(0x7F if all). If that is one less than the largest that it can be, and
the number is past the middle of that level and the next, the largest */
long online_statistics_level(
	long value,
	long maximum_level)
{
	long level;

	for (level= 0; level<NUMBER_OF_ONLINE_STATISTICS_LEVELS-1; level++)
	{
		if (value<unknown_004ce7b4[level+1])
		{
			if (level==maximum_level-1)
			{
				long middle= (long)((unsigned long)unknown_004ce7b4[level]+(unsigned long)unknown_004ce7b4[level+1])/2;

				if (value>middle)
				{
					level= maximum_level;
				}
			}

			return level;
		}
	}

	return level;
}

/* 0x000749b0: a number of a record is set, between the smallest and the
largest that it can be: it is there from now */
void online_statistics_set(
	long record_index,
	long place_index,
	long kind,
	long value)
{
	byte *entry;

	if (value<unknown_0046722c[kind][0])
	{
		value= unknown_0046722c[kind][0];
	}
	else if (value>unknown_0046722c[kind][1])
	{
		value= unknown_0046722c[kind][1];
	}
	entry= online_statistics_value(record_index, place_index, kind);
	if (!entry[ONLINE_STATISTICS_VALUE_VALID_OFFSET] ||
		*(const long *)(entry+ONLINE_STATISTICS_VALUE_VALUE_OFFSET)!=value)
	{
		*(long *)(entry+ONLINE_STATISTICS_VALUE_VALUE_OFFSET)= value;
		entry[ONLINE_STATISTICS_VALUE_VALID_OFFSET]= TRUE;
	}
}

/* 0x00074a00: a number of a record that is there goes up by what it is
given, between the smallest and the largest that it can be */
void __attribute__((stdcall)) online_statistics_add(
	long record_index,
	long place_index,
	long kind,
	long amount)
{
	byte *entry= online_statistics_value(record_index, place_index, kind);
	long value;

	if (!entry[ONLINE_STATISTICS_VALUE_VALID_OFFSET] || amount==0)
	{
		return;
	}
	value= (long)((unsigned long)*(const long *)(entry+ONLINE_STATISTICS_VALUE_VALUE_OFFSET)+(unsigned long)amount);
	if (value<unknown_0046722c[kind][0])
	{
		value= unknown_0046722c[kind][0];
	}
	else if (value>unknown_0046722c[kind][1])
	{
		value= unknown_0046722c[kind][1];
	}
	*(long *)(entry+ONLINE_STATISTICS_VALUE_VALUE_OFFSET)= value;
}

/* 0x00074a60: a number of a record that is there becomes what it is given,
between the smallest and the largest that it can be, if that is more */
void online_statistics_set_maximum(
	long record_index,
	long place_index,
	long kind,
	long value)
{
	byte *entry= online_statistics_value(record_index, place_index, kind);

	if (!entry[ONLINE_STATISTICS_VALUE_VALID_OFFSET])
	{
		return;
	}
	if (value<unknown_0046722c[kind][0])
	{
		value= unknown_0046722c[kind][0];
	}
	else if (value>unknown_0046722c[kind][1])
	{
		value= unknown_0046722c[kind][1];
	}
	if (value>*(const long *)(entry+ONLINE_STATISTICS_VALUE_VALUE_OFFSET))
	{
		*(long *)(entry+ONLINE_STATISTICS_VALUE_VALUE_OFFSET)= value;
	}
}
