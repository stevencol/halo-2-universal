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

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

/* ---------- types */

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned char boolean;

/* ---------- prototypes/CSERIES.C */

boolean bit_vector_test_flag(const unsigned long *vector, long bit);
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
