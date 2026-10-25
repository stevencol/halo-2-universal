/*
ACTORS.C

the actors: the characters that the game moves by itself. Worked out from
halo2ship.exe; the name of the file is a guess, by the name that the game
gives to its table of data ("actor").
*/

#include "cseries/cseries.h"
#include <string.h>

#include <math.h>
#include <stdint.h>

#include "cache/cache_files.h"
#include "game/game.h"
#include "main/main.h"
#include "math/real_math.h"
#include "memory/data.h"
#include "memory/hash_table.h"
#include "objects/objects.h"
#include "saved games/game_state.h"

/* ---------- constants */

enum
{
	MAXIMUM_ACTORS= 0x100,
	ACTOR_SIZE= 0x888,

	/* of the table of who has each place to fire from */
	ACTOR_FIRING_POSITION_OWNER_LIST_COUNT= 0x400,

	/* of what unknown_004f93a0 points to */
	ACTOR_UNKNOWN_SIZE= 0x640
};

/* ---------- globals */

/* 0x004f55f0: the actors ("actor": 0x100 of 0x888 bytes). 0x004f93a0: 0x640
bytes of the state of the game. 0x00557c6c: who has each place to fire from
("actor firing-position owners": a number for each key) */
struct data_array *actor_data;
byte *unknown_004f93a0;
struct hash_table *actor_firing_position_owners;

/* ---------- code */

/* 0x0025dd20: the hash of a key of the table of owners: its lowest byte, by
four. In the game it is far from the function that gives it to the table, so
it may be of another file */
unsigned long HASH_TABLE_CALLBACK actor_firing_position_owner_hash(
	long key)
{
	return (unsigned long)(key&0xFF)<<2;
}

/* 0x0025dd30: whether two keys of the table of owners are the same. Far
from the others too */
boolean HASH_TABLE_CALLBACK actor_firing_position_owner_compare(
	long key,
	long other)
{
	return key==other;
}

/* 0x001dfae0: the table of the actors is made, in the memory of the state of
the game (data_new, which the game has written here); the 0x640 bytes are
taken there (game_state_malloc, written here too), and so is the table of the
owners */
void actors_initialize(
	void)
{
	actor_data= data_new("actor", MAXIMUM_ACTORS, ACTOR_SIZE, 0, game_state_allocation);
	unknown_004f93a0= game_state_malloc(ACTOR_UNKNOWN_SIZE);
	actor_firing_position_owners= hash_table_new(game_state_allocation, MAXIMUM_ACTORS,
		"actor firing-position owners", sizeof(long), ACTOR_FIRING_POSITION_OWNER_LIST_COUNT,
		actor_firing_position_owner_hash, actor_firing_position_owner_compare);
}

/* 0x001e4990: the first thing of the block that the definition of a
character (a tag) has at 0x34, or that of the character that it comes from
(at 8), and so on; NULL if none has one. By the tags of other games of the
same engine it is what is general of the character: its kind is the short at
4 of it */
void *character_definition_get_unknown34(
	long character_index)
{
	while (character_index!=NONE)
	{
		const byte *character= (const byte *)(uintptr_t)global_tag_instances[character_index&0xFFFF].address;

		if (*(const long *)(character+0x34)>0)
		{
			return *(void *const *)(character+0x38);
		}
		character_index= *(const long *)(character+8);
	}

	return NULL;
}

/* ---------- small functions of the actors

Most of the functions that follow take, besides the actor, one or two things
on the stack that they do not use: they look like functions that a table
names, all with the same arguments. What the things are is not known */

#define ACTOR_GET(actor_index) ((byte *)actor_data->data+((actor_index)&0xFFFF)*ACTOR_SIZE)

/* 0x001a8380: an actor has 4 for its short at 0x84; always true */
boolean __attribute__((stdcall)) actor_unknown_001a8380(
	long actor_index,
	long unknown1)
{
	byte *actor= ACTOR_GET(actor_index);

	(void)unknown1;
	*(short *)(actor+0x84)= 4;

	return TRUE;
}

/* 0x001b2310: an actor has 3 for its short at 0x84; always true */
boolean __attribute__((stdcall)) actor_unknown_001b2310(
	long actor_index,
	long unknown1)
{
	byte *actor= ACTOR_GET(actor_index);

	(void)unknown1;
	*(short *)(actor+0x84)= 3;

	return TRUE;
}

/* 0x001ab3a0: an actor has the time of the game for its number at 0x1F8 */
void __attribute__((stdcall)) actor_unknown_001ab3a0(
	long actor_index,
	long unknown1)
{
	byte *actor= ACTOR_GET(actor_index);

	(void)unknown1;
	*(unsigned long *)(actor+0x1F8)= game_time_globals->time;
}

/* 0x001c04c0: an actor has the time of the game for its number at 0x1F4 */
void __attribute__((stdcall)) actor_unknown_001c04c0(
	long actor_index,
	long unknown1)
{
	byte *actor= ACTOR_GET(actor_index);

	(void)unknown1;
	*(unsigned long *)(actor+0x1F4)= game_time_globals->time;
}

/* 0x001b5790: an actor has NONE for its number at 0x344 */
void __attribute__((stdcall)) actor_unknown_001b5790(
	long actor_index,
	long unknown1)
{
	byte *actor= ACTOR_GET(actor_index);

	(void)unknown1;
	*(long *)(actor+0x344)= NONE;
}

/* 0x001b5dd0: an actor has zeros for its bytes at 0x220 and 0x221 */
void __attribute__((stdcall)) actor_unknown_001b5dd0(
	long actor_index,
	long unknown1)
{
	byte *actor= ACTOR_GET(actor_index);

	(void)unknown1;
	actor[0x220]= 0;
	actor[0x221]= 0;
}

/* 0x001b6880: an actor has 1 for its byte at 0x4A1 */
void __attribute__((stdcall)) actor_unknown_001b6880(
	long actor_index,
	long unknown1,
	long unknown2)
{
	byte *actor= ACTOR_GET(actor_index);

	(void)unknown1;
	(void)unknown2;
	actor[0x4A1]= 1;
}

/* 0x001be6b0: an actor has 0 for its byte at 0x228 */
void __attribute__((stdcall)) actor_unknown_001be6b0(
	long actor_index,
	long unknown1)
{
	byte *actor= ACTOR_GET(actor_index);

	(void)unknown1;
	actor[0x228]= 0;
}

/* 0x001c1640: an actor has 0 for its byte at 0x354 */
void __attribute__((stdcall)) actor_unknown_001c1640(
	long actor_index,
	long unknown1)
{
	byte *actor= ACTOR_GET(actor_index);

	(void)unknown1;
	actor[0x354]= 0;
}

/* 0x001ad550: 3 for an actor that has something at 0x338 (not NONE), 0 if
not */
long __attribute__((stdcall)) actor_unknown_001ad550(
	long actor_index)
{
	if (*(const long *)(ACTOR_GET(actor_index)+0x338)!=NONE)
	{
		return 3;
	}

	return 0;
}

/* 0x001b2da0: the same for what it has at 0x85C */
long __attribute__((stdcall)) actor_unknown_001b2da0(
	long actor_index)
{
	if (*(const long *)(ACTOR_GET(actor_index)+0x85C)!=NONE)
	{
		return 3;
	}

	return 0;
}

/* 0x001e4260: an actor takes the bit 1 of its number at 0x810 */
void actor_unknown_001e4260(
	long actor_index)
{
	*(unsigned long *)(ACTOR_GET(actor_index)+0x810)|= 2;
}

/* 0x001f3610: the short at 0x61E of an actor is no less than a number:
it takes the number if it had less. Always true */
boolean actor_unknown_001f3610(
	long actor_index,
	short minimum)
{
	byte *actor= ACTOR_GET(actor_index);

	if (*(const short *)(actor+0x61E)<minimum)
	{
		*(short *)(actor+0x61E)= minimum;
	}

	return TRUE;
}

/* 0x00502418, of props.c: who each actor knows of */
extern struct data_array *prop_ref_data;

/* 0x0046fbe4, 0x0046fbe8: two numbers that the game keeps with its data,
0xFFFF and 0xFFFE, and whose low halves (NONE and -2) some functions of the
actors answer with. What the answers mean is not known */
long unknown_0046fbe4= 0xFFFF;
long unknown_0046fbe8= 0xFFFE;

#define ACTOR_PROP_REF_GET(prop_ref_index) \
	((const byte *)prop_ref_data->data+((prop_ref_index)&0xFFFF)*0x3C)

/* 0x001aeb60: 3 for an actor whose thing of prop_ref_data at 0x338 has 1 for
its short at 0x18; 0 if it has none, or another number */
long __attribute__((stdcall)) actor_unknown_001aeb60(
	long actor_index)
{
	long prop_ref_index= *(const long *)(ACTOR_GET(actor_index)+0x338);

	if (prop_ref_index!=NONE && *(const short *)(ACTOR_PROP_REF_GET(prop_ref_index)+0x18)==1)
	{
		return 3;
	}

	return 0;
}

/* 0x001bca60: 3 if that thing has 2 or more for its byte at 0x26 */
long __attribute__((stdcall)) actor_unknown_001bca60(
	long actor_index)
{
	long prop_ref_index= *(const long *)(ACTOR_GET(actor_index)+0x338);

	if (prop_ref_index!=NONE && *(const char *)(ACTOR_PROP_REF_GET(prop_ref_index)+0x26)>=2)
	{
		return 3;
	}

	return 0;
}

/* 0x001c10a0: 3 for an actor that has its byte at 0x22A, not the one at
0x223, and something at 0x338 */
long __attribute__((stdcall)) actor_unknown_001c10a0(
	long actor_index)
{
	const byte *actor= ACTOR_GET(actor_index);

	if (actor[0x22A] && !actor[0x223] && *(const long *)(actor+0x338)!=NONE)
	{
		return 3;
	}

	return 0;
}

/* 0x001b3540: an actor has 1 for its bytes at 0x4AE and 0x484, and, if it
has something at 0x26C, 0.6 for its real number at 0x4B4 */
void __attribute__((stdcall)) actor_unknown_001b3540(
	long actor_index,
	long unknown1)
{
	byte *actor= ACTOR_GET(actor_index);

	(void)unknown1;
	actor[0x4AE]= 1;
	actor[0x484]= 1;
	if (*(const long *)(actor+0x26C)!=NONE)
	{
		*(float *)(actor+0x4B4)= 0.6f;
	}
}

/* 0x001bce80: the same, and 1 for its byte at 0x485 too */
void __attribute__((stdcall)) actor_unknown_001bce80(
	long actor_index,
	long unknown1)
{
	byte *actor= ACTOR_GET(actor_index);

	(void)unknown1;
	actor[0x4AE]= 1;
	actor[0x484]= 1;
	actor[0x485]= 1;
	if (*(const long *)(actor+0x26C)!=NONE)
	{
		*(float *)(actor+0x4B4)= 0.6f;
	}
}

/* 0x001b8370: an actor has 4 and 2 for its shorts at 0x41C and 0x420, and 1
for its bytes at 0x449 and 0x44A */
void __attribute__((stdcall)) actor_unknown_001b8370(
	long actor_index,
	long unknown1,
	long unknown2)
{
	byte *actor= ACTOR_GET(actor_index);

	(void)unknown1;
	(void)unknown2;
	*(short *)(actor+0x41C)= 4;
	*(short *)(actor+0x420)= 2;
	actor[0x449]= 1;
	actor[0x44A]= 1;
}

/* 0x001bcc50: an actor has 3 and 2 for those shorts, and 1 for its bytes at
0x4AE and 0x488 */
void __attribute__((stdcall)) actor_unknown_001bcc50(
	long actor_index,
	long unknown1)
{
	byte *actor= ACTOR_GET(actor_index);

	(void)unknown1;
	*(short *)(actor+0x41C)= 3;
	*(short *)(actor+0x420)= 2;
	actor[0x4AE]= 1;
	actor[0x488]= 1;
}

/* 0x001bd1e0: an actor has 4 and 4 for those shorts, and the vector forward
at 0x424 */
void __attribute__((stdcall)) actor_unknown_001bd1e0(
	long actor_index,
	long unknown1)
{
	byte *actor= ACTOR_GET(actor_index);

	(void)unknown1;
	*(short *)(actor+0x41C)= 4;
	*(short *)(actor+0x420)= 4;
	*(real_vector3d *)(actor+0x424)= *global_forward3d;
}

/* 0x001b56b0: an actor that has nothing at 0x344 takes what something names
at 0x10. The answer is unknown_0046fbe4 if neither has anything, and
unknown_0046fbe8 if not */
short __attribute__((stdcall)) actor_unknown_001b56b0(
	long actor_index,
	const byte *unknown1,
	long unknown2)
{
	byte *actor= ACTOR_GET(actor_index);

	(void)unknown2;
	if (*(const long *)(actor+0x344)==NONE)
	{
		if (*(const long *)(unknown1+0x10)==NONE)
		{
			return (short)unknown_0046fbe4;
		}
		*(long *)(actor+0x344)= *(const long *)(unknown1+0x10);
	}

	return (short)unknown_0046fbe8;
}

/* 0x001bc580: unknown_0046fbe4 if something names nothing at 0xC, or the
actor has NONE for its short at 0x31C; unknown_0046fbe8 if not */
short __attribute__((stdcall)) actor_unknown_001bc580(
	long actor_index,
	const byte *unknown1,
	long unknown2)
{
	(void)unknown2;
	if (*(const long *)(unknown1+0xC)==NONE || *(const short *)(ACTOR_GET(actor_index)+0x31C)==NONE)
	{
		return (short)unknown_0046fbe4;
	}

	return (short)unknown_0046fbe8;
}

/* 0x001b1f30: unknown_0046fbe4 if something has its byte at 0x12 and the
actor has 2 for its short at 0x504; unknown_0046fbe8 if not */
short __attribute__((stdcall)) actor_unknown_001b1f30(
	long actor_index,
	const byte *unknown1,
	long unknown2)
{
	(void)unknown2;
	if (unknown1[0x12] && *(const short *)(ACTOR_GET(actor_index)+0x504)==2)
	{
		return (short)unknown_0046fbe4;
	}

	return (short)unknown_0046fbe8;
}

/* 0x001e4220: an actor takes a value for its byte at 0x6D0, and has the bit
27 of its number at 0x810 if the value is not zero */
void actor_set_unknown6D0(
	long actor_index,
	boolean value)
{
	byte *actor= ACTOR_GET(actor_index);

	actor[0x6D0]= value;
	if (value)
	{
		*(unsigned long *)(actor+0x810)|= 0x8000000;
	}
	else
	{
		*(unsigned long *)(actor+0x810)&= ~0x8000000;
	}
}

/* 0x00502420, of ai.c: the clumps */
extern struct data_array *clump_data;

/* the object of a whole number, as bytes */
#define ACTOR_OBJECT_GET(object_index) \
	((byte *)((const struct object_header_datum *)object_header_data->data)[(object_index)&0xFFFF].datum)

/* the short at 0x36 of what an object has at the place that its short at
0x346 says */
#define ACTOR_OBJECT_UNKNOWN346_UNKNOWN36(object) \
	(*(const short *)((object)+*(const short *)((object)+0x346)+0x36))

/* 0x001aa6e0: an actor has 4 and 2 for its shorts at 0x41C and 0x420 and 0
for the one at 0x3E0; and 1 for its byte at 0x44D if it has its byte at
0x50C and 1 for its short at 0x504 */
void __attribute__((stdcall)) actor_unknown_001aa6e0(
	long actor_index,
	long unknown1)
{
	byte *actor= ACTOR_GET(actor_index);

	(void)unknown1;
	*(short *)(actor+0x41C)= 4;
	*(short *)(actor+0x420)= 2;
	*(short *)(actor+0x3E0)= 0;
	if (actor[0x50C] && *(const short *)(actor+0x504)==1)
	{
		actor[0x44D]= 1;
	}
}

/* 0x001ac380: an actor has 4 and 2 for those shorts, and 1 for its byte at
0x488 if its object (0x18) has 7 for that short at 0x36 */
void __attribute__((stdcall)) actor_unknown_001ac380(
	long actor_index,
	long unknown1)
{
	byte *actor= ACTOR_GET(actor_index);
	const byte *object= ACTOR_OBJECT_GET(*(const long *)(actor+0x18));

	(void)unknown1;
	*(short *)(actor+0x41C)= 4;
	*(short *)(actor+0x420)= 2;
	if (ACTOR_OBJECT_UNKNOWN346_UNKNOWN36(object)==7)
	{
		actor[0x488]= 1;
	}
}

/* 0x001b09b0: an actor has nothing in several places: 0 for its byte at
0x50C and its shorts at 0x5B4, 0x5B6, 0x4AC and 0x504, and NONE for its number
at 0x5AC and its short at 0x5B0. Always true */
boolean __attribute__((stdcall)) actor_unknown_001b09b0(
	long actor_index,
	long unknown1)
{
	byte *actor= ACTOR_GET(actor_index);

	(void)unknown1;
	actor[0x50C]= 0;
	*(long *)(actor+0x5AC)= NONE;
	*(short *)(actor+0x5B0)= NONE;
	*(short *)(actor+0x5B4)= 0;
	*(short *)(actor+0x5B6)= 0;
	*(short *)(actor+0x4AC)= 0;
	*(short *)(actor+0x504)= 0;

	return TRUE;
}

/* 0x001b4330: 3 for an actor that has its byte at 0x227 and whose clump
(0x7C) has 1 for its short at 0x26; 0 if not */
long __attribute__((stdcall)) actor_unknown_001b4330(
	long actor_index)
{
	const byte *actor= ACTOR_GET(actor_index);
	long clump_index= *(const long *)(actor+0x7C);

	if (clump_index!=NONE && actor[0x227])
	{
		if (*(const short *)((const byte *)clump_data->data+(clump_index&0xFFFF)*0x50+0x26)==1)
		{
			return 3;
		}
	}

	return 0;
}

/* 0x001b8540: if the thing of prop_ref_data of an actor (0x338) has less
than 2 for its byte at 0x27, something has zeros for its bytes at 0xC and
0xD and its short at 0xE, and the answer is true */
boolean __attribute__((stdcall)) actor_unknown_001b8540(
	long actor_index,
	byte *unknown1)
{
	long prop_ref_index= *(const long *)(ACTOR_GET(actor_index)+0x338);

	if (prop_ref_index!=NONE && *(const char *)(ACTOR_PROP_REF_GET(prop_ref_index)+0x27)<2)
	{
		unknown1[0xC]= 0;
		unknown1[0xD]= 0;
		*(short *)(unknown1+0xE)= 0;

		return TRUE;
	}

	return FALSE;
}

/* 0x001be040: 3 for an actor that has an object at 0x5AC, 6 for its short at
0x5B0, and whose object has more than zero for its real number at 0xEC */
long __attribute__((stdcall)) actor_unknown_001be040(
	long actor_index)
{
	const byte *actor= ACTOR_GET(actor_index);
	long object_index= *(const long *)(actor+0x5AC);

	if (object_index!=NONE && *(const short *)(actor+0x5B0)==6)
	{
		if (*(const float *)(ACTOR_OBJECT_GET(object_index)+0xEC)>0.0f)
		{
			return 3;
		}
	}

	return 0;
}

/* 0x001a9b50: an actor that did not have 1 for its byte at 0x225 has it;
and then, unless it has its byte at 7, its object takes the bit 5 of its
number at 0x134 and the actor 0 for its byte at 0x226. The answer is always
unknown_0046fbe4 */
short __attribute__((stdcall)) actor_unknown_001a9b50(
	long actor_index,
	long unknown1)
{
	byte *actor= ACTOR_GET(actor_index);

	(void)unknown1;
	if (actor[0x225]!=1)
	{
		actor[0x225]= 1;
		if (!actor[7])
		{
			*(unsigned long *)(ACTOR_OBJECT_GET(*(const long *)(actor+0x18))+0x134)|= 0x20;
			actor[0x226]= 0;
		}
	}

	return (short)unknown_0046fbe4;
}

/* 0x001acd30: true for an actor that has 12 or more for its short at
0x328. For the others: if the object of the actor has 5 for that short at
0x36, whether something has less than 2 for its real number at 0x28; if not,
whether it has 1 or more for its byte at 0x27 */
boolean actor_unknown_001acd30(
	long actor_index,
	const byte *unknown)
{
	const byte *actor= ACTOR_GET(actor_index);

	const byte *object;

	if (*(const short *)(actor+0x328)>=12)
	{
		return TRUE;
	}

	object= ACTOR_OBJECT_GET(*(const long *)(actor+0x18));
	if (ACTOR_OBJECT_UNKNOWN346_UNKNOWN36(object)==5)
	{
		return 2.0f>*(const float *)(unknown+0x28);
	}

	return *(const char *)(unknown+0x27)>=1;
}

/* 0x001b1080: something has things of 12 bytes and how many they are in
its short at 0x7C. If the one with a number names an actor at 4 and has 3 for
its short at 8, and the actor has from 0 to 3 for its short at 0x190, this
gives the one of the four things of 0x40 bytes that the actor has at 0x90
with that number; NULL if not */
byte *unknown_get_actor_unknown90(
	short index,
	const byte *unknown)
{
	if (index>=0 && index<*(const short *)(unknown+0x7C))
	{
		const byte *element= unknown+index*12;
		long actor_index= *(const long *)(element+4);

		if (actor_index!=NONE && *(const short *)(element+8)==3)
		{
			byte *actor= ACTOR_GET(actor_index);
			short current= *(const short *)(actor+0x190);

			if (current>=0 && current<4)
			{
				return actor+0x90+current*0x40;
			}
		}
	}

	return NULL;
}

/* 0x001b0710: two distances for an object and an actor: 16 if the object
has NONE for its short at 0x1FC and the actor nothing at 0x26C, 100 if not;
and that times 2.25. Each goes where the caller says, if it says where */
void actor_unknown_001b0710(
	long object_index,
	long actor_index,
	float *first,
	float *second)
{
	float value= 100.0f;

	if (*(const short *)(ACTOR_OBJECT_GET(object_index)+0x1FC)==NONE)
	{
		if (*(const long *)(ACTOR_GET(actor_index)+0x26C)==NONE)
		{
			value= 16.0f;
		}
	}
	if (first)
	{
		*first= value;
	}
	if (second)
	{
		*second= value*2.25f;
	}
}

/* 0x00502414, of props.c: "tracking" */
extern struct data_array *tracking_data;

/* what several functions do to an actor, and the game has written in each:
it has nothing in these places (the same as actor_unknown_001b09b0) */
static inline __attribute__((always_inline)) void actor_clear_unknown5AC_inline(
	byte *actor)
{
	actor[0x50C]= 0;
	*(long *)(actor+0x5AC)= NONE;
	*(short *)(actor+0x5B0)= NONE;
	*(short *)(actor+0x5B4)= 0;
	*(short *)(actor+0x5B6)= 0;
	*(short *)(actor+0x4AC)= 0;
	*(short *)(actor+0x504)= 0;
}

/* 0x001f86a0: an actor has nothing in those places */
void actor_clear_unknown5AC(
	long actor_index)
{
	actor_clear_unknown5AC_inline(ACTOR_GET(actor_index));
}

/* 0x001b1eb0: something takes 3 for its short at 0x10, unless it has its
byte at 0xD, and the number at 0x418 of an actor at 0x14; the actor has 0 for
its byte at 0x3F2 and nothing in those places. Always true */
boolean __attribute__((stdcall)) actor_unknown_001b1eb0(
	long actor_index,
	byte *unknown1)
{
	byte *actor= ACTOR_GET(actor_index);

	if (!unknown1[0xD])
	{
		*(short *)(unknown1+0x10)= 3;
	}
	*(long *)(unknown1+0x14)= *(const long *)(actor+0x418);
	actor[0x3F2]= 0;
	actor_clear_unknown5AC_inline(actor);

	return TRUE;
}

/* 0x001b3ee0: something has 0 for its byte at 0xC and NONE at 0x10; an
actor has nothing in those places and takes the bit 0 of its number at 0x314.
Always true */
boolean __attribute__((stdcall)) actor_unknown_001b3ee0(
	long actor_index,
	byte *unknown1)
{
	byte *actor= ACTOR_GET(actor_index);

	unknown1[0xC]= 0;
	*(long *)(unknown1+0x10)= NONE;
	actor_clear_unknown5AC_inline(actor);
	*(unsigned long *)(actor+0x314)|= 1;

	return TRUE;
}

/* 0x001c10f0: an actor has nothing in those places; something has 0 for its
bytes at 0xC and 0x20 and its short at 0x38, and NONE at 0x10. Always true */
boolean __attribute__((stdcall)) actor_unknown_001c10f0(
	long actor_index,
	byte *unknown1)
{
	actor_clear_unknown5AC_inline(ACTOR_GET(actor_index));
	unknown1[0xC]= 0;
	*(long *)(unknown1+0x10)= NONE;
	*(short *)(unknown1+0x38)= 0;
	unknown1[0x20]= 0;

	return TRUE;
}

/* 0x001b4f60: 0x38 for an actor that has the bit 0 of its byte at 0x6FC;
unknown_0046fbe4 if not */
short __attribute__((stdcall)) actor_unknown_001b4f60(
	long actor_index,
	long unknown1)
{
	(void)unknown1;
	if (ACTOR_GET(actor_index)[0x6FC]&1)
	{
		return 0x38;
	}

	return (short)unknown_0046fbe4;
}

/* 0x001b4f90: if an actor has more than 0 for its short at 0x3B0 and
something at 0x3B4, something else takes that at 0x10, NONE for its short at
0x14 and 1 for its byte at 0xC, and the answer is 0x38; unknown_0046fbe4 if
not */
short __attribute__((stdcall)) actor_unknown_001b4f90(
	long actor_index,
	byte *unknown1)
{
	const byte *actor= ACTOR_GET(actor_index);

	if (*(const short *)(actor+0x3B0)>0 && *(const long *)(actor+0x3B4)!=NONE)
	{
		*(long *)(unknown1+0x10)= *(const long *)(actor+0x3B4);
		*(short *)(unknown1+0x14)= NONE;
		unknown1[0xC]= 1;

		return 0x38;
	}

	return (short)unknown_0046fbe4;
}

/* 0x001b5a20: 3 for an actor whose thing of prop_ref_data (0x338) names one
of tracking_data at 0x14 and has 3 or more for its short at 0x24, if that one
has 0 for its byte at 0xD9; 0 if not. The game looks at whether the place of
the thing of tracking_data, and the place 0x70 after it, are nothing */
long __attribute__((stdcall)) actor_unknown_001b5a20(
	long actor_index)
{
	long prop_ref_index= *(const long *)(ACTOR_GET(actor_index)+0x338);

	if (prop_ref_index!=NONE)
	{
		const byte *prop_ref= ACTOR_PROP_REF_GET(prop_ref_index);
		long tracking_index= *(const long *)(prop_ref+0x14);

		if (tracking_index!=NONE)
		{
			uintptr_t tracking= (uintptr_t)tracking_data->data+(tracking_index&0xFFFF)*0x124;

			if (tracking && tracking+0x70 && *(const short *)(prop_ref+0x24)>=3)
			{
				if (!((const byte *)tracking)[0x70+0x69])
				{
					return 3;
				}
			}
		}
	}

	return 0;
}

/* an actor has two numbers for its shorts at 0x41C and 0x420, and 1 for its
byte at 0x488, if its thing of prop_ref_data has 1 or 2 for its short at
0x24 */
static inline __attribute__((always_inline)) void actor_unknown_001bf360_inline(
	long actor_index,
	short first)
{
	byte *actor= ACTOR_GET(actor_index);
	long prop_ref_index= *(const long *)(actor+0x338);

	if (prop_ref_index!=NONE)
	{
		short unknown24= *(const short *)(ACTOR_PROP_REF_GET(prop_ref_index)+0x24);

		if (unknown24>=1 && unknown24<=2)
		{
			*(short *)(actor+0x41C)= first;
			*(short *)(actor+0x420)= 2;
			actor[0x488]= 1;
		}
	}
}

/* 0x001bf360: that, with 3 and 2 */
void __attribute__((stdcall)) actor_unknown_001bf360(
	long actor_index,
	long unknown1)
{
	(void)unknown1;
	actor_unknown_001bf360_inline(actor_index, 3);
}

/* 0x001c22c0: that, with 4 and 2 */
void __attribute__((stdcall)) actor_unknown_001c22c0(
	long actor_index,
	long unknown1)
{
	(void)unknown1;
	actor_unknown_001bf360_inline(actor_index, 4);
}

/* 0x001c0230: if something has its byte at 0x1B, an actor has 4 and 4 for
those shorts and the vector that the thing has at 0x1C at 0x424 */
void __attribute__((stdcall)) actor_unknown_001c0230(
	long actor_index,
	const byte *unknown1)
{
	if (unknown1[0x1B])
	{
		byte *actor= ACTOR_GET(actor_index);

		*(short *)(actor+0x41C)= 4;
		*(short *)(actor+0x420)= 4;
		*(real_vector3d *)(actor+0x424)= *(const real_vector3d *)(unknown1+0x1C);
	}
}

/* 0x001c14b0: if an actor has 2 for its short at 0x504 and something has its
byte at 0x20, the actor has 4 and 4 for those shorts, the vector that the
thing has at 0x14 at 0x424, and 1 for its byte at 0x44D */
void __attribute__((stdcall)) actor_unknown_001c14b0(
	long actor_index,
	const byte *unknown1)
{
	byte *actor= ACTOR_GET(actor_index);

	if (*(const short *)(actor+0x504)==2 && unknown1[0x20])
	{
		*(short *)(actor+0x41C)= 4;
		*(short *)(actor+0x420)= 4;
		*(real_vector3d *)(actor+0x424)= *(const real_vector3d *)(unknown1+0x14);
		actor[0x44D]= 1;
	}
}

/* 0x001c8440: a number is made as many times bigger as the real number at
0x7C0 of an actor says, if there is an actor, something has the bit 3 of its
byte at 4 and that real number is more than zero; whether it was */
boolean actor_unknown_001c8440(
	long actor_index,
	float *value,
	const byte *unknown)
{
	if (actor_index!=NONE)
	{
		const byte *actor= ACTOR_GET(actor_index);

		if ((unknown[4]&8) && *(const float *)(actor+0x7C0)>0.0f)
		{
			*value= *value**(const float *)(actor+0x7C0);

			return TRUE;
		}
	}

	return FALSE;
}

/* 0x001e1fd0: one of the numbers that the object of an actor has at 0x218,
the one that its byte at 0x213 says; NONE if the actor has no object or the
byte is NONE */
long actor_get_object_unknown218(
	long actor_index)
{
	long object_index= *(const long *)(ACTOR_GET(actor_index)+0x18);

	if (object_index!=NONE)
	{
		const byte *object= ACTOR_OBJECT_GET(object_index);
		short index= *(const char *)(object+0x213);

		if (index!=NONE)
		{
			return *(const long *)(object+0x218+index*4);
		}
	}

	return NONE;
}

/* 0x004f55d0 and 0x005047f4, of ai.c; 0x0051e9d8, of squads.c; 0x005044c8,
of swarms.c */
extern byte *ai_globals;
extern byte *unknown_005047f4;
extern struct data_array *squad_data;
extern struct data_array *swarm_data;

/* 0x00470c40, 0x00470c44: two more numbers like unknown_0046fbe4 and
unknown_0046fbe8, with the same values, that the functions from 0x255520 on
answer with: those are of another file in the game */
long unknown_00470c40= 0xFFFF;
long unknown_00470c44= 0xFFFE;

/* 0x001e4290: an actor takes or loses the bit 11 of its number at 0x810.
It does not take it (and nothing changes) if it has 0 for its byte at 0x229
and its object of 0x26C has its byte at 0x34C */
void actor_set_unknown810_bit11(
	long actor_index,
	boolean value)
{
	byte *actor= ACTOR_GET(actor_index);

	if (value)
	{
		if (!actor[0x229] && *(const long *)(actor+0x26C)!=NONE)
		{
			if (ACTOR_OBJECT_GET(*(const long *)(actor+0x26C))[0x34C])
			{
				return;
			}
		}
		*(unsigned long *)(actor+0x810)|= 0x800;
	}
	else
	{
		*(unsigned long *)(actor+0x810)&= ~0x800;
	}
}

/* 0x001e4650: an actor takes a value for its byte at 0x225, if it had
another; and then, unless it has its byte at 7, its object takes or loses
the bit 5 of its number at 0x134 and the actor has 0 for its byte at 0x226 */
void actor_set_unknown225(
	long actor_index,
	boolean value)
{
	byte *actor= ACTOR_GET(actor_index);

	if (value!=actor[0x225])
	{
		actor[0x225]= value;
		if (!actor[7])
		{
			unsigned long *flags= (unsigned long *)(ACTOR_OBJECT_GET(*(const long *)(actor+0x18))+0x134);

			if (value)
			{
				*flags|= 0x20;
			}
			else
			{
				*flags&= ~0x20;
			}
			actor[0x226]= 0;
		}
	}
}

/* 0x001f5560: an actor with nothing at 0x26C that has its byte at 0x622
gives a vector: its real numbers at 0x624 and 0x628 times the one at 0x62C,
and the one at 0x630. The byte is 0 after, and the answer always true */
boolean actor_unknown_001f5560(
	long actor_index,
	float *vector)
{
	byte *actor= ACTOR_GET(actor_index);

	if (*(const long *)(actor+0x26C)==NONE && actor[0x622])
	{
		float scale= *(const float *)(actor+0x62C);
		float j= *(const float *)(actor+0x628)*scale;
		float k= *(const float *)(actor+0x630);

		vector[0]= *(const float *)(actor+0x624)*scale;
		vector[1]= j;
		vector[2]= k;
	}
	actor[0x622]= 0;

	return TRUE;
}

/* 0x00203fb0: an actor leaves the list of them all, which starts at 0x14 of
ai_globals and goes on through the number at 0x20 of each: what named it
names the one after it, and the actor has NONE there and 0 for its bytes at
0xA and 0xB. Nothing if the byte at 1 of ai_globals is not set */
void actor_unknown_00203fb0(
	long actor_index)
{
	if (ai_globals[1])
	{
		byte *actor= ACTOR_GET(actor_index);
		long *link= (long *)(ai_globals+0x14);

		while (*link!=actor_index)
		{
			link= (long *)(ACTOR_GET(*link)+0x20);
		}
		*link= *(const long *)(actor+0x20);
		actor[0xA]= 0;
		*(long *)(actor+0x20)= NONE;
		actor[0xB]= 0;
	}
}

/* 0x00204d30: a walk through the actors starts: those of a squad (from the
one that the squad names at 0x68), or all of them for NONE. Nothing if the
byte at 1 of ai_globals is not set */
void actor_iterator_new(
	long squad_index,
	long *iterator)
{
	if (ai_globals[1])
	{
		iterator[0]= squad_index;
		iterator[1]= NONE;
		if (squad_index==NONE)
		{
			iterator[2]= *(const long *)(ai_globals+0x14);
		}
		else
		{
			iterator[2]= *(const long *)((const byte *)squad_data->data+(squad_index&0xFFFF)*0x98+0x68);
		}
	}
}

/* 0x00204d70: the next actor of a walk, or NULL when there are no more */
byte *actor_iterator_next(
	long *iterator)
{
	if (ai_globals[1])
	{
		long actor_index= iterator[2];

		if (actor_index!=NONE)
		{
			byte *actor= ACTOR_GET(actor_index);

			iterator[1]= actor_index;
			iterator[2]= *(const long *)(actor+0x20);

			return actor;
		}
	}

	return NULL;
}

/* 0x00255520: the object of an actor loses the bit 21 of its number at
0x134 */
void __attribute__((stdcall)) actor_unknown_00255520(
	long actor_index,
	long unknown1)
{
	(void)unknown1;
	*(unsigned long *)(ACTOR_OBJECT_GET(*(const long *)(ACTOR_GET(actor_index)+0x18))+0x134)&= ~0x200000;
}

/* 0x00255660: an actor takes the vector that something has at 0x10 at
0x458, 0x4000089 for its number at 0x450 and 1 for its byte at 0x456 */
void __attribute__((stdcall)) actor_unknown_00255660(
	long actor_index,
	const byte *unknown1)
{
	byte *actor= ACTOR_GET(actor_index);

	*(real_vector3d *)(actor+0x458)= *(const real_vector3d *)(unknown1+0x10);
	*(unsigned long *)(actor+0x450)= 0x4000089;
	actor[0x456]= 1;
}

/* 0x00257010: an actor takes its own vector at 0x290 at 0x458, 0x6000085
for its number at 0x450, 0 for its short at 0x454 and 1 for its byte at
0x456 */
void __attribute__((stdcall)) actor_unknown_00257010(
	long actor_index,
	long unknown1)
{
	byte *actor= ACTOR_GET(actor_index);

	(void)unknown1;
	*(real_vector3d *)(actor+0x458)= *(const real_vector3d *)(actor+0x290);
	actor[0x456]= 1;
	*(unsigned long *)(actor+0x450)= 0x6000085;
	*(short *)(actor+0x454)= 0;
}

/* 0x002562c0: the swarm of an actor (0x1C), if it has one, has 0 for its
short at 0x1C */
void __attribute__((stdcall)) actor_unknown_002562c0(
	long actor_index,
	long unknown1)
{
	long swarm_index= *(const long *)(ACTOR_GET(actor_index)+0x1C);

	(void)unknown1;
	if (swarm_index!=NONE)
	{
		*(short *)((byte *)swarm_data->data+(swarm_index&0xFFFF)*0x34+0x1C)= 0;
	}
}

/* 0x002570b0: unknown_00470c44 if unknown_005047f4 starts with a short that
is 1 and the actor has something at 0x338; unknown_00470c40 if not */
short __attribute__((stdcall)) actor_unknown_002570b0(
	long actor_index,
	long unknown1,
	long unknown2)
{
	(void)unknown1;
	(void)unknown2;
	if (*(const short *)unknown_005047f4==1 && *(const long *)(ACTOR_GET(actor_index)+0x338)!=NONE)
	{
		return (short)unknown_00470c44;
	}

	return (short)unknown_00470c40;
}

/* 0x00257a40: the clump of an actor (0x7C), if it has one, has the time of
the game for its number at 0x4C */
void __attribute__((stdcall)) actor_unknown_00257a40(
	long actor_index,
	long unknown1,
	long unknown2)
{
	long clump_index= *(const long *)(ACTOR_GET(actor_index)+0x7C);

	(void)unknown1;
	(void)unknown2;
	if (clump_index!=NONE)
	{
		*(unsigned long *)((byte *)clump_data->data+(clump_index&0xFFFF)*0x50+0x4C)= game_time_globals->time;
	}
}

/* 0x0025aba0: an actor has 0 for its byte at 0x3B8, and each of the three
things of 12 bytes that it has at 0x1C4, if it is in use (its first short is
not NONE), has one less in its short at 8; when that gets to zero the thing
is free: NONE for its shorts at 0, 4 and 6, and 0 for the one at 10 */
void actor_unknown_0025aba0(
	long actor_index)
{
	byte *actor= ACTOR_GET(actor_index);
	long index;

	actor[0x3B8]= 0;
	for (index= 0; index<3; index++)
	{
		short *element= (short *)(actor+0x1C4+index*12);

		if (element[0]!=NONE)
		{
			element[4]--;
			if (element[4]<=0)
			{
				element[0]= NONE;
				element[2]= NONE;
				element[3]= NONE;
				element[5]= 0;
			}
		}
	}
}

/* 0x00502408, of command_scripts.c; 0x0050241c, of props.c; 0x00502424 and
0x0051eca4, of ai.c */
extern struct data_array *command_script_data;
extern struct data_array *prop_data;
extern struct data_array *joint_state_data;
extern struct data_array *dynamic_firing_point_data;

/* 0x00470fa0: a NONE that the game keeps with its data */
long unknown_00470fa0= NONE;

/* the four things of 12 bytes that an actor has at 0x194: a short that is
NONE for one not in use, another short, and something of joint_state_data */
#define ACTOR_UNKNOWN194(actor, index) ((actor)+0x194+(index)*12)
#define ACTOR_JOINT_STATE_GET(joint_index) \
	((byte *)joint_state_data->data+((joint_index)&0xFFFF)*0xBC)

/* 0x0025bba0: an actor that has 0 or less for its short at 0x684 takes a
value for its number at 0x68C, a second and a half, in ticks, for its short
at 0x686, 1 for the one at 0x688 and 0 for the one at 0x684 */
void actor_unknown_0025bba0(
	long actor_index,
	long value)
{
	byte *actor= ACTOR_GET(actor_index);

	if (*(const short *)(actor+0x684)<=0)
	{
		*(short *)(actor+0x686)= (short)lrintf((float)game_time_globals->ticks_per_second*1.5f);
		*(short *)(actor+0x688)= 1;
		*(long *)(actor+0x68C)= value;
		*(short *)(actor+0x684)= 0;
	}
}

/* 0x00262890: whether a number, other than unknown_00470fa0, is one of the
four that an actor has every 6 bytes from 0x402 */
boolean actor_unknown_00262890(
	long actor_index,
	long value)
{
	const byte *actor= ACTOR_GET(actor_index);

	if (value!=unknown_00470fa0)
	{
		short index;

		for (index= 0; index<4; index++)
		{
			if (value==*(const long *)(actor+(index+0xAB)*6))
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

/* 0x0026dbd0: an actor takes one of the things of
dynamic_firing_point_data for its number at 0x3F4, if it had another, and
the thing has one more in its short at 0xA */
void actor_set_dynamic_firing_point(
	long actor_index,
	long dynamic_firing_point_index)
{
	byte *actor= ACTOR_GET(actor_index);
	byte *dynamic_firing_point=
		(byte *)dynamic_firing_point_data->data+(dynamic_firing_point_index&0xFFFF)*0x484;

	if (*(const long *)(actor+0x3F4)!=dynamic_firing_point_index)
	{
		*(long *)(actor+0x3F4)= dynamic_firing_point_index;
		(*(short *)(dynamic_firing_point+0xA))++;
	}
}

/* 0x0026e9c0: which of the four things at 0x194 of an actor is in use and
names a thing of joint_state_data; NONE if none */
short actor_find_unknown194(
	long actor_index,
	long joint_index)
{
	const byte *actor= ACTOR_GET(actor_index);
	short index;

	for (index= 0; index<4; index++)
	{
		const byte *element= ACTOR_UNKNOWN194(actor, index);

		if (*(const short *)element!=NONE && *(const long *)(element+4)==joint_index)
		{
			return index;
		}
	}

	return NONE;
}

/* 0x0026ed40: one of the four things at 0x194 of an actor is not in use
any more; in its thing of joint_state_data, the place of 12 bytes that its
second short says has NONE at 4 and 1 for its short at 8. Always true */
boolean actor_unknown_0026ed40(
	long actor_index,
	short index)
{
	byte *element= ACTOR_UNKNOWN194(ACTOR_GET(actor_index), index);
	byte *joint= ACTOR_JOINT_STATE_GET(*(const long *)(element+4));

	*(long *)(joint+*(const short *)(element+2)*12+4)= NONE;
	*(short *)(joint+*(const short *)(element+2)*12+8)= 1;
	*(short *)element= NONE;

	return TRUE;
}

/* 0x0026ecc0: the same thing stops being in use in another way: that place
has 3 for its short at 8, something takes the second short at 0x18, the
thing of joint_state_data at 0x10 and 0 for its byte at 0xC, and the thing of
joint_state_data has one more in its short at 0x7C. Always true */
boolean actor_unknown_0026ecc0(
	long actor_index,
	byte *unknown,
	short index)
{
	byte *element= ACTOR_UNKNOWN194(ACTOR_GET(actor_index), index);
	short place= *(const short *)(element+2);
	byte *joint= ACTOR_JOINT_STATE_GET(*(const long *)(element+4));

	*(short *)(joint+place*12+8)= 3;
	*(short *)(unknown+0x18)= place;
	unknown[0xC]= 0;
	*(long *)(unknown+0x10)= *(const long *)(element+4);
	(*(short *)(joint+0x7C))++;
	*(short *)element= NONE;

	return TRUE;
}

/* 0x00275ad0: the actor of an object (0x12C), if the object has one and it
has its byte at 0xA, takes a value for its byte at 0xB */
void object_set_actor_unknownB(
	long object_index,
	boolean value)
{
	if (object_index!=NONE)
	{
		long actor_index= *(const long *)(ACTOR_OBJECT_GET(object_index)+0x12C);

		if (actor_index!=NONE)
		{
			byte *actor= ACTOR_GET(actor_index);

			if (actor[0xA])
			{
				actor[0xB]= value;
			}
		}
	}
}

/* 0x00298b60: whether an actor has its byte at 0x6F8, 2 or more for its
short at 0x41C, and for the one at 0x420 either 2, or 1 with the same number
at 0x424 as at 0x338 */
boolean actor_unknown_00298b60(
	long actor_index)
{
	const byte *actor= ACTOR_GET(actor_index);

	if (actor[0x6F8] && *(const short *)(actor+0x41C)>=2)
	{
		short unknown420= *(const short *)(actor+0x420);

		if (unknown420==2)
		{
			return TRUE;
		}
		if (unknown420==1 && *(const long *)(actor+0x424)==*(const long *)(actor+0x338))
		{
			return TRUE;
		}
	}

	return FALSE;
}

/* a number that counts down, one less and never under zero; the game does
it with real numbers */
static inline __attribute__((always_inline)) long actor_count_down_inline(
	long count)
{
	float less= (float)(count-1);

	if (!(less>0.0f))
	{
		less= 0.0f;
	}

	return (long)less;
}

/* 0x00297600: the numbers at 0x698 and 0x69C of an actor count down */
void actor_unknown_00297600(
	long actor_index)
{
	byte *actor= ACTOR_GET(actor_index);

	*(long *)(actor+0x698)= actor_count_down_inline(*(const long *)(actor+0x698));
	*(long *)(actor+0x69C)= actor_count_down_inline(*(const long *)(actor+0x69C));
}

/* 0x0025aad0: the real number at 0x60 of the command script of an actor
(0x858), if the actor has one and not its byte at 7, the script has 0 or 1
for its short at 0x30 and the number is more than zero; whether it gave it */
boolean actor_unknown_0025aad0(
	long actor_index,
	float *value)
{
	const byte *actor= ACTOR_GET(actor_index);
	long command_script_index= *(const long *)(actor+0x858);

	if (command_script_index!=NONE && !actor[7])
	{
		const byte *command_script=
			(const byte *)command_script_data->data+(command_script_index&0xFFFF)*0xD4;
		short unknown30= *(const short *)(command_script+0x30);

		if ((unknown30==0 || unknown30==1) && *(const float *)(command_script+0x60)>0.0f)
		{
			*value= *(const float *)(command_script+0x60);

			return TRUE;
		}
	}

	return FALSE;
}

/* 0x002687a0: a clump that has its byte at 0x1C looks at its actors (from
the one at 0x18, each naming the next at 0x80): if one has its byte at 9,
the clump has the time of the game at 0x20; if none, it has 0 for that byte
and the tick before at 0x20 */
void clump_unknown_002687a0(
	long clump_index)
{
	byte *clump= (byte *)clump_data->data+(clump_index&0xFFFF)*0x50;

	if (clump[0x1C])
	{
		long actor_index;

		for (actor_index= *(const long *)(clump+0x18); actor_index!=NONE; )
		{
			const byte *actor= ACTOR_GET(actor_index);

			actor_index= *(const long *)(actor+0x80);
			if (actor[9])
			{
				*(unsigned long *)(clump+0x20)= game_time_globals->time;
				return;
			}
		}
		clump[0x1C]= 0;
		*(unsigned long *)(clump+0x20)= game_time_globals->time-1;
	}
}

/* 0x0025da40: something names a prop at 8. If it has something at 0x14:
true if the prop names no actor at 0x1C; if it names one, false for an actor
of the type 15, and for the others whether it has 0 for its byte at 0x267 or
has the one at 0x268. If it has nothing at 0x14: whether the prop has its
byte at 0x25 and not the one at 0x23 */
boolean unknown_prop_unknown_0025da40(
	const byte *unknown)
{
	const byte *prop= (const byte *)prop_data->data+(*(const long *)(unknown+8)&0xFFFF)*0xC4;

	if (*(const long *)(unknown+0x14)!=NONE)
	{
		long actor_index= *(const long *)(prop+0x1C);

		if (actor_index!=NONE)
		{
			const byte *actor= ACTOR_GET(actor_index);

			if (*(const short *)(actor+4)==15)
			{
				return FALSE;
			}
			if (actor[0x267] && !actor[0x268])
			{
				return FALSE;
			}
		}

		return TRUE;
	}

	return prop[0x25] && !prop[0x23];
}

/* 0x0051e9dc, of squads.c: the groups of squads */
extern struct data_array *squad_group_data;

/* 0x00471004: another NONE that the game keeps with its data */
long unknown_00471004= NONE;

/* ---------- functions that take something that names a thing of joint_state_data

Like the ones above, these take an actor first, which they do not use; then
something that names a thing of joint_state_data at 0x10 (called a state
here, a guess), and some a number more */

/* 0x001b0d10: unknown_0046fbe4 if the thing of joint_state_data has NONE at
4; unknown_0046fbe8 if not */
short __attribute__((stdcall)) joint_unknown_001b0d10(
	long actor_index,
	const byte *state,
	long unknown2)
{
	(void)actor_index;
	(void)unknown2;
	if (*(const long *)(ACTOR_JOINT_STATE_GET(*(const long *)(state+0x10))+4)==NONE)
	{
		return (short)unknown_0046fbe4;
	}

	return (short)unknown_0046fbe8;
}

/* 0x001b68b0: the thing of joint_state_data has NONE at 0x84, if it had a
value there */
void __attribute__((stdcall)) joint_unknown_001b68b0(
	long actor_index,
	const byte *state,
	long value)
{
	byte *joint= ACTOR_JOINT_STATE_GET(*(const long *)(state+0x10));

	(void)actor_index;
	if (*(const long *)(joint+0x84)==value)
	{
		*(long *)(joint+0x84)= NONE;
	}
}

/* 0x001b8460: the same at 0x80 */
void __attribute__((stdcall)) joint_unknown_001b8460(
	long actor_index,
	const byte *state,
	long value)
{
	byte *joint= ACTOR_JOINT_STATE_GET(*(const long *)(state+0x10));

	(void)actor_index;
	if (*(const long *)(joint+0x80)==value)
	{
		*(long *)(joint+0x80)= NONE;
	}
}

/* 0x001b68f0: the thing of joint_state_data has NONE at 0x84, if it has 1
for its short at 0x94 */
void __attribute__((stdcall)) joint_unknown_001b68f0(
	long actor_index,
	const byte *state)
{
	byte *joint= ACTOR_JOINT_STATE_GET(*(const long *)(state+0x10));

	(void)actor_index;
	if (*(const short *)(joint+0x94)==1)
	{
		*(long *)(joint+0x84)= NONE;
	}
}

/* 0x00257cb0: the thing of joint_state_data has NONE at 0x80 and 0x84, if
it had a value at one of the two */
void __attribute__((stdcall)) joint_unknown_00257cb0(
	long actor_index,
	const byte *state,
	long value)
{
	byte *joint= ACTOR_JOINT_STATE_GET(*(const long *)(state+0x10));

	(void)actor_index;
	if (*(const long *)(joint+0x80)==value || *(const long *)(joint+0x84)==value)
	{
		*(long *)(joint+0x80)= NONE;
		*(long *)(joint+0x84)= NONE;
	}
}

/* 0x001bb530: the state has NONE at 0x1C and the thing of joint_state_data
at 0x80, if one of the two had a value */
void __attribute__((stdcall)) joint_unknown_001bb530(
	long actor_index,
	byte *state,
	long value)
{
	byte *joint= ACTOR_JOINT_STATE_GET(*(const long *)(state+0x10));

	(void)actor_index;
	if (*(const long *)(state+0x1C)==value || *(const long *)(joint+0x80)==value)
	{
		*(long *)(state+0x1C)= NONE;
		*(long *)(joint+0x80)= NONE;
	}
}

/* 0x001bdfd0: the state has NONE at 0x20 and the thing of joint_state_data
at 0x80, and the state loses the bits 0 and 4 of its short at 0x1C, if the
state had a value at 0x20 (and the bit 0) or the thing had it at 0x80 */
void __attribute__((stdcall)) joint_unknown_001bdfd0(
	long actor_index,
	byte *state,
	long value)
{
	byte *joint= ACTOR_JOINT_STATE_GET(*(const long *)(state+0x10));
	word flags= *(const word *)(state+0x1C);

	(void)actor_index;
	if (((flags&1) && *(const long *)(state+0x20)==value) || *(const long *)(joint+0x80)==value)
	{
		*(long *)(state+0x20)= NONE;
		*(word *)(state+0x1C)= flags&~0x11;
		*(long *)(joint+0x80)= NONE;
	}
}

/* 0x001b4f10: the state has NONE at 0x24, if it had a value there; and its
thing of joint_state_data, if it names one, NONE at 0x88 if it had the value
there */
void __attribute__((stdcall)) joint_unknown_001b4f10(
	long actor_index,
	byte *state,
	long value)
{
	long joint_index;

	(void)actor_index;
	if (*(const long *)(state+0x24)==value)
	{
		*(long *)(state+0x24)= NONE;
	}
	joint_index= *(const long *)(state+0x10);
	if (joint_index!=NONE)
	{
		byte *joint= ACTOR_JOINT_STATE_GET(joint_index);

		if (*(const long *)(joint+0x88)==value)
		{
			*(long *)(joint+0x88)= NONE;
		}
	}
}

/* 0x001adce0: an answer from the thing of joint_state_data. For a state
that has its byte at 0xC: unknown_0046fbe8 unless the thing has 1 for its
short at 2; then unknown_0046fbe4 if it has less than 2 for its short at
0x7C, and if not it takes 1 for its byte at 0x80 and the answer is 0x46. For
the others: 0x46 if the thing has its byte at 0x80, unknown_0046fbe4 if it
has the one at 0x81, unknown_0046fbe8 if neither */
short __attribute__((stdcall)) joint_unknown_001adce0(
	long actor_index,
	const byte *state,
	long unknown2)
{
	byte *joint= ACTOR_JOINT_STATE_GET(*(const long *)(state+0x10));

	(void)actor_index;
	(void)unknown2;
	if (state[0xC])
	{
		if (*(const short *)(joint+2)!=1)
		{
			return (short)unknown_0046fbe8;
		}
		if (*(const short *)(joint+0x7C)<2)
		{
			return (short)unknown_0046fbe4;
		}
		joint[0x80]= 1;

		return 0x46;
	}
	if (joint[0x80])
	{
		return 0x46;
	}
	if (joint[0x81])
	{
		return (short)unknown_0046fbe4;
	}

	return (short)unknown_0046fbe8;
}

/* ---------- props, squads and command scripts */

#define ACTOR_PROP_GET(prop_index) ((byte *)prop_data->data+((prop_index)&0xFFFF)*0xC4)

/* 0x001b6010: a prop keeps the last five numbers that it had at 0x54, in a
ring at 0x3E: its byte at 0x52 says where the next goes and the one at 0x53
how many there are. This puts the number of 0x54 in the ring, unless it is
unknown_00471004, and leaves NONE for the two shorts there. Always true */
boolean prop_unknown_001b6010(
	long prop_index)
{
	byte *prop= ACTOR_PROP_GET(prop_index);
	long value= *(const long *)(prop+0x54);

	if (value!=unknown_00471004)
	{
		long count;

		*(long *)(prop+*(const char *)(prop+0x52)*4+0x3E)= value;
		*(short *)(prop+0x54)= NONE;
		*(short *)(prop+0x56)= NONE;
		prop[0x52]= (byte)((*(const char *)(prop+0x52)+1)%5);
		count= *(const char *)(prop+0x53)+1;
		if (count>5)
		{
			count= 5;
		}
		prop[0x53]= (byte)count;
	}

	return TRUE;
}

/* 0x001b6070: whether the ring of a prop has a number, which comes as its
two halves */
boolean prop_unknown_001b6070(
	long prop_index,
	short low,
	short high)
{
	const byte *prop= ACTOR_PROP_GET(prop_index);
	long value= (word)low | high<<16;
	short count= *(const char *)(prop+0x53);
	short index;

	for (index= 0; index<count; index++)
	{
		if (value==*(const long *)(prop+index*4+0x3E))
		{
			return TRUE;
		}
	}

	return FALSE;
}

/* 0x002052d0: whether a squad is of a group of squads, or of one of the
groups that the group is of: the squad names its group at 0x6C and each group
the one it is of at 0x10 */
boolean squad_is_in_group(
	long squad_index,
	long squad_group_index)
{
	long index= *(const long *)((const byte *)squad_data->data+(squad_index&0xFFFF)*0x98+0x6C);

	while (index!=NONE)
	{
		if (index==squad_group_index)
		{
			return TRUE;
		}
		index= *(const long *)((const byte *)squad_group_data->data+(index&0xFFFF)*0x38+0x10);
	}

	return FALSE;
}

/* 0x00256790: whether a thing of prop_ref_data has 1 or 2 for its short at
0x24 and 7 for the one at 0x18, and its object (0x20) has the bit 2 of its
byte at 0x10A, is of the type 0 and does not have both of the bits 0 and 1
of its byte at 0x19 */
boolean prop_ref_unknown_00256790(
	long prop_ref_index)
{
	const byte *prop_ref= ACTOR_PROP_REF_GET(prop_ref_index);
	short unknown24= *(const short *)(prop_ref+0x24);

	if (unknown24>=1 && unknown24<=2 && *(const short *)(prop_ref+0x18)==7)
	{
		const byte *object= ACTOR_OBJECT_GET(*(const long *)(prop_ref+0x20));

		if (((object[0x10A]>>2)&1) && !object[0xAA])
		{
			if (!(object[0x19]&1) || !(object[0x19]&2))
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

/* 0x00259e70: a command script that has its byte at 0xAC, or something at
0xB0, has neither, and 0 for its bytes at 0xD1, 0x52 and 0x46 */
void command_script_unknown_00259e70(
	long command_script_index)
{
	byte *command_script= (byte *)command_script_data->data+(command_script_index&0xFFFF)*0xD4;

	if (command_script[0xAC] || *(const long *)(command_script+0xB0)!=NONE)
	{
		command_script[0xAC]= 0;
		*(long *)(command_script+0xB0)= NONE;
		command_script[0xD1]= 0;
		command_script[0x52]= 0;
		command_script[0x46]= 0;
	}
}

/* 0x00502410: a command script, or NONE. Which one it is is not known (the
one that is running?) */
long unknown_00502410;

#define ACTOR_CLUMP_GET(clump_index) ((byte *)clump_data->data+((clump_index)&0xFFFF)*0x50)

/* what a thing of prop_ref_data has of tracking_data: the place 0x70 after
the one it names at 0x14, or NULL if it names none. The game looks at
whether the place of the one it names is nothing */
static inline __attribute__((always_inline)) byte *prop_ref_get_tracking_unknown70_inline(
	const byte *prop_ref)
{
	long tracking_index= *(const long *)(prop_ref+0x14);

	if (tracking_index!=NONE)
	{
		uintptr_t tracking= (uintptr_t)tracking_data->data+(tracking_index&0xFFFF)*0x124;

		if (tracking)
		{
			return (byte *)(tracking+0x70);
		}
	}

	return NULL;
}

/* 0x0025cca0: 3 for a thing of prop_ref_data that has 3 for its short at
0x1C; 0 if not */
long prop_ref_unknown_0025cca0(
	long prop_ref_index)
{
	if (*(const short *)(ACTOR_PROP_REF_GET(prop_ref_index)+0x1C)==3)
	{
		return 3;
	}

	return 0;
}

/* 0x0025d700: what a thing of prop_ref_data has of tracking_data */
byte *prop_ref_index_get_tracking_unknown70(
	long prop_ref_index)
{
	return prop_ref_get_tracking_unknown70_inline(ACTOR_PROP_REF_GET(prop_ref_index));
}

/* 0x0025d740: the same, for a thing and not its number */
byte *prop_ref_get_tracking_unknown70(
	const byte *prop_ref)
{
	return prop_ref_get_tracking_unknown70_inline(prop_ref);
}

/* 0x0025d9b0: whether that of a thing of prop_ref_data names something at
0x50 */
boolean prop_ref_unknown_0025d9b0(
	long prop_ref_index)
{
	const byte *unknown70= prop_ref_get_tracking_unknown70_inline(ACTOR_PROP_REF_GET(prop_ref_index));

	if (unknown70)
	{
		return *(const long *)(unknown70+0x50)!=NONE;
	}

	return FALSE;
}

/* 0x0025da00: the byte at 0x88 of that, for a thing of prop_ref_data that
has 3 or more for its short at 0x24; false if it has less or names none */
boolean prop_ref_unknown_0025da00(
	const byte *prop_ref)
{
	if (*(const short *)(prop_ref+0x24)>=3)
	{
		const byte *unknown70= prop_ref_get_tracking_unknown70_inline(prop_ref);

		if (unknown70)
		{
			return unknown70[0x88];
		}
	}

	return FALSE;
}

/* 0x0025d970: whether that byte is 0, for the same things; false for the
others */
boolean prop_ref_unknown_0025d970(
	const byte *prop_ref)
{
	if (*(const short *)(prop_ref+0x24)>=3)
	{
		const byte *unknown70= prop_ref_get_tracking_unknown70_inline(prop_ref);

		if (unknown70)
		{
			return !unknown70[0x88];
		}
	}

	return FALSE;
}

/* 0x0026add0: whether a prop has 1 or more for its short at 4, or one of
its things of prop_ref_data (from the one at 0x18, each naming the next at
0x34) has 1 or more for its short at 0x24 */
boolean prop_unknown_0026add0(
	long prop_index)
{
	const byte *prop= ACTOR_PROP_GET(prop_index);
	long prop_ref_index;

	if (*(const short *)(prop+4)>=1)
	{
		return TRUE;
	}
	for (prop_ref_index= *(const long *)(prop+0x18); prop_ref_index!=NONE; )
	{
		const byte *prop_ref= ACTOR_PROP_REF_GET(prop_ref_index);

		if (*(const short *)(prop_ref+0x24)>=1)
		{
			return TRUE;
		}
		prop_ref_index= *(const long *)(prop_ref+0x34);
	}

	return FALSE;
}

/* 0x0026b8a0: whether one of the things of prop_ref_data of a prop has 1 or
2 for its short at 0x24 and names something other than a value at 4 */
boolean prop_unknown_0026b8a0(
	long prop_index,
	long value)
{
	long prop_ref_index= *(const long *)(ACTOR_PROP_GET(prop_index)+0x18);

	while (prop_ref_index!=NONE)
	{
		const byte *prop_ref= ACTOR_PROP_REF_GET(prop_ref_index);
		short unknown24= *(const short *)(prop_ref+0x24);

		prop_ref_index= *(const long *)(prop_ref+0x34);
		if (unknown24>=1 && unknown24<=2 && *(const long *)(prop_ref+4)!=value)
		{
			return TRUE;
		}
	}

	return FALSE;
}

/* 0x0026b230: the prop of a clump (from the one at 0x14, each naming the
next at 0x14) that names a value at 8; NONE if none */
long clump_find_prop(
	long clump_index,
	long value)
{
	long prop_index= *(const long *)(ACTOR_CLUMP_GET(clump_index)+0x14);

	while (prop_index!=NONE)
	{
		const byte *prop= ACTOR_PROP_GET(prop_index);

		if (*(const long *)(prop+8)==value)
		{
			return prop_index;
		}
		prop_index= *(const long *)(prop+0x14);
	}

	return NONE;
}

/* 0x00268810: a clump has 1 for its byte at 0x1C and its short at 0x24 */
void clump_unknown_00268810(
	long clump_index)
{
	byte *clump= ACTOR_CLUMP_GET(clump_index);

	clump[0x1C]= 1;
	*(short *)(clump+0x24)= 1;
}

/* 0x0026b630: a clump has 0 for its short at 0x3E. Always NONE */
short __attribute__((stdcall)) clump_unknown_0026b630(
	long clump_index,
	long unknown1,
	long unknown2)
{
	(void)unknown1;
	(void)unknown2;
	*(short *)(ACTOR_CLUMP_GET(clump_index)+0x3E)= 0;

	return NONE;
}

/* 0x0026b660: if more than 30 seconds have gone by since the time that a
clump has at 0x28, it has 0 for its byte at 0x30 and the answer is 0; NONE if
not */
short __attribute__((stdcall)) clump_unknown_0026b660(
	long clump_index,
	long unknown1,
	long unknown2)
{
	byte *clump= ACTOR_CLUMP_GET(clump_index);

	(void)unknown1;
	(void)unknown2;
	if ((float)(long)(game_time_globals->time-*(const unsigned long *)(clump+0x28))*game_time_globals->seconds_per_tick>30.0f)
	{
		clump[0x30]= 0;

		return 0;
	}

	return NONE;
}

/* 0x0026b750: whether something has more than zero for its real number at
0x14 */
long __attribute__((stdcall)) unknown_0026b750(
	long unknown1,
	long unknown2,
	const byte *unknown3)
{
	(void)unknown1;
	(void)unknown2;

	return *(const float *)(unknown3+0x14)>0.0f;
}

/* 0x0026b770: a clump, if there is one, has the time of the game for its
number at 0x40 */
void clump_unknown_0026b770(
	long clump_index)
{
	if (clump_index!=NONE)
	{
		*(unsigned long *)(ACTOR_CLUMP_GET(clump_index)+0x40)= game_time_globals->time;
	}
}

/* 0x0026bda0: a walk takes what a clump names at 0x18 for its second
number */
void clump_unknown_0026bda0(
	long clump_index,
	long *iterator)
{
	iterator[1]= *(const long *)(ACTOR_CLUMP_GET(clump_index)+0x18);
}

/* 0x0026be30: the next thing of prop_ref_data of a walk, or NULL when there
are no more: each names the next at 0x2C */
byte *prop_ref_iterator_next(
	long *iterator)
{
	long prop_ref_index= iterator[1];

	if (prop_ref_index!=NONE)
	{
		byte *prop_ref= (byte *)ACTOR_PROP_REF_GET(prop_ref_index);

		iterator[0]= prop_ref_index;
		iterator[1]= *(const long *)(prop_ref+0x2C);

		return prop_ref;
	}

	return NULL;
}

/* 0x0026e030: one of the points of 0x20 bytes of a thing of
dynamic_firing_point_data. The number has the thing in its high half (15
bits) and the point in its low one; NULL if the point is not one of those
that the thing counts at 8, or the thing does not have its bit at 0x40 */
byte *__attribute__((stdcall)) dynamic_firing_point_get_point(
	long index)
{
	byte *dynamic_firing_point= (byte *)dynamic_firing_point_data->data+((index>>16)&0x7FFF)*0x484;
	long point= (short)index;

	if (point>=0 && point<*(const short *)(dynamic_firing_point+8))
	{
		if (*(const unsigned long *)(dynamic_firing_point+0x40+(point>>5)*4)&(1<<(point&0x1F)))
		{
			return dynamic_firing_point+0x44+point*0x20;
		}
	}

	return NULL;
}

/* 0x00276990: the command script of unknown_00502410, if there is one,
takes a value for its number at 0x28, 1 for its short at 4 and zero for its
real number at 8 */
void __attribute__((stdcall)) command_script_unknown_00276990(
	long value)
{
	if (unknown_00502410!=NONE)
	{
		byte *command_script= (byte *)command_script_data->data+(unknown_00502410&0xFFFF)*0xD4;

		*(short *)(command_script+4)= 1;
		*(long *)(command_script+0x28)= value;
		*(float *)(command_script+8)= 0.0f;
	}
}

/* 0x005044c4, of swarms.c */
extern byte *unknown_005044c4;

/* 0x0046fca8: seven numbers that look like names of the kind that the tags
use (their top byte is how long the name is); the actors take three like
them at 0x450 */
long unknown_0046fca8[7]=
{
	0x6000085, 0x6000084, 0x6000086, 0x4000089, 0x7000039, 0x70000C9, 0x700002C
};

/* the command script of unknown_00502410 */
#define COMMAND_SCRIPT_CURRENT_GET() \
	((byte *)command_script_data->data+(unknown_00502410&0xFFFF)*0xD4)

/* what the two functions that follow do, with 2 or 1: the command script of
unknown_00502410, if there is one, takes a value for its bytes at 0x52 and
0x46; and if the value is not zero, a kind for its shorts at 0x54 and 0x48,
a number at 0x58 and 0x4C, and 0 for its byte at 0x51 */
static inline __attribute__((always_inline)) void command_script_unknown_00276f80_inline(
	boolean value,
	long number,
	short kind)
{
	if (unknown_00502410!=NONE)
	{
		byte *command_script= COMMAND_SCRIPT_CURRENT_GET();

		command_script[0x52]= value;
		if (value)
		{
			*(short *)(command_script+0x54)= kind;
			*(long *)(command_script+0x58)= number;
			command_script[0x51]= 0;
		}
		command_script[0x46]= value;
		if (value)
		{
			*(short *)(command_script+0x48)= kind;
			*(long *)(command_script+0x4C)= number;
		}
	}
}

/* 0x00276f80: that, with 2 */
void command_script_unknown_00276f80(
	boolean value,
	long number)
{
	command_script_unknown_00276f80_inline(value, number, 2);
}

/* 0x00277060: that, with 1 */
void command_script_unknown_00277060(
	boolean value,
	long number)
{
	command_script_unknown_00276f80_inline(value, number, 1);
}

/* 0x00277250: the command script of unknown_00502410, if there is one, has
0 for its short at 4 and some seconds, in ticks, for its real number at 8 */
void __attribute__((stdcall)) command_script_unknown_00277250(
	float seconds)
{
	if (unknown_00502410!=NONE)
	{
		byte *command_script= COMMAND_SCRIPT_CURRENT_GET();

		*(short *)(command_script+4)= 0;
		*(float *)(command_script+8)= (float)lrintf((float)game_time_globals->ticks_per_second*seconds);
	}
}

/* 0x002775d0: the command script of unknown_00502410, if there is one,
takes one of the seven numbers of unknown_0046fca8 at 0x94, or NONE for a
number that is not from 0 to 6 */
void command_script_unknown_002775d0(
	short index)
{
	if (unknown_00502410!=NONE)
	{
		byte *command_script= COMMAND_SCRIPT_CURRENT_GET();

		if (index>=0 && index<7)
		{
			*(long *)(command_script+0x94)= unknown_0046fca8[index];
		}
		else
		{
			*(long *)(command_script+0x94)= NONE;
		}
	}
}

/* 0x00277620: the command script of unknown_00502410, if there is one,
takes a kind from 0 to 5 for its short at 6, three numbers at 8, 0x28 and
0x2C, 0x12 for its short at 4 and 1 for its byte at 0x5C. Nothing for
another kind */
void command_script_unknown_00277620(
	short kind,
	long unknown8,
	long unknown28,
	long unknown2C)
{
	if (unknown_00502410!=NONE && kind>=0 && kind<6)
	{
		byte *command_script= COMMAND_SCRIPT_CURRENT_GET();

		*(short *)(command_script+6)= kind;
		*(long *)(command_script+8)= unknown8;
		*(short *)(command_script+4)= 0x12;
		*(long *)(command_script+0x28)= unknown28;
		*(long *)(command_script+0x2C)= unknown2C;
		command_script[0x5C]= 1;
	}
}

/* 0x00290b90: unknown_005044c4 starts with five things of 0x50 bytes. Each
one that names a value first is nothing again: its swarm (0x48), if it has
one, has 0 for its byte at 0x31, and the thing has NONE first and at 0x48 and
0 for its short at 0x4C */
void __attribute__((stdcall)) swarm_unknown_00290b90(
	long value)
{
	byte *element= unknown_005044c4;
	long count;

	for (count= 5; count>0; count--, element+= 0x50)
	{
		if (*(const long *)element==value)
		{
			long swarm_index= *(const long *)(element+0x48);

			if (swarm_index!=NONE)
			{
				((byte *)swarm_data->data+(swarm_index&0xFFFF)*0x34)[0x31]= 0;
			}
			*(long *)element= NONE;
			*(long *)(element+0x48)= NONE;
			*(short *)(element+0x4C)= 0;
		}
	}
}

/* 0x00290bf0: the objects of a swarm (from the one at 0x18; each names the
next at 0xC of what it has at the place that its short at 0x13A says) take a
value for their short at 0x12E. It stops at an object that has something in
its number at 0x134, or nothing at that place */
void swarm_unknown_00290bf0(
	long swarm_index,
	short value)
{
	long object_index= *(const long *)((const byte *)swarm_data->data+(swarm_index&0xFFFF)*0x34+0x18);

	while (object_index!=NONE)
	{
		byte *object= ACTOR_OBJECT_GET(object_index);
		uintptr_t unknown13A;

		if (*(const long *)(object+0x134))
		{
			break;
		}
		unknown13A= (uintptr_t)object+*(const short *)(object+0x13A);
		if (!unknown13A)
		{
			break;
		}
		*(short *)(object+0x12E)= value;
		object_index= *(const long *)(unknown13A+0xC);
	}
}

/* ---------- what a tag has, or the tag that it takes from

The seven functions that follow are given a tag that names another at 8, the
one that it takes what it does not have from (the tags of the characters do
that: each can be a kind of another). Each looks for the first, from the tag
up, that has something in one of its blocks, and gives where the things of
the block are; NULL if none has */

static inline __attribute__((always_inline)) void *tag_get_inherited_block_inline(
	long tag_index,
	long offset)
{
	while (tag_index!=NONE)
	{
		const byte *tag= (const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address;

		if (*(const long *)(tag+offset)>0)
		{
			return *(void *const *)(tag+offset+4);
		}
		tag_index= *(const long *)(tag+8);
	}

	return NULL;
}

/* 0x001e4a10: the block at 0x3C */
void *tag_get_inherited_unknown3C(
	long tag_index)
{
	return tag_get_inherited_block_inline(tag_index, 0x3C);
}

/* 0x001e49d0: the block at 0x44 */
void *tag_get_inherited_unknown44(
	long tag_index)
{
	return tag_get_inherited_block_inline(tag_index, 0x44);
}

/* 0x001e5160: the block at 0x54 */
void *tag_get_inherited_unknown54(
	long tag_index)
{
	return tag_get_inherited_block_inline(tag_index, 0x54);
}

/* 0x001e4a50: the block at 0x5C */
void *tag_get_inherited_unknown5C(
	long tag_index)
{
	return tag_get_inherited_block_inline(tag_index, 0x5C);
}

/* 0x001e4a90: the block at 0xB4 */
void *tag_get_inherited_unknownB4(
	long tag_index)
{
	return tag_get_inherited_block_inline(tag_index, 0xB4);
}

/* 0x001e4ad0: the block at 0xBC */
void *tag_get_inherited_unknownBC(
	long tag_index)
{
	return tag_get_inherited_block_inline(tag_index, 0xBC);
}

/* 0x001e4b10: the block at 0xC4 */
void *tag_get_inherited_unknownC4(
	long tag_index)
{
	return tag_get_inherited_block_inline(tag_index, 0xC4);
}

/* 0x001f8640: the byte that an actor has at 0x50C */
byte actor_get_unknown50C(
	long actor_index)
{
	return ACTOR_GET(actor_index)[0x50C];
}

/* 0x001f8660: whether an actor has that byte, and 1 for its short at 0x504 */
boolean actor_unknown504_is_1_with_unknown50C(
	long actor_index)
{
	const byte *actor= ACTOR_GET(actor_index);

	return actor[0x50C] && *(const short *)(actor+0x504)==1;
}

/* 0x001f86f0: whether an actor has 2 for its short at 0x504 */
boolean actor_unknown504_is_2(
	long actor_index)
{
	return *(const short *)(ACTOR_GET(actor_index)+0x504)==2;
}

/* 0x001f8720: whether it has 3 */
boolean actor_unknown504_is_3(
	long actor_index)
{
	return *(const short *)(ACTOR_GET(actor_index)+0x504)==3;
}

/* 0x001f8750: an actor takes 3 for that short: it counts one more at 0x508
and loses the byte at 0x50C */
void actor_set_unknown504_to_3(
	long actor_index)
{
	byte *actor= ACTOR_GET(actor_index);

	(*(short *)(actor+0x508))++;
	actor[0x50C]= 0;
	*(short *)(actor+0x504)= 3;
}

/* 0x001fa5e0: where one of the numbers that something has at 0x14 is, if
the last argument is NONE and the number is one of those that there are
(0x10); nowhere if not */
long *__attribute__((stdcall)) unknown10_get_element_for_none(
	long index,
	const byte *unknown,
	long value)
{
	if (value==NONE && index>=0 && index<*(const long *)(unknown+0x10))
	{
		return *(long *const *)(unknown+0x14)+index;
	}

	return NULL;
}

/* 0x001e69d0: whether one of four pairs of a time and a key (a short) has a
key and a time that is less than some ticks old */
boolean unknown_times_has_recent(
	const byte *times,
	long key,
	long ticks)
{
	unsigned long time= game_time_globals->time;
	long index;

	for (index= 0; index<4; index++)
	{
		if (*(const unsigned short *)(times+index*8+4)==key &&
			(long)(time-*(const unsigned long *)(times+index*8))<ticks)
		{
			return TRUE;
		}
	}

	return FALSE;
}

/* 0x001a58b0: whether an object is of the kinds 0, 1 or 7, without the bit
0x10 of its header's flags nor the bit 0x01 of its byte at 0x04, and for
the kinds 0 and 1 with less than 1.0 at 0x2B0 */
boolean actor_unknown_001a58b0(
	long object_index)
{
	const struct object_header_datum *header= (const struct object_header_datum *)object_header_data->data+(object_index&0xFFFF);
	const byte *object= (const byte *)header->datum;
	unsigned long kind= 1UL<<(header->type&0x1F);

	if (!(kind&0x83) || (header->flags&0x10) || (object[0x04]&1))
	{
		return FALSE;
	}

	return !(kind&3) || 1.0f>*(const real *)(object+0x2B0);
}

/* 0x001a6d30: FALSE when the unit of a player has no parent or NONE at its
short at 0x1FC; TRUE without a unit */
boolean actor_unknown_001a6d30(
	long player_index)
{
	long unit_index= *(const long *)((const byte *)player_data->data+(player_index&0xFFFF)*0x21C+0x2C);
	const byte *unit;

	if (unit_index==NONE)
	{
		return TRUE;
	}
	unit= (const byte *)((const struct object_header_datum *)object_header_data->data)[unit_index&0xFFFF].datum;

	return *(const long *)(unit+0x14)!=NONE && *(const short *)(unit+0x1FC)!=NONE;
}

/* 0x001a7ad0: the bit of the byte at 0x86 of an actor (0x888 bytes), and
from the bit 10 one of three by its bytes at 0x267 and 0x266 */
unsigned long actor_unknown_001a7ad0(
	long actor_index)
{
	const byte *actor= (const byte *)actor_data->data+(actor_index&0xFFFF)*0x888;
	long which= 0;

	if (actor[0x267])
	{
		which= 2;
	}
	else if (actor[0x266])
	{
		which= 1;
	}

	return (1UL<<(actor[0x86]&0x1F))|((1UL<<which)<<10);
}

/* 0x002055f0: something of 0x40 bytes is made from a byte, longs and shorts,
two points (0x1C and 0x28) and a long that can be given at 0x3A (NONE in
its two shorts when it is not) */
void __attribute__((stdcall)) ai_unknown_002055f0(
	byte *data,
	const unsigned long *second_point,
	byte value00,
	long value04,
	long value08,
	short value0C,
	long value10,
	long value14,
	short value18,
	const unsigned long *first_point,
	long value34,
	const unsigned long *value3A)
{
	data[0x00]= value00;
	*(long *)(data+0x04)= value04;
	*(long *)(data+0x08)= value08;
	*(short *)(data+0x0C)= value0C;
	*(long *)(data+0x10)= value10;
	*(long *)(data+0x14)= value14;
	*(short *)(data+0x18)= value18;
	((unsigned long *)(data+0x1C))[0]= first_point[0];
	((unsigned long *)(data+0x1C))[1]= first_point[1];
	((unsigned long *)(data+0x1C))[2]= first_point[2];
	((unsigned long *)(data+0x28))[0]= second_point[0];
	((unsigned long *)(data+0x28))[1]= second_point[1];
	((unsigned long *)(data+0x28))[2]= second_point[2];
	*(long *)(data+0x34)= value34;
	data[0x38]= 0;
	if (!value3A)
	{
		*(short *)(data+0x3A)= NONE;
		*(short *)(data+0x3C)= NONE;
		return;
	}
	*(unsigned long *)(data+0x3A)= *value3A;
}

/* 0x0023a220: whether a point is inside a polygon of so many points (two
reals each), going around it: on the inner side of each edge, or out of it
by no more than a tolerance */
boolean __attribute__((stdcall)) ai_point_in_polygon2d(
	const real *point,
	const real *points,
	short count,
	real tolerance)
{
	real limit= 0.0f-tolerance;
	short index;

	for (index= 0; index<count; index++)
	{
		long next= index+1>=count ? 0 : index+1;
		real cross= (point[1]-points[index*2+1])*(points[next*2]-points[index*2])
			- (point[0]-points[index*2])*(points[next*2+1]-points[index*2+1]);

		if (limit>cross)
		{
			return FALSE;
		}
	}

	return TRUE;
}

/* 0x001db070: the long at 0x10 of the entry (0x14 bytes) that matches a long
and five bytes, in the entries of the element (0x14 bytes, of the block at
0x1C) that matches a long and two bytes; NONE if none */
void __attribute__((stdcall)) ai_unknown_001db070(
	const byte *data,
	long *result,
	long value04,
	char value0A,
	char value0B,
	long entry04,
	long entry08,
	char entry0E,
	char entry0F,
	char entry0C)
{
	long index;

	for (index= 0; index<*(const long *)(data+0x1C); index++)
	{
		const byte *element= *(const byte *const *)(data+0x20)+index*0x14;

		if (*(const long *)(element+0x04)==value04 && (char)element[0x0A]==value0A && (char)element[0x0B]==value0B)
		{
			long other;

			for (other= 0; other<*(const long *)(element+0x0C); other++)
			{
				const byte *entry= *(const byte *const *)(element+0x10)+other*0x14;

				if (*(const long *)(entry+0x04)==entry04 && *(const long *)(entry+0x08)==entry08
					&& (char)entry[0x0E]==entry0E && (char)entry[0x0F]==entry0F && (char)entry[0x0C]==entry0C)
				{
					*result= *(const long *)(entry+0x10);
					return;
				}
			}
		}
	}
	*result= NONE;
}

/* 0x0023a160: whether a point is within a distance of the inner side of each
edge (not of length 0) of a polygon of so many points */
boolean __attribute__((stdcall)) ai_point_near_polygon2d(
	const real *points,
	const real *point,
	real distance,
	short count)
{
	real squared= distance*distance;
	short index;

	for (index= 0; index<count; index++)
	{
		long next= index+1>=count ? 0 : index+1;
		real x= point[0]-points[index*2];
		real y= point[1]-points[index*2+1];
		real edge_x= points[next*2]-points[index*2];
		real edge_y= points[next*2+1]-points[index*2+1];
		real length= edge_x*edge_x + edge_y*edge_y;

		if (length!=0.0f)
		{
			real cross= x*edge_y - y*edge_x;

			if (cross>0.0f && cross*cross>length*squared)
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}

/* 0x001e46c0: the next element in use of a data array of an iterator (at
0x04; the element at 0x00, its index with the salt at 0x08 and without at
0x0C), skipping those without their byte at 0x09 when the iterator asks
(0x10); the index is copied at 0x14. Nothing at the end, or without the
AI's globals on (their byte at 0x01) */
byte *ai_iterator_next_unknown_001e46c0(
	byte *iterator)
{
	if (!((const byte *)ai_globals)[1])
	{
		return NULL;
	}
	for (;;)
	{
		const struct data_array *data= *(const struct data_array *const *)(iterator+0x04);
		long index= *(const long *)(iterator+0x0C)+1;
		byte *element= NULL;

		for (; index>=0 && index<data->first_unallocated; index++)
		{
			if (data->in_use_bit_vector[index>>5]&(1UL<<(index&0x1F)))
			{
				element= (byte *)data->data+data->size*index;
				*(long *)(iterator+0x0C)= index;
				*(long *)(iterator+0x08)= (long)*(const short *)element<<16 | index;
				break;
			}
		}
		if (!element)
		{
			*(long *)(iterator+0x0C)= data->maximum_count;
			*(long *)(iterator+0x08)= NONE;
		}
		*(byte **)iterator= element;
		if (!element || !iterator[0x10] || element[9])
		{
			*(long *)(iterator+0x14)= *(const long *)(iterator+0x08);
			return element;
		}
	}
}

/* 0x001c9b50: bits of the numbers (at 0x254, below a count) of the actors,
with the AI on; TRUE */
boolean __attribute__((stdcall)) ai_unknown_001c9b50(
	unsigned long *bits,
	short count)
{
	byte iterator[0x18];
	const byte *actor;

	memset(bits, 0, (unsigned long)(((long)count+31)>>5)*4);
	if (!((const byte *)ai_globals)[1])
	{
		return TRUE;
	}
	*(struct data_array **)(iterator+0x04)= actor_data;
	*(long *)(iterator+0x0C)= NONE;
	*(long *)(iterator+0x08)= NONE;
	iterator[0x10]= TRUE;
	while ((actor= ai_iterator_next_unknown_001e46c0(iterator))!=NULL)
	{
		short number= *(const short *)(actor+0x254);

		if (number>=0 && number<count)
		{
			bits[number>>5]|= 1UL<<(number&0x1F);
		}
	}

	return TRUE;
}
