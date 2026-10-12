/*
OBJECTS.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.
*/

#include "objects.h"
#include "objects/object_types.h"

/* ---------- macros */

/* the header of an object that is known to be there */
#define OBJECT_HEADER_GET(object_index) \
	(&((struct object_header_datum *)object_header_data->data)[(object_index)&0xFFFF])
#define OBJECT_GET(object_index) (OBJECT_HEADER_GET(object_index)->datum)
/* the havok component of an index that is not NONE; its salt is not looked
at */
#define HAVOK_COMPONENT_GET(havok_component_index) \
	(&((struct havok_component *)havok_component_data->data)[(havok_component_index)&0xFFFF])

/* ---------- globals */

/* 0x004e0300 */
struct data_array *object_header_data;

/* ---------- code */

/* 0x000b7360: marks the object and every object it is attached to, up to
the first one marked already. The name is a guess */
void object_wake(
	long object_index)
{
	if (TEST_FLAG(OBJECT_HEADER_GET(object_index)->flags, _object_header_awake_bit))
	{
		return;
	}

	while (object_index!=NONE)
	{
		struct object_header_datum *header= OBJECT_HEADER_GET(object_index);

		header->flags|= FLAG(_object_header_awake_bit);
		object_index= header->datum->parent_object_index;
	}

	return;
}

/* 0x000baeb0: the next object of the iterator's types whose header has all
the iterator's flags; NULL when there are no more. As in Halo 1 */
void *object_iterator_next(
	struct object_iterator *iterator)
{
	short absolute_index= iterator->absolute_index;
	struct object_header_datum *header=
		&((struct object_header_datum *)object_header_data->data)[absolute_index];
	void *result= NULL;

	while (absolute_index<object_header_data->first_unallocated)
	{
		long index= (long)((unsigned long)(word)header->identifier<<16 | (unsigned long)(long)absolute_index);

		absolute_index++;
		if (header->identifier &&
			(header->flags&iterator->header_flags)==iterator->header_flags &&
			(iterator->type_mask & (1UL<<(header->type&31))))
		{
			iterator->index= index;
			result= header->datum;
			break;
		}
		header++;
	}
	iterator->absolute_index= absolute_index;

	return result;
}

/* 0x000badc0: the object, if it is there and is of one of the types of the
mask; NULL otherwise. As in Halo 1 */
void *object_try_and_get_and_verify_type(
	long object_index,
	unsigned long type_mask)
{
	struct object_header_datum *header= datum_try_and_get(object_header_data, object_index);

	if (header && (type_mask & (1UL<<(header->type&31))))
	{
		return header->datum;
	}

	return NULL;
}

/* 0x000bae20: the same as object_try_and_get_and_verify_type: in Halo 1 this
one stops the game if the object is not there, and the game is built
without it. The name is Halo 1's */
void *object_get_and_verify_type(
	long object_index,
	unsigned long type_mask)
{
	struct object_header_datum *header= datum_try_and_get(object_header_data, object_index);

	if (header && (type_mask & (1UL<<(header->type&31))))
	{
		return header->datum;
	}

	return NULL;
}

/* 0x000bae80: an iterator that goes through the objects of some types (all
with 0) whose headers have some flags, from the first. As in Halo 1 */
void object_iterator_new(
	struct object_iterator *iterator,
	unsigned long type_mask,
	byte header_flags)
{
	iterator->signature= OBJECT_ITERATOR_SIGNATURE;
	if (!type_mask)
	{
		type_mask= (unsigned long)NONE;
	}
	iterator->type_mask= type_mask;
	iterator->header_flags= header_flags;
	iterator->absolute_index= 0;
	iterator->index= NONE;
}

/* 0x000baf40: the first object, going from an object to what it is attached
to, that has not the bit 26 of its flags; NONE if there is none. The name is
a guess */
long object_get_first_unflagged_ancestor(
	long object_index)
{
	while (object_index!=NONE)
	{
		struct object_datum *object= OBJECT_GET(object_index);

		if (!(object->flags&FLAG(_object_unknown26_bit)))
		{
			break;
		}
		object_index= object->parent_object_index;
	}

	return object_index;
}

/* 0x000baf80: the object that an object is attached to, through every
object between them; the object itself if it is attached to nothing */
long object_get_ultimate_parent(
	long object_index)
{
	long parent_index= NONE;

	while (object_index!=NONE)
	{
		parent_index= object_index;
		object_index= OBJECT_GET(object_index)->parent_object_index;
	}

	return parent_index;
}

/* 0x000b9dd0: where the object is in the world. The position of an object
attached to another is from the node of the other that it is attached to */
void object_get_origin(
	long object_index,
	real_point3d *origin)
{
	struct object_datum *object= OBJECT_GET(object_index);

	if (object->parent_object_index==NONE)
	{
		*origin= object->position;
	}
	else
	{
		struct object_datum *parent= OBJECT_GET(object->parent_object_index);
		real_matrix4x3 *node_matrices= (real_matrix4x3 *)((byte *)parent+parent->node_matrices_offset);

		matrix4x3_transform_point(&node_matrices[object->parent_node_index], &object->position, origin);
	}

	return;
}

/* the main rigid body of the havok component of an object, and the
component; NONE if it has none */
static inline __attribute__((always_inline)) short object_get_main_rigid_body(
	const struct object_datum *object,
	struct havok_component **component)
{
	short rigid_body_index= NONE;

	if (object->havok_component_index!=NONE)
	{
		struct havok_component *havok_component= HAVOK_COMPONENT_GET(object->havok_component_index);

		if (havok_component->rigid_body_index>=0 &&
			havok_component->rigid_body_index<havok_component->rigid_body_count)
		{
			rigid_body_index= havok_component->rigid_body_index;
		}
		*component= havok_component;
	}

	return rigid_body_index;
}

/* 0x000b9ef0: where the center of mass of an object is: that of a rigid
body of the havok component of the object that it is attached to at the
top (or of itself), or the object's origin if there is none. The name is a
guess */
real_point3d *object_get_center_of_mass(
	long object_index,
	real_point3d *center)
{
	struct havok_component *component;
	short rigid_body_index=
		object_get_main_rigid_body(OBJECT_GET(object_get_ultimate_parent(object_index)), &component);

	if (rigid_body_index!=NONE)
	{
		*center= component->rigid_bodies[rigid_body_index].rigid_body->motion->center_of_mass;
		return center;
	}
	object_get_origin(object_index, center);

	return center;
}

/* 0x000ba1d0: how fast the object that an object is attached to at the top
(or the object itself) moves and turns; either can be NULL. They are
Havok's, of the main rigid body of its havok component, if it has one;
0 if the object is fixed. The name is Halo 1's */
void object_get_velocities(
	long object_index,
	real_vector3d *translational_velocity,
	real_vector3d *angular_velocity)
{
	struct object_datum *object= OBJECT_GET(object_get_ultimate_parent(object_index));
	struct havok_component *component;
	short rigid_body_index= object_get_main_rigid_body(object, &component);

	if (rigid_body_index!=NONE)
	{
		boolean fixed= TEST_FLAG(object->physics_flags, _object_physics_fixed_bit);

		if (translational_velocity)
		{
			if (fixed)
			{
				*translational_velocity= *global_zero_vector3d;
			}
			else
			{
				havok_component_get_rigid_body_linear_velocity(component, rigid_body_index, translational_velocity);
			}
		}
		if (angular_velocity)
		{
			if (fixed)
			{
				*angular_velocity= *global_zero_vector3d;
			}
			else
			{
				havok_component_get_rigid_body_angular_velocity(component, rigid_body_index, angular_velocity);
			}
		}
		return;
	}
	if (translational_velocity)
	{
		*translational_velocity= object->translational_velocity;
	}
	if (angular_velocity)
	{
		*angular_velocity= object->angular_velocity;
	}
}

/* a vector turned by the rotation of a matrix, with no scale */
static inline __attribute__((always_inline)) void object_rotate_vector(
	const real_matrix4x3 *matrix,
	const real_vector3d *vector,
	real_vector3d *result)
{
	real i= vector->i;
	real j= vector->j;
	real k= vector->k;

	result->i= matrix->up.i*k + matrix->left.i*j + matrix->forward.i*i;
	result->j= matrix->up.j*k + matrix->left.j*j + matrix->forward.j*i;
	result->k= matrix->up.k*k + matrix->left.k*j + matrix->forward.k*i;
}

/* 0x000b9fc0: where the object faces and what is up for it, in the world:
those of an object attached to another are turned by the node of the other
that it is attached to. Either can be not asked for. As Halo 1's */
void object_get_orientation(
	long object_index,
	real_vector3d *forward,
	real_vector3d *up)
{
	struct object_datum *object= OBJECT_GET(object_index);

	if (object->parent_object_index==NONE)
	{
		if (forward)
		{
			*forward= object->forward;
		}
		if (up)
		{
			*up= object->up;
		}
	}
	else
	{
		struct object_datum *parent= OBJECT_GET(object->parent_object_index);
		const real_matrix4x3 *node_matrix=
			&((const real_matrix4x3 *)((const byte *)parent+parent->node_matrices_offset))[object->parent_node_index];

		if (forward)
		{
			object_rotate_vector(node_matrix, &object->forward, forward);
		}
		if (up)
		{
			object_rotate_vector(node_matrix, &object->up, up);
		}
	}
}

/* 0x000ba160: the matrix that takes a point of the object to the world */
real_matrix4x3 *object_get_world_matrix(
	long object_index,
	real_matrix4x3 *matrix)
{
	struct object_datum *object= OBJECT_GET(object_index);

	matrix4x3_from_point_and_vectors(matrix, &object->position, &object->forward, &object->up);
	if (object->parent_object_index!=NONE)
	{
		struct object_datum *parent= OBJECT_GET(object->parent_object_index);
		real_matrix4x3 *node_matrices= (real_matrix4x3 *)((byte *)parent+parent->node_matrices_offset);

		matrix4x3_multiply(&node_matrices[object->parent_node_index], matrix, matrix);
	}

	return matrix;
}

/* 0x000ba300: the location of the object that this one is attached to,
through every object between them */
void object_get_location(
	long object_index,
	struct location *location)
{
	*location= OBJECT_GET(object_get_ultimate_parent(object_index))->location;

	return;
}

#include <string.h>

/* what the game finds of a marker of an object: 0x70 bytes, with where it is
at 0x60; and the names of two markers */
enum
{
	OBJECT_MARKER_SIZE= 0x70,
	OBJECT_MARKER_POSITION_OFFSET= 0x60,
	_object_marker_name95= 0x4000095,
	_object_marker_nameBD= 0x40000BD
};

/* 0x000b8d30: the markers of an object that have a name, up to so many; how
many it found */
short __attribute__((stdcall)) unknown_000b8d30(boolean unknown, byte *markers, long object_index, long name,
	long maximum_count);

/* 0x000caf60: where the marker of an object that has the name of 0x4000095
is. The game does not look at whether there is one */
void object_get_marker95_position(
	long object_index,
	real_point3d *position)
{
	byte marker[OBJECT_MARKER_SIZE];

	unknown_000b8d30(FALSE, marker, object_index, _object_marker_name95, 1);
	memcpy(position, marker+OBJECT_MARKER_POSITION_OFFSET, sizeof(*position));
}

/* 0x000caf90: where the marker of an object that has the name of 0x40000BD
is. The game does not look at whether there is one */
void object_get_markerBD_position(
	long object_index,
	real_point3d *position)
{
	byte marker[OBJECT_MARKER_SIZE];

	unknown_000b8d30(FALSE, marker, object_index, _object_marker_nameBD, 1);
	memcpy(position, marker+OBJECT_MARKER_POSITION_OFFSET, sizeof(*position));
}

/* 0x0003d380: after a state of the game was loaded, each object has NONE at
0xCC. It is one of the things that are done after a state is loaded
(game_state.c); object_iterator_new is written here. The game has it far
from the rest of this file, and what the number at 0xCC is of is not known */
void __attribute__((stdcall)) objects_unknown_0003d380(
	unsigned long flags)
{
	struct object_iterator iterator;
	byte *object;

	(void)flags;
	iterator.signature= OBJECT_ITERATOR_SIGNATURE;
	iterator.type_mask= 0xFFFFFFFF;
	iterator.header_flags= 0;
	iterator.absolute_index= 0;
	iterator.index= NONE;
	while ((object= object_iterator_next(&iterator))!=NULL)
	{
		*(long *)(object+0xCC)= NONE;
	}
}


enum
{
	/* of an object: the thing that the simulation tells of it, or NONE, and
	a byte that goes with it */
	OBJECT_SIMULATION_ENTITY_OFFSET= 0xD4,
	OBJECT_SIMULATION_UNKNOWND8_OFFSET= 0xD8
};

/* 0x000bb7b0: an object lets go of the thing that the simulation tells of
it: its type is told (object_types_call_unknown54), and it has none */
void object_detach_simulation_entity(
	long object_index)
{
	byte *object= (byte *)((const struct object_header_datum *)object_header_data->data)[object_index&0xFFFF].datum;

	object_types_call_unknown54(object_index);
	*(long *)(object+OBJECT_SIMULATION_ENTITY_OFFSET)= NONE;
	object[OBJECT_SIMULATION_UNKNOWND8_OFFSET]= 0;
}

/* 0x000bb7f0: every object that has one lets go of the thing that the
simulation tells of it: it is what is done when a game in the network of
kind 4 or 5 is no more (simulation_after_load). The game has
object_iterator_new and object_detach_simulation_entity written here */
void objects_detach_simulation_entities(
	void)
{
	struct object_iterator iterator;
	const byte *object;

	iterator.signature= OBJECT_ITERATOR_SIGNATURE;
	iterator.type_mask= 0xFFFFFFFF;
	iterator.header_flags= 0;
	iterator.absolute_index= 0;
	iterator.index= NONE;
	while ((object= object_iterator_next(&iterator))!=NULL)
	{
		if (*(const long *)(object+OBJECT_SIMULATION_ENTITY_OFFSET)!=NONE)
		{
			object_detach_simulation_entity(iterator.index);
		}
	}
}
