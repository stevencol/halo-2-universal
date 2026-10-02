/*
BITMAPS.H

How many bytes the pixels of a bitmap take.
*/

#ifndef __BITMAPS_H
#define __BITMAPS_H

#include "cseries/cseries.h"
#include "bitmaps/bitmap_definitions.h"

/* ---------- constants */

enum
{
	NUMBER_OF_BITMAP_FORMATS= 24,
	/* a compressed bitmap is made of blocks of 4 by 4 pixels */
	BITMAP_COMPRESSED_BLOCK_SIZE= 4,
	NUMBER_OF_CUBE_MAP_FACES= 6,
	BITS_PER_BYTE= 8
};

/* ---------- prototypes/BITMAPS.C */

long bitmap_pixels_size(const struct bitmap_data *bitmap);
long bitmap_mipmap_pixel_count(const struct bitmap_data *bitmap, byte mipmap_index);
long bitmap_row_size(byte mipmap_index, const struct bitmap_data *bitmap);
long bitmap_dimensions_mipmap_size(short width, byte mipmap_index, short height, short depth, short format,
	short row_alignment);
long bitmap_dimensions_size(short mipmap_count, short width, short height, short depth, short format,
	short row_alignment);

/* ---------- globals */

extern const short bitmap_format_bits_per_pixel[NUMBER_OF_BITMAP_FORMATS];

#endif
