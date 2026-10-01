/*
CSERIES.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

Where these functions go and their names are a guess.
*/

#include "cseries.h"

#include <string.h>

/* ---------- code */

/* 0x000bfe60: whether a bit of a vector of bits is set */
boolean bit_vector_test_flag(
	const unsigned long *vector,
	long bit)
{
	return BIT_VECTOR_TEST_FLAG(vector, bit);
}

/* 0x001373c0: takes the last element off a path: the path ends where its
last backslash was. A path with no backslash becomes empty */
void path_remove_last_element(
	char *path)
{
	short index= (short)strlen(path);

	while (index>0 && path[index]!='\\')
	{
		index--;
	}
	path[index]= 0;

	return;
}
