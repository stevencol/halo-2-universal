/*
BINARY_SEARCH.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The name of the function and where it goes are a guess.
*/

#include "binary_search.h"

#include "cseries/cseries.h"

/* ---------- code */

/* 0x0013ddd0: the index of the element of a sorted array that a key is;
NONE if no element is */
long binary_search(
	long count,
	const void *key,
	const void *elements,
	long element_size,
	binary_search_compare_proc compare,
	void *context)
{
	const byte *first= elements;

	while (count)
	{
		const byte *middle= first+(count>>1)*element_size;
		long result= compare(key, middle, context);

		if (!result)
		{
			return (long)(middle-(const byte *)elements)/element_size;
		}
		if (result>0)
		{
			first= middle+element_size;
			count--;
		}
		count>>= 1;
	}

	return NONE;
}

/* 0x0013dd70: the same, of a sorted array of numbers of 4 bytes: the key
and the numbers themselves are given to the function that compares */
long binary_search_4byte(
	long count,
	long key,
	const long *elements,
	binary_search_4byte_compare_proc compare,
	void *context)
{
	const long *first= elements;

	while (count)
	{
		const long *middle= first+(count>>1);
		long result= compare(key, *middle, context);

		if (!result)
		{
			return (long)(middle-elements);
		}
		if (result>0)
		{
			first= middle+1;
			count--;
		}
		count>>= 1;
	}

	return NONE;
}
