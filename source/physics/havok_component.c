/*
HAVOK_COMPONENT.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

What an object has of Havok, the physics library. The names of the file and
of its functions are guesses.
*/

#include "havok_component.h"

/* ---------- prototypes of functions that are not decompiled */

/* 0x002da5d0: transform= transform*other. Havok's (hkTransform::setMulEq);
the name is a guess */
void __attribute__((stdcall)) hkTransform_setMulEq(struct havok_transform *transform,
	const struct havok_transform *other);

/* ---------- globals */

/* 0x0051e9b8: of struct havok_component, "havok components". The code that
makes it is not decompiled */
struct data_array *havok_component_data;

/* ---------- code */

/* 0x001d0870: where a rigid body of a havok component is */
void havok_component_get_rigid_body_position(
	const struct havok_component *component,
	long rigid_body_index,
	real_point3d *position)
{
	if (TEST_FLAG(component->flags, _havok_component_transformed_bit))
	{
		real_matrix4x3 matrix;

		havok_component_get_rigid_body_matrix(component, rigid_body_index, &matrix);
		*position= matrix.position;
	}
	else
	{
		const struct havok_transform *transform= &component->rigid_bodies[rigid_body_index].rigid_body->motion->transform;

		position->x= transform->translation[0];
		position->y= transform->translation[1];
		position->z= transform->translation[2];
	}
}

/* 0x001d08e0: where a rigid body of a havok component is and how it is
turned, as a matrix of scale 1 */
void havok_component_get_rigid_body_matrix(
	const struct havok_component *component,
	long rigid_body_index,
	real_matrix4x3 *matrix)
{
	struct havok_transform transform= component->rigid_bodies[rigid_body_index].rigid_body->motion->transform;

	if (TEST_FLAG(component->flags, _havok_component_transformed_bit))
	{
		hkTransform_setMulEq(&transform, &component->transform);
	}
	matrix->scale= 1.0f;
	matrix->forward.i= transform.rotation[0][0];
	matrix->forward.j= transform.rotation[0][1];
	matrix->forward.k= transform.rotation[0][2];
	matrix->left.i= transform.rotation[1][0];
	matrix->left.j= transform.rotation[1][1];
	matrix->left.k= transform.rotation[1][2];
	matrix->up.i= transform.rotation[2][0];
	matrix->up.j= transform.rotation[2][1];
	matrix->up.k= transform.rotation[2][2];
	matrix->position.x= transform.translation[0];
	matrix->position.y= transform.translation[1];
	matrix->position.z= transform.translation[2];
}

/* 0x001d09d0: how fast a rigid body of a havok component moves; 0 if it is
fixed */
void havok_component_get_rigid_body_linear_velocity(
	const struct havok_component *component,
	long rigid_body_index,
	real_vector3d *velocity)
{
	const struct havok_rigid_body *rigid_body= component->rigid_bodies[rigid_body_index].rigid_body;

	if (rigid_body->fixed)
	{
		*velocity= *global_zero_vector3d;
	}
	else
	{
		*velocity= rigid_body->motion->linear_velocity;
	}
}

/* 0x001d0ad0: how fast a rigid body of a havok component turns; 0 if it is
fixed */
void havok_component_get_rigid_body_angular_velocity(
	const struct havok_component *component,
	long rigid_body_index,
	real_vector3d *velocity)
{
	const struct havok_rigid_body *rigid_body= component->rigid_bodies[rigid_body_index].rigid_body;

	if (rigid_body->fixed)
	{
		*velocity= *global_zero_vector3d;
	}
	else
	{
		*velocity= rigid_body->motion->angular_velocity;
	}
}

/* 0x001d0b20: how a rigid body of a havok component is turned */
void havok_component_get_rigid_body_rotation(
	const struct havok_component *component,
	long rigid_body_index,
	real_matrix3x3 *rotation)
{
	struct havok_motion *motion= component->rigid_bodies[rigid_body_index].rigid_body->motion;
	struct havok_matrix3 matrix;

	motion->vtable->get_rotation(motion, &matrix);
	rotation->forward.i= matrix.columns[0][0];
	rotation->forward.j= matrix.columns[0][1];
	rotation->forward.k= matrix.columns[0][2];
	rotation->left.i= matrix.columns[1][0];
	rotation->left.j= matrix.columns[1][1];
	rotation->left.k= matrix.columns[1][2];
	rotation->up.i= matrix.columns[2][0];
	rotation->up.j= matrix.columns[2][1];
	rotation->up.k= matrix.columns[2][2];
}

/* 0x001d0bb0: the mass of a rigid body of a havok component, but never less
than 1; 1 for a body that nothing moves */
real havok_component_get_rigid_body_mass(
	const struct havok_component *component,
	long rigid_body_index)
{
	const struct havok_motion *motion= component->rigid_bodies[rigid_body_index].rigid_body->motion;
	real mass= motion->inverse_mass!=0.0f ? 1.0f/motion->inverse_mass : 0.0f;

	return 1.0f>mass ? 1.0f : mass;
}

/* 0x001d0c00: whether a rigid body of a havok component is keyframed: moved
by the game, not by the physics */
boolean havok_component_rigid_body_is_keyframed(
	const struct havok_component *component,
	long rigid_body_index)
{
	struct havok_motion *motion= component->rigid_bodies[rigid_body_index].rigid_body->motion;

	return motion->vtable->get_type(motion)==_havok_motion_keyframed;
}
