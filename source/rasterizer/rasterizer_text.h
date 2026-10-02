/*
RASTERIZER_TEXT.H

The pictures of the characters that are drawn. The name of the file is
Halo 1's; that these functions are of it is a guess, and so are their names.
*/

#ifndef __RASTERIZER_TEXT_H
#define __RASTERIZER_TEXT_H

#include "cseries/cseries.h"
#include "bitmaps/bitmap_definitions.h"
#include "math/real_math.h"
#include "text/draw_string.h"
#include "text/font_cache.h"

/* ---------- constants */

enum
{
	HARDWARE_CHARACTER_CACHE_BITMAP_WIDTH= 256,
	HARDWARE_CHARACTER_CACHE_BITMAP_HEIGHT= 256,
	/* the pixels that a character can have */
	MAXIMUM_HARDWARE_CHARACTER_PIXELS= 0x1000,
	/* white that is not seen: what is around the picture of a character */
	HARDWARE_CHARACTER_BORDER_PIXEL= 0x0FFF
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

/* a corner of a character that is painted: where it is on the screen, where
it is in the bitmap of the characters, in pixels, and its colour */
struct rasterizer_text_vertex
{
	real_point2d position;
	real_point2d texcoord;
	unsigned long color;
};

typedef char verify_rasterizer_text_vertex_size[
	sizeof(struct rasterizer_text_vertex) == 0x14 ? 1 : -1];

enum
{
	NUMBER_OF_RASTERIZER_TEXT_VERTICES= 4
};

/* what the console is told before the characters of a text are painted.
Most of it is not known */
struct rasterizer_text_begin_parameters
{
	long unknown0;
	byte unknown4[8];
	/* what is painted from */
	struct bitmap_data *bitmap;
	byte unknown10[0x18];
	real unknown28;
	real unknown2C;
	byte unknown30[0x10];
	/* by what the places in the bitmap are multiplied: 1 if they are in
	pixels, and if not what makes them go from 0 to 1 */
	real_point2d texture_scale;
	byte unknown48[0x10];
	long unknown58;
	long unknown5C;
	byte unknown60[0x34];
	short unknown94;
	byte unknown96;
	boolean unknown97;
};

typedef char verify_rasterizer_text_begin_parameters_size[
	sizeof(struct rasterizer_text_begin_parameters) == 0x98 ? 1 : -1];
typedef char verify_rasterizer_text_begin_parameters_unknown58_offset[
	offsetof(struct rasterizer_text_begin_parameters, unknown58) == 0x58 ? 1 : -1];

/* 0x004b9dd0: the part of the screen that the game draws in, and the part of
it that the texts are set in, if they are told no other. They are part of a
larger structure; the names are those of Halo 1's camera */
struct rasterizer_text_bounds
{
	struct rectangle2d viewport_bounds;
	struct rectangle2d window_bounds;
};

/* is given the four corners of a character before it is painted, and what
it was to be told: it can change them, and whether the character is painted.
It takes its arguments off the stack itself */
typedef boolean (__attribute__((stdcall)) *rasterizer_text_vertices_proc)(struct rasterizer_text_vertex *vertices,
	long data);

/* ---------- prototypes/RASTERIZER_TEXT.C */

void __attribute__((stdcall)) cache_hardware_format_character(long character_index);
void __attribute__((stdcall)) rasterizer_draw_characters(const struct rectangle2d *clip,
	const struct rectangle2d *bounds, struct draw_string_point *end, short leading, const long *characters,
	real scale, long unknown5, long unknown6, boolean unknown7);
void __attribute__((stdcall)) rasterizer_draw_character(unsigned long color, rasterizer_text_vertices_proc proc,
	long height, long width, const struct font_glyph_slot *slot, real x, real y, short skipped_width,
	short skipped_height, real scale, long data);
void DRAW_STRING_CALLBACK rasterizer_draw_character_with_dropshadow(const struct text_iterator *iterator,
	long font_index, long character, unsigned long color, unsigned long shadow_color, real x, real y,
	real skipped_width, real skipped_height, real width, real height, real scale, long vertices_proc,
	long vertices_proc_data);

boolean rasterizer_text_cache_initialize(void);
void rasterizer_text_cache_flush(void);

/* ---------- globals */

extern struct hardware_character_cache hardware_character_cache;
extern struct rasterizer_text_bounds rasterizer_text_bounds;
extern boolean rasterizer_text_disabled;
extern short rasterizer_text_suspended;
extern short magic_number;

#endif
