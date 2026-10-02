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

/* ---------- prototypes of functions of the console's libraries */

/* 0x003303f3 */
void GetSystemTimeAsFileTime(struct file_time *time);

/* ---------- code */

/* 0x00072c70: does nothing; takes an argument off the stack. The game puts
it where it has to give a function and has nothing to do. Where this
function goes and its name are a guess */
void __attribute__((stdcall)) no_operation1(
	unsigned long unused)
{
	(void)unused;
}

/* 0x00175f40: does nothing. The same, for a function with no arguments */
void no_operation(
	void)
{
}

/* 0x00136e70: the time of the console's clock. Where this function goes and
its name are a guess */
void file_time_get_current(
	struct file_time *time)
{
	struct file_time now;

	GetSystemTimeAsFileTime(&now);
	*time= now;
}

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

/* 0x00122810: adds a string to the end of a string of 256 characters; the
string. The name is a guess */
char *csstrcat256(
	char *destination,
	const char *source)
{
	unsigned long length;

	for (length= 0; length<256 && destination[length]; length++)
	{
	}
	strncpy(destination+length, source, 256-length);
	destination[255]= 0;

	return destination;
}

/* 0x0011c9c0: writes formatted text to a string of a size; the string. The
name is that of Halo 1's function */
char *csnzprintf(
	char *buffer,
	unsigned long size,
	const char *format,
	...)
{
	va_list arguments;

	va_start(arguments, format);
	vsnprintf(buffer, size-1, format, arguments);
	va_end(arguments);
	buffer[size-1]= 0;

	return buffer;
}

/* 0x0011ca80: the game's language for a language of the console
(XGetLanguage): English for the ones that the game does not have. The name
is a guess */
long language_from_xbox_language(
	long xbox_language)
{
	switch (xbox_language)
	{
	case 2:
		return _language_japanese;
	case 3:
		return _language_german;
	case 4:
		return _language_french;
	case 5:
		return _language_spanish;
	case 6:
		return _language_italian;
	case 7:
		return _language_korean;
	case 8:
		return _language_chinese;
	}

	return _language_english;
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

#define SYSTEM_TICKS_PER_SECOND 733333333LL

/* 0x00223bb0: the seconds that a count of the processor's ticks (rdtsc) is:
the console's processor does 733,333,333 a second. The name is a guess */
float system_ticks_to_seconds(
	long long ticks)
{
	long long seconds= ticks/SYSTEM_TICKS_PER_SECOND;
	long long remainder= ticks-seconds*SYSTEM_TICKS_PER_SECOND;

	return (float)((long double)remainder/(long double)SYSTEM_TICKS_PER_SECOND+(long double)seconds);
}
