/*
DATA.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

Not decompiled yet: data_new (0x0016b570) and data_dispose (0x0016b5d0),
which use what allocated the array.
*/

#include "data.h"

#include <string.h>

/* ---------- macros */

#define DATUM_GET(data, absolute_index) \
	((struct datum_header *)((byte *)(data)->data+(data)->size*(absolute_index)))
#define DATUM_INDEX(identifier, absolute_index) \
	((long)((unsigned long)(word)(identifier)<<16 | (unsigned long)(absolute_index)))
#define DATUM_INDEX_TO_ABSOLUTE_INDEX(index) ((long)((index)&0xFFFF))
#define DATUM_INDEX_TO_IDENTIFIER(index) ((short)((index)>>16))

/* ---------- code */

/* 0x0016b650 */
void data_initialize_disconnected(
	struct data_array *data,
	const char *name,
	long maximum_count,
	long size,
	byte alignment_bits,
	void *allocation,
	unsigned long *in_use_bit_vector)
{
	memset(data, 0, sizeof(*data));
	strncpy(data->name, name, sizeof(data->name));
	data->name[DATA_ARRAY_NAME_LENGTH]= 0;
	data->flags|= FLAG(_data_array_unknown0_bit) | FLAG(_data_array_unknown1_bit);
	data->maximum_count= maximum_count;
	data->size= size;
	data->data= NULL;
	data->valid= FALSE;
	data->alignment_bits= alignment_bits;
	data->signature= DATA_ARRAY_SIGNATURE;
	data->in_use_bit_vector= in_use_bit_vector;
	data->allocation= allocation;

	return;
}

/* 0x0016b5f0: the elements follow the array, at the first aligned address,
and the vector of bits follows the elements */
void data_initialize(
	struct data_array *data,
	const char *name,
	long maximum_count,
	long size,
	long alignment_bits,
	void *allocation)
{
	unsigned long alignment_mask= (1UL<<alignment_bits)-1;
	byte *datums= (byte *)(((unsigned long)data+sizeof(struct data_array)+alignment_mask) & ~alignment_mask);

	data_initialize_disconnected(data, name, maximum_count, size, (byte)alignment_bits, allocation,
		(unsigned long *)(datums+maximum_count*size));
	data->flags&= ~(FLAG(_data_array_unknown0_bit) | FLAG(_data_array_unknown1_bit));
	data->data= datums;
	memset(data->in_use_bit_vector, 0, BIT_VECTOR_SIZE_IN_LONGS(maximum_count)*sizeof(unsigned long));

	return;
}

/* 0x0016b6b0: gives an array made with data_initialize_disconnected its
elements, which have their identifiers already: those with an identifier are
in use. The name is a guess */
void data_connect(
	struct data_array *data,
	long maximum_count,
	void *datums)
{
	long absolute_index;

	data->flags&= ~FLAG(_data_array_unknown1_bit);
	data->maximum_count= maximum_count;
	data->data= datums;
	data->valid= TRUE;
	data->first_unallocated= 0;
	data->actual_count= 0;
	data->next_index= maximum_count;
	strncpy((char *)&data->next_identifier, data->name, sizeof(data->next_identifier));
	data->next_identifier|= 0x8000;

	for (absolute_index= 0; absolute_index<data->maximum_count; absolute_index++)
	{
		struct datum_header *header= DATUM_GET(data, absolute_index);

		if (header->identifier)
		{
			BIT_VECTOR_OR_FLAG(data->in_use_bit_vector, absolute_index);
			data->first_unallocated= absolute_index+1;
			data->actual_count++;
		}
		else
		{
			if (TEST_FLAG(data->flags, _data_array_fill_deleted_bit))
			{
				memset(header, 0xBA, data->size);
				header->identifier= 0;
			}
			BIT_VECTOR_AND_FLAG(data->in_use_bit_vector, absolute_index);
			if (absolute_index<data->next_index)
			{
				data->next_index= absolute_index;
			}
		}
	}

	return;
}

/* 0x0016b790 */
void data_make_valid(
	struct data_array *data)
{
	data->valid= TRUE;
	data_delete_all(data);

	return;
}

/* 0x0016b7a0 */
void data_delete_all(
	struct data_array *data)
{
	long absolute_index;

	data->first_unallocated= 0;
	data->actual_count= 0;
	data->next_index= 0;
	/* the identifiers start from the first two characters of the name */
	strncpy((char *)&data->next_identifier, data->name, sizeof(data->next_identifier));
	data->next_identifier|= 0x8000;

	if (TEST_FLAG(data->flags, _data_array_fill_deleted_bit))
	{
		memset(data->data, 0xBA, data->size*data->maximum_count);
	}
	for (absolute_index= 0; absolute_index<data->maximum_count; absolute_index++)
	{
		DATUM_GET(data, absolute_index)->identifier= 0;
	}
	memset(data->in_use_bit_vector, 0, BIT_VECTOR_SIZE_IN_LONGS(data->maximum_count)*sizeof(unsigned long));

	return;
}

/* 0x0016b840 */
long datum_new(
	struct data_array *data)
{
	long first_unallocated= data->first_unallocated;
	long absolute_index;
	struct datum_header *header;

	/* the first element not in use among those that were ever in use */
	for (absolute_index= data->next_index; absolute_index<data->first_unallocated; absolute_index++)
	{
		if (!BIT_VECTOR_TEST_FLAG(data->in_use_bit_vector, absolute_index))
		{
			break;
		}
	}
	if (absolute_index>=data->first_unallocated)
	{
		/* or the one after them */
		if (first_unallocated>=data->maximum_count)
		{
			return NONE;
		}
		absolute_index= first_unallocated;
	}
	if (absolute_index==NONE)
	{
		return NONE;
	}

	header= DATUM_GET(data, absolute_index);
	BIT_VECTOR_OR_FLAG(data->in_use_bit_vector, absolute_index);
	data->actual_count++;
	data->next_index= absolute_index+1;
	if (data->first_unallocated<=absolute_index)
	{
		data->first_unallocated= absolute_index+1;
	}
	datum_initialize(data, header);

	return DATUM_INDEX(header->identifier, absolute_index);
}

/* 0x0016b910 */
long datum_new_at_index(
	struct data_array *data,
	long index)
{
	long absolute_index= DATUM_INDEX_TO_ABSOLUTE_INDEX(index);
	short identifier= DATUM_INDEX_TO_IDENTIFIER(index);
	struct datum_header *header;

	if (absolute_index>=data->maximum_count)
	{
		return NONE;
	}
	header= DATUM_GET(data, absolute_index);
	if (header->identifier)
	{
		return NONE;
	}

	BIT_VECTOR_OR_FLAG(data->in_use_bit_vector, absolute_index);
	data->actual_count++;
	if (data->first_unallocated<=absolute_index)
	{
		data->first_unallocated= absolute_index+1;
	}
	datum_initialize(data, header);
	header->identifier= identifier;

	return DATUM_INDEX(identifier, absolute_index);
}

/* 0x0016b990 */
long datum_new_at_absolute_index(
	struct data_array *data,
	long absolute_index)
{
	struct datum_header *header;

	if (absolute_index<0 || absolute_index>=data->maximum_count)
	{
		return NONE;
	}
	header= DATUM_GET(data, absolute_index);
	if (header->identifier)
	{
		return NONE;
	}

	BIT_VECTOR_OR_FLAG(data->in_use_bit_vector, absolute_index);
	data->actual_count++;
	if (data->first_unallocated<=absolute_index)
	{
		data->first_unallocated= absolute_index+1;
	}
	datum_initialize(data, header);

	return DATUM_INDEX(header->identifier, absolute_index);
}

/* 0x0016ba00 */
void datum_initialize(
	struct data_array *data,
	struct datum_header *header)
{
	memset(header, 0, data->size);
	header->identifier= data->next_identifier++;
	if (data->next_identifier==NONE)
	{
		data->next_identifier= (short)0x8000;
	}

	return;
}

/* 0x0016ba40 */
void datum_delete(
	struct data_array *data,
	long index)
{
	long absolute_index= DATUM_INDEX_TO_ABSOLUTE_INDEX(index);
	struct datum_header *header= DATUM_GET(data, absolute_index);

	if (TEST_FLAG(data->flags, _data_array_fill_deleted_bit))
	{
		memset(header, 0xBA, data->size);
	}
	BIT_VECTOR_AND_FLAG(data->in_use_bit_vector, absolute_index);
	header->identifier= 0;

	if (absolute_index<data->next_index)
	{
		data->next_index= absolute_index;
	}
	if (absolute_index+1==data->first_unallocated)
	{
		do
		{
			header= (struct datum_header *)((byte *)header-data->size);
			data->first_unallocated--;
		}
		while (data->first_unallocated>0 && !header->identifier);
	}
	data->actual_count--;

	return;
}

/* 0x0016bae0 */
void *datum_try_and_get(
	struct data_array *data,
	long index)
{
	struct datum_header *header;

	if (index!=NONE && DATUM_INDEX_TO_ABSOLUTE_INDEX(index)<data->first_unallocated)
	{
		header= DATUM_GET(data, DATUM_INDEX_TO_ABSOLUTE_INDEX(index));
		if (header->identifier && header->identifier==DATUM_INDEX_TO_IDENTIFIER(index))
		{
			return header;
		}
	}

	return NULL;
}

/* 0x0016bb20 */
void *datum_try_and_get_absolute(
	struct data_array *data,
	long absolute_index)
{
	struct datum_header *header;

	if (absolute_index!=NONE && absolute_index>=0 && absolute_index<data->first_unallocated)
	{
		header= DATUM_GET(data, absolute_index);
		if (header->identifier)
		{
			return header;
		}
	}

	return NULL;
}

/* 0x0016bb50 */
long datum_absolute_index_to_index(
	struct data_array *data,
	long absolute_index)
{
	if (absolute_index==NONE)
	{
		return NONE;
	}

	return DATUM_INDEX(DATUM_GET(data, absolute_index)->identifier, absolute_index);
}

/* 0x0016bb70: the next element in use; the iterator starts with an absolute
index of NONE */
void *data_iterator_next(
	struct data_iterator *iterator)
{
	struct data_array *data= iterator->data;
	long absolute_index= data_next_absolute_index(data, iterator->absolute_index+1);
	struct datum_header *header;

	if (absolute_index==NONE)
	{
		iterator->absolute_index= data->maximum_count;
		iterator->index= NONE;
		return NULL;
	}

	header= DATUM_GET(data, absolute_index);
	iterator->absolute_index= absolute_index;
	iterator->index= DATUM_INDEX(header->identifier, absolute_index);

	return header;
}

/* 0x0016bbc0: the index of the element in use after one; the first, after
NONE */
long data_next_index(
	struct data_array *data,
	long index)
{
	long absolute_index= data_next_absolute_index(data,
		index==NONE ? 0 : DATUM_INDEX_TO_ABSOLUTE_INDEX(index)+1);

	if (absolute_index==NONE)
	{
		return NONE;
	}

	return DATUM_INDEX(DATUM_GET(data, absolute_index)->identifier, absolute_index);
}

/* 0x0016bc00: the first element in use from an absolute index on */
long data_next_absolute_index(
	struct data_array *data,
	long absolute_index)
{
	if (absolute_index<0)
	{
		return NONE;
	}
	for (; absolute_index<data->first_unallocated; absolute_index++)
	{
		if (BIT_VECTOR_TEST_FLAG(data->in_use_bit_vector, absolute_index))
		{
			return absolute_index;
		}
	}

	return NONE;
}

/* 0x0016bc40 */
long data_last_index(
	struct data_array *data)
{
	long index= NONE;
	long next_index;

	while ((next_index= data_next_index(data, index))!=NONE)
	{
		index= next_index;
	}

	return index;
}

/* 0x0016bcc0: the index of the element in use before one; the last, before
NONE. The game's function reads the identifier of the element before the
first one when there is none in use, and does nothing with it */
long data_prev_index(
	struct data_array *data,
	long index)
{
	long absolute_index= (index==NONE ? data->first_unallocated : DATUM_INDEX_TO_ABSOLUTE_INDEX(index))-1;

	if (absolute_index>=data->first_unallocated)
	{
		return NONE;
	}
	for (; absolute_index>=0; absolute_index--)
	{
		struct datum_header *header= DATUM_GET(data, absolute_index);

		if (header->identifier)
		{
			return DATUM_INDEX(header->identifier, absolute_index);
		}
	}

	return NONE;
}
