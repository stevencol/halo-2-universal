/*
OBJECTS.H

The objects of the game world. Each has a header in a data array, as in
Halo 1, and the header has the address of the object's data, of a size that
depends on the type of object.

Only what the functions decompiled so far use is worked out.
*/

#ifndef __OBJECTS_H
#define __OBJECTS_H

#include "cseries/cseries.h"
#include "math/real_math.h"
#include "memory/data.h"

/* ---------- constants */

enum
{
	_object_header_unknown0_bit,
	/* object_wake sets it; the name is a guess */
	_object_header_awake_bit
};

/* ---------- constants */

/* what an iterator that object_iterator_new made has at its end */
#define OBJECT_ITERATOR_SIGNATURE 0x86868686UL

enum
{
	/* an object that object_get_first_unflagged_ancestor goes past */
	_object_unknown26_bit= 26
};

/* ---------- structures */

struct object_datum;

struct object_header_datum
{
	short identifier;
	byte flags;
	byte type;
	short cluster_index;
	short data_size;
	struct object_datum *datum;
};

/* where an object is in the level; what its two parts are is not worked out */
struct location
{
	long unknown[2];
};

struct object_datum
{
	byte unknown0[4];
	/* what the bits are is not known */
	unsigned long flags;
	byte unknown8[0xC];
	/* the object that this one is attached to, and the node of it; NONE for
	an object attached to nothing */
	long parent_object_index;
	char parent_node_index;
	byte unknown19[0xF];
	struct location location;
	byte unknown30[0x34];
	/* in the world, or from the parent's node */
	real_point3d position;
	real_vector3d forward;
	real_vector3d up;
	byte unknown88[0x8E];
	/* the offset in the object of the matrix of each of its nodes */
	short node_matrices_offset;
};

/* goes through the objects of some types whose header has some flags */
struct object_iterator
{
	unsigned long type_mask;
	byte header_flags;
	byte pad;
	/* of the next header to look at; 0 to start */
	short absolute_index;
	/* of the object that object_iterator_next returned */
	long index;
	/* OBJECT_ITERATOR_SIGNATURE */
	unsigned long signature;
};

typedef char verify_object_header_datum_size[
	sizeof(struct object_header_datum) == 0xC ? 1 : -1];
typedef char verify_object_iterator_size[
	sizeof(struct object_iterator) == 0x10 ? 1 : -1];
typedef char verify_object_datum_location_offset[
	offsetof(struct object_datum, location) == 0x28 ? 1 : -1];
typedef char verify_object_datum_position_offset[
	offsetof(struct object_datum, position) == 0x64 ? 1 : -1];
typedef char verify_object_datum_node_matrices_offset_offset[
	offsetof(struct object_datum, node_matrices_offset) == 0x116 ? 1 : -1];

/* ---------- prototypes/OBJECTS.C */

void object_wake(long object_index);
void *object_try_and_get_and_verify_type(long object_index, unsigned long type_mask);
void *object_get_and_verify_type(long object_index, unsigned long type_mask);
void object_iterator_new(struct object_iterator *iterator, unsigned long type_mask, byte header_flags);
long object_get_first_unflagged_ancestor(long object_index);
void *object_iterator_next(struct object_iterator *iterator);
long object_get_ultimate_parent(long object_index);
void object_get_origin(long object_index, real_point3d *origin);
real_matrix4x3 *object_get_world_matrix(long object_index, real_matrix4x3 *matrix);
void object_get_location(long object_index, struct location *location);

/* ---------- globals */

extern struct data_array *object_header_data;

#endif
