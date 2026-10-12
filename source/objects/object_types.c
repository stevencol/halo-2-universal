/*
OBJECT_TYPES.C

what each type of object does. Worked out from halo2ship.exe; the name of the
file is a guess, by the one that the first game has.
*/

#include "object_types.h"

#include "cache/cache_files.h"
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

typedef void (__attribute__((stdcall)) *object_type_placement_proc)(long *placement);
typedef boolean (__attribute__((stdcall)) *object_type_bool_proc2)(long object_index, long unknown0, long unknown1);
typedef boolean (__attribute__((stdcall)) *object_type_bool_proc3)(long object_index, long unknown0, float *unknown2,
	boolean *unknown3);
typedef void (__attribute__((stdcall)) *object_type_proc3)(long object_index, long unknown0, long unknown1,
	long unknown2);

/* 0x001088a0: the type of object that a tag is of, by its group; NONE if it
is of none of the thirteen */
long __attribute__((stdcall)) object_type_from_tag(
	short tag_index)
{
	unsigned long group_tag= global_tag_instances[tag_index].group_tag;
	long type;

	for (type= 0; type<NUMBER_OF_OBJECT_TYPES; type++)
	{
		if (*(const unsigned long *)(object_type_definitions[type]+OBJECT_TYPE_GROUP_TAG_OFFSET)==group_tag)
		{
			return type;
		}
	}

	return NONE;
}

/* 0x00108a40: the types that a tag is of do what they have at 0x28 with
something that starts with the tag: what an object is made from. The game
does not look whether the tag is of a type */
void object_types_call_unknown28(
	long *placement)
{
	byte *const *types= (byte *const *)(object_type_definitions[object_type_from_tag((short)placement[0])]+
		OBJECT_TYPE_TYPES_OFFSET);
	short index;

	for (index= 0; types[index]; index++)
	{
		object_type_placement_proc proc= *(object_type_placement_proc *)(types[index]+OBJECT_TYPE_UNKNOWN28_OFFSET);

		if (proc)
		{
			proc(placement);
		}
	}
}

/* 0x00108a90: whether every type of an object that has a function at 0x2C
says yes to it, with two things more; after one says no the others are not
asked */
boolean __attribute__((stdcall)) object_types_call_unknown2C(
	long object_index,
	long unknown0,
	long unknown1)
{
	byte *const *types= object_types_get(object_index);
	boolean result= TRUE;
	short index;

	for (index= 0; types[index]; index++)
	{
		object_type_bool_proc2 proc= *(object_type_bool_proc2 *)(types[index]+OBJECT_TYPE_UNKNOWN2C_OFFSET);

		if (proc)
		{
			result= result && proc(object_index, unknown0, unknown1);
		}
	}

	return result;
}

/* 0x00108d30: the types of an object are asked what they have at 0x48, with
two things more, until one says yes: whether one did */
boolean __attribute__((stdcall)) object_types_call_unknown48(
	long object_index,
	long unknown1,
	long unknown0)
{
	byte *const *types= object_types_get(object_index);
	boolean result= FALSE;
	short index;

	for (index= 0; types[index] && !result; index++)
	{
		object_type_bool_proc2 proc= *(object_type_bool_proc2 *)(types[index]+OBJECT_TYPE_UNKNOWN48_OFFSET);

		if (proc)
		{
			result= proc(object_index, unknown0, unknown1);
		}
	}

	return result;
}

/* 0x00108d90: the same with what they have at 0x4C, which is given one
thing more and where to put a real and a byte: both start as nothing, and
are nothing again if no type says yes */
boolean __attribute__((stdcall)) object_types_call_unknown4C(
	float *unknown2,
	boolean *unknown3,
	long object_index,
	long unknown0)
{
	byte *const *types= object_types_get(object_index);
	boolean result= FALSE;
	short index;

	*unknown2= 0.0f;
	*unknown3= FALSE;
	for (index= 0; types[index]; index++)
	{
		object_type_bool_proc3 proc= *(object_type_bool_proc3 *)(types[index]+OBJECT_TYPE_UNKNOWN4C_OFFSET);

		if (proc)
		{
			result= proc(object_index, unknown0, unknown2, unknown3);
		}
		if (result)
		{
			return result;
		}
	}
	*unknown2= 0.0f;
	*unknown3= FALSE;

	return FALSE;
}

/* 0x00109290: the types of an object do what they have at 0x68, with three
things more */
void __attribute__((stdcall)) object_types_call_unknown68(
	long object_index,
	long unknown2,
	long unknown0,
	long unknown1)
{
	byte *const *types= object_types_get(object_index);
	short index;

	for (index= 0; types[index]; index++)
	{
		object_type_proc3 proc= *(object_type_proc3 *)(types[index]+OBJECT_TYPE_UNKNOWN68_OFFSET);

		if (proc)
		{
			proc(object_index, unknown0, unknown1, unknown2);
		}
	}
}

typedef boolean (__attribute__((stdcall)) *object_type_bool_proc)(long object_index);

/* 0x00108fd0: whether one of the types of an object says yes to what they
have at 0x64: all of them are asked */
boolean __attribute__((stdcall)) object_types_call_unknown64(
	long object_index)
{
	byte *const *types= object_types_get(object_index);
	boolean result= FALSE;
	short index;

	for (index= 0; types[index]; index++)
	{
		object_type_bool_proc proc= *(object_type_bool_proc *)(types[index]+OBJECT_TYPE_UNKNOWN64_OFFSET);

		if (proc && proc(object_index))
		{
			result= TRUE;
		}
	}

	return result;
}

/* 0x00109050: the types of an object do what they have at 0x6C, with three
things more */
void __attribute__((stdcall)) object_types_call_unknown6C(
	long object_index,
	long unknown0,
	long unknown1,
	long unknown2)
{
	byte *const *types= object_types_get(object_index);
	short index;

	for (index= 0; types[index]; index++)
	{
		object_type_proc3 proc= *(object_type_proc3 *)(types[index]+OBJECT_TYPE_UNKNOWN6C_OFFSET);

		if (proc)
		{
			proc(object_index, unknown0, unknown1, unknown2);
		}
	}
}
