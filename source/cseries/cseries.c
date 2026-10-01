/*
CSERIES.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

Where these functions go and their names are a guess.
*/

#include "cseries.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <wctype.h>

/* ---------- code */

/* 0x000b66f0: writes formatted text to a string of 256 characters; the
string. The name is a guess */
char *csprintf256(
	char *buffer,
	const char *format,
	...)
{
	va_list arguments;

	va_start(arguments, format);
	vsnprintf(buffer, 256, format, arguments);
	va_end(arguments);
	buffer[255]= 0;

	return buffer;
}

/* 0x0011c920: compares two strings without regard to case; -1, 0 or 1. As
in Halo 1 */
long csstrcasecmp(
	const char *s1,
	const char *s2)
{
	word c1= (word)towlower((wint_t)(short)(signed char)*s1);
	word c2= (word)towlower((wint_t)(short)(signed char)*s2);

	while (c1)
	{
		if (!c2)
		{
			return 1;
		}
		if (c1!=c2)
		{
			return c1>c2 ? 1 : -1;
		}
		s1++;
		s2++;
		c1= (word)towlower((wint_t)(short)(signed char)*s1);
		c2= (word)towlower((wint_t)(short)(signed char)*s2);
	}

	return c2 ? -1 : 0;
}

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
