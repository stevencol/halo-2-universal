/*
REAL_MATH.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.
*/

#include "real_math.h"

#include <math.h>

/* ---------- code */

/* 0x0002b400: as normalize3d */
real normalize2d(
	real_vector2d *vector)
{
	real magnitude= sqrtf(vector->i*vector->i + vector->j*vector->j);
	real scale;

	if (k_real_epsilon>fabsf(magnitude))
	{
		return 0.0f;
	}

	scale= 1.0f/magnitude;
	vector->i*= scale;
	vector->j*= scale;

	return magnitude;
}

/* 0x00024590: the game's function uses the x87 instructions, which keep a
sum of squares too small or too large for a real. For a vector shorter than
about 1e-19 or longer than about 1e19 this one gives another result; so does
distance3d */
real magnitude3d(
	const real_vector3d *vector)
{
	return sqrtf(vector->i*vector->i + vector->j*vector->j + vector->k*vector->k);
}

/* 0x0003ea30 */
real distance3d(
	const real_point3d *a,
	const real_point3d *b)
{
	real x= a->x-b->x;
	real y= a->y-b->y;
	real z= a->z-b->z;

	return sqrtf(x*x + y*y + z*z);
}

/* 0x00120850: the axis where the vector is longest: 0, 1 or 2. As in Halo 1 */
short projection_from_vector3d(
	const real_vector3d *vector)
{
	real i= fabsf(vector->i);
	real j= fabsf(vector->j);
	real k= fabsf(vector->k);

	if (!(k<j) && !(k<i))
	{
		return 2;
	}
	else if (!(j<i))
	{
		return 1;
	}

	return 0;
}

/* 0x0011e5e0: whether the segment from the point along the vector touches
the sphere. As in Halo 1 */
unsigned char fast_vector_intersects_sphere(
	const real_point3d *point,
	const real_vector3d *vector,
	const real_point3d *center,
	real radius)
{
	real y= point->y-center->y;
	real z= point->z-center->z;
	real x= point->x-center->x;
	real c= (z*z + y*y + x*x) - radius*radius;
	real a, b, discriminant, negative_a_minus_b;

	if (0.0f>c)
	{
		/* the point is in the sphere */
		return 1;
	}

	b= vector->k*z + vector->j*y + vector->i*x;
	if (b>=0.0f)
	{
		/* the vector goes away from the sphere */
		return 0;
	}

	a= vector->k*vector->k + vector->j*vector->j + vector->i*vector->i;
	discriminant= b*b - a*c;
	if (0.0f>=discriminant)
	{
		return 0;
	}

	negative_a_minus_b= (0.0f-a)-b;
	if (0.0f>negative_a_minus_b)
	{
		return 1;
	}

	return discriminant>negative_a_minus_b*negative_a_minus_b;
}

/* 0x001d8d60: written with the processor's SSE instructions in the game,
which keeps the quaternion at an address that is a multiple of 16. It takes
the reciprocal of the square root with the processor's estimate and one step
to refine it. The name is a guess */
void fast_quaternion_normalize(
	real_quaternion *quaternion)
{
	real magnitude_squared= (quaternion->v.k*quaternion->v.k + quaternion->v.i*quaternion->v.i) +
		(quaternion->w*quaternion->w + quaternion->v.j*quaternion->v.j);
	real estimate= reciprocal_square_root_estimate(magnitude_squared);
	real scale= 0.5f*estimate*(3.0f - magnitude_squared*estimate*estimate);

	quaternion->v.i= scale*quaternion->v.i;
	quaternion->v.j= scale*quaternion->v.j;
	quaternion->v.k= scale*quaternion->v.k;
	quaternion->w= scale*quaternion->w;

	return;
}

/* 0x0011d000: a vector at a right angle to one, in the plane of the two
axes where the vector is longest */
void perpendicular3d(
	const real_vector3d *vector,
	real_vector3d *result)
{
	real i= fabsf(vector->i);
	real j= fabsf(vector->j);
	real k= fabsf(vector->k);

	if (!(j<i) && !(k<i))
	{
		result->i= 0.0f;
		result->j= vector->k;
		result->k= 0.0f-vector->j;
	}
	else if (!(k<j))
	{
		result->i= 0.0f-vector->k;
		result->j= 0.0f;
		result->k= vector->i;
	}
	else
	{
		result->i= vector->j;
		result->j= 0.0f-vector->i;
		result->k= 0.0f;
	}

	return;
}

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
