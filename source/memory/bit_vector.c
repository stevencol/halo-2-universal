/*
BIT_VECTOR.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

Bits, one after another in numbers of 32. The names of the file and of the
functions are guesses.
*/

#include "bit_vector.h"

/* ---------- code */

/* how many numbers of 32 bits so many bits take */
static inline __attribute__((always_inline)) long bit_vector_word_count(
	long bit_count)
{
	return (bit_count+(BIT_VECTOR_WORD_BITS-1))>>5;
}

/* the bits of the last number that are of the vector: all of them, if the
bits fill it */
static inline __attribute__((always_inline)) unsigned long bit_vector_last_word_mask(
	long bit_count)
{
	long bits= bit_count&(BIT_VECTOR_WORD_BITS-1);

	return bits>0 ? 0xFFFFFFFFUL>>(BIT_VECTOR_WORD_BITS-bits) : 0xFFFFFFFFUL;
}

/* how many bits of a number are set */
static inline __attribute__((always_inline)) long word_count_bits(
	unsigned long value)
{
	value= ((value>>1)&0x55555555)+(value&0x55555555);
	value= ((value>>2)&0x33333333)+(value&0x33333333);
	value= ((value>>4)&0x0F0F0F0F)+(value&0x0F0F0F0F);
	value= ((value>>8)&0x00FF00FF)+(value&0x00FF00FF);

	return (long)((value&0xFFFF)+(value>>16));
}

/* the highest bit of a number that is set; NONE if none is */
static inline __attribute__((always_inline)) long word_highest_bit_set(
	unsigned long value)
{
	return value ? (BIT_VECTOR_WORD_BITS-1)-__builtin_clzl(value) : NONE;
}

/* 0x0013e3a0: the highest bit of a number that is set; 0 if none is */
long highest_bit_set(
	unsigned long value)
{
	long bit= 0;

	if (value)
	{
		while (value!=1)
		{
			value>>= 1;
			bit++;
		}
	}

	return bit;
}

/* 0x0013e3c0: how many bits it takes to tell so many things apart: one at
the least */
long bits_to_count(
	unsigned long count)
{
	long bits= 0;

	if (count>1)
	{
		for (count--; count!=1; count>>= 1)
		{
			bits++;
		}
	}

	return bits+1;
}

/* 0x0013e3e0: the bits that are set in one of two vectors */
void bit_vector_or(
	long bit_count,
	const unsigned long *vector,
	const unsigned long *other,
	unsigned long *result)
{
	long index;

	for (index= bit_vector_word_count(bit_count)-1; index>=0; index--)
	{
		result[index]= other[index]|vector[index];
	}
}

/* 0x0013e420: how many bits of a vector are set. With no bits, the game
counts those of the number before the vector */
long bit_vector_count_bits(
	const unsigned long *vector,
	long bit_count)
{
	long word_count= bit_vector_word_count(bit_count);
	long count= 0;
	long index;

	for (index= 0; index<word_count-1; index++)
	{
		count+= word_count_bits(vector[index]);
	}

	return word_count_bits(vector[word_count-1]&bit_vector_last_word_mask(bit_count))+count;
}

/* 0x0013e530: the highest bit of a vector that is set; NONE if none is. With
no bits, the game looks at the number before the vector */
long bit_vector_highest_bit_set(
	const unsigned long *vector,
	long bit_count)
{
	long word_count= bit_vector_word_count(bit_count);
	long bit= word_highest_bit_set(vector[word_count-1]&bit_vector_last_word_mask(bit_count));
	long index;

	if (bit>=0)
	{
		bit+= (word_count-1)*BIT_VECTOR_WORD_BITS;
	}
	for (index= word_count-2; bit==NONE && index>=0; index--)
	{
		bit= word_highest_bit_set(vector[index]);
		if (bit>=0)
		{
			bit+= index*BIT_VECTOR_WORD_BITS;
		}
	}

	return bit;
}
