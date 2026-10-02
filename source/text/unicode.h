/*
UNICODE.H

Strings of 16-bit characters.
*/

#ifndef __UNICODE_H
#define __UNICODE_H

#include "cseries/cseries.h"

/* ---------- types */

/* a character of the console's wide strings: 16 bits */
typedef unsigned short wchar;

/* ---------- constants */

enum
{
	/* characters that say how the lines that follow them are set */
	_unicode_character_justify_left= 0xE405,
	_unicode_character_justify_right,
	_unicode_character_justify_center,

	NUMBER_OF_WHITE_SPACE_RANGES= 12,
	UNICODE_ESCAPE_NOTE_MILLISECONDS= 60000
};

/* ---------- structures */

/* what a letter after a bar stands for */
struct unicode_escape
{
	wchar character;
	long value;
};

/* a range of characters, from the first to the last */
struct character_range
{
	wchar first;
	wchar last;
};

/* ---------- prototypes/UNICODE.C */

void ustrnzcat(wchar *destination, const wchar *source, long count);
void ustrnzcpy(wchar *destination, const wchar *source, long count);
void ustrnupr(wchar *string, long count);
void usnprintf(long count, wchar *string, const wchar *format, ...);
void usnzprintf(wchar *string, long count, const wchar *format, ...);
long unicode_escape_character(wchar character, boolean *found);
boolean character_cannot_start_line(unsigned long character);
boolean character_cannot_end_line(unsigned long character);
boolean character_can_break_line(long character, long next);
void wchar_string_to_ascii_string(const wchar *source, char *destination, long count);
void ascii_string_to_wchar_string(const char *source, wchar *destination, long count);
boolean character_in_ranges(wchar character, long range_count, const struct character_range *ranges);
boolean character_is_east_asian(long character);

#endif
