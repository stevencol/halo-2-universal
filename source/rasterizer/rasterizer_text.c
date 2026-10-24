/*
RASTERIZER_TEXT.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The pictures of the characters that are drawn. The name of the file is
Halo 1's; that these functions are of it is a guess, and so are their names.
*/

#include "rasterizer_text.h"

#include "bitmaps/bitmaps.h"
#include "cache/texture_cache.h"
#include "text/font_cache.h"

#include <string.h>

/* ---------- prototypes of functions that are not decompiled */

/* 0x0001d000: makes the texture that a bitmap is drawn with; whether it
could */
boolean unknown_0001d000(struct bitmap_data *bitmap);

/* 0x00035b90: the console is made ready to paint characters from a
bitmap. The names of these three are Halo 1's */
void __attribute__((stdcall)) rasterizer_text_begin(const struct rasterizer_text_begin_parameters *parameters);

/* 0x000363a0: paints a rectangle of the bitmap of the characters, with
its four corners */
void __attribute__((stdcall)) rasterizer_text_draw_character(struct rasterizer_text_vertex *vertices);

/* 0x00036560: no more characters are painted */
void rasterizer_text_end(void);

/* 0x0001d5e0: where the pixels of the texture of a bitmap are, to write
them, and how many bytes a row of them takes; before, the console is waited
for if it is asked. NULL if the bitmap has no texture */
byte *__attribute__((stdcall)) unknown_0001d5e0(struct bitmap_data *bitmap, long wait, long *pitch);

/* ---------- constants */

/* how the texture of the bitmap of the characters is asked for: it is waited
for, and it is the first level of detail */
#define HARDWARE_CHARACTER_CACHE_TEXTURE_FLAGS \
	(FLAG(_texture_cache_block_bit)|FLAG(_texture_cache_load_bit)|FLAG(_texture_cache_exact_bit))

/* ---------- globals */

/* 0x004b62a0 */
struct hardware_character_cache hardware_character_cache;

/* 0x004b9dd0 */
struct rasterizer_text_bounds rasterizer_text_bounds;

/* 0x0051080c, 0x00485602: while any of the two is not 0, no text is painted.
What they are is not known, and their names are guesses */
boolean rasterizer_text_disabled;

/* 0x004c1a18: characters are being painted: something of the rasterizer has
to be put back when no more are */
boolean rasterizer_text_unknown_004c1a18;

/* 0x0001e930: something of the rasterizer is put back */
void __attribute__((stdcall)) unknown_0001e930(long unknown);
short rasterizer_text_suspended;

/* 0x004670c0: one more with each text that is painted; nothing is known to
look at it. The name is Halo 1's */
short magic_number= 12;

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

/* a place of the bitmap of the characters is left free: the character that
had it has none */
static inline __attribute__((always_inline)) void flush_hardware_character(
	long index)
{
	struct font_glyph_slot *slot= &font_glyph_slots[index];

	if (slot->character_index!=NONE)
	{
		font_character_get(slot->character_index)->glyph_slot_index= NONE;
	}
	slot->character_index= NONE;
}

/* the oldest place of the bitmap of the characters is left free */
static inline __attribute__((always_inline)) void flush_oldest_hardware_character(
	void)
{
	long index= hardware_character_cache.read_index;

	if (index!=NONE)
	{
		flush_hardware_character(index);
	}
	hardware_character_cache.read_index= (short)((hardware_character_cache.read_index+1)&(NUMBER_OF_FONT_GLYPH_SLOTS-1));
}

/* 0x0001f5c0: puts the picture of a character in the bitmap of the
characters, if it has no place in it. The name is Halo 1's.

The pictures are put in rows, from left to right, one pixel away from each
other; a row is as high as the highest of its pictures, and when the bottom
of the bitmap is reached the rows start again from the top. The places are
given in turn, and the oldest ones are left free: those whose pictures the
new one is put over, and one more if there is none left. Around the picture
the pixels are made white that is not seen. If a place was left free the
console is waited for, so that it is not drawing from the bitmap */
void __attribute__((stdcall)) cache_hardware_format_character(
	long character_index)
{
	word pixels[MAXIMUM_HARDWARE_CHARACTER_PIXELS];
	struct font_character *character;
	const struct font_character_header *header= NULL;
	struct font_glyph_slot *slot;
	boolean flushed= FALSE;
	long pitch;
	byte *base;

	if (character_index!=NONE && font_character_get(character_index)->glyph_slot_index!=NONE)
	{
		return;
	}
	character= font_character_get(character_index);
	if (character->state==_font_character_state_ready)
	{
		header= &character->header;
	}

	if (header->width+hardware_character_cache.x0>HARDWARE_CHARACTER_CACHE_BITMAP_WIDTH)
	{
		hardware_character_cache.y0= (short)(hardware_character_cache.y0+hardware_character_cache.maximum_character_height);
		hardware_character_cache.x0= 1;
		hardware_character_cache.maximum_character_height= 0;
	}
	if (header->height+hardware_character_cache.y0>HARDWARE_CHARACTER_CACHE_BITMAP_HEIGHT)
	{
		hardware_character_cache.y0= 1;
		hardware_character_cache.x0= 1;
		hardware_character_cache.maximum_character_height= 0;
		while (hardware_character_cache.read_index!=hardware_character_cache.write_index &&
			font_glyph_slots[hardware_character_cache.read_index].y0>0)
		{
			flushed= TRUE;
			flush_oldest_hardware_character();
		}
	}
	if (header->height+1>hardware_character_cache.maximum_character_height)
	{
		long top= hardware_character_cache.maximum_character_height+hardware_character_cache.y0;
		long bottom= header->height+hardware_character_cache.y0;

		while (hardware_character_cache.read_index!=hardware_character_cache.write_index)
		{
			long y= font_glyph_slots[hardware_character_cache.read_index].y0;

			if (y<top || y>=bottom)
			{
				break;
			}
			flushed= TRUE;
			flush_oldest_hardware_character();
		}
		hardware_character_cache.maximum_character_height= (short)(header->height+1);
	}
	if ((short)((hardware_character_cache.write_index+1)&(NUMBER_OF_FONT_GLYPH_SLOTS-1))==
		hardware_character_cache.read_index)
	{
		flushed= TRUE;
		flush_oldest_hardware_character();
	}

	if (character_index!=NONE)
	{
		character->glyph_slot_index= hardware_character_cache.write_index;
	}
	slot= &font_glyph_slots[hardware_character_cache.write_index];
	slot->character_index= character_index;
	slot->x0= hardware_character_cache.x0;
	slot->y0= hardware_character_cache.y0;
	font_character_get_pixels(character_index, pixels);
	base= unknown_0001d5e0(hardware_character_cache.bitmap, flushed, &pitch);
	if (base)
	{
		struct bitmap_data *bitmap= hardware_character_cache.bitmap;
		long left= slot->x0-1;
		long top= slot->y0-1;
		long right= header->width+slot->x0+1;
		long bottom= header->height+slot->y0+1;
		short border_left= (short)(left>0 ? left : 0);
		short border_top= (short)(top>0 ? top : 0);
		short border_right= (short)(right<=bitmap->width-1 ? right : bitmap->width-1);
		short border_bottom= (short)(bottom<=bitmap->height-1 ? bottom : bitmap->height-1);
		long source= 0;
		long row;

		for (row= border_top; row<border_bottom; row++)
		{
			word *destination= (word *)(base + row*pitch) + border_left;
			long column;

			for (column= border_left; column<border_right; column++)
			{
				*destination++= HARDWARE_CHARACTER_BORDER_PIXEL;
			}
		}
		for (row= 0; row<header->height; row++)
		{
			word *destination= (word *)(base + (slot->y0+row)*pitch) + slot->x0;
			long column;

			for (column= 0; column<header->width; column++)
			{
				destination[column]= pixels[source+column];
			}
			source+= header->width;
		}
		bitmap= hardware_character_cache.bitmap;
		if (bitmap->frame_index<=texture_cache_globals.frame_index || !bitmap->hardware_format)
		{
			if (!texture_cache_bitmap_get_loaded(HARDWARE_CHARACTER_CACHE_TEXTURE_FLAGS, bitmap, 0.0f))
			{
				texture_cache_bitmap_get_hardware_format(bitmap, HARDWARE_CHARACTER_CACHE_TEXTURE_FLAGS, 0.0f);
			}
		}
	}
	hardware_character_cache.x0= (short)(hardware_character_cache.x0+header->width+1);
	hardware_character_cache.write_index= (short)((hardware_character_cache.write_index+1)&(NUMBER_OF_FONT_GLYPH_SLOTS-1));
}

/* 0x0001f070: paints a text in a rectangle of the screen, by a scale, and
leaves where it ends in a place, if one is given: see
draw_string_draw_characters. Nothing is painted if there is no bitmap of the
characters, if the text is empty or while texts are not painted. The name is
a guess.

With no rectangle the text is set in the window of the texts, and what is
out of the screen is not painted; a rectangle that cuts the text starts no
farther left or up than the screen, and ends no farther than it */
void __attribute__((stdcall)) rasterizer_draw_characters(
	const struct rectangle2d *clip,
	const struct rectangle2d *bounds,
	struct draw_string_point *end,
	short leading,
	const long *characters,
	real scale,
	long unknown5,
	long unknown6,
	boolean unknown7)
{
	struct rasterizer_text_begin_parameters parameters;
	struct rectangle2d text_bounds;
	struct rectangle2d text_clip;
	struct bitmap_data *bitmap;
	const struct rectangle2d *viewport= &rasterizer_text_bounds.viewport_bounds;

	if (rasterizer_text_disabled || rasterizer_text_suspended)
	{
		return;
	}
	magic_number++;
	bitmap= hardware_character_cache.initialized ? hardware_character_cache.bitmap : NULL;
	if (!bitmap || !*characters)
	{
		return;
	}

	if (!bounds)
	{
		text_bounds.y0= (short)(rasterizer_text_bounds.window_bounds.y0-viewport->y0);
		text_bounds.x0= (short)(rasterizer_text_bounds.window_bounds.x0-viewport->x0);
		text_bounds.y1= (short)(rasterizer_text_bounds.window_bounds.y1-viewport->y0);
		text_bounds.x1= (short)(rasterizer_text_bounds.window_bounds.x1-viewport->x0);
	}
	else
	{
		text_bounds= *bounds;
	}
	if (!clip)
	{
		text_clip.y0= 0;
		text_clip.x0= 0;
		text_clip.y1= (short)(viewport->y1-viewport->y0);
		text_clip.x1= (short)(viewport->x1-viewport->x0);
	}
	else
	{
		long bottom= viewport->y1-viewport->y0;
		long right= viewport->x1-viewport->x0;

		text_clip.y0= (short)(clip->y0<0 ? 0 : clip->y0);
		text_clip.x0= (short)(clip->x0<0 ? 0 : clip->x0);
		text_clip.y1= (short)(bottom>clip->y1 ? clip->y1 : bottom);
		text_clip.x1= (short)(right>clip->x1 ? clip->x1 : right);
	}

	memset(&parameters, 0, sizeof(parameters));
	if (bitmap->flags&FLAG(_bitmap_linear_bit))
	{
		parameters.texture_scale.x= 1.0f;
		parameters.texture_scale.y= 1.0f;
	}
	else
	{
		parameters.texture_scale.x= 1.0f/(real)bitmap->width;
		parameters.texture_scale.y= 1.0f/(real)bitmap->height;
	}
	parameters.bitmap= bitmap;
	parameters.unknown2C= 1.0f;
	parameters.unknown28= 1.0f;
	parameters.unknown0= 0;
	parameters.unknown96= 0;
	parameters.unknown94= 0;
	parameters.unknown97= unknown7;
	if (unknown5)
	{
		parameters.unknown58= unknown5;
	}
	if (unknown6)
	{
		parameters.unknown5C= unknown6;
	}
	rasterizer_text_begin(&parameters);
	draw_string_draw_characters(rasterizer_draw_character_with_dropshadow, &text_bounds, end, &text_clip, leading,
		scale, characters);
	rasterizer_text_end();
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
	rasterizer_text_draw_character(vertices);
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
	cache_hardware_format_character(character_index);
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

/* 0x00036560: no more characters are painted: if any were, something of the
rasterizer is put back */
void rasterizer_text_end(
	void)
{
	if (rasterizer_text_unknown_004c1a18)
	{
		unknown_0001e930(1);
		rasterizer_text_unknown_004c1a18= FALSE;
	}
}

/* 0x005093e0: it points to something of some other part */
byte *unknown_005093e0;

/* 0x0001fd00: what unknown_005093e0 points to, if there is something, loses
its byte at 0xA0. It is after the others of this file in the game */
void unknown_005093e0_clear_unknownA0(
	void)
{
	if (unknown_005093e0)
	{
		unknown_005093e0[0xA0]= 0;
	}
}

/* 0x004b5690 and 0x004b569d: something of the text, and a byte set when what
unknown_005093e0 points to is cleared */
long unknown_004b5690;
byte unknown_004b569d;

/* 0x0001fcd0: clears the 0xFF longs that unknown_005093e0 points to, if it
points to something, and then marks unknown_004b569d if unknown_004b5690 is set */
void unknown_005093e0_clear(
	void)
{
	if (unknown_005093e0)
	{
		memset(unknown_005093e0, 0, 0xFF*sizeof(long));
		if (unknown_004b5690 && !unknown_004b569d)
		{
			unknown_004b569d= TRUE;
		}
	}
}
