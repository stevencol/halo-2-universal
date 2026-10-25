/*
HAVOK.C

the part of the game that the physics are (Havok, the library): it starts and
stops the others of this folder. Worked out from halo2ship.exe; the name of
the file is a guess, by what its functions start.
*/

#include "cseries/cseries.h"

#include <stdint.h>
#include <string.h>
#include <math.h>

#include "cache/cache_files.h"
#include "cache/texture_cache.h"
#include "cache/animation_cache.h"
#include "game/game.h"
#include "memory/data.h"
#include "objects/objects.h"
#include "physics/havok_component.h"
#include "saved games/game_state.h"
#include "scenario/scenario.h"

/* ---------- constants */

enum
{
	/* of what unknown_0051e9cc points to: numbers that are NONE in a new map */
	HAVOK_UNKNOWN_0051E9CC_COUNT= 2000
};

/* ---------- globals */

/* 0x0051e9a0: four bytes of the state of the game: a number that is zero
when the part starts. 0x00479888: memory of the console that the part gives
back when it stops; the number before it says how far back it really starts.
0x0051e9cc: the 2000 numbers */
long *unknown_0051e9a0;
void *unknown_00479888;
long *unknown_0051e9cc;

/* 0x0051e9b8, of havok_component.c */
extern struct data_array *havok_component_data;

/* 0x00510c44, of game_unknown_00131ff0.c: 0x452E8 bytes of the console's
memory, which that part shares between its two things */
extern byte *unknown_00510c44;

/* 0x0047989c: something that gives memory, made at 0x105410 of the memory
of unknown_00479888: 0x24 bytes that start with where its functions are
(0x004576a4, a table of functions of the game). 0x00510c48: set when it is
made */
void *unknown_0047989c;
boolean unknown_00510c48;
void *unknown_004576a4[1];

/* ---------- prototypes */

void *VirtualAlloc(void *address, unsigned long size, unsigned long type, unsigned long protection);
int VirtualFree(void *address, unsigned long size, unsigned long type);
uint32_t GetLastError(void);

void havok_unknown_001edbc0(void);
void havok_unknown_00146b30(void);
void havok_unknown_00146b80(void);
void havok_unknown_00146de0(void);
void impacts_initialize(void);
void impacts_dispose(void);
void havok_components_initialize(void);

/* ---------- code */

/* 0x001c25a0: the first of the functions of the part (game.c): the four
bytes take their memory in the state of the game (game_state_malloc, which
the game has written here), and the others start */
void havok_initialize(
	void)
{
	unknown_0051e9a0= game_state_malloc(sizeof(long));
	*unknown_0051e9a0= 0;
	havok_unknown_001edbc0();
	impacts_initialize();
	havok_components_initialize();
	havok_unknown_00146b30();
	havok_unknown_00146b80();
}

/* 0x001c2600: the second: havok_unknown_00146de0, the memory of the console is
given back (VirtualFree; if that fails, the error is asked for), and so are
the table of the components (data_dispose, which the game has written here)
and those of the impacts */
void havok_dispose(
	void)
{
	havok_unknown_00146de0();
	if (!VirtualFree((byte *)unknown_00479888-((long *)unknown_00479888)[-1], 0, 0x8000))
	{
		GetLastError();
	}
	unknown_00479888= NULL;
	data_dispose(havok_component_data);
	havok_component_data= NULL;
	impacts_dispose();
}

/* 0x001c2670: the third: for a new map the 2000 numbers are NONE */
void havok_initialize_for_new_map(
	void)
{
	long index;

	for (index= 0; index<HAVOK_UNKNOWN_0051E9CC_COUNT; index++)
	{
		unknown_0051e9cc[index]= NONE;
	}
}

/* 0x001edbc0: the 2000 numbers take their memory in the state of the game
(game_state_malloc, which the game has written here), if they have none. In
the game it is far from the others of this file, so it may be of another */
void havok_unknown_001edbc0(
	void)
{
	if (!unknown_0051e9cc)
	{
		unknown_0051e9cc= game_state_malloc(HAVOK_UNKNOWN_0051E9CC_COUNT*(long)sizeof(long));
	}
}

/* 0x00146b30: the memory of the console that havok_dispose gives back is
taken: 0x105470 bytes that start at an address that is a multiple of 0x1000,
with their size and how far back the memory really starts kept just before
them; none if there is no memory (and the error is asked for). Far from the
others too */
void havok_unknown_00146b30(
	void)
{
	byte *memory= VirtualAlloc(NULL, 0x105470+0x1000, 0x101000, 4);

	if (!memory)
	{
		GetLastError();
		unknown_00479888= NULL;
	}
	else
	{
		byte *aligned= (byte *)(((uintptr_t)memory+0x1000)&~(uintptr_t)0xFFF);

		((long *)aligned)[-2]= 0x105470;
		((long *)aligned)[-1]= (long)(aligned-memory);
		unknown_00479888= aligned;
	}
}

/* 0x00146b80: the thing that gives memory is made, in its place of the
memory of unknown_00479888, to give the 0x452E8 bytes of unknown_00510c44:
from the first address that is a multiple of 16, and a size that is a
multiple of 4. Far from the others too */
void havok_unknown_00146b80(
	void)
{
	byte *memory= unknown_00510c44;
	unsigned long size= 0x452E8;
	byte *thing;

	unknown_00510c48= TRUE;
	if ((uintptr_t)memory&0xF)
	{
		unsigned long padding= (unsigned long)(0-(uintptr_t)memory)&0xF;

		memory+= padding;
		size-= padding;
	}
	thing= (byte *)((uintptr_t)unknown_00479888+0x105410);
	if (thing)
	{
		*(long *)(thing+4)= 1;
		*(long *)(thing+8)= 0;
		*(long *)(thing+0xC)= 0;
		*(long *)(thing+0x10)= 0;
		*(long *)(thing+0x14)= NONE;
		*(long *)(thing+0x20)= 0;
		*(byte **)(thing+0x18)= memory;
		*(unsigned long *)(thing+0x1C)= size&0x0FFFFFFC;
		*(void **)thing= unknown_004576a4;
		unknown_0047989c= thing;
	}
	else
	{
		unknown_0047989c= NULL;
	}
}

/* 0x004798a0: another thing that gives memory, as unknown_0047989c.
0x0047988c: the memory that it gives, which was taken from the cache of the
textures */
void *unknown_004798a0;
void *unknown_0047988c;

/* the ninth of the functions of a thing that gives memory: it is closed (the
number is zero here; a guess is that it says whether to free the thing) */
typedef void (__attribute__((thiscall)) *havok_memory_close_proc)(void *thing, long unknown);

/* 0x00146de0: the two things that give memory are closed, if they are
there, and there are none; the memory of the second goes back to the cache
of the textures. Far from the others too */
void havok_unknown_00146de0(
	void)
{
	if (unknown_0047989c)
	{
		((havok_memory_close_proc)(*(void ***)unknown_0047989c)[8])(unknown_0047989c, 0);
		unknown_0047989c= NULL;
		unknown_00510c48= FALSE;
	}
	if (unknown_004798a0)
	{
		((havok_memory_close_proc)(*(void ***)unknown_004798a0)[8])(unknown_004798a0, 0);
		unknown_004798a0= NULL;
		texture_cache_return_memory(unknown_0047988c);
		unknown_0047988c= NULL;
	}
}

/* 0x0051ebfc and 0x0051ec00, of impacts.c */
extern struct data_array *impact_data;
extern struct data_array *impact_array_data;

/* 0x0047f04c: how many times the state of the game is being written or read
(it starts at NONE in the game). 0x0047f058 and 0x0047f059: whether two
halves of the physics are there. 0x0047f048: something of Havok's (its world,
perhaps), and 0x0051ecac something that is given to it */
long unknown_0047f04c;
boolean unknown_0047f058;
boolean unknown_0047f059;
void *unknown_0047f048;
void *unknown_0051ecac;

void unknown_001c2b10(void);
void havok_unknown_001c29d0(void);
void havok_unknown_001c2890(void);
void unknown_001c2910(void);
void havok_unknown_001c2a10(void);
void __attribute__((stdcall)) havok_unknown_001c39c0(byte *entity, boolean unknown2);
/* its first argument is in ecx in the game (a method of a class of Havok's) */
void unknown_0030be40(void *unknown, void *unknown2);

/* 0x001c3540: before the state of the game is written, and before it is
read (game_state.c). Without the bit 0 the first half of the physics goes,
and the impacts with it; with the bit 1 the second goes. What the bits say
is not known */
void __attribute__((stdcall)) havok_before_save_or_load(
	unsigned long flags)
{
	if (!(flags&1))
	{
		unknown_001c2b10();
		data_delete_all(impact_data);
		data_delete_all(impact_array_data);
		unknown_0047f058= FALSE;
	}
	if (flags&2)
	{
		havok_unknown_001c29d0();
		havok_unknown_001c2890();
		unknown_0047f059= FALSE;
	}
	unknown_0047f04c++;
}

/* 0x001c3590: after the state of the game is written, and after it is read:
the halves come back, the second first */
void __attribute__((stdcall)) havok_after_save_or_load(
	unsigned long flags)
{
	unknown_0047f04c--;
	if (flags&2)
	{
		unknown_0047f059= TRUE;
		unknown_001c2910();
		havok_unknown_001c39c0(unknown_0047f048, 0);
		unknown_0030be40(unknown_0047f048, unknown_0051ecac);
	}
	if (!(flags&1))
	{
		unknown_0047f058= TRUE;
		havok_unknown_001c2a10();
	}
}

/* 0x0051e9a4: the world of the physics, it seems: what the thing of
0x0047f048 (one of its bodies, then) is put in and taken out of. 0x0047f050:
how many things are in it. 0x004f55b0: another thing of Havok's that is let
go of with that one */
void *unknown_0051e9a4;
long unknown_0047f050;
void *unknown_004f55b0;

/* functions of Havok's; the first argument of each is in ecx in the game.
0x0030bd50 and 0x0030bc90 are of a thing of the world, 0x0030f800 and
0x0030f2d0 of the world: they take a thing out of it and put it in */
void unknown_0030bd50(void *entity, void *unknown);
void unknown_0030bc90(void *entity);
void unknown_0030f800(void *world, byte *removed, void *entity);
void unknown_0030f2d0(void *world, void *entity);
void unknown_00278f00(void);
boolean unknown_00279740(void *object);
void havok_unknown_001c51c0(long unknown);

/* a thing of Havok's that is counted (a short, at 6): one less has it, and
if nothing has it, it is no more: unless unknown_00279740 says that it is to
stay, its first function is called with 1 */
static void havok_object_release(
	void *object)
{
	short *count= (short *)((byte *)object+6);

	if (--*count==0)
	{
		if (unknown_00279740(object))
		{
			*count= 1;
		}
		else
		{
			((void (__attribute__((thiscall)) *)(void *, long))(*(void ***)object)[0])(object, 1);
		}
	}
}

/* 0x001c29d0: the thing of 0x0047f048 loses what unknown_0051ecac is to it,
and is taken out of the world */
void havok_unknown_001c29d0(
	void)
{
	byte removed;

	unknown_0030bd50(unknown_0047f048, unknown_0051ecac);
	unknown_0047f050--;
	unknown_0030f800(unknown_0051e9a4, &removed, unknown_0047f048);
	unknown_00278f00();
}

/* 0x001c2890: the thing of 0x004f55b0, if there is one, and that of
0x0047f048 are let go of, and there are none */
void havok_unknown_001c2890(
	void)
{
	if (unknown_004f55b0)
	{
		havok_object_release(unknown_004f55b0);
		unknown_004f55b0= NULL;
	}
	havok_object_release(unknown_0047f048);
	unknown_0047f048= NULL;
}

/* 0x001c39c0: a thing is put in the world. When the world has 0x352 things
or more, havok_unknown_001c51c0 is told first, with what the thing has for the key
0x2001 in its list of keys and values (at 0x30; so many at 0x34), or zero.
With the second argument, a thing whose number at 0x40 of what it has at
0x44 is 1 has unknown_0030bc90 done to it */
void __attribute__((stdcall)) havok_unknown_001c39c0(
	byte *entity,
	boolean unknown2)
{
	if (unknown_0047f050>=0x352)
	{
		const long *properties= *(const long *const *)(entity+0x30);
		long count= *(const long *)(entity+0x34);
		long value= 0;
		long index;

		for (index= 0; index<count; index++)
		{
			if (properties[index*2]==0x2001)
			{
				value= properties[index*2+1];
				break;
			}
		}
		havok_unknown_001c51c0(value);
	}
	unknown_0047f050++;
	unknown_0030f2d0(unknown_0051e9a4, entity);
	if (unknown2 && *(const long *)(*(const byte *const *)(entity+0x44)+0x40)==1)
	{
		unknown_0030bc90(entity);
	}
	unknown_00278f00();
}

/* 0x004f55b4: two users of the second memory of the physics: whether each is
there, and what it is known by. 0x0051e9b0 and 0x0051e9b4: something that is
there, and a byte that is cleared when no user is left */
struct havok_unknown_004f55b4
{
	boolean active;
	long unknown4;
	long unknown8;
};

struct havok_unknown_004f55b4 unknown_004f55b4[2];
long unknown_0051e9b0;
boolean unknown_0051e9b4;

/* 0x001cab80: the user that is known by a number is there no more; and when
none is left, the two bytes are cleared, and if unknown_0051e9b0 is something,
the thing that gives the memory is made again (havok_unknown_00146b80) */
void havok_unknown_001cab80(
	long unknown)
{
	long count= 0;
	long index;

	for (index= 0; index<2; index++)
	{
		if (unknown_004f55b4[index].active)
		{
			if (unknown_004f55b4[index].unknown4==unknown)
			{
				unknown_004f55b4[index].active= FALSE;
			}
			else
			{
				count++;
			}
		}
	}
	if (count==0)
	{
		unknown_0051e9b4= FALSE;
		unknown_00510c48= FALSE;
		if (unknown_0051e9b0)
		{
			havok_unknown_00146b80();
		}
	}
}

/* 0x0047f054: the time of the game when the objects last got their parts of
the physics */
long unknown_0047f054;

void unknown_00146bf0(void);
void __attribute__((stdcall)) unknown_001c35f0(long object_index);
void unknown_001c50c0(void);

/* 0x001c2a10: the first half of the physics comes back (after the state of
the game is written or read): no object has what it is in the physics (a
number at 0xB4), and then each object that has none gets its own
(unknown_001c35f0, between two of unknown_00146bf0); and the time is kept. The
objects are taken from the table by the index that the walk leaves */
void havok_unknown_001c2a10(
	void)
{
	struct object_iterator iterator;

	iterator.signature= OBJECT_ITERATOR_SIGNATURE;
	iterator.type_mask= 0xFFFFFFFF;
	iterator.header_flags= 0;
	iterator.absolute_index= 0;
	iterator.index= NONE;
	while (object_iterator_next(&iterator)!=NULL)
	{
		*(long *)((byte *)((struct object_header_datum *)object_header_data->data)[iterator.index&0xFFFF].datum+0xB4)= NONE;
	}
	iterator.signature= OBJECT_ITERATOR_SIGNATURE;
	iterator.type_mask= 0xFFFFFFFF;
	iterator.header_flags= 0;
	iterator.absolute_index= 0;
	iterator.index= NONE;
	while (object_iterator_next(&iterator)!=NULL)
	{
		long object_index= iterator.index;

		if (*(long *)((byte *)((struct object_header_datum *)object_header_data->data)[object_index&0xFFFF].datum+0xB4)==NONE)
		{
			unknown_00146bf0();
			unknown_001c35f0(object_index);
			unknown_00278f00();
			unknown_00146bf0();
		}
	}
	unknown_001c50c0();
	unknown_0047f054= (long)game_time_globals->time;
}

/* 0x001c4040: something of the world of the physics is taken out, to make
room: whether it could */
boolean unknown_001c4040(boolean unknown, long pass, long unknown3, long unknown4, long unknown5);

/* 0x001c51c0: while the world of the physics has more than 0x34E things,
room is made (unknown_001c4040): with passes from 0 to 2 that go on when
nothing could be taken out, and then the same three passes with its first
argument set */
void havok_unknown_001c51c0(
	long unknown)
{
	boolean second= FALSE;
	long pass= 0;

	while (unknown_0047f050>0x34E)
	{
		if (!unknown_001c4040(second, pass, 0, 1, unknown))
		{
			if (pass<2)
			{
				pass++;
			}
			else if (!second)
			{
				second= TRUE;
				pass= 0;
			}
		}
	}
}

/* ---------- something with a tag at 0x68

The four functions that follow (0x1cbd30 to 0x1cc2d0) are between those of
the minds and those of the components of the physics in the game, and take
something that names a tag at 0x68. Neither the thing nor the tag is known */

#define UNKNOWN68_TAG_GET(unknown) \
	((const byte *)(uintptr_t)global_tag_instances[*(const long *)((const byte *)(unknown)+0x68)&0xFFFF].address)

/* 0x001cc2d0: the tag of the thing */
const byte *unknown68_get_tag(
	const byte *unknown)
{
	return UNKNOWN68_TAG_GET(unknown);
}

/* 0x001cbd30: the short at 0xC of the tag of the thing */
short unknown68_get_tag_unknownC(
	const byte *unknown)
{
	return *(const short *)(UNKNOWN68_TAG_GET(unknown)+0xC);
}

/* 0x001cbe90: the same, as a number; zero if the tag is nowhere */
long unknown68_get_tag_unknownC_or_zero(
	const byte *unknown)
{
	const byte *tag= UNKNOWN68_TAG_GET(unknown);

	if (tag)
	{
		return *(const short *)(tag+0xC);
	}

	return 0;
}

/* 0x001cbe20: one of the things of 0xC bytes that the tag of the thing has
at 0x18; NULL for NONE */
const byte *unknown68_get_tag_unknown18(
	long index,
	const byte *unknown)
{
	if (index==NONE)
	{
		return NULL;
	}

	return *(const byte *const *)(UNKNOWN68_TAG_GET(unknown)+0x18)+index*0xC;
}

/* 0x001dafc0: the tag that one of the things of 0x20 bytes that something
has at 0x4C names at 4; NULL for a number past the last, or if it names
none */
void *unknown4C_get_tag(
	long index,
	const byte *unknown)
{
	if (index<*(const long *)(unknown+0x4C))
	{
		long tag_index= *(const long *)(*(const byte *const *)(unknown+0x50)+index*0x20+4);

		if (tag_index!=NONE)
		{
			return (void *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address;
		}
	}

	return NULL;
}

/* ---------- lists of pairs of shorts

The five functions that follow (0x1dadb0 to 0x1dae80) take something that
has at 0x4C a block of pairs of shorts, a key and a value, and at 0x54
another block. They are next to unknown4C_get_tag in the game, which reads
the block at 0x4C in another way: the things are not the same */

/* 0x001dadb0: the value of the first pair that has a key; NONE if none */
short unknown4C_find_pair(
	const byte *unknown,
	long key)
{
	long count= *(const long *)(unknown+0x4C);
	const short *pair= *(const short *const *)(unknown+0x50);
	long index;

	for (index= 0; index<count; index++, pair+= 2)
	{
		if (pair[0]==key)
		{
			return pair[1];
		}
	}

	return NONE;
}

/* 0x001dade0: the value of the first pair that has a key and a value that
is more than a number; NONE if none */
short unknown4C_find_pair_above(
	const byte *unknown,
	long key,
	long minimum)
{
	long count= *(const long *)(unknown+0x4C);
	const short *pair= *(const short *const *)(unknown+0x50);
	long index;

	for (index= 0; index<count; index++, pair+= 2)
	{
		if (pair[0]==key && pair[1]>minimum)
		{
			return pair[1];
		}
	}

	return NONE;
}

/* 0x001dae20: the value of the first pair whose key is 0 */
short unknown4C_find_pair0(
	const byte *unknown)
{
	long count= *(const long *)(unknown+0x4C);
	const short *pair= *(const short *const *)(unknown+0x50);
	long index;

	for (index= 0; index<count; index++, pair+= 2)
	{
		if (pair[0]==0)
		{
			return pair[1];
		}
	}

	return NONE;
}

/* 0x001dae50: the value of the first pair whose key is 1 */
short unknown4C_find_pair1(
	const byte *unknown)
{
	long count= *(const long *)(unknown+0x4C);
	const short *pair= *(const short *const *)(unknown+0x50);
	long index;

	for (index= 0; index<count; index++, pair+= 2)
	{
		if (pair[0]==1)
		{
			return pair[1];
		}
	}

	return NONE;
}

/* 0x001dae80: the first short of the block at 0x54; NONE if it has none */
long unknown54_get_first(
	const byte *unknown)
{
	if (*(const long *)(unknown+0x54)>0)
	{
		return **(const short *const *)(unknown+0x58);
	}

	return NONE;
}

/* six small functions of things that are not known, which are among the
ones of this file in the game */

/* 0x001c2460: where something has one of its things: of 0x50 bytes, 0x20
after what it has at 0x2C, if it has a number at 0x28; of 0x80 bytes, 0x30
after what it has at 0x34, if not */
byte *unknown28_get_element(
	const byte *unknown,
	long index)
{
	if (*(const long *)(unknown+0x28))
	{
		return *(byte *const *)(unknown+0x2C)+index*0x50+0x20;
	}

	return *(byte *const *)(unknown+0x34)+index*0x80+0x30;
}

/* 0x001c58a0: whether unknown_0047f054 is a time of the last three ticks */
boolean unknown_0047f054_is_recent(
	void)
{
	long value= unknown_0047f054;

	if (value!=NONE)
	{
		unsigned long time= game_time_globals->time;

		return value>=(long)(time-3) && value<=(long)time;
	}

	return FALSE;
}

/* 0x001cbde0: which of the things of 0x20 bytes of the tag of the thing
(the block at 0xC) has a number for its first; NONE if none */
long unknown68_find_tag_unknownC(
	const byte *unknown,
	long key)
{
	const byte *tag= UNKNOWN68_TAG_GET(unknown);
	long count= *(const long *)(tag+0xC);
	const byte *element= *(const byte *const *)(tag+0x10);
	long index;

	for (index= 0; index<count; index++, element+= 0x20)
	{
		if (*(const long *)element==key)
		{
			return index;
		}
	}

	return NONE;
}

/* 0x001ce2e0: the byte that something has at 4, to where the caller says.
In the game the thing is in ecx (a class, most likely) */
void __attribute__((stdcall)) unknown_get_unknown4_byte(
	const byte *unknown,
	byte *value)
{
	*value= unknown[4];
}

/* 0x001dd490: which of the things of 0x20 bytes of a block at 0xC has all
of some bits in its byte at 0xA; NONE if none */
long unknownC_find_with_flags(
	const byte *unknown,
	unsigned long flags)
{
	long count= *(const long *)(unknown+0xC);
	const byte *element= *(const byte *const *)(unknown+0x10);
	long index;

	for (index= 0; index<count; index++, element+= 0x20)
	{
		if ((element[0xA]&flags)==flags)
		{
			return index;
		}
	}

	return NONE;
}

/* 0x001dd7a0: 0x14 bytes of something are left with nothing in them */
void unknown_001dd7a0_clear(
	byte *unknown)
{
	*(long *)unknown= NONE;
	*(long *)(unknown+4)= NONE;
	*(short *)(unknown+8)= NONE;
	*(short *)(unknown+0xA)= 0;
	*(long *)(unknown+0xC)= NONE;
	*(long *)(unknown+0x10)= NONE;
}

/* 0x001c2330: the longs for so many bits of a list are those of another
and a mask */
void __attribute__((stdcall)) havok_unknown_001c2330(
	const unsigned long *mask,
	long count,
	const unsigned long *bits,
	unsigned long *result)
{
	long longs= (count+0x1F)>>5;
	long index;

	for (index= 0; index<longs; index++)
	{
		result[index]= bits[index]&mask[index];
	}
}

/* 0x001c3930: TRUE unless the first byte of a tag is a kind of 0, 1, 7, 11
or 12 (the bits of 0x1883, by the shift of the processor); then whether
there is room: unknown_0051e9a0 below 0x200 without unknown_0047f058, or
fewer havok components than they can be */
boolean havok_unknown_001c3930(
	long tag_index)
{
	byte kind= *(const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address;

	if (!((1UL<<(kind&0x1F))&0x1883))
	{
		return TRUE;
	}
	if (!unknown_0047f058)
	{
		return *(const long *)unknown_0051e9a0<0x200;
	}

	return havok_component_data->actual_count<havok_component_data->maximum_count;
}

/* 0x001c5e80: a header of 0x10 bytes ('papb' ... 'pape') with the eight
bytes of something, or zeros for none */
void havok_unknown_001c5e80(
	unsigned long *header,
	const byte *data)
{
	if (!data)
	{
		header[0]= 0;
		header[1]= 0;
		header[2]= 0;
		header[3]= 0;
	}
	else
	{
		header[1]= *(const unsigned long *)data;
		((byte *)header)[8]= data[4];
		((byte *)header)[9]= data[5];
		((byte *)header)[10]= data[6];
		((byte *)header)[11]= data[7];
	}
	header[0]= 0x62706170;
	header[3]= 0x65706170;
}

/* 0x001c61c0: the same with 'nocb' ... 'noce' and seven bytes */
void havok_unknown_001c61c0(
	byte *header,
	const byte *data)
{
	if (!data)
	{
		memset(header, 0, 0x0F);
	}
	else
	{
		*(unsigned long *)(header+4)= *(const unsigned long *)data;
		header[8]= data[4];
		header[9]= data[5];
		header[10]= data[6];
	}
	*(unsigned long *)header= 0x62636F6E;
	*(unsigned long *)(header+0x0B)= 0x65636F6E;
}

/* something of 0x20 bytes that is nothing: NONE in its long and shorts,
zeros, 1.0 and 0.0 */
static inline __attribute__((always_inline)) void havok_unknown_clear(
	byte *data)
{
	*(long *)data= NONE;
	*(short *)(data+0x04)= NONE;
	*(short *)(data+0x06)= NONE;
	*(real *)(data+0x1C)= 0.0f;
	data[0x10]= 0;
	data[0x11]= 0;
	*(short *)(data+0x12)= 0;
	*(real *)(data+0x18)= 1.0f;
	*(short *)(data+0x14)= 0;
	*(short *)(data+0x16)= 0;
	*(long *)(data+0x08)= NONE;
	*(short *)(data+0x0E)= NONE;
	data[0x0C]= 0xFF;
	data[0x0D]= 0xFF;
}

/* 0x001c62f0, 0x001c6340 and 0x001c63f0: it is made nothing (three copies
of the same in the game) */
void havok_unknown_001c62f0(
	byte *data)
{
	havok_unknown_clear(data);
}

void havok_unknown_001c6340(
	byte *data)
{
	havok_unknown_clear(data);
}

void havok_unknown_001c63f0(
	byte *data)
{
	havok_unknown_clear(data);
}

/* 0x001cb0a0: three entries of 0x20 bytes get NONE in their longs at 0x00
and 0x08 and their shorts at 0x04 and 0x06, and 0 at 0x10 */
void havok_unknown_001cb0a0(
	byte *data)
{
	long index;

	for (index= 2; index>=0; index--)
	{
		byte *entry= data+index*0x20;

		*(long *)entry= NONE;
		*(short *)(entry+0x04)= NONE;
		*(short *)(entry+0x06)= NONE;
		*(long *)(entry+0x08)= NONE;
		entry[0x10]= 0;
	}
}

/* 0x001ccc50: two of those things of 0x20 bytes (at 0x20 and 0x40) of
something are made nothing */
void havok_unknown_001ccc50(
	byte *data)
{
	havok_unknown_clear(data+0x20);
	havok_unknown_clear(data+0x40);
}

/* 0x001c6470: one of those things of 0x20 bytes is made from two longs (not
NONE, the second not NONE in its high short), a long at 0x08, a short at
0x12 (whose bit 15 becomes the bit 0x02 at 0x10), three bytes and 1.0;
FALSE if it cannot be */
boolean __attribute__((stdcall)) havok_unknown_001c6470(
	byte *data,
	long first,
	unsigned long flags,
	long second,
	long third,
	byte value0C,
	byte value0D,
	signed char value0E)
{
	if (first==NONE || (short)((unsigned long)second>>16)==NONE)
	{
		return FALSE;
	}
	*(short *)(data+0x04)= NONE;
	*(short *)(data+0x06)= NONE;
	data[0x10]= 0;
	data[0x11]= 0;
	*(short *)(data+0x14)= 0;
	*(short *)(data+0x16)= 0;
	*(long *)(data+0x08)= third;
	data[0x0C]= value0C;
	*(short *)(data+0x12)= (short)flags;
	data[0x0D]= value0D;
	*(long *)data= first;
	*(real *)(data+0x1C)= 0.0f;
	*(long *)(data+0x04)= second;
	*(real *)(data+0x18)= 1.0f;
	*(short *)(data+0x0E)= value0E;
	data[0x10]= (byte)(flags>>14)&2;
	return TRUE;
}

/* 0x001cf8b0: two reals of something (0x08, 0x0C) from two of another (0x3C,
0x40); the first is 0.0 when the object of a havok component is of the kind
1 with the bit 19 of the long at 0x1EC of its definition, and something at
0x248 or the bit 0x02 of its flags at 0x134 */
void havok_unknown_001cf8b0(
	long havok_component_index,
	unsigned long *result,
	const unsigned long *values)
{
	long object_index= *(const long *)((const byte *)havok_component_data->data+(havok_component_index&0xFFFF)*0xA0+8);
	const struct object_header_datum *header= (const struct object_header_datum *)object_header_data->data+(object_index&0xFFFF);

	if (((1UL<<(header->type&0x1F))&2))
	{
		const byte *object= (const byte *)header->datum;
		const byte *definition= (const byte *)(uintptr_t)global_tag_instances[*(const long *)object&0xFFFF].address;

		if (((*(const unsigned long *)(definition+0x1EC)>>19)&1)
			&& (*(const long *)(object+0x248)!=NONE || ((*(const unsigned long *)(object+0x134)>>1)&1)))
		{
			*(real *)&result[2]= 0.0f;
			result[3]= values[0x40/4];
			return;
		}
	}
	result[2]= values[0x3C/4];
	result[3]= values[0x40/4];
}

/* 0x0046fbf4: a byte that is TRUE when the game starts */
byte unknown_0046fbf4= TRUE;

/* 0x001cb680: of the two parts (0x00 and 0x20) of something, one gets the
bits 0x380 of its short at 0x12 and the other loses them, by its real at
0x80 (below 0.45, or above 0.55; between, nothing changes), and the bit
0x20 of its byte at 0x6C says which; a part that is NONE keeps its short */
void havok_unknown_001cb680(
	byte *data)
{
	unsigned short first= *(const unsigned short *)(data+0x12);
	unsigned short second= *(const unsigned short *)(data+0x32);
	real value= *(const real *)(data+0x80);

	if (!unknown_0046fbf4 || *(const long *)(data+0x20)==NONE || *(const short *)(data+0x26)==NONE || 0.45f>value)
	{
		data[0x6C]&= ~0x20;
		first&= 0xFC7F;
		second|= 0x380;
	}
	else if (value>0.55f)
	{
		data[0x6C]|= 0x20;
		first|= 0x380;
		second&= 0xFC7F;
	}
	else
	{
		return;
	}
	if (*(const long *)data!=NONE && *(const short *)(data+0x06)!=NONE)
	{
		*(unsigned short *)(data+0x12)= first;
	}
	if (*(const long *)(data+0x20)!=NONE && *(const short *)(data+0x26)!=NONE)
	{
		*(unsigned short *)(data+0x32)= second;
	}
}

/* 0x001cafc0: something of 0x84 bytes is made nothing: three of those things
of 0x20 bytes, and then NONE and zeros */
void havok_unknown_001cafc0(
	byte *data)
{
	havok_unknown_clear(data);
	havok_unknown_clear(data+0x20);
	havok_unknown_clear(data+0x40);
	*(short *)(data+0x6C)= 0;
	*(short *)(data+0x6E)= 0;
	*(long *)(data+0x68)= NONE;
	data[0x65]= 0;
	data[0x64]= 0;
	data[0x67]= 0;
	data[0x61]= 0;
	data[0x60]= 0;
	data[0x63]= 0;
	*(long *)(data+0x70)= NONE;
	*(long *)(data+0x7C)= NONE;
	*(long *)(data+0x74)= NONE;
	*(long *)(data+0x78)= NONE;
	*(long *)(data+0x80)= 0;
}

/* 0x001ccba0: three of those things of 0x20 bytes (at 0x00, 0x20 and 0x40)
are made nothing */
void havok_unknown_001ccba0(
	byte *data)
{
	havok_unknown_clear(data);
	havok_unknown_clear(data+0x20);
	havok_unknown_clear(data+0x40);
}

/* whether the object of a havok component is of the kind 0 */
static inline __attribute__((always_inline)) boolean havok_component_of_kind0(
	long havok_component_index)
{
	long object_index;

	if (havok_component_index==NONE)
	{
		return FALSE;
	}
	object_index= *(const long *)((const byte *)havok_component_data->data+(havok_component_index&0xFFFF)*0xA0+8);

	return (1UL<<(((const struct object_header_datum *)object_header_data->data)[object_index&0xFFFF].type&0x1F))&1;
}

/* 0x002276d0: whether two havok components each have the bit 11 of their
flags (0x04) when their objects are of the kind 0 */
long __attribute__((stdcall)) havok_unknown_002276d0(
	long first,
	long second)
{
	boolean first_kind0= havok_component_of_kind0(first);
	boolean second_kind0= havok_component_of_kind0(second);
	boolean first_bit= TRUE;
	boolean second_bit= TRUE;

	if (first_kind0)
	{
		first_bit= (*(const unsigned long *)((const byte *)havok_component_data->data+(first&0xFFFF)*0xA0+4)>>11)&1;
	}
	if (second_kind0)
	{
		second_bit= (*(const unsigned long *)((const byte *)havok_component_data->data+(second&0xFFFF)*0xA0+4)>>11)&1;
	}

	return first_bit && second_bit;
}

/* 0x001caed0: as 0x001cafc0, with the eight bytes from 0x60 cleared first
(the game clears the three things of 0x20 bytes first too, which 0x001cafc0
does again) */
void havok_unknown_001caed0(
	byte *data)
{
	memset(data+0x60, 0, 8);
	havok_unknown_001cafc0(data);
}

/* 0x004798b0: something of Havok that only its address is asked of */
byte unknown_004798b0[0x10];

/* 0x001473b0: where that is */
void *havok_unknown_001473b0(
	void)
{
	return unknown_004798b0;
}

/* 0x002365e0: two numbers by a kind (0 and 1); NONE for another */
long havok_unknown_002365e0(
	long kind)
{
	switch (kind)
	{
	case 0:
		return 0x070002FF;
	case 1:
		return 0x08000300;
	default:
		return NONE;
	}
}

/* 0x001c55b0: in a list (each points to the next at 0x0C), the long at
0x04 of the one before the first whose thing (at 0x00) has 0xCABCABB0 at
0x08; NONE if none has */
long havok_unknown_001c55b0(
	const byte *node)
{
	const byte *next;

	for (next= *(const byte *const *)(node+0x0C); next; next= *(const byte *const *)(node+0x0C))
	{
		if (*(const unsigned long *)(*(const byte *const *)next+0x08)==0xCABCABB0)
		{
			break;
		}
		node= next;
	}

	return *(const byte *const *)(node+0x0C) ? *(const long *)(node+0x04) : NONE;
}

/* 0x0022cb90: sixteen differences of longs (from 0xF0348 to 0xF0384 less
those 0x44 after them) by longs (from 0x6C to 0xA8), added, as a real
scaled by 1.0344942e-06 */
real havok_unknown_0022cb90(
	const byte *data)
{
	const long *at= (const long *)(data+0xF0384);
	long total= 0;
	long index;

	for (index= 0; index<16; index++)
	{
		total+= (at[0]-at[0x44/4])*at[-0xF02DC/4];
		at--;
	}

	return (real)total*1.0344942e-06f;
}

/* 0x00224350: the bigger of the two reals (0x04 and 0x08) of what the thing
of something points to at 0x48; 0 and 1 when its byte at 0x01 has some of
its high bits */
real havok_unknown_00224350(
	const byte *data)
{
	const byte *definition= *(const byte *const *)(*(const byte *const *)(data+0x08)+0x48);
	real low= 0.0f;
	real high= 1.0f;

	if (!(definition[1]>>4))
	{
		low= *(const real *)(definition+0x04);
		high= *(const real *)(definition+0x08);
	}

	return low>high ? low : high;
}

/* 0x001c6bb0: the definition of a tag (0x00) or, by an index (0x04, NONE
for the tag itself), the definition of the tag of an element (0x20 bytes,
at 0x50, as many as at 0x4C) of it; NULL for an index past them or an
element without a tag */
void *havok_unknown_001c6bb0(
	const byte *reference)
{
	const byte *definition= (const byte *)(uintptr_t)global_tag_instances[*(const long *)reference&0xFFFF].address;
	short index= *(const short *)(reference+0x04);
	long tag;

	if (index==NONE)
	{
		return (void *)definition;
	}
	if (index>=*(const long *)(definition+0x4C))
	{
		return NULL;
	}
	tag= *(const long *)(*(const byte *const *)(definition+0x50)+index*0x20+0x04);
	if (tag==NONE)
	{
		return NULL;
	}

	return (void *)(uintptr_t)global_tag_instances[tag&0xFFFF].address;
}

/* 0x00246eb0: how long the vector at 0x28 of something is, less a half,
from 0 to 1 */
real havok_unknown_00246eb0(
	const byte *data)
{
	const real *vector= (const real *)(data+0x28);
	long double i= vector[0];
	long double j= vector[1];
	long double k= vector[2];
	real value= (real)(__builtin_sqrtl(i*i+j*j+k*k)-0.5L);

	if (0.0f>value)
	{
		return 0.0f;
	}
	if (value>1.0f)
	{
		return 1.0f;
	}

	return value;
}

/* 0x001dd560: in a block (how many at 0x00, the elements at 0x04, so many
bytes each), sorted by their first long, the one with a long; NULL if
none */
void *__attribute__((stdcall)) havok_unknown_001dd560(
	const byte *block,
	long key,
	long size)
{
	long low= 0;
	long high= *(const long *)block-1;
	byte *found= NULL;
	boolean last= FALSE;

	while (!last && low<=high)
	{
		long middle= (low+high)>>1;
		byte *element= *(byte *const *)(block+0x04)+middle*size;

		last= low==high;
		if (*(const long *)element==key)
		{
			found= element;
		}
		else if (*(const long *)element>key)
		{
			high= middle-1;
		}
		else
		{
			low= middle+1;
		}
		if (found)
		{
			break;
		}
	}

	return found;
}

/* 0x0054e898: the short of 0x001ee410 for none */
short unknown_0054e898;

/* 0x001ee410: a short of something: of an element (0x14 bytes, by its short
at 0x18) of the block at 0x5C of the definition of its tag (0x1C), or, with
no tag, of the structure bsp (at 0x10); unknown_0054e898 with neither */
void unknown_001ee410(
	short *result,
	const byte *data)
{
	long tag= *(const long *)(data+0x1C);
	short index= *(const short *)(data+0x18);

	if (tag!=NONE)
	{
		const byte *definition= (const byte *)(uintptr_t)global_tag_instances[tag&0xFFFF].address;

		*result= *(const short *)(*(const byte *const *)(definition+0x5C)+index*0x14+0x10);
		return;
	}
	if (index!=NONE)
	{
		*result= *(const short *)(*(const byte *const *)((const byte *)global_structure_bsp+0x10)+index*0x14+0x08);
		return;
	}
	*result= unknown_0054e898;
}

/* 0x001db120: three searches (as havok_unknown_001dd560) one inside the
other: by a long in the block at 0x34 of something, then by a second in the
block of what it found (0x14 bytes each), then by a third (0x34 bytes);
NULL if one fails */
void *__attribute__((stdcall)) havok_unknown_001db120(
	const byte *data,
	long first,
	long second,
	long third)
{
	const byte *found= havok_unknown_001dd560(data+0x34, first, 0x14);

	if (!found)
	{
		return NULL;
	}
	found= havok_unknown_001dd560(found+4, second, 0x14);
	if (!found)
	{
		return NULL;
	}

	return havok_unknown_001dd560(found+4, third, 0x34);
}

/* 0x001c4fc0: the things (at 0x44 of what each of the elements at 0x70,
0x60 bytes, as many as at 0x74, of a havok component has at 0x40) are
marked (bit 0 of 0xAC, cleared first when their time at 0xA8 was another)
with the time of the game */
void havok_component_unknown_001c4fc0(
	long havok_component_index)
{
	const byte *component;
	long index;

	if (havok_component_index==NONE)
	{
		return;
	}
	component= (const byte *)havok_component_data->data+(havok_component_index&0xFFFF)*0xA0;
	for (index= 0; index<*(const long *)(component+0x74); index++)
	{
		const byte *element= *(const byte *const *)(component+0x70)+index*0x60;
		byte *thing= *(byte *const *)(*(const byte *const *)(element+0x40)+0x44);

		if (thing)
		{
			if (*(const unsigned long *)(thing+0xA8)!=game_time_globals->time)
			{
				*(short *)(thing+0xAC)= 0;
			}
			thing[0xAC]|= 1;
			*(unsigned long *)(thing+0xA8)= game_time_globals->time;
		}
	}
}

/* 0x001cf950: something of 0xA0 bytes of Havok as it starts: a number at
0x08, a time 0.35 of a second ago at 0x10, NONE, zeros, and three points
whose third is -0 (0x70 to 0x94) */
void havok_unknown_001cf950(
	byte *data,
	long number)
{
	long index;

	*(long *)(data+0x08)= number;
	*(long *)(data+0x04)= 0;
	*(long *)(data+0x0C)= NONE;
	data[0x18]= 0xFF;
	data[0x19]= 0xFF;
	*(long *)(data+0x20)= NONE;
	*(real *)(data+0x14)= 0.0f;
	data[0x1A]= 0;
	data[0x1B]= 0;
	data[0x1C]= 0;
	*(long *)(data+0x10)= -lrintf((real)game_time_globals->ticks_per_second*0.35f);
	for (index= 0; index<3; index++)
	{
		*(unsigned long *)(data+0x70+index*12)= 0;
		*(unsigned long *)(data+0x74+index*12)= 0;
		*(unsigned long *)(data+0x78+index*12)= 0x80000000;
	}
	*(unsigned long *)(data+0x94)= 0;
	*(unsigned long *)(data+0x98)= 0;
	*(unsigned long *)(data+0x9C)= 0;
}

/* 0x001cc2f0: four names of something (0x07000101 for those of its owner,
at 0x70 to 0x7C; 0x07000001 and NONE for the defaults 0x06000086,
0x07000083, 0x030000D9 and 0x0400000C), the second from the block at 0x54
of the definition of the owner (0x68) by the third, unless it is
0x0400054B */
void __attribute__((stdcall)) havok_unknown_001cc2f0(
	unsigned long first,
	unsigned long second,
	unsigned long third,
	unsigned long *names,
	const byte *owner,
	unsigned long fourth)
{
	const byte *found;
	unsigned long name;

	names[0]= first;
	names[1]= second;
	names[2]= third;
	names[3]= fourth;
	if (first==0x07000101)
	{
		names[0]= *(const unsigned long *)(owner+0x70);
		if (names[0]==NONE)
		{
			names[0]= 0x06000086;
		}
	}
	if (names[0]==0x07000001)
	{
		names[0]= 0x06000086;
	}
	if (second==0x07000101)
	{
		names[1]= *(const unsigned long *)(owner+0x74);
	}
	if (names[1]==0x07000001 || names[1]==NONE)
	{
		names[1]= 0x07000083;
	}
	if (third==0x07000101)
	{
		names[2]= *(const unsigned long *)(owner+0x78);
	}
	if (names[2]==0x07000001 || names[2]==NONE)
	{
		names[2]= 0x030000D9;
	}
	name= names[1];
	found= havok_unknown_001dd560((const byte *)(uintptr_t)global_tag_instances[*(const long *)(owner+0x68)&0xFFFF].address+0x54, (long)names[2], 8);
	if (found && *(const unsigned long *)(found+4)!=NONE && *(const unsigned long *)(found+4)!=0x030000D9 && name!=0x0400054B)
	{
		name= *(const unsigned long *)(found+4);
	}
	names[1]= name;
	if (fourth==0x07000101)
	{
		names[3]= *(const unsigned long *)(owner+0x7C);
		if (names[3]==NONE)
		{
			names[3]= 0x0400000C;
		}
	}
	if (names[3]==0x07000001)
	{
		names[3]= 0x0400000C;
	}
}

/* 0x00479898: a third thing that gives memory */
byte *unknown_00479898;

/* 0x00146fd0: how busy Havok is, from 0 to 3: 3 when either of the two
things has a count at 0x20; for the third, by how full it is (its long at
0xF0340, by 1/118) and havok_unknown_0022cb90 below 0.65, 0.75 or 0.85 */
long havok_unknown_00146fd0(
	void)
{
	long first= 0;
	long second= 0;
	long third= 0;
	long result;

	if (unknown_004798a0 && *(const long *)((const byte *)unknown_004798a0+0x20)>0)
	{
		first= 3;
	}
	if (unknown_0047989c && *(const long *)((const byte *)unknown_0047989c+0x20)>0)
	{
		second= 3;
	}
	if (unknown_00479898)
	{
		third= 3;
		if (0.8f>(real)*(const long *)(unknown_00479898+0xF0340)*0.0084745763f)
		{
			real value= havok_unknown_0022cb90(unknown_00479898);

			if (0.85f>value)
			{
				third= 2;
				if (0.75f>value)
				{
					third= 1;
					if (0.65f>value)
					{
						third= 0;
					}
				}
			}
		}
	}
	result= third>first ? third : first;

	return second>result ? second : result;
}

/* 0x001ddb90: what havok_unknown_001db120 finds gets its animations (by
the two lists at 0x24 and 0x2C, of indices of the block at 0xB0 of
something, 0x14 bytes, those with one at 0x10) predicted: the first soon,
the second not, each when asked */
void __attribute__((stdcall)) havok_unknown_001ddb90(
	boolean first_list,
	const byte *data,
	long first,
	long second,
	long third,
	boolean second_list)
{
	const byte *found;
	long index;

	if (!first_list && !second_list)
	{
		return;
	}
	found= havok_unknown_001db120(data, first, second, third);
	if (!found)
	{
		return;
	}
	if (first_list)
	{
		for (index= 0; index<*(const long *)(found+0x24); index++)
		{
			struct cached_animation *animation= (struct cached_animation *)(*(byte *const *)(data+0xB0)+(*(const long *const *)(found+0x28))[index]*0x14);

			if (((const byte *)animation)[0x10])
			{
				animation_cache_predict(animation, TRUE);
			}
		}
	}
	if (second_list)
	{
		for (index= 0; index<*(const long *)(found+0x2C); index++)
		{
			struct cached_animation *animation= (struct cached_animation *)(*(byte *const *)(data+0xB0)+(*(const long *const *)(found+0x30))[index]*0x14);

			if (((const byte *)animation)[0x10])
			{
				animation_cache_predict(animation, FALSE);
			}
		}
	}
}

/* 0x001dd840: the animation (0x6C bytes, by an index, of the block at 0x30
of a graph), when it is streamed (bit 4 of its byte at 0x16), has its
resource (0x14 bytes, by its long at 0x24, of the block at 0xB0) asked for
soon, when it has one */
void havok_unknown_001dd840(
	long animation_index,
	const byte *graph)
{
	const byte *animation= animation_index==NONE ? NULL : *(const byte *const *)(graph+0x30)+animation_index*0x6C;
	struct cached_animation *resource;

	if (!(animation[0x16]&0x10))
	{
		return;
	}
	resource= (struct cached_animation *)(*(byte *const *)(graph+0xB0)+*(const long *)(animation+0x24)*0x14);
	if (((const byte *)resource)[0x10])
	{
		animation_cache_predict(resource, TRUE);
	}
}

/* 0x001ddab0: all the resources of a graph (0x14 bytes, as many as at 0xAC,
at 0xB0) that are streamed are asked for, not soon */
void havok_unknown_001ddab0(
	const byte *graph)
{
	long index;

	for (index= 0; index<*(const long *)(graph+0xAC); index++)
	{
		struct cached_animation *resource= (struct cached_animation *)(*(byte *const *)(graph+0xB0)+index*0x14);

		if (((const byte *)resource)[0x10])
		{
			animation_cache_predict(resource, FALSE);
		}
	}
}

boolean unit_counter_update(signed char *counter);

/* 0x001cd4e0: the two counters of an animation state (0x64, then 0x60) are
updated; the first is cleared when it reached its end; whether either says
so */
boolean havok_unknown_001cd4e0(
	byte *state)
{
	boolean result= unit_counter_update((signed char *)(state+0x64));

	if (state[0x67]&2)
	{
		state[0x65]= 0;
		state[0x64]= 0;
		state[0x67]= 0;
	}
	result|= unit_counter_update((signed char *)(state+0x60));

	return result;
}

/* 0x001ddc70: the animations of a mode of a graph are asked for (as
havok_unknown_001ddb90) for a weapon class and type, then with the default
name (0x030000D9) in place of each, then of all */
void __attribute__((stdcall)) havok_unknown_001ddc70(
	boolean first_list,
	const byte *graph,
	long mode,
	long weapon_class,
	long weapon_type,
	boolean second_list)
{
	if (!first_list && !second_list)
	{
		return;
	}
	havok_unknown_001ddb90(first_list, graph, mode, weapon_class, weapon_type, second_list);
	havok_unknown_001ddb90(first_list, graph, mode, weapon_class, 0x030000D9, second_list);
	havok_unknown_001ddb90(first_list, graph, mode, 0x030000D9, weapon_type, second_list);
	havok_unknown_001ddb90(first_list, graph, mode, 0x030000D9, 0x030000D9, second_list);
	havok_unknown_001ddb90(first_list, graph, 0x030000D9, 0x030000D9, 0x030000D9, second_list);
}

void __attribute__((stdcall)) unit_counter_start(signed char *counter, boolean up, real seconds);
real unit_counter_seconds(const signed char *counter);

/* 0x001cb5f0: when a blend of an animation state (its counter at 0x64) is
done or would end within some seconds, the orientations of its nodes (0x20
bytes each) are set to their targets and the blend starts again over the
seconds; whether it did */
boolean __attribute__((stdcall)) havok_unknown_001cb5f0(
	long node_count,
	byte *state,
	real seconds,
	void *orientations,
	const void *targets)
{
	signed char *counter;

	if (node_count<=0 || !(seconds>0.0f))
	{
		return FALSE;
	}
	counter= (signed char *)(state+0x64);
	if (counter[1] && !(counter[3]&2) && seconds<unit_counter_seconds(counter))
	{
		return FALSE;
	}
	counter[1]= 0;
	counter[0]= 0;
	counter[3]= 0;
	memcpy(orientations, targets, (unsigned long)node_count*0x20);
	unit_counter_start(counter, TRUE, seconds);

	return TRUE;
}

/* 0x001dcfa0: the next pair (8 bytes) of an iterator over the animations of
a mode of a graph (the iterator: the pair at 0x00, its index at 0x08, a step
at 0x0A, the mode at 0x0C and a weapon class at 0x10), trying the mode and
class, then the default name (0x030000D9) for each, in four steps; FALSE
when there are no more */
boolean havok_unknown_001dcfa0(
	byte *iterator,
	const byte *graph)
{
	long index= *(const short *)(iterator+0x08)+1;
	short *step= (short *)(iterator+0x0A);

	for (;;)
	{
		const byte *entry= NULL;

		while (*step<4 && !entry)
		{
			short current= *step;
			long mode= current<2 ? *(const long *)(iterator+0x0C) : 0x030000D9;
			long weapon_class= current==0 || current==2 ? *(const long *)(iterator+0x10) : 0x030000D9;
			const byte *mode_entry= havok_unknown_001dd560(graph+0x34, mode, 0x14);

			entry= mode_entry ? havok_unknown_001dd560(mode_entry+4, weapon_class, 0x14) : NULL;
			if (!entry)
			{
				*step= current+1;
				index= 0;
			}
		}
		if (!entry)
		{
			return FALSE;
		}
		if (index<*(const long *)(entry+0x0C))
		{
			const unsigned long *pair= (const unsigned long *)(*(const byte *const *)(entry+0x10)+index*8);

			*(unsigned long *)iterator= pair[0];
			*(unsigned long *)(iterator+4)= pair[1];
			*(short *)(iterator+0x08)= (short)index;
			return TRUE;
		}
		(*step)++;
	}
}

/* 0x001ddaf0: the resources of a graph and of each graph it inherits from
(by the tag at 0x04 of its entries, 0x20 bytes, as many as at 0x4C, at
0x50) are asked for */
void havok_unknown_001ddaf0(
	const byte *graph)
{
	long index;

	havok_unknown_001ddab0(graph);
	for (index= 0; index<*(const long *)(graph+0x4C); index++)
	{
		long tag= *(const long *)(*(const byte *const *)(graph+0x50)+index*0x20+0x04);
		const byte *inherited= NULL;

		if (tag!=NONE)
		{
			inherited= (const byte *)(uintptr_t)global_tag_instances[tag&0xFFFF].address;
		}
		havok_unknown_001ddab0(inherited);
	}
}

void object_unknown_000bba20(long object_index);

/* 0x001c4a20: the object of the havok component of a thing (its key
0x2001) wakes (object_unknown_000bba20) when the component has the bit 5 of
its flags (0x04) */
void havok_unknown_001c4a20(
	const byte *unknown)
{
	long count= *(const long *)(unknown+0x34);
	const long *pair= *(const long *const *)(unknown+0x30);
	long component_index= 0;
	long index;
	const byte *component;

	for (index= 0; index<count; index++)
	{
		if (pair[index*2]==0x2001)
		{
			component_index= pair[index*2+1];
			break;
		}
	}
	component= (const byte *)havok_component_data->data+(component_index&0xFFFF)*0xA0;
	if ((*(const unsigned long *)(component+0x04)>>5)&1)
	{
		object_unknown_000bba20(*(const long *)(component+0x08));
	}
}

/* 0x001ddd00: havok_unknown_001ddc70 for a graph and each graph it inherits
from (as havok_unknown_001ddaf0 finds them) */
void __attribute__((stdcall)) havok_unknown_001ddd00(
	const byte *graph,
	long mode,
	long weapon_class,
	long weapon_type,
	boolean first_list,
	boolean second_list)
{
	long index;

	if (!first_list && !second_list)
	{
		return;
	}
	havok_unknown_001ddc70(first_list, graph, mode, weapon_class, weapon_type, second_list);
	for (index= 0; index<*(const long *)(graph+0x4C); index++)
	{
		long tag= *(const long *)(*(const byte *const *)(graph+0x50)+index*0x20+0x04);
		const byte *inherited= NULL;

		if (tag!=NONE)
		{
			inherited= (const byte *)(uintptr_t)global_tag_instances[tag&0xFFFF].address;
		}
		havok_unknown_001ddc70(first_list, inherited, mode, weapon_class, weapon_type, second_list);
	}
}

/* 0x00510c20 and 0x00510c21, of the animation cache */
extern boolean animation_cache_initialized;
extern boolean animation_cache_opened;

/* 0x001dd9d0: with the animation cache open, an animation of a graph (its
index in the high short of an id, and in the low one the graph it inherits
it from, NONE for none) has its resource asked for, and so have those that
follow it (each to the next at 0x32, from its parent at 0x30 if it has
one) */
void __attribute__((stdcall)) havok_unknown_001dd9d0(
	const byte *graph,
	unsigned long id)
{
	short index= (short)(id>>16);
	short inherited= (short)id;
	const byte *animation;

	if (!animation_cache_initialized || !animation_cache_opened || index==NONE)
	{
		return;
	}
	if (inherited!=NONE)
	{
		graph= unknown4C_get_tag(inherited, graph);
	}
	animation= *(const byte *const *)(graph+0x30)+index*0x6C;
	if (*(const short *)(animation+0x30)!=NONE)
	{
		animation= *(const byte *const *)(graph+0x30)+*(const short *)(animation+0x30)*0x6C;
	}
	havok_unknown_001dd840(index, graph);
	while (*(const short *)(animation+0x32)!=NONE)
	{
		short next= *(const short *)(animation+0x32);

		havok_unknown_001dd840(next, graph);
		animation= *(const byte *const *)(graph+0x30)+next*0x6C;
	}
}

/* 0x001daea0: an animation of a graph (by an id as havok_unknown_001dd9d0
takes it, from the graph it inherits it from if it has one), with its
resource and those of those that follow it asked for; NULL for none */
byte *__attribute__((stdcall)) havok_unknown_001daea0(
	const byte *graph,
	unsigned long id)
{
	short index= (short)(id>>16);
	const byte *owner= graph;
	byte *animation;

	if (index==NONE)
	{
		return NULL;
	}
	if ((short)id!=NONE)
	{
		owner= unknown4C_get_tag((short)id, graph);
	}
	animation= *(byte *const *)(owner+0x30)+index*0x6C;
	if (animation)
	{
		havok_unknown_001dd9d0(graph, id);
	}

	return animation;
}

/* 0x001dd7c0: the data of an animation of a graph (by an id as
havok_unknown_001dd9d0 takes it): with the flag 0x10 at 0x16, that of its
resource of the animation cache (the resource at 0x24, of those at 0xB0 of
the graph, 0x14 bytes each), from the offset at 0x28; otherwise the pointer
at 0x38 */
byte *__attribute__((stdcall)) havok_unknown_001dd7c0(
	const byte *graph,
	unsigned long id)
{
	const byte *animation;

	if ((short)id!=NONE)
	{
		graph= unknown4C_get_tag((short)id, graph);
	}
	animation= (short)(id>>16)!=NONE ? *(const byte *const *)(graph+0x30)+(short)(id>>16)*0x6C : NULL;
	if (animation[0x16]&0x10)
	{
		struct cached_animation *resource= (struct cached_animation *)(*(byte *const *)(graph+0xB0)+*(const long *)(animation+0x24)*0x14);

		return (byte *)animation_cache_get_data(resource, *(const long *)animation)+*(const long *)(animation+0x28);
	}

	return *(byte *const *)(animation+0x38);
}

/* 0x001daf30: with the resources of an animation of a graph asked for
(havok_unknown_001dd9d0), its blend screen (the byte at 0x12, of those at
0x28 of the graph it is in, 0x1C bytes each), from 0x04; NULL for no
animation or no blend screen */
byte *__attribute__((stdcall)) havok_unknown_001daf30(
	const byte *graph,
	unsigned long id)
{
	short index= (short)(id>>16);
	const byte *animation;
	signed char blend_screen;

	if (index==NONE)
	{
		return NULL;
	}
	havok_unknown_001dd9d0(graph, id);
	if ((short)id!=NONE)
	{
		graph= unknown4C_get_tag((short)id, graph);
	}
	animation= *(const byte *const *)(graph+0x30)+index*0x6C;
	blend_screen= (signed char)animation[0x12];
	if (blend_screen==NONE)
	{
		return NULL;
	}

	return *(byte *const *)(graph+0x28)+blend_screen*0x1C+0x04;
}

/* 0x001c6440: the animation of a channel of animations (its graph a tag at
0x00, the id of the animation at 0x04), as havok_unknown_001daea0 gives
it; NULL for none */
byte *havok_unknown_001c6440(
	const byte *channel)
{
	unsigned long id= *(const unsigned long *)(channel+0x04);

	if ((short)(id>>16)==NONE)
	{
		return NULL;
	}

	return havok_unknown_001daea0(
		(const byte *)(uintptr_t)global_tag_instances[*(const long *)channel&0xFFFF].address, id);
}

/* 0x001c7300: whether a channel of animations has no animation (no graph
or no id), or one without the bit 0 at 0x18 */
boolean havok_unknown_001c7300(
	const byte *channel)
{
	if (*(const long *)channel==NONE || *(const short *)(channel+0x06)==NONE)
	{
		return TRUE;
	}

	return !(havok_unknown_001c6440(channel)[0x18]&1);
}

/* 0x001ddb40: what is wanted of an animation of a graph (by an id as
havok_unknown_001dd9d0 takes it): its data (havok_unknown_001dd7c0), where
its sizes are (0x3C), the byte at 0x13, the byte at 0x11 and the short at
0x14; nothing for no animation */
void __attribute__((stdcall)) havok_unknown_001ddb40(
	byte *data,
	const byte *graph,
	unsigned long id)
{
	const byte *animation;

	if ((short)(id>>16)==NONE)
	{
		return;
	}
	animation= havok_unknown_001daea0(graph, id);
	*(byte **)data= havok_unknown_001dd7c0(graph, id);
	*(const byte **)(data+0x04)= animation+0x3C;
	data[0x08]= animation[0x13];
	data[0x09]= animation[0x11];
	*(short *)(data+0x0A)= *(const short *)(animation+0x14);
}

/* 0x001c36f0: an object without a havok component (NONE at 0xB4) gets
one (unknown_001c35f0, between unknown_00146bf0 and unknown_00278f00), and
so do its children (from 0x10, each to the next at 0x0C), up to the one that
is the parent given */
void __attribute__((stdcall)) havok_unknown_001c36f0(
	long parent_index,
	long object_index)
{
	const byte *object= (const byte *)((const struct object_header_datum *)object_header_data->data)[object_index&0xFFFF].datum;
	long child_index= *(const long *)(object+0x10);

	if (*(const long *)(object+0xB4)==NONE)
	{
		unknown_00146bf0();
		unknown_001c35f0(object_index);
		unknown_00278f00();
		unknown_00146bf0();
	}
	while (child_index!=NONE)
	{
		const byte *child= (const byte *)((const struct object_header_datum *)object_header_data->data)[child_index&0xFFFF].datum;

		if (child_index==parent_index)
		{
			break;
		}
		havok_unknown_001c36f0(parent_index, child_index);
		child_index= *(const long *)(child+0x0C);
	}
}

/* 0x001dd0b0: the id (as havok_unknown_001dd9d0 takes it) of the animation
of a name (the long at 0x00 of each) in a graph or, after it, in those it
inherits from, in order; its resources are asked for. NONE for none */
void __attribute__((stdcall)) havok_unknown_001dd0b0(
	const byte *graph,
	unsigned long *id,
	long name)
{
	const byte *current= graph;
	long graph_index= NONE;

	if (!graph)
	{
		*id= NONE;
		return;
	}
	while (current)
	{
		long index;

		for (index= 0; index<*(const long *)(current+0x2C); index++)
		{
			if (*(const long *)(*(const byte *const *)(current+0x30)+index*0x6C)==name)
			{
				unsigned long found= (unsigned short)graph_index|(unsigned long)index<<16;

				havok_unknown_001dd9d0(graph, found);
				*id= found;
				return;
			}
		}
		graph_index++;
		if (graph_index>=*(const long *)(graph+0x4C))
		{
			break;
		}
		current= unknown4C_get_tag(graph_index, graph);
	}
	*id= NONE;
}

/* 0x0020aa70, of units.c */
void __attribute__((stdcall)) unit_unknown_0020aa70(unsigned long *last, unsigned long *point, const byte *data, short index);

/* 0x001c6dc0: how a channel of animations moves at a frame: by what is
wanted of its animation (havok_unknown_001ddb40), unit_unknown_0020aa70
gives a vector and a real; zeros for no animation */
void __attribute__((stdcall)) havok_unknown_001c6dc0(
	const byte *channel,
	real *vector,
	real *value,
	long frame)
{
	byte data[12];

	vector[0]= 0.0f;
	vector[1]= 0.0f;
	vector[2]= 0.0f;
	*value= 0.0f;
	if (*(const short *)(channel+0x06)==NONE)
	{
		return;
	}
	memset(data, 0, 10);
	havok_unknown_001ddb40(data,
		(const byte *)(uintptr_t)global_tag_instances[*(const long *)channel&0xFFFF].address,
		*(const unsigned long *)(channel+0x04));
	unit_unknown_0020aa70((unsigned long *)value, (unsigned long *)vector, data, (short)frame);
}

/* 0x004687a4, of real_math.c */
extern const real_vector3d *global_zero_vector3d;

/* 0x001c6d00: how fast a channel of animations moves: what
havok_unknown_001c6dc0 gives at its frame (the real at 0x1C, truncated),
times 30 times its rate (the real at 0x18); zeros for no animation */
void __attribute__((stdcall)) havok_unknown_001c6d00(
	const byte *channel,
	real *vector,
	real *value)
{
	real movement[3];
	real movement_value;
	long frame;
	real scale;

	*(real_vector3d *)vector= *global_zero_vector3d;
	*value= 0.0f;
	if (*(const short *)(channel+0x06)==NONE)
	{
		return;
	}
	frame= (long)*(const real *)(channel+0x1C);
	scale= *(const real *)(channel+0x18)*30.0f;
	havok_unknown_001c6dc0(channel, movement, &movement_value, frame);
	vector[0]= movement[0]*scale;
	vector[1]= movement[1]*scale;
	vector[2]= movement[2]*scale;
	*value= movement_value*scale;
}

/* 0x001cbd10: the id of the animation of a name in the graph of a state of
animations (a tag at 0x68), as havok_unknown_001dd0b0 finds it */
unsigned long *havok_unknown_001cbd10(
	const byte *state,
	unsigned long *id,
	long name)
{
	havok_unknown_001dd0b0(
		(const byte *)(uintptr_t)global_tag_instances[*(const long *)(state+0x68)&0xFFFF].address, id, name);

	return id;
}

/* 0x000d5910, of units.c */
boolean __attribute__((stdcall)) unit_unknown_000d5910(const byte *datum, boolean force);

/* 0x001edc10: the 2000 of unknown_0051e9cc are NONE, but those of the
objects of the type 6 in a cluster (0x04 of the header), without the flag
0x10 of the header, whose datum of the scenario (0x5C bytes each, at 0x54;
by the short at 0x1A of the object) is made now: the number at 0x58 of the
datum, less one, has the object */
void havok_unknown_001edc10(
	void)
{
	const struct data_array *data= object_header_data;
	long index;

	memset(unknown_0051e9cc, 0xFF, HAVOK_UNKNOWN_0051E9CC_COUNT*sizeof(long));
	for (index= 0; index<data->first_unallocated; index++)
	{
		const byte *header;
		long object_index;
		short scenario_index;
		const byte *datum;
		long mapping;

		if (!(data->in_use_bit_vector[index>>5]&(1UL<<(index&31))))
		{
			continue;
		}
		header= (const byte *)data->data+data->size*index;
		object_index= (long)(*(const short *)header<<16)|index;
		if (*(const short *)(header+0x04)==NONE || !((1UL<<(header[0x03]&31))&0x40) || (header[0x02]&0x10))
		{
			continue;
		}
		scenario_index= *(const short *)((const byte *)((const struct object_header_datum *)data->data)[index].datum+0x1A);
		if (scenario_index==NONE)
		{
			continue;
		}
		datum= *(const byte *const *)((const byte *)global_scenario+0x54)+scenario_index*0x5C;
		if (!unit_unknown_000d5910(datum, TRUE))
		{
			continue;
		}
		mapping= *(const short *)(datum+0x58)-1;
		if (mapping!=NONE)
		{
			unknown_0051e9cc[mapping]= object_index;
		}
	}
}

/* 0x001c69b0: the events of the animation of a channel (4 bytes each, at
0x50, as many as at 0x4C: a type and a frame in the short at 0x02) between
its frame (0x1C) and the end (the frames at 0x14 and one more, when its rate
at 0x18 is above zero, or 0) have the bits of their types in the short at
0x16; the frame itself counts when it is the end, not when it is the start */
void havok_unknown_001c69b0(
	byte *channel)
{
	const byte *animation;
	real end= 0.0f;
	real position;
	real lower;
	real upper;
	long index;

	*(unsigned short *)(channel+0x16)= 0;
	if (*(const short *)(channel+0x06)==NONE)
	{
		return;
	}
	animation= havok_unknown_001daea0(
		(const byte *)(uintptr_t)global_tag_instances[*(const long *)channel&0xFFFF].address,
		*(const unsigned long *)(channel+0x04));
	position= *(const real *)(channel+0x1C);
	if (*(const real *)(channel+0x18)>0.0f)
	{
		end= (real)*(const short *)(animation+0x14)+1.0f;
	}
	lower= position>end ? end : position;
	upper= position>end ? position : end;
	for (index= 0; index<*(const long *)(animation+0x4C); index++)
	{
		const byte *event= *(const byte *const *)(animation+0x50)+index*4;
		real frame= (real)*(const short *)(event+0x02);
		boolean set;

		if (lower==*(const real *)(channel+0x1C))
		{
			set= frame>lower && upper>=frame;
		}
		else
		{
			set= frame>=lower && upper>frame;
		}
		if (set)
		{
			*(unsigned short *)(channel+0x16)|= (unsigned short)(1UL<<(event[0]&31));
		}
	}
}

/* 0x000e0d70, of objects.c */
boolean __attribute__((stdcall)) object_unknown_000e0d70(long object_index, real *offset);

/* 0x001d43d0: the matrix of a havok component from that of its body: the
same, raised by the offset object_unknown_000e0d70 gives for a biped when
the component has the bit 9 at 0x04; for a component with a main body (the
byte at 0x18, of those at 0x74) and a node (the byte at 0x19), times the
inverse of that node of the render model of its object's model */
void __attribute__((stdcall)) havok_unknown_001d43d0(
	const byte *component,
	const real_matrix4x3 *matrix,
	real_matrix4x3 *result)
{
	signed char main_body= (signed char)component[0x18];
	long object_index= *(const long *)(component+0x08);

	*result= *matrix;
	if ((*(const unsigned long *)(component+0x04)>>9)&1)
	{
		const struct object_header_datum *header= (const struct object_header_datum *)object_header_data->data+(object_index&0xFFFF);
		real offset;

		if (((1UL<<(header->type&31))&1) && object_unknown_000e0d70(object_index, &offset))
		{
			result->position.z+= offset;
		}
		return;
	}
	if (main_body<0 || main_body>=*(const long *)(component+0x74) || component[0x19]==0xFF)
	{
		return;
	}
	{
		const byte *object= (const byte *)((const struct object_header_datum *)object_header_data->data)[object_index&0xFFFF].datum;
		const byte *definition= (const byte *)(uintptr_t)global_tag_instances[*(const long *)object&0xFFFF].address;
		const byte *model= (const byte *)(uintptr_t)global_tag_instances[*(const long *)(definition+0x38)&0xFFFF].address;
		const byte *render_model= (const byte *)(uintptr_t)global_tag_instances[*(const long *)(model+0x04)&0xFFFF].address;

		matrix4x3_multiply(result,
			(const real_matrix4x3 *)(*(const byte *const *)(render_model+0x4C)+(signed char)component[0x19]*0x60+0x28), result);
	}
}

/* 0x001c6560: a channel of animations goes to a frame (0x1C), kept between
0 and just under its animation's frames (0x14); one that plays once (the
bit 0 at 0x12, without the bits 9 at 0x11) has the bit 1 at 0x11 when the
next tick (its rate at 0x18, 30 frames a second) gets to the end. Then its
events (havok_unknown_001c69b0) */
void __attribute__((stdcall)) havok_unknown_001c6560(
	byte *channel,
	real frame)
{
	const byte *animation= NULL;
	real position;

	if (*(const short *)(channel+0x06)!=NONE)
	{
		animation= havok_unknown_001daea0(
			(const byte *)(uintptr_t)global_tag_instances[*(const long *)channel&0xFFFF].address,
			*(const unsigned long *)(channel+0x04));
	}
	position= 0.0f>frame ? 0.0f
		: frame>(real)*(const short *)(animation+0x14)-0.0001f ? (real)*(const short *)(animation+0x14)-0.0001f : frame;
	*(real *)(channel+0x1C)= position;
	if ((channel[0x12]&1) && !(channel[0x11]&9))
	{
		if (game_time_globals->seconds_per_tick**(const real *)(channel+0x18)*30.0f + position
			>=(real)*(const short *)(animation+0x14)-0.0001f)
		{
			channel[0x11]|= 2;
		}
		else
		{
			channel[0x11]&= ~2;
		}
	}
	havok_unknown_001c69b0(channel);
}

/* 0x001ce180: the resources of the animations of a mode, a weapon class
and a weapon type of a state of animations (its graph a tag at 0x68) are
asked for (havok_unknown_001ddd00), by the names havok_unknown_001cc2f0
finds for them */
void __attribute__((stdcall)) havok_unknown_001ce180(
	const byte *state,
	unsigned long mode,
	unsigned long weapon_class,
	unsigned long weapon_type,
	boolean urgent,
	boolean other)
{
	unsigned long names[4];

	if (*(const long *)(state+0x68)==NONE)
	{
		return;
	}
	havok_unknown_001cc2f0(mode, weapon_class, weapon_type, names, state, 0x07000101);
	havok_unknown_001ddd00((const byte *)(uintptr_t)global_tag_instances[*(const long *)(state+0x68)&0xFFFF].address,
		names[0], names[1], names[2], urgent, other);
}
