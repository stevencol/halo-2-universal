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

/* each of the types that the type of an object is of, from the first to
itself, does one of the things that it does for an object: those that have
a function for it at a place. The game has it written in each of the
functions that do one */
static inline __attribute__((always_inline)) void object_types_call(
	long object_index,
	long proc_offset)
{
	const struct object_header_datum *header=
		(const struct object_header_datum *)object_header_data->data+(object_index&0xFFFF);
	byte *const *types= (byte *const *)(object_type_definitions[*((const signed char *)header->datum+0xAA)]+
		OBJECT_TYPE_TYPES_OFFSET);
	short index;

	for (index= 0; types[index]; index++)
	{
		object_type_proc proc= *(object_type_proc *)(types[index]+proc_offset);

		if (proc)
		{
			proc(object_index);
		}
	}
}

/* 0x00108e80: the types of an object are told that it lets go of the thing
that the simulation tells of it */
void object_types_call_unknown54(
	long object_index)
{
	object_types_call(object_index, OBJECT_TYPE_UNKNOWN54_OFFSET);
}

/* 0x00108960: the types of an object do what they have at 0x20 */
void object_types_call_unknown20(
	long object_index)
{
	object_types_call(object_index, OBJECT_TYPE_UNKNOWN20_OFFSET);
}

/* 0x001089d0: the types of an object do what they have at 0x24 */
void object_types_call_unknown24(
	long object_index)
{
	object_types_call(object_index, OBJECT_TYPE_UNKNOWN24_OFFSET);
}

/* 0x00108b80: the types of an object do what they have at 0x38 */
void object_types_call_unknown38(
	long object_index)
{
	object_types_call(object_index, OBJECT_TYPE_UNKNOWN38_OFFSET);
}

/* 0x00108bf0: the types of an object do what they have at 0x3C */
void object_types_call_unknown3C(
	long object_index)
{
	object_types_call(object_index, OBJECT_TYPE_UNKNOWN3C_OFFSET);
}

/* 0x00108e10: the types of an object do what they have at 0x50 */
void object_types_call_unknown50(
	long object_index)
{
	object_types_call(object_index, OBJECT_TYPE_UNKNOWN50_OFFSET);
}

/* 0x001091b0: the types of an object do what they have at 0x78 */
void object_types_call_unknown78(
	long object_index)
{
	object_types_call(object_index, OBJECT_TYPE_UNKNOWN78_OFFSET);
}

typedef void (__attribute__((stdcall)) *object_type_proc1)(long object_index, long unknown0);
typedef void (__attribute__((stdcall)) *object_type_proc2)(long object_index, long unknown0, long unknown1);

/* the types that the type of an object is of, from the first to itself, with
nothing after the last */
static inline __attribute__((always_inline)) byte *const *object_types_get(
	long object_index)
{
	const struct object_header_datum *header=
		(const struct object_header_datum *)object_header_data->data+(object_index&0xFFFF);

	return (byte *const *)(object_type_definitions[*((const signed char *)header->datum+0xAA)]+OBJECT_TYPE_TYPES_OFFSET);
}

/* object_types_call, for a function that is given one thing more, and two */
static inline __attribute__((always_inline)) void object_types_call1(
	long object_index,
	long proc_offset,
	long unknown0)
{
	byte *const *types= object_types_get(object_index);
	short index;

	for (index= 0; types[index]; index++)
	{
		object_type_proc1 proc= *(object_type_proc1 *)(types[index]+proc_offset);

		if (proc)
		{
			proc(object_index, unknown0);
		}
	}
}

static inline __attribute__((always_inline)) void object_types_call2(
	long object_index,
	long proc_offset,
	long unknown0,
	long unknown1)
{
	byte *const *types= object_types_get(object_index);
	short index;

	for (index= 0; types[index]; index++)
	{
		object_type_proc2 proc= *(object_type_proc2 *)(types[index]+proc_offset);

		if (proc)
		{
			proc(object_index, unknown0, unknown1);
		}
	}
}

/* 0x00108c60: the types of an object do what they have at 0x40 */
void __attribute__((stdcall)) object_types_call_unknown40(
	long object_index)
{
	object_types_call(object_index, OBJECT_TYPE_UNKNOWN40_OFFSET);
}

/* 0x00108cd0: the types of an object do what they have at 0x44 */
void object_types_call_unknown44(
	long object_index)
{
	object_types_call(object_index, OBJECT_TYPE_UNKNOWN44_OFFSET);
}

/* 0x00108b10: the types of an object do what they have at 0x30, with one thing more */
void __attribute__((stdcall)) object_types_call_unknown30(
	long object_index,
	long unknown0)
{
	object_types_call1(object_index, OBJECT_TYPE_UNKNOWN30_OFFSET, unknown0);
}

/* 0x00108ef0: the types of an object do what they have at 0x58, with one thing more */
void __attribute__((stdcall)) object_types_call_unknown58(
	long object_index,
	long unknown0)
{
	object_types_call1(object_index, OBJECT_TYPE_UNKNOWN58_OFFSET, unknown0);
}

/* 0x00108f60: the types of an object do what they have at 0x5C, with one thing more */
void __attribute__((stdcall)) object_types_call_unknown5C(
	long object_index,
	long unknown0)
{
	object_types_call1(object_index, OBJECT_TYPE_UNKNOWN5C_OFFSET, unknown0);
}

/* 0x001090d0: the types of an object do what they have at 0x70, with one thing more */
void __attribute__((stdcall)) object_types_call_unknown70(
	long object_index,
	long unknown0)
{
	object_types_call1(object_index, OBJECT_TYPE_UNKNOWN70_OFFSET, unknown0);
}

/* 0x00109140: the types of an object do what they have at 0x74, with two things more */
void __attribute__((stdcall)) object_types_call_unknown74(
	long object_index,
	long unknown0,
	long unknown1)
{
	object_types_call2(object_index, OBJECT_TYPE_UNKNOWN74_OFFSET, unknown0, unknown1);
}

/* 0x00109220: the types of an object do what they have at 0x7C, with two things more */
void __attribute__((stdcall)) object_types_call_unknown7C(
	long object_index,
	long unknown0,
	long unknown1)
{
	object_types_call2(object_index, OBJECT_TYPE_UNKNOWN7C_OFFSET, unknown0, unknown1);
}
