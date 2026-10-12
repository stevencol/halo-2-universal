/*
RANDOM_MATH.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The random numbers are those of Halo 1: each one takes the seed to the next
with the same two constants, and the high 16 bits of the seed are the number.
The name real_seed_random is a guess; real_seed_random_range is Halo 1's.
*/

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
