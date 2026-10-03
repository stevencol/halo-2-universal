/*
MATRIX_MATH.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The functions are those of Halo 1 (math/matrix_math.c of halo-ce-universal)
but for matrix3x3_from_angles, which that game does not have.

A matrix has a scale and the vectors forward, left and up of a rotation; a
matrix4x3 has a position too. A point of the matrix's space is, outside it,
position + scale*(x*forward + y*left + z*up).
*/

#include "real_math.h"

#include <math.h>
#include <string.h>

/* ---------- code */

/* 0x00141590: result can be the matrix */
void matrix4x3_inverse(
	const real_matrix4x3 *matrix,
	real_matrix4x3 *result)
{
	real x, y, z;
	real swap;

	if (matrix->scale==0.0f)
	{
		memset(result, 0, sizeof(*result));
		return;
	}

	x= 0.0f-matrix->position.x;
	y= 0.0f-matrix->position.y;
	z= 0.0f-matrix->position.z;
	if (matrix->scale==1.0f)
	{
		result->scale= 1.0f;
	}
	else
	{
		real inverse_scale= 1.0f/matrix->scale;

		x*= inverse_scale;
		y*= inverse_scale;
		z*= inverse_scale;
		result->scale= inverse_scale;
	}

	/* the rotation transposed */
	result->n[0][0]= matrix->n[0][0];
	result->n[1][1]= matrix->n[1][1];
	result->n[2][2]= matrix->n[2][2];
	swap= matrix->n[0][1];
	result->n[0][1]= matrix->n[1][0];
	result->n[1][0]= swap;
	swap= matrix->n[2][0];
	result->n[2][0]= matrix->n[0][2];
	result->n[0][2]= swap;
	swap= matrix->n[2][1];
	result->n[2][1]= matrix->n[1][2];
	result->n[1][2]= swap;

	result->position.x= result->n[2][0]*z + result->n[0][0]*x + result->n[1][0]*y;
	result->position.y= result->n[2][1]*z + result->n[0][1]*x + result->n[1][1]*y;
	result->position.z= result->n[0][2]*x + result->n[2][2]*z + result->n[1][2]*y;

	return;
}

/* 0x001416c0 */
void matrix4x3_translation(
	real_matrix4x3 *matrix,
	const real_point3d *point)
{
	matrix->scale= 1.0f;
	matrix->forward.i= 1.0f;
	matrix->forward.j= 0.0f;
	matrix->forward.k= 0.0f;
	matrix->left.i= 0.0f;
	matrix->left.j= 1.0f;
	matrix->left.k= 0.0f;
	matrix->up.i= 0.0f;
	matrix->up.j= 0.0f;
	matrix->up.k= 1.0f;
	matrix->position= *point;

	return;
}

/* 0x00141710 */
void matrix4x3_rotation_from_vectors(
	real_matrix4x3 *matrix,
	const real_vector3d *forward,
	const real_vector3d *up)
{
	matrix->scale= 1.0f;
	matrix->forward= *forward;
	/* left is up x forward */
	matrix->left.k= forward->j*up->i - forward->i*up->j;
	matrix->left.j= up->k*forward->i - forward->k*up->i;
	matrix->left.i= forward->k*up->j - up->k*forward->j;
	matrix->up= *up;
	matrix->position.x= 0.0f;
	matrix->position.y= 0.0f;
	matrix->position.z= 0.0f;

	return;
}

/* 0x00141ce0 */
void matrix4x3_rotation_from_angles(
	real_matrix4x3 *matrix,
	real yaw,
	real pitch,
	real roll)
{
	real cosine_roll= cosf(roll);
	real sine_roll= sinf(roll);
	real cosine_pitch= cosf(pitch);
	real sine_pitch= sinf(pitch);
	real cosine_yaw= cosf(yaw);
	real sine_yaw= sinf(yaw);

	matrix->scale= 1.0f;
	matrix->n[0][1]= sine_yaw*cosine_roll - sine_pitch*sine_roll*cosine_yaw;
	matrix->n[0][0]= cosine_yaw*cosine_pitch;
	matrix->n[0][2]= sine_pitch*cosine_roll*cosine_yaw + sine_yaw*sine_roll;
	matrix->n[1][0]= -(sine_yaw*cosine_pitch);
	matrix->n[1][1]= sine_pitch*sine_roll*sine_yaw + cosine_yaw*cosine_roll;
	matrix->n[2][0]= 0.0f-sine_pitch;
	matrix->n[1][2]= cosine_yaw*sine_roll - sine_pitch*cosine_roll*sine_yaw;
	matrix->n[2][1]= 0.0f-cosine_pitch*sine_roll;
	matrix->n[2][2]= cosine_pitch*cosine_roll;
	matrix->position.x= 0.0f;
	matrix->position.y= 0.0f;
	matrix->position.z= 0.0f;

	return;
}

/* 0x001420f0 */
void matrix4x3_from_point_and_vectors(
	real_matrix4x3 *matrix,
	const real_point3d *point,
	const real_vector3d *forward,
	const real_vector3d *up)
{
	matrix4x3_rotation_from_vectors(matrix, forward, up);
	matrix->position= *point;

	return;
}

/* 0x001421f0: the game's function is written with the processor's SSE
instructions. It takes the reciprocal with the processor's estimate and one
step to refine it */
void matrix4x3_from_orientation(
	real_matrix4x3 *matrix,
	const real_orientation *orientation)
{
	const real_quaternion *quaternion= &orientation->rotation;
	real magnitude_squared= (quaternion->v.i*quaternion->v.i + quaternion->v.k*quaternion->v.k) +
		(quaternion->v.j*quaternion->v.j + quaternion->w*quaternion->w);

	if (magnitude_squared>k_real_epsilon)
	{
		real estimate= reciprocal_estimate(magnitude_squared);
		real reciprocal= (estimate+estimate) - magnitude_squared*estimate*estimate;
		real s= reciprocal+reciprocal;
		real is= quaternion->v.i*s;
		real js= quaternion->v.j*s;
		real ks= quaternion->v.k*s;
		real wi= quaternion->w*is;
		real wj= quaternion->w*js;
		real wk= quaternion->w*ks;
		real ii= quaternion->v.i*is;
		real ij= quaternion->v.i*js;
		real ik= quaternion->v.i*ks;
		real jj= quaternion->v.j*js;
		real jk= quaternion->v.j*ks;
		real kk= ks*quaternion->v.k;

		matrix->forward.i= 1.0f-kk-jj;
		matrix->forward.j= ij+wk;
		matrix->forward.k= ik-wj;
		matrix->left.i= ij-wk;
		matrix->left.j= 1.0f-kk-ii;
		matrix->left.k= jk+wi;
		matrix->up.i= ik+wj;
		matrix->up.j= jk-wi;
		matrix->up.k= 1.0f-ii-jj;
	}
	else
	{
		matrix->forward.i= 1.0f;
		matrix->forward.j= 0.0f;
		matrix->forward.k= 0.0f;
		matrix->left.i= 0.0f;
		matrix->left.j= 1.0f;
		matrix->left.k= 0.0f;
		matrix->up.i= 0.0f;
		matrix->up.j= 0.0f;
		matrix->up.k= 1.0f;
	}
	matrix->scale= orientation->scale;
	matrix->position= orientation->translation;

	return;
}

/* 0x001421b0: a matrix of scale 1 that turns as a quaternion and is at a
place. The name is Halo 1's */
void matrix4x3_from_point_and_quaternion(
	real_matrix4x3 *matrix,
	const real_point3d *point,
	const real_quaternion *quaternion)
{
	matrix3x3_rotation_from_quaternion((real_matrix3x3 *)&matrix->forward, quaternion);
	matrix->position.x= 0.0f;
	matrix->position.y= 0.0f;
	matrix->position.z= 0.0f;
	matrix->scale= 1.0f;
	matrix->position= *point;
}

/* 0x00142360: how a matrix turns, where it is and how large it is. The name
is a guess */
void matrix4x3_to_orientation(
	const real_matrix4x3 *matrix,
	real_orientation *orientation)
{
	matrix3x3_rotation_to_quaternion((const real_matrix3x3 *)&matrix->forward, &orientation->rotation);
	orientation->translation= matrix->position;
	orientation->scale= matrix->scale;
}

/* 0x001424f0: the rotation that goes from one matrix to another, as a
vector: its axis by its angle. The name is a guess */
real_vector3d *matrix4x3_rotation_between(
	real_vector3d *vector,
	const real_matrix4x3 *from,
	const real_matrix4x3 *to)
{
	real_matrix4x3 inverse;
	real_matrix4x3 rotation;
	real_quaternion quaternion;
	real angle;

	matrix4x3_inverse(from, &inverse);
	matrix4x3_multiply(to, &inverse, &rotation);
	matrix3x3_rotation_to_quaternion((const real_matrix3x3 *)&rotation.forward, &quaternion);
	quaternion_to_axis_angle(&quaternion, vector, &angle);
	vector->i*= angle;
	vector->j*= angle;
	vector->k*= angle;

	return vector;
}

/* 0x00142390: a matrix whose up is the plane's normal and whose position is
the point of the plane nearest to the origin. As in Halo 1 */
void matrix4x3_from_plane(
	real_matrix4x3 *matrix,
	const real_plane3d *plane)
{
	real_vector3d forward;
	real_point3d position;

	perpendicular3d(&plane->n, &forward);
	normalize3d(&forward);
	position.x= plane->d*plane->n.i;
	position.y= plane->n.j*plane->d;
	position.z= plane->n.k*plane->d;

	matrix->scale= 1.0f;
	matrix->forward= forward;
	/* left is up x forward */
	matrix->left.j= plane->n.k*forward.i - plane->n.i*forward.k;
	matrix->left.k= plane->n.i*forward.j - plane->n.j*forward.i;
	matrix->left.i= plane->n.j*forward.k - plane->n.k*forward.j;
	matrix->up= plane->n;
	matrix->position= position;

	return;
}

/* 0x00142570: result can be the point */
real_point3d *matrix4x3_transform_point(
	const real_matrix4x3 *matrix,
	const real_point3d *point,
	real_point3d *result)
{
	real x= point->x;
	real y= point->y;
	real z= point->z;

	if (matrix->scale!=1.0f)
	{
		x*= matrix->scale;
		y*= matrix->scale;
		z*= matrix->scale;
	}
	result->x= matrix->up.i*z + matrix->left.i*y + matrix->forward.i*x + matrix->position.x;
	result->y= matrix->up.j*z + matrix->left.j*y + matrix->forward.j*x + matrix->position.y;
	result->z= matrix->up.k*z + matrix->left.k*y + matrix->forward.k*x + matrix->position.z;

	return result;
}

/* 0x00142640: result can be the vector */
real_vector3d *matrix4x3_transform_vector(
	const real_matrix4x3 *matrix,
	const real_vector3d *vector,
	real_vector3d *result)
{
	real i= vector->i;
	real j= vector->j;
	real k= vector->k;

	if (matrix->scale!=1.0f)
	{
		i*= matrix->scale;
		j*= matrix->scale;
		k*= matrix->scale;
	}
	result->i= matrix->up.i*k + matrix->left.i*j + matrix->forward.i*i;
	result->j= matrix->up.j*k + matrix->left.j*j + matrix->forward.j*i;
	result->k= matrix->up.k*k + matrix->left.k*j + matrix->forward.k*i;

	return result;
}

/* 0x00142700: result can be the point */
real_point3d *matrix4x3_inverse_transform_point(
	const real_matrix4x3 *matrix,
	const real_point3d *point,
	real_point3d *result)
{
	real x, y, z;

	if (matrix->scale==0.0f)
	{
		result->x= 0.0f;
		result->y= 0.0f;
		result->z= 0.0f;
		return result;
	}

	x= point->x-matrix->position.x;
	y= point->y-matrix->position.y;
	z= point->z-matrix->position.z;
	if (matrix->scale!=1.0f)
	{
		real inverse_scale= 1.0f/matrix->scale;

		x*= inverse_scale;
		y*= inverse_scale;
		z*= inverse_scale;
	}
	result->x= matrix->forward.k*z + matrix->forward.j*y + matrix->forward.i*x;
	result->y= matrix->left.k*z + matrix->left.j*y + matrix->left.i*x;
	result->z= matrix->up.k*z + matrix->up.j*y + matrix->up.i*x;

	return result;
}

/* 0x001427f0: result can be the vector */
real_vector3d *matrix4x3_inverse_transform_vector(
	const real_matrix4x3 *matrix,
	const real_vector3d *vector,
	real_vector3d *result)
{
	real i= vector->i;
	real j= vector->j;
	real k= vector->k;

	if (matrix->scale!=1.0f)
	{
		real inverse_scale= 1.0f/matrix->scale;

		i*= inverse_scale;
		j*= inverse_scale;
		k*= inverse_scale;
	}
	result->i= matrix->forward.k*k + matrix->forward.j*j + matrix->forward.i*i;
	result->j= matrix->left.k*k + matrix->left.j*j + matrix->left.i*i;
	result->k= matrix->up.k*k + matrix->up.j*j + matrix->up.i*i;

	return result;
}

/* 0x001428b0 */
void matrix4x3_inverse_transform_plane(
	const real_matrix4x3 *matrix,
	const real_plane3d *plane,
	real_plane3d *result)
{
	real d= 0.0f;
	real i, j, k;

	if (matrix->scale!=0.0f)
	{
		d= plane->d - (matrix->position.z*plane->n.k + matrix->position.y*plane->n.j +
			matrix->position.x*plane->n.i);
		if (matrix->scale!=1.0f)
		{
			d/= matrix->scale;
		}
	}
	result->d= d;

	i= plane->n.i;
	j= plane->n.j;
	k= plane->n.k;
	if (matrix->scale!=1.0f)
	{
		real inverse_scale= 1.0f/matrix->scale;

		i*= inverse_scale;
		j*= inverse_scale;
		k*= inverse_scale;
	}
	result->n.i= matrix->forward.k*k + matrix->forward.j*j + matrix->forward.i*i;
	result->n.j= matrix->left.k*k + matrix->left.j*j + matrix->left.i*i;
	result->n.k= matrix->up.k*k + matrix->up.j*j + matrix->up.i*i;

	return;
}

/* 0x001429d0: the results can be the points */
void matrix4x3_transform_points(
	const real_matrix4x3 *matrix,
	long count,
	const real_point3d *points,
	real_point3d *results)
{
	real_matrix4x3 m= *matrix;
	long index;
	long row, column;

	/* the rotation with the scale in it */
	for (row= 0; row<3; row++)
	{
		for (column= 0; column<3; column++)
		{
			m.n[row][column]*= m.scale;
		}
	}

	for (index= 0; index<count; index++)
	{
		real x= points[index].x;
		real y= points[index].y;
		real z= points[index].z;

		results[index].x= (x*m.forward.i + y*m.left.i) + (z*m.up.i + m.position.x);
		results[index].y= (x*m.forward.j + y*m.left.j) + (z*m.up.j + m.position.y);
		results[index].z= (x*m.forward.k + y*m.left.k) + (z*m.up.k + m.position.z);
	}

	return;
}

/* 0x00142a60: a point goes through b first, then through a. The result can
be a or b */
void matrix4x3_multiply(
	const real_matrix4x3 *a,
	const real_matrix4x3 *b,
	real_matrix4x3 *result)
{
	real_matrix4x3 m= *a;
	real scale= b->scale;
	long row;

	/* each row of b is read before the row of the result is written */
	{
		real x= b->position.x;
		real y= b->position.y;
		real z= b->position.z;

		result->position.x= (x*m.forward.i + y*m.left.i + z*m.up.i)*m.scale + m.position.x;
		result->position.y= (x*m.forward.j + y*m.left.j + z*m.up.j)*m.scale + m.position.y;
		result->position.z= (x*m.forward.k + y*m.left.k + z*m.up.k)*m.scale + m.position.z;
	}
	for (row= 2; row>=0; row--)
	{
		real i= b->n[row][0];
		real j= b->n[row][1];
		real k= b->n[row][2];

		result->n[row][0]= i*m.forward.i + (j*m.left.i + k*m.up.i);
		result->n[row][1]= i*m.forward.j + (j*m.left.j + k*m.up.j);
		result->n[row][2]= i*m.forward.k + (j*m.left.k + k*m.up.k);
	}
	result->scale= scale*m.scale;

	return;
}

/* 0x00142b80: result can be the matrix */
void matrix3x3_transpose(
	const real_matrix3x3 *matrix,
	real_matrix3x3 *result)
{
	real swap;

	if (matrix==result)
	{
		swap= matrix->n[0][1];
		result->n[0][1]= matrix->n[1][0];
		result->n[1][0]= swap;
		swap= matrix->n[0][2];
		result->n[0][2]= matrix->n[2][0];
		result->n[2][0]= swap;
		swap= matrix->n[1][2];
		result->n[1][2]= matrix->n[2][1];
		result->n[2][1]= swap;
		return;
	}

	result->n[0][0]= matrix->n[0][0];
	result->n[0][1]= matrix->n[1][0];
	result->n[0][2]= matrix->n[2][0];
	result->n[1][0]= matrix->n[0][1];
	result->n[1][1]= matrix->n[1][1];
	result->n[1][2]= matrix->n[2][1];
	result->n[2][0]= matrix->n[0][2];
	result->n[2][1]= matrix->n[1][2];
	result->n[2][2]= matrix->n[2][2];

	return;
}

/* 0x00142bf0: the caller gives the determinant. result can be the matrix */
real_matrix3x3 *matrix3x3_inverse(
	const real_matrix3x3 *matrix,
	real determinant,
	real_matrix3x3 *result)
{
	real_matrix3x3 copy;
	real inverse_determinant;
	short i, j;

	if (matrix==result)
	{
		copy= *matrix;
		matrix= &copy;
	}

	inverse_determinant= 1.0f/determinant;
	for (i= 0; i<3; i++)
	{
		for (j= 0; j<3; j++)
		{
			short i1= i<2 ? i+1 : 0;
			short i2= i<1 ? 2 : i-1;
			short j1= j<2 ? j+1 : 0;
			short j2= j<1 ? 2 : j-1;

			result->n[j][i]= (matrix->n[i1][j1]*matrix->n[i2][j2] - matrix->n[i1][j2]*matrix->n[i2][j1])*
				inverse_determinant;
		}
	}

	return result;
}

/* 0x00142eb0: a vector goes through a first, then through b. The result can
be a or b. As in Halo 1 */
void matrix3x3_multiply(
	const real_matrix3x3 *a,
	const real_matrix3x3 *b,
	real_matrix3x3 *result)
{
	real_matrix3x3 copy;

	if (b==result)
	{
		copy= *b;
		b= &copy;
	}
	if (a==result)
	{
		copy= *a;
		a= &copy;
	}

	result->n[0][0]= b->n[0][0]*a->n[0][0] + b->n[1][0]*a->n[0][1] + a->n[0][2]*b->n[2][0];
	result->n[0][1]= b->n[1][1]*a->n[0][1] + b->n[0][1]*a->n[0][0] + b->n[2][1]*a->n[0][2];
	result->n[0][2]= b->n[1][2]*a->n[0][1] + b->n[0][2]*a->n[0][0] + b->n[2][2]*a->n[0][2];
	result->n[1][0]= b->n[1][0]*a->n[1][1] + a->n[1][0]*b->n[0][0] + a->n[1][2]*b->n[2][0];
	result->n[1][1]= b->n[2][1]*a->n[1][2] + b->n[1][1]*a->n[1][1] + b->n[0][1]*a->n[1][0];
	result->n[1][2]= b->n[2][2]*a->n[1][2] + b->n[1][2]*a->n[1][1] + b->n[0][2]*a->n[1][0];
	result->n[2][0]= b->n[0][0]*a->n[2][0] + b->n[1][0]*a->n[2][1] + b->n[2][0]*a->n[2][2];
	result->n[2][1]= b->n[2][1]*a->n[2][2] + b->n[1][1]*a->n[2][1] + b->n[0][1]*a->n[2][0];
	result->n[2][2]= b->n[2][2]*a->n[2][2] + b->n[1][2]*a->n[2][1] + b->n[0][2]*a->n[2][0];

	return;
}

/* 0x00142d10 */
void matrix3x3_from_forward_and_up(
	real_matrix3x3 *matrix,
	const real_vector3d *forward,
	const real_vector3d *up)
{
	matrix->forward= *forward;
	/* left is up x forward */
	matrix->left.j= forward->i*up->k - up->i*forward->k;
	matrix->left.k= forward->j*up->i - up->j*forward->i;
	matrix->left.i= up->j*forward->k - forward->j*up->k;
	matrix->up= *up;

	return;
}

/* 0x00142da0: not the rotation of matrix4x3_rotation_from_angles */
void matrix3x3_from_angles(
	real_matrix3x3 *matrix,
	real yaw,
	real pitch,
	real roll)
{
	real cosine_roll= cosf(roll);
	real sine_roll= sinf(roll);
	real cosine_pitch= cosf(pitch);
	real sine_pitch= sinf(pitch);
	real cosine_yaw= cosf(yaw);
	real sine_yaw= sinf(yaw);

	matrix->n[0][0]= cosine_yaw*cosine_pitch;
	matrix->n[0][1]= sine_yaw*cosine_pitch;
	matrix->n[1][0]= (0.0f - cosine_yaw*sine_roll*sine_pitch) - sine_yaw*cosine_roll;
	matrix->n[1][1]= cosine_yaw*cosine_roll - sine_yaw*sine_roll*sine_pitch;
	matrix->n[0][2]= sine_pitch;
	matrix->n[1][2]= cosine_pitch*sine_roll;
	matrix->n[2][0]= sine_yaw*sine_roll - cosine_yaw*cosine_roll*sine_pitch;
	matrix->n[2][1]= (0.0f - sine_yaw*cosine_roll*sine_pitch) - cosine_yaw*sine_roll;
	matrix->n[2][2]= cosine_pitch*cosine_roll;

	return;
}

/* 0x00143070: result can be the vector */
void matrix3x3_transform_vector(
	const real_matrix3x3 *matrix,
	const real_vector3d *vector,
	real_vector3d *result)
{
	real_vector3d copy;

	if (vector==result)
	{
		copy= *vector;
		vector= &copy;
	}
	result->i= matrix->up.i*vector->k + matrix->left.i*vector->j + matrix->forward.i*vector->i;
	result->j= matrix->up.j*vector->k + matrix->forward.j*vector->i + matrix->left.j*vector->j;
	result->k= matrix->up.k*vector->k + matrix->forward.k*vector->i + matrix->left.k*vector->j;

	return;
}

/* 0x00141e10 */
void matrix3x3_rotation_from_quaternion(
	real_matrix3x3 *matrix,
	const real_quaternion *quaternion)
{
	real i= quaternion->v.i;
	real j= quaternion->v.j;
	real k= quaternion->v.k;
	real w= quaternion->w;
	real magnitude_squared= i*i + j*j + k*k + w*w;
	real s= magnitude_squared>k_real_epsilon ? 2.0f/magnitude_squared : 0.0f;
	real is= i*s;
	real js= j*s;
	real ks= s*k;
	real wk= ks*w;
	real wi= is*w;
	real wj= js*w;
	real ii= i*is;
	real ij= i*js;
	real ik= i*ks;
	real jj= j*js;
	real jk= j*ks;
	real kk= ks*k;

	matrix->n[0][0]= 1.0f-(kk+jj);
	matrix->n[1][0]= ij-wk;
	matrix->n[0][1]= ij+wk;
	matrix->n[2][0]= ik+wj;
	matrix->n[1][1]= 1.0f-(kk+ii);
	matrix->n[2][1]= jk-wi;
	matrix->n[0][2]= ik-wj;
	matrix->n[1][2]= jk+wi;
	matrix->n[2][2]= 1.0f-(jj+ii);

	return;
}

/* 0x00141f60 */
void matrix3x3_rotation_to_quaternion(
	const real_matrix3x3 *matrix,
	real_quaternion *quaternion)
{
	real *q= &quaternion->v.i;
	real trace= matrix->n[1][1] + matrix->n[0][0] + matrix->n[2][2];
	real s, t;

	if (trace>0.0f)
	{
		s= sqrtf(trace+1.0f)*0.5f;
		t= 0.25f/s;
		quaternion->w= s;
		quaternion->v.i= (matrix->n[1][2]-matrix->n[2][1])*t;
		quaternion->v.j= (matrix->n[2][0]-matrix->n[0][2])*t;
		quaternion->v.k= (matrix->n[0][1]-matrix->n[1][0])*t;
	}
	else
	{
		/* from the greatest element of the diagonal */
		unsigned long i= matrix->n[1][1]>matrix->n[0][0] ? 1 : 0;
		unsigned long j, k;

		if (matrix->n[2][2]>matrix->n[i][i])
		{
			i= 2;
		}
		j= (i+1)%3;
		k= (i+2)%3;

		s= sqrtf(((matrix->n[i][i]-matrix->n[j][j])-matrix->n[k][k])+1.0f)*0.5f;
		t= 0.25f/s;
		q[i]= s;
		quaternion->w= (matrix->n[j][k]-matrix->n[k][j])*t;
		q[j]= (matrix->n[i][j]+matrix->n[j][i])*t;
		q[k]= (matrix->n[i][k]+matrix->n[k][i])*t;
	}

	if (quaternion->w<0.0f)
	{
		quaternion->w= 0.0f-quaternion->w;
		quaternion->v.i= 0.0f-quaternion->v.i;
		quaternion->v.j= 0.0f-quaternion->v.j;
		quaternion->v.k= 0.0f-quaternion->v.k;
	}

	return;
}
