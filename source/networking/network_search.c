/*
NETWORK_SEARCH.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

What a search of games of the online service found: up to fifty results of
0x204 bytes each, and a list of the identifiers (eight bytes each) that
were already seen. It is in the zone from 0x00090020 to 0x00091530, which
uses the online tasks. The name of the file is a guess, and so are the
names of its functions.
*/

#include "cseries/cseries.h"

#include <string.h>

/* ---------- constants */

enum
{
	MAXIMUM_NETWORK_SEARCH_RESULTS= 50,
	NETWORK_SEARCH_RESULT_SIZE= 0x204,

	/* of the search */
	NETWORK_SEARCH_STARTED_OFFSET= 0x4,
	NETWORK_SEARCH_UNKNOWN14_OFFSET= 0x14,
	NETWORK_SEARCH_UNKNOWN1C_OFFSET= 0x1C,
	NETWORK_SEARCH_UNKNOWN20_OFFSET= 0x20,
	NETWORK_SEARCH_UNKNOWN34_OFFSET= 0x34,
	NETWORK_SEARCH_RESULTS_OFFSET= 0x8C,
	NETWORK_SEARCH_IDENTIFIER_COUNT_OFFSET= 0x94,
	NETWORK_SEARCH_MAXIMUM_IDENTIFIERS_OFFSET= 0x98,
	NETWORK_SEARCH_IDENTIFIERS_OFFSET= 0xA0,

	/* of each result */
	NETWORK_SEARCH_RESULT_UNKNOWN1_OFFSET= 0x1,
	NETWORK_SEARCH_RESULT_SEEN_OFFSET= 0x2,
	NETWORK_SEARCH_RESULT_IDENTIFIER_OFFSET= 0x1C,
	NETWORK_SEARCH_RESULT_UNKNOWN48_OFFSET= 0x48,
	NETWORK_SEARCH_RESULT_UNKNOWN74_OFFSET= 0x74,
	NETWORK_SEARCH_RESULT_UNKNOWN1F8_OFFSET= 0x1F8
};

/* ---------- prototypes */

/* the time of the console, in milliseconds */
long GetTickCount(void);

/* ---------- globals */

/* 0x00510548, 0x0051054c (main.c): whether the network keeps a time of its
own, and that time */
extern boolean unknown_00510548;
extern long unknown_0051054c;

/* 0x004ce394: how long a search has to have gone on (in milliseconds) to
be taken with not all of its results */
long unknown_004ce394;

/* ---------- private code */

static inline __attribute__((always_inline)) byte *network_search_get_result(
	const byte *search,
	long index)
{
	return *(byte *const *)(search+NETWORK_SEARCH_RESULTS_OFFSET)+index*NETWORK_SEARCH_RESULT_SIZE;
}

/* ---------- code */

/* 0x00091290: whether the identifier is in the list of the ones seen */
boolean __attribute__((stdcall)) network_search_has_identifier(
	const byte *search,
	const byte *identifier)
{
	const byte *identifiers= *(const byte *const *)(search+NETWORK_SEARCH_IDENTIFIERS_OFFSET);
	boolean found= FALSE;
	long index;

	for (index= 0; index<*(const long *)(search+NETWORK_SEARCH_IDENTIFIER_COUNT_OFFSET) && !found; index++)
	{
		if (memcmp(identifiers+index*8, identifier, 8)==0)
		{
			found= TRUE;
		}
	}

	return found;
}

/* 0x000912d0: a result is seen, once: its identifier goes into the list of
the ones seen, if it was not there and the list has room */
void network_search_result_see(
	byte *result,
	byte *search)
{
	long count;

	if (result[NETWORK_SEARCH_RESULT_SEEN_OFFSET])
	{
		return;
	}
	result[NETWORK_SEARCH_RESULT_SEEN_OFFSET]= TRUE;
	result[NETWORK_SEARCH_RESULT_UNKNOWN1_OFFSET]= FALSE;
	if (network_search_has_identifier(search, result+NETWORK_SEARCH_RESULT_IDENTIFIER_OFFSET))
	{
		return;
	}
	count= *(const long *)(search+NETWORK_SEARCH_IDENTIFIER_COUNT_OFFSET);
	if (count<*(const long *)(search+NETWORK_SEARCH_MAXIMUM_IDENTIFIERS_OFFSET))
	{
		memcpy(*(byte **)(search+NETWORK_SEARCH_IDENTIFIERS_OFFSET)+count*8, result+NETWORK_SEARCH_RESULT_IDENTIFIER_OFFSET, 8);
		*(long *)(search+NETWORK_SEARCH_IDENTIFIER_COUNT_OFFSET)= count+1;
	}
}

/* 0x00091320: the first result with something at 0 and at 0x74 whose eight
bytes at 0xBC are the four after the first of what is given is seen */
void __attribute__((stdcall)) network_search_see(
	byte *search,
	const byte *data)
{
	long index;

	if (!search[0])
	{
		return;
	}
	for (index= 0; index<MAXIMUM_NETWORK_SEARCH_RESULTS; index++)
	{
		byte *result= network_search_get_result(search, index);

		if (result[0] && result[NETWORK_SEARCH_RESULT_UNKNOWN74_OFFSET] && memcmp(data+4, result+0xBC, 8)==0)
		{
			network_search_result_see(result, search);
			return;
		}
	}
}

/* 0x00091380: two numbers of the search (the second one more, and the
first that one if it is NONE, unless 0x14 says otherwise), how many results
it has and the sum of their numbers at 0x48. Nothing if it is not going
on */
void __attribute__((stdcall)) network_search_get_counts(
	const byte *search,
	long *first,
	long *second,
	long *count,
	long *total)
{
	long first_value;
	long second_value;
	long result_count= 0;
	unsigned long sum= 0;
	long index;

	if (!search[0])
	{
		return;
	}
	if (search[NETWORK_SEARCH_UNKNOWN14_OFFSET])
	{
		second_value= *(const long *)(search+NETWORK_SEARCH_UNKNOWN20_OFFSET);
		first_value= *(const long *)(search+NETWORK_SEARCH_UNKNOWN1C_OFFSET);
	}
	else
	{
		first_value= *(const long *)(search+NETWORK_SEARCH_UNKNOWN1C_OFFSET);
		second_value= *(const long *)(search+NETWORK_SEARCH_UNKNOWN20_OFFSET)+1;
		if (first_value==NONE)
		{
			first_value= *(const long *)(search+NETWORK_SEARCH_UNKNOWN20_OFFSET);
		}
	}
	for (index= 0; index<MAXIMUM_NETWORK_SEARCH_RESULTS; index++)
	{
		const byte *result= network_search_get_result(search, index);

		if (result[0])
		{
			result_count++;
			sum+= *(const unsigned long *)(result+NETWORK_SEARCH_RESULT_UNKNOWN48_OFFSET);
		}
	}
	*first= first_value;
	*second= second_value;
	*count= result_count;
	*total= (long)sum;
}

/* 0x00091400: the first result (with something at 0, 1 and 0x74) whose
number at 0x1F8 is at least the one of the search at 0x34. It is taken if
all the results with something at 0 and 1 have something at 0x74, or if a
quarter of them do and the search has gone on longer than unknown_004ce394.
0x44 bytes of what it has are given, if asked for */
boolean __attribute__((stdcall)) network_search_best_result(
	const byte *search,
	byte *data)
{
	const byte *best= NULL;
	long count= 0;
	long ready_count= 0;
	long index;

	if (!search[0])
	{
		return FALSE;
	}
	for (index= 0; index<MAXIMUM_NETWORK_SEARCH_RESULTS; index++)
	{
		const byte *result= network_search_get_result(search, index);

		if (result[0] && result[NETWORK_SEARCH_RESULT_UNKNOWN1_OFFSET])
		{
			count++;
			if (result[NETWORK_SEARCH_RESULT_UNKNOWN74_OFFSET])
			{
				ready_count++;
				if (*(const long *)(result+NETWORK_SEARCH_RESULT_UNKNOWN1F8_OFFSET)>=*(const long *)(search+NETWORK_SEARCH_UNKNOWN34_OFFSET) &&
					!best)
				{
					best= result;
				}
			}
		}
	}
	if (!best)
	{
		return FALSE;
	}
	if (ready_count!=count)
	{
		long time;

		if (ready_count<(count>>2))
		{
			return FALSE;
		}
		time= unknown_00510548 ? unknown_0051054c : GetTickCount();
		if ((long)((unsigned long)time-*(const unsigned long *)(search+NETWORK_SEARCH_STARTED_OFFSET))<=unknown_004ce394)
		{
			return FALSE;
		}
	}
	if (data)
	{
		memset(data, 0, 0x44);
		*(long *)data= *(const short *)(best+0xAA);
		memcpy(data+4, best+0xBC, 8);
		memcpy(data+0xC, best+0xC4, 0x10);
		memcpy(data+0x1C, best+0xD4, 0x24);
		*(long *)(data+0x40)= *(const long *)(best+0x90);
	}

	return TRUE;
}
