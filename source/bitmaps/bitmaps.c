/*
BITMAPS.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

How many bytes the pixels of a bitmap take. The name of the file is Halo 1's;
that these functions are of it is a guess, and so are their names.
*/

#include "bitmaps.h"

#include "cache/texture_cache.h"

#include <math.h>
#include <string.h>

/* ---------- prototypes of functions of the console's libraries */

/* 0x003fb640: one user less of a texture */
unsigned long D3DResource_Release(uint32_t resource);

/* 0x003312cc */
void *VirtualAlloc(void *address, unsigned long size, unsigned long type, unsigned long protection);

/* 0x003312fa */
int VirtualFree(void *address, unsigned long size, unsigned long type);

/* 0x002d1d3e */
uint32_t GetLastError(void);

/* ---------- globals */

/* 0x00453550: the bits that a pixel takes in each format; 0 for a format
that there is not */
const short bitmap_format_bits_per_pixel[NUMBER_OF_BITMAP_FORMATS]=
{
	8, 8, 8, 16, 0, 0, 16, 0, 16, 16, 32, 32, 0, 0, 4, 8, 8, 8, 8, 128, 96, 48, 16, 16
};

/* 0x0055e72c: set when bitmap_get_pixel is asked for a block of a
compressed bitmap out of its pixels */
boolean bitmap_pixel_out_of_bounds;

/* ---------- prototypes of functions that are not decompiled */

/* 0x00135ca0: a pixel of a format as one of 32 bits, alpha, red, green and
blue. The name is Halo 1's */
unsigned long bitmap_format_to_a8r8g8b8(short format, const byte *pixels, long pixel_index);

/* 0x000358d0: where a pixel is in a swizzled texture of a width and a
height, in two parts that are or'ed */
void __attribute__((stdcall)) unknown_000358d0(short width, short y, short x, long height, unsigned long *index);

/* 0x00223ed0, 0x00224040 and 0x00224080: a pixel of a block of 4 by 4 of
the formats dxt1, dxt3 and dxt5 */
void __attribute__((stdcall)) unknown_00223ed0(unsigned long *color, const byte *block, long x, long y);
void __attribute__((stdcall)) unknown_00224040(const byte *block, long x, long y, unsigned long *color);
void __attribute__((stdcall)) unknown_00224080(const byte *block, unsigned long *color, long x, long y);

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

/* 0x00135880: a real of 16 bits as one of 32: its sign is its high bit, its
exponent its 5 low bits, and the rest is what is after the point. No number
is told apart: not zero either */
real real16_to_real(
	word value)
{
	union
	{
		unsigned long bits;
		real value;
	} result;
	unsigned long bits= value;

	result.bits= ((((bits>>REAL16_FRACTION_SHIFT)&REAL16_FRACTION_MASK) | ((bits>>REAL16_SIGN_SHIFT)<<18))<<13) |
		(((bits&REAL16_EXPONENT_MASK)+REAL16_EXPONENT_BIAS)<<23);

	return result.value;
}

/* 0x001358c0: the bits that a pixel takes in a format; 0 if there is none */
long bitmap_format_get_bits(
	short format)
{
	return bitmap_format_get_bits_per_pixel(format);
}

/* 0x001358e0: a bitmap that the game makes, with memory of the console for
it and, but for _bitmap_no_pixels_bit, for its pixels. NULL if there is no
memory for it; with none for its pixels, the bitmap has no pixels */
struct bitmap_data *bitmap_2d_new(
	short width,
	short height,
	short mipmap_count,
	short format,
	word flags)
{
	struct bitmap_data *bitmap= VirtualAlloc(NULL, sizeof(struct bitmap_data), BITMAP_ALLOCATION_TYPE,
		BITMAP_ALLOCATION_PROTECTION);

	if (!bitmap)
	{
		GetLastError();

		return NULL;
	}
	memset(bitmap, 0, sizeof(struct bitmap_data));
	bitmap->mipmap_count= mipmap_count;
	bitmap->type= _bitmap_type_2d;
	bitmap->width= width;
	bitmap->signature= BITMAP_SIGNATURE;
	bitmap->height= height;
	bitmap->depth= 1;
	bitmap->format= format;
	bitmap->flags= flags|FLAG(_bitmap_allocated_bit);
	if (!(width&(width-1)) && !(height&(height-1)))
	{
		bitmap->flags|= FLAG(_bitmap_power_of_two_dimensions_bit);
	}
	if (format>=_bitmap_format_dxt1 && format<=_bitmap_format_dxt5)
	{
		bitmap->flags|= FLAG(_bitmap_compressed_bit);
	}
	if (format==_bitmap_format_p8)
	{
		bitmap->flags|= FLAG(_bitmap_palettized_bit);
	}
	bitmap->base_address= 0;
	if (!(flags&FLAG(_bitmap_no_pixels_bit)))
	{
		void *pixels= VirtualAlloc(NULL, bitmap_pixels_size(bitmap), BITMAP_ALLOCATION_TYPE,
			BITMAP_ALLOCATION_PROTECTION);

		if (!pixels)
		{
			GetLastError();
		}
		bitmap->base_address= (uint32_t)pixels;
	}

	return bitmap;
}

/* 0x001359d0: the end of a bitmap: it is taken out of the texture cache, the
texture that it is drawn with is let go of, and if the bitmap was made by the
game the memory of its pixels and its own are given back */
void bitmap_delete(
	struct bitmap_data *bitmap)
{
	if (!bitmap)
	{
		return;
	}
	texture_cache_bitmap_delete(bitmap);
	if (bitmap->hardware_format)
	{
		D3DResource_Release(bitmap->hardware_format);
		bitmap->hardware_format= 0;
	}
	if (bitmap->flags&FLAG(_bitmap_allocated_bit))
	{
		if (bitmap->base_address && !VirtualFree((void *)bitmap->base_address, 0, MEM_RELEASE))
		{
			GetLastError();
		}
		if (!VirtualFree(bitmap, 0, MEM_RELEASE))
		{
			GetLastError();
		}
	}
}

/* one of the sizes of a bitmap at the next mipmap: half, and no less than
the least that there is */
static inline __attribute__((always_inline)) short bitmap_dimension_next_mipmap(
	short dimension,
	short minimum)
{
	short half= (short)(dimension>>1);

	return minimum<=half ? half : minimum;
}

static inline __attribute__((always_inline)) short bitmap_get_minimum_dimension(
	const struct bitmap_data *bitmap)
{
	return (bitmap->flags&FLAG(_bitmap_compressed_bit)) ? BITMAP_COMPRESSED_BLOCK_SIZE : 1;
}

/* 0x00135a30: where a pixel of a bitmap is, at a mipmap: the mipmaps are one
after another, and each has its rows one after another. A compressed bitmap
has no mipmap smaller than a block */
long bitmap_2d_get_pixel_address(
	const struct bitmap_data *bitmap,
	short mipmap_index,
	short x,
	short y)
{
	short minimum= bitmap_get_minimum_dimension(bitmap);
	short bits_per_pixel= bitmap_format_get_bits_per_pixel(bitmap->format);
	short width= bitmap->width;
	short height= bitmap->height;
	long pixel_count= 0;
	short index;

	for (index= 0; index<mipmap_index; index++)
	{
		pixel_count+= (long)width*(long)height;
		width= bitmap_dimension_next_mipmap(width, minimum);
		height= bitmap_dimension_next_mipmap(height, minimum);
	}

	return (long)(((long)width*y + pixel_count + x)*bits_per_pixel/BITS_PER_BYTE + (long)bitmap->base_address);
}

/* 0x00135af0: where a pixel of a bitmap of three dimensions is, at a mipmap.
Its depth is its low byte, and is halved to no less than 1 */
long bitmap_3d_get_pixel_address(
	const struct bitmap_data *bitmap,
	short x,
	short y,
	short z,
	short mipmap_index)
{
	short minimum= bitmap_get_minimum_dimension(bitmap);
	short bits_per_pixel= bitmap_format_get_bits_per_pixel(bitmap->format);
	short width= bitmap->width;
	short height= bitmap->height;
	short depth= (char)bitmap->depth;
	long pixel_count= 0;
	short index;

	for (index= 0; index<mipmap_index; index++)
	{
		pixel_count+= (long)width*(long)height*(long)depth;
		width= bitmap_dimension_next_mipmap(width, minimum);
		height= bitmap_dimension_next_mipmap(height, minimum);
		depth= (short)(depth&~1)>=2 ? (short)(depth>>1) : 1;
	}

	return (long)((((long)height*z + y)*width + pixel_count + x)*bits_per_pixel/BITS_PER_BYTE + (long)bitmap->base_address);
}

/* 0x00135c00: where a pixel of a cube map is: of one of its faces, at a
mipmap. The six faces of each mipmap are one after another, and those of a
mipmap after those of the one before. A compressed cube map has no mipmap
smaller than a block */
long bitmap_cube_map_get_pixel_address(
	const struct bitmap_data *bitmap,
	short x,
	short y,
	short face,
	short mipmap_index)
{
	short minimum_size= bitmap_get_minimum_dimension(bitmap);
	short bits_per_pixel= bitmap_format_get_bits_per_pixel(bitmap->format);
	short size= bitmap->width;
	long pixel_count= 0;
	short index;

	for (index= 0; index<mipmap_index; index++)
	{
		pixel_count+= (long)size*(long)size*NUMBER_OF_CUBE_MAP_FACES;
		size= bitmap_dimension_next_mipmap(size, minimum_size);
	}

	return (long)((x + ((long)face*size + y)*size + pixel_count)*bits_per_pixel/BITS_PER_BYTE + (long)bitmap->base_address);
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

/* 0x00136140: a pixel of some pixels of a width, as one of 32 bits. A
pixel of a compressed bitmap is from its block of 4 by 4, which has to be
in a buffer; if it is not, the first block of the buffer is looked at and
bitmap_pixel_out_of_bounds is set. 0 for a compressed format that is not
dxt. The name is a guess */
unsigned long __attribute__((stdcall)) bitmap_get_pixel(
	short y,
	short width,
	const byte *buffer,
	unsigned long buffer_size,
	const byte *pixels,
	short x,
	short height,
	short format,
	unsigned long flags)
{
	if (TEST_FLAG(flags, _bitmap_compressed_bit))
	{
		short block_size= (short)(bitmap_format_get_bits_per_pixel(format)*BITMAP_COMPRESSED_BLOCK_SIZE*
			BITMAP_COMPRESSED_BLOCK_SIZE/BITS_PER_BYTE);
		long block_index= (long)(short)(y/BITMAP_COMPRESSED_BLOCK_SIZE)*width/BITMAP_COMPRESSED_BLOCK_SIZE +
			(short)(x/BITMAP_COMPRESSED_BLOCK_SIZE);
		const byte *block= pixels+block_index*block_size;
		long block_x= x&(BITMAP_COMPRESSED_BLOCK_SIZE-1);
		long block_y= y&(BITMAP_COMPRESSED_BLOCK_SIZE-1);
		/* where the game leaves the pixel is where it was given the flags */
		unsigned long color= flags;

		if (block<buffer || block>=buffer+buffer_size)
		{
			if (!bitmap_pixel_out_of_bounds)
			{
				bitmap_pixel_out_of_bounds= TRUE;
			}
			block_y= 0;
			block= buffer;
			block_x= 0;
		}
		switch (format)
		{
		case _bitmap_format_dxt1:
			unknown_00223ed0(&color, block, block_x, block_y);
			break;
		case _bitmap_format_dxt3:
			unknown_00224040(block, block_x, block_y, &color);
			break;
		case _bitmap_format_dxt5:
			unknown_00224080(block, &color, block_x, block_y);
			break;
		default:
			return 0;
		}

		return color;
	}
	if (TEST_FLAG(flags, _bitmap_swizzled_bit))
	{
		unsigned long index[2];

		unknown_000358d0(width, y, x, height, index);
		return bitmap_format_to_a8r8g8b8(format, pixels, (long)(index[1]|index[0]));
	}

	return bitmap_format_to_a8r8g8b8(format, pixels, (long)y*width + x);
}

/* one of the coordinates of a pixel at a point of a bitmap from 0 to 1, of
a size: the nearest, and the bitmap is as if it was repeated */
static inline __attribute__((always_inline)) long bitmap_sample_coordinate(
	short size,
	real coordinate)
{
	long pixel= lrintf((real)size*coordinate - 0.5f);

	if ((size&(size-1))==0)
	{
		return pixel&(size-1);
	}

	return (pixel%size + size)%size;
}

/* 0x001362b0: the pixel of a bitmap at a point from 0 to 1, as one of 32
bits, at a detail: 1 or more is the largest mipmap, and less goes to the
smaller ones. NONE for a bitmap whose pixels are not there. The name is a
guess */
unsigned long bitmap_sample(
	const struct bitmap_data *bitmap,
	real detail,
	const real_point2d *point)
{
	long mipmap_index;
	short width;
	short height;
	long x;
	long y;
	long address;

	if (!bitmap->base_address)
	{
		return NONE;
	}
	if (1.0f>detail && bitmap->mipmap_count>0)
	{
		mipmap_index= lrintf((1.0f-detail)*(real)bitmap->mipmap_count);
	}
	else
	{
		mipmap_index= 0;
	}
	width= bitmap_mipmap_dimension(bitmap->width, (byte)mipmap_index);
	if (TEST_FLAG(bitmap->flags, _bitmap_compressed_bit))
	{
		width= bitmap_dimension_round_up_to_block(width);
	}
	height= bitmap_mipmap_dimension(bitmap->height, (byte)mipmap_index);
	if (TEST_FLAG(bitmap->flags, _bitmap_compressed_bit))
	{
		height= bitmap_dimension_round_up_to_block(height);
	}
	x= bitmap_sample_coordinate(width, point->x);
	y= bitmap_sample_coordinate(height, point->y);
	switch (bitmap->type)
	{
	case _bitmap_type_2d:
		address= bitmap_2d_get_pixel_address(bitmap, (short)mipmap_index, 0, 0);
		break;
	case _bitmap_type_3d:
		address= bitmap_3d_get_pixel_address(bitmap, 0, 0, 0, (short)mipmap_index);
		break;
	default:
		address= bitmap_cube_map_get_pixel_address(bitmap, 0, 0, 0, (short)mipmap_index);
		break;
	}

	return bitmap_get_pixel((short)y, width, (const byte *)bitmap->base_address, bitmap->pixels_sizes[0],
		(const byte *)address, (short)x, height, bitmap->format, bitmap->flags);
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
