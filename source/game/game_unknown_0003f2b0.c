/*
GAME_UNKNOWN_0003F2B0.C

a part of the game: a number that is zero when a structure bsp comes.
Worked out from halo2ship.exe; what it is for is not known, and the name of
the file is only the address of its function.
*/

#include "cseries/cseries.h"
#include "scenario/scenario.h"
#include "cache/cache_files.h"
#include "cache/geometry_cache.h"
#include "objects/objects.h"
#include "math/real_math.h"
#include "physics/bsp3d.h"
#include <stdint.h>

/* ---------- globals */

/* 0x00509438 */
long unknown_00509438;

/* ---------- code */

/* 0x0003f2b0: the fifth of the functions of the part (game.c): for a new
structure bsp the number is zero */
void game_unknown_0003f2b0_initialize_for_new_structure_bsp(
	void)
{
	unknown_00509438= 0;
}

/* 0x00040e10: a number from two bytes: 0 if both are set, 1 if only the
first, 2 if only the second and 3 if none. It is after this file in the
game */
long unknown_00040e10(
	boolean first,
	boolean second)
{
	if (first)
	{
		return !second;
	}

	return second ? 2 : 3;
}

/* 0x00040de0: 0 if there is nothing, 1 if the first byte is set, 2 if only
the second, and NONE if none. It is before unknown_00040e10 in the game */
long __attribute__((stdcall)) unknown_00040de0(
	const void *something,
	boolean first,
	boolean second)
{
	if (something==NULL)
	{
		return 0;
	}
	if (first)
	{
		return 1;
	}

	return second ? 2 : NONE;
}

/* 0x004b9ee9 and 0x004b9eec: a byte that says yes, and an index of the list
of shorts at 0xB0 of the structure bsp */
byte unknown_004b9ee9;
long unknown_004b9eec;

/* 0x0003eb20: whether the short of the structure bsp that unknown_004b9eec
names is this number; FALSE if any of them is not there. It is before
game_unknown_0003f2b0_initialize_for_new_structure_bsp in the game; whether it
is of this file is not known */
boolean unknown_004b9eec_matches(
	long value)
{
	const byte *structure_bsp= global_structure_bsp;

	if (value!=NONE && unknown_004b9ee9 && unknown_004b9eec!=NONE && global_structure_bsp_index!=NONE &&
		*(const long *)(structure_bsp+0xAC)>0 &&
		((const short *)(uintptr_t)*(const unsigned long *)(structure_bsp+0xB0))[unknown_004b9eec]==value)
	{
		return TRUE;
	}

	return FALSE;
}

/* 0x00547f88: the first of the two things of game_unknown_00131ff0.c; its
first long points to what 0x0003f3f0 reads */
extern byte unknown_00547f88[];

/* 0x004c56c0: 512 bits, cleared by 0x0002b5d0 */
unsigned long unknown_004c56c0[16];

/* 0x0003f3f0: for each of the entries (0x1A bytes) at 0xA6E of what the
first thing points to (as many as its short at 0xA6C), the bit named by the
entry's first short is set in unknown_004c56c0 */
void unknown_0003f3f0(
	void)
{
	const byte *data= *(const byte *const *)unknown_00547f88;
	long index;

	for (index= 0; index<*(const short *)(data+0xA6C); index++)
	{
		short bit= *(const short *)(data+index*0x1A+0xA6E);

		unknown_004c56c0[bit>>5]|= 1UL<<(bit&0x1F);
	}
}

/* 0x0003fd20: whether a sphere is in front of any of six planes by at least
its radius times the square root of three (the half diagonal of a cube) */
boolean unknown_0003fd20(
	const real_plane3d *planes,
	const real_point3d *center,
	real radius)
{
	real distance= radius*1.7320508f;
	long index;

	for (index= 0; index<6; index++)
	{
		const real_plane3d *plane= planes+index;

		if (plane->n.i*center->x + plane->n.k*center->z + plane->n.j*center->y - plane->d>=distance)
		{
			return TRUE;
		}
	}

	return FALSE;
}

/* 0x004c6700: eight of something of 0x408 bytes (a long at 0x400 and a
short at 0x404). 0x004c8740: sixteen numbers. 0x0050943c and 0x00509440:
two more */
byte unknown_004c6700[8*0x408];
long unknown_004c8740[16];
long unknown_0050943c;
long unknown_00509440;

/* 0x0003fd70: the sixteen are NONE, and the eight have NONE and 0 */
void unknown_0003fd70(
	void)
{
	long index;

	for (index= 0; index<16; index++)
	{
		unknown_004c8740[index]= NONE;
	}
	unknown_0050943c= 0;
	for (index= 0; index<8; index++)
	{
		*(long *)(unknown_004c6700+index*0x408+0x400)= NONE;
		*(short *)(unknown_004c6700+index*0x408+0x404)= 0;
	}
	unknown_00509440= 0;
}

/* 0x0002b720: the leaf of the structure bsp (in its bsp3d at 0x18) where a
point is, and its cluster (the short of the leaf, 8 bytes, at 0x30); when
there is none, a leaf and cluster kept from before stay if they are good
(below the counts at 0x2C and 0x9C), and are NONE otherwise. Whether there
was a leaf */
boolean __attribute__((stdcall)) unknown_0002b720(
	const real_point3d *point,
	long *leaf_index,
	long *cluster_index)
{
	const byte *bsp= (const byte *)global_structure_bsp;
	long index= bsp3d_test_point(*(const struct bsp3d *const *)(bsp+0x18), 0, point);

	if (index!=NONE)
	{
		*leaf_index= index;
		*cluster_index= *(const short *)(*(const byte *const *)(bsp+0x30)+index*8);
		return TRUE;
	}
	if (*leaf_index<0 || *leaf_index>=*(const long *)(bsp+0x2C) || *cluster_index<0 || *cluster_index>=*(const long *)(bsp+0x9C))
	{
		*leaf_index= NONE;
		*cluster_index= NONE;
	}

	return FALSE;
}

/* 0x0002d000: how the geometry of something is (an instance 0x7FF is
none): with a lightmap of the structure bsp (the first of its count at
0x80, at 0x84, with a bitmap at 0x1C and the same tag at 0x04 as at 0x08 of
the bsp), for a tag 1 when the geometry of its section (by the instance, at
0x5C and 0x64) is loaded, for no tag whether that of the instance (at 0x54)
is and the bsp's instance has a short at 0x56 (0 or 1); 2 for an instance
0x7FF or, without a lightmap, an object with the bit 0 at 0x07; 3 otherwise */
long unknown_0002d000(
	long object_index,
	long tag_index,
	long instance)
{
	const byte *lightmaps= global_structure_lightmap;
	const byte *bsp= global_structure_bsp;
	const byte *lightmap;

	if (!lightmaps || *(const long *)(lightmaps+0x80)<=0 || !bsp
		|| *(const long *)((lightmap= *(const byte *const *)(lightmaps+0x84))+0x1C)==NONE
		|| *(const long *)(lightmap+0x04)!=*(const long *)(bsp+0x08))
	{
		const byte *object;

		if (tag_index!=NONE && object_index!=NONE
			&& (object= object_try_and_get_and_verify_type(object_index, NONE)) && (object[0x07]&1))
		{
			return 2;
		}
		return 3;
	}
	if (tag_index!=NONE)
	{
		const byte *data;

		if (instance==0x7FF)
		{
			return 2;
		}
		data= (const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address;
		if (*(const long *)(data+0x08)!=*(const long *)(*(const byte *const *)(lightmap+0x5C)+instance*12+8))
		{
			return 3;
		}
		if (!geometry_cache_block_load((struct geometry_block_info *)(*(byte *const *)(lightmap+0x44)
			+*(const unsigned short *)(*(const byte *const *)(lightmap+0x64)+instance*12+2)*0x38+0x0C), 3))
		{
			return 3;
		}
		return 1;
	}
	if (instance==0x7FF)
	{
		return 0;
	}
	if (!geometry_cache_block_load((struct geometry_block_info *)(*(byte *const *)(lightmap+0x44)
		+*(const unsigned short *)(*(const byte *const *)(lightmap+0x54)+instance*12+2)*0x38+0x0C), 3))
	{
		return 3;
	}

	return *(const short *)(*(const byte *const *)(bsp+0x144)+instance*0x58+0x56)!=0;
}
