/*
GAME_UNKNOWN_0002B4A0.C

a part of the game: it has a table of data ("cached object render states"),
and its first function sets the time of the game in seconds. It comes after
the rasterizer in the table of the parts, so it may be what draws the world.
Worked out from halo2ship.exe; what it is for is not known, and the name of
the file is only the address of its first function.
*/

#include "cseries/cseries.h"
#include "memory/data.h"
#include "saved games/game_state.h"
#include "bitmaps/bitmap_utilities.h"
#include "objects/objects.h"
#include "cache/cache_files.h"
#include "scenario/scenario.h"

#include <stdint.h>
#include <string.h>

/* ---------- globals */

/* 0x00509434: its table of data ("cached object render states": 0x100 of
0x100 bytes). 0x004c1bd0: a number that is NONE when the table is made */
struct data_array *cached_object_render_state_data;
long unknown_004c1bd0;

/* 0x004ba040, of game.c: the time of the game, in seconds */
extern double game_seconds;

/* what the part sets when it starts: three numbers that are zero
(0x004ba034, 0x004ba038, 0x004ba03c) and two lists of four that are NONE
(0x004ba054, 0x004ba064); and 0x0050942c: 0x18 bytes of the state of the
game */
long unknown_004ba034;
long unknown_004ba038;
long unknown_004ba03c;
long unknown_004ba054[4];
long unknown_004ba064[4];
byte *unknown_0050942c;

/* what it sets for a new map: a number that is zero (0x004ba134), 0x30 bytes
that are zeros but for the first number, which is NONE (0x004b9970), 0x6A8
bytes of zeros (0x004c5018) and a byte that is zero (0x005093fc) */
long unknown_004ba134;
byte unknown_004b9970[0x30];
byte unknown_004c5018[0x6A8];
byte unknown_005093fc;

void unknown_0003b8a0(void);
void unknown_000167a0(void);

/* 0x004c1a5c, 0x004c1a74, 0x004c1a8c: three memories of the console, 0x18
bytes from one to the next */
void *unknown_004c1a5c;
void *unknown_004c1a74;
void *unknown_004c1a8c;

int VirtualFree(void *address, unsigned long size, unsigned long type);
uint32_t GetLastError(void);

void game_unknown_0002b4a0_unknown_0003b950(void);

/* ---------- code */

/* 0x0002b5c0: the part is closed: game_unknown_0002b4a0_unknown_0003b950, and it has no table */
void game_unknown_0002b4a0_dispose(
	void)
{
	game_unknown_0002b4a0_unknown_0003b950();
	cached_object_render_state_data= NULL;
}

/* 0x0002b5a0: when a map goes, the table cannot be used, if it is there and
could */
void game_unknown_0002b4a0_dispose_from_old_map(
	void)
{
	struct data_array *data= cached_object_render_state_data;

	if (data && data->valid)
	{
		data->valid= FALSE;
	}
}

/* 0x0003b950: each of the three memories of the console that is there is
given back (VirtualFree; if that fails, the error is asked for), and there
is none */
void game_unknown_0002b4a0_unknown_0003b950(
	void)
{
	if (unknown_004c1a5c)
	{
		if (!VirtualFree(unknown_004c1a5c, 0, 0x8000))
		{
			GetLastError();
		}
		unknown_004c1a5c= NULL;
	}
	if (unknown_004c1a74)
	{
		if (!VirtualFree(unknown_004c1a74, 0, 0x8000))
		{
			GetLastError();
		}
		unknown_004c1a74= NULL;
	}
	if (unknown_004c1a8c)
	{
		if (!VirtualFree(unknown_004c1a8c, 0, 0x8000))
		{
			GetLastError();
		}
		unknown_004c1a8c= NULL;
	}
}

/* 0x0003d270: the table is made, in the memory of the state of the game
(data_new, which the game has written here). In the game it is far from the
others of this file, so it may be of another */
void cached_object_render_states_initialize(
	void)
{
	cached_object_render_state_data= data_new("cached object render states", 0x100, 0x100, 0, game_state_allocation);
	unknown_004c1bd0= NONE;
}

/* 0x0002b4a0: the first of the functions of the part (game.c): the time of
the game is zero, its numbers are as they start, unknown_0003b8a0, the table
and unknown_000167a0 start, and the 0x18 bytes take their memory in the state
of the game (game_state_malloc, which the game has written here) */
void game_unknown_0002b4a0_initialize(
	void)
{
	long index;

	unknown_004ba034= 0;
	game_seconds= 0.0;
	unknown_004ba038= 0;
	unknown_004ba03c= 0;
	for (index= 0; index<4; index++)
	{
		unknown_004ba054[index]= NONE;
		unknown_004ba064[index]= NONE;
	}
	unknown_0003b8a0();
	cached_object_render_states_initialize();
	unknown_000167a0();
	unknown_0050942c= game_state_malloc(0x18);
}

/* 0x0002b540: the third: for a new map the time of the game is zero, the
table is good to use, with nothing in it, and the rest is zeros */
void game_unknown_0002b4a0_initialize_for_new_map(
	void)
{
	game_seconds= 0.0;
	unknown_004ba134= 0;
	data_make_valid(cached_object_render_state_data);
	memset(unknown_004b9970, 0, sizeof(unknown_004b9970));
	memset(unknown_004c5018, 0, sizeof(unknown_004c5018));
	*(long *)unknown_004b9970= NONE;
	unknown_005093fc= 0;
}

/* 0x000255e0: the table unknown_004b9970 is emptied, as for a new map,
without the rest */
void unknown_004b9970_reset(
	void)
{
	memset(unknown_004b9970, 0, sizeof(unknown_004b9970));
	*(long *)unknown_004b9970= NONE;
	unknown_005093fc= 0;
}

/* 0x0003ba90: whether something lacks a bit in its number at 8 */
long __attribute__((stdcall)) unknown8_lacks_bit(
	long bit,
	const byte *unknown)
{
	return !((1u<<(bit&31))&*(const unsigned long *)(unknown+8));
}

/* 0x004670e4, 0x004ba01c .. 0x004ba02c: some settings that a kind sets */
float unknown_004670e4= 0.85f;
long unknown_004ba01c;
byte unknown_004ba020;
byte unknown_004ba021;
byte unknown_004ba022;
byte unknown_004ba023;
byte unknown_004ba024;
byte unknown_004ba025;
float unknown_004ba028;
float unknown_004ba02c;

/* 0x0002c490: the settings of a kind: 2 and 3, 4, or the others */
void game_unknown_0002b4a0_set_kind(
	long kind)
{
	byte others;

	if (kind==2 || kind==3)
	{
		unknown_004670e4= 0.3f;
		unknown_004ba02c= 0.5f;
		unknown_004ba01c= 3;
		unknown_004ba028= 0.8f;
		others= FALSE;
		unknown_004ba021= TRUE;
	}
	else if (kind==4)
	{
		unknown_004670e4= 0.25f;
		unknown_004ba02c= 0.25f;
		unknown_004ba01c= 2;
		unknown_004ba028= 0.7f;
		others= FALSE;
		unknown_004ba021= TRUE;
	}
	else
	{
		unknown_004670e4= 0.85f;
		others= TRUE;
		unknown_004ba021= FALSE;
		unknown_004ba02c= 1.0f;
		unknown_004ba01c= 4;
		unknown_004ba028= 1.0f;
	}
	unknown_004ba020= others;
	unknown_004ba022= others;
	unknown_004ba024= others;
	unknown_004ba023= others;
	unknown_004ba025= others;
}

/* 0x004c0b80: something with a long at 0x2A78. 0x004c0b78 and 0x004c0b7c:
two numbers. 0x004c1528 to 0x004c19ac: a number, 0x480 bytes and a number */
byte *unknown_004c0b80;
long unknown_004c0b78;
long unknown_004c0b7c;
long unknown_004c1528;
byte unknown_004c152c[0x480];
long unknown_004c19ac;

/* 0x0002cb30: when that long is zero, the numbers and the 0x480 bytes are
zeros (the game copies nine zeros there and then them along it); otherwise
the second number is the first */
void unknown_0002cb30(
	void)
{
	if (*(const long *)(unknown_004c0b80+0x2A78))
	{
		unknown_004c0b78= unknown_004c0b7c;
		return;
	}
	unknown_004c0b7c= 0;
	unknown_004c0b78= 0;
	unknown_004c1528= 0;
	memset(unknown_004c152c, 0, sizeof(unknown_004c152c));
	unknown_004c19ac= 0;
}

/* 0x004ba138: sixteen of something of 0x20 bytes */
byte unknown_004ba138[16*0x20];

/* 0x00044940: a place of one of those: by its tag (0x04: in the block at
0x28 of its definition, 0x5C bytes) or, without, in the structure bsp (in
the block at 0xA0, 0xB0 bytes, or by a flag in that at 0x13C, 0xC8 bytes, by
a short of that at 0x144), all by the bits of its long at 0x0C */
byte *__attribute__((stdcall)) unknown_00044940(
	short index,
	boolean other)
{
	const byte *entry= unknown_004ba138+index*0x20;
	unsigned long bits= *(const unsigned long *)(entry+0x0C);
	long tag= *(const long *)(entry+0x04);
	const byte *bsp= (const byte *)global_structure_bsp;

	if (tag!=NONE)
	{
		const byte *definition= (const byte *)(uintptr_t)global_tag_instances[tag&0xFFFF].address;

		return *(byte *const *)(definition+0x28)+((bits>>9)&0x1FF)*0x5C+4;
	}
	if (!other)
	{
		return *(byte *const *)(bsp+0xA0)+((bits>>9)&0x1FF)*0xB0;
	}

	return *(byte *const *)(bsp+0x13C)+*(const short *)(*(const byte *const *)(bsp+0x144)+((bits>>18)&0x7FF)*0x58+0x34)*0xC8;
}

/* 0x000460d0: whether one of those sixteen (by the bit 0x40 of its flags,
TRUE without it) has a bit set among bits for its place (as
unknown_00044940 finds it: the list at 0x34 or 0x50 there, 0x48 bytes by
a number, its first at 0x0A and how many at 0x0C) */
boolean __attribute__((stdcall)) unknown_000460d0(
	long number,
	const unsigned long *bits,
	short index)
{
	const byte *entry= unknown_004ba138+index*0x20;
	unsigned long flags= *(const unsigned long *)entry;
	unsigned long place= *(const unsigned long *)(entry+0x0C);
	long tag= *(const long *)(entry+0x04);
	const byte *bsp= (const byte *)global_structure_bsp;
	const byte *list;
	const byte *element;
	long first;
	long last;
	boolean result= FALSE;

	if (!(flags&0x40))
	{
		return TRUE;
	}
	if (tag!=NONE)
	{
		const byte *definition= (const byte *)(uintptr_t)global_tag_instances[tag&0xFFFF].address;

		list= *(const byte *const *)(*(const byte *const *)(definition+0x28)+((place>>9)&0x1FF)*0x5C+0x34);
	}
	else if (!((flags>>12)&1))
	{
		list= *(const byte *const *)(*(const byte *const *)(bsp+0xA0)+((place>>9)&0x1FF)*0xB0+0x50);
	}
	else
	{
		short cluster= *(const short *)(*(const byte *const *)(bsp+0x144)+((place>>18)&0x7FF)*0x58+0x34);

		list= *(const byte *const *)(*(const byte *const *)(bsp+0x13C)+cluster*0xC8+0x50);
	}
	element= *(const byte *const *)(list+0x04)+number*0x48;
	first= *(const short *)(element+0x0A);
	last= *(const short *)(element+0x0C)+first;
	for (; first<last && !result; first++)
	{
		result= (bits[first>>5]&(1UL<<(first&0x1F)))!=0;
	}

	return result;
}

boolean object_unknown_000bad50(long object_index, long index, unsigned long *result);

/* 0x0003dd10: the cached render state of an object (its 0xCC; when asked,
or when its definition has the bit 0 at 0x1C) keeps the colors the object
has (as object_unknown_000bad50 gives them, up to four) from 0xA8, and how
many at 0xB8 */
void __attribute__((stdcall)) cached_object_render_state_unknown_0003dd10(
	long object_index,
	boolean always)
{
	const byte *object= (const byte *)((const struct object_header_datum *)object_header_data->data)[object_index&0xFFFF].datum;
	long state_index= *(const long *)(object+0xCC);
	byte *state;
	long index;

	if (state_index==NONE)
	{
		return;
	}
	if (!always && !(((const byte *)(uintptr_t)global_tag_instances[*(const long *)object&0xFFFF].address)[0x1C]&1))
	{
		return;
	}
	state= (byte *)cached_object_render_state_data->data+((state_index&0xFFFF)<<8);
	state[0xB8]= 0;
	for (index= 0; index<4; index++)
	{
		real_rgb_color color;

		if (object_unknown_000bad50(object_index, index, (unsigned long *)&color))
		{
			*(pixel32 *)(state+0xA8+index*4)= real_rgb_color_to_pixel32(&color);
			state[0xB8]++;
		}
	}
}
