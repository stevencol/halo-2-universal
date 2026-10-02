/*
CSERIES.H

The types and the constants that all the game's code uses, with the names
they have in Halo 1.

The code decompiled from the game is 32-bit code: long is 32 bits and so is
a pointer. The definitions with pointers in them have the size they have in
the game only when compiled for a 32-bit target.
*/

#ifndef __CSERIES_H
#define __CSERIES_H

#include <stddef.h>

/* ---------- constants */

#define NONE -1

#define UNSIGNED_CHAR_MAX 255
#define UNSIGNED_SHORT_MAX 65535

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

/* the languages of the game's text. The names are those of the console's
languages */
enum
{
	_language_english,
	_language_japanese,
	_language_german,
	_language_french,
	_language_spanish,
	_language_italian,
	_language_korean,
	_language_chinese,
	NUMBER_OF_LANGUAGES
};

/* ---------- types */

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned char boolean;

/* a time of the console's clock (FILETIME) */
struct file_time
{
	unsigned long low;
	unsigned long high;
};

/* ---------- prototypes/CSERIES.C */

char *csprintf(char *buffer, const char *format, ...);
char *csprintf256(char *buffer, const char *format, ...);
char *csnzprintf(char *buffer, unsigned long size, const char *format, ...);
char *csstrcat256(char *destination, const char *source);
void file_time_get_current(struct file_time *time);
void __attribute__((stdcall)) no_operation1(unsigned long unused);
void no_operation(void);
long language_from_xbox_language(long xbox_language);
long csstrcasecmp(const char *s1, const char *s2);
boolean bit_vector_test_flag(const unsigned long *vector, long bit);
void bit_vector_fill(unsigned long *vector, long count, byte value);
float system_ticks_to_seconds(long long ticks);
void path_remove_last_element(char *path);

/* ---------- macros */

#define FLAG(bit) (1<<(bit))
#define TEST_FLAG(flags, bit) (((flags)&FLAG(bit))!=0)

/* a bit of a vector of bits made of 32-bit words */
#define BIT_VECTOR_WORD_INDEX(bit) ((bit)>>5)
#define BIT_VECTOR_WORD_FLAG(bit) (1UL<<((bit)&31))
#define BIT_VECTOR_SIZE_IN_LONGS(bit_count) (((bit_count)+31)>>5)
#define BIT_VECTOR_TEST_FLAG(vector, bit) \
	(((vector)[BIT_VECTOR_WORD_INDEX(bit)]&BIT_VECTOR_WORD_FLAG(bit))!=0)
#define BIT_VECTOR_OR_FLAG(vector, bit) \
	((vector)[BIT_VECTOR_WORD_INDEX(bit)]|= BIT_VECTOR_WORD_FLAG(bit))
#define BIT_VECTOR_AND_FLAG(vector, bit) \
	((vector)[BIT_VECTOR_WORD_INDEX(bit)]&= ~BIT_VECTOR_WORD_FLAG(bit))

#endif
