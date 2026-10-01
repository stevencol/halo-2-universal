/*
DATA.H

The data arrays: tables of elements of one size, each with an identifier,
which most of the game's systems keep their state in (the objects, the
players, the effects and so on). An index of an element is its identifier in
the high 16 bits and its place in the table, the absolute index, in the low
16 bits.

As in Halo 1 (memory/data.h of halo-ce-universal), with these differences:
the counts are 32 bits, a vector of bits tells the elements in use, the
elements can be aligned, and an array remembers what allocated it.
*/

#ifndef __DATA_H
#define __DATA_H

#include "cseries/cseries.h"

/* ---------- constants */

#define DATA_ARRAY_SIGNATURE 'd@t@'

enum
{
	DATA_ARRAY_NAME_LENGTH= 31
};

enum
{
	/* data_initialize_disconnected sets the first two and data_initialize
	clears them */
	_data_array_unknown0_bit,
	_data_array_unknown1_bit,
	/* data_new made the array, so data_dispose frees it */
	_data_array_allocated_bit,
	/* an element that is deleted is filled with 0xBA */
	_data_array_fill_deleted_bit
};

/* ---------- structures */

struct data_array
{
	char name[DATA_ARRAY_NAME_LENGTH+1];
	long maximum_count;
	long size;
	byte alignment_bits;
	boolean valid;
	word flags;
	unsigned long signature;
	void *allocation;
	/* no element before this one is free */
	long next_index;
	/* no element from this one on is in use */
	long first_unallocated;
	long actual_count;
	short next_identifier;
	word pad;
	void *data;
	unsigned long *in_use_bit_vector;
};

struct datum_header
{
	short identifier;
};

struct data_iterator
{
	struct data_array *data;
	long index;
	long absolute_index;
};

typedef char verify_data_array_size[
	sizeof(struct data_array) == 0x4C ? 1 : -1];
typedef char verify_data_array_next_index_offset[
	offsetof(struct data_array, next_index) == 0x34 ? 1 : -1];
typedef char verify_data_array_data_offset[
	offsetof(struct data_array, data) == 0x44 ? 1 : -1];
typedef char verify_data_iterator_size[
	sizeof(struct data_iterator) == 0xC ? 1 : -1];

/* ---------- prototypes/DATA.C */

void data_initialize_disconnected(struct data_array *data, const char *name, long maximum_count, long size,
	byte alignment_bits, void *allocation, unsigned long *in_use_bit_vector);
void data_initialize(struct data_array *data, const char *name, long maximum_count, long size,
	long alignment_bits, void *allocation);
void data_make_valid(struct data_array *data);
void data_delete_all(struct data_array *data);

long datum_new(struct data_array *data);
long datum_new_at_index(struct data_array *data, long index);
long datum_new_at_absolute_index(struct data_array *data, long absolute_index);
void datum_initialize(struct data_array *data, struct datum_header *header);
void datum_delete(struct data_array *data, long index);

void *datum_try_and_get(struct data_array *data, long index);
void *datum_try_and_get_absolute(struct data_array *data, long absolute_index);
long datum_absolute_index_to_index(struct data_array *data, long absolute_index);

void *data_iterator_next(struct data_iterator *iterator);
long data_next_index(struct data_array *data, long index);
long data_next_absolute_index(struct data_array *data, long absolute_index);
long data_last_index(struct data_array *data);
long data_prev_index(struct data_array *data, long index);

#endif
