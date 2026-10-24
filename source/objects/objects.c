/*
OBJECTS.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.
*/

#include "objects.h"
#include "objects/object_types.h"

#include <stdint.h>

#include "cache/cache_files.h"
#include "memory/memory_pool.h"
#include "objects/cluster_partitions.h"

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

/* an object as bytes, from its index */
#define OBJECT_BYTES(object_index) ((byte *)OBJECT_GET(object_index))

/* 0x0009d250: whether an object does not have the bit 2 of its byte at
0x10A (what object_list_count counts). The game has it far from the rest of
this file, as most of those that follow */
long __attribute__((stdcall)) object_unknown_0009d250(
	long object_index)
{
	return !((OBJECT_BYTES(object_index)[0x10A]>>2)&1);
}

/* 0x000a0930: the object of something (its third number) keeps the first
number of it at 0xD4, where an object has its entity of the simulation;
nothing for no object */
void __attribute__((stdcall)) object_unknown_000a0930(
	const long *unknown)
{
	if (unknown[2]!=NONE)
	{
		*(long *)(OBJECT_BYTES(unknown[2])+0xD4)= unknown[0];
	}
}

/* 0x000b8bd0: one of the things of 0x34 bytes that an object has at the
offset that its short at 0x116 gives (its nodes, by the size) */
byte *object_get_unknown116(
	long object_index,
	short index)
{
	byte *object= OBJECT_BYTES(object_index);

	return object+*(short *)(object+0x116)+index*0x34;
}

/* 0x000bf5a0: whether an object has something at the offset that its short
at 0x12A gives */
boolean object_has_unknown12A(
	long object_index)
{
	return *(short *)(OBJECT_BYTES(object_index)+0x12A)!=NONE;
}

/* 0x000bf5d0: the same, for the short at 0x112 */
boolean object_has_unknown112(
	long object_index)
{
	return *(short *)(OBJECT_BYTES(object_index)+0x112)!=NONE;
}

/* 0x000bfc90: whether an object, if there is one, has anything in its byte
at 0xB3 */
boolean object_unknown_000bfc90(
	long object_index)
{
	if (object_index!=NONE && OBJECT_BYTES(object_index)[0xB3]>0)
	{
		return TRUE;
	}

	return FALSE;
}

/* 0x000d0590: something of four numbers starts for an object: the object,
NONE, a short that is NONE, and what the object has at 0x10 */
void object_unknown_000d0590(
	byte *unknown,
	long object_index)
{
	const byte *object= OBJECT_BYTES(object_index);

	*(long *)unknown= object_index;
	*(long *)(unknown+4)= NONE;
	*(short *)(unknown+8)= NONE;
	*(long *)(unknown+0xC)= *(const long *)(object+0x10);
}

/* 0x004de2ec: a memory pool, most likely the one that the objects are in */
struct memory_pool *unknown_004de2ec;

/* 0x000b8c00, of units.c */
byte *object_get_unknown116_and_count(long object_index, long *count);

#define OBJECT_BYTES_GET(object_index) \
	((byte *)((struct object_header_datum *)object_header_data->data)[(object_index)&0xFFFF].datum)

/* 0x000bb780: an object takes a value for its number at 0xD4, and its type
is told (object_types_call_unknown50) */
void object_set_unknownD4(
	long object_index,
	long value)
{
	*(long *)(OBJECT_BYTES_GET(object_index)+0xD4)= value;
	object_types_call_unknown50(object_index);
}

/* 0x000bb880: the type of every object is given a value
(object_types_call_unknown58) */
void __attribute__((stdcall)) objects_call_unknown58(
	long value)
{
	struct object_iterator iterator;

	iterator.signature= OBJECT_ITERATOR_SIGNATURE;
	iterator.type_mask= 0xFFFFFFFF;
	iterator.header_flags= 0;
	iterator.absolute_index= 0;
	iterator.index= NONE;
	while (object_iterator_next(&iterator))
	{
		object_types_call_unknown58(iterator.index, value);
	}
}

/* 0x000bb8f0: the same with object_types_call_unknown5C */
void objects_call_unknown5C(
	long value)
{
	struct object_iterator iterator;

	iterator.signature= OBJECT_ITERATOR_SIGNATURE;
	iterator.type_mask= 0xFFFFFFFF;
	iterator.header_flags= 0;
	iterator.absolute_index= 0;
	iterator.index= NONE;
	while (object_iterator_next(&iterator))
	{
		object_types_call_unknown5C(iterator.index, value);
	}
}

/* 0x000bf360: the memory pool is made compact. It takes six things that it
does not use, and gives zero: it looks like a function that a table names */
long __attribute__((stdcall)) objects_compact_memory_pool(
	long unknown1,
	long unknown2,
	long unknown3,
	long unknown4,
	long unknown5,
	long unknown6)
{
	(void)unknown1;
	(void)unknown2;
	(void)unknown3;
	(void)unknown4;
	(void)unknown5;
	(void)unknown6;
	memory_pool_compact(unknown_004de2ec);

	return 0;
}

/* 0x000be1d0: if the tag that the definition of an object names at 0x38
names two more, at 4 and 0x14, the type of the object is given what the
object has at the place that its short at 0x116 says, and how many
(object_types_call_unknown74) */
void object_unknown_000be1d0(
	long object_index)
{
	const byte *object= OBJECT_BYTES_GET(object_index);
	const byte *definition= (const byte *)(uintptr_t)global_tag_instances[*(const long *)object&0xFFFF].address;
	long tag_index= *(const long *)(definition+0x38);

	if (tag_index!=NONE)
	{
		const byte *tag= (const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address;

		if (*(const long *)(tag+4)!=NONE && *(const long *)(tag+0x14)!=NONE)
		{
			long count;
			byte *unknown116= object_get_unknown116_and_count(object_index, &count);

			object_types_call_unknown74(object_index, count, (long)(uintptr_t)unknown116);
		}
	}
}

/* three small functions of things that are not known; they are just before
object_wake in the game */

/* 0x000b6760: whether something has a number at 0 and a short at 6 */
long unknown_has_unknown0_and_unknown6(
	const byte *unknown)
{
	return *(const long *)unknown!=NONE && *(const short *)(unknown+6)!=NONE;
}

/* 0x000b6780: and a number at 0x68 too */
long unknown_has_unknown0_unknown6_and_unknown68(
	const byte *unknown)
{
	return *(const long *)(unknown+0x68)!=NONE && *(const long *)unknown!=NONE &&
		*(const short *)(unknown+6)!=NONE;
}

/* 0x000b67a0: the byte that something has at 0x18, if it is not less than
zero and is less than the number at 0x74; NONE if not */
short unknown_get_unknown18_below_unknown74(
	const byte *unknown)
{
	signed char index= (signed char)unknown[0x18];

	if (index>=0 && index<*(const long *)(unknown+0x74))
	{
		return index;
	}

	return NONE;
}

/* 0x004de2f4: it points to something whose first byte says whether it is
set */
byte *unknown_004de2f4;

/* 0x000b8820: whether unknown_004de2f4 points to something that is set */
long unknown_004de2f4_is_set(
	void)
{
	return unknown_004de2f4 && *unknown_004de2f4;
}

/* 0x004de2d4 and 0x004de2e0: the objects of each cluster, those whose bit 9
of the flags at 0x04 is not set and those whose is. The names are as in
Halo 1 */
struct cluster_partition noncollideable_object_cluster_partition;
struct cluster_partition collideable_object_cluster_partition;

/* an object and the rest of the reference of a list of the first table of a
partition, and the next of the list */
static inline __attribute__((always_inline)) long object_cluster_reference_next(
	const struct cluster_partition *partition,
	long *reference_index,
	void **data)
{
	const struct data_array *references= partition->unknown4;
	byte *reference= (byte *)references->data+(*reference_index&0xFFFF)*references->size;

	*data= reference+0x0C;
	*reference_index= *(const long *)(reference+0x08);
	return *(const long *)(reference+0x04);
}

/* 0x000b8940: the first object in a cluster of the collideable ones; what
its reference has after 0x0C, and the next reference. NONE if there is none */
long __attribute__((stdcall)) cluster_get_first_collideable_object(
	long *reference_index,
	short cluster_index,
	void **data)
{
	*reference_index= collideable_object_cluster_partition.unknown0[cluster_index];
	if (*reference_index==NONE)
	{
		return NONE;
	}

	return object_cluster_reference_next(&collideable_object_cluster_partition, reference_index, data);
}

/* 0x000b89b0: the next one */
long __attribute__((stdcall)) cluster_get_next_collideable_object(
	long *reference_index,
	void **data)
{
	if (*reference_index==NONE)
	{
		return NONE;
	}

	return object_cluster_reference_next(&collideable_object_cluster_partition, reference_index, data);
}

/* 0x000b8a10: as 0x000b8940, of the ones that are not collideable */
long __attribute__((stdcall)) cluster_get_first_noncollideable_object(
	long *reference_index,
	short cluster_index,
	void **data)
{
	*reference_index= noncollideable_object_cluster_partition.unknown0[cluster_index];
	if (*reference_index==NONE)
	{
		return NONE;
	}

	return object_cluster_reference_next(&noncollideable_object_cluster_partition, reference_index, data);
}

/* where an object is: its partition and its next reference of the second
table */
struct object_cluster_iterator
{
	const struct cluster_partition *partition;
	long reference_index;
};

/* 0x000b8b20: the next cluster of an object, NONE after the last. As in
Halo 1 */
short object_get_next_cluster(
	struct object_cluster_iterator *iterator)
{
	const struct data_array *references= iterator->partition->unknown8;
	const byte *reference;

	if (iterator->reference_index==NONE)
	{
		return NONE;
	}
	reference= (const byte *)references->data+(iterator->reference_index&0xFFFF)*references->size;
	iterator->reference_index= *(const long *)(reference+0x08);
	return *(const short *)(reference+0x04);
}

/* 0x001d1260 and 0x001d1540: something of a havok component */
void unknown_001d1260(struct havok_component *component);
void unknown_001d1540(struct havok_component *component);

/* 0x000b8840: the bit 0x40 of the byte at 0xC0 of an object is cleared,
after unknown_001d1260 of its havok component if it has one */
void object_unknown_000b8840(
	long object_index)
{
	byte *object= OBJECT_BYTES(object_index);
	long havok_component_index= *(const long *)(object+0xB4);

	if (havok_component_index!=NONE)
	{
		unknown_001d1260(HAVOK_COMPONENT_GET(havok_component_index));
	}
	object[0xC0]&= ~0x40;
}

/* 0x000b8890: the bit is set, after unknown_001d1540 */
void __attribute__((stdcall)) object_unknown_000b8890(
	long object_index)
{
	byte *object= OBJECT_BYTES(object_index);
	long havok_component_index= *(const long *)(object+0xB4);

	if (havok_component_index!=NONE)
	{
		unknown_001d1540(HAVOK_COMPONENT_GET(havok_component_index));
	}
	object[0xC0]|= 0x40;
}

/* 0x000b8c40: a long (0x08) of an element (0x18 bytes) of the block at 0x94
of the tag of an object; NONE for one that is not there */
long object_unknown_000b8c40(
	long object_index,
	short index)
{
	const byte *tag= (const byte *)(uintptr_t)
		global_tag_instances[*(const long *)OBJECT_BYTES(object_index)&0xFFFF].address;

	if (index>=0 && index<*(const long *)(tag+0x94))
	{
		return *(const long *)(*(const byte *const *)(tag+0x98)+index*0x18+0x08);
	}

	return NONE;
}

/* the object of an index, without a salt check, as bytes */
#define OBJECT_DATA(object_index) \
	((byte *)((const struct object_header_datum *)object_header_data->data)[(object_index)&0xFFFF].datum)

/* 0x000b9d20: the bit 0x40 of the flags (0x02) of the header of the last
parent of an object (or of itself, without one) */
long object_unknown_000b9d20(
	long object_index)
{
	const struct object_header_datum *headers= (const struct object_header_datum *)object_header_data->data;
	long parent_index= NONE;

	while (object_index!=NONE)
	{
		parent_index= object_index;
		object_index= *(const long *)(OBJECT_DATA(object_index)+0x14);
	}

	return (headers[parent_index&0xFFFF].flags>>6)&1;
}

/* 0x000ba690: of what an object has at its short at 0x11A, of a size in
its short at 0x118: a tenth of the size, and where it starts and its first
and second tenths are (each if asked) */
void __attribute__((stdcall)) object_unknown_000ba690(
	long object_index,
	long *count,
	byte **first,
	byte **second,
	byte **third)
{
	byte *object= OBJECT_DATA(object_index);
	byte *data= object+*(const short *)(object+0x11A);

	*count= *(const short *)(object+0x118)/10;
	if (first)
	{
		*first= data;
	}
	if (second)
	{
		*second= data+*count;
	}
	if (third)
	{
		*third= data+*count*2;
	}
}

/* 0x000bad50: the index-th of the second half of the twelve-byte entries
that an object has at its short at 0x126 (of a size in its short at 0x124);
FALSE for one that is not there */
boolean object_unknown_000bad50(
	long object_index,
	long index,
	unsigned long *result)
{
	const byte *object= OBJECT_DATA(object_index);
	const unsigned long *entry;
	long count= (long)((unsigned long)(long)*(const short *)(object+0x124)/12)/2;

	if (index<0 || index>=count)
	{
		return FALSE;
	}
	entry= (const unsigned long *)(object+*(const short *)(object+0x126))+(count+index)*3;
	result[0]= entry[0];
	result[1]= entry[1];
	result[2]= entry[2];
	return TRUE;
}

/* 0x000bafb0: whether an object is the other or one of its parents (not
for NONE) */
long object_unknown_000bafb0(
	long object_index,
	long other_index)
{
	long previous_index= NONE;

	while (object_index!=NONE && previous_index!=other_index)
	{
		previous_index= object_index;
		object_index= *(const long *)(OBJECT_DATA(object_index)+0x14);
	}

	return other_index!=NONE && previous_index==other_index;
}

/* 0x000bbf40: the bit 0x80000 of the flags (0x04) of an object is set or
cleared; nothing for NONE */
void __attribute__((stdcall)) object_unknown_000bbf40(
	long object_index,
	boolean set)
{
	if (object_index!=NONE)
	{
		unsigned long *flags= (unsigned long *)(OBJECT_DATA(object_index)+0x04);

		if (set)
		{
			*flags|= 0x80000;
		}
		else
		{
			*flags&= ~0x80000UL;
		}
	}
}

/* 0x000be650: an object is taken out of a list where each has the next at
0x0C, from where the first is said */
void object_unknown_000be650(
	long *link,
	long object_index)
{
	byte *object;

	if (*link==NONE)
	{
		return;
	}
	for (;;)
	{
		object= OBJECT_DATA(*link);
		if (*link==object_index)
		{
			break;
		}
		link= (long *)(object+0x0C);
		if (*link==NONE)
		{
			return;
		}
	}
	*link= *(const long *)(object+0x0C);
	*(long *)(object+0x0C)= NONE;
}

/* 0x004de2fc: a number for each time. 0x004de300: the number of the last
time each was marked */
unsigned long unknown_004de2fc;
unsigned long unknown_004de300[0x800];

/* 0x000bec70: whether something was not marked this time yet; it is marked */
boolean object_unknown_000bec70(
	long index)
{
	if (unknown_004de300[index&0xFFFF]==unknown_004de2fc)
	{
		return FALSE;
	}
	unknown_004de300[index&0xFFFF]= unknown_004de2fc;
	return TRUE;
}

/* 0x000bf830: an object gets a real at 0xF0 and a short at 0x104 (0 if
below zero, no more than 0x7FFE) */
void __attribute__((stdcall)) object_unknown_000bf830(
	long object_index,
	real value,
	short number)
{
	byte *object= OBJECT_DATA(object_index);

	*(real *)(object+0xF0)= value;
	if (number<0)
	{
		*(short *)(object+0x104)= 0;
		return;
	}
	if (number>0x7FFE)
	{
		number= 0x7FFE;
	}
	*(short *)(object+0x104)= number;
}

/* the entries of eight bytes that an object has at its short at 0x11E, as
many as its short at 0x11C says in bytes */
#define OBJECT_UNKNOWN11E(object) ((byte *)(object)+*(const short *)((const byte *)(object)+0x11E))
#define OBJECT_UNKNOWN11E_COUNT(object) \
	((long)((unsigned long)(long)*(const short *)((const byte *)(object)+0x11C)>>3))

/* 0x000fd410: the long of the entry that an object names at 0x14C, if the
entry is there and of kind 3; NONE if not */
long object_unknown_000fd410(
	long object_index)
{
	const byte *object= OBJECT_DATA(object_index);
	long index= *(const long *)(object+0x14C);

	if (index!=NONE && index>=0 && index<OBJECT_UNKNOWN11E_COUNT(object) && OBJECT_UNKNOWN11E(object)[index*8]==3)
	{
		return *(const long *)(OBJECT_UNKNOWN11E(object)+index*8+4);
	}

	return NONE;
}

/* 0x000fd460: that entry, if it is there, gets the kind 3 and a long */
void __attribute__((stdcall)) object_unknown_000fd460(
	long object_index,
	long value)
{
	byte *object= OBJECT_DATA(object_index);
	long index= *(const long *)(object+0x14C);

	if (index!=NONE && index>=0 && index<OBJECT_UNKNOWN11E_COUNT(object))
	{
		OBJECT_UNKNOWN11E(object)[index*8]= 3;
		*(long *)(OBJECT_UNKNOWN11E(object)+*(const long *)(object+0x14C)*8+4)= value;
	}
}

/* 0x000edfa0: an object without a parent gets two reals at 0x70, 0.0 at
0x78 and global_up3d at 0x7C */
void object_unknown_000edfa0(
	long object_index,
	const unsigned long *values)
{
	unsigned long *object= (unsigned long *)OBJECT_DATA(object_index);

	if (object[0x14/4]==NONE)
	{
		unsigned long first= values[0];

		object[0x74/4]= values[1];
		object[0x70/4]= first;
		*(real *)&object[0x78/4]= 0.0f;
		object[0x7C/4]= ((const unsigned long *)global_up3d)[0];
		object[0x80/4]= ((const unsigned long *)global_up3d)[1];
		object[0x84/4]= ((const unsigned long *)global_up3d)[2];
	}
}

/* 0x000fbfd0: an object gets at 0x170 the real at 0x178 of its definition
with the bit 3 of its flags, and that at 0x16C without */
void object_unknown_000fbfd0(
	long object_index)
{
	unsigned long *object= (unsigned long *)OBJECT_DATA(object_index);
	const byte *definition= (const byte *)(uintptr_t)global_tag_instances[object[0]&0xFFFF].address;

	if ((object[1]>>3)&1)
	{
		object[0x170/4]= *(const unsigned long *)(definition+0x178);
		return;
	}
	object[0x170/4]= *(const unsigned long *)(definition+0x16C);
}

/* 0x001896c0: a sound is played */
void __attribute__((stdcall)) sound_unknown_001896c0(long sound_index, real scale);

/* 0x000f8160: the sound that a definition names at 0x138, if any */
void object_definition_unknown_000f8160(
	long definition_index)
{
	long sound_index= *(const long *)((const byte *)(uintptr_t)global_tag_instances[definition_index&0xFFFF].address+0x138);

	if (sound_index!=NONE)
	{
		sound_unknown_001896c0(sound_index, 1.0f);
	}
}

/* 0x000f8110: the same, of the definition of an object */
void object_unknown_000f8110(
	long object_index)
{
	long sound_index= *(const long *)((const byte *)(uintptr_t)
		global_tag_instances[*(const long *)OBJECT_DATA(object_index)&0xFFFF].address+0x138);

	if (sound_index!=NONE)
	{
		sound_unknown_001896c0(sound_index, 1.0f);
	}
}

/* 0x000b6d60: whether an object is seen by a list of bits of clusters: TRUE
with the bit 0x02 of its flags; for the kind 7, what object_types_call_unknown48
says when it says something; else the bit of its cluster (FALSE for none) */
boolean __attribute__((stdcall)) object_unknown_000b6d60(
	long object_index,
	const unsigned long *cluster_bits)
{
	const struct object_header_datum *header= (const struct object_header_datum *)object_header_data->data+(object_index&0xFFFF);
	boolean result= FALSE;

	if ((((const unsigned long *)header->datum)[1]>>1)&1)
	{
		return TRUE;
	}
	if ((signed char)(byte)(1UL<<(header->type&0x1F))<0
		&& object_types_call_unknown48(object_index, (long)(uintptr_t)&result, (long)(uintptr_t)cluster_bits))
	{
		return result;
	}
	if (header->cluster_index==NONE)
	{
		return result;
	}

	return (cluster_bits[header->cluster_index>>5]&(1UL<<(header->cluster_index&0x1F)))!=0;
}

/* 0x000bf9a0: the tag (and the long at 0x0C) that the tag the definition of
an object names at 0x38 names at 0x04; TRUE if that tag has something in
its blocks at 0x74 and 0x7C. FALSE without them */
boolean __attribute__((stdcall)) object_unknown_000bf9a0(
	long object_index,
	long *tag_index,
	long *value)
{
	const byte *definition= (const byte *)(uintptr_t)
		global_tag_instances[*(const long *)OBJECT_DATA(object_index)&0xFFFF].address;
	const byte *first;
	const byte *second;
	long first_index= *(const long *)(definition+0x38);

	if (first_index==NONE)
	{
		return FALSE;
	}
	first= (const byte *)(uintptr_t)global_tag_instances[first_index&0xFFFF].address;
	if (*(const long *)(first+0x04)==NONE || *(const long *)(first+0x0C)==NONE)
	{
		return FALSE;
	}
	second= (const byte *)(uintptr_t)global_tag_instances[*(const long *)(first+0x04)&0xFFFF].address;
	*tag_index= *(const long *)(first+0x04);
	*value= *(const long *)(first+0x0C);

	return *(const long *)(second+0x74)>0 && *(const long *)(second+0x7C)>0;
}

/* 0x00113870: twenty bytes from 0x80 of what an object keeps at its short at
0x33E: zeros, and 1, 1, 2, 1 and 1 at 0x82, 0x86, 0x8A, 0x8E and 0x92 */
void object_unknown_00113870(
	long object_index)
{
	byte *object= OBJECT_DATA(object_index);
	byte *data= object+*(const short *)(object+0x33E);
	long index;

	for (index= 0x80; index<0x94; index++)
	{
		data[index]= 0;
	}
	data[0x82]= 1;
	data[0x86]= 1;
	data[0x8A]= 2;
	data[0x8E]= 1;
	data[0x92]= 1;
}
