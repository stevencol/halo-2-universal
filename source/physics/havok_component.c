/*
HAVOK_COMPONENT.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

What an object has of Havok, the physics library. The names of the file and
of its functions are guesses.
*/

#include "havok_component.h"

#include "game/game.h"

/* ---------- prototypes of functions that are not decompiled */

/* 0x002da5d0: transform= transform*other. Havok's (hkTransform::setMulEq);
the name is a guess */
void __attribute__((stdcall)) hkTransform_setMulEq(struct havok_transform *transform,
	const struct havok_transform *other);

/* 0x002da3f0: transform= the inverse of other. Havok's
(hkTransform::setInverse); the name is a guess */
void __attribute__((stdcall)) hkTransform_setInverse(struct havok_transform *transform,
	const struct havok_transform *other);

/* 0x001d1260 and 0x001d1540: what is done with a component before and
after a fixed rigid body of it is put somewhere, when it has
_havok_component_unknown5_bit */
void unknown_001d1260(struct havok_component *component);
void unknown_001d1540(struct havok_component *component);

/* 0x002da2d0: the quaternion of a rotation. Havok's (hkQuaternion::set) */
void __attribute__((stdcall)) hkQuaternion_set(struct havok_vector4 *quaternion, const struct havok_matrix3 *rotation);

/* 0x002db880: makes a keyframed body move and turn so that it is in a place
and turned as a quaternion says after a time, given as its inverse. Havok's
(hkKeyFrameUtility::applyHardKeyFrame) */
void hkKeyFrameUtility_applyHardKeyFrame(const struct havok_vector4 *position, const struct havok_vector4 *orientation,
	real inverse_time, struct havok_rigid_body *rigid_body);

/* 0x0030bc40: whether a body of Havok is awake; Havok's (hkEntity::isActive),
which gives where it leaves it */
boolean *__attribute__((stdcall)) hkEntity_isActive(const struct havok_rigid_body *rigid_body, boolean *active);

/* 0x0030bc60: wakes a body of Havok (hkEntity::activate) */
void hkEntity_activate(struct havok_rigid_body *rigid_body);

/* ---------- private code */

/* wakes a body that is in Havok's world and asleep */
static inline __attribute__((always_inline)) void havok_rigid_body_wake(
	struct havok_rigid_body *rigid_body)
{
	boolean active;

	if (!*hkEntity_isActive(rigid_body, &active) && rigid_body->simulation_island)
	{
		hkEntity_activate(rigid_body);
	}
}

/* puts a body of Havok somewhere, waking it; a fixed body in a world of
Havok is not */
static inline __attribute__((always_inline)) void havok_rigid_body_set_transform_inline(
	struct havok_rigid_body *rigid_body,
	const struct havok_transform *transform)
{
	havok_rigid_body_wake(rigid_body);
	if (!rigid_body->fixed || !rigid_body->world)
	{
		rigid_body->motion->vtable->set_transform(rigid_body->motion, transform);
	}
}

/* a matrix as a transform of Havok; its scale is not looked at */
static inline __attribute__((always_inline)) void havok_transform_from_matrix(
	struct havok_transform *transform,
	const real_matrix4x3 *matrix)
{
	transform->rotation[0][0]= matrix->forward.i;
	transform->rotation[0][1]= matrix->forward.j;
	transform->rotation[0][2]= matrix->forward.k;
	transform->rotation[0][3]= 0.0f;
	transform->rotation[1][0]= matrix->left.i;
	transform->rotation[1][1]= matrix->left.j;
	transform->rotation[1][2]= matrix->left.k;
	transform->rotation[1][3]= 0.0f;
	transform->rotation[2][0]= matrix->up.i;
	transform->rotation[2][1]= matrix->up.j;
	transform->rotation[2][2]= matrix->up.k;
	transform->rotation[2][3]= 0.0f;
	transform->translation[0]= matrix->position.x;
	transform->translation[1]= matrix->position.y;
	transform->translation[2]= matrix->position.z;
	transform->translation[3]= 0.0f;
}

/* a transform of the world as one of the rigid bodies of a component, if
theirs are from the component's */
static inline __attribute__((always_inline)) void havok_component_transform_to_rigid_bodies(
	const struct havok_component *component,
	struct havok_transform *transform)
{
	if (TEST_FLAG(component->flags, _havok_component_transformed_bit))
	{
		struct havok_transform inverse;

		hkTransform_setInverse(&inverse, &component->transform);
		hkTransform_setMulEq(transform, &inverse);
	}
}

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

/* 0x001d0a20: how fast a point of the world moves if it moves with a rigid
body of a havok component; 0 if the body is fixed */
void havok_component_get_rigid_body_point_velocity(
	const struct havok_component *component,
	long rigid_body_index,
	const real_point3d *point,
	real_vector3d *velocity)
{
	struct havok_rigid_body *rigid_body= component->rigid_bodies[rigid_body_index].rigid_body;

	if (rigid_body->fixed)
	{
		*velocity= *global_zero_vector3d;
	}
	else
	{
		struct havok_vector4 position;
		struct havok_vector4 result;

		position.v[0]= point->x;
		position.v[1]= point->y;
		position.v[2]= point->z;
		position.v[3]= 0.0f;
		rigid_body->motion->vtable->get_point_velocity(rigid_body->motion, &position, &result);
		velocity->i= result.v[0];
		velocity->j= result.v[1];
		velocity->k= result.v[2];
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

/* 0x001d0dd0: makes a rigid body of a havok component move, waking it; not
if it is fixed */
void havok_component_set_rigid_body_linear_velocity(
	const struct havok_component *component,
	long rigid_body_index,
	const real_vector3d *velocity)
{
	struct havok_rigid_body *rigid_body= component->rigid_bodies[rigid_body_index].rigid_body;

	if (!rigid_body->fixed)
	{
		struct havok_vector4 vector;

		vector.v[0]= velocity->i;
		vector.v[1]= velocity->j;
		vector.v[2]= velocity->k;
		vector.v[3]= 0.0f;
		havok_rigid_body_wake(rigid_body);
		rigid_body->motion->vtable->set_linear_velocity(rigid_body->motion, &vector);
	}
}

/* 0x001d0e50: makes a rigid body of a havok component turn, waking it; not
if it is fixed or the component's bodies are not made to turn */
void havok_component_set_rigid_body_angular_velocity(
	const struct havok_component *component,
	long rigid_body_index,
	const real_vector3d *velocity)
{
	struct havok_rigid_body *rigid_body;
	struct havok_vector4 vector;

	if (TEST_FLAG(component->flags, _havok_component_no_angular_velocity_bit))
	{
		return;
	}
	rigid_body= component->rigid_bodies[rigid_body_index].rigid_body;
	if (rigid_body->fixed)
	{
		return;
	}
	vector.v[0]= velocity->i;
	vector.v[1]= velocity->j;
	vector.v[2]= velocity->k;
	vector.v[3]= 0.0f;
	havok_rigid_body_wake(rigid_body);
	rigid_body->motion->vtable->set_angular_velocity(rigid_body->motion, &vector);
}

/* 0x001d0c20: puts a rigid body of a havok component where a matrix says;
its scale is not looked at */
void havok_component_set_rigid_body_matrix(
	struct havok_component *component,
	long rigid_body_index,
	const real_matrix4x3 *matrix)
{
	struct havok_transform transform;

	havok_transform_from_matrix(&transform, matrix);
	havok_component_set_rigid_body_transform(component, rigid_body_index, &transform);
}

/* 0x001d0cf0: puts a rigid body of a havok component somewhere: from the
component's transform, if the bodies' are */
void havok_component_set_rigid_body_transform(
	struct havok_component *component,
	long rigid_body_index,
	const struct havok_transform *transform)
{
	struct havok_transform result= *transform;
	struct havok_rigid_body *rigid_body;

	havok_component_transform_to_rigid_bodies(component, &result);
	rigid_body= component->rigid_bodies[rigid_body_index].rigid_body;
	if (rigid_body->fixed && TEST_FLAG(component->flags, _havok_component_unknown5_bit))
	{
		unknown_001d1260(component);
		havok_rigid_body_set_transform(component->rigid_bodies[rigid_body_index].rigid_body, &result);
		unknown_001d1540(component);
	}
	else
	{
		havok_rigid_body_set_transform_inline(rigid_body, &result);
	}
}

/* 0x001d8e80: puts a body of Havok somewhere, waking it; a fixed body in a
world of Havok is not. It is far from the other functions: it can be of
another file */
void havok_rigid_body_set_transform(
	struct havok_rigid_body *rigid_body,
	const struct havok_transform *transform)
{
	havok_rigid_body_set_transform_inline(rigid_body, transform);
}

/* 0x001d0ee0: makes a keyframed rigid body of a havok component move and
turn so that it is where a matrix says in a tick; its scale is not looked
at */
void havok_component_set_rigid_body_keyframe(
	struct havok_component *component,
	long rigid_body_index,
	const real_matrix4x3 *matrix)
{
	real ticks_per_second= (real)game_time_globals->ticks_per_second;
	struct havok_transform transform;
	struct havok_vector4 orientation;

	havok_transform_from_matrix(&transform, matrix);
	havok_component_transform_to_rigid_bodies(component, &transform);
	hkQuaternion_set(&orientation, (const struct havok_matrix3 *)transform.rotation);
	hkKeyFrameUtility_applyHardKeyFrame((const struct havok_vector4 *)transform.translation, &orientation,
		ticks_per_second, component->rigid_bodies[rigid_body_index].rigid_body);
}

/* 0x001cec30: the table of what the objects have in the library of the
physics is made ("havok components": 0x200 of 0xA0 bytes, at addresses that
are multiples of 16), in memory of the heap (data_new, which the game has
written here) */
void havok_components_initialize(
	void)
{
	havok_component_data= data_new("havok components", 0x200, 0xA0, 4, heap_allocation);
}
