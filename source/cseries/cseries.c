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

/* 0x003312cc */
void *VirtualAlloc(void *address, unsigned long size, unsigned long type, unsigned long protection);

/* 0x002d1d3e */
unsigned long GetLastError(void);

/* MEM_COMMIT | MEM_TOP_DOWN, and PAGE_READWRITE */
#define SYSTEM_ALLOCATION_TYPE 0x101000
#define SYSTEM_ALLOCATION_PROTECTION 4

/* ---------- prototypes of functions of the console's libraries */

/* 0x003303f3 */
void GetSystemTimeAsFileTime(struct file_time *time);

/* 0x0033039a: the language of the console */
long XGetLanguage(void);

/* ---------- globals */

/* 0x0047ff38 (cache_files.c): the language of the game, or NONE */
extern long global_language;

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

/* 0x000b66c0: writes formatted text to a string of 1,024 characters; the
string. The name is a guess */
char *csprintf(
	char *buffer,
	const char *format,
	...)
{
	va_list arguments;

	va_start(arguments, format);
	vsnprintf(buffer, 1023, format, arguments);
	va_end(arguments);
	buffer[1023]= 0;

	return buffer;
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

/* 0x0011c9a0: how many characters a string has before its end, and no more
than a number. The name is Halo 3's */
unsigned long csstrnlen(
	const char *string,
	unsigned long maximum)
{
	unsigned long length= 0;

	while (length<maximum && *string++)
	{
		length++;
	}

	return length;
}

/* 0x0011c9e0: as csnzprintf, after what the buffer has. The name is
Halo 3's */
char *csnzappendf(
	char *buffer,
	unsigned long size,
	const char *format,
	...)
{
	unsigned long length= strlen(buffer);
	char *end= buffer+length;
	va_list arguments;

	size-= length;
	va_start(arguments, format);
	vsnprintf(end, size-1, format, arguments);
	va_end(arguments);
	end[size-1]= 0;

	return buffer;
}

/* 0x0011ca20: memory of the console, from the top; NULL if there is none.
The name is a guess */
void *__attribute__((stdcall)) system_allocate(
	unsigned long size)
{
	void *memory= VirtualAlloc(NULL, size, SYSTEM_ALLOCATION_TYPE, SYSTEM_ALLOCATION_PROTECTION);

	if (!memory)
	{
		GetLastError();
	}

	return memory;
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

/* 0x0012afc0: fills a vector of so many bits with a byte: 0 clears them
all. The name is a guess */
void bit_vector_fill(
	unsigned long *vector,
	long count,
	byte value)
{
	memset(vector, value, (unsigned long)(((count+31)>>5)<<2));
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

/* 0x00020670: a function is called with each of the elements of an array,
as what makes them: it takes the element in a register. Nothing for an array
of none */
void __attribute__((stdcall)) array_construct_elements(
	void *array,
	unsigned long size,
	long count,
	void *(__attribute__((thiscall)) *constructor)(void *element))
{
	while (--count>=0)
	{
		constructor(array);
		array= (byte *)array+size;
	}
}

/* 0x000bfe20: a bit of a number is set or not */
void flags_set_bit(
	unsigned long *flags,
	long bit,
	boolean value)
{
	if (value)
	{
		*flags|= 1u<<(bit&31);
	}
	else
	{
		*flags&= ~(1u<<(bit&31));
	}
}

/* 0x000bfe40: the same for a number of 16 bits */
void __attribute__((stdcall)) word_flags_set_bit(
	word *flags,
	long bit,
	boolean value)
{
	if (value)
	{
		*flags|= (word)(1u<<(bit&31));
	}
	else
	{
		*flags&= (word)~(1u<<(bit&31));
	}
}

/* 0x000bfe80: each bit of a vector of 232 bits (seven numbers and eight
bits more) is turned: the bits past the last are left as zeros */
void bit_vector232_not(
	unsigned long *vector)
{
	long index;

	for (index= 0; index<8; index++)
	{
		vector[index]= ~vector[index];
	}
	vector[7]&= 0xFF;
}

/* 0x000bfed0: the bits of a vector of 256 from a number on are zeros. The
number of the vector that has the last bit that stays is always written,
with all its bits if it is full: for no bits at all that is the number
before the vector, which the game reads and writes back as it was */
void bit_vector256_clear_from(
	long count,
	unsigned long *vector)
{
	long last= ((count+31)>>5)-1;
	unsigned long mask= 0xFFFFFFFF;
	long index;

	for (index= last+1; index<8; index++)
	{
		vector[index]= 0;
	}
	if (count&31)
	{
		mask>>= 32-(count&31);
	}
	vector[last]&= mask;
}

/* 0x000bff10: whether a vector of 256 bits has none */
boolean bit_vector256_is_empty(
	const unsigned long *vector)
{
	boolean empty= vector[7]==0;

	empty&= vector[6]==0;
	empty&= vector[5]==0;
	empty&= vector[4]==0;
	empty&= vector[3]==0;
	empty&= vector[2]==0;
	empty&= vector[1]==0;
	empty&= vector[0]==0;

	return empty;
}

/* 0x00547634, 0x00547638: two real numbers that the game works out when it
starts: a whole turn and half of one, most likely */
float unknown_00547634;
float unknown_00547638;

/* 0x000bff60: one angle less another, brought to within half a turn: a whole
turn less if it is half a turn or more, and a whole turn more if after that
it is not more than half a turn the other way */
float __attribute__((stdcall)) angle_difference(
	float angle,
	float other)
{
	float difference= angle-other;

	if (difference>=unknown_00547638)
	{
		difference-= unknown_00547634;
	}
	if (!(0.0f-unknown_00547638<difference))
	{
		difference= unknown_00547634+difference;
	}

	return difference;
}

/* 0x0011cae0: the language of the game, worked out once from that of the
console */
long get_current_language(
	void)
{
	if (global_language==NONE)
	{
		global_language= language_from_xbox_language(XGetLanguage());
	}

	return global_language;
}

/* 0x000b6190: whether two texts are the same, as strcmp(...)==0 written
two characters at a time. It is before csprintf in the game */
boolean cseries_unknown_000b6190(
	const char *first,
	const char *second)
{
	for (;;)
	{
		if (first[0]!=second[0])
		{
			return FALSE;
		}
		if (!first[0])
		{
			return TRUE;
		}
		if (first[1]!=second[1])
		{
			return FALSE;
		}
		if (!first[1])
		{
			return TRUE;
		}
		first+= 2;
		second+= 2;
	}
}

/* 0x000b6720: how long a text is, five characters at a time, up to 0xFF.
It is after csprintf256 in the game */
unsigned long cseries_unknown_000b6720(
	const char *string)
{
	unsigned long length= 0;

	for (;;)
	{
		if (!string[0])
		{
			return length;
		}
		if (!string[1])
		{
			return length+1;
		}
		if (!string[2])
		{
			return length+2;
		}
		if (!string[3])
		{
			return length+3;
		}
		if (!string[4])
		{
			return length+4;
		}
		string+= 5;
		length+= 5;
		if (length>=0xFF)
		{
			return length;
		}
	}
}

/* 0x00044690: the next bit that is set of a list of longs (as many as the
first short; the second is the one being looked at, and the long after it
what is left of it), and it is cleared there; FALSE after the last */
boolean __attribute__((stdcall)) bit_vector_iterator_next(
	byte *iterator,
	long *bit)
{
	short *count= (short *)iterator;
	short *index= (short *)(iterator+2);
	unsigned long *bits= (unsigned long *)(iterator+4);
	long which;

	if (!*bits)
	{
		do
		{
			(*index)++;
			if (*index>=*count)
			{
				break;
			}
			*bits= (*(const unsigned long *const *)(iterator+8))[*index];
		}
		while (!*bits);
	}
	if (*index>=*count)
	{
		return FALSE;
	}
	which= __builtin_ctz(*bits);
	*bit= *index*32+which;
	*bits&= ~(1UL<<which);
	return TRUE;
}
