/*
BIT_VECTOR.H

Bits, one after another in numbers of 32. The names are guesses.
*/

#ifndef __BIT_VECTOR_H
#define __BIT_VECTOR_H

#include "cseries/cseries.h"

/* ---------- constants */

enum
{
	BIT_VECTOR_WORD_BITS= 32
};

/* ---------- prototypes/BIT_VECTOR.C */

long highest_bit_set(unsigned long value);
long bits_to_count(unsigned long count);
void bit_vector_or(long bit_count, const unsigned long *vector, const unsigned long *other, unsigned long *result);
long bit_vector_count_bits(const unsigned long *vector, long bit_count);
long bit_vector_highest_bit_set(const unsigned long *vector, long bit_count);

#endif
