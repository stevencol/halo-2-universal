/*
FONTS.H

The fonts that the game has loaded. The names are guesses.
*/

#ifndef __FONTS_H
#define __FONTS_H

#include "cseries/cseries.h"

#include <stdint.h>

/* ---------- constants */

enum
{
	NUMBER_OF_FONT_SLOTS= 10,
	MAXIMUM_FONT_KERNING_PAIRS= 96,
	/* the characters that have kerning are of 8 bits */
	NUMBER_OF_FONT_KERNING_CHARACTERS= 256,
	/* the height of a line of a font that is not there */
	FONT_DEFAULT_LINE_HEIGHT= 10,
	/* the priority that the task that loads a font gets when the font is
	asked for */
	FONT_LOAD_PRIORITY= 6
};

/* ---------- structures */

/* how much nearer or farther a character is drawn when it is after another */
struct font_kerning_pair
{
	byte first;
	byte second;
	short offset;
};

/* a place for a font: what the game keeps of one that it has loaded */
struct font
{
	byte unknown0[4];
	/* a line is as high as the three */
	short ascending_height;
	short descending_height;
	short leading_height;
	byte unknownA[0x16];
	/* in the order of their first characters, and of the second ones */
	long kerning_pair_count;
	struct font_kerning_pair kerning_pairs[MAXIMUM_FONT_KERNING_PAIRS];
	/* a bit for each character that is the first of a pair */
	unsigned long kerning_characters[NUMBER_OF_FONT_KERNING_CHARACTERS/32];
	/* the file of the font, open */
	uint32_t file;
	boolean loaded;
	boolean loading;
	byte pad1CA[2];
	/* the task that loads it */
	long load_task;
};

typedef char verify_font_size[
	sizeof(struct font) == 0x1D0 ? 1 : -1];
typedef char verify_font_kerning_characters_offset[
	offsetof(struct font, kerning_characters) == 0x1A4 ? 1 : -1];

/* ---------- prototypes/FONTS.C */

struct font *font_get(long font_slot_index);
long font_get_line_height(long font_index);
short font_get_kerning(const struct font *font, unsigned long first, unsigned long second);

/* ---------- globals */

extern struct font font_slots[NUMBER_OF_FONT_SLOTS];

#endif
