/*
HAVOK_COMPONENT.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

What an object has of Havok, the physics library. The names of the file and
of its functions are guesses.
*/

#include "havok_component.h"

/* ---------- globals */

/* 0x0051e9b8: of struct havok_component, "havok components". The code that
makes it is not decompiled */
struct data_array *havok_component_data;

/* ---------- code */

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
