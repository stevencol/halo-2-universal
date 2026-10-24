/*
HAVOK.C

the part of the game that the physics are (Havok, the library): it starts and
stops the others of this folder. Worked out from halo2ship.exe; the name of
the file is a guess, by what its functions start.
*/

#include "cseries/cseries.h"

#include <stdint.h>
#include <string.h>

#include "cache/cache_files.h"
#include "cache/texture_cache.h"
#include "game/game.h"
#include "memory/data.h"
#include "objects/objects.h"
#include "physics/havok_component.h"
#include "saved games/game_state.h"

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
