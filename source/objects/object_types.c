/*
OBJECT_TYPES.C

what each type of object does. Worked out from halo2ship.exe; the name of the
file is a guess, by the one that the first game has.
*/

#include "object_types.h"

#include "objects/objects.h"

/* ---------- structures */

typedef void (__attribute__((stdcall)) *object_type_proc)(long object_index);

/* ---------- globals */

/* 0x00468630: what each type of object is */
byte *object_type_definitions[NUMBER_OF_OBJECT_TYPES];

/* ---------- code */

/* 0x00108e80: the types that the type of an object is of are told, from the
first to itself, that the object lets go of the thing that the simulation
tells of it: each that has a function for it at 0x54 */
void object_types_call_unknown54(
	long object_index)
{
	const struct object_header_datum *header=
		(const struct object_header_datum *)object_header_data->data+(object_index&0xFFFF);
	byte *const *types= (byte *const *)(object_type_definitions[*((const signed char *)header->datum+0xAA)]+
		OBJECT_TYPE_TYPES_OFFSET);
	short index;

	for (index= 0; types[index]; index++)
	{
		object_type_proc proc= *(object_type_proc *)(types[index]+OBJECT_TYPE_UNKNOWN54_OFFSET);

		if (proc)
		{
			proc(object_index);
		}
	}
}
