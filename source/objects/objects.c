/*
OBJECTS.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.
*/

#include "objects.h"

/* ---------- macros */

/* the header of an object that is known to be there */
#define OBJECT_HEADER_GET(object_index) \
	(&((struct object_header_datum *)object_header_data->data)[(object_index)&0xFFFF])
#define OBJECT_GET(object_index) (OBJECT_HEADER_GET(object_index)->datum)

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
