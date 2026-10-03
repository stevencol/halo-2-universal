/*
BITMAPS.H

How many bytes the pixels of a bitmap take.
*/

#ifndef __BITMAPS_H
#define __BITMAPS_H

#include "cseries/cseries.h"
#include "bitmaps/bitmap_definitions.h"
#include "math/real_math.h"

/* ---------- constants */

enum
{
	NUMBER_OF_BITMAP_FORMATS= 24,
	/* a compressed bitmap is made of blocks of 4 by 4 pixels */
	BITMAP_COMPRESSED_BLOCK_SIZE= 4,
	NUMBER_OF_CUBE_MAP_FACES= 6,
	BITS_PER_BYTE= 8,

	/* a real of 16 bits */
	REAL16_EXPONENT_MASK= 0x1F,
	REAL16_FRACTION_SHIFT= 5,
	REAL16_FRACTION_MASK= 0x3FF,
	REAL16_SIGN_SHIFT= 15,
	REAL16_EXPONENT_BIAS= 0x70,

	MEM_RELEASE= 0x8000,
	/* MEM_COMMIT | MEM_TOP_DOWN, and PAGE_READWRITE */
	BITMAP_ALLOCATION_TYPE= 0x101000,
	BITMAP_ALLOCATION_PROTECTION= 4
};

/* ---------- prototypes/BITMAPS.C */

real real16_to_real(word value);
long bitmap_format_get_bits(short format);
struct bitmap_data *bitmap_2d_new(short width, short height, short mipmap_count, short format, word flags);
void bitmap_delete(struct bitmap_data *bitmap);
long bitmap_2d_get_pixel_address(const struct bitmap_data *bitmap, short mipmap_index, short x, short y);
long bitmap_3d_get_pixel_address(const struct bitmap_data *bitmap, short x, short y, short z, short mipmap_index);
long bitmap_cube_map_get_pixel_address(const struct bitmap_data *bitmap, short x, short y, short face,
	short mipmap_index);
long bitmap_pixels_size(const struct bitmap_data *bitmap);
unsigned long __attribute__((stdcall)) bitmap_get_pixel(short y, short width, const byte *buffer,
	unsigned long buffer_size, const byte *pixels, short x, short height, short format, unsigned long flags);
long bitmap_mipmap_pixel_count(const struct bitmap_data *bitmap, byte mipmap_index);
long bitmap_row_size(byte mipmap_index, const struct bitmap_data *bitmap);
long bitmap_dimensions_mipmap_size(short width, byte mipmap_index, short height, short depth, short format,
	short row_alignment);
long bitmap_dimensions_size(short mipmap_count, short width, short height, short depth, short format,
	short row_alignment);

/* ---------- globals */

extern const short bitmap_format_bits_per_pixel[NUMBER_OF_BITMAP_FORMATS];
extern boolean bitmap_pixel_out_of_bounds;

#endif
