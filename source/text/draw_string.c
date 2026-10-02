/*
DRAW_STRING.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

Going through a text to draw it. The name of the file is Halo 1's; that
these functions are of it is a guess, and so are their names.
*/

#include "draw_string.h"

#include "text/unicode.h"

/* ---------- code */

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
