/*
BITMAP_UTILITIES.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.
*/

#include "bitmap_utilities.h"

/* ---------- code */

/* 0x00131bb0 */
void pixel32_to_real_rgb_color(
	pixel32 color,
	real_rgb_color *result)
{
	result->red= (real)((color>>16)&0xFF)*(1.0f/255.0f);
	result->green= (real)((color>>8)&0xFF)*(1.0f/255.0f);
	result->blue= (real)(color&0xFF)*(1.0f/255.0f);

	return;
}
