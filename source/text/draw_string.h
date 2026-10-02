/*
DRAW_STRING.H

Going through a text to draw it. The name of the file is Halo 1's; that
these functions are of it is a guess, and so are their names.
*/

#ifndef __DRAW_STRING_H
#define __DRAW_STRING_H

#include "cseries/cseries.h"

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
	LAST_PRIVATE_CHARACTER= 0xE3FF
};

/* ---------- structures */

/* where a text is gone through: its characters are of 32 bits */
#pragma pack(push, 2)
struct text_iterator
{
	long unknown0;
	long unknown4;
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
	long unknown24;
	long unknown28;
};
#pragma pack(pop)

typedef char verify_text_iterator_size[
	sizeof(struct text_iterator) == 0x2C ? 1 : -1];
typedef char verify_text_iterator_element_offset[
	offsetof(struct text_iterator, element) == 0x1C ? 1 : -1];

/* ---------- prototypes/DRAW_STRING.C */

void text_iterator_next(struct text_iterator *iterator);
boolean character_is_private_picture(long character);

#endif
