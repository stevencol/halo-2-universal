/*
STRUCTURE_LIGHTMAP_PREDICT.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

Asking the caches for what will be drawn soon (the bitmaps and the
geometry of the lightmap of the structure bsp, and the geometry of a part
of a model), so that it is loaded before it is needed. The name of the
file is a guess, and so are the names of its functions.
*/

#include "cseries/cseries.h"

#include "cache/cache_files.h"
#include "cache/geometry_cache.h"
#include "cache/texture_cache.h"
#include "scenario/scenario.h"

/* ---------- constants */

enum
{
	/* of the lightmap: its groups (a count and an address) */
	STRUCTURE_LIGHTMAP_GROUP_COUNT_OFFSET= 0x80,
	STRUCTURE_LIGHTMAP_GROUPS_OFFSET= 0x84,

	/* of its first group: a number that has to be the one of the structure
	bsp (at 8), and the tag of its bitmaps */
	STRUCTURE_LIGHTMAP_GROUP_CHECKSUM_OFFSET= 0x4,
	STRUCTURE_LIGHTMAP_GROUP_BITMAP_TAG_OFFSET= 0x1C,
	STRUCTURE_LIGHTMAP_GROUP_GEOMETRY_OFFSET= 0x44,
	STRUCTURE_BSP_CHECKSUM_OFFSET= 0x8,

	/* in the bitmaps: the address of the block of bitmaps, and the size of
	one */
	BITMAP_GROUP_BITMAPS_OFFSET= 0x48,
	BITMAP_DATA_SIZE= 0x74,

	/* in the geometry of the lightmap: what the cache knows of each block */
	STRUCTURE_LIGHTMAP_GEOMETRY_SIZE= 0x38,
	STRUCTURE_LIGHTMAP_GEOMETRY_INFO_OFFSET= 0xC
};

/* ---------- private code */

/* the first group of the lightmap, if there is a lightmap with groups and
a structure bsp that it is for; NULL if not */
static inline __attribute__((always_inline)) const byte *structure_lightmap_first_group(
	void)
{
	const byte *lightmap= global_structure_lightmap;
	const byte *group;

	if (!lightmap || *(const long *)(lightmap+STRUCTURE_LIGHTMAP_GROUP_COUNT_OFFSET)<=0 || !global_structure_bsp)
	{
		return NULL;
	}
	group= *(const byte *const *)(lightmap+STRUCTURE_LIGHTMAP_GROUPS_OFFSET);
	if (*(const long *)(group+STRUCTURE_LIGHTMAP_GROUP_BITMAP_TAG_OFFSET)==NONE ||
		*(const long *)(group+STRUCTURE_LIGHTMAP_GROUP_CHECKSUM_OFFSET)!=
			*(const long *)((const byte *)global_structure_bsp+STRUCTURE_BSP_CHECKSUM_OFFSET))
	{
		return NULL;
	}

	return group;
}

/* the bitmap of the place of a block of shorts (each of four bytes) of the
first group of the lightmap, if it has one */
static inline __attribute__((always_inline)) boolean structure_lightmap_predict_bitmap(
	long block_offset,
	long index)
{
	const byte *group= structure_lightmap_first_group();
	long bitmap_index;
	const byte *bitmaps;

	if (!group)
	{
		return TRUE;
	}
	bitmap_index= *(const short *)(*(const byte *const *)(group+block_offset)+index*4);
	if (bitmap_index==NONE)
	{
		return TRUE;
	}
	bitmaps= (const byte *)(uintptr_t)global_tag_instances[*(const long *)(group+STRUCTURE_LIGHTMAP_GROUP_BITMAP_TAG_OFFSET)&0xFFFF].address;

	return texture_cache_bitmap_predict((struct bitmap_data *)(*(byte *const *)(bitmaps+BITMAP_GROUP_BITMAPS_OFFSET)+
		bitmap_index*BITMAP_DATA_SIZE));
}

/* the geometry of the place of a block (each of twelve bytes, with the
index at 2) of the first group of the lightmap */
static inline __attribute__((always_inline)) boolean structure_lightmap_predict_geometry(
	long block_offset,
	long index)
{
	const byte *group= structure_lightmap_first_group();
	unsigned short geometry_index;

	if (!group)
	{
		return TRUE;
	}
	geometry_index= *(const unsigned short *)(*(const byte *const *)(group+block_offset)+index*12+2);

	return geometry_cache_block_predict((struct geometry_block_info *)(*(byte *const *)(group+STRUCTURE_LIGHTMAP_GROUP_GEOMETRY_OFFSET)+
		geometry_index*STRUCTURE_LIGHTMAP_GEOMETRY_SIZE+STRUCTURE_LIGHTMAP_GEOMETRY_INFO_OFFSET));
}

/* ---------- code */

/* 0x0016e770: the geometry of a part of a model (render model): the one
that the part (0x58 bytes each, at 0x144) says at 0x34, of those of 0xC8
bytes at 0x13C */
boolean render_model_predict_geometry(
	long model_index,
	long part_index)
{
	const byte *model= (const byte *)(uintptr_t)global_tag_instances[model_index&0xFFFF].address;
	short section_index= *(const short *)(*(const byte *const *)(model+0x144)+part_index*0x58+0x34);

	return geometry_cache_block_predict((struct geometry_block_info *)(*(byte *const *)(model+0x13C)+section_index*0xC8+0x28));
}

/* 0x0016e7b0: the bitmap of a place of the block at 0x2C of the first group
of the lightmap; TRUE if there is nothing to ask for */
boolean __attribute__((stdcall)) structure_lightmap_predict_bitmap_2c(
	long index)
{
	return structure_lightmap_predict_bitmap(0x2C, index);
}

/* 0x0016e820: the same with the block at 0x4C */
boolean __attribute__((stdcall)) structure_lightmap_predict_bitmap_4c(
	long index)
{
	return structure_lightmap_predict_bitmap(0x4C, index);
}

/* 0x0016e890: the geometry of a place of the block at 0x64 of the first
group of the lightmap; TRUE if there is nothing to ask for */
boolean structure_lightmap_predict_geometry_64(
	long index)
{
	return structure_lightmap_predict_geometry(0x64, index);
}

/* 0x0016e8f0: the same with the block at 0x54 */
boolean structure_lightmap_predict_geometry_54(
	long index)
{
	return structure_lightmap_predict_geometry(0x54, index);
}
