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

/* ---------- structures */

/* what moves a rigid body of Havok */
struct havok_motion
{
	byte unknown0[0x40];
	real_vector3d linear_velocity;
	byte unknown4C[4];
	real_vector3d angular_velocity;
	byte unknown5C[0x14];
	/* in the world */
	real_point3d center_of_mass;
};

/* Havok's own rigid body */
struct havok_rigid_body
{
	byte unknown0[0x3C];
	struct havok_motion *motion;
	/* it does not move: its velocities are 0 */
	boolean fixed;
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
	byte unknown0[0x18];
	/* the main rigid body: its center of mass and its velocities are those
	of the object */
	char rigid_body_index;
	byte unknown19[0x57];
	struct havok_component_rigid_body *rigid_bodies;
	long rigid_body_count;
	byte unknown78[0x28];
};

typedef char verify_havok_motion_center_of_mass_offset[
	offsetof(struct havok_motion, center_of_mass) == 0x70 ? 1 : -1];
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

#endif
