/*
BITMAPS.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

How many bytes the pixels of a bitmap take. The name of the file is Halo 1's;
that these functions are of it is a guess, and so are their names.
*/

#include "bitmaps.h"

/* ---------- globals */

/* 0x00453550: the bits that a pixel takes in each format; 0 for a format
that there is not */
const short bitmap_format_bits_per_pixel[NUMBER_OF_BITMAP_FORMATS]=
{
	8, 8, 8, 16, 0, 0, 16, 0, 16, 16, 32, 32, 0, 0, 4, 8, 8, 8, 8, 128, 96, 48, 16, 16
};

/* ---------- code */

/* one of the sizes of a bitmap at a mipmap: half at each one, and no less
than 1 */
static inline __attribute__((always_inline)) short bitmap_mipmap_dimension(
	short dimension,
	byte mipmap_index)
{
	short reduced= (short)(dimension>>(mipmap_index&31));

	return reduced>1 ? reduced : 1;
}

/* up to the size of a block of a compressed bitmap. The game does the sum
with the low byte of the size */
static inline __attribute__((always_inline)) short bitmap_dimension_round_up_to_block(
	short dimension)
{
	return (short)(dimension + ((byte)-(byte)dimension&(BITMAP_COMPRESSED_BLOCK_SIZE-1)));
}

static inline __attribute__((always_inline)) short bitmap_format_get_bits_per_pixel(
	short format)
{
	return format==NONE ? 0 : bitmap_format_bits_per_pixel[format];
}

/* 0x00136490: the bytes of the pixels of a bitmap, with all of its mipmaps */
long bitmap_pixels_size(
	const struct bitmap_data *bitmap)
{
	short mipmap_count= bitmap->mipmap_count;
	long pixel_count= 0;
	short mipmap_index;

	for (mipmap_index= 0; mipmap_index<=mipmap_count; mipmap_index++)
	{
		pixel_count+= bitmap_mipmap_pixel_count(bitmap, (byte)mipmap_index);
	}

	return bitmap_format_get_bits_per_pixel(bitmap->format)*pixel_count/BITS_PER_BYTE;
}

/* 0x001364e0: the pixels of a mipmap of a bitmap: of its six faces, if it
is a cube map. A compressed bitmap has whole blocks */
long bitmap_mipmap_pixel_count(
	const struct bitmap_data *bitmap,
	byte mipmap_index)
{
	short width= bitmap_mipmap_dimension(bitmap->width, mipmap_index);
	short height= bitmap_mipmap_dimension(bitmap->height, mipmap_index);
	char depth= (char)((char)bitmap->depth>>(mipmap_index&31));
	long pixel_count;

	if (bitmap->flags&FLAG(_bitmap_compressed_bit))
	{
		width= bitmap_dimension_round_up_to_block(width);
		height= bitmap_dimension_round_up_to_block(height);
	}
	if (depth<=1)
	{
		depth= 1;
	}
	pixel_count= (long)height*(long)width*(long)depth;
	if (bitmap->type==_bitmap_type_cube_map)
	{
		pixel_count*= NUMBER_OF_CUBE_MAP_FACES;
	}

	return pixel_count;
}

/* 0x001365a0: the bytes of a row of the pixels of a bitmap, at a mipmap */
long bitmap_row_size(
	byte mipmap_index,
	const struct bitmap_data *bitmap)
{
	short width= bitmap_mipmap_dimension(bitmap->width, mipmap_index);

	if (bitmap->flags&FLAG(_bitmap_compressed_bit))
	{
		width= bitmap_dimension_round_up_to_block(width);
	}

	return (long)bitmap_format_get_bits_per_pixel(bitmap->format)*(long)width/BITS_PER_BYTE;
}

/* 0x00136600: the bytes of a mipmap of a bitmap of some sizes and a format:
its rows, each of a number of bytes that is a multiple of another if one is
given, which is a power of two. The bytes of a row are a number of 16 bits */
long bitmap_dimensions_mipmap_size(
	short width,
	byte mipmap_index,
	short height,
	short depth,
	short format,
	short row_alignment)
{
	short row_size;

	width= bitmap_mipmap_dimension(width, mipmap_index);
	height= bitmap_mipmap_dimension(height, mipmap_index);
	depth= bitmap_mipmap_dimension(depth, mipmap_index);
	if (format>=_bitmap_format_dxt1 && format<=_bitmap_format_dxt5)
	{
		width= bitmap_dimension_round_up_to_block(width);
		height= bitmap_dimension_round_up_to_block(height);
	}
	row_size= (short)((long)bitmap_format_get_bits_per_pixel(format)*(long)width/BITS_PER_BYTE);
	if (row_alignment>0)
	{
		row_size= (short)((row_size+row_alignment-1)&~(row_alignment-1));
	}

	return (long)row_size*(long)depth*(long)height;
}

/* 0x001366d0: those of all of its mipmaps */
long bitmap_dimensions_size(
	short mipmap_count,
	short width,
	short height,
	short depth,
	short format,
	short row_alignment)
{
	long size= 0;
	short mipmap_index;

	for (mipmap_index= 0; mipmap_index<=mipmap_count; mipmap_index++)
	{
		size+= bitmap_dimensions_mipmap_size(width, (byte)mipmap_index, height, depth, format, row_alignment);
	}

	return size;
}
