/*
REAL_MATH.H
*/

#ifndef __REAL_MATH_H
#define __REAL_MATH_H

/* ---------- constants */

/* a real with a magnitude under this one is taken as zero */
#define k_real_epsilon 0.0001f

/* ---------- structures */

typedef float real;

typedef struct real_point3d
{
	real x, y, z;
} real_point3d;

typedef struct real_vector3d
{
	real i, j, k;
} real_vector3d;

typedef struct real_plane3d
{
	real_vector3d n;
	real d;
} real_plane3d;

typedef struct real_rectangle3d
{
	real x0, x1, y0, y1, z0, z1;
} real_rectangle3d;

/* ---------- prototypes/REAL_MATH.C */

real normalize3d(real_vector3d *vector);

#endif
