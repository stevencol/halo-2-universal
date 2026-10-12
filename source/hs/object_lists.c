/*
OBJECT_LISTS.C

the lists of objects that the scripts make. Worked out from halo2ship.exe;
the name of the file is a guess, by the names that the game gives to its two
tables of data ("object list header", "list object").
*/

#include "cseries/cseries.h"
#include "memory/data.h"
#include "objects/objects.h"
#include "saved games/game_state.h"

/* ---------- globals */

/* 0x004f55d8, 0x004f55d4, of hs.c: the lists ("object list header": 0x30 of
0xC bytes) and what is in them ("list object": 0x80) */
extern struct data_array *object_list_header_data;
extern struct data_array *list_object_data;

/* 0x00296270: a table of data of another make, in the memory of the state of
the game: by its name and how many things it has room for */
struct data_array *__attribute__((stdcall)) reference_list_new(const char *name, long maximum_count, long unknown);

/* ---------- code */

/* 0x001ded00: the lists of objects make their two tables, in the memory of
the state of the game: the lists with data_new, which the game has written
here, and what is in them with reference_list_new */
void object_lists_initialize(
	void)
{
	object_list_header_data= data_new("object list header", 0x30, 0xC, 0, game_state_allocation);
	list_object_data= reference_list_new("list object", 0x80, 0);
}

/* 0x002963b0, of reference_lists.c */
void reference_list_delete_all(long first_reference_index, struct data_array *references);

/* 0x001deed0: each list of objects that nothing uses (the short at 4 of its
header counts who does) is deleted: its objects, which are a list of the
other table from the number at 8 of the header, and its header. It is done
each tick after the threads of the scripts have run */
void object_lists_delete_unused(
	void)
{
	long list_index;

	for (list_index= data_next_index(object_list_header_data, NONE); list_index!=NONE;
		list_index= data_next_index(object_list_header_data, list_index))
	{
		const byte *header= (const byte *)object_list_header_data->data+(list_index&0xFFFF)*0xC;

		if (*(const short *)(header+4)==0)
		{
			reference_list_delete_all(*(const long *)(header+8), list_object_data);
			datum_delete(object_list_header_data, list_index);
		}
	}
}

enum
{
	/* of the header of a list: how many use it, how many objects it has (two
	shorts), and the first of them in the other table, or NONE */
	OBJECT_LIST_HEADER_SIZE= 0xC,
	OBJECT_LIST_HEADER_REFERENCE_COUNT_OFFSET= 4,
	OBJECT_LIST_HEADER_COUNT_OFFSET= 6,
	OBJECT_LIST_HEADER_FIRST_OFFSET= 8
};

#define OBJECT_LIST_HEADER_GET(list_index) \
	((byte *)object_list_header_data->data+((list_index)&0xFFFF)*OBJECT_LIST_HEADER_SIZE)

/* 0x00296330 and 0x00296370, of reference_lists.c */
void reference_list_remove(long *first_reference_index, long index, struct data_array *references);
boolean reference_list_contains(const long *first_reference_index, const struct data_array *references, long index);

/* 0x001ded60: a new list, with no objects: its index, or NONE if there is no
room, even after the lists that nothing uses are deleted. How many use it is
not set here */
long object_list_new(
	void)
{
	long list_index= datum_new(object_list_header_data);

	if (list_index==NONE)
	{
		object_lists_delete_unused();
		list_index= datum_new(object_list_header_data);
	}
	if (list_index!=NONE)
	{
		byte *header= OBJECT_LIST_HEADER_GET(list_index);

		*(short *)(header+OBJECT_LIST_HEADER_COUNT_OFFSET)= 0;
		*(long *)(header+OBJECT_LIST_HEADER_FIRST_OFFSET)= NONE;
	}

	return list_index;
}

/* 0x001dedb0: an object is put first in a list. The list counts one more
even if there was no room for it in the table */
void object_list_add(
	long list_index,
	long object_index)
{
	byte *header= OBJECT_LIST_HEADER_GET(list_index);
	long reference_index= datum_new(list_object_data);

	if (reference_index!=NONE)
	{
		byte *reference= (byte *)list_object_data->data+(reference_index&0xFFFF)*list_object_data->size;

		*(long *)(reference+4)= object_index;
		*(long *)(reference+8)= *(long *)(header+OBJECT_LIST_HEADER_FIRST_OFFSET);
		*(long *)(header+OBJECT_LIST_HEADER_FIRST_OFFSET)= reference_index;
	}
	(*(short *)(header+OBJECT_LIST_HEADER_COUNT_OFFSET))++;
}

/* 0x001dee00: an object is taken out of a list; whether it was in it */
boolean object_list_remove(
	long list_index,
	long object_index)
{
	byte *header= OBJECT_LIST_HEADER_GET(list_index);
	long *first= (long *)(header+OBJECT_LIST_HEADER_FIRST_OFFSET);

	if (reference_list_contains(first, list_object_data, object_index))
	{
		reference_list_remove(first, object_index, list_object_data);
		(*(short *)(header+OBJECT_LIST_HEADER_COUNT_OFFSET))--;

		return TRUE;
	}

	return FALSE;
}

/* 0x001dee80: the next object of a walk over a list, or NONE when it is
over: the walk is a number that starts as the first of the list. The one
after is asked of the processor ahead of time */
long object_list_get_next(
	long *iterator)
{
	const byte *reference;
	long next_reference_index;

	if (*iterator==NONE)
	{
		return NONE;
	}
	reference= (const byte *)list_object_data->data+(*iterator&0xFFFF)*list_object_data->size;
	next_reference_index= *(const long *)(reference+8);
	if (next_reference_index!=NONE)
	{
		__builtin_prefetch((const byte *)list_object_data->data+(next_reference_index&0xFFFF)*list_object_data->size);
	}
	*iterator= next_reference_index;

	return *(const long *)(reference+4);
}

/* 0x001defb0: how many objects of a list do not have the bit 2 of their byte
at 0x10A (the dead, perhaps: it is what a script counts); none for no list */
short object_list_count(
	long list_index)
{
	short count= 0;

	if (list_index!=NONE)
	{
		long iterator= *(const long *)(OBJECT_LIST_HEADER_GET(list_index)+OBJECT_LIST_HEADER_FIRST_OFFSET);
		long object_index;

		while ((object_index= object_list_get_next(&iterator))!=NONE)
		{
			const byte *object= (const byte *)((struct object_header_datum *)object_header_data->data)[object_index&0xFFFF].datum;

			if (!((object[0x10A]>>2)&1))
			{
				count++;
			}
		}
	}

	return count;
}
