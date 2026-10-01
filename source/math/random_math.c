/*
RANDOM_MATH.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The random numbers are those of Halo 1: each one takes the seed to the next
with the same two constants, and the high 16 bits of the seed are the number.
The name real_seed_random is a guess; real_seed_random_range is Halo 1's.
*/

#include "real_math.h"

/* ---------- constants */

enum
{
	RANDOM_A= 1664525L,
	RANDOM_C= 1013904223L
};

/* ---------- code */

/* 0x000259a0: from 0 to 1 */
real real_seed_random(
	unsigned long *seed)
{
	*seed= *seed*RANDOM_A + RANDOM_C;

	return (real)(*seed>>16)*(1.0f/65535.0f);
}

/* 0x000259d0 */
real real_seed_random_range(
	unsigned long *seed,
	real lower_bound,
	real upper_bound)
{
	real random= real_seed_random(seed);

	return (upper_bound-lower_bound)*random + lower_bound;
}
