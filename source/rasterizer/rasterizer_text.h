/*
RASTERIZER_TEXT.H

The pictures of the characters that are drawn. The name of the file is
Halo 1's; that these functions are of it is a guess, and so are their names.
*/

#ifndef __RASTERIZER_TEXT_H
#define __RASTERIZER_TEXT_H

#include "cseries/cseries.h"
#include "bitmaps/bitmap_definitions.h"

/* ---------- constants */

enum
{
	HARDWARE_CHARACTER_CACHE_BITMAP_WIDTH= 256,
	HARDWARE_CHARACTER_CACHE_BITMAP_HEIGHT= 256
};

/* ---------- structures */

/* 0x004b62a0: the bitmap that the characters are drawn from, and where the
next one goes in it. In the game the places of the characters
(font_glyph_slots, 0x004b62b0) are after it, in the same structure. The
names of the fields are Halo 1's */
struct hardware_character_cache
{
	boolean initialized;
	byte unused1;
	short read_index;
	short write_index;
	short x0;
	short y0;
	short maximum_character_height;
	struct bitmap_data *bitmap;
};

typedef char verify_hardware_character_cache_size[
	sizeof(struct hardware_character_cache) == 0x10 ? 1 : -1];

/* ---------- prototypes/RASTERIZER_TEXT.C */

boolean rasterizer_text_cache_initialize(void);
void rasterizer_text_cache_flush(void);

/* ---------- globals */

extern struct hardware_character_cache hardware_character_cache;

#endif
