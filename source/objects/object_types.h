/*
OBJECT_TYPES.H

what each type of object does. Worked out from halo2ship.exe; the name of the
file is a guess, by the one that the first game has.
*/

#ifndef __OBJECT_TYPES_H
#define __OBJECT_TYPES_H

#include "cseries/cseries.h"

/* ---------- constants */

enum
{
	/* how many types of object the table is declared for: a guess */
	NUMBER_OF_OBJECT_TYPES= 13,
	/* of what a type of object is: the function for an object that lets go
	of the thing that the simulation tells of it, and the types that the
	type is of, from the first to itself, with nothing after the last */
	OBJECT_TYPE_UNKNOWN54_OFFSET= 0x54,
	/* the group of the tags of the type, and five more functions: for
	something that starts with a tag, one that every type has to say yes
	to, two that go on until a type says yes, and one with three things
	more */
	OBJECT_TYPE_GROUP_TAG_OFFSET= 0x4,
	OBJECT_TYPE_UNKNOWN28_OFFSET= 0x28,
	OBJECT_TYPE_UNKNOWN2C_OFFSET= 0x2C,
	OBJECT_TYPE_UNKNOWN48_OFFSET= 0x48,
	OBJECT_TYPE_UNKNOWN4C_OFFSET= 0x4C,
	OBJECT_TYPE_UNKNOWN68_OFFSET= 0x68,
	/* and two more: one that all the types are asked, and another with
	three things more */
	OBJECT_TYPE_UNKNOWN64_OFFSET= 0x64,
	/* the function that sets a type up, how many types a type can be of,
	and the type that comes after it in the list of them all */
	OBJECT_TYPE_INITIALIZE_OFFSET= 0x10,
	MAXIMUM_OBJECT_TYPE_TYPES= 16,
	OBJECT_TYPE_NEXT_OFFSET= 0xC4,
	OBJECT_TYPE_UNKNOWN6C_OFFSET= 0x6C,
	/* six more functions of a type that are given an object and nothing
	else: what each is for is not known */
	OBJECT_TYPE_UNKNOWN20_OFFSET= 0x20,
	OBJECT_TYPE_UNKNOWN24_OFFSET= 0x24,
	OBJECT_TYPE_UNKNOWN38_OFFSET= 0x38,
	OBJECT_TYPE_UNKNOWN3C_OFFSET= 0x3C,
	OBJECT_TYPE_UNKNOWN50_OFFSET= 0x50,
	OBJECT_TYPE_UNKNOWN78_OFFSET= 0x78,
	/* two more of those, and six that are given one thing more, or two */
	OBJECT_TYPE_UNKNOWN40_OFFSET= 0x40,
	OBJECT_TYPE_UNKNOWN44_OFFSET= 0x44,
	OBJECT_TYPE_UNKNOWN30_OFFSET= 0x30,
	OBJECT_TYPE_UNKNOWN58_OFFSET= 0x58,
	OBJECT_TYPE_UNKNOWN5C_OFFSET= 0x5C,
	OBJECT_TYPE_UNKNOWN70_OFFSET= 0x70,
	OBJECT_TYPE_UNKNOWN74_OFFSET= 0x74,
	OBJECT_TYPE_UNKNOWN7C_OFFSET= 0x7C,
	OBJECT_TYPE_TYPES_OFFSET= 0x84
};

/* ---------- globals */

extern byte *object_type_definitions[NUMBER_OF_OBJECT_TYPES];
extern byte *object_type_definition_list;

/* ---------- prototypes/OBJECT_TYPES.C */

void object_types_call_unknown54(long object_index);
void object_types_call_unknown20(long object_index);
void object_types_call_unknown24(long object_index);
void object_types_call_unknown38(long object_index);
void object_types_call_unknown3C(long object_index);
void object_types_call_unknown50(long object_index);
void object_types_call_unknown78(long object_index);
void __attribute__((stdcall)) object_types_call_unknown40(long object_index);
void object_types_call_unknown44(long object_index);
void __attribute__((stdcall)) object_types_call_unknown30(long object_index, long unknown0);
void __attribute__((stdcall)) object_types_call_unknown58(long object_index, long unknown0);
void __attribute__((stdcall)) object_types_call_unknown5C(long object_index, long unknown0);
void __attribute__((stdcall)) object_types_call_unknown70(long object_index, long unknown0);
void __attribute__((stdcall)) object_types_call_unknown74(long object_index, long unknown0, long unknown1);
void __attribute__((stdcall)) object_types_call_unknown7C(long object_index, long unknown0, long unknown1);
void object_types_initialize(void);
long __attribute__((stdcall)) object_type_from_tag(short tag_index);
boolean __attribute__((stdcall)) object_types_call_unknown64(long object_index);
void __attribute__((stdcall)) object_types_call_unknown6C(long object_index, long unknown0, long unknown1,
	long unknown2);
void object_types_call_unknown28(long *placement);
boolean __attribute__((stdcall)) object_types_call_unknown2C(long object_index, long unknown0, long unknown1);
boolean __attribute__((stdcall)) object_types_call_unknown48(long object_index, long unknown1, long unknown0);
boolean __attribute__((stdcall)) object_types_call_unknown4C(float *unknown2, boolean *unknown3, long object_index,
	long unknown0);
void __attribute__((stdcall)) object_types_call_unknown68(long object_index, long unknown2, long unknown0,
	long unknown1);

#endif
