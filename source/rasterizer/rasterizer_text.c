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

/* 0x000363a0: paints a rectangle of the bitmap of the characters, with
its four corners */
void __attribute__((stdcall)) unknown_000363a0(struct rasterizer_text_vertex *vertices);

/* 0x0001f5c0: puts the picture of a character in the bitmap of the
characters, if it is not there */
void __attribute__((stdcall)) unknown_0001f5c0(long character_index);

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

/* 0x0001f3a0: paints what is seen of a character, from its place in the
bitmap of the characters: so many pixels wide and high, by a scale, at a
place of the screen, from so many pixels into its picture. A function, if
there is one, is given its corners first, and can have it not painted. As
Halo 1's function of this name */
void __attribute__((stdcall)) rasterizer_draw_character(
	unsigned long color,
	rasterizer_text_vertices_proc proc,
	long height,
	long width,
	const struct font_glyph_slot *slot,
	real x,
	real y,
	short skipped_width,
	short skipped_height,
	real scale,
	long data)
{
	struct rasterizer_text_vertex vertices[NUMBER_OF_RASTERIZER_TEXT_VERTICES];
	long u0= (short)(slot->x0+skipped_width);
	long v0= (short)(slot->y0+skipped_height);
	real x1= (real)width*scale + x;
	real y1= (real)height*scale + y;

	vertices[0].position.x= x;
	vertices[0].position.y= y;
	vertices[0].texcoord.x= (real)u0;
	vertices[0].texcoord.y= (real)v0;
	vertices[0].color= color;
	vertices[1].position.x= x1;
	vertices[1].position.y= y;
	vertices[1].texcoord.x= (real)(u0+width);
	vertices[1].texcoord.y= (real)v0;
	vertices[1].color= color;
	vertices[2].position.x= x1;
	vertices[2].position.y= y1;
	vertices[2].texcoord.x= (real)(u0+width);
	vertices[2].texcoord.y= (real)(v0+height);
	vertices[2].color= color;
	vertices[3].position.x= x;
	vertices[3].position.y= y1;
	vertices[3].texcoord.x= (real)u0;
	vertices[3].texcoord.y= (real)(v0+height);
	vertices[3].color= color;
	if (proc && !proc(vertices, data))
	{
		return;
	}
	unknown_000363a0(vertices);
}

/* 0x0001f490: what is done with each character of a text that is painted:
its picture is put in the bitmap of the characters, if it is not there, and
it is painted from it. With a shadow (a shadow colour that is not all
transparent), the shadow is painted first: the same picture in the colour of
the shadow, as far down and to the right as the scale. As Halo 1's function
of this name */
void DRAW_STRING_CALLBACK rasterizer_draw_character_with_dropshadow(
	const struct text_iterator *iterator,
	long font_index,
	long character,
	unsigned long color,
	unsigned long shadow_color,
	real x,
	real y,
	real skipped_width,
	real skipped_height,
	real width,
	real height,
	real scale,
	long vertices_proc,
	long vertices_proc_data)
{
	long character_index= font_character_find(font_slot_indices[font_index], character);
	long glyph_slot_index;
	boolean shadow;

	(void)iterator;

	if (character_index==NONE)
	{
		font_character_get_header(character, font_index,
			FLAG(_font_character_request_wait_bit)|FLAG(_font_character_request_new_bit));
		character_index= font_character_find(font_slot_indices[font_index], character);
		if (character_index==NONE)
		{
			return;
		}
	}
	unknown_0001f5c0(character_index);
	glyph_slot_index= font_character_get(character_index)->glyph_slot_index;
	if (glyph_slot_index==NONE)
	{
		return;
	}
	shadow= (shadow_color&0xFF000000)!=0;
	for (;;)
	{
		real offset= shadow ? scale : 0.0f;

		rasterizer_draw_character(shadow ? shadow_color : color, (rasterizer_text_vertices_proc)vertices_proc,
			(long)height, (long)width, &font_glyph_slots[glyph_slot_index], offset+x, offset+y,
			(short)(long)skipped_width, (short)(long)skipped_height, scale, vertices_proc_data);
		if (!shadow)
		{
			break;
		}
		shadow= FALSE;
	}
}
