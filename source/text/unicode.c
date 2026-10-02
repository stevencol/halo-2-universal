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

/* ---------- prototypes of functions of the console's libraries */

/* 0x003314b0: the milliseconds since the console started */
long GetTickCount(void);

/* ---------- globals */

/* 0x0055e730: when a letter after a bar that stands for nothing can be noted
again */
unsigned long unicode_escape_note_time;

/* 0x004536cc: the characters that are spaces */
const struct character_range white_space_ranges[NUMBER_OF_WHITE_SPACE_RANGES]=
{
	{ 0x0009, 0x000D },
	{ 0x0020, 0x0020 },
	{ 0x0085, 0x0085 },
	{ 0x00A0, 0x00A0 },
	{ 0x1680, 0x1680 },
	{ 0x180E, 0x180E },
	{ 0x2000, 0x200A },
	{ 0x2028, 0x2028 },
	{ 0x2029, 0x2029 },
	{ 0x202F, 0x202F },
	{ 0x205F, 0x205F },
	{ 0x3000, 0x3000 }
};

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

/* 0x0013fb50: writes formatted text to a string, of count characters at
most: if the text is as long or longer, it has no end. The name is a guess */
void usnprintf(
	long count,
	wchar *string,
	const wchar *format,
	...)
{
	va_list arguments;

	va_start(arguments, format);
	_vsnwprintf(string, (size_t)count, format, arguments);
	va_end(arguments);
}

/* 0x0013fbd0: the character that a letter after a bar stands for in a text
of the game: the bar itself, an end of line, a tab, or how the lines that
follow are set (left, right, centre). Whether the letter is one of them: if
it is not, the character is the bar. A letter that is none is noted, at most
once a minute: nothing is done with the note */
long unicode_escape_character(
	wchar character,
	boolean *found)
{
	const struct unicode_escape escapes[]=
	{
		{ '|', '|' },
		{ 'l', _unicode_character_justify_left },
		{ 'r', _unicode_character_justify_right },
		{ 'c', _unicode_character_justify_center },
		{ 'n', '\r' },
		{ 't', '\t' },
		{ 0, 0 }
	};
	long index;

	for (index= 0; escapes[index].character; index++)
	{
		if (character==escapes[index].character)
		{
			*found= TRUE;

			return escapes[index].value;
		}
	}
	if (character)
	{
		unsigned long time= (unsigned long)GetTickCount();

		*found= FALSE;
		if (time>unicode_escape_note_time)
		{
			unicode_escape_note_time= time+UNICODE_ESCAPE_NOTE_MILLISECONDS;
		}
	}
	else
	{
		*found= FALSE;
	}

	return '|';
}

/* 0x0013fde0: whether a line cannot start with a character: what closes, and
the small letters of Japanese writing */
boolean character_cannot_start_line(
	unsigned long character)
{
	switch (character)
	{
	case 0x0021: case 0x0022: case 0x0025: case 0x0029: case 0x002C: case 0x002E:
	case 0x003A: case 0x003B: case 0x003F: case 0x005D: case 0x007D: case 0x00B0:
	case 0x00B7: case 0x2013: case 0x2014: case 0x2019: case 0x201D: case 0x2022:
	case 0x2026: case 0x2027: case 0x2032: case 0x2033: case 0x2103: case 0x3001:
	case 0x3002: case 0x3009: case 0x300B: case 0x300D: case 0x300F: case 0x3011:
	case 0x3015: case 0x301E: case 0x3041: case 0x3043: case 0x3045: case 0x3047:
	case 0x3049: case 0x3063: case 0x3083: case 0x3085: case 0x3087: case 0x308E:
	case 0x30A1: case 0x30A3: case 0x30A5: case 0x30A7: case 0x30A9: case 0x30C3:
	case 0x30E3: case 0x30E5: case 0x30E7: case 0x30EE: case 0x30FC: case 0xFE30:
	case 0xFE50: case 0xFE51: case 0xFE52: case 0xFE54: case 0xFE55: case 0xFE56:
	case 0xFE57: case 0xFE5A: case 0xFE5C: case 0xFE5E: case 0xFF01: case 0xFF05:
	case 0xFF09: case 0xFF0C: case 0xFF0E: case 0xFF1A: case 0xFF1B: case 0xFF1F:
	case 0xFF3D: case 0xFF5D: case 0xFF61: case 0xFF64: case 0xFF70: case 0xFF9E:
	case 0xFF9F: case 0xFFE0:
		return TRUE;
	}

	return FALSE;
}

/* 0x00140260: whether a line cannot end with a character: what opens */
boolean character_cannot_end_line(
	unsigned long character)
{
	switch (character)
	{
	case 0x0022: case 0x0024: case 0x0028: case 0x005B: case 0x005C: case 0x007B:
	case 0x2018: case 0x201C: case 0x2035: case 0x3008: case 0x300A: case 0x300C:
	case 0x300E: case 0x3010: case 0x3014: case 0x301D: case 0xFE59: case 0xFE5B:
	case 0xFE5D: case 0xFF04: case 0xFF08: case 0xFF3B: case 0xFF5B: case 0xFFE1:
	case 0xFFE5: case 0xFFE6:
		return TRUE;
	}

	return FALSE;
}

/* 0x0013fd20: whether a line can end after a character, with another one
after it: if the first is a space, or either is of the writings that have no
spaces between words, and the first can end a line and the second can start
one */
boolean character_can_break_line(
	long character,
	long next)
{
	boolean cannot_start;
	boolean cannot_end;

	if (!character)
	{
		return FALSE;
	}
	cannot_start= character_cannot_start_line((unsigned long)next);
	cannot_end= character_cannot_end_line((unsigned long)character);

	return (character_in_ranges((wchar)character, NUMBER_OF_WHITE_SPACE_RANGES, white_space_ranges) ||
		character_is_east_asian(character) || character_is_east_asian(next)) && !cannot_start && !cannot_end;
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
