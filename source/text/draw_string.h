/*
DRAW_STRING.H

Going through a text to draw it. The name of the file is Halo 1's; that
these functions are of it is a guess, and so are their names.
*/

#ifndef __DRAW_STRING_H
#define __DRAW_STRING_H

#include "cseries/cseries.h"
#include "math/real_math.h"
#include "text/unicode.h"

/* ---------- constants */

/* what the next thing of a text is */
enum
{
	_text_element_end,
	_text_element_line_end,
	_text_element_tab,
	/* how the lines that follow are set: left, right or centre */
	_text_element_justification,
	_text_element_character,
	NUMBER_OF_TEXT_ELEMENTS
};

enum
{
	_text_justification_left,
	_text_justification_right,
	_text_justification_center,
	NUMBER_OF_TEXT_JUSTIFICATIONS
};

enum
{
	/* the characters that the game draws with pictures of its own */
	FIRST_PRIVATE_CHARACTER= 0xE000,
	LAST_PRIVATE_CHARACTER= 0xE3FF,

	NUMBER_OF_FONTS= 11,
	MAXIMUM_NUMBER_OF_TAB_STOPS= 16,
	/* with its end */
	MAXIMUM_DRAW_STRING_CHARACTERS= 2048,
	/* in the flags of how texts are drawn: lines are ended where they can */
	_draw_string_wrap_bit= 0
};

/* the parts of the colour of a text are not all over this */
#define DRAW_STRING_MAXIMUM_WHITE 0.68f

/* ---------- structures */

/* where a text is gone through: its characters are of 32 bits */
#pragma pack(push, 2)
struct text_iterator
{
	long font_index;
	/* NULL if it is not there */
	void *font;
	const long *characters;
	/* of the next character */
	short index;
	short unknownE;
	short justification;
	/* the last character, and the one before it if it was one that is
	drawn; 0 if it was not */
	long character;
	long previous_character;
	/* a line can end between the two */
	boolean can_break;
	/* lines are ended where they can: if not, only where the text says */
	boolean wrap;
	/* what the last thing was, and the one before it */
	long element;
	long previous_element;
	/* of the text and of its shadow: 0 if it has none */
	unsigned long color;
	unsigned long shadow_color;
};
#pragma pack(pop)

/* a rectangle of pixels: its top, left, bottom and right */
struct rectangle2d
{
	short y0;
	short x0;
	short y1;
	short x1;
};

/* where a text ends */
struct draw_string_point
{
	short x;
	short y;
};

/* 0x004e7394: the bounds of the text that is measured, and how much the
font that it is measured with goes up and down from the line */
struct draw_string_bounds
{
	struct rectangle2d bounds;
	short ascending_height;
	short descending_height;
};

/* how much of a text fits in a line: how wide it is, and the index of the
character after it. It is returned in two registers */
struct draw_string_line
{
	real width;
	long index;
};

/* what is done with each character of a text that is gone through: it is
told where the text is gone through, the font, the character, its colour,
where what is seen of it is and how wide and high it is, how much of it is
not seen at its left and at its top, and the scale. It takes its arguments
off the stack itself */
#define DRAW_STRING_CALLBACK __attribute__((stdcall))

typedef void (DRAW_STRING_CALLBACK *draw_string_character_proc)(const struct text_iterator *iterator,
	long font_index, long character, unsigned long color, long unknown5, real x, real y, real skipped_width,
	real skipped_height, real width, real height, real scale, long unknown13, long unknown14);

/* 0x004e73a0: how the texts are drawn */
struct draw_string_globals
{
	long font_index;
	unsigned long flags;
	long style;
	long justification;
	real_argb_color color;
	boolean shadow;
	byte pad21[3];
	real_argb_color shadow_color;
	byte unknown34[8];
	short tab_stop_count;
	short tab_stops[MAXIMUM_NUMBER_OF_TAB_STOPS];
	/* how far from the left of where a text is drawn its first line begins,
	and the others */
	short first_line_indent;
	short line_indent;
	byte pad62[2];
};

typedef char verify_draw_string_globals_size[
	sizeof(struct draw_string_globals) == 0x64 ? 1 : -1];
typedef char verify_draw_string_globals_tab_stop_count_offset[
	offsetof(struct draw_string_globals, tab_stop_count) == 0x3C ? 1 : -1];

typedef char verify_text_iterator_size[
	sizeof(struct text_iterator) == 0x2C ? 1 : -1];
typedef char verify_text_iterator_element_offset[
	offsetof(struct text_iterator, element) == 0x1C ? 1 : -1];

/* ---------- prototypes/DRAW_STRING.C */

void draw_string_initialize(void);
void DRAW_STRING_CALLBACK draw_string_bounds_character(const struct text_iterator *iterator, long font_index,
	long character, unsigned long color, long unknown5, real x, real y, real skipped_width, real skipped_height,
	real width, real height, real scale, long unknown13, long unknown14);
void draw_string_get_bounds(const wchar *string, const struct rectangle2d *rectangle, struct rectangle2d *bounds,
	struct rectangle2d *cursor, real scale);
void draw_string_get_characters_bounds(struct rectangle2d *cursor, struct rectangle2d *bounds,
	const struct rectangle2d *rectangle, const long *characters, real scale);
struct draw_string_line DRAW_STRING_CALLBACK draw_string_measure_line(struct text_iterator *iterator,
	const real_point2d *position, const real_rectangle2d *bounds, real scale);
void draw_string_set_tab_stops(short count, const short *tab_stops);
void draw_string_set_color_exact(const real_argb_color *color);
void draw_string_set_color(const real_argb_color *color);
void draw_string_set_shadow_color(const real_argb_color *color);
void draw_string_set_font(long font_index);
void draw_string_set_font_and_format(long font_index, const real_argb_color *shadow_color, long style,
	long justification, long flags, const real_argb_color *color);
boolean draw_string_precache_string(const wchar *string, long font_index);
boolean draw_string_precache_characters(const long *characters, long font_index);
boolean draw_string_precache(const wchar *string);

/* ---------- globals */

extern long font_slot_indices[NUMBER_OF_FONTS];
boolean text_iterator_new(struct text_iterator *iterator, long font_index, short justification,
	const long *characters, short unknown, const real_argb_color *color, const boolean *shadow,
	const real_argb_color *shadow_color);
void text_iterator_next(struct text_iterator *iterator);
boolean character_is_private_picture(long character);

#endif
