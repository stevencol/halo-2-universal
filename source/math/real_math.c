/*
REAL_MATH.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.
*/

#include "real_math.h"

#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

/* ---------- globals */

static const real_argb_color real_argb_white= { 1.0f, { 1.0f, 1.0f, 1.0f } };

/* 0x004686cc: white that hides all that is behind it. The name is Halo 1's */
const real_argb_color *global_real_argb_white= &real_argb_white;

/* 0x005476c4 and 0x005476c8: pi and twice it, as the game has them; they
are not in the executable, and are set when the game starts. The names are
guesses */
real real_math_pi;
real real_math_two_pi;

static const real_vector3d zero_vector3d= { 0.0f, 0.0f, 0.0f };

/* 0x004687a4: the vector (0, 0, 0). The name is Halo 1's */
const real_vector3d *global_zero_vector3d= &zero_vector3d;

static const real_vector3d forward3d= { 1.0f, 0.0f, 0.0f };

/* 0x004687a8: the vector forward, (1, 0, 0). The name is Halo 1's */
const real_vector3d *global_forward3d= &forward3d;

static const real_vector3d up3d= { 0.0f, 0.0f, 1.0f };
static const real_vector3d down3d= { 0.0f, 0.0f, -1.0f };

/* 0x004687b0 and 0x004687bc: the vectors up and down. The names are Halo
1's */
const real_vector3d *global_up3d= &up3d;
const real_vector3d *global_down3d= &down3d;

static const real_quaternion identity_quaternion= { { 0.0f, 0.0f, 0.0f }, 1.0f };
static const real_matrix4x3 identity4x3=
{
	1.0f,
	{ 1.0f, 0.0f, 0.0f },
	{ 0.0f, 1.0f, 0.0f },
	{ 0.0f, 0.0f, 1.0f },
	{ 0.0f, 0.0f, 0.0f }
};

/* 0x004687cc and 0x004687d0: the quaternion that does not turn and the
matrix that changes nothing. The names are Halo 1's */
const real_quaternion *global_identity_quaternion= &identity_quaternion;
const real_matrix4x3 *global_identity4x3= &identity4x3;

/* the corners and the edges of a box */
#define RECTANGLE3D_VERTEX_COUNT 8
#define RECTANGLE3D_EDGE_COUNT 12

/* what undoes an orientation of scale 0 */
#define ORIENTATION_INVERSE_OF_NO_SCALE 10000.0f

/* ---------- code */

/* the bits of a real */
static inline __attribute__((always_inline)) uint32_t real_bits(
	real value)
{
	uint32_t bits;

	memcpy(&bits, &value, sizeof(bits));

	return bits;
}

/* 0x0011f470: a real from a range as a byte, from 0 for the least to 255 for
the most: the greatest whose real is no more than the real. 0 for a range of
nothing. The name is a guess */
unsigned char __attribute__((stdcall)) quantize_real_to_byte(
	real minimum,
	real maximum,
	real value)
{
	real range= maximum-minimum;
	unsigned char quantized;

	if (!(range>k_real_epsilon))
	{
		return 0;
	}
	quantized= (unsigned char)(long)((value-minimum)/range*255.0f);
	while (quantized)
	{
		real decoded= quantized==UCHAR_MAX ? maximum : (real)quantized*(1.0f/255.0f)*range + minimum;

		if (!(decoded>value))
		{
			break;
		}
		quantized--;
	}

	return quantized;
}

/* 0x0011f4f0: a point in a box as three integers of some bits, each from 0
for the least of its bounds to all the bits for the most, the nearest; a
point out of the box is as the nearest point in it. The name is a guess */
void quantize_real_point3d(
	const real_point3d *point,
	const real_bounds *bounds,
	long bits,
	long *quantized)
{
	real maximum= (real)((1<<bits)-1);
	const real *values= &point->x;
	long axis;

	for (axis= 0; axis<3; axis++)
	{
		real value;

		if (bounds[axis].lower>values[axis])
		{
			value= bounds[axis].lower;
		}
		else if (values[axis]>bounds[axis].upper)
		{
			value= bounds[axis].upper;
		}
		else
		{
			value= values[axis];
		}
		quantized[axis]= lrintf((value-bounds[axis].lower)/((bounds[axis].upper-bounds[axis].lower)/maximum));
	}
}

/* 0x0011f580: the point of three integers of some bits in a box: the least
of the bounds for 0, the most for all the bits or more, and between them
the mean that they weigh. The name is a guess */
void dequantize_real_point3d(
	const long *quantized,
	const real_bounds *bounds,
	long bits,
	real_point3d *point)
{
	long maximum= (1<<bits)-1;
	real *values= &point->x;
	long axis;

	for (axis= 0; axis<3; axis++)
	{
		long value= quantized[axis];

		if (value==0)
		{
			values[axis]= bounds[axis].lower;
		}
		else if (value>=maximum)
		{
			values[axis]= bounds[axis].upper;
		}
		else
		{
			values[axis]= ((real)(maximum-value)*bounds[axis].lower + (real)value*bounds[axis].upper)/(real)maximum;
		}
	}
}

/* 0x0011e000: how far a point is from a segment, squared: the segment goes
from a place along a vector. A segment whose length squared is no more than
k_real_epsilon is its start. The name is a guess */
real point_to_segment_distance_squared3d(
	const real_vector3d *direction,
	const real_point3d *start,
	const real_point3d *point)
{
	real j= direction->j;
	real k= direction->k;
	real length_squared= j*j + k*k + direction->i*direction->i;

	if (length_squared>k_real_epsilon)
	{
		real x= point->x-start->x;
		real y= point->y-start->y;
		real z= point->z-start->z;
		real t= (direction->i*x + z*k + y*j)/length_squared;
		real minus_t;

		if (0.0f>t)
		{
			t= 0.0f;
		}
		else if (t>1.0f)
		{
			t= 1.0f;
		}
		minus_t= 0.0f-t;
		x= direction->i*minus_t + x;
		y= minus_t*direction->j + y;
		z= minus_t*direction->k + z;

		return z*z + y*y + x*x;
	}
	else
	{
		real x= start->x-point->x;
		real y= start->y-point->y;
		real z= start->z-point->z;

		return z*z + y*y + x*x;
	}
}

/* 0x0011d3b0: with two vectors of length 1 and an angle, given by its sine
and its cosine: if they are no farther apart than the angle, the first, and
0; if not, the second turned by the angle around the line at right
angles to both, and 1. For two that are on one line, the line is any at
right angles to the second. The name is a guess */
unsigned char __attribute__((stdcall)) limit_angle_between_vectors3d(
	const real_vector3d *a,
	const real_vector3d *b,
	real_vector3d *result,
	real sine,
	real cosine)
{
	real_vector3d axis;
	real dot= a->j*b->j + a->i*b->i + b->k*a->k;

	if (dot>=cosine)
	{
		*result= *a;

		return 0;
	}

	axis.i= a->k*b->j - a->j*b->k;
	axis.j= a->i*b->k - b->i*a->k;
	axis.k= a->j*b->i - a->i*b->j;
	if (normalize3d(&axis)==0.0f)
	{
		perpendicular3d(b, &axis);
		normalize3d(&axis);
	}

	*result= *b;
	{
		real projection= (result->k*axis.k + result->j*axis.j + result->i*axis.i)*(1.0f-cosine);
		real cross_j= result->k*axis.i - result->i*axis.k;
		real cross_k= result->i*axis.j - result->j*axis.i;

		result->i= result->i*cosine + projection*axis.i - (result->j*axis.k - result->k*axis.j)*sine;
		result->j= result->j*cosine + projection*axis.j - cross_j*sine;
		result->k= result->k*cosine + projection*axis.k - cross_k*sine;
	}

	return 1;
}

/* 0x0011dbb0: the orientation that is one after another: b, and then a.
The rotations are multiplied, the translation of b is turned and scaled by a
and moved by a's, and the scales are multiplied. The result can be either of
the two; then that one is copied first (if both are, both go to the same
copy, which is the same). The name is a guess */
void __attribute__((stdcall)) orientation_multiply(
	real_orientation *result,
	const real_orientation *a,
	const real_orientation *b)
{
	real_orientation copy;
	real dot;
	real twice_w;
	real w_squared;
	real cross_i;
	real cross_j;
	real cross_k;

	if (a==result)
	{
		copy= *a;
		a= &copy;
	}
	if (b==result)
	{
		copy= *b;
		b= &copy;
	}
	quaternion_multiply(&a->rotation, &b->rotation, &result->rotation);

	/* b's translation turned by a's rotation */
	dot= (a->rotation.v.j*b->translation.y + b->translation.x*a->rotation.v.i +
		b->translation.z*a->rotation.v.k)*2.0f;
	twice_w= a->rotation.w*2.0f;
	w_squared= a->rotation.w*a->rotation.w*2.0f - 1.0f;
	cross_j= b->translation.x*a->rotation.v.k - a->rotation.v.i*b->translation.z;
	cross_k= a->rotation.v.i*b->translation.y - a->rotation.v.j*b->translation.x;
	cross_i= a->rotation.v.j*b->translation.z - b->translation.y*a->rotation.v.k;
	result->translation.x= cross_i*twice_w + b->translation.x*w_squared + a->rotation.v.i*dot;
	result->translation.y= a->rotation.v.j*dot + cross_j*twice_w + w_squared*b->translation.y;
	result->translation.z= b->translation.z*w_squared + cross_k*twice_w + dot*a->rotation.v.k;

	if (a->scale!=1.0f)
	{
		result->translation.x= result->translation.x*a->scale;
		result->translation.y= result->translation.y*a->scale;
		result->translation.z= result->translation.z*a->scale;
		result->scale= b->scale*a->scale;
	}
	else
	{
		result->scale= b->scale;
	}
	result->translation.x= a->translation.x+result->translation.x;
	result->translation.y= a->translation.y+result->translation.y;
	result->translation.z= a->translation.z+result->translation.z;
}

/* 0x0011da10: the quaternion that is a part of the way from one to another,
along the sphere: none of it at 0 and all of it at 1. Quaternions that are
more than half a turn apart go the short way. For two that are about the
same (or opposite), the nearest of the two to the part. The name is a guess.

The game keeps the sines and the angle in the x87 between the steps: what it
does with them is done here with long double */
void __attribute__((stdcall)) quaternion_slerp(
	const real_quaternion *to,
	const real_quaternion *from,
	real_quaternion *result,
	real t)
{
	real one_minus_t= 1.0f-t;
	real dot= to->v.k*from->v.k + to->v.j*from->v.j + from->v.i*to->v.i + to->w*from->w;
	real clamped= dot;
	long double angle;
	real angle_real;
	long double sine;
	real to_scale;
	real from_scale;

	if (dot>1.0f)
	{
		clamped= 1.0f;
	}
	else if (-1.0f>dot)
	{
		clamped= -1.0f;
	}
	angle= acosl(clamped);
	angle_real= (real)angle;
	sine= sinl(angle);
	if (!(k_real_epsilon>fabsl(sine)))
	{
		long double inverse_sine= 1.0L/sine;

		to_scale= (real)(sinl((long double)angle_real*t)*inverse_sine);
		from_scale= (real)(sinl((long double)angle_real*one_minus_t)*inverse_sine);
		if (0.0f>clamped)
		{
			to_scale= 0.0f-to_scale;
		}
	}
	else
	{
		real nearest= (real)lrintf(t);

		if (0.0f>nearest)
		{
			to_scale= 0.0f;
			from_scale= 1.0f-to_scale;
		}
		else if (nearest>1.0f)
		{
			to_scale= 1.0f;
			from_scale= 1.0f-to_scale;
		}
		else
		{
			to_scale= (real)lrintf(t);
			from_scale= 1.0f-to_scale;
		}
	}
	result->v.i= from->v.i*from_scale + to->v.i*to_scale;
	result->v.j= to->v.j*to_scale + from_scale*from->v.j;
	result->v.k= to->v.k*to_scale + from_scale*from->v.k;
	result->w= to->w*to_scale + from->w*from_scale;
}

/* 0x0011dd80: the orientation that undoes another: the other way round, from
the other side, and the other size. An orientation of scale 0 is undone with
a scale of 10000. The name is a guess */
void orientation_inverse(
	real_orientation *result,
	const real_orientation *orientation)
{
	real w;
	real dot;
	real w_squared;
	real twice_w;
	real cross_i;
	real cross_j;
	real cross_k;
	real inverse_scale;

	result->rotation= orientation->rotation;
	result->rotation.w*= -1.0f;
	w= result->rotation.w;

	/* the translation turned by the rotation */
	dot= (orientation->translation.x*result->rotation.v.i + orientation->translation.y*result->rotation.v.j +
		result->rotation.v.k*orientation->translation.z)*2.0f;
	w_squared= w*w*2.0f - 1.0f;
	twice_w= w*2.0f;
	cross_j= result->rotation.v.k*orientation->translation.x - result->rotation.v.i*orientation->translation.z;
	cross_k= orientation->translation.y*result->rotation.v.i - orientation->translation.x*result->rotation.v.j;
	cross_i= result->rotation.v.j*orientation->translation.z - orientation->translation.y*result->rotation.v.k;
	result->translation.x= cross_i*twice_w + orientation->translation.x*w_squared + result->rotation.v.i*dot;
	result->translation.y= orientation->translation.y*w_squared + result->rotation.v.j*dot + cross_j*twice_w;
	result->translation.z= result->rotation.v.k*dot + cross_k*twice_w + w_squared*orientation->translation.z;

	if (orientation->scale==0.0f)
	{
		inverse_scale= ORIENTATION_INVERSE_OF_NO_SCALE;
	}
	else if (orientation->scale==1.0f)
	{
		inverse_scale= 1.0f;
	}
	else
	{
		inverse_scale= 1.0f/orientation->scale;
	}
	result->translation.x= inverse_scale*result->translation.x;
	result->translation.y= result->translation.y*inverse_scale;
	result->translation.z= result->translation.z*inverse_scale;
	result->translation.x= result->translation.x*-1.0f;
	result->translation.y= result->translation.y*-1.0f;
	result->scale= inverse_scale;
	result->translation.z= result->translation.z*-1.0f;
}

/* whether two vectors are the same, but for less than k_real_epsilon in each
of their parts */
static inline __attribute__((always_inline)) int vectors3d_close(
	const real_vector3d *a,
	const real_vector3d *b)
{
	return k_real_epsilon>fabsf(a->i-b->i) && k_real_epsilon>fabsf(a->j-b->j) && k_real_epsilon>fabsf(a->k-b->k);
}

/* 0x0011d180: three vectors of length 1 at right angles to each other: forward,
which is a direction (forward if it has no length), left, which is at right
angles to it and to a vector that is about up, and up. If that vector is
on the line of the direction, up is used, or forward if the direction is up
or down. The name is a guess */
void __attribute__((stdcall)) orthonormal_basis3d(
	const real_vector3d *direction,
	const real_vector3d *up_hint,
	real_vector3d *forward,
	real_vector3d *left,
	real_vector3d *up)
{
	real_vector3d i;

	*forward= *direction;
	if (normalize3d(forward)==0.0f)
	{
		*forward= *global_forward3d;
	}

	i.i= up_hint->j*forward->k - up_hint->k*forward->j;
	i.j= forward->i*up_hint->k - up_hint->i*forward->k;
	i.k= up_hint->i*forward->j - up_hint->j*forward->i;
	*left= i;
	if (normalize3d(left)==0.0f)
	{
		real_vector3d hint;

		if (vectors3d_close(forward, global_up3d) || vectors3d_close(forward, global_down3d))
		{
			hint= *global_forward3d;
		}
		else
		{
			hint= *global_up3d;
		}
		i.k= hint.i*forward->j - forward->i*hint.j;
		i.j= forward->i*hint.k - forward->k*hint.i;
		i.i= forward->k*hint.j - hint.k*forward->j;
		*left= i;
		normalize3d(left);
	}

	i.k= left->j*forward->i - left->i*forward->j;
	i.j= forward->k*left->i - forward->i*left->k;
	i.i= forward->j*left->k - left->j*forward->k;
	*up= i;
	normalize3d(up);
}

/* 0x0011cf50: the angle between two vectors of length 1, from 0 to pi: 0 for
two that are the same to the bit. The name is a guess */
real angle_between_vectors3d(
	const real_vector3d *a,
	const real_vector3d *b)
{
	real dot;

	if (real_bits(b->i)==real_bits(a->i) && real_bits(b->j)==real_bits(a->j) && real_bits(b->k)==real_bits(a->k))
	{
		return 0.0f;
	}
	dot= b->k*a->k + b->j*a->j + b->i*a->i;
	if (-1.0f>dot)
	{
		dot= -1.0f;
	}
	else if (dot>1.0f)
	{
		dot= 1.0f;
	}

	return acosf(dot);
}

/* 0x0011d090: a vector that is up for something that faces a way: at right
angles to the way, and to the side, which is level. For a way that is
straight up or down, the side is forward. The name is a guess */
real_vector3d *generate_up_vector3d(
	const real_vector3d *forward,
	real_vector3d *up)
{
	real_vector3d side;
	real i= -forward->i;
	real j= forward->j;
	real magnitude= sqrtf(i*i + j*j);
	real_vector3d result;

	if (k_real_epsilon>fabsf(magnitude) || magnitude==0.0f)
	{
		side= *global_forward3d;
	}
	else
	{
		real scale= 1.0f/magnitude;

		side.i= scale*j;
		side.j= scale*i;
		side.k= scale*0.0f;
	}
	result.k= side.i*forward->j - forward->i*side.j;
	result.j= forward->i*side.k - forward->k*side.i;
	result.i= forward->k*side.j - side.k*forward->j;
	*up= result;

	return up;
}

/* 0x0011d790: the axis of the rotation of a quaternion, of length 1, and
its angle, from 0 to pi: a rotation of more than pi is the other way. A
quaternion of no rotation has an axis of length 0. The name is a guess */
void quaternion_to_axis_angle(
	const real_quaternion *quaternion,
	real_vector3d *axis,
	real *angle)
{
	real w;
	real magnitude;

	*axis= quaternion->v;
	w= quaternion->w;
	magnitude= normalize3d(axis);
	*angle= 2.0f*atan2f(magnitude, w);
	if (*angle>real_math_pi)
	{
		axis->i= 0.0f-axis->i;
		axis->k= 0.0f-axis->k;
		axis->j= 0.0f-axis->j;
		*angle= real_math_two_pi-*angle;
	}
}

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

/* 0x001201a0: as normalize3d, but a vector too short becomes another one.
The name is a guess */
real normalize3d_with_default(
	real_vector3d *vector,
	const real_vector3d *default_vector)
{
	real magnitude= sqrtf(vector->i*vector->i + vector->j*vector->j + vector->k*vector->k);
	real scale;

	if (k_real_epsilon>fabsf(magnitude))
	{
		*vector= *default_vector;
		return 0.0f;
	}

	scale= 1.0f/magnitude;
	vector->i*= scale;
	vector->j*= scale;
	vector->k*= scale;

	return magnitude;
}

/* 0x0011e6f0: how far along the vector, as a fraction of it, the segment
from the point first touches the sphere: 0 for a point in the sphere, and
the greatest real if it does not touch it. The name is a guess */
real fast_vector_sphere_intersection(
	const real_point3d *point,
	const real_vector3d *vector,
	const real_point3d *center,
	real radius)
{
	real y= point->y-center->y;
	real z= point->z-center->z;
	real x= point->x-center->x;
	real c= (z*z + y*y + x*x) - radius*radius;
	real a, b, discriminant;

	if (0.0f>c)
	{
		return 0.0f;
	}

	b= vector->k*z + vector->j*y + vector->i*x;
	if (b>=0.0f)
	{
		return k_real_max;
	}

	a= vector->k*vector->k + vector->j*vector->j + vector->i*vector->i;
	discriminant= b*b - a*c;
	if (0.0f>=discriminant)
	{
		return k_real_max;
	}

	return (-b - sqrtf(discriminant))/a;
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

/* 0x000a75d0: makes a vector longer than a length of that length, and tells
whether it did. As in Halo 1 */
unsigned char limit3d(
	real_vector3d *vector,
	real length)
{
	real magnitude_squared= vector->i*vector->i + vector->j*vector->j + vector->k*vector->k;

	if (magnitude_squared>length*length)
	{
		real scale= length/sqrtf(magnitude_squared);

		vector->i= scale*vector->i;
		vector->j= scale*vector->j;
		vector->k= scale*vector->k;
		return 1;
	}

	return 0;
}

/* 0x0011df30: the yaw and the pitch of a direction. As in Halo 1 */
void euler_angles2d_from_vector3d(
	real_euler_angles2d *angles,
	const real_vector3d *vector)
{
	angles->yaw= atan2f(vector->j, vector->i);
	angles->pitch= atan2f(vector->k, sqrtf(vector->i*vector->i + vector->j*vector->j));

	return;
}

/* 0x0011d610: a quaternion of length zero becomes the one of no rotation.
As in Halo 1 */
void quaternion_normalize(
	real_quaternion *quaternion)
{
	real magnitude_squared= quaternion->v.i*quaternion->v.i + quaternion->v.j*quaternion->v.j +
		quaternion->v.k*quaternion->v.k + quaternion->w*quaternion->w;

	if (magnitude_squared>0.0f)
	{
		real scale= 1.0f/sqrtf(magnitude_squared);

		quaternion->v.i= scale*quaternion->v.i;
		quaternion->v.j= scale*quaternion->v.j;
		quaternion->v.k= scale*quaternion->v.k;
		quaternion->w= scale*quaternion->w;
	}
	else
	{
		quaternion->v.i= 0.0f;
		quaternion->v.j= 0.0f;
		quaternion->v.k= 0.0f;
		quaternion->w= 1.0f;
	}

	return;
}

/* 0x0011d820: the rotation of b, then that of a. The result can be a or b */
void quaternion_multiply(
	const real_quaternion *a,
	const real_quaternion *b,
	real_quaternion *result)
{
	real_quaternion copy;

	if (a==result)
	{
		copy= *a;
		a= &copy;
	}
	if (b==result)
	{
		copy= *b;
		b= &copy;
	}

	result->v.i= (b->w*a->v.i + b->v.k*a->v.j + a->w*b->v.i) - b->v.j*a->v.k;
	result->v.j= (a->v.k*b->v.i + b->w*a->v.j + a->w*b->v.j) - a->v.i*b->v.k;
	result->v.k= (b->w*a->v.k + b->v.j*a->v.i + a->w*b->v.k) - b->v.i*a->v.j;
	result->w= ((b->w*a->w - a->v.i*b->v.i) - b->v.j*a->v.j) - a->v.k*b->v.k;

	return;
}

/* 0x0011d6a0: the rotation that takes a direction to another, both of
length 1. For two opposite directions, half a turn around a perpendicular.
The name is a guess */
void quaternion_from_vectors3d(
	real_quaternion *result,
	const real_vector3d *a,
	const real_vector3d *b)
{
	real s= (a->j*b->j + a->i*b->i + b->k*a->k + 1.0f)*2.0f;

	if (!(s>0.0f))
	{
		s= 0.0f;
	}
	s= sqrtf(s);

	if (s<0.001f)
	{
		perpendicular3d(a, &result->v);
		result->w= 0.0f;
	}
	else
	{
		real reciprocal= 1.0f/s;
		real k= a->i*b->j - a->j*b->i;
		real j= b->i*a->k - b->k*a->i;
		real i= a->j*b->k - a->k*b->j;

		result->v.i= i*reciprocal;
		result->v.j= j*reciprocal;
		result->v.k= k*reciprocal;
		result->w= s*0.5f;
	}

	return;
}

/* 0x0011d950: a quaternion between two, by the short way around, of length
1. As in Halo 1 */
void fast_quaternion_interpolate_and_normalize(
	const real_quaternion *a,
	const real_quaternion *b,
	real t,
	real_quaternion *result)
{
	real dot= b->v.k*a->v.k + b->v.j*a->v.j + a->w*b->w + a->v.i*b->v.i;
	real u= 1.0f-t;

	if (0.0f>dot)
	{
		t= 0.0f-t;
	}
	result->v.i= a->v.i*u + b->v.i*t;
	result->v.j= b->v.j*t + u*a->v.j;
	result->v.k= b->v.k*t + u*a->v.k;
	result->w= a->w*u + t*b->w;
	quaternion_normalize(result);

	return;
}

/* 0x0011f5f0: makes a rectangle as large as it has to be for some points
to be in it. The name is a guess */
void real_rectangle2d_enclose_points(
	real_rectangle2d *bounds,
	long count,
	const real_point2d *points)
{
	long index;

	for (index= 0; index<count; index++)
	{
		const real_point2d *point= &points[index];

		if (bounds->x0>point->x)
		{
			bounds->x0= point->x;
		}
		if (point->x>bounds->x1)
		{
			bounds->x1= point->x;
		}
		if (bounds->y0>point->y)
		{
			bounds->y0= point->y;
		}
		if (point->y>bounds->y1)
		{
			bounds->y1= point->y;
		}
	}
}

/* 0x0011f770: makes a box as large as it has to be for some points to be in
it. The name is a guess */
void real_rectangle3d_enclose_points(
	real_rectangle3d *bounds,
	long count,
	const real_point3d *points)
{
	long index;

	for (index= 0; index<count; index++)
	{
		const real_point3d *point= &points[index];

		if (bounds->x0>point->x)
		{
			bounds->x0= point->x;
		}
		if (point->x>bounds->x1)
		{
			bounds->x1= point->x;
		}
		if (bounds->y0>point->y)
		{
			bounds->y0= point->y;
		}
		if (point->y>bounds->y1)
		{
			bounds->y1= point->y;
		}
		if (bounds->z0>point->z)
		{
			bounds->z0= point->z;
		}
		if (point->z>bounds->z1)
		{
			bounds->z1= point->z;
		}
	}
}

/* 0x0011f9a0: the eight corners of a box, and how many they are */
long rectangle3d_build_vertices(
	const real_rectangle3d *bounds,
	real_point3d *vertices)
{
	long index;

	for (index= 0; index<8; index++)
	{
		vertices[index].x= index&1 ? bounds->x1 : bounds->x0;
		vertices[index].y= index&2 ? bounds->y1 : bounds->y0;
		vertices[index].z= index&4 ? bounds->z1 : bounds->z0;
	}

	return 8;
}

/* 0x0011fa40: the twelve edges of a box, as the two ends of each: those of
the bottom, then those that go up, then those of the top. How many edges.
The name is a guess */
long __attribute__((stdcall)) rectangle3d_build_edges(
	const real_rectangle3d *bounds,
	real_point3d *points)
{
	long ends[RECTANGLE3D_EDGE_COUNT*2]=
	{
		0, 2, 2, 3, 3, 1, 1, 0,
		0, 4, 1, 5, 2, 6, 3, 7,
		4, 5, 5, 7, 7, 6, 6, 4
	};
	real_point3d vertices[RECTANGLE3D_VERTEX_COUNT];
	long index;

	rectangle3d_build_vertices(bounds, vertices);
	for (index= 0; index<RECTANGLE3D_EDGE_COUNT*2; index++)
	{
		points[index]= vertices[ends[index]];
	}

	return RECTANGLE3D_EDGE_COUNT;
}

/* 0x0011df60: as in Halo 1 */
void vectors3d_from_euler_angles3d(
	real_vector3d *forward,
	real_vector3d *up,
	const real_euler_angles3d *angles)
{
	real_matrix4x3 matrix;

	matrix4x3_rotation_from_angles(&matrix, angles->yaw, angles->pitch, angles->roll);
	*forward= matrix.forward;
	*up= matrix.up;

	return;
}

/* 0x0011dfb0 */
void vectors3d_from_euler_angles2d(
	real_vector3d *forward,
	real_vector3d *up,
	const real_euler_angles2d *angles)
{
	real_matrix4x3 matrix;

	matrix4x3_rotation_from_angles(&matrix, angles->yaw, angles->pitch, 0.0f);
	*forward= matrix.forward;
	*up= matrix.up;

	return;
}

/* 0x00440b94: for each projection (the axis that a plane faces most) and
each sign of the plane along it, the two axes of the plane's 2D points and
the axis of the projection */
static const short projection_axes[3][2][3]=
{
	{{2, 1, 0}, {1, 2, 0}},
	{{0, 2, 1}, {2, 0, 1}},
	{{1, 0, 2}, {0, 1, 2}}
};

/* 0x00120810: a point as a 2D point of a plane. As in Halo 1 */
void project_point3d(
	const real_point3d *point3d,
	short projection,
	unsigned char sign,
	real_point2d *point2d)
{
	const short *axes= projection_axes[projection][sign];
	const real *coordinates= &point3d->x;
	real y= coordinates[axes[1]];

	point2d->x= coordinates[axes[0]];
	point2d->y= y;

	return;
}

/* 0x00120790: a 2D point of a plane as a point on the plane. As in Halo 1 */
void project_point2d(
	const real_point2d *point2d,
	const real_plane3d *plane,
	short projection,
	unsigned char sign,
	real_point3d *point3d)
{
	const short *axes= projection_axes[projection][sign];
	const real *normal= &plane->n.i;
	real *coordinates= &point3d->x;

	coordinates[axes[0]]= point2d->x;
	coordinates[axes[1]]= point2d->y;
	if (k_real_epsilon>fabsf(normal[projection]))
	{
		coordinates[projection]= 0.0f;
	}
	else
	{
		coordinates[projection]=
			((plane->d - normal[axes[0]]*point2d->x) - normal[axes[1]]*point2d->y)/normal[projection];
	}

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

/* one of the three numbers of a vector that is kept in 32 bits: from -1, for
0, to 1, for the most that its bits hold */
static inline __attribute__((always_inline)) real vector3d_decompress_component(
	long value,
	long maximum)
{
	if (!value)
	{
		return -1.0f;
	}
	if (value>=maximum)
	{
		return 1.0f;
	}

	return ((real)value-(real)(maximum-value))*(1.0f/(real)maximum);
}

/* 0x00188ea0: a vector that is kept in 32 bits: 11 for i, 11 for j and 10
for k. It is made of length 1. The name is made up, and so is the file: the
function is far from the rest of these */
real_vector3d *vector3d_decompress(
	unsigned long compressed,
	real_vector3d *vector)
{
	vector->i= vector3d_decompress_component(compressed&0x7FF, 0x7FF);
	vector->j= vector3d_decompress_component((compressed>>11)&0x7FF, 0x7FF);
	vector->k= vector3d_decompress_component(compressed>>22, 0x3FF);
	normalize3d(vector);

	return vector;
}

/* a real from -1 to 1 */
static inline __attribute__((always_inline)) real vector3d_compress_pin(
	real value)
{
	if (-1.0f>value)
	{
		return -1.0f;
	}

	return value>1.0f ? 1.0f : value;
}

/* 0x00188dd0: a vector in 32 bits, as vector3d_decompress takes it: 11 bits
for i, 11 for j and 10 for k, each to the nearest of what its bits hold.
What is out of -1 to 1 is taken as the end that it is past. The name is
made up */
unsigned long vector3d_compress(
	const real_vector3d *vector)
{
	real k= vector3d_compress_pin(vector->k);
	real j= vector3d_compress_pin(vector->j);
	real i= vector3d_compress_pin(vector->i);
	long compressed_k= lrintf((k-(-1.0f))*511.5f);
	long compressed_j= lrintf((j-(-1.0f))*1023.5f);
	long compressed_i= lrintf((i-(-1.0f))*1023.5f);

	return (unsigned long)(((compressed_k<<11)|compressed_j)<<11)|(unsigned long)compressed_i;
}
/* 0x00024550: the square of the distance between two points; the game adds
z first */
real distance_squared3d(
	const real_point3d *a,
	const real_point3d *b)
{
	real x= a->x-b->x;
	real y= a->y-b->y;
	real z= a->z-b->z;

	return z*z + x*x + y*y;
}

/* 0x000a7570: whether each coordinate of a point is a number (not infinite
and not NaN) */
unsigned char valid_real_point3d(
	const real_point3d *point)
{
	const real *coordinates= &point->x;
	long index;

	for (index= 0; index<3; index++)
	{
		uint32_t bits;

		memcpy(&bits, &coordinates[index], sizeof(bits));
		if ((bits&0x7F800000)==0x7F800000)
		{
			return 0;
		}
	}

	return 1;
}

/* 0x000461c0: whether a point is no farther from a centre than a radius. As
in Halo 1 */
long __attribute__((stdcall)) point_in_sphere(
	const real_point3d *point,
	const real_point3d *center,
	real radius)
{
	real x= point->x-center->x;
	real z= point->z-center->z;
	real y= point->y-center->y;

	return radius*radius>=x*x + z*z + y*y;
}

/* 0x00048e70: the least whole number that is not below a real, worked out
from its bits (the shift is that of the processor, by the low five bits).
The name is a guess */
long __attribute__((stdcall)) real_ceiling_to_long(
	real value)
{
	long bits;
	long exponent;
	long shift;
	long sign;
	long result;

	memcpy(&bits, &value, sizeof(bits));
	exponent= ((bits>>23)&0xFF)-0x7F;
	shift= (0x96-((bits>>23)&0xFF))&0x1F;
	sign= bits>>31;
	result= ((((bits&0x7FFFFF)|0x800000)>>shift)^sign)-sign;
	result&= ~(exponent>>31);
	if (!sign && bits && (exponent<0 || (((1UL<<shift)-1)&(unsigned long)bits&0x7FFFFF)))
	{
		result++;
	}

	return result;
}

/* 0x000aba40: whether two angles are numbers (not infinite nor NaN by their
bits), the first from 0 to two pi and the second from minus to plus half
pi. The name is a guess, as those of Halo 1 */
unsigned char valid_real_euler_angles2d(
	const real *angles)
{
	unsigned long bits;

	memcpy(&bits, &angles[0], sizeof(bits));
	if ((bits&0x7F800000)==0x7F800000)
	{
		return 0;
	}
	memcpy(&bits, &angles[1], sizeof(bits));
	if ((bits&0x7F800000)==0x7F800000)
	{
		return 0;
	}

	return angles[0]>=0.0f && 6.2831855f>=angles[0] && angles[1]>=-1.5707964f && 1.5707964f>=angles[1];
}

/* 0x00172350: whether the three reals of a point are numbers (not infinite
nor NaN by their bits) from -50000 to 50000. The name is a guess, as those
of Halo 1 */
unsigned char valid_world_real_point3d(
	const real *point)
{
	long index;

	for (index= 0; index<3; index++)
	{
		unsigned long bits;

		memcpy(&bits, &point[index], sizeof(bits));
		if ((bits&0x7F800000)==0x7F800000 || !(point[index]>=-50000.0f) || !(50000.0f>=point[index]))
		{
			return 0;
		}
	}

	return 1;
}

/* 0x0001c1a0: how big a real is, without its sign */
real real_math_unknown_0001c1a0(
	real value)
{
	return fabsf(value);
}

/* 0x0010e8e0: 3t^2-2t^3, from 0 to 1 (among the functions of the units in
the game) */
real real_math_unknown_0010e8e0(
	real t)
{
	real square= t*t;
	real value= square*3.0f-square*t*2.0f;

	if (0.0f>value)
	{
		return 0.0f;
	}
	if (value>1.0f)
	{
		return 1.0f;
	}

	return value;
}

/* 0x000a0200: whether two reals are less than 0.001 apart (and their
difference is a number) */
long __attribute__((stdcall)) real_math_unknown_000a0200(
	real a,
	real b)
{
	real difference= a-b;
	unsigned long bits;

	memcpy(&bits, &difference, sizeof(bits));
	if ((bits&0x7F800000)==0x7F800000)
	{
		return 0;
	}

	return 0.001f>fabsf(difference);
}

/* 0x0004efb0: a vector made one long, unless it is zero; the vector */
real *real_math_unknown_0004efb0(
	real *vector)
{
	real square= vector[0]*vector[0]+vector[1]*vector[1]+vector[2]*vector[2];

	if (square!=0.0f)
	{
		long double scale= 1.0L/__builtin_sqrtl((long double)square);

		vector[0]= (real)(scale*vector[0]);
		vector[1]= (real)(scale*vector[1]);
		vector[2]= (real)(scale*vector[2]);
	}

	return vector;
}

/* 0x000a0190: whether a vector is less than 0.001 from being one long (its
square, and the difference is a number) */
long real_math_unknown_000a0190(
	const real *vector)
{
	real difference= vector[0]*vector[0]+vector[1]*vector[1]+vector[2]*vector[2]-1.0f;
	unsigned long bits;

	memcpy(&bits, &difference, sizeof(bits));
	if ((bits&0x7F800000)==0x7F800000)
	{
		return 0;
	}

	return 0.001f>fabsf(difference);
}

/* 0x000a74c0: whether two vectors are one long each and perpendicular (each
less than 0.001 away) */
long real_math_unknown_000a74c0(
	const real *forward,
	const real *up)
{
	real difference= forward[0]*forward[0]+forward[1]*forward[1]+forward[2]*forward[2]-1.0f;
	unsigned long bits;

	memcpy(&bits, &difference, sizeof(bits));
	if ((bits&0x7F800000)==0x7F800000 || !(0.001f>fabsf(difference)))
	{
		return 0;
	}
	if (!real_math_unknown_000a0190(up))
	{
		return 0;
	}

	return real_math_unknown_000a0200(up[2]*forward[2] + up[1]*forward[1] + forward[0]*up[0], 0.0f) ? 1 : 0;
}

/* 0x0013d9f0: whether a type (of a part of a geometry) is 4 or 5 (among the
functions of the geometry in the game) */
long real_math_unknown_0013d9f0(
	long type)
{
	return type==4 || type==5;
}

/* 0x00131a00: a color from hue, saturation and value (the hue from 0 to 1) */
real_rgb_color *real_math_hsv_to_rgb(
	const real *hsv,
	real_rgb_color *rgb)
{
	real hue= hsv[0]*6.0f;
	long sector;
	real fraction;
	real p;
	real q;
	real t;

	if (hsv[1]==0.0f)
	{
		rgb->blue= hsv[2];
		rgb->green= hsv[2];
		rgb->red= hsv[2];
		return rgb;
	}
	sector= (long)hue;
	if ((real)sector>hue)
	{
		sector--;
	}
	fraction= hue-(real)sector;
	q= (1.0f-hsv[1]*fraction)*hsv[2];
	p= (1.0f-hsv[1])*hsv[2];
	t= (1.0f-(1.0f-fraction)*hsv[1])*hsv[2];
	switch (sector)
	{
	case 0:
		rgb->red= hsv[2];
		rgb->green= t;
		rgb->blue= p;
		break;
	case 1:
		rgb->red= q;
		rgb->green= hsv[2];
		rgb->blue= p;
		break;
	case 2:
		rgb->red= p;
		rgb->green= hsv[2];
		rgb->blue= t;
		break;
	case 3:
		rgb->red= p;
		rgb->green= q;
		rgb->blue= hsv[2];
		break;
	case 4:
		rgb->red= t;
		rgb->green= p;
		rgb->blue= hsv[2];
		break;
	case 5:
		rgb->red= hsv[2];
		rgb->green= p;
		rgb->blue= q;
		break;
	}

	return rgb;
}

/* 0x001318d0: a color as hue, saturation and value, all from 0 to 1, as
Halo 1's real_rgb_color_to_real_hsv_color */
real *real_rgb_color_to_real_hsv_color(
	const real *rgb,
	real *hsv)
{
	real maximum= rgb[0]>(rgb[1]>rgb[2] ? rgb[1] : rgb[2]) ? rgb[0] : rgb[1]>rgb[2] ? rgb[1] : rgb[2];
	real minimum= rgb[0]>(rgb[1]>rgb[2] ? rgb[2] : rgb[1]) ? (rgb[1]>rgb[2] ? rgb[2] : rgb[1]) : rgb[0];
	real delta= maximum-minimum;
	real saturation;

	hsv[2]= maximum;
	saturation= maximum==0.0f ? 0.0f : delta/maximum;
	hsv[1]= saturation;
	if (saturation==0.0f)
	{
		hsv[0]= 0.0f;
		return hsv;
	}
	if (rgb[0]==maximum)
	{
		hsv[0]= (rgb[1]-rgb[2])/delta;
	}
	else if (rgb[1]==maximum)
	{
		hsv[0]= (rgb[2]-rgb[0])/delta + 2.0f;
	}
	else
	{
		hsv[0]= (rgb[0]-rgb[1])/delta + 4.0f;
	}
	hsv[0]*= 1.0f/6.0f;
	if (0.0f>hsv[0])
	{
		hsv[0]+= 1.0f;
	}

	return hsv;
}
