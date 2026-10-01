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
	MAXIMUM_BITMAP_PIXELS_BLOCKS= 6
};

enum
{
	/* the only type that the shaders of the structure bsps use */
	_bitmap_type_2d
};

enum
{
	/* the formats that the diffuse maps of the structure bsps' shaders
	have */
	_bitmap_format_y8= 1,
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
	/* the texels are in the order of the console's swizzled textures; the
	bitmaps that are not compressed have it */
	_bitmap_swizzled_bit= 3
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
	uint8_t unknown10[0xC];
	/* the pixels at up to six levels of detail, the largest first, each with
	its mipmaps after it. The two high bits of an offset tell the file, as
	in a geometry block's offset; NONE is no block */
	uint32_t pixels_offsets[MAXIMUM_BITMAP_PIXELS_BLOCKS];
	uint32_t pixels_sizes[MAXIMUM_BITMAP_PIXELS_BLOCKS];
	uint8_t unknown4C[0x28];
};

struct bitmap_group
{
	uint8_t unknown0[0x44];
	struct tag_block bitmaps; /* bitmap_data */
	/* the rest of the tag */
};

typedef char verify_bitmap_data_size[
	sizeof(struct bitmap_data) == 0x74 ? 1 : -1];
typedef char verify_bitmap_data_pixels_offsets_offset[
	offsetof(struct bitmap_data, pixels_offsets) == 0x1C ? 1 : -1];
typedef char verify_bitmap_data_pixels_sizes_offset[
	offsetof(struct bitmap_data, pixels_sizes) == 0x34 ? 1 : -1];
typedef char verify_bitmap_group_bitmaps_offset[
	offsetof(struct bitmap_group, bitmaps) == 0x44 ? 1 : -1];

#endif
