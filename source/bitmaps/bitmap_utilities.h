/*
BITMAP_UTILITIES.H
*/

#ifndef __BITMAP_UTILITIES_H
#define __BITMAP_UTILITIES_H

#include "math/real_math.h"

/* ---------- types */

/* a color of 8 bits for each of alpha, red, green and blue, from the high
bits to the low */
typedef unsigned long pixel32;

/* ---------- prototypes/BITMAP_UTILITIES.C */

void pixel32_to_real_rgb_color(pixel32 color, real_rgb_color *result);

/* ---------- prototypes/BITMAP_COLOR_CONVERSION.C */

pixel32 real_argb_color_to_pixel32(const real_argb_color *color);
pixel32 real_rgb_color_to_pixel32(const real_rgb_color *color);
pixel32 real_alpha_to_pixel32(real alpha);
pixel32 real_alpha_and_rgb_color_to_pixel32(real alpha, const real_rgb_color *color);

#endif
