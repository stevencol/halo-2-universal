/*
SOUND_SPEAKERS.H

How loud each speaker is for a sound that comes from a direction. Worked out
from halo2ship.exe; the name of the file is a guess, and so are the names of
its functions.
*/

#ifndef __SOUND_SPEAKERS_H
#define __SOUND_SPEAKERS_H

#include "cseries/cseries.h"
#include "math/real_math.h"

/* ---------- constants */

enum
{
	/* the speakers: four around, and one more */
	NUMBER_OF_SOUND_SPEAKERS_AROUND= 4,
	NUMBER_OF_SOUND_SPEAKERS= 5
};

/* ---------- prototypes/SOUND_SPEAKERS.C */

boolean __attribute__((stdcall)) sound_speakers_unknown_00192950(long index);
void __attribute__((stdcall)) sound_speakers_get_gains4(const real_vector3d *direction, real *gains, long unknown);
void __attribute__((stdcall)) sound_speakers_get_gains5(real *gains, const real_vector3d *direction, long unknown);
real __attribute__((stdcall)) sound_speakers_distance_gain(real minimum_distance, real maximum_distance, real distance);

/* ---------- globals */

extern word *unknown_00510c9c;

#endif
