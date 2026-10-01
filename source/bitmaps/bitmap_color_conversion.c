/*
BITMAP_COLOR_CONVERSION.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

Each component is rounded to the nearest integer, as the processor rounds.
*/

#include "bitmap_utilities.h"

#include <math.h>

/* ---------- code */

/* 0x00131e50: a component over 1 or under 0 gets into the bits of the
others */
pixel32 real_argb_color_to_pixel32(
	const real_argb_color *color)
{
	long alpha= lrintf(color->alpha*255.0f);
	long red= lrintf(color->rgb.red*255.0f);
	long green= lrintf(color->rgb.green*255.0f);
	long blue= lrintf(255.0f*color->rgb.blue);

	return (pixel32)blue | (pixel32)green<<8 | (pixel32)red<<16 | (pixel32)alpha<<24;
}

/* 0x00131f40: a color with an alpha apart. The name is a guess */
pixel32 real_alpha_and_rgb_color_to_pixel32(
	real alpha,
	const real_rgb_color *color)
{
	return real_rgb_color_to_pixel32(color) | real_alpha_to_pixel32(alpha);
}

/* 0x00131fc0: as in Halo 1 */
pixel32 real_alpha_to_pixel32(
	real alpha)
{
	return (pixel32)lrintf(alpha*255.0f)<<24;
}

/* 0x00131ed0 */
pixel32 real_rgb_color_to_pixel32(
	const real_rgb_color *color)
{
	long red= lrintf(color->red*255.0f);
	long green= lrintf(color->green*255.0f);
	long blue= lrintf(255.0f*color->blue);

	return ((pixel32)blue&0xFF) | ((pixel32)green&0xFF)<<8 | ((pixel32)red&0xFF)<<16;
}
