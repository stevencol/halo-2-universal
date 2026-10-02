/*
RASTERIZER_TEXT.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The pictures of the characters that are drawn. The name of the file is
Halo 1's; that these functions are of it is a guess, and so are their names.
*/

#include "rasterizer_text.h"

#include "bitmaps/bitmaps.h"
#include "text/font_cache.h"

#include <string.h>

/* ---------- prototypes of functions that are not decompiled */

/* 0x0001d000: makes the texture that a bitmap is drawn with; whether it
could */
boolean unknown_0001d000(struct bitmap_data *bitmap);

/* ---------- globals */

/* 0x004b62a0 */
struct hardware_character_cache hardware_character_cache;

/* ---------- code */

/* 0x0001eff0: makes the bitmap that the characters are drawn from, of 256 by
256 pixels of 16 bits, with its texture and with no pixels of its own, and
leaves all the places of the characters free. Whether it could: if the
bitmap cannot be made what there was stays, and if its texture cannot be
made the bitmap is not given back */
boolean rasterizer_text_cache_initialize(
	void)
{
	boolean result= TRUE;
	struct bitmap_data *bitmap= bitmap_2d_new(HARDWARE_CHARACTER_CACHE_BITMAP_WIDTH,
		HARDWARE_CHARACTER_CACHE_BITMAP_HEIGHT, 0, _bitmap_format_a4r4g4b4,
		FLAG(_bitmap_linear_bit)|FLAG(_bitmap_no_pixels_bit));
	short index;

	if (bitmap)
	{
		memset(&hardware_character_cache, 0, sizeof(hardware_character_cache));
		memset(font_glyph_slots, 0, sizeof(font_glyph_slots));
	}
	if (bitmap && unknown_0001d000(bitmap))
	{
		hardware_character_cache.bitmap= bitmap;
		hardware_character_cache.initialized= TRUE;
		hardware_character_cache.x0= 1;
		hardware_character_cache.y0= 1;
	}
	else
	{
		result= FALSE;
	}
	for (index= 0; index<NUMBER_OF_FONT_GLYPH_SLOTS; index++)
	{
		struct font_glyph_slot *slot= &font_glyph_slots[index];

		memset(slot, 0, sizeof(*slot));
		slot->character_index= NONE;
	}

	return result;
}

/* 0x0001f2b0: all the places of the characters are left free: the
characters that had them have none */
void rasterizer_text_cache_flush(
	void)
{
	short index;

	if (!hardware_character_cache.initialized)
	{
		return;
	}
	for (index= 0; index<NUMBER_OF_FONT_GLYPH_SLOTS; index++)
	{
		struct font_glyph_slot *slot= &font_glyph_slots[index];

		if (slot->character_index!=NONE)
		{
			font_character_get(slot->character_index)->glyph_slot_index= NONE;
			slot->character_index= NONE;
		}
	}
}
