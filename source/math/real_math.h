/*
REAL_MATH.H
*/

#ifndef __REAL_MATH_H
#define __REAL_MATH_H

/* ---------- constants */

/* a real with a magnitude under this one is taken as zero */
#define k_real_epsilon 0.0001f

/* the greatest real */
#define k_real_max 3.4028234663852886e+38f

/* ---------- structures */

typedef float real;

typedef struct real_vector2d
{
	real i, j;
} real_vector2d;

/* in radians */
typedef struct real_euler_angles2d
{
	real yaw, pitch;
} real_euler_angles2d;

typedef struct real_euler_angles3d
{
	real yaw, pitch, roll;
} real_euler_angles3d;

typedef struct real_point2d
{
	real x, y;
} real_point2d;

/* its left, right, top and bottom */
typedef struct real_rectangle2d
{
	real x0, x1, y0, y1;
} real_rectangle2d;

typedef struct real_rgb_color
{
	real red, green, blue;
} real_rgb_color;

typedef struct real_argb_color
{
	real alpha;
	real_rgb_color rgb;
} real_argb_color;

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

typedef struct real_quaternion
{
	real_vector3d v;
	real w;
} real_quaternion;

typedef struct real_orientation
{
	real_quaternion rotation;
	real_point3d translation;
	real scale;
} real_orientation;

typedef struct real_matrix3x3
{
	union
	{
		real n[3][3];
		struct
		{
			real_vector3d forward;
			real_vector3d left;
			real_vector3d up;
		};
	};
} real_matrix3x3;

typedef struct real_matrix4x3
{
	real scale;
	union
	{
		real n[4][3];
		struct
		{
			real_vector3d forward;
			real_vector3d left;
			real_vector3d up;
			real_point3d position;
		};
	};
} real_matrix4x3;

typedef char verify_real_orientation_size[
	sizeof(real_orientation) == 0x20 ? 1 : -1];
typedef char verify_real_matrix3x3_size[
	sizeof(real_matrix3x3) == 0x24 ? 1 : -1];
typedef char verify_real_matrix4x3_size[
	sizeof(real_matrix4x3) == 0x34 ? 1 : -1];

/* ---------- globals */

extern const real_argb_color *global_real_argb_white;

/* ---------- prototypes/REAL_MATH.C */

real normalize2d(real_vector2d *vector);
real normalize3d(real_vector3d *vector);
real normalize3d_with_default(real_vector3d *vector, const real_vector3d *default_vector);
real fast_vector_sphere_intersection(const real_point3d *point, const real_vector3d *vector,
	const real_point3d *center, real radius);
real magnitude3d(const real_vector3d *vector);
real distance3d(const real_point3d *a, const real_point3d *b);
void perpendicular3d(const real_vector3d *vector, real_vector3d *result);
short projection_from_vector3d(const real_vector3d *vector);
unsigned char fast_vector_intersects_sphere(const real_point3d *point, const real_vector3d *vector,
	const real_point3d *center, real radius);
void fast_quaternion_normalize(real_quaternion *quaternion);
unsigned char limit3d(real_vector3d *vector, real length);
void euler_angles2d_from_vector3d(real_euler_angles2d *angles, const real_vector3d *vector);
void quaternion_normalize(real_quaternion *quaternion);
void quaternion_multiply(const real_quaternion *a, const real_quaternion *b, real_quaternion *result);
void quaternion_from_vectors3d(real_quaternion *result, const real_vector3d *a, const real_vector3d *b);
void fast_quaternion_interpolate_and_normalize(const real_quaternion *a, const real_quaternion *b, real t,
	real_quaternion *result);
long rectangle3d_build_vertices(const real_rectangle3d *bounds, real_point3d *vertices);
void vectors3d_from_euler_angles3d(real_vector3d *forward, real_vector3d *up, const real_euler_angles3d *angles);
void vectors3d_from_euler_angles2d(real_vector3d *forward, real_vector3d *up, const real_euler_angles2d *angles);
void project_point3d(const real_point3d *point3d, short projection, unsigned char sign, real_point2d *point2d);
void project_point2d(const real_point2d *point2d, const real_plane3d *plane, short projection,
	unsigned char sign, real_point3d *point3d);

/* ---------- prototypes/RANDOM_MATH.C */

real real_seed_random(unsigned long *seed);
real real_seed_random_range(unsigned long *seed, real lower_bound, real upper_bound);

/* ---------- public code */

/* the processor's estimate of 1/value, which the game's SSE code refines.
Where the processor has no such instruction, the quotient itself, which
differs from the estimate in its last bits */
#ifdef __SSE__
#include <xmmintrin.h>

static inline real reciprocal_estimate(
	real value)
{
	return _mm_cvtss_f32(_mm_rcp_ss(_mm_set_ss(value)));
}

/* the processor's estimate of 1/sqrt(value) */
static inline real reciprocal_square_root_estimate(
	real value)
{
	return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(value)));
}
#else
static inline real reciprocal_estimate(
	real value)
{
	return 1.0f/value;
}

static inline real reciprocal_square_root_estimate(
	real value)
{
	return 1.0f/__builtin_sqrtf(value);
}
#endif

/* ---------- prototypes/MATRIX_MATH.C */

void matrix4x3_inverse(const real_matrix4x3 *matrix, real_matrix4x3 *result);
void matrix4x3_translation(real_matrix4x3 *matrix, const real_point3d *point);
void matrix4x3_rotation_from_vectors(real_matrix4x3 *matrix, const real_vector3d *forward, const real_vector3d *up);
void matrix4x3_rotation_from_angles(real_matrix4x3 *matrix, real yaw, real pitch, real roll);
void matrix4x3_from_point_and_vectors(real_matrix4x3 *matrix, const real_point3d *point,
	const real_vector3d *forward, const real_vector3d *up);
void matrix4x3_from_orientation(real_matrix4x3 *matrix, const real_orientation *orientation);
real_point3d *matrix4x3_transform_point(const real_matrix4x3 *matrix, const real_point3d *point,
	real_point3d *result);
real_vector3d *matrix4x3_transform_vector(const real_matrix4x3 *matrix, const real_vector3d *vector,
	real_vector3d *result);
real_point3d *matrix4x3_inverse_transform_point(const real_matrix4x3 *matrix, const real_point3d *point,
	real_point3d *result);
real_vector3d *matrix4x3_inverse_transform_vector(const real_matrix4x3 *matrix, const real_vector3d *vector,
	real_vector3d *result);
void matrix4x3_inverse_transform_plane(const real_matrix4x3 *matrix, const real_plane3d *plane,
	real_plane3d *result);
void matrix4x3_transform_points(const real_matrix4x3 *matrix, long count, const real_point3d *points,
	real_point3d *results);
void matrix4x3_multiply(const real_matrix4x3 *a, const real_matrix4x3 *b, real_matrix4x3 *result);

void matrix4x3_from_plane(real_matrix4x3 *matrix, const real_plane3d *plane);
void matrix3x3_multiply(const real_matrix3x3 *a, const real_matrix3x3 *b, real_matrix3x3 *result);
void matrix3x3_transpose(const real_matrix3x3 *matrix, real_matrix3x3 *result);
real_matrix3x3 *matrix3x3_inverse(const real_matrix3x3 *matrix, real determinant, real_matrix3x3 *result);
void matrix3x3_from_forward_and_up(real_matrix3x3 *matrix, const real_vector3d *forward, const real_vector3d *up);
void matrix3x3_from_angles(real_matrix3x3 *matrix, real yaw, real pitch, real roll);
void matrix3x3_transform_vector(const real_matrix3x3 *matrix, const real_vector3d *vector, real_vector3d *result);
void matrix3x3_rotation_from_quaternion(real_matrix3x3 *matrix, const real_quaternion *quaternion);
void matrix3x3_rotation_to_quaternion(const real_matrix3x3 *matrix, real_quaternion *quaternion);

#endif
