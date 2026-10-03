/*
HAVOK_COMPONENT.H

What an object has of Havok, the physics library: some rigid bodies. The
names of the file and of everything in it are guesses.
*/

#ifndef __HAVOK_COMPONENT_H
#define __HAVOK_COMPONENT_H

#include "cseries/cseries.h"
#include "math/real_math.h"
#include "memory/data.h"

/* ---------- constants */

/* the methods of Havok's objects take the object in ecx */
#define HAVOK_CONVENTION __attribute__((thiscall))

/* what kind of motion moves a rigid body (Havok's hkMotion::MotionType) */
enum
{
	_havok_motion_keyframed= 6
};

/* of havok_component->flags */
enum
{
	/* the rigid bodies are not made to turn */
	_havok_component_no_angular_velocity_bit= 1,
	/* a fixed rigid body is put somewhere between two functions that are
	not decompiled, 0x001d1260 and 0x001d1540 */
	_havok_component_unknown5_bit= 5,
	/* the transforms of the rigid bodies are from the component's */
	_havok_component_transformed_bit= 6
};

/* ---------- structures */

/* a rotation as Havok has it: three columns, each of four reals */
struct havok_matrix3
{
	real columns[3][4];
} __attribute__((aligned(16)));

/* a vector as Havok has it (hkVector4): the fourth real is 0 for a vector
of three */
struct havok_vector4
{
	real v[4];
} __attribute__((aligned(16)));

/* where something is and how it is turned, as Havok has it (hkTransform):
the three columns of the rotation and the translation, each of four reals */
struct havok_transform
{
	real rotation[3][4];
	real translation[4];
} __attribute__((aligned(16)));

struct havok_motion;

/* the methods of what moves a rigid body: only those that are used */
struct havok_motion_vtable
{
	byte unknown0[0x18];
	long (HAVOK_CONVENTION *get_type)(struct havok_motion *motion);
	byte unknown1C[8];
	void (HAVOK_CONVENTION *get_rotation)(struct havok_motion *motion, struct havok_matrix3 *rotation);
	byte unknown28[0x24];
	/* puts the body somewhere */
	void (HAVOK_CONVENTION *set_transform)(struct havok_motion *motion, const struct havok_transform *transform);
	void (HAVOK_CONVENTION *set_linear_velocity)(struct havok_motion *motion, const struct havok_vector4 *velocity);
	void (HAVOK_CONVENTION *set_angular_velocity)(struct havok_motion *motion, const struct havok_vector4 *velocity);
	/* how fast a point of the world, that moves with the body, moves */
	void (HAVOK_CONVENTION *get_point_velocity)(struct havok_motion *motion, const struct havok_vector4 *point,
		struct havok_vector4 *velocity);
};

/* what moves a rigid body of Havok */
struct havok_motion
{
	const struct havok_motion_vtable *vtable;
	byte unknown4[0x28];
	/* 0 for a body that nothing moves */
	real inverse_mass;
	byte unknown30[0x10];
	real_vector3d linear_velocity;
	byte unknown4C[4];
	real_vector3d angular_velocity;
	byte unknown5C[0x14];
	/* in the world */
	real_point3d center_of_mass;
	byte unknown7C[4];
	/* of the rigid body, in the world */
	struct havok_transform transform;
};

/* Havok's own rigid body */
struct havok_rigid_body
{
	byte unknown0[8];
	/* the world of Havok that it is in, it seems: a fixed body in one is
	not put somewhere through what moves it */
	void *world;
	byte unknownC[0x30];
	struct havok_motion *motion;
	/* it does not move: its velocities are 0 */
	boolean fixed;
	byte unknown41[3];
	/* where the body is simulated, if it is in Havok's world: it can be
	asleep */
	byte *simulation_island;
};

/* a rigid body of a havok component */
struct havok_component_rigid_body
{
	byte unknown0[0x40];
	struct havok_rigid_body *rigid_body;
	byte unknown44[0x1C];
};

struct havok_component
{
	byte unknown0[4];
	/* _havok_component_..._bit */
	unsigned long flags;
	byte unknown8[0x10];
	/* the main rigid body: its center of mass and its velocities are those
	of the object */
	char rigid_body_index;
	byte unknown19[0x17];
	/* what the transforms of the rigid bodies are from, with
	_havok_component_transformed_bit */
	struct havok_transform transform;
	struct havok_component_rigid_body *rigid_bodies;
	long rigid_body_count;
	byte unknown78[0x28];
};

typedef char verify_havok_motion_vtable_get_rotation_offset[
	offsetof(struct havok_motion_vtable, get_rotation) == 0x24 ? 1 : -1];
typedef char verify_havok_motion_inverse_mass_offset[
	offsetof(struct havok_motion, inverse_mass) == 0x2C ? 1 : -1];
typedef char verify_havok_motion_linear_velocity_offset[
	offsetof(struct havok_motion, linear_velocity) == 0x40 ? 1 : -1];
typedef char verify_havok_motion_center_of_mass_offset[
	offsetof(struct havok_motion, center_of_mass) == 0x70 ? 1 : -1];
typedef char verify_havok_motion_transform_offset[
	offsetof(struct havok_motion, transform) == 0x80 ? 1 : -1];
typedef char verify_havok_component_transform_offset[
	offsetof(struct havok_component, transform) == 0x30 ? 1 : -1];
typedef char verify_havok_component_rigid_bodies_offset[
	offsetof(struct havok_component, rigid_bodies) == 0x70 ? 1 : -1];
typedef char verify_havok_motion_vtable_get_point_velocity_offset[
	offsetof(struct havok_motion_vtable, get_point_velocity) == 0x58 ? 1 : -1];
typedef char verify_havok_rigid_body_simulation_island_offset[
	offsetof(struct havok_rigid_body, simulation_island) == 0x44 ? 1 : -1];
typedef char verify_havok_motion_vtable_set_transform_offset[
	offsetof(struct havok_motion_vtable, set_transform) == 0x4C ? 1 : -1];
typedef char verify_havok_rigid_body_motion_offset[
	offsetof(struct havok_rigid_body, motion) == 0x3C ? 1 : -1];
typedef char verify_havok_rigid_body_fixed_offset[
	offsetof(struct havok_rigid_body, fixed) == 0x40 ? 1 : -1];
typedef char verify_havok_component_rigid_body_size[
	sizeof(struct havok_component_rigid_body) == 0x60 ? 1 : -1];
typedef char verify_havok_component_size[
	sizeof(struct havok_component) == 0xA0 ? 1 : -1];

/* ---------- globals */

extern struct data_array *havok_component_data;

/* ---------- prototypes/HAVOK_COMPONENT.C */

void havok_component_get_rigid_body_linear_velocity(const struct havok_component *component, long rigid_body_index,
	real_vector3d *velocity);
void havok_component_get_rigid_body_angular_velocity(const struct havok_component *component, long rigid_body_index,
	real_vector3d *velocity);
void havok_component_get_rigid_body_position(const struct havok_component *component, long rigid_body_index,
	real_point3d *position);
void havok_component_get_rigid_body_matrix(const struct havok_component *component, long rigid_body_index,
	real_matrix4x3 *matrix);
void havok_component_get_rigid_body_point_velocity(const struct havok_component *component, long rigid_body_index,
	const real_point3d *point, real_vector3d *velocity);
void havok_component_set_rigid_body_linear_velocity(const struct havok_component *component, long rigid_body_index,
	const real_vector3d *velocity);
void havok_component_set_rigid_body_angular_velocity(const struct havok_component *component, long rigid_body_index,
	const real_vector3d *velocity);
void havok_rigid_body_set_transform(struct havok_rigid_body *rigid_body, const struct havok_transform *transform);
void havok_component_set_rigid_body_transform(struct havok_component *component, long rigid_body_index,
	const struct havok_transform *transform);
void havok_component_set_rigid_body_matrix(struct havok_component *component, long rigid_body_index,
	const real_matrix4x3 *matrix);
void havok_component_get_rigid_body_rotation(const struct havok_component *component, long rigid_body_index,
	real_matrix3x3 *rotation);
real havok_component_get_rigid_body_mass(const struct havok_component *component, long rigid_body_index);
boolean havok_component_rigid_body_is_keyframed(const struct havok_component *component, long rigid_body_index);

#endif
