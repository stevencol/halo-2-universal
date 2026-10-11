/*
SOUND_SPEAKERS.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

How loud each speaker is for a sound that comes from a direction. The names
are guesses.
*/

#include "sound_speakers.h"

/* ---------- prototypes */

/* 0x00192b90: how loud one of the speakers is for a direction */
real unknown_00192b90(long index, const real_vector3d *direction, long unknown);

/* ---------- globals */

/* 0x00510c9c: sixteen bits, one for each of something; of the players, by
who asks */
word *unknown_00510c9c;

/* ---------- code */

/* 0x00192950: whether one of the sixteen bits of unknown_00510c9c is set */
boolean __attribute__((stdcall)) sound_speakers_unknown_00192950(
	long index)
{
	return (*unknown_00510c9c&(1<<(byte)index))!=0;
}

/* 0x00192ce0: how loud each of the four speakers around is for a direction */
void __attribute__((stdcall)) sound_speakers_get_gains4(
	const real_vector3d *direction,
	real *gains,
	long unknown)
{
	real found[NUMBER_OF_SOUND_SPEAKERS_AROUND];
	long index;

	for (index= 0; index<NUMBER_OF_SOUND_SPEAKERS_AROUND; index++)
	{
		found[index]= unknown_00192b90(index, direction, unknown);
	}
	for (index= 0; index<NUMBER_OF_SOUND_SPEAKERS_AROUND; index++)
	{
		gains[index]= found[index];
	}
}

/* 0x00192d40: how loud each of the five speakers is for a direction */
void __attribute__((stdcall)) sound_speakers_get_gains5(
	real *gains,
	const real_vector3d *direction,
	long unknown)
{
	long index;

	for (index= 0; index<NUMBER_OF_SOUND_SPEAKERS; index++)
	{
		gains[index]= unknown_00192b90(index, direction, unknown);
	}
}
