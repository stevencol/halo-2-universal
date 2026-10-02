/*
UNICODE.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

ustrnupr is Halo 1's name. The others are guesses: ustrnzcat and ustrnzcpy
after the functions of later games of the series that do the same.

Every count is of characters, with the zero that ends the string.
*/

#include <stdarg.h>
#include <stddef.h>
#include "unicode.h"

/* ---------- prototypes of functions of the C library */

/* 0x00321ef9: of characters of 16 bits */
int _vsnwprintf(wchar *buffer, size_t size, const wchar *format, va_list arguments);

/* ---------- code */

/* 0x0013fa70: adds a string to the end of another that has room for count
characters */
void ustrnzcat(
	wchar *destination,
	const wchar *source,
	long count)
{
	while (*destination)
	{
		destination++;
		count--;
	}
	while (*source && count>1)
	{
		*destination++= *source++;
		count--;
	}
	*destination= 0;

	return;
}

/* 0x0013fac0: copies a string to room for count characters; what is left
of the room is zeros, as with the C library's function */
void ustrnzcpy(
	wchar *destination,
	const wchar *source,
	long count)
{
	unsigned long index= 0;
	unsigned long copy_count= (unsigned long)(count-1);

	for (; index<copy_count && source[index]; index++)
	{
		destination[index]= source[index];
	}
	for (; index<copy_count; index++)
	{
		destination[index]= 0;
	}
	destination[count-1]= 0;

	return;
}

/* 0x0013fae0: makes the letters capitals, the two accented ones that the
game's text has too. It goes on one character past the count */
void ustrnupr(
	wchar *string,
	long count)
{
	for (; *string && count>=0; string++, count--)
	{
		wchar character= *string;

		if (character>='a' && character<='z')
		{
			character-= 'a'-'A';
		}
		else if (character==0xE9)
		{
			character= 0xC9;
		}
		else if (character==0xF3)
		{
			character= 0xD3;
		}
		*string= character;
	}

	return;
}

/* 0x0013fb70: writes formatted text to a string that has room for count
characters. The name is a guess, after csnzprintf */
void usnzprintf(
	wchar *string,
	long count,
	const wchar *format,
	...)
{
	va_list arguments;

	va_start(arguments, format);
	_vsnwprintf(string, (size_t)(count-1), format, arguments);
	va_end(arguments);
	string[count-1]= 0;
}

/* 0x0013fb90: a character that is not ASCII becomes a question mark */
void wchar_string_to_ascii_string(
	const wchar *source,
	char *destination,
	long count)
{
	wchar character;

	do
	{
		if (count<1)
		{
			return;
		}

		character= *source;
		if (count==1)
		{
			*destination= 0;
		}
		else if (character<0x80)
		{
			*destination= (char)character;
		}
		else
		{
			*destination= '?';
		}
		source++;
		destination++;
		count--;
	}
	while (character);

	return;
}

/* 0x0013fc90: a character that is not ASCII becomes a white square */
void ascii_string_to_wchar_string(
	const char *source,
	wchar *destination,
	long count)
{
	for (; count>=1; source++, destination++, count--)
	{
		if (count==1)
		{
			*destination= 0;
		}
		else if ((byte)*source<0x80)
		{
			*destination= (wchar)(short)*source;
		}
		else
		{
			*destination= 0x25A1;
		}

		if (!*source)
		{
			break;
		}
	}

	return;
}

/* 0x0013fcd0: whether a character is in one of some ranges, which are in
order */
boolean character_in_ranges(
	wchar character,
	long range_count,
	const struct character_range *ranges)
{
	long low= 0;
	long high= range_count-1;

	while (low<=high)
	{
		long middle= (high+low)/2;

		if (character<ranges[middle].first)
		{
			high= middle-1;
		}
		else if (character<=ranges[middle].last)
		{
			return TRUE;
		}
		else
		{
			low= middle+1;
		}
	}

	return FALSE;
}

/* 0x0013fd90: whether a character is of Chinese, Japanese or Korean writing,
or of the full width forms */
boolean character_is_east_asian(
	long character)
{
	return
		(character>=0x1100 && character<=0x11FF) ||
		(character>=0x3000 && character<=0xD7AF) ||
		(character>=0xF900 && character<=0xFAFF) ||
		(character>=0xFF00 && character<=0xFFDC);
}
