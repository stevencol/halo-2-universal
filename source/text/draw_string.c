/*
DRAW_STRING.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

Going through a text to draw it. The name of the file is Halo 1's; that
these functions are of it is a guess, and so are their names.
*/

#include "draw_string.h"

#include "text/font_cache.h"
#include "text/fonts.h"
#include "text/unicode.h"

#include <string.h>

/* ---------- prototypes of functions that are not decompiled */

/* 0x0013f0e0: goes through the characters of a text as they are drawn in a
rectangle, by a scale, and calls a function with each one that is not out
of another rectangle, if there is one; where the text ends is left in a
place. The lines can be farther from each other than the font says */
void DRAW_STRING_CALLBACK unknown_0013f0e0(draw_string_character_proc proc, const struct rectangle2d *rectangle,
	struct draw_string_point *end, const struct rectangle2d *clip, short leading, real scale, const long *characters);

/* ---------- globals */

/* 0x004e73a0: how texts are drawn */
struct draw_string_globals draw_string_globals;

/* 0x004e7394: the bounds of the text that is measured, and two heights of
its font */
struct draw_string_bounds draw_string_bounds;

/* 0x004e28f4: the place of each font */
long font_slot_indices[NUMBER_OF_FONTS];

/* ---------- code */

/* the characters of a text of the game, in which a bar and a letter are an
order (unicode_escape_character): 2047 at most, and their end. The game has
this in each function that takes a text */
static inline __attribute__((always_inline)) void draw_string_parse(
	const wchar *string,
	long *characters)
{
	long count;

	for (count= MAXIMUM_DRAW_STRING_CHARACTERS; count>0; count--)
	{
		long value;

		if (count==1)
		{
			*characters= 0;
			break;
		}
		value= *string++;
		if (value=='|')
		{
			boolean found= FALSE;

			value= unicode_escape_character(*string, &found);
			if (found)
			{
				string++;
			}
		}
		*characters++= value;
		if (!value)
		{
			break;
		}
	}
}

/* 0x0013e8a0: texts are drawn with the first font, white, with no shadow and
no tabs, to the left */
void draw_string_initialize(
	void)
{
	memset(&draw_string_globals, 0, sizeof(draw_string_globals));
	draw_string_globals.font_index= 0;
	draw_string_globals.tab_stop_count= 0;
	draw_string_globals.flags= 0;
	draw_string_globals.justification= 0;
	draw_string_globals.first_line_indent= 0;
	draw_string_globals.line_indent= 0;
	draw_string_set_color(global_real_argb_white);
}

/* 0x0013e8e0: what is done with each character when a text is measured: the
bounds of the text grow to hold it. What is seen of the character is so wide
and so high, by a scale, from a place; of what else is told of it, only the
font is looked at */
void DRAW_STRING_CALLBACK draw_string_bounds_character(
	const struct text_iterator *iterator,
	long font_index,
	long character,
	unsigned long color,
	long unknown5,
	real x,
	real y,
	real skipped_width,
	real skipped_height,
	real width,
	real height,
	real scale,
	long unknown13,
	long unknown14)
{
	const struct font *font= font_get(font_slot_indices[font_index]);
	real right= width*scale + x + 1.0f;
	real bottom= height*scale + y + 1.0f;

	(void)iterator;
	(void)character;
	(void)color;
	(void)unknown5;
	(void)skipped_width;
	(void)skipped_height;
	(void)unknown13;
	(void)unknown14;

	if ((real)draw_string_bounds.bounds.x0>x)
	{
		draw_string_bounds.bounds.x0= (short)(long)x;
	}
	if ((real)draw_string_bounds.bounds.y0>y)
	{
		draw_string_bounds.bounds.y0= (short)(long)y;
	}
	if (right>(real)draw_string_bounds.bounds.x1)
	{
		draw_string_bounds.bounds.x1= (short)(long)right;
	}
	if (bottom>(real)draw_string_bounds.bounds.y1)
	{
		draw_string_bounds.bounds.y1= (short)(long)bottom;
	}
	if (font)
	{
		draw_string_bounds.ascending_height= font->ascending_height;
		draw_string_bounds.descending_height= font->descending_height;
	}
}

/* 0x0013e9c0: how much room a text of the game takes, drawn in a rectangle
by a scale: see draw_string_get_characters_bounds */
void draw_string_get_bounds(
	const wchar *string,
	const struct rectangle2d *rectangle,
	struct rectangle2d *bounds,
	struct rectangle2d *cursor,
	real scale)
{
	long characters[MAXIMUM_DRAW_STRING_CHARACTERS];

	draw_string_parse(string, characters);
	draw_string_get_characters_bounds(cursor, bounds, rectangle, characters, scale);
}

/* 0x0013ea60: how much room the characters of a text take, with how texts
are drawn now: the text is gone through as if it were drawn, and each
character makes the bounds grow. Also where the text ends: a place one pixel
wide, as high as a line */
void draw_string_get_characters_bounds(
	struct rectangle2d *cursor,
	struct rectangle2d *bounds,
	const struct rectangle2d *rectangle,
	const long *characters,
	real scale)
{
	const struct font *font= font_get(font_slot_indices[draw_string_globals.font_index]);
	struct draw_string_point end;

	draw_string_bounds.bounds.y0= 0x7FFF;
	draw_string_bounds.bounds.x0= 0x7FFF;
	draw_string_bounds.bounds.y1= -0x8000;
	draw_string_bounds.bounds.x1= -0x8000;
	if (font)
	{
		draw_string_bounds.ascending_height= font->ascending_height;
		draw_string_bounds.descending_height= font->descending_height;
	}
	unknown_0013f0e0(draw_string_bounds_character, rectangle, &end, NULL, 0, scale, characters);
	cursor->x0= end.x;
	cursor->x1= end.x+1;
	cursor->y0= end.y-draw_string_bounds.ascending_height;
	cursor->y1= draw_string_bounds.descending_height+end.y;
	*bounds= draw_string_bounds.bounds;
}

/* 0x0013ef40: how much of a text fits in a line, from where it is gone
through: the characters are taken while they end before the right of the
bounds, from a place, and the first one always. With lines that end where
they can (_draw_string_wrap_bit), the line ends at the last place where it
can before the character that does not fit, if there is one; with no such
place, and with lines that end only where the text says, the characters go
on over the right, and they are not counted in the width. The line also ends
at what is not a character */
struct draw_string_line DRAW_STRING_CALLBACK draw_string_measure_line(
	struct text_iterator *iterator,
	const real_point2d *position,
	const real_rectangle2d *bounds,
	real scale)
{
	struct draw_string_line line;
	long index= 0;
	real width= 0.0f;
	long break_index= 0;
	real break_width= 0.0f;
	long count= 0;
	boolean done= FALSE;

	do
	{
		text_iterator_next(iterator);
		if (iterator->element!=_text_element_character)
		{
			done= TRUE;
		}
		else
		{
			const struct font_character_header *header= font_character_get_header(iterator->character,
				iterator->font_index, FLAG(_font_character_request_wait_bit)|FLAG(_font_character_request_new_bit));

			if (header)
			{
				real kerning= (real)font_get_kerning(iterator->font, iterator->previous_character,
					iterator->character)*scale;
				real origin= (real)header->origin_x*scale;
				real advance= (real)header->advance*scale;

				if (iterator->can_break)
				{
					break_index= index;
					break_width= width;
				}
				if (bounds->x1>position->x + advance + origin + kerning + width || !count)
				{
					count++;
					width= advance + origin + kerning + width;
				}
				else if (draw_string_globals.flags&FLAG(_draw_string_wrap_bit))
				{
					done= TRUE;
					if (break_index>0)
					{
						index= break_index;
						width= break_width;
						break;
					}
				}
			}
		}
		index= iterator->index;
	} while (!done);
	line.width= width;
	line.index= index;

	return line;
}

/* 0x0013eb20: where the tabs of the texts that are drawn are: 16 at most */
void draw_string_set_tab_stops(
	short count,
	const short *tab_stops)
{
	if (count>MAXIMUM_NUMBER_OF_TAB_STOPS)
	{
		count= MAXIMUM_NUMBER_OF_TAB_STOPS;
	}
	draw_string_globals.tab_stop_count= count;
	if (count>0)
	{
		memcpy(draw_string_globals.tab_stops, tab_stops, (unsigned long)count*sizeof(short));
	}
}

/* a part of a colour, from 0 to 1 */
static inline __attribute__((always_inline)) real draw_string_color_part_pin(
	real part)
{
	if (part<0.0f)
	{
		return 0.0f;
	}
	if (part>1.0f)
	{
		return 1.0f;
	}

	return part;
}

/* 0x0013eb60: the colour of the texts that are drawn, as it is given: each
of its parts is taken into 0 to 1 */
void draw_string_set_color_exact(
	const real_argb_color *color)
{
	boolean valid=
		color->alpha>=0.0f && color->alpha<=1.0f &&
		color->rgb.red>=0.0f && color->rgb.red<=1.0f &&
		color->rgb.green>=0.0f && color->rgb.green<=1.0f &&
		color->rgb.blue>=0.0f && color->rgb.blue<=1.0f;

	draw_string_globals.color= *color;
	if (!valid)
	{
		draw_string_globals.color.alpha= draw_string_color_part_pin(draw_string_globals.color.alpha);
		draw_string_globals.color.rgb.red= draw_string_color_part_pin(draw_string_globals.color.rgb.red);
		draw_string_globals.color.rgb.green= draw_string_color_part_pin(draw_string_globals.color.rgb.green);
		draw_string_globals.color.rgb.blue= draw_string_color_part_pin(draw_string_globals.color.rgb.blue);
	}
}

/* 0x0013ec70: the colour of the texts that are drawn. One that is near white
is made darker: if its three parts are over 0.68, they are scaled so that the
least of them is 0.68 */
void draw_string_set_color(
	const real_argb_color *color)
{
	real_argb_color scaled= *color;

	if (scaled.rgb.red>DRAW_STRING_MAXIMUM_WHITE && scaled.rgb.green>DRAW_STRING_MAXIMUM_WHITE &&
		scaled.rgb.blue>DRAW_STRING_MAXIMUM_WHITE)
	{
		real least= scaled.rgb.green>scaled.rgb.blue ? scaled.rgb.blue : scaled.rgb.green;
		real scale;

		if (!(scaled.rgb.red>least))
		{
			least= scaled.rgb.red;
		}
		if (DRAW_STRING_MAXIMUM_WHITE>least)
		{
			least= DRAW_STRING_MAXIMUM_WHITE;
		}
		else if (least>1.0f)
		{
			least= 1.0f;
		}
		scale= DRAW_STRING_MAXIMUM_WHITE/least;
		scaled.rgb.red= scale*scaled.rgb.red;
		scaled.rgb.green= scale*scaled.rgb.green;
		scaled.rgb.blue= scale*scaled.rgb.blue;
	}
	draw_string_set_color_exact(&scaled);
}

/* the colour of the shadow of the texts that are drawn; none, if none is
given */
static inline __attribute__((always_inline)) void draw_string_set_shadow_color_inline(
	const real_argb_color *color)
{
	if (color)
	{
		draw_string_globals.shadow= TRUE;
		draw_string_globals.shadow_color= *color;
	}
	else
	{
		draw_string_globals.shadow= FALSE;
	}
}

/* 0x0013ed50 */
void draw_string_set_shadow_color(
	const real_argb_color *color)
{
	draw_string_set_shadow_color_inline(color);
}

/* the font of the texts that are drawn: the game waits until it is loaded */
static inline __attribute__((always_inline)) void draw_string_set_font_inline(
	long font_index)
{
	font_get(font_slot_indices[font_index]);
	draw_string_globals.font_index= font_index;
}

/* 0x0013ed90 */
void draw_string_set_font(
	long font_index)
{
	draw_string_set_font_inline(font_index);
}

/* 0x0013edb0: all of how the texts are drawn: the font, the colour, the
colour of the shadow, and the format */
void draw_string_set_font_and_format(
	long font_index,
	const real_argb_color *shadow_color,
	long style,
	long justification,
	long flags,
	const real_argb_color *color)
{
	draw_string_set_font_inline(font_index);
	draw_string_set_color(color);
	draw_string_set_shadow_color_inline(shadow_color);
	draw_string_globals.style= style;
	draw_string_globals.justification= justification;
	draw_string_globals.flags= flags;
}

/* 0x0013ee20: gets ready the characters of a text of the game, in which a
bar and a letter are an order (unicode_escape_character): 2047 at most */
boolean draw_string_precache_string(
	const wchar *string,
	long font_index)
{
	long characters[MAXIMUM_DRAW_STRING_CHARACTERS];

	draw_string_parse(string, characters);

	return draw_string_precache_characters(characters, font_index);
}

/* 0x0013eeb0: gets ready each of the characters of a text that are drawn,
with a font. Whether all of them could be */
boolean draw_string_precache_characters(
	const long *characters,
	long font_index)
{
	struct text_iterator iterator;
	boolean result= TRUE;

	if (text_iterator_new(&iterator, font_index, (short)draw_string_globals.justification, characters,
		(short)draw_string_globals.style, &draw_string_globals.color, &draw_string_globals.shadow,
		&draw_string_globals.shadow_color))
	{
		do
		{
			text_iterator_next(&iterator);
			if (iterator.element==_text_element_character && !font_character_precache(font_index, iterator.character))
			{
				result= FALSE;
			}
		} while (iterator.element!=_text_element_end);
	}

	return result;
}

/* 0x0013ef30: with the font of the texts that are drawn */
boolean draw_string_precache(
	const wchar *string)
{
	return draw_string_precache_string(string, draw_string_globals.font_index);
}

/* a colour as a number of 32 bits: each of its parts, from 0 to 1, times
255, with what is after the point cut. A part that is not from 0 to 1 goes
over the others */
static inline __attribute__((always_inline)) unsigned long draw_string_color_to_pixel32(
	real alpha,
	const real_rgb_color *color)
{
	unsigned long pixel= (unsigned long)(long)(alpha*255.0f);

	pixel= (pixel<<8) | (unsigned long)(long)(color->red*255.0f);
	pixel= (pixel<<8) | (unsigned long)(long)(color->green*255.0f);
	pixel= (pixel<<8) | (unsigned long)(long)(color->blue*255.0f);

	return pixel;
}

/* 0x0013f470: starts to go through a text, to draw it with a font and a
colour, and with a shadow of another colour if it has one: the shadow is no
more opaque than the text. Whether the font is there */
boolean text_iterator_new(
	struct text_iterator *iterator,
	long font_index,
	short justification,
	const long *characters,
	short unknown,
	const real_argb_color *color,
	const boolean *shadow,
	const real_argb_color *shadow_color)
{
	memset(iterator, 0, sizeof(*iterator));
	iterator->characters= characters;
	iterator->unknownE= unknown;
	iterator->justification= justification;
	iterator->font_index= font_index;
	iterator->index= 0;
	iterator->can_break= FALSE;
	iterator->wrap= (byte)draw_string_globals.flags&FLAG(_draw_string_wrap_bit);
	iterator->color= draw_string_color_to_pixel32(color->alpha, &color->rgb);
	if (*shadow)
	{
		iterator->shadow_color= draw_string_color_to_pixel32(
			shadow_color->alpha>color->alpha ? color->alpha : shadow_color->alpha, &shadow_color->rgb);
	}
	else
	{
		iterator->shadow_color= 0;
	}
	iterator->font= font_get(font_slot_indices[font_index]);

	return iterator->font!=NULL;
}

/* 0x0013f5a0: the next thing of a text: a character, the end of a line, a
tab, how the lines that follow are set, or the end of the text. A line feed
is nothing: it is passed. Of a character that is drawn it is told whether a
line can end before it */
void text_iterator_next(
	struct text_iterator *iterator)
{
	long character;
	long element;

	do
	{
		character= iterator->characters[iterator->index];
		iterator->index++;
	} while (character=='\n');

	switch (character)
	{
	case 0:
		element= _text_element_end;
		break;
	case '\r':
		element= _text_element_line_end;
		break;
	case '\t':
		element= _text_element_tab;
		break;
	case _unicode_character_justify_left:
		iterator->justification= _text_justification_left;
		element= _text_element_justification;
		break;
	case _unicode_character_justify_right:
		iterator->justification= _text_justification_right;
		element= _text_element_justification;
		break;
	case _unicode_character_justify_center:
		iterator->justification= _text_justification_center;
		element= _text_element_justification;
		break;
	default:
		iterator->can_break= iterator->wrap ? character_can_break_line(iterator->character, character) : FALSE;
		element= _text_element_character;
		break;
	}

	iterator->previous_character= iterator->previous_element==_text_element_character ? iterator->character : 0;
	iterator->character= character;
	iterator->previous_element= iterator->element;
	iterator->element= element;
}

/* 0x0013f660: whether a character is one of the pictures of the game: one
of those from 0xE000 to 0xE3FF, but for some that are not pictures */
boolean character_is_private_picture(
	long character)
{
	if (character>=0xE112 && character<=0xE12B)
	{
		return FALSE;
	}
	if (character<FIRST_PRIVATE_CHARACTER || character>LAST_PRIVATE_CHARACTER)
	{
		return FALSE;
	}
	if (character<=0xE002 || character==0xE004 || character==0xE008)
	{
		return FALSE;
	}
	if ((character>=0xE106 && character<=0xE111) || (character>=0xE12C && character<=0xE131))
	{
		return FALSE;
	}

	return TRUE;
}
