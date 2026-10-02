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
	byte pad5E[2];
};

typedef char verify_draw_string_globals_size[
	sizeof(struct draw_string_globals) == 0x60 ? 1 : -1];
typedef char verify_draw_string_globals_tab_stop_count_offset[
	offsetof(struct draw_string_globals, tab_stop_count) == 0x3C ? 1 : -1];

typedef char verify_text_iterator_size[
	sizeof(struct text_iterator) == 0x2C ? 1 : -1];
typedef char verify_text_iterator_element_offset[
	offsetof(struct text_iterator, element) == 0x1C ? 1 : -1];

/* ---------- prototypes/DRAW_STRING.C */

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
boolean text_iterator_new(struct text_iterator *iterator, long font_index, short justification,
	const long *characters, short unknown, const real_argb_color *color, const boolean *shadow,
	const real_argb_color *shadow_color);
void text_iterator_next(struct text_iterator *iterator);
boolean character_is_private_picture(long character);

#endif
