/*
RANDOM_MATH.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The random numbers are those of Halo 1: each one takes the seed to the next
with the same two constants, and the high 16 bits of the seed are the number.
The name real_seed_random is a guess; real_seed_random_range is Halo 1's.
*/

#include "cseries/cseries.h"
#include "real_math.h"

#include <stddef.h>

/* ---------- constants */

enum
{
	RANDOM_A= 1664525L,
	RANDOM_C= 1013904223L
};

/* ---------- prototypes */

/* of the C library and of the console's libraries */
long time(long *timer);
int rand(void);
unsigned long GetTickCount(void);

/* ---------- code */

/* 0x001462b0: a number to start random numbers with, from the time, the
random numbers of the C library and how long the console has been on. The
game has it far from the rest of this file */
long random_seed_from_time(
	void)
{
	long now= time(NULL);
	long random= rand();

	return random^GetTickCount()^now;
}

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

/* 0x004e7408, of game.c: where the game keeps the number that its random
numbers come from */
extern unsigned long *global_random_seed_reference;

void *game_state_malloc(long size);

/* 0x00146240: the first function of a part of the game (game.c): the two
numbers that the random numbers come from take their memory in the state of
the game (game_state_malloc, which the game has written here); the first is
the one that every game starts with (0x78A8, GAME_DEFAULT_RANDOM_SEED of
game.h), and the second comes from the time (random_seed_from_time, written
here too) */
void random_math_initialize(
	void)
{
	global_random_seed_reference= game_state_malloc(2*(long)sizeof(unsigned long));
	global_random_seed_reference[0]= 0x78A8;
	global_random_seed_reference[1]= (unsigned long)random_seed_from_time();
}

/* 0x004417f0: 1026 directions (of length one) to pick one of at random */
real_vector3d random_directions[1026];

/* 0x001462e0: a direction picked at random, as Halo 1's
seed_random_direction3d: the high 16 bits of the next seed, times 1026, give
one of random_directions */
real_vector3d *seed_random_direction3d(
	unsigned long *seed,
	real_vector3d *direction)
{
	short index;

	*seed= *seed*RANDOM_A + RANDOM_C;
	index= (short)(((*seed>>16)*1026)>>16);
	*direction= random_directions[index];

	return direction;
}

/* 0x0016dce0: one of some choices (0x1C bytes each) picked at random by
their weights (the reals at 0x18), with the random numbers of the game: the
first whose weight and those before it reach the number; NONE for none. The
game takes the number times the sum in the x87's precision */
long unknown_0016dce0(
	long count,
	const byte *choices)
{
	real total= 0.0f;
	real random;
	real sum;
	long index;

	if (count==0)
	{
		return NONE;
	}
	if (count==1)
	{
		return 0;
	}
	if (count<0)
	{
		return NONE;
	}
	for (index= 0; index<count; index++)
	{
		total= *(const real *)(choices+index*0x1C+0x18)+total;
	}
	*global_random_seed_reference= *global_random_seed_reference*RANDOM_A + RANDOM_C;
	random= (real)((long double)(*global_random_seed_reference>>16)*(1.0f/65535.0f)*total);
	sum= 0.0f;
	for (index= 0; index<count; index++)
	{
		sum= *(const real *)(choices+index*0x1C+0x18)+sum;
		if (sum>=random || index==count-1)
		{
			return index;
		}
	}

	return NONE;
}
