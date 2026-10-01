/*
REAL_MATH.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.
*/

#include "real_math.h"

#include <math.h>

/* ---------- code */

/* 0x00030bf0: makes the vector of length 1 and returns the length it had; a
vector too short stays as it is and 0 is returned. As in Halo 1 */
real normalize3d(
	real_vector3d *vector)
{
	real magnitude= sqrtf(vector->i*vector->i + vector->j*vector->j + vector->k*vector->k);
	real scale;

	if (k_real_epsilon>fabsf(magnitude))
	{
		return 0.0f;
	}

	scale= 1.0f/magnitude;
	vector->i*= scale;
	vector->j*= scale;
	vector->k*= scale;

	return magnitude;
}
