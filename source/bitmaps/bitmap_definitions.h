/*
BITMAP_DEFINITIONS.H

The bitmap group (bitm), a tag of one or more textures. The pixels are in
blocks of the cache file apart from the tag.
*/

#ifndef __BITMAP_DEFINITIONS_H
#define __BITMAP_DEFINITIONS_H

#include "tag_files/tag_groups.h"

#include <stddef.h>

/* ---------- constants */

#define BITMAP_GROUP_TAG 'bitm'
#define BITMAP_SIGNATURE 'bitm'

enum
{
	MAXIMUM_BITMAP_PIXELS_BLOCKS= 6,
	/* the console's game reads three, and keeps things of its own where the
	other three are */
	NUMBER_OF_BITMAP_CACHE_LEVELS= 3
};

enum
{
	/* the only type that the shaders of the structure bsps use */
	_bitmap_type_2d,
	_bitmap_type_3d,
	_bitmap_type_cube_map
};

enum
{
	/* the formats that the tools read. The diffuse maps of the shaders of
	the levels and of their objects have y8, ay8, r5g6b5, a4r4g4b4, x8r8g8b8
	and the three dxt formats */
	_bitmap_format_a8= 0,
	_bitmap_format_y8= 1,
	/* one value for the alpha and for the luminance */
	_bitmap_format_ay8= 2,
	_bitmap_format_a8y8= 3,
	_bitmap_format_r5g6b5= 6,
	_bitmap_format_a4r4g4b4= 9,
	_bitmap_format_x8r8g8b8= 10,
	_bitmap_format_a8r8g8b8= 11,
	_bitmap_format_dxt1= 14,
	_bitmap_format_dxt3= 15,
	_bitmap_format_dxt5= 16
};

enum
{
	/* in blocks of 4 by 4 pixels: its sizes are taken up to whole blocks */
	_bitmap_compressed_bit= 1,
	/* the texels are in the order of the console's swizzled textures; the
	bitmaps that are not compressed have it */
	_bitmap_swizzled_bit= 3,
	/* of sizes that are not powers of two: no mipmaps */
	_bitmap_linear_bit= 4,
	/* the bitmaps of 32 bits that have it are given another format */
	_bitmap_unknown5_bit= 5,
	_bitmap_unknown7_bit= 7,
	/* made by the game: its memory and that of its pixels are the console's,
	and are given back when the bitmap is deleted */
	_bitmap_allocated_bit= 8
};

/* ---------- structures */

struct bitmap_data
{
	uint32_t signature;
	int16_t width;
	int16_t height;
	int16_t depth;
	int16_t type;
	int16_t format;
	uint16_t flags;
	uint8_t unknown10[4];
	int16_t mipmap_count;
	/* in hundredths of a level: detail that the bitmap gives up of its own */
	uint8_t mip_bias;
	/* what the bitmap is for; the values are not known */
	uint8_t usage;
	uint8_t unknown18[4];
	/* the pixels at up to six levels of detail, the largest first, each with
	its mipmaps after it. The two high bits of an offset tell the file, as
	in a geometry block's offset; NONE is no block */
	uint32_t pixels_offsets[MAXIMUM_BITMAP_PIXELS_BLOCKS-NUMBER_OF_BITMAP_CACHE_LEVELS];
	/* in the map, the offsets of the fourth to the sixth; in the game, the
	blocks of the texture cache that have the pixels (NONE: not there) */
	uint32_t cache_block_indices[NUMBER_OF_BITMAP_CACHE_LEVELS];
	uint32_t pixels_sizes[MAXIMUM_BITMAP_PIXELS_BLOCKS-NUMBER_OF_BITMAP_CACHE_LEVELS];
	/* in the map, the sizes of the fourth to the sixth; in the game, where
	the pixels are in the texture cache */
	uint32_t cache_addresses[NUMBER_OF_BITMAP_CACHE_LEVELS];
	uint32_t unknown4C;
	/* what follows is the game's, and is 0 in the map: the texture to draw
	with (a pointer), and where its pixels are */
	uint32_t hardware_format;
	uint32_t base_address;
	/* asked for with this much detail to lose or more, the bitmap is drawn
	with what there is */
	float maximum_mip_bias;
	/* Direct3D's texture of the pixels that are read with the map; until it
	is made, where they are and what they are like */
	uint32_t texture[5];
	/* of the texture cache: the last frame that it was drawn in */
	int32_t frame_index;
};

struct bitmap_group
{
	uint8_t unknown0[0x44];
	struct tag_block bitmaps; /* bitmap_data */
	/* the rest of the tag */
};

/* ---------- prototypes/BITMAP_GROUP.C */

struct bitmap_data *bitmap_group_get_bitmap(long bitmap_group_index, short bitmap_index);

typedef char verify_bitmap_data_size[
	sizeof(struct bitmap_data) == 0x74 ? 1 : -1];
typedef char verify_bitmap_data_pixels_offsets_offset[
	offsetof(struct bitmap_data, pixels_offsets) == 0x1C ? 1 : -1];
typedef char verify_bitmap_data_pixels_sizes_offset[
	offsetof(struct bitmap_data, pixels_sizes) == 0x34 ? 1 : -1];
typedef char verify_bitmap_group_bitmaps_offset[
	offsetof(struct bitmap_group, bitmaps) == 0x44 ? 1 : -1];

#endif
