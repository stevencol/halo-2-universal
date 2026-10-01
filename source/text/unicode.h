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

/* ---------- structures */

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
void wchar_string_to_ascii_string(const wchar *source, char *destination, long count);
void ascii_string_to_wchar_string(const char *source, wchar *destination, long count);
boolean character_in_ranges(wchar character, long range_count, const struct character_range *ranges);
boolean character_is_east_asian(long character);

#endif
