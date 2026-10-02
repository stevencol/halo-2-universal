/*
DRAW_STRING.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

Going through a text to draw it. The name of the file is Halo 1's; that
these functions are of it is a guess, and so are their names.
*/

#include "draw_string.h"

#include "text/unicode.h"

#include <string.h>

/* ---------- prototypes of functions that are not decompiled */

/* 0x001224c0: the font of one of the places for fonts, once it is loaded:
it waits for it. NULL if there is none */
void *unknown_001224c0(long font_slot_index);

/* ---------- globals */

/* 0x004e73a4: how texts are drawn */
byte draw_string_flags;

/* 0x004e28f4: the place of each font */
long font_slot_indices[NUMBER_OF_FONTS];

/* ---------- code */

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
	iterator->wrap= draw_string_flags&FLAG(_draw_string_wrap_bit);
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
	iterator->font= unknown_001224c0(font_slot_indices[font_index]);

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
