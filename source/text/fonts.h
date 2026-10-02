/*
FONTS.H

The fonts that the game has loaded. The names are guesses.
*/

#ifndef __FONTS_H
#define __FONTS_H

#include "cseries/cseries.h"
#include "cseries/async.h"

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
	FONT_LOAD_PRIORITY= 6,
	/* the priority of the task that loads a font that is not waited for,
	and the category of the tasks of the fonts */
	FONT_LOAD_PRIORITY_NO_WAIT= 2,
	FONT_TASK_CATEGORY= 7,

	FONT_NAME_LENGTH= 31,
	FONT_PATH_LENGTH= 256,
	/* in the file of a font: what goes to its place */
	FONT_FILE_HEADER_OFFSET= 0x200,

	/* a pixel of a character is a colour of 12 bits, and how opaque it is
	in its 4 high bits; in the file, the opacity has 3 */
	FONT_PIXEL_COLOR_MASK= 0xFFF,
	FONT_PIXEL_MAXIMUM_ALPHA= 7
};

/* what a byte of the pixels of a character in the file is, by its two high
bits */
enum
{
	_font_pixels_clear_run,
	_font_pixels_opaque_run,
	_font_pixels_one_and_run,
	_font_pixels_two
};

/* ---------- structures */

/* how much nearer or farther a character is drawn when it is after another */
struct font_kerning_pair
{
	byte first;
	byte second;
	short offset;
};

#define FONT_SIGNATURE 0xF0000001UL

/* what the task that loads a font is told */
struct font_load_parameters
{
	long font_slot_index;
	long unknown4;
	char name[FONT_NAME_LENGTH+1];
};

/* a place for a font: what the game keeps of one that it has loaded */
struct font
{
	/* what the file of a font starts with */
	unsigned long signature;
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

long ASYNC_CALLBACK font_load_work(void *parameters);
void font_load(long font_slot_index, const char *name, boolean wait);
struct font *font_get(long font_slot_index);
long font_get_line_height(long font_index);
short font_get_kerning(const struct font *font, unsigned long first, unsigned long second);
long font_character_decode_pixels(const byte *source, long size, word *pixels);

/* ---------- globals */

extern struct font font_slots[NUMBER_OF_FONT_SLOTS];
extern const char *font_directory;

#endif
